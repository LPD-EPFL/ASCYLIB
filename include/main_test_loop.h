/*   
 *   File: main_test_loop.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   main_test_loop.h is part of ASCYLIB
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

#ifndef _MAIN_TEST_LOOP_H_
#define _MAIN_TEST_LOOP_H_

#define TEST_VARS_GLOBAL						\
  volatile int phase_put = 0;						\
  volatile uint32_t phase_put_threshold_start = 0.99999 * UINT_MAX;	\
  volatile uint32_t phase_put_threshold_stop  = 0.9999999 * UINT_MAX;  \
  __thread volatile ticks phase_start, phase_stop;			

#define UNIFORM_WORKLOAD 1

#if UNIFORM_WORKLOAD == 0
#  define TEST_LOOP(algo_type)						\
  c = (uint32_t)(my_random(&(seeds[0]),&(seeds[1]),&(seeds[2])));	\
  key = (c & rand_max) + rand_min;					\
  if (!phase_put && c > phase_put_threshold_start)			\
    {									\
      phase_start = getticks();						\
      phase_put = 1;							\
      MEM_BARRIER;							\
    }									\
  else if (phase_put && c > phase_put_threshold_stop)			\
    {									\
      phase_stop = getticks();						\
      if (!ID) printf("[%2u]phase dur = %f\n", ID, (phase_stop-phase_start)/2.8e9); \
      phase_put = 0;							\
    }									\
									\
  if (phase_put || unlikely(c <= scale_put))				\
    {									\
      int res;								\
      START_TS(1);							\
      res = DS_ADD(set, key, algo_type);				\
      if(res)								\
	{								\
	  END_TS(1, my_putting_count_succ);				\
	  ADD_DUR(my_putting_succ);					\
	  my_putting_count_succ++;					\
	}								\
      END_TS_ELSE(4, my_putting_count - my_putting_count_succ,		\
		  my_putting_fail);					\
      my_putting_count++;						\
    }									\
  else if(unlikely(c <= scale_rem))					\
    {									\
      int removed;							\
      START_TS(2);							\
      removed = DS_REMOVE(set, key, algo_type);				\
      if(removed != 0)							\
	{								\
	  END_TS(2, my_removing_count_succ);				\
	  ADD_DUR(my_removing_succ);					\
	  my_removing_count_succ++;					\
	}								\
      END_TS_ELSE(5, my_removing_count - my_removing_count_succ,	\
		  my_removing_fail);					\
      my_removing_count++;						\
    }									\
  else									\
    {									\
      int res;								\
      START_TS(0);							\
      res = (sval_t) DS_CONTAINS(set, key, algo_type);			\
      if(res != 0)							\
	{								\
	  END_TS(0, my_getting_count_succ);				\
	  ADD_DUR(my_getting_succ);					\
	  my_getting_count_succ++;					\
	}								\
      END_TS_ELSE(3, my_getting_count - my_getting_count_succ,		\
		  my_getting_fail);					\
      my_getting_count++;						\
    }

#else

#  define TEST_LOOP(algo_type)						\
  c = (uint32_t)(my_random(&(seeds[0]),&(seeds[1]),&(seeds[2])));	\
  key = (c & rand_max) + rand_min;					\
									\
  if (unlikely(c <= scale_put))						\
    {									\
      int res;								\
      START_TS(1);							\
      res = DS_ADD(set, key, algo_type);				\
      if(res)								\
	{								\
	  END_TS(1, my_putting_count_succ);				\
	  ADD_DUR(my_putting_succ);					\
	  my_putting_count_succ++;					\
	}								\
      END_TS_ELSE(4, my_putting_count - my_putting_count_succ,		\
		  my_putting_fail);					\
      my_putting_count++;						\
    }									\
  else if(unlikely(c <= scale_rem))					\
    {									\
      int removed;							\
      START_TS(2);							\
      removed = DS_REMOVE(set, key, algo_type);				\
      if(removed != 0)							\
	{								\
	  END_TS(2, my_removing_count_succ);				\
	  ADD_DUR(my_removing_succ);					\
	  my_removing_count_succ++;					\
	}								\
      END_TS_ELSE(5, my_removing_count - my_removing_count_succ,	\
		  my_removing_fail);					\
      my_removing_count++;						\
    }									\
  else									\
    {									\
      int res;								\
      START_TS(0);							\
      res = (sval_t) DS_CONTAINS(set, key, algo_type);			\
      if(res != 0)							\
	{								\
	  END_TS(0, my_getting_count_succ);				\
	  ADD_DUR(my_getting_succ);					\
	  my_getting_count_succ++;					\
	}								\
      END_TS_ELSE(3, my_getting_count - my_getting_count_succ,		\
		  my_getting_fail);					\
      my_getting_count++;						\
    }

#endif	/* UNIFORM_WORKLOAD */

#define POW_CORRECTED 0

//  double pow_tot_correction = (throughput * eng_per_test_iter_nj[num_threads-1][0]) / 1e9;
//   printf("#Duration: %f, %f, %f\n", s.duration[0], s.duration[1], s.duration[2]);


#if RAPL_READ_ENABLE == 1
#  if POW_CORRECTED == 1
#    define RR_PRINT_CORRECTED()					\
  rapl_stats_t s;							\
  RR_STATS(&s);								\
  if (num_threads > (CORES_PER_SOCKET*NUMBER_OF_SOCKETS))		\
    {									\
      num_threads = (CORES_PER_SOCKET*NUMBER_OF_SOCKETS);		\
    }									\
  double static_pow = 0;						\
  int si;								\
  for (si = 0; si < NUMBER_OF_SOCKETS; si++)				\
    {									\
      if (s.power_total[si] > 0)					\
	{								\
	  static_pow += static_power[si + 1];				\
	}								\
    }									\
  double pow_tot_corrected = s.power_total[NUMBER_OF_SOCKETS] - static_pow;	\
  printf("#Total Power Corrected                     : %11f (correction= %10f) W\n", \
	 pow_tot_corrected, static_pow);				\
  double eop = (1e6 * s.power_total[NUMBER_OF_SOCKETS]) / throughput;	\
  double eop_corrected = (1e6 * pow_tot_corrected) / throughput;	\
  printf("#Energy per Operation                      : %11f (corrected = %10f) uJ\n", eop, eop_corrected);
#  else	 /* not corrected */
#    define RR_PRINT_CORRECTED()					\
  rapl_stats_t s;							\
  RR_STATS(&s);								\
  if (num_threads > (CORES_PER_SOCKET*NUMBER_OF_SOCKETS))		\
    {									\
      num_threads = (CORES_PER_SOCKET*NUMBER_OF_SOCKETS);		\
    }									\
  double pow_tot_corrected = s.power_total[NUMBER_OF_SOCKETS];	\
  printf("#Total Power Corrected                     : %11f (correction= %10f) W\n",  pow_tot_corrected, 0.0); \
  double eop = (1e6 * s.power_total[NUMBER_OF_SOCKETS]) / throughput;	\
  double eop_corrected = eop;						\
  printf("#Energy per Operation                      : %11f (corrected = %10f) uJ\n", eop, eop_corrected);
#  endif
/* double pow_tot_corrected = s.power_total[NUMBER_OF_SOCKETS] - pow_tot_correction - static_pow; \ */


#else
#  define RR_PRINT_CORRECTED()
#endif    



#endif	/* _MAIN_TEST_LOOP_H_ */
