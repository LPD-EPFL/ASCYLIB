#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>
#include <malloc.h>
#include "utils.h"
#include "atomic_ops.h"
#include "rapl_read.h"
#ifdef __sparc__
#  include <sys/types.h>
#  include <sys/processor.h>
#  include <sys/procset.h>
#endif

#include "intset.h"

/* ################################################################### *
 * Definition of macros: per data structure
 * ################################################################### */

#define DS_CONTAINS(s,k)    sl_contains(s, k)
#define DS_ADD(s,k,v)       sl_add(s, k, v)
#define DS_REMOVE(s,k)      sl_remove(s, k)
#define DS_SIZE(s)          sl_set_size(s)
#define DS_NEW()            sl_set_new()

#define DS_TYPE             sl_intset_t
#define DS_NODE             sl_node_t

/* ################################################################### *
 * GLOBALS
 * ################################################################### */

RETRY_STATS_VARS_GLOBAL;

size_t initial = DEFAULT_INITIAL;
size_t range = DEFAULT_RANGE;
size_t update = DEFAULT_UPDATE;
size_t load_factor;
size_t num_threads = DEFAULT_NB_THREADS;
size_t duration = DEFAULT_DURATION;

size_t print_vals_num = 100;
size_t pf_vals_num = 1023;
size_t put, put_explicit = false;
double update_rate, put_rate, get_rate;

size_t size_after = 0;
int seed = 0;
__thread unsigned long * seeds;
uint32_t rand_max;
#define rand_min 1

static volatile int stop;
TEST_VARS_GLOBAL
;

volatile ticks *putting_succ;
volatile ticks *putting_fail;
volatile ticks *getting_succ;
volatile ticks *getting_fail;
volatile ticks *removing_succ;
volatile ticks *removing_fail;
volatile ticks *putting_count;
volatile ticks *putting_count_succ;
volatile ticks *getting_count;
volatile ticks *getting_count_succ;
volatile ticks *removing_count;
volatile ticks *removing_count_succ;
volatile ticks *total;

/* ################################################################### *
 * LOCALS
 * ################################################################### */

#ifdef DEBUG
extern __thread uint32_t put_num_restarts;
extern __thread uint32_t put_num_failed_expand;
extern __thread uint32_t put_num_failed_on_new;
#endif

barrier_t barrier, barrier_global;

typedef struct thread_data {
    uint32_t id;
    DS_TYPE* set;
} thread_data_t;

void*
test(void* thread) {
    thread_data_t* td = (thread_data_t*) thread;
    uint32_t ID = td->id;
    int phys_id = the_cores[ID];
    set_cpu(phys_id);
    ssalloc_init();

    DS_TYPE* set = td->set;

    PF_INIT(3, SSPFD_NUM_ENTRIES, ID);

#if defined(COMPUTE_LATENCY)
    volatile ticks my_putting_succ = 0;
    volatile ticks my_putting_fail = 0;
    volatile ticks my_getting_succ = 0;
    volatile ticks my_getting_fail = 0;
    volatile ticks my_removing_succ = 0;
    volatile ticks my_removing_fail = 0;
#endif
    uint64_t my_putting_count = 0;
    uint64_t my_getting_count = 0;
    uint64_t my_removing_count = 0;

    uint64_t my_putting_count_succ = 0;
    uint64_t my_getting_count_succ = 0;
    uint64_t my_removing_count_succ = 0;

#if defined(COMPUTE_LATENCY) && PFD_TYPE == 0
    volatile ticks start_acq, end_acq;
    volatile ticks correction = getticks_correction_calc();
#endif

    seeds = seed_rand();
#if GC == 1
    alloc = (ssmem_allocator_t*) malloc(sizeof(ssmem_allocator_t));
    assert(alloc != NULL);
    ssmem_alloc_init_fs_size(alloc, SSMEM_DEFAULT_MEM_SIZE, SSMEM_GC_FREE_SET_SIZE, ID);
#endif

    RR_INIT(phys_id);
    barrier_cross(&barrier);

    uint64_t key;
    strkey_t strkey;
    strval_t strval = { "val" };

    int c = 0;
    uint32_t scale_rem = (uint32_t)(update_rate * UINT_MAX);
    uint32_t scale_put = (uint32_t)(put_rate * UINT_MAX);

    int i;
    uint32_t num_elems_thread = (uint32_t)(initial / num_threads);
    int32_t missing = (uint32_t) initial - (num_elems_thread * num_threads);
    if (ID < missing) {
        num_elems_thread++;
    }

#if INITIALIZE_FROM_ONE == 1
    num_elems_thread = (ID == 0) * initial;
#endif

    for (i = 0; i < num_elems_thread; i++) {
        key =
                (my_random(&(seeds[0]), &(seeds[1]), &(seeds[2]))
                        % (rand_max + 1)) + rand_min;
        snprintf(strkey.key, STRING_LENGTH, "%lu", key);

        if (DS_ADD(set, strkey, strval) == false) {
            i--;
        }
    } MEM_BARRIER;

    RETRY_STATS_ZERO();

    barrier_cross(&barrier);

    if (!ID) {
        printf("#BEFORE size is: %zu\n", (size_t) DS_SIZE(set));
    }

    barrier_cross(&barrier_global);

    RR_START_SIMPLE();

    while (stop == 0) {
        c = (uint32_t)(my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])));
        key = (c & rand_max) + rand_min;
        snprintf(strkey.key, STRING_LENGTH, "%lu", key);

        if (unlikely(c <= scale_put)) {
            int res;
            START_TS(1);
            res = DS_ADD(set, strkey, strval);
            if (res) {
                END_TS(1, my_putting_count_succ); ADD_DUR(my_putting_succ);
                my_putting_count_succ++;
            } END_TS_ELSE(4, my_putting_count - my_putting_count_succ,
                    my_putting_fail);
            my_putting_count++;
        } else if (unlikely(c <= scale_rem)) {
            strval_t removed;
            START_TS(2);
            removed = DS_REMOVE(set, strkey);
            //if(removed != 0)
            if (strcmp(removed.val, "") != 0) {
                END_TS(2, my_removing_count_succ); ADD_DUR(my_removing_succ);
                my_removing_count_succ++;
            } END_TS_ELSE(5, my_removing_count - my_removing_count_succ,
                    my_removing_fail);
            my_removing_count++;
        } else {
            strval_t res;
            START_TS(0);
            res = DS_CONTAINS(set, strkey);
            //if(res != 0)
            if (strcmp(res.val, "") != 0) {
                END_TS(0, my_getting_count_succ); ADD_DUR(my_getting_succ);
                my_getting_count_succ++;
            } END_TS_ELSE(3, my_getting_count - my_getting_count_succ,
                    my_getting_fail);
            my_getting_count++;
        }
    }

    barrier_cross(&barrier);
    RR_STOP_SIMPLE();

    if (!ID) {
        size_after = DS_SIZE(set);
        printf("#AFTER  size is: %zu\n", size_after);
    }

    barrier_cross(&barrier);

