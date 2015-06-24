/*   
 *   File: ssmp.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: interface of libssmp
 *   ssmp.h is part of ssmp
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2013  Vasileios Trigonakis
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
 *
 */

#ifndef _SSMP_H_
#define _SSMP_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sched.h>
#include <inttypes.h>
#include <malloc.h>
#include <pthread.h>

#ifdef PLATFORM_NUMA
#  include <numa.h>
#endif /* PLATFORM_NUMA */

/* ------------------------------------------------------------------------------- */
/* settings */
/* ------------------------------------------------------------------------------- */
#define SSMP_MEM_NAME        "/ssmp_mem2"
#define USE_ATOMIC           0		/* set to 1 to use atomic ops for synchronizing
					   msg flags */
#define SSMP_NUM_BARRIERS    16 /* number of available barriers */
#define SSMP_CACHE_LINE_SIZE 64
#define SSMP_FLAG_TYPE       volatile uint8_t

/* ------------------------------------------------------------------------------- */
/* defines */
/* ------------------------------------------------------------------------------- */

extern uint8_t id_to_core[];
extern const uint8_t node_to_node_hops[8][8];
typedef uint64_t ticks;

#define SSMP_BUF_EMPTY       0
#define SSMP_BUF_MESSG       1
#define SSMP_BUF_LOCKD       2

#ifdef SSMP_DEBUG
#  define PD(args...) printf("[%d] ", ssmp_id_); printf(args); printf("\n"); fflush(stdout)
#else
#  define PD(args...) 
#endif

#ifndef ALIGNED
#  if __GNUC__
#    define ALIGNED(N) __attribute__ ((aligned (N)))
#  else
#    define ALIGNED(N)
#  endif
#endif

#define SSMP_INC_ALIGN(v)			\
  while (v % SSMP_CACHE_LINE_SIZE)		\
    {						\
      v++;					\
    }

/* ------------------------------------------------------------------------------- */
/* types */
/* ------------------------------------------------------------------------------- */

/*
  ssmp_msg_t and ssmp_buf_t types for messages
  and ssmp_barrier_t type for barriers are defined
  per architecture in the corresponding arch files!
*/
#if defined(__sparc__)
#  include "ssmp_sparc.h"
#elif defined(__tile__)
#  include "ssmp_tile.h"
#elif defined(__x86_64__) | defined(__i386__)
#  include "ssmp_x86.h"
#endif

/*
  chunk type to be used for sending big messages
 */
typedef struct 
{
  volatile unsigned char data[SSMP_CHUNK_SIZE - 1];
  SSMP_FLAG_TYPE state;
} ssmp_chunk_t;


/*
  type used for color-based function, i.e. functions that operate
  on a subset of the cores according to a color function
*/
typedef struct ALIGNED(SSMP_CACHE_LINE_SIZE) ssmp_color_buf_struct
{
  uint64_t num_ues;
  SSMP_FLAG_TYPE** buf_state;
  volatile ssmp_msg_t** buf;
  uint8_t* from;
} ssmp_color_buf_t;

/* ------------------------------------------------------------------------------- */
/* init / term the MP system */
/* ------------------------------------------------------------------------------- */

/* initialize the system: called before forking */
extern void ssmp_init(int num_procs);
/* initilize the memory structures of the system: called by every proc after forking  */
extern void ssmp_mem_init(int id, int num_ues);
/* terminate the system */
extern void ssmp_term(void);

/* ------------------------------------------------------------------------------- */
/* sending functions */
/* ------------------------------------------------------------------------------- */

/* send the contents of msg to core to */
extern inline void ssmp_send(uint32_t to, volatile ssmp_msg_t* msg);
/* send the contents of msg to core to without checking whether the buffer
   for receiving messages of to if free */
extern inline void ssmp_send_no_sync(uint32_t to, volatile ssmp_msg_t* msg);
/* send a message of size length (> 64 bytes) */
extern inline void ssmp_send_big(int to, void* data, size_t length);

/* ------------------------------------------------------------------------------- */
/* broadcasting functions */
/* ------------------------------------------------------------------------------- */

/* broadcast msg to every other core */
extern inline void ssmp_broadcast(ssmp_msg_t* msg);

/* ------------------------------------------------------------------------------- */
/* receiving functions (blocking) */
/* ------------------------------------------------------------------------------- */

/* receive a message from core from */
extern inline void ssmp_recv_from(uint32_t from, volatile ssmp_msg_t* msg);
/* receive a message of size lenght (> 64 bytes) from core from */
extern inline void ssmp_recv_from_big(int from, void* data, size_t length);

/* blocking receive from any other process. Sender at msg->sender */
extern inline void ssmp_recv(ssmp_msg_t* msg);

