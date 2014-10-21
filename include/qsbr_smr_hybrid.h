#ifndef __QSBR_SMR_HYBRID_H
#define __QSBR_SMR_HYBRID_H

#include "mr.h"
#include "lock_if.h"

// QSBR STUFF
#define FUZZY 0
#define NOT_FUZZY 1

#define N_EPOCHS 3
#define QUIESCENCE_THRESHOLD 100

// How many milliseconds should the sleeper threads sleep
#define SLEEP_AMOUNT 150
#define MARGIN 50

// SMR STUFF

/* Parameters to the algorithm:
 *  K: Number of hazard pointers per CPU.
 *  H: Number of hazard pointers required.
 *  R: Chosen such that R = H + Omega(H).
 */
#define K 2
#define H (K * ltd.nthreads)
#define R (100 + 2*H)

typedef ALIGNED(CACHE_LINE_SIZE) struct fallback_flag {
  volatile uint8_t flag;
  char padding[CACHE_LINE_SIZE - sizeof(uint8_t)];
} fallback_flag_t;

fallback_flag_t fallback;

struct hazard_pointer {
    void *p;
    char padding[CACHE_LINE_SIZE - sizeof(void *)];
};

typedef ALIGNED(CACHE_LINE_SIZE) struct hazard_pointer hazard_pointer_t;

/* Must be dynamically initialized to be an array of size H. */
hazard_pointer_t *HP;
 
struct qsbr_globals {
    ptlock_t update_lock ALIGNED(CACHE_LINE_SIZE);
    int global_epoch ALIGNED(CACHE_LINE_SIZE);
    // OANA IGOR Should we pad here? It's not clear how big the padding should be...
};

struct shared_thread_data {
    /* EBR per-thread data:
     *  limbo_list: three lists of nodes awaiting physical deletion, one
     *              for each epoch
     *  in_critical: flag telling us whether we're in a critical section
     *               with respect to memory reclamation
     *  epoch: the local epoch
     */
    mr_node_t *limbo_list [N_EPOCHS];
    int epoch;
    int in_critical;
    char padding[CACHE_LINE_SIZE - 2 * sizeof(int) - N_EPOCHS * sizeof(mr_node_t*)];
};

struct local_thread_data {
  // mr_node_t *rlist;
  mr_node_t **plist;
  uint64_t rcount;
  uint64_t nthreads;
  uint64_t thread_index;
  char padding[CACHE_LINE_SIZE - 3*sizeof(uint64_t) - sizeof(mr_node_t **)];
};

typedef ALIGNED(CACHE_LINE_SIZE) struct shared_thread_data shared_thread_data_t;
typedef ALIGNED(CACHE_LINE_SIZE) struct local_thread_data local_thread_data_t; 

void scan();

void quiescent_state (int flag);

void free_node_later(void *);

#endif