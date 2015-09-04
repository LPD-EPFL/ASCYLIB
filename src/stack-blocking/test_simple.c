#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>

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

#include "bl-stack.h"

/* ################################################################### *
 * Definition of macros: per data structure
 * ################################################################### */

#define DS_ADD(q,v,t)       push(q,v)
#define DS_REMOVE(q,t)      pop(q)
#define DS_SIZE(s)          stack_size(s)
#define DS_NEW(n)           create_stack(n)

#define DS_TYPE             stack_t
#define DS_NODE             qnode_t


/* ################################################################### *
 * GLOBALS
 * ################################################################### */

#if TSX_STATS == 1
__thread uint64_t locked = 0;
#endif

RETRY_STATS_VARS_GLOBAL;

size_t initial = DEFAULT_INITIAL;
size_t range = DEFAULT_RANGE; 
size_t update = 100;
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
TEST_VARS_GLOBAL;

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


#if WAIT_LOCK_STATS == 1
__thread ticks getticks_correction = 0;
ticks getticks_correction_calc() 
{
#    define getticks_calc_reps 1000000
  ticks t_dur = 0;
  uint32_t i;
  for (i = 0; i < getticks_calc_reps; i++) {
    ticks t_start = getticks();
    ticks t_end = getticks();
    t_dur += t_end - t_start;
  }
  getticks_correction = (ticks)(t_dur / (double) getticks_calc_reps);
  /* printf("(cor: %llu)", (unsigned long long int) getticks_correction); */
  return getticks_correction;
}

__thread volatile ticks wait_lock;
__thread ticks gt_correction;
#endif

#if SLOW_CORE == 1
__thread uint32_t slow_thread;
#endif
/* ################################################################### *
 * LOCALS
 * ################################################################### */

#ifdef DEBUG
extern __thread uint32_t put_num_restarts;
extern __thread uint32_t put_num_failed_expand;
extern __thread uint32_t put_num_failed_on_new;
#endif

barrier_t barrier, barrier_global;

typedef struct thread_data
{
    int32_t id;
    DS_TYPE* set;
} thread_data_t;

    void*
