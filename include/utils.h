#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_
//some utility functions
//#define USE_MUTEX_LOCKS
//#define ADD_PADDING
/* #define OPTERON */
/* #define OPTERON_OPTIMIZE */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef __sparc__
#  include <sys/types.h>
#  include <sys/processor.h>
#  include <sys/procset.h>
#elif defined(__tile__)
#  include <arch/atomic.h>
#  include <arch/cycle.h>
#  include <tmc/cpus.h>
#  include <tmc/task.h>
#  include <tmc/spin.h>
#  include <sched.h>
#else
#  if defined(PLATFORM_MCORE)
#    include <numa.h>
#  endif
#  if defined(__SSE__)
#    include <xmmintrin.h>
#else
#define _mm_pause() asm volatile ("nop")
#  endif
#  if defined(__SSE2__)
#    include <emmintrin.h>
#  endif
#endif
#include <pthread.h>
#include "getticks.h"
#include "random.h"
#include "measurements.h"
#include "ssalloc.h"
#include "atomic_ops_if.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DO_ALIGN
/* #define DO_PAD */

#if defined(DO_ALIGN)
#  define ALIGNED(N) __attribute__ ((aligned (N)))
#else
#  define ALIGNED(N)
#endif

#ifdef __sparc__
#  define PAUSE    asm volatile("rd    %%ccr, %%g0\n\t" \
				::: "memory")

#elif defined(__tile__)
#  define PAUSE cycle_relax()
#else
#  define PAUSE _mm_pause()
#endif
  static inline void
  pause_rep(uint32_t num_reps)
  {
    uint32_t i;
    for (i = 0; i < num_reps; i++)
      {
	PAUSE;
	/* PAUSE; */
	/* asm volatile ("NOP"); */
      }
  }

  static inline void
  nop_rep(uint32_t num_reps)
  {
    uint32_t i;
    for (i = 0; i < num_reps; i++)
      {
	asm volatile ("NOP");
      }
  }


  //machine dependent parameters
#ifdef __sparc__
#  define NUMBER_OF_SOCKETS 8
#  define CORES_PER_SOCKET 8
#  define CACHE_LINE_SIZE 64
#  define NOP_DURATION 9
  //mapping from threads to cores on the niagara
#  define ALTERNATE_SOCKETS
#  ifdef ALTERNATE_SOCKETS
  static uint8_t __attribute__ ((unused)) the_cores[] = {
    0, 8, 16, 24, 32, 40, 48, 56, 
    1, 9, 17, 25, 33, 41, 49, 57, 
    2, 10, 18, 26, 34, 42, 50, 58, 
    3, 11, 19, 27, 35, 43, 51, 59, 
    4, 12, 20, 28, 36, 44, 52, 60, 
    5, 13, 21, 29, 37, 45, 53, 61, 
    6, 14, 22, 30, 38, 46, 54, 62, 
    7, 15, 23, 31, 39, 47, 55, 63 
  };

  static uint8_t __attribute__ ((unused)) the_sockets[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    2, 2, 2, 2, 2, 2, 2, 2, 
    3, 3, 3, 3, 3, 3, 3, 3, 
    4, 4, 4, 4, 4, 4, 4, 4, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 6, 6, 6, 6, 6, 6, 6, 
    7, 7, 7, 7, 7, 7, 7, 7 
  };

  //static uint8_t __attribute__ ((unused)) the_sockets[] = {
  //    0, 1, 2, 3, 4, 5, 6, 7, 
  //    0, 1, 2, 3, 4, 5, 6, 7, 
  //    0, 1, 2, 3, 4, 5, 6, 7, 
  //    0, 1, 2, 3, 4, 5, 6, 7, 
  //    0, 1, 2, 3, 4, 4, 5, 7, 
  //    0, 1, 2, 3, 4, 5, 6, 7, 
  //    0, 1, 2, 3, 4, 5, 6, 7, 
  //    0, 1, 2, 3, 4, 5, 6, 7 
  //};

#  else
  static uint8_t __attribute__ ((unused)) the_cores[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 
    8, 9, 10, 11, 12, 13, 14, 15, 
    16, 17, 18, 19, 20, 21, 22, 23, 
    24, 25, 26, 27, 28, 29, 30, 31, 
    32, 33, 34, 35, 36, 37, 38, 39, 
    40, 41, 42, 43, 44, 45, 46, 47, 
    48, 49, 50, 51, 52, 53, 54, 55, 
    56, 57, 58, 59, 60, 61, 62, 63 
  };
  static uint8_t __attribute__ ((unused)) the_sockets[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    2, 2, 2, 2, 2, 2, 2, 2, 
    3, 3, 3, 3, 3, 3, 3, 3, 
    4, 4, 4, 4, 4, 4, 4, 4, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 6, 6, 6, 6, 6, 6, 6, 
    7, 7, 7, 7, 7, 7, 7, 7 
  };

#  endif
#endif	/* __sparc__ */

#if defined __tile__
#  define NUMBER_OF_SOCKETS 1
#  define CORES_PER_SOCKET 36
#  define CACHE_LINE_SIZE 64
#  define NOP_DURATION 4
  static uint8_t __attribute__ ((unused)) the_cores[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 
    8, 9, 10, 11, 12, 13, 14, 15, 
    16, 17, 18, 19, 20, 21, 22, 23, 
    24, 25, 26, 27, 28, 29, 30, 31, 
    32, 33, 34, 35 
  };
#endif	/*  */


