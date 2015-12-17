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

#ifdef __tile__
#  error OPTIK does not yet include the appropriate memory barriers for TILERA.
#endif

#ifndef OPTIK_STATS
#  define OPTIK_STATS 0
#endif

#if OPTIK_STATS == 1
extern __thread size_t __optik_trylock_calls;
extern __thread size_t __optik_trylock_cas;
extern __thread size_t __optik_trylock_calls_suc;
extern size_t __optik_trylock_calls_tot;
extern size_t __optik_trylock_cas_tot;
extern size_t __optik_trylock_calls_suc_tot;
#  define OPTIK_STATS_VARS_DEFINITION()					\
  __thread size_t __optik_trylock_calls = 0;				\
  __thread size_t __optik_trylock_cas = 0;				\
  __thread size_t __optik_trylock_calls_suc = 0;			\
  size_t __optik_trylock_calls_tot = 0;					\
  size_t __optik_trylock_cas_tot = 0;					\
  size_t __optik_trylock_calls_suc_tot = 0

#  define OPTIK_STATS_TRYLOCK_CALLS_INC()       __optik_trylock_calls++;
#  define OPTIK_STATS_TRYLOCK_CAS_INC()	        __optik_trylock_cas++;
#  define OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(by) __optik_trylock_calls_suc+=by;
#  define OPTIK_STATS_PUBLISH()			\
  __sync_fetch_and_add(&__optik_trylock_calls_tot, __optik_trylock_calls); \
  __sync_fetch_and_add(&__optik_trylock_cas_tot, __optik_trylock_cas); \
  __sync_fetch_and_add(&__optik_trylock_calls_suc_tot, __optik_trylock_calls_suc)

