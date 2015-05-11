/*   
 *   File: optik.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   bst.h is part of ASCYLIB
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
 * 	     	      Tudor David <tudor.david@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * ASCYLIB is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _H_OPTIK_
#define _H_OPTIK_

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <atomic_ops.h>

#include "common.h"
#include "utils.h"


typedef union optik32
{
  struct
  {
    volatile uint16_t version;
    volatile uint16_t ticket;
  };
  volatile uint32_t to_uint32;
} optik32_t;


typedef union tl
{
  optik32_t lr[2];
  uint64_t to_uint64;
} tl_t;


#define OPTIK_TICKET   0
#define OPTIK_INTEGER  1
#define OPTIK_VERSION  OPTIK_TICKET

#define OPTIK_RLS_ATOMIC   0
#define OPTIK_RLS_STORE    1
#define OPTIK_RLS_BARRIER  2
#define OPTIK_RLS_TYPE     OPTIK_RLS_ATOMIC

#define OPTIK_PAUSE() asm volatile ("mfence");


#if OPTIK_VERSION == OPTIK_TICKET

#  define OPTIK_INIT {{0}}

typedef volatile union optik
{
  /* uint8_t padding[64]; */
  struct
  {
    volatile uint32_t version;
    volatile uint32_t ticket;
  };
  volatile uint64_t to_uint64;
} optik_t;

static inline void
optik_init(optik_t* ol)
{
  ol->to_uint64 = 0;
}

static inline int
optik_is_same_version(optik_t v1, optik_t v2)
{
  return v1.to_uint64 == v2.to_uint64;
}

static inline int
optik_trylock_version(optik_t* ol, optik_t ol_old)
{
  uint32_t version = ol_old.version;
  if (unlikely(version != ol_old.ticket))
    {
      return 0;
    }

#  if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
  optik_t olo = { .version = version, .ticket = version };
  optik_t oln = { .version = version, .ticket = (version + 1) };
#  else
  optik_t olo = { version, version };
  optik_t oln = { version, (version + 1) };
#  endif
  return CAS_U64(&ol->to_uint64, olo.to_uint64, oln.to_uint64) == olo.to_uint64;
}

static inline int
optik_lock(optik_t* ol)
{
  uint32_t ticket = FAI_U32(&ol->ticket);
  while (ticket != ol->version)
    {
      OPTIK_PAUSE();
    }

  return 1;
}

static inline int
optik_lock_version(optik_t* ol, optik_t ol_old)
{
  uint32_t ticket = FAI_U32(&ol->ticket);
  while (ticket != ol->version)
    {
      OPTIK_PAUSE();
    }

  return (ol_old.version == ticket);
}

static inline int
optik_lock_vdelete(optik_t* ol)
{
  uint32_t ticket = FAI_U32(&ol->ticket);
  while (ticket != ol->version)
    {
      OPTIK_PAUSE();
      if (ol->version == INT_MAX)
	{
	  return 0;
	}
    }

  ol->version = INT_MAX;
  return 1;
}

static inline int
optik_trylock_vdelete(optik_t* ol, optik_t ol_old)
{
  uint32_t version = ol_old.version;
  if (unlikely(version != ol_old.ticket))
    {
      return 0;
    }

#  if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
  optik_t olo = { .version = version, .ticket = version };
  optik_t oln = { .version = INT_MAX, .ticket = version };
#  else
  optik_t olo = { version, version };
  optik_t oln = { INT_MAX, version };
#  endif
  return CAS_U64(&ol->to_uint64, olo.to_uint64, oln.to_uint64) == olo.to_uint64;
}

static inline int
optik_is_deleted(optik_t ol)
{
  return ol.version == INT_MAX;
}

static inline void
optik_unlock(optik_t* ol)
{
#  ifdef __tile__
  MEM_BARRIER;
#  endif
#  if OPTIK_RLS_ATOMIC >= OPTIK_RLS_STORE
  COMPILER_NO_REORDER(ol->version++);
#    if OPTIK_RLS_ATOMIC == OPTIK_RLS_BARRIER
  asm volatile ("sfence");
#    endif
#  elif OPTIK_RLS_TYPE == OPTIK_RLS_ATOMIC
  FAI_U32(&ol->version);
#  endif
}

static inline optik_t
optik_unlockv(optik_t* ol)
{
#  ifdef __tile__
  MEM_BARRIER;
#  endif
  return (optik_t) IAF_U64(&ol->to_uint64);
}

static inline void
optik_revert(optik_t* ol)
{
#  if OPTIK_RLS_ATOMIC >= OPTIK_RLS_STORE
  COMPILER_NO_REORDER(ol->ticket--);
#    if OPTIK_RLS_ATOMIC == OPTIK_RLS_BARRIER
  asm volatile ("sfence");
#    endif
#  elif OPTIK_RLS_TYPE == OPTIK_RLS_ATOMIC
  FAD_U32(&ol->ticket);
#  endif
}


#elif OPTIK_VERSION == OPTIK_INTEGER

#  define OPTIK_INIT    {{0}}
#  define OPTIK_LOCKED  INT_MAX

typedef union optik
{
  struct 
  {
    volatile uint32_t version;
    volatile uint32_t oversion;
  };
  volatile uint64_t to_uint64;
} optik_t;

static inline void
optik_lock_init(optik_t* ol)
{
  ol->to_uint64 = 0;
}

static inline int
optik_trylock_version(optik_t* ol, optik_t ol_old)
{
  uint32_t ov = ol_old.version;
  if (unlikely(ol->version != ov || ov == OPTIK_LOCKED))
    {
      return 0;
    }

  int res = (CAS_U32(&ol->version, ov, OPTIK_LOCKED) == ov);
  if (likely(res))
    {
      ol->oversion = ov;
    }
  return res;
}

static inline int
optik_lock(optik_t* ol)
{
  do
    {
      const uint32_t ov = ol->version;
      if ((ov != OPTIK_LOCKED) && CAS_U32(&ol->version, ov, OPTIK_LOCKED) == ov)
	{
	  break;
	}
      OPTIK_PAUSE();
    }
  while (1);
  return 1;
}

static inline int
optik_lock_version(optik_t* ol, optik_t ol_old)
{
  const uint32_t ov = ol_old.version;
  if ((ov != OPTIK_LOCKED) && CAS_U32(&ol->version, ov, OPTIK_LOCKED) == ov)
    {
      ol->oversion = ov;
      return 1;
    }

  optik_lock(ol);
  return 0;
}

static inline void
optik_unlock(optik_t* ol)
{
#  ifdef __tile__
  MEM_BARRIER;
#  endif
   /* COMPILER_NO_REORDER(ol->version++); */
  COMPILER_NO_REORDER(ol->version = ol->oversion + 1;);
}

static inline void
optik_revert(optik_t* ol)
{
  /* COMPILER_NO_REORDER(ol->ticket--); */
  COMPILER_NO_REORDER(ol->version = ol->oversion;);
}

#endif	/* OPTIK_VERSION */

#endif	/* _H_OPTIK_ */
