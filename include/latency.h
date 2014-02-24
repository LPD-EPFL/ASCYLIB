#ifndef _LATENCY_H_
#define _LATENCY_H_

#if defined(USE_SSPFD)
#  define PFD_TYPE 1
#  define SSPFD_DO_TIMINGS 1
#else
#  define PFD_TYPE 0
#  define SSPFD_NUM_ENTRIES 0
#  undef SSPFD_DO_TIMINGS
#  define SSPFD_DO_TIMINGS 0
#endif
#include "sspfd.h"

#if !defined(COMPUTE_LATENCY)
#  define START_TS(s)
#  define END_TS(s, i)
#  define ADD_DUR(tar)
#  define ADD_DUR_FAIL(tar)
#  define PF_INIT(s, e, id)
#elif PFD_TYPE == 0
#  define START_TS(s)				\
  {						\
    asm volatile ("");				\
    start_acq = getticks();			\
    asm volatile ("");
#  define END_TS(s, i)				\
    asm volatile ("");				\
    end_acq = getticks();			\
    asm volatile ("");				\
    }

#  define ADD_DUR(tar) tar += (end_acq - start_acq - correction)
#  define ADD_DUR_FAIL(tar)					\
  else								\
    {								\
      ADD_DUR(tar);						\
    }
#  define PF_INIT(s, e, id)
#else
#  define SSPFD_NUM_ENTRIES  (pf_vals_num + 1)
#  define START_TS(s)      SSPFDI(s)
#  define END_TS(s, i)     SSPFDO(s, i & pf_vals_num)

#  define ADD_DUR(tar) 
#  define ADD_DUR_FAIL(tar)
#  define PF_INIT(s, e, id) SSPFDINIT(s, e, id)
#endif


static inline void
print_latency_stats(int ID, size_t num_entries, size_t num_entries_print)
{
#if (PFD_TYPE == 1) && defined(COMPUTE_LATENCY)
  if (ID == 0)
    {
      printf("#latency_get: ");
      SSPFDPN_COMMA(0, num_entries, num_entries_print);
      printf("#latency_put: ");
      SSPFDPN_COMMA(1, num_entries, num_entries_print);
      printf("#latency_rem: ");
      SSPFDPN_COMMA(2, num_entries, num_entries_print);
    }
#  if LATENCY_PRINT_ALL_CORES == 1
  else
    {
      printf("#latency_get: ");
      SSPFDPRINTV_COMMA(0, num_entries, num_entries_print);
      printf("#latency_put: ");
      SSPFDPRINTV_COMMA(1, num_entries, num_entries_print);
      printf("#latency_rem: ");
      SSPFDPRINTV_COMMA(2, num_entries, num_entries_print);
    }
#  endif
#endif
}

#endif /* _LATENCY_H_ */