test(void* thread) 
{
    thread_data_t* td = (thread_data_t*) thread;
    int32_t ID = td->id;
#if SLOW_CORE == 1
  if (td->id == 0) {
    slow_thread = 1;
  } else {
    slow_thread = 0;
  }
#endif
    int phys_id = the_cores[ID];
    set_cpu(phys_id);
    ssalloc_init();

    DS_TYPE* set = td->set;

    PF_INIT(3, SSPFD_NUM_ENTRIES, ID);

#if WAIT_LOCK_STATS == 1
  wait_lock = 0;
  gt_correction = getticks_correction_calc();
#endif
#if defined(COMPUTE_LATENCY)
    volatile ticks my_putting_succ = 0;
    volatile ticks my_putting_fail = 0;
    volatile ticks my_removing_succ = 0;
    volatile ticks my_removing_fail = 0;
#endif
    uint64_t my_putting_count = 0;
    uint64_t my_removing_count = 0;

    uint64_t my_putting_count_succ = 0;
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

    uint32_t scale_rem = (uint32_t) (update_rate * UINT_MAX);
    uint32_t scale_put = (uint32_t) (put_rate * UINT_MAX);

    int i;
    uint64_t c = 0;  
    uint32_t num_elems_thread = (uint32_t) (initial / num_threads);
    int32_t missing = (uint32_t) initial - (num_elems_thread * num_threads);
    if (ID < missing)
    {
        num_elems_thread++;
    }

#if INITIALIZE_FROM_ONE == 1
    num_elems_thread = (ID == 0) * initial;
#endif    

    qvalue_t* val = (qvalue_t*) malloc (sizeof(qvalue_t));
    *val = ID;
    for(i = 0; i < num_elems_thread; i++) 
    {
        //key = (my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) % (rand_max + 1)) + rand_min;
        //TODO use allocator
        //TODO right now I am using the same value each time I insert; I could do a new allocation for each insertions
        /*c = (uint64_t)(my_random(&(seeds[0]),&(seeds[1]),&(seeds[2])));	*/
        /*qvalue_t* val = (qvalue_t*) malloc (sizeof(qvalue_t));*/
        /**val = c;*/

        DS_ADD(set,val,ID);
    }
    MEM_BARRIER;

    barrier_cross(&barrier);

    if (!ID)
    {
        printf("#BEFORE size is: %zu\n", (size_t) DS_SIZE(set));
    }

    RETRY_STATS_ZERO();

    barrier_cross(&barrier_global);

    RR_START_SIMPLE();
    uint64_t do_remove = ID%2;

    while (stop == 0) 
    {
        if ( do_remove ){
            START_TS(2);
            qvalue_t* popped = DS_REMOVE(set, ID);
            END_TS(2, my_removing count_succ);
            if (popped!=NULL) {
                ADD_DUR(my_removing_succ);
                my_removing_count_succ++;
            }
#if GC == 1
            //ssmem_free(alloc,popped);
#endif
            do_remove = 0;  
            /*fprintf(stderr, "thread %d after remove: stack size is %d\n",ID,stack_size(set));*/
            /*print_stack(set);*/
        } else {
            /*c = (uint64_t)(my_random(&(seeds[0]),&(seeds[1]),&(seeds[2])));	*/
            /*//TODO: use custom allocator*/
            /*qvalue_t* val = (qvalue_t*) malloc (sizeof(qvalue_t));*/
            /**val = c;*/
            START_TS(1);
            DS_ADD(set, val, ID);
            END_TS(1, my_putting_count_succ);
            ADD_DUR(my_putting_succ);
            my_putting_count_succ++;
            do_remove = 1;
            /*fprintf(stderr, "thread %d after insert: stack size is %d\n",ID,stack_size(set));*/
            /*print_stack(set);*/
        }
        /*if (my_putting_count_succ % 1000 == 0) {*/
            /*fprintf(stderr, "thread %d: stack size is %d\n",ID,stack_size(set));*/
        /*}*/
    }

    barrier_cross(&barrier);
    RR_STOP_SIMPLE();

    if (!ID)
    {
        size_after = DS_SIZE(set);
        printf("#AFTER  size is: %zu\n", size_after);
    }

    barrier_cross(&barrier);

#if defined(COMPUTE_LATENCY)
    putting_succ[ID] += my_putting_succ;
    putting_fail[ID] += my_putting_fail;
    removing_succ[ID] += my_removing_succ;
    removing_fail[ID] += my_removing_fail;
#endif
    putting_count[ID] += my_putting_count;
    removing_count[ID]+= my_removing_count;

    putting_count_succ[ID] += my_putting_count_succ;
    removing_count_succ[ID]+= my_removing_count_succ;

    EXEC_IN_DEC_ID_ORDER(ID, num_threads)
    {
        print_latency_stats(ID, SSPFD_NUM_ENTRIES, print_vals_num);
        RETRY_STATS_SHARE();
    }
    EXEC_IN_DEC_ID_ORDER_END(&barrier);

    SSPFDTERM();

#if WAIT_LOCK_STATS == 1
    printf("Thread %d cycles waiting for locks: %llu\n", ID, (long long unsigned int) wait_lock);
#endif
#if GC == 1
    ssmem_term();
    free(alloc);
#endif

    pthread_exit(NULL);
}

    int