#  define OPTIK_STATS_PRINT()						\
  printf("[OPTIK] %-10s tot: %-10zu | cas: %-10zu | suc: %-10zu | "	\
	 "succ-cas: %6.2f%% | succ-tot: %6.2f%% | cas/suc: %.2f\n", "trylock", \
	 __optik_trylock_calls_tot, __optik_trylock_cas_tot, __optik_trylock_calls_suc_tot, \
	 100 * (1 - ((double) (__optik_trylock_cas_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_cas_tot)), \
	 100 * (1 - ((double) (__optik_trylock_calls_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_calls_tot)), \
	 (double) __optik_trylock_cas_tot / __optik_trylock_calls_suc_tot)

#  define OPTIK_STATS_PRINT_DUR(dur_ms)					\
  printf("[OPTIK] %-10s tot: %-10.0f | cas: %-10.0f | suc: %-10.0f | "	\
	 "succ-cas: %6.2f%% | succ-tot: %6.2f%% | cas/suc: %.2f\n", "trylock/s", \
	 __optik_trylock_calls_tot / (dur_ms / 1000.0), __optik_trylock_cas_tot / (dur_ms / 1000.0), \
	 __optik_trylock_calls_suc_tot / (dur_ms / 1000.0),		\
	 100 * (1 - ((double) (__optik_trylock_cas_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_cas_tot)), \
	 100 * (1 - ((double) (__optik_trylock_calls_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_calls_tot)), \
	 (double) __optik_trylock_cas_tot / __optik_trylock_calls_suc_tot)


#elif OPTIK_STATS == 2 // only CAS
extern __thread size_t __optik_trylock_calls;
extern __thread size_t __optik_trylock_cas;
extern __thread size_t __optik_trylock_calls_suc;
extern size_t __optik_trylock_calls_tot;
extern size_t __optik_trylock_cas_tot;
extern size_t __optik_trylock_calls_suc_tot;
#  define OPTIK_STATS_VARS_DEFINITION()					\
  __thread size_t __optik_trylock_calls = 0;				\
  __thread size_t __optik_trylock_cas = 0;				\
  __thread size_t __optik_trylock_calls_suc = 0;			\
  size_t __optik_trylock_calls_tot = 0;					\
  size_t __optik_trylock_cas_tot = 0;					\
  size_t __optik_trylock_calls_suc_tot = 0

#  define OPTIK_STATS_TRYLOCK_CALLS_INC()
#  define OPTIK_STATS_TRYLOCK_CAS_INC()	        __optik_trylock_cas++;
#  define OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(by)
#  define OPTIK_STATS_PUBLISH()			\
  __sync_fetch_and_add(&__optik_trylock_cas_tot, __optik_trylock_cas);

#  define OPTIK_STATS_PRINT()						\
  printf("[OPTIK] %-10s tot: %-10zu | cas: %-10zu | suc: %-10zu | "\
	 "succ-cas: %6.2f%% | succ-tot: %6.2f%% | cas/suc: %.2f\n", "trylock", \
	 __optik_trylock_calls_tot, __optik_trylock_cas_tot, __optik_trylock_calls_suc_tot,	\
	 100 * (1 - ((double) (__optik_trylock_cas_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_cas_tot)), \
	 100 * (1 - ((double) (__optik_trylock_calls_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_calls_tot)), \
	 (double) __optik_trylock_cas_tot / __optik_trylock_calls_suc_tot)

#  define OPTIK_STATS_PRINT_DUR(dur_ms)					\
  printf("[OPTIK] %-10s tot: %-10.0f | cas: %-10.0f | suc: %-10.0f | "	\
	 "succ-cas: %6.2f%% | succ-tot: %6.2f%% | cas/suc: %.2f\n", "trylock/s", \
	 __optik_trylock_calls_tot / (dur_ms / 1000.0), __optik_trylock_cas_tot / (dur_ms / 1000.0), \
	 __optik_trylock_calls_suc_tot / (dur_ms / 1000.0),		\
	 100 * (1 - ((double) (__optik_trylock_cas_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_cas_tot)), \
	 100 * (1 - ((double) (__optik_trylock_calls_tot - __optik_trylock_calls_suc_tot) / __optik_trylock_calls_tot)), \
	 (double) __optik_trylock_cas_tot / __optik_trylock_calls_suc_tot)

#else
#  define OPTIK_STATS_VARS_DEFINITION()
#  define OPTIK_STATS_TRYLOCK_CALLS_INC()	   
#  define OPTIK_STATS_TRYLOCK_CAS_INC()	   
#  define OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(by)
#  define OPTIK_STATS_PUBLISH()
#  define OPTIK_STATS_PRINT()
#  define OPTIK_STATS_PRINT_DUR(dur_ms)
#endif


#define OPTIK_TICKET      0
#define OPTIK_INTEGER     1
#define OPTIK_SEPARATE    2
#ifndef OPTIK_VERSION
#  define OPTIK_VERSION   OPTIK_INTEGER
#endif

#define OPTIK_RLS_ATOMIC   0
#define OPTIK_RLS_STORE    1
#define OPTIK_RLS_BARRIER  2
#define OPTIK_RLS_TYPE     OPTIK_RLS_ATOMIC

#define OPTIK_PAUSE() asm volatile ("mfence");


#if OPTIK_VERSION == OPTIK_TICKET

static inline const char*
optik_get_type_name()
{
  return "OPTIK-ticket";
}

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
optik_is_locked(optik_t ol)
{
  if (ol.ticket != ol.version)
    {
      return 1;
    }
  return 0;
}

static inline int
optik_is_deleted(optik_t ol)
{
  return (ol.ticket < ol.version);
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
  OPTIK_STATS_TRYLOCK_CALLS_INC();
  uint32_t version = ol_old.version;
  if (unlikely(version != ol_old.ticket || version != ol->ticket))
    {
      return 0;
    }

  OPTIK_STATS_TRYLOCK_CAS_INC();

  optik_t olo = ol_old;
  ol_old.ticket++;
  optik_t oln = ol_old;

  int ret = CAS_U64(&ol->to_uint64, olo.to_uint64, oln.to_uint64) == olo.to_uint64;
  OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(ret);
  return ret;
}

static inline int
optik_trylock_vdelete(optik_t* ol, optik_t ol_old)
{
  OPTIK_STATS_TRYLOCK_CALLS_INC();
  uint32_t version = ol_old.version;
  if (unlikely(version != ol_old.ticket || version != ol->ticket))
    {
      return 0;
    }

  OPTIK_STATS_TRYLOCK_CAS_INC();
  optik_t olo = ol_old;
  ol_old.ticket = version - 1;
  optik_t oln = ol_old;

  int ret = CAS_U64(&ol->to_uint64, olo.to_uint64, oln.to_uint64) == olo.to_uint64;
  OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(ret);
  return ret;
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
  optik_t olo = {{ version, version }};
  optik_t oln = {{ version, (version + 1) }};
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
      cpause(di << 8);
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

#define OPTIK_DELETED ((uint64_t) -1)

static inline const char*
optik_get_type_name()
{
  return "OPTIK-integer";
}

#  define OPTIK_INIT    0
#  define OPTIK_LOCKED  0x1LL //odd values -> locked

typedef volatile uint64_t optik_t;


static inline int
optik_is_locked(optik_t ol)
{
  return (ol & OPTIK_LOCKED);
}

static inline optik_t
optik_get_version_wait(optik_t* ol)
{
  do
    {
      /* PREFETCHW(ol); */
      optik_t olv = *ol;
      if (likely(!optik_is_locked(olv)))
	{
	  return olv;
	}

      cpause(128);
    }
  while (1);
}

static inline int
optik_is_deleted(optik_t ol)
{
  return (ol == OPTIK_DELETED);
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
optik_is_same_version(optik_t v1, optik_t v2)
{
  return v1 == v2;
}

static inline int
optik_trylock_version(optik_t* ol, optik_t ol_old)
{
  OPTIK_STATS_TRYLOCK_CALLS_INC();
  if (unlikely(optik_is_locked(ol_old) || *ol != ol_old))
    {
      return 0;
    }

  OPTIK_STATS_TRYLOCK_CAS_INC();
  int ret = CAS_U64(ol, ol_old, ol_old + 1) == ol_old;
  OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(ret);
  return ret;
}

static inline int
optik_trylock_vdelete(optik_t* ol, optik_t ol_old)
{
  OPTIK_STATS_TRYLOCK_CALLS_INC();
  if (unlikely(optik_is_locked(ol_old) || *ol != ol_old))
    {
      return 0;
    }

  OPTIK_STATS_TRYLOCK_CAS_INC();
  int ret = CAS_U64(ol, ol_old, OPTIK_DELETED) == ol_old;
  OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(ret);
  return ret;
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

static inline optik_t
optik_unlockv(optik_t* ol)
{
#  ifdef __tile__
  MEM_BARRIER;
#  endif
  return (optik_t) IAF_U64(ol);
}


static inline void
optik_revert(optik_t* ol)
{
  FAD_U64(ol);
}

#elif OPTIK_VERSION == OPTIK_SEPARATE

static inline const char*
optik_get_type_name()
{
  return "OPTIK-separate";
}

#  define OPTIK_INIT    { 0, 0 }
#  define OPTIK_LOCKED  0x1 
#  define OPTIK_FREE    0x0 

typedef volatile struct 
{
  uint32_t lock;
  uint32_t version;
} optik_t;


static inline int
optik_is_locked(optik_t ol)
{
  return (ol.lock == OPTIK_LOCKED);
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

static inline void
optik_init(optik_t* ol)
{
  ol->lock = OPTIK_FREE;
  ol->version = 0;
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

      OPTIK_STATS_TRYLOCK_CAS_INC();
      if (CAS_U32(&ol->lock, 0, 1) == 0)
	{
	  break;
	}
    }
  while (1);
  return 1;
}

static inline int
optik_is_same_version(optik_t v1, optik_t v2)
{
  return v1.version == v2.version;
}


static inline int
optik_trylock_version(optik_t* ol, optik_t ol_old)
{
  OPTIK_STATS_TRYLOCK_CALLS_INC();
  optik_lock(ol);
  if (ol->version != ol_old.version)
    {
      ol->lock = OPTIK_FREE;
      return 0;
    }

  OPTIK_STATS_TRYLOCK_CALLS_SUC_INC(1);
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

      if (CAS_U32(&ol->lock, 0, 1) == 0)
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

      if (CAS_U32(&ol->lock, 0, 1) == 0)
	{
	  break;
	}
    }
  while (1);
  return ol_cur.version == ol_old.version;
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

      if (CAS_U32(&ol->lock, 0, 1) == 0)
	{
	  break;
	}
    }
  while (1);
  return ol_cur.version == ol_old.version;
}

static inline int
optik_trylock(optik_t* ol)
{
  if (unlikely(optik_is_locked(*ol)))
    {
      return 0;
    }
  return CAS_U32(&ol->lock, 0, 1) == 0;
}

static inline void
optik_unlock(optik_t* ol)
{
  ol->version++;
#  ifdef __tile__
  MEM_BARRIER;
#  endif
#  if OPTIK_RLS_ATOMIC == OPTIK_RLS_ATOMIC || OPTIK_RLS_ATOMIC == OPTIK_RLS_BARRIER
  COMPILER_NO_REORDER(ol->lock = 0);
#    if OPTIK_RLS_ATOMIC == OPTIK_RLS_BARRIER
  asm volatile ("sfence");
#    endif
#  elif OPTIK_RLS_TYPE == OPTIK_RLS_ATOMIC
  FAD_U32(&ol->lock);
#  endif
}
static inline void
optik_revert(optik_t* ol)
{
  /* FAD_U32(&ol->version); */
  optik_unlock(ol);
}

#endif	/* OPTIK_VERSION */

#endif	/* _H_OPTIK_ */
