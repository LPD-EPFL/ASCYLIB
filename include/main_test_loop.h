#ifndef _MAIN_TEST_LOOP_H_
#define _MAIN_TEST_LOOP_H_
#define TEST_LOOP(algo_type)						\
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
      res = DS_CONTAINS(set, key, algo_type);				\
      if(res != 0)							\
	{								\
	  END_TS(0, my_getting_count_succ);				\
	  ADD_DUR(my_getting_succ);					\
	  my_getting_count_succ++;					\
	}								\
      END_TS_ELSE(3, my_getting_count - my_getting_count_succ,\
		  my_getting_fail);					\
      my_getting_count++;						\
    }


#define TEST_LOOP_NA()						\
  c = (uint32_t)(my_random(&(seeds[0]),&(seeds[1]),&(seeds[2])));	\
  key = (c & rand_max) + rand_min;					\
									\
  if (unlikely(c <= scale_put))						\
    {									\
      int res;								\
      START_TS(1);							\
      res = DS_ADD(set, key);				\
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
      removed = DS_REMOVE(set, key);				\
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
      res = DS_CONTAINS(set, key);				\
      if(res != 0)							\
	{								\
	  END_TS(0, my_getting_count_succ);				\
	  ADD_DUR(my_getting_succ);					\
	  my_getting_count_succ++;					\
	}								\
      END_TS_ELSE(3, my_getting_count - my_getting_count_succ,\
		  my_getting_fail);					\
      my_getting_count++;						\
    }


#if RAPL_READ_ENABLE == 1
#  define RR_PRINT_CORRECTED()						\
  rapl_stats_t s;							\
  RR_STATS(&s);								\
  if (num_threads > (CORES_PER_SOCKET*NUMBER_OF_SOCKETS))		\
    {									\
      num_threads = (CORES_PER_SOCKET*NUMBER_OF_SOCKETS);		\
    }									\
  double pow_tot_correction = (throughput * eng_per_test_iter_nj[num_threads-1][0]) / 1e9; \
  double static_pow = 0;						\
  int si;								\
  for (si = 0; si < NUMBER_OF_SOCKETS; si++)				\
    {									\
      if (s.power_total[si] > 0)					\
	{								\
	  static_pow += static_power[si + 1];				\
	}								\
    }									\
  double pow_tot_corrected = s.power_total[NUMBER_OF_SOCKETS] - 0; \
  printf("#Total Power Corrected                     : %11f (correction= %10f) W\n",  pow_tot_corrected, pow_tot_correction + static_pow); \
  double eop = (1e6 * s.power_total[NUMBER_OF_SOCKETS]) / throughput;	\
  double eop_corrected = (1e6 * pow_tot_corrected) / throughput;	\
  printf("#Energy per Operation                      : %11f (corrected = %10f) uJ\n", eop, eop_corrected);

  /* double pow_tot_corrected = s.power_total[NUMBER_OF_SOCKETS] - pow_tot_correction - static_pow; \ */


#else
#  define RR_PRINT_CORRECTED()
#endif    



#endif	/* _MAIN_TEST_LOOP_H_ */
