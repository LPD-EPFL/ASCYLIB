/*
 * File: mcs.h
 * Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Vasileios Trigonakis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef _MCS_IN_H_
#define _MCS_IN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <malloc.h>
#include <limits.h>
#include <assert.h>

#if !defined(__x86_64__)
#  error This file is designed to work only on x86_64 architectures! 
#endif

#define LOCK_IN_NAME "MCS"

/* ******************************************************************************** */
/* settings *********************************************************************** */
#define USE_FUTEX_COND 1	/* use futex-based (1) or spin-based futexs */
#define PADDING        1        /* padd locks/conditionals to cache-line */
#define FREQ_CPU_GHZ   2.8	/* core frequency in GHz */
#define REPLACE_MUTEX  1	/* ovewrite the pthread_[mutex|cond] functions */
#define MCS_MAX_THR    81
/* ******************************************************************************** */
 

static inline void* 
swap_ptr(volatile void* ptr, void *x) 
{
  asm volatile("xchgq %0,%1"
	       :"=r" ((unsigned long long) x)
	       :"m" (*(volatile long long *)ptr), "0" ((unsigned long long) x)
	       :"memory");

  return x;
}


#define CACHE_LINE_SIZE 64

#if !defined(PAUSE_IN)
#  define PAUSE_IN()			\
  ;
#endif

typedef struct mcs_lock 
{
  volatile uint64_t waiting;
  volatile struct mcs_lock* next;
} mcs_lock_t;

typedef struct mcs_lock_local
{
  volatile uint64_t waiting;
  volatile struct mcs_lock* next;
#if PADDING == 1
  uint8_t padding[CACHE_LINE_SIZE - sizeof(uint64_t) - sizeof(struct mcs_lock*)];
#endif
} mcs_lock_local_t;

#define MCS_LOCK_INITIALIZER { .waiting = 0, .next = NULL, .local = { 0 } }


extern __thread mcs_lock_local_t __mcs_local;

static inline mcs_lock_t*
mcs_get_local(lock)
{
  return (mcs_lock_t*) &__mcs_local;
}


static inline int
mcs_lock_trylock(mcs_lock_t* lock) 
{
  if (lock->next != NULL)
    {
      return 1;
    }

  volatile mcs_lock_t* local = mcs_get_local();
  local->next = NULL;
  return __sync_val_compare_and_swap(&lock->next, NULL, local) != NULL;
}

static inline int
mcs_lock_lock(mcs_lock_t* lock) 
{
  volatile mcs_lock_t* local = mcs_get_local(lock);
  local->next = NULL;
  
  mcs_lock_t* pred = swap_ptr((void*) &lock->next, (void*) local);

  if (pred == NULL)  		/* lock was free */
    {
      return 0;
    }
  local->waiting = 1; // word on which to spin
  pred->next = local; // make pred point to me
  while (local->waiting != 0) 
    {
      PAUSE_IN();
    }
  return 0;
}


static inline int
mcs_lock_unlock(mcs_lock_t* lock) 
{
  volatile mcs_lock_t* local = mcs_get_local();

  volatile mcs_lock_t* succ;

  if (!(succ = local->next)) /* I seem to have no succ. */
    { 
      /* try to fix global pointer */
      if (__sync_val_compare_and_swap(&lock->next, local, NULL) == local) 
	{
	  return 0;
	}
      do 
	{
	  succ = local->next;
	  PAUSE_IN();
	} 
      while (!succ); // wait for successor
    }
  succ->waiting = 0;
  return 0;
}

static inline int 
mcs_lock_init(mcs_lock_t* lock, pthread_mutexattr_t* a)
{
  lock->next = NULL;
  asm volatile ("mfence");
  return 0;
}

static inline int 
mcs_lock_destroy(mcs_lock_t* the_lock)
{
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif


