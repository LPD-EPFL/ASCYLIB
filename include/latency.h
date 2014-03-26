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
#  define END_TS_ELSE(s, i, inc)
#  define ADD_DUR(tar)
#  define ADD_DUR_FAIL(tar)
#  define PF_INIT(s, e, id)
#elif PFD_TYPE == 0
#  define START_TS(s)				\
    asm volatile ("");				\
    start_acq = getticks();			\
    asm volatile ("lfence");
#  define END_TS(s, i)				\
    asm volatile ("");				\
    end_acq = getticks();			\
    asm volatile ("");
#  define END_TS_ELSE(s, i, inc)		\
  else						\
    {						\
      END_TS(s, i);				\
      ADD_DUR(inc);				\
    }
#  define ADD_DUR(tar) tar += (end_acq - start_acq - correction)
#  define ADD_DUR_FAIL(tar)					\
  else								\
    {								\
      ADD_DUR(tar);						\
    }
#  define PF_INIT(s, e, id)
#else  /* PFD_TYPE == 1 */

#  define PF_NUM_STORES 6
#  define SSPFD_NUM_ENTRIES  (pf_vals_num + 1)
#  define PF_INIT(s, e, id) SSPFDINIT(PF_NUM_STORES, e, id)

#  if LATENCY_ALL_CORES == 0
#    define START_TS(s)      SSPFDI_ID_G(0); asm volatile ("lfence");
#    define END_TS(s, i)     SSPFDO_ID_G(s, i & pf_vals_num, 0)
#    define END_TS_ELSE(s, i, inc)     else { SSPFDO_ID_G(s, (i) & pf_vals_num, 0); }

#    define ADD_DUR(tar) 
#    define ADD_DUR_FAIL(tar)
#  else
#    define START_TS(s)      SSPFDI_G(); asm volatile ("lfence");
#    define END_TS(s, i)     SSPFDO_G(s, i & pf_vals_num)
#    define END_TS_ELSE(s, i, inc)     else { SSPFDO_G(s, (i) & pf_vals_num); }

#    define ADD_DUR(tar) 
#    define ADD_DUR_FAIL(tar)
#  endif
#endif


static inline void
print_latency_stats(int ID, size_t num_entries, size_t num_entries_print)
{
#if (PFD_TYPE == 1) && defined(COMPUTE_LATENCY)
  if (ID == 0)
    {
      printf("get ------------------------------------------------------------------------\n");
      printf("#latency_get_suc: ");
      SSPFDPN_COMMA(0, num_entries, num_entries_print);
      printf("#latency_get_fal: ");
      SSPFDPN_COMMA(3, num_entries, num_entries_print);
      printf("put ------------------------------------------------------------------------\n");
      printf("#latency_put_suc: ");
      SSPFDPN_COMMA(1, num_entries, num_entries_print);
      printf("#latency_put_fal: ");
      SSPFDPN_COMMA(4, num_entries, num_entries_print);
      printf("rem ------------------------------------------------------------------------\n");
      printf("#latency_rem_suc: ");
      SSPFDPN_COMMA(2, num_entries, num_entries_print);
      printf("#latency_rem_fal: ");
      SSPFDPN_COMMA(5, num_entries, num_entries_print);
    }
#  if LATENCY_ALL_CORES == 1
  else
    {
      printf("#latency_get_suc: ");
      SSPFDPN_COMMA(0, num_entries, num_entries_print);
      printf("#latency_get_fal: ");
      SSPFDPN_COMMA(3, num_entries, num_entries_print);
      printf("#latency_put_suc: ");
      SSPFDPN_COMMA(1, num_entries, num_entries_print);
      printf("#latency_put_fal: ");
      SSPFDPN_COMMA(4, num_entries, num_entries_print);
      printf("#latency_rem_suc: ");
      SSPFDPN_COMMA(2, num_entries, num_entries_print);
      printf("#latency_rem_fal: ");
      SSPFDPN_COMMA(5, num_entries, num_entries_print);
    }
#  endif
#endif
}

#endif /* _LATENCY_H_ */