#if defined(COMPUTE_LATENCY)
    putting_succ[ID] += my_putting_succ;
    putting_fail[ID] += my_putting_fail;
    getting_succ[ID] += my_getting_succ;
    getting_fail[ID] += my_getting_fail;
    removing_succ[ID] += my_removing_succ;
    removing_fail[ID] += my_removing_fail;
#endif
    putting_count[ID] += my_putting_count;
    getting_count[ID] += my_getting_count;
    removing_count[ID] += my_removing_count;

    putting_count_succ[ID] += my_putting_count_succ;
    getting_count_succ[ID] += my_getting_count_succ;
    removing_count_succ[ID] += my_removing_count_succ;

    EXEC_IN_DEC_ID_ORDER(ID, num_threads)
                {
                    print_latency_stats(ID, SSPFD_NUM_ENTRIES, print_vals_num);
                    RETRY_STATS_SHARE();
                }EXEC_IN_DEC_ID_ORDER_END(&barrier);

    SSPFDTERM();
#if GC == 1
    ssmem_term();
    free(alloc);
#endif

    pthread_exit(NULL);
}

int merge_operator1(strkey_t key, strval_t existing_val, strval_t val, strval_t *new_val){
    strval_t update = {"lol"};
    *new_val = update;
    return true;
}

int merge_operator2(strkey_t key, strval_t existing_val, strval_t val, strval_t *new_val){
    
    char res[STRING_LENGTH];
    strncpy(res, existing_val.val, STRING_LENGTH);
    strcat(res, val.val);

    strncpy((*new_val).val, res, STRING_LENGTH);
    return true;
}

int main(int argc, char **argv) {

    // strkey_t min;
    // strncpy(min.key, STR_KEY_MIN, 10);

    // strkey_t max;
    // strncpy(max.key, STR_KEY_MAX, 10);

    // strkey_t norm;
    // strncpy(norm.key, "trool", 10);

    // printf("%d\n", strkey_compare( min,  min));
    // printf("%d\n", strkey_compare( min,  max));
    // printf("%d\n", strkey_compare( max,  max));
    // printf("%d\n", strkey_compare( max,  min));

    // printf("%d\n", strkey_compare( min,  norm));
    // printf("%d\n", strkey_compare( max,  norm));

    set_cpu(the_cores[0]);
    ssalloc_init();

    #if GC == 1
    alloc = (ssmem_allocator_t*) malloc(sizeof(ssmem_allocator_t));
    assert(alloc != NULL);
    ssmem_alloc_init_fs_size(alloc, SSMEM_DEFAULT_MEM_SIZE, SSMEM_GC_FREE_SET_SIZE, 0);
    #endif

    seeds = seed_rand();


    rand_max = range - 1;

    // levelmax = floor_log_2((unsigned int) initial);
    levelmax = 10;
    size_pad_32 = sizeof(sl_node_t) + (levelmax * sizeof(sl_node_t *));
    while (size_pad_32 & 31) {
        size_pad_32++;
    }

    DS_TYPE* set = DS_NEW();
    assert(set != NULL);

    strkey_t a = {"a"}, b = {"b"}, c = {"c"};
    strval_t x = {"x"}, y = {"y"}, z = {"z"};

    sl_add(set, a, x);
    sl_add(set, b, y);
    sl_add(set, c, z);

    strval_t ret;
    ret = sl_contains(set, a);
    printf("a -> %s\n", ret.val);
    ret = sl_contains(set, b);
    printf("b -> %s\n", ret.val);
    ret = sl_contains(set, c);
    printf("c -> %s\n", ret.val);

    sl_merge(set, b, x, &merge_operator1);

    ret = sl_contains(set, a);
    printf("a -> %s\n", ret.val);
    ret = sl_contains(set, b);
    printf("b -> %s\n", ret.val);
    ret = sl_contains(set, c);
    printf("c -> %s\n", ret.val);


    sl_merge(set, c, z, &merge_operator2);
    sl_merge(set, b, z, &merge_operator2);


    ret = sl_contains(set, a);
    printf("a -> %s\n", ret.val);
    ret = sl_contains(set, b);
    printf("b -> %s\n", ret.val);
    ret = sl_contains(set, c);
    printf("c -> %s\n", ret.val);

    #if GC == 1
    ssmem_term();
    free(alloc);
    #endif

    pthread_exit(NULL);
    return 0;
}