/* ------------------------------------------------------------------------------- */
/* color-based recv fucntions */
/* ------------------------------------------------------------------------------- */

/* initialize the color buf data structure to be used with consequent ssmp_recv_color
   calls. A node is considered a participant if the call to color(ID) returns 1 */
extern void ssmp_color_buf_init(ssmp_color_buf_t* cbuf, int (*color)(int));
extern void ssmp_color_buf_free(ssmp_color_buf_t* cbuf);

/* blocking receive from any of the participants according to the color function */
extern inline void ssmp_recv_color(ssmp_color_buf_t* cbuf, ssmp_msg_t* msg);
/* blocking receive from any of the participants according to the color function.
   Fair version: starts checking for incoming messages from the coreID of the
   last sender + 1, so that not coreID = 0 has priority. */
extern inline void ssmp_recv_color_start(ssmp_color_buf_t* cbuf, ssmp_msg_t* msg);

/* ------------------------------------------------------------------------------- */
/* barrier functions */
/* ------------------------------------------------------------------------------- */
extern int ssmp_color_app(int id);

extern inline ssmp_barrier_t*  ssmp_get_barrier(int barrier_num);
/* initialize a barrier. The participants of the barrier can be provided either as a bitmap (supports
 upto 64 participants), or as a color function. The color function has priority over the bitmap. */
extern inline void ssmp_barrier_init(int barrier_num, long long int participants, int (*color)(int));
/* wait on a barrier until all participants reach this call*/
extern inline void ssmp_barrier_wait(int barrier_num);


/* ------------------------------------------------------------------------------- */
/* help functions */
/* ------------------------------------------------------------------------------- */

/* return the current cpu time in micros */
extern inline double wtime(void);
/* wait for a number of cycles */
extern inline void wait_cycles(uint64_t cycles);
/* call _mm_mpause() repeatedly */
extern inline void _mm_pause_rep(uint32_t num_reps);
/* get the number of interconnect hops between two cores on a NUMA machine */
extern inline uint32_t get_num_hops(uint32_t core1, uint32_t core2);

/* set the core on which the process will run on */
extern void set_cpu(int cpu);

/* get the value of the timestamp counter of the core */
extern inline ticks getticks(void);
/* the cost (in cycles) of a getticks call */
extern __thread ticks getticks_correction;
extern ticks getticks_correction_calc();

/* round up to next higher power of 2 (return x if it's already a power
   of 2) for 32-bit numbers */
extern inline uint32_t pow2roundup(uint32_t x);


/* 
 * Returns a pseudo-random value in [1;range).
 * Depending on the symbolic constant RAND_MAX>=32767 defined in stdlib.h,
 * the granularity of rand() could be lower-bounded by the 32767^th which might 
 * be too high for given values of range and initial.
 */
/* returns 1 if the two cores are on the same socket, else 0 */
extern inline uint32_t ssmp_cores_on_same_socket(uint32_t core1, uint32_t core2);
/* get the ssmp_id_ */
extern inline int ssmp_id();
/* get the number of processes */
extern inline int ssmp_num_ues();

/* --------------------------------------------------------------------------------------
 * headers for platform specific implementations
 * --------------------------------------------------------------------------------------
 */

extern void ssmp_init_platf(int num_procs);
extern void ssmp_mem_init_platf(int id, int num_ues);
extern void ssmp_term_platf(void);
extern inline void ssmp_send_platf(uint32_t to, volatile ssmp_msg_t* msg);
extern inline void ssmp_send_no_sync_platf(uint32_t to, volatile ssmp_msg_t* msg);
extern inline void ssmp_send_big_platf(int to, void* data, size_t length);
extern inline void ssmp_recv_from_platf(uint32_t from, volatile ssmp_msg_t* msg);
extern inline void ssmp_recv_from_big_platf(int from, void* data, size_t length);
extern inline void ssmp_recv_platf(ssmp_msg_t* msg);
extern void ssmp_color_buf_init_platf(ssmp_color_buf_t* cbuf, int (*color)(int));
extern void ssmp_color_buf_free_platf(ssmp_color_buf_t* cbuf);
extern inline void ssmp_recv_color_platf(ssmp_color_buf_t* cbuf, ssmp_msg_t* msg);
extern inline void ssmp_recv_color_start_platf(ssmp_color_buf_t* cbuf, ssmp_msg_t* msg);
extern inline void ssmp_barrier_init_platf(int barrier_num, long long int participants, int (*color)(int));
extern inline void ssmp_barrier_wait_platf(int barrier_num);
extern void set_cpu_platf(int cpu);
extern void set_numa_platf(int cpu);
extern inline ticks getticks_platf(void);

#endif
