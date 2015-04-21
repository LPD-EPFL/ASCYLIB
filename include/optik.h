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


#define OPTIK_INIT {{0}}

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

typedef union optik
{
  struct
  {
    volatile uint32_t version;
    volatile uint32_t ticket;
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
  uint32_t version = ol_old.version;
  if (unlikely(version != ol_old.ticket))
    {
      return 0;
    }

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
  optik_t olo = { .version = version, .ticket = version };
  optik_t oln = { .version = version, .ticket = (version + 1) };
#else
  optik_t olo = { version, version };
  optik_t oln = { version, (version + 1) };
#endif
  return CAS_U64(&ol->to_uint64, olo.to_uint64, oln.to_uint64) == olo.to_uint64;
}

static inline void
optik_unlock(optik_t* ol)
{
#ifdef __tile__
  MEM_BARRIER;
#endif
   /* COMPILER_NO_REORDER(ol->version++); */
  FAI_U32(&ol->version);
}

static inline void
optik_revert(optik_t* ol)
{
  /* COMPILER_NO_REORDER(ol->ticket--); */
  FAD_U32(&ol->ticket);
}

/* static inline int */
/* tl_trylock_version(volatile tl_t* tl, volatile tl_t* tl_old, int right) */
/* { */
/*   uint16_t version = tl_old->lr[right].version; */
/*   if (unlikely(version != tl_old->lr[right].ticket)) */
/*     { */
/*       return 0; */
/*     } */

/* #if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6 */
/*   tl32_t tlo = { .version = version, .ticket = version }; */
/*   tl32_t tln = { .version = version, .ticket = (version + 1) }; */
/*   return CAS_U32(&tl->lr[right].to_uint32, tlo.to_uint32, tln.to_uint32) == tlo.to_uint32; */
/* #else */
/*   tl32_t tlo = { version, version }; */
/*   tl32_t tln = { version, (version + 1) }; */
/* #endif */
/*   return CAS_U32(&tl->lr[right].to_uint32, tlo.to_uint32, tln.to_uint32) == tlo.to_uint32; */
/* } */

/* #define TLN_REMOVED  0x0000FFFF0000FFFF0000LL */

/* static inline int */
/* tl_trylock_version_both(volatile tl_t* tl, volatile tl_t* tl_old) */
/* { */
/*   uint16_t v0 = tl_old->lr[0].version; */
/*   uint16_t v1 = tl_old->lr[1].version; */
/*   if (unlikely(v0 != tl_old->lr[0].ticket || v1 != tl_old->lr[1].ticket)) */
/*     { */
/*       return 0; */
/*     } */

/* #if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6 */
/*   tl_t tlo = { .to_uint64 = tl_old->to_uint64 }; */
/*   return CAS_U64(&tl->to_uint64, tlo.to_uint64, TLN_REMOVED) == tlo.to_uint64; */
/* #else */
/*   /\* tl_t tlo; *\/ */
/*   /\* tlo.uint64_t = tl_old->to_uint64; *\/ */
/*   uint64_t tlo = *(uint64_t*) tl_old; */

/*   return CAS_U64((uint64_t*) tl, tlo, TLN_REMOVED) == tlo; */
/* #endif */

/* } */

/* static inline void */
/* tl_unlock(volatile tl_t* tl, int right) */
/* { */
/* #ifdef __tile__ */
/*   MEM_BARRIER; */
/* #endif */
/*   COMPILER_NO_REORDER(tl->lr[right].version++); */
/* } */

/* static inline void */
/* tl_revert(volatile tl_t* tl, int right) */
/* { */
/*   /\* PREFETCHW(tl); *\/ */
/*   COMPILER_NO_REORDER(tl->lr[right].ticket--); */
/* } */

#endif	/* _H_OPTIK_ */