#if defined(OPTERON)
#  define NUMBER_OF_SOCKETS 8
#  define CORES_PER_SOCKET 6
#  define CACHE_LINE_SIZE 64
#  define NOP_DURATION 2
  static uint8_t  the_cores[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 
    8, 9, 10, 11, 12, 13, 14, 15, 
    16, 17, 18, 19, 20, 21, 22, 23, 
    24, 25, 26, 27, 28, 29, 30, 31, 
    32, 33, 34, 35, 36, 37, 38, 39, 
    40, 41, 42, 43, 44, 45, 46, 47  
  };
#endif	/*  */

#if defined(XEON)
#  define NUMBER_OF_SOCKETS 8
#  define CORES_PER_SOCKET 10
#  define CACHE_LINE_SIZE 64
#  define NOP_DURATION 1
  static uint8_t __attribute__ ((unused)) the_cores[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    0, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79 
  };
  static uint8_t __attribute__ ((unused)) the_sockets[] = 
  {
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  };

#endif

#if defined(LAPTOP)
#  define NUMBER_OF_SOCKETS 1
#  define CORES_PER_SOCKET 8
#  define CACHE_LINE_SIZE 64
#  define NOP_DURATION 1
  static uint8_t  the_cores[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 
    8, 9, 10, 11, 12, 13, 14, 15, 
    16, 17, 18, 19, 20, 21, 22, 23, 
    24, 25, 26, 27, 28, 29, 30, 31, 
    32, 33, 34, 35, 36, 37, 38, 39, 
    40, 41, 42, 43, 44, 45, 46, 47  
  };
  static uint8_t __attribute__ ((unused)) the_sockets[] = 
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };

#endif


  /* PLATFORM specific -------------------------------------------------------------------- */
#if defined(OPTERON)
#  define PREFETCHW(x)		     asm volatile("prefetchw %0" :: "m" (*(unsigned long *)x))
#elif defined(__sparc__)
#  define PREFETCHW(x)		
#elif defined(XEON)
#  define PREFETCHW(x)		
#else
#  define PREFETCHW(x)		
#endif

  //debugging functions
#ifdef DEBUG
#  define DPRINT(args...) fprintf(stderr,args);
#  define DDPRINT(fmt, args...) printf("%s:%s:%d: "fmt, __FILE__, __FUNCTION__, __LINE__, args)
#else
#  define DPRINT(...)
#  define DDPRINT(fmt, ...)
#endif



  static inline int get_cluster(int thread_id) {
#ifdef __solaris__
    if (thread_id>64){
      perror("Thread id too high");
      return 0;
    }
    return thread_id/CORES_PER_SOCKET;
    //    return the_sockets[thread_id];
#elif XEON
    if (thread_id>=80){
      perror("Thread id too high");
      return 0;
    }

    return the_sockets[thread_id];
#elif defined(__tile__)
    return 0;
#else
    return thread_id/CORES_PER_SOCKET;
#endif
  }

  static inline double wtime(void)
  {
    struct timeval t;
    gettimeofday(&t,NULL);
    return (double)t.tv_sec + ((double)t.tv_usec)/1000000.0;
  }

  static inline 
  void set_cpu(int cpu) 
  {
#ifdef __sparc__
    processor_bind(P_LWPID,P_MYID, cpu, NULL);
#elif defined(__tile__)
    if (cpu>=tmc_cpus_grid_total()) {
      perror("Thread id too high");
    }
    // cput_set_t cpus;
    if (tmc_cpus_set_my_cpu(cpu)<0) {
      tmc_task_die("tmc_cpus_set_my_cpu() failed."); 
    }    
#else
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
#if defined(PLATFORM_NUMA)
    numa_set_preferred(get_cluster(cpu));
#endif
    pthread_t thread = pthread_self();
    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &mask) != 0) 
      {
	fprintf(stderr, "Error setting thread affinity\n");
      }
#endif
  }


  static inline void cdelay(ticks cycles){
    ticks __ts_end = getticks() + (ticks) cycles;
    while (getticks() < __ts_end);
  }

  static inline void cpause(ticks cycles){
#if defined(XEON)
    cycles >>= 3;
    ticks i;
    for (i=0;i<cycles;i++) {
      _mm_pause();
    }
#else
    ticks i;
    for (i=0;i<cycles;i++) {
      __asm__ __volatile__("nop");
    }
#endif
  }

  static inline void udelay(unsigned int micros)
  {
    double __ts_end = wtime() + ((double) micros / 1000000);
    while (wtime() < __ts_end);
  }

  //getticks needs to have a correction because the call itself takes a
  //significant number of cycles and skewes the measurement
  extern inline ticks getticks_correction_calc();

  static inline ticks get_noop_duration() {
#define NOOP_CALC_REPS 1000000
    ticks noop_dur = 0;
    uint32_t i;
    ticks corr = getticks_correction_calc();
    ticks start;
    ticks end;
    start = getticks();
    for (i=0;i<NOOP_CALC_REPS;i++) {
      __asm__ __volatile__("nop");
    }
    end = getticks();
    noop_dur = (ticks)((end-start-corr)/(double)NOOP_CALC_REPS);
    return noop_dur;
  }

  /// Round up to next higher power of 2 (return x if it's already a power
  /// of 2) for 32-bit numbers
  static inline uint32_t pow2roundup (uint32_t x){
    if (x==0) return 1;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
  }

#ifdef __cplusplus
}

#endif


#endif
