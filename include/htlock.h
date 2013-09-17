/* Hierarchical ticket lock */

#ifndef _HTICKET_H_
#define _HTICKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#if defined(PLATFORM_MCORE)
#  include <numa.h>
#  include <emmintrin.h>
#endif
#include <pthread.h>
#include <assert.h>
#include "utils.h"

#define NB_TICKETS_LOCAL	128


typedef struct htlock_global
{
  volatile uint32_t nxt;
  volatile uint32_t cur;
  uint8_t padding[CACHE_LINE_SIZE - 8];
} htlock_global_t;

typedef struct htlock_local
{
  volatile int32_t nxt;
  volatile int32_t cur;
  uint8_t padding[CACHE_LINE_SIZE - 8];
} htlock_local_t;

typedef struct ALIGNED(CACHE_LINE_SIZE) htlock
{
  htlock_global_t* global;
  htlock_local_t* local[NUMBER_OF_SOCKETS];
} htlock_t;

typedef struct htlock ptlock_t;

extern htlock_t* create_htlock();
extern void init_htlock(htlock_t* htl); /* initiliazes an htlock */
extern void init_alloc_htlock(htlock_t* htl);
extern void init_thread_htlocks(uint32_t thread_num);
extern htlock_t* init_htlocks(uint32_t num_locks);
extern uint32_t is_free_hticket(htlock_t* htl);
extern void free_htlocks(htlock_t* locks);

extern void htlock_lock(htlock_t* l);
extern uint32_t htlock_trylock(htlock_t* l);

extern void htlock_release(htlock_t* l);
extern inline void htlock_release_try(htlock_t* l);	/* trylock rls */

static inline void 
wait_cycles(uint64_t cycles)
{
#if defined(__x86_64__)
  if (cycles < 256)
    {
      cycles /= 6;
      while (cycles--)
	{
        PAUSE;
	}
    }
  else
#endif
    {
      ticks _start_ticks = getticks();
      ticks _end_ticks = _start_ticks + cycles - 130;
      while (getticks() < _end_ticks);
    }
}

#endif	/* _HTICKET_H_ */


