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


#define OPTIK_TICKET   0
#define OPTIK_INTEGER  1
#define OPTIK_VERSION  OPTIK_INTEGER

#define OPTIK_RLS_ATOMIC   0
#define OPTIK_RLS_STORE    1
#define OPTIK_RLS_BARRIER  2
#define OPTIK_RLS_TYPE     OPTIK_RLS_STORE

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

static inline uint32_t
optik_get_version(optik_t ol)
{
  return ol.version;
}

static inline uint32_t
optik_get_n_locked(optik_t ol)
{
  return ol.version;
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
optik_trylock(optik_t* ol)
{
  uint32_t version = ol->version;
  if (unlikely(ol->ticket != version))
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
optik_is_locked(optik_t ol)
{
  if (ol.ticket != ol.version)
    {
      return 1;
    }
  return 0;
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
optik_lock_backoff(optik_t* ol)
{
  uint32_t ticket = FAI_U32(&ol->ticket);
  int distance;
  do
    {
      distance = (ticket - ol->version);
      if (distance == 0)
	{
	  break;
	}
      const uint32_t di = (distance > 0) ? distance : -distance;
      cpause(di << 7);
    }
  while (1);

  return 1;
}

static inline int
optik_num_queued(optik_t ol)
{
  return (ol.ticket - ol.version);
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
optik_lock_version_backoff(optik_t* ol, optik_t ol_old)
{
  uint32_t ticket = FAI_U32(&ol->ticket);
  int distance;
  do
    {
      distance = (ticket - ol->version);
      if (distance == 0)
	{
	  break;
	}
      const uint32_t di = (distance > 0) ? distance : -distance;
      cpause(di << 8);
    }
  while (1);

  return (ol_old.version == ticket);
}

static inline int
optik_lock_vdelete(optik_t* ol)
{
  if (ol->version == INT_MAX)
    {
      return 0;
    }

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
optik_lock_if_not_deleted(optik_t* ol)
{
  if (ol->version == INT_MAX)
    {
      return 0;
    }

  uint32_t ticket = FAI_U32(&ol->ticket);
  while (ticket != ol->version)
    {
      OPTIK_PAUSE();
      if (ol->version == INT_MAX)
	{
	  return 0;
	}
    }

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
#  if OPTIK_RLS_ATOMIC == OPTIK_RLS_ATOMIC || OPTIK_RLS_ATOMIC == OPTIK_RLS_BARRIER
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
  FAD_U32(&ol->ticket);
}


#elif OPTIK_VERSION == OPTIK_INTEGER

#  define OPTIK_INIT    0
#  define OPTIK_LOCKED  0x1LL //odd values -> locked

typedef volatile uint64_t optik_t;


static inline int
optik_is_locked(optik_t ol)
{
  return (ol & OPTIK_LOCKED);
}

static inline uint32_t
optik_get_version(optik_t ol)
{
  return ol;
}

static inline uint32_t
optik_get_n_locked(optik_t ol)
{
  return ol >> 1;
}

static inline void
optik_init(optik_t* ol)
{
  *ol = 0;
}

static inline int
optik_trylock_version(optik_t* ol, optik_t ol_old)
{
  optik_t ol_new = *ol;
  if (unlikely(optik_is_locked(ol_new) || ol_new != ol_old))
    {
      return 0;
    }

  return CAS_U64(ol, ol_old, ++ol_new) == ol_old;
}

static inline int
optik_lock(optik_t* ol)
{
  optik_t ol_old;
  do
    {
      while (1)
	{
	  ol_old = *ol;
	  if (!optik_is_locked(ol_old))
	    {
	      break;
	    }
	  OPTIK_PAUSE();
	}

      if (CAS_U64(ol, ol_old, ol_old + 1) == ol_old)
	{
	  break;
	}
    }
  while (1);
  return 1;
}

static inline int
optik_lock_backoff(optik_t* ol)
{
  optik_t ol_old;
  do
    {
      while (1)
	{
	  ol_old = *ol;
	  if (!optik_is_locked(ol_old))
	    {
	      break;
	    }
	  cpause(128);
	}

      if (CAS_U64(ol, ol_old, ol_old + 1) == ol_old)
	{
	  break;
	}
    }
  while (1);
  return 1;
}

static inline int
optik_lock_version(optik_t* ol, optik_t ol_old)
{
  optik_t ol_cur;
  do
    {
      while (1)
	{
	  ol_cur = *ol;
	  if (!optik_is_locked(ol_cur))
	    {
	      break;
	    }
	  OPTIK_PAUSE();
	}

      if (CAS_U64(ol, ol_cur, ol_cur + 1) == ol_cur)
	{
	  break;
	}
    }
  while (1);
  return ol_cur == ol_old;
}

static inline int
optik_lock_version_backoff(optik_t* ol, optik_t ol_old)
{
  optik_t ol_cur;
  do
    {
      while (1)
	{
	  ol_cur = *ol;
	  if (!optik_is_locked(ol_cur))
	    {
	      break;
	    }
	  cpause(128);
	}

      if (CAS_U64(ol, ol_cur, ol_cur + 1) == ol_cur)
	{
	  break;
	}
    }
  while (1);
  return ol_cur == ol_old;
}

static inline int
optik_trylock(optik_t* ol)
{
  optik_t ol_new = *ol;
  if (unlikely(optik_is_locked(ol_new)))
    {
      return 0;
    }
  return CAS_U64(ol, ol_new, ol_new + 1) == ol_new;
}

static inline void
optik_unlock(optik_t* ol)
{
#  ifdef __tile__
  MEM_BARRIER;
#  endif
#  if OPTIK_RLS_ATOMIC == OPTIK_RLS_ATOMIC || OPTIK_RLS_ATOMIC == OPTIK_RLS_BARRIER
  COMPILER_NO_REORDER((*ol)++);
#    if OPTIK_RLS_ATOMIC == OPTIK_RLS_BARRIER
  asm volatile ("sfence");
#    endif
#  elif OPTIK_RLS_TYPE == OPTIK_RLS_ATOMIC
  FAI_U64(ol);
#  endif
}
static inline void
optik_revert(optik_t* ol)
{
  FAD_U64(ol);
}

#endif	/* OPTIK_VERSION */

#endif	/* _H_OPTIK_ */
