#ifndef _MEASUREMENTS_H_
#define _MEASUREMENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* #define DO_TIMINGS */

#ifdef SSMP
#  include <ssmp.h>
#endif

#include "getticks.h"

#ifndef REF_SPEED_GHZ
#  if defined(PLATFORM_MCORE) | defined(XEON) | defined(OPTERON)
#    define REF_SPEED_GHZ           2.1
#  elif defined(HASWELL)
#    define REF_SPEED_GHZ           3
#  elif defined(LPDXEON)
#    define REF_SPEED_GHZ           2.8
#  elif defined(SCC)
#    define REF_SPEED_GHZ           0.533
#  elif defined(MAGLITE)
#    define REF_SPEED_GHZ           1.2
#  elif defined(T44)
#    define REF_SPEED_GHZ           2.998
#  elif defined(__tilepro__)
#    define REF_SPEED_GHZ           0.7
#  elif defined(__tilegx__)
#    define REF_SPEED_GHZ           1.2
#  elif defined(LAPTOP) | defined(IGORLAPTOPLINUX) | defined(OANALAPTOPLINUX)
#    define REF_SPEED_GHZ           2.4
#  elif defined(LPDPC4)
#    define REF_SPEED_GHZ           3.5
#  else
#    define REF_SPEED_GHZ          FREQ_GHZ
#  endif
#endif  /* REF_SPEED_GHZ */

  /* 
#DO_TIMINGS_TICKS
#DO_TIMINGS_TICKS_SIMPLE
#DO_TIMINGS_STD
 */
#define DO_TIMINGS_TICKS_SIMPLE

#ifndef DO_TIMINGS
#  undef DO_TIMINGS_TICKS
#  undef DO_TIMINGS_TICKS_SIMPLE
#  undef DO_TIMINGS_STD
#  define PF_MSG(pos, msg)        
#  define PF_START(pos)           
#  define PF_STOP(pos)            
#  define PF_KILL(pos)            
#  define PF_PRINT_TICKS          
#  define PF_PRINT                
#  define PF_EXCLUDE(pos)    
#  define PF_CORRECTION 
#else
#  define PF_MSG(pos, msg)        SET_PROF_MSG_POS(pos, msg)
#  define PF_START(pos)           ENTRY_TIME_POS(pos)
#  define PF_STOP(pos)            EXIT_TIME_POS(pos)
#  define PF_KILL(pos)            KILL_ENTRY_TIME_POS(pos)
#  define PF_PRINT_TICKS          REPORT_TIMINGS
#  define PF_PRINT                REPORT_TIMINGS_SECS
#  define PF_EXCLUDE(pos)         EXCLUDE_ENTRY(pos)
#  define PF_CORRECTION           MEASUREREMENT_CORRECTION
#endif

#ifdef DO_TIMINGS
#  ifndef DO_TIMINGS_STD
#    ifndef DO_TIMINGS_TICKS
#      ifndef DO_TIMINGS_TICKS_SIMPLE
#        error Define either DO_TIMINGS_STD or DO_TIMINGS_TICKS
#      endif
#    endif
#  endif
#endif

#include <stdio.h>
#include <math.h>

  // =============================== GETTIMEOFDAY ================================ {{{
#ifdef DO_TIMINGS_STD
#  define ENTRY_TIMES_SIZE 8
  EXINLINED struct timeval entry_time[ENTRY_TIMES_SIZE];
  EXINLINED bool entry_time_valid[ENTRY_TIMES_SIZE];
  EXINLINED long long total_sum_sec[ENTRY_TIMES_SIZE];
  EXINLINED long long total_sum_usec[ENTRY_TIMES_SIZE];
  EXINLINED long long total_samples[ENTRY_TIMES_SIZE];

#  define ENTRY_TIME ENTRY_TIME_POS(0)
#  define EXIT_TIME EXIT_TIME_POS(0)
#  define KILL_ENTRY_TIME KILL_ENTRY_TIME_POS(0)

#  define ENTRY_TIME_POS(position)		\
  do {						\
    gettimeofday(&entry_time[position], NULL);	\
    entry_time_valid[position] = true;		\
  } while (0);

#  define EXIT_TIME_POS(position)					\
  do {									\
    if (entry_time_valid[position]) {					\
      entry_time_valid[position] = false;				\
      struct timeval exit_time;						\
      gettimeofday(&exit_time, NULL);					\
      total_sum_sec[position] += (exit_time.tv_sec - entry_time[position].tv_sec); \
      total_sum_usec[position] += (exit_time.tv_usec - entry_time[position].tv_usec); \
      while (total_sum_usec[position] > 1000000) {			\
	total_sum_usec[position] -= 1000000;				\
	total_sum_sec[position] ++;					\
      };  total_samples[position]++; }					\
  } while (0);