main(int argc, char **argv) 
{
    set_cpu(the_cores[0]);
    ssalloc_init();
    seeds = seed_rand();

    struct option long_options[] = {
        // These options don't set a flag
        {"help",                      no_argument,       NULL, 'h'},
        {"duration",                  required_argument, NULL, 'd'},
        {"initial-size",              required_argument, NULL, 'i'},
        {"num-threads",               required_argument, NULL, 'n'},
        {"range",                     required_argument, NULL, 'r'},
        {"update-rate",               required_argument, NULL, 'u'},
        {"num-buckets",               required_argument, NULL, 'b'},
        {"print-vals",                required_argument, NULL, 'v'},
        {"vals-pf",                   required_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };

    fprintf(stderr, "Data strucutre sizes\n");
    fprintf(stderr, "sizeof qnode_t %d\n", sizeof(qnode_t));
    fprintf(stderr, "sizeof stack_t %d\n", sizeof(stack_t));

    int i, c;
    while(1) 
    {
        i = 0;
        c = getopt_long(argc, argv, "hAf:d:i:n:r:s:u:m:a:p:b:v:f:", long_options, &i);

        if(c == -1)
            break;

        if(c == 0 && long_options[i].flag == 0)
            c = long_options[i].val;

        switch(c) 
        {
            case 0:
                /* Flag is automatically set */
                break;
            case 'h':
                printf("intset -- STM stress test "
                        "(linked list)\n"
                        "\n"
                        "Usage:\n"
                        "  intset [options...]\n"
                        "\n"
                        "Options:\n"
                        "  -h, --help\n"
                        "        Print this message\n"
                        "  -d, --duration <int>\n"
                        "        Test duration in milliseconds\n"
                        "  -i, --initial-size <int>\n"
                        "        Number of elements to insert before test\n"
                        "  -n, --num-threads <int>\n"
                        "        Number of threads\n"
                        "  -r, --range <int>\n"
                        "        Range of integer values inserted in set\n"
                        "  -u, --update-rate <int>\n"
                        "        Percentage of update transactions\n"
                        "  -p, --put-rate <int>\n"
                        "        Percentage of put update transactions (should be less than percentage of updates)\n"
                        "  -b, --num-buckets <int>\n"
                        "        Number of initial buckets (stronger than -l)\n"
                        "  -v, --print-vals <int>\n"
                        "        When using detailed profiling, how many values to print.\n"
                        "  -f, --val-pf <int>\n"
                        "        When using detailed profiling, how many values to keep track of.\n"
                        );
                exit(0);
            case 'd':
                duration = atoi(optarg);
                break;
            case 'i':
                initial = atoi(optarg);
                break;
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 'r':
                range = atol(optarg);
                break;
            case 'u':
                update = atoi(optarg);
                break;
            case 'p':
                put_explicit = 1;
                put = atoi(optarg);
                break;
            case 'v':
                print_vals_num = atoi(optarg);
                break;
            case 'f':
                pf_vals_num = pow2roundup(atoi(optarg)) - 1;
                break;
            case '?':
            default:
                printf("Use -h or --help for help\n");
                exit(1);
        }
    }


    if (!is_power_of_two(initial))
    {
        size_t initial_pow2 = pow2roundup(initial);
        printf("** rounding up initial (to make it power of 2): old: %zu / new: %zu\n", initial, initial_pow2);
        initial = initial_pow2;
    }

    if (range < initial)
    {
        range = 2 * initial;
    }

    printf("## Initial: %zu / Range: %zu /\n", initial, range);

    double kb = initial * sizeof(DS_NODE) / 1024.0;
    double mb = kb / 1024.0;
    printf("Sizeof initial: %.2f KB = %.2f MB\n", kb, mb);

    if (!is_power_of_two(range))
    {
        size_t range_pow2 = pow2roundup(range);
        printf("** rounding up range (to make it power of 2): old: %zu / new: %zu\n", range, range_pow2);
        range = range_pow2;
    }
#if GC == 1
    alloc = (ssmem_allocator_t*) malloc(sizeof(ssmem_allocator_t));
    assert(alloc != NULL);
    ssmem_alloc_init_fs_size(alloc, SSMEM_DEFAULT_MEM_SIZE, SSMEM_GC_FREE_SET_SIZE, num_threads);
#endif

    if (put > update)
    {
        put = update;
    }

    update_rate = update / 100.0;

    if (put_explicit)
    {
        put_rate = put / 100.0;
    }
    else
    {
        put_rate = update_rate / 2;
    }



    get_rate = 1 - update_rate;

    /* printf("num_threads = %u\n", num_threads); */
    /* printf("cap: = %u\n", num_buckets); */
    /* printf("num elem = %u\n", num_elements); */
    /* printf("filing rate= %f\n", filling_rate); */
    /* printf("update = %f (putting = %f)\n", update_rate, put_rate); */


    rand_max = range - 1;

    struct timeval start, end;
    struct timespec timeout;
    timeout.tv_sec = duration / 1000;
    timeout.tv_nsec = (duration % 1000) * 1000000;

    stop = 0;

    DS_TYPE* set = DS_NEW(num_threads);
    assert(set != NULL);

    /* Initializes the local data */
    putting_succ = (ticks *) calloc(num_threads , sizeof(ticks));
    putting_fail = (ticks *) calloc(num_threads , sizeof(ticks));
    getting_succ = (ticks *) calloc(num_threads , sizeof(ticks));
    getting_fail = (ticks *) calloc(num_threads , sizeof(ticks));
    removing_succ = (ticks *) calloc(num_threads , sizeof(ticks));
    removing_fail = (ticks *) calloc(num_threads , sizeof(ticks));
    putting_count = (ticks *) calloc(num_threads , sizeof(ticks));
    putting_count_succ = (ticks *) calloc(num_threads , sizeof(ticks));
    getting_count = (ticks *) calloc(num_threads , sizeof(ticks));
    getting_count_succ = (ticks *) calloc(num_threads , sizeof(ticks));
    removing_count = (ticks *) calloc(num_threads , sizeof(ticks));
    removing_count_succ = (ticks *) calloc(num_threads , sizeof(ticks));

    pthread_t threads[num_threads];
    pthread_attr_t attr;
    int rc;
    void *status;

    barrier_init(&barrier_global, num_threads + 1);
    barrier_init(&barrier, num_threads);

    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    thread_data_t* tds = (thread_data_t*) malloc(num_threads * sizeof(thread_data_t));

    long t;
    for(t = 0; t < num_threads; t++)
    {
        tds[t].id = t;
        tds[t].set = set;
        rc = pthread_create(&threads[t], &attr, test, tds + t);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }

    }

    /* Free attribute and wait for the other threads */
    pthread_attr_destroy(&attr);

    barrier_cross(&barrier_global);
    gettimeofday(&start, NULL);
    nanosleep(&timeout, NULL);

    stop = 1;
    gettimeofday(&end, NULL);
    duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);

    for(t = 0; t < num_threads; t++) 
    {
        rc = pthread_join(threads[t], &status);
        if (rc) 
        {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    free(tds);

    volatile ticks putting_suc_total = 0;
    volatile ticks putting_fal_total = 0;
    volatile ticks getting_suc_total = 0;
    volatile ticks getting_fal_total = 0;
    volatile ticks removing_suc_total = 0;
    volatile ticks removing_fal_total = 0;
    volatile uint64_t putting_count_total = 0;
    volatile uint64_t putting_count_total_succ = 0;
    volatile uint64_t getting_count_total = 0;
    volatile uint64_t getting_count_total_succ = 0;
    volatile uint64_t removing_count_total = 0;
    volatile uint64_t removing_count_total_succ = 0;

    for(t=0; t < num_threads; t++) 
    {
        putting_suc_total += putting_succ[t];
        putting_fal_total += putting_fail[t];
        getting_suc_total += getting_succ[t];
        getting_fal_total += getting_fail[t];
        removing_suc_total += removing_succ[t];
        removing_fal_total += removing_fail[t];
        putting_count_total += putting_count[t];
        putting_count_total_succ += putting_count_succ[t];
        getting_count_total += getting_count[t];
        getting_count_total_succ += getting_count_succ[t];
        removing_count_total += removing_count[t];
        removing_count_total_succ += removing_count_succ[t];
    }

#if defined(COMPUTE_LATENCY)
    printf("#thread srch_suc srch_fal insr_suc insr_fal remv_suc remv_fal   ## latency (in cycles) \n"); fflush(stdout);
    long unsigned get_suc = (getting_count_total_succ) ? getting_suc_total / getting_count_total_succ : 0;
    long unsigned get_fal = (getting_count_total - getting_count_total_succ) ? getting_fal_total / (getting_count_total - getting_count_total_succ) : 0;
    long unsigned put_suc = putting_count_total_succ ? putting_suc_total / putting_count_total_succ : 0;
    long unsigned put_fal = (putting_count_total - putting_count_total_succ) ? putting_fal_total / (putting_count_total - putting_count_total_succ) : 0;
    long unsigned rem_suc = removing_count_total_succ ? removing_suc_total / removing_count_total_succ : 0;
    long unsigned rem_fal = (removing_count_total - removing_count_total_succ) ? removing_fal_total / (removing_count_total - removing_count_total_succ) : 0;
    printf("%-7zu %-8lu %-8lu %-8lu %-8lu %-8lu %-8lu\n", num_threads, get_suc, get_fal, put_suc, put_fal, rem_suc, rem_fal);
#endif

#define LLU long long unsigned int

    int UNUSED pr = (int) (putting_count_total_succ - removing_count_total_succ);
    if (size_after != (initial + pr))
    /*if (size_after != (pr))*/
    {
        printf("// WRONG size. %zu + %d != %zu\n", initial, pr, size_after);
        assert(size_after == (initial + pr));
    }

    printf("    : %-10s | %-10s | %-11s | %-11s | %s\n", "total", "success", "succ %", "total %", "effective %");
    uint64_t total = putting_count_total + getting_count_total + removing_count_total;
    double putting_perc = 100.0 * (1 - ((double)(total - putting_count_total) / total));
    double putting_perc_succ = (1 - (double) (putting_count_total - putting_count_total_succ) / putting_count_total) * 100;
    double getting_perc = 100.0 * (1 - ((double)(total - getting_count_total) / total));
    double getting_perc_succ = (1 - (double) (getting_count_total - getting_count_total_succ) / getting_count_total) * 100;
    double removing_perc = 100.0 * (1 - ((double)(total - removing_count_total) / total));
    double removing_perc_succ = (1 - (double) (removing_count_total - removing_count_total_succ) / removing_count_total) * 100;
    printf("srch: %-10llu | %-10llu | %10.1f%% | %10.1f%% | \n", (LLU) getting_count_total, 
            (LLU) getting_count_total_succ,  getting_perc_succ, getting_perc);
    printf("insr: %-10llu | %-10llu | %10.1f%% | %10.1f%% | %10.1f%%\n", (LLU) putting_count_total, 
            (LLU) putting_count_total_succ, putting_perc_succ, putting_perc, (putting_perc * putting_perc_succ) / 100);
    printf("rems: %-10llu | %-10llu | %10.1f%% | %10.1f%% | %10.1f%%\n", (LLU) removing_count_total, 
            (LLU) removing_count_total_succ, removing_perc_succ, removing_perc, (removing_perc * removing_perc_succ) / 100);

    double throughput = (putting_count_total + getting_count_total + removing_count_total) * 1000.0 / duration;
    printf("#txs %zu\t(%-10.0f\n", num_threads, throughput);
    printf("#Mops %.3f\n", throughput / 1e6);

    RR_PRINT_UNPROTECTED(RAPL_PRINT_POW);
    RR_PRINT_CORRECTED();    
    RETRY_STATS_PRINT(total, putting_count_total, removing_count_total, putting_count_total_succ + removing_count_total_succ);    

    pthread_exit(NULL);

    return 0;
}