#  define KILL_ENTRY_TIME_POS(position) do {	\
    entry_time_valid[position] = false;		\
  } while (0);

#  ifndef _MEASUREMENTS_ID_
#    define _MEASUREMENTS_ID_ -1
#  endif

#  define REPORT_TIMINGS REPORT_TIMINGS_RANGE(0,ENTRY_TIMES_SIZE)

#  define REPORT_TIMINGS_RANGE(start,end) do {				\
    for (int i=start;i<end;i++) {					\
      if (total_samples[i]) {						\
	fprintf(stderr, "TIMINGS[%d][%d]: total samples: %lld, total time: %lld + %lld, average time usec: %g\n", \
		_MEASUREMENTS_ID_, i, total_samples[i], total_sum_sec[i], total_sum_usec[i], 1.0*(total_sum_sec[i]*1000000.0 + total_sum_usec[i])/total_samples[i]); \
      }}} while(0);


  // }}} 
  // ================================== TICKS ================================== {{{
#elif defined DO_TIMINGS_TICKS
  
#  include <stdint.h>

#  define ENTRY_TIMES_SIZE 16

  extern void prints_ticks_stats(int start, int end);

  enum timings_bool_t 
    {
      M_FALSE, M_TRUE
    };

  extern __thread ticks entry_time[ENTRY_TIMES_SIZE];
  extern __thread enum timings_bool_t entry_time_valid[ENTRY_TIMES_SIZE];
  extern __thread ticks total_sum_ticks[ENTRY_TIMES_SIZE];
  extern __thread long long total_samples[ENTRY_TIMES_SIZE];
  extern __thread const char *measurement_msgs[ENTRY_TIMES_SIZE];
  extern __thread ticks getticks_correction;
  extern ticks getticks_correction_calc();

#  define MEASUREREMENT_CORRECTION getticks_correction_calc();
#  define SET_PROF_MSG(msg) SET_PROF_MSG_POS(0, msg) 
#  define ENTRY_TIME ENTRY_TIME_POS(0)
#  define EXIT_TIME EXIT_TIME_POS(0)
#  define KILL_ENTRY_TIME KILL_ENTRY_TIME_POS(0)

#  define SET_PROF_MSG_POS(pos, msg)		\
  measurement_msgs[pos] = msg;

#  define ENTRY_TIME_POS(position)		\
  do {						\
    entry_time[position] = getticks();		\
    entry_time_valid[position] = M_TRUE;	\
  } while (0);

#  define EXIT_TIME_POS(position)					\
  do {									\
    ticks exit_time = getticks();					\
    if (entry_time_valid[position]) {					\
      entry_time_valid[position] = M_FALSE;				\
      total_sum_ticks[position] += (exit_time - entry_time[position] - getticks_correction); \
      total_samples[position]++;					\
    }} while (0);

#  define KILL_ENTRY_TIME_POS(position)		\
  do {						\
    entry_time_valid[position] = M_FALSE;	\
  } while (0);

#  define EXCLUDE_ENTRY(position)		\
  do {						\
    total_samples[position] = 0;		\
  } while(0);

#  ifndef _MEASUREMENTS_ID_
#    define _MEASUREMENTS_ID_ -1
#  endif

#  define REPORT_TIMINGS REPORT_TIMINGS_RANGE(0,ENTRY_TIMES_SIZE)

#  define REPORT_TIMINGS_RANGE(start,end)				\
  do {                                                                  \
    int i;                                                              \
    for (i = start; i < end; i++) {					\
      if (total_samples[i]) {						\
	printf("[%02d]%s:\n", i, measurement_msgs[i]);			\
	printf("  samples: %-16llu| ticks: %-16llu| avg ticks: %-16llu\n", \
	       total_samples[i], total_sum_ticks[i], total_sum_ticks[i] / total_samples[i]); \
      }									\
    }                                                                   \
  }									\
  while (0);

#  define REPORT_TIMINGS_SECS REPORT_TIMINGS_SECS_RANGE(0, ENTRY_TIMES_SIZE)

#  define REPORT_TIMINGS_SECS_RANGE_(start,end)				\
  do {                                                                  \
    int i;                                                              \
    for (i = start; i < end; i++) {					\
      if (total_samples[i]) {						\
	printf("[%02d]%s:\n", i, measurement_msgs[i]);			\
	printf("  samples: %-16llu | secs: %-4.10f | avg ticks: %-4.10f\n", \
	       total_samples[i], total_sum_ticks[i] / (REF_SPEED_GHZ * 1.e9), \
	       (total_sum_ticks[i] / total_samples[i])/ (REF_SPEED_GHZ * 1.e9)); \
      }									\
    }                                                                   \
  }									\
  while (0);

#  define REPORT_TIMINGS_SECS_RANGE(start,end)	\
  prints_ticks_stats(start, end);


  // }}} 
  // ================================== TICKS SIMPLE version ================================== {{{
#elif defined(DO_TIMINGS_TICKS_SIMPLE)
  
#  include <stdint.h>

#  define ENTRY_TIMES_SIZE 16

  extern void prints_ticks_stats(int start, int end);

  enum timings_bool_t 
    {
      M_FALSE, M_TRUE
    };

  extern __thread ticks entry_time[ENTRY_TIMES_SIZE];
  extern __thread ticks total_sum_ticks[ENTRY_TIMES_SIZE];
  extern __thread long long total_samples[ENTRY_TIMES_SIZE];
  extern __thread const char *measurement_msgs[ENTRY_TIMES_SIZE];
  extern __thread ticks getticks_correction;
  extern ticks getticks_correction_calc();

#  define MEASUREREMENT_CORRECTION getticks_correction_calc();
#  define SET_PROF_MSG(msg) SET_PROF_MSG_POS(0, msg) 
#  define ENTRY_TIME ENTRY_TIME_POS(0)
#  define EXIT_TIME EXIT_TIME_POS(0)
#  define KILL_ENTRY_TIME KILL_ENTRY_TIME_POS(0)

#  define SET_PROF_MSG_POS(pos, msg)		\
  measurement_msgs[pos] = msg;

#  define ENTRY_TIME_POS(position)		\
  do {						\
    entry_time[position] = getticks();		\
  } while (0);

#  define EXIT_TIME_POS(position)					\
  do {									\
    total_sum_ticks[position] +=					\
      (getticks() - entry_time[position] - getticks_correction);	\
    total_samples[position]++;						\
  } while (0);

#  define EXCLUDE_ENTRY(position)		\
  do {						\
    total_samples[position] = 0;		\
  } while(0);

#  ifndef _MEASUREMENTS_ID_
#    define _MEASUREMENTS_ID_ -1
#  endif

#  define REPORT_TIMINGS REPORT_TIMINGS_RANGE(0,ENTRY_TIMES_SIZE)

#  define REPORT_TIMINGS_RANGE(start,end)				\
  do {                                                                  \
    int i;                                                              \
    for (i = start; i < end; i++) {					\
      if (total_samples[i]) {						\
	printf("[%02d]%s:\n", i, measurement_msgs[i]);			\
	printf("  samples: %-16llu| ticks: %-16llu| avg ticks: %-16llu\n", \
	       total_samples[i], total_sum_ticks[i], total_sum_ticks[i] / total_samples[i]); \
      }									\
    }                                                                   \
  }									\
  while (0);

#  define REPORT_TIMINGS_SECS REPORT_TIMINGS_SECS_RANGE(0, ENTRY_TIMES_SIZE)

#  define REPORT_TIMINGS_SECS_RANGE_(start,end)				\
  do {                                                                  \
    int i;                                                              \
    for (i = start; i < end; i++) {					\
      if (total_samples[i]) {						\
	printf("[%02d]%s:\n", i, measurement_msgs[i]);			\
	printf("  samples: %-16llu | secs: %-4.10f | avg ticks: %-4.10f\n", \
	       total_samples[i], total_sum_ticks[i] / (REF_SPEED_GHZ * 1.e9), \
	       (total_sum_ticks[i] / total_samples[i])/ (REF_SPEED_GHZ * 1.e9)); \
      }									\
    }                                                                   \
  }									\
  while (0);

#  define REPORT_TIMINGS_SECS_RANGE(start,end)	\
  prints_ticks_stats(start, end);


  // }}}
  // ================================== OTHER ================================== {{{
#else
#  define ENTRY_TIME
#  define EXIT_TIME
#  define KILL_ENTRY_TIME
#  define REPORT_TIMINGS
#  define REPORT_TIMINGS_RANGE(x,y)
#  define ENTRY_TIME_POS(X)
#  define EXIT_TIME_POS(X)
#  define KILL_ENTRY_TIME_POS(X)
#endif

#ifdef	__cplusplus
}
#endif

#endif
