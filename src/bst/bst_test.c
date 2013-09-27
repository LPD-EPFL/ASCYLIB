#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>


#include "bst_ellen.h"
#include "measurements.h"
#include "utils.h"
#include "ssalloc.h"

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

//not used any more; TODO remove this parameter
#define DEFAULT_SEED 0

//default percentage of reads
#define DEFAULT_READS 90
//default percentage of inserts
#define DEFAULT_INSERTS 5
//default percentage of remove operations
#define DEFAULT_REMOVES 5

//default number of threads
#define DEFAULT_NUM_THREADS 4

//default experiment duration in miliseconds
#define DEFAULT_DURATION 10000

//the maximum value the key stored in the bst can take; defines the key range
#define DEFAULT_MAX_KEY 255

//#define DEBUG 1

int duration;
int num_threads;
uint32_t finds;
uint32_t removes;
uint32_t inserts;
uint32_t max_key;
int seed;

//static volatile int stop;

//used to signal the threads when to stop
ALIGNED(64) uint8_t running[64];

//per-thread seeds for the custom random function
__thread unsigned long * seeds;

//the root of the binary search tree
node_t * root;


//a simple barrier implementation
//used to make sure all threads start the experiment at the same time
typedef struct barrier {
    pthread_cond_t complete;
    pthread_mutex_t mutex;
    int count;
    int crossing;
} barrier_t;

void barrier_init(barrier_t *b, int n)
{
    pthread_cond_init(&b->complete, NULL);
    pthread_mutex_init(&b->mutex, NULL);
    b->count = n;
    b->crossing = 0;
}

void barrier_cross(barrier_t *b)
{
    pthread_mutex_lock(&b->mutex);
    /* One more thread through */
    b->crossing++;
    /* If not all here, wait */
    if (b->crossing < b->count) {
        pthread_cond_wait(&b->complete, &b->mutex);
    } else {
        pthread_cond_broadcast(&b->complete);
        /* Reset for next time */
        b->crossing = 0;
    }
    pthread_mutex_unlock(&b->mutex);
}

//data structure through which we send parameters to and get results from the worker threads
typedef ALIGNED(64) struct thread_data {
    //pointer to the global barrier
    barrier_t *barrier;
    //counts the number of operations each thread performs
    unsigned long num_operations;
    //total operation time (not used here)
    ticks total_time;
    //seed (not used here)
    unsigned int seed;
    //the number of elements each thread should add at the beginning of its execution
    uint64_t num_add;
    //number of inserts a thread performs
    unsigned long num_insert;
    //number of removes a thread performs
    unsigned long num_remove;
    //number of searches a thread performs
    unsigned long num_search;
    //the id of the thread (used for thread placement on cores)
    int id;
} thread_data_t;

void *test(void *data)
{
    DDPRINT("starting test\n",NULL);
    //get the per-thread data
    thread_data_t *d = (thread_data_t *)data;
    //scale percentages of the various operations to the range 0..255
    //this saves us a floating point operation during the benchmark
    //e.g instead of random()%100 to determine the next operation we will do, we can simply do random()&256
    //this saves time on some platfroms
    uint32_t read_thresh = 256 * finds / 100;
    uint32_t write_thresh = 256 * (finds + inserts) / 100;
    //place the thread on the apropriate cpu
    set_cpu(the_cores[d->id]);
    //initialize the custom memeory allocator for this thread (we do not use malloc due to concurrency bottleneck issues)
    ssalloc_init();
    //for fine-grain latency measurements, we need to get the lenght of a getticks() function call, which is also counted
    //by default when we do getticks(); //code... getticks(); PF_START and PF_STOP use this when fine grain measurements are enabled
    PF_CORRECTION;
    uint32_t rand_max;
    //seed the custom random number generator
    seeds = seed_rand();
    rand_max = max_key;
    uint32_t op;
    key_t key;
    int i;

    DDPRINT("staring initial insert\n",NULL);
    DDPRINT("number of inserts: %u up to %u\n",d->num_add,rand_max);
    //before starting the test, we insert a number of elements in the data structure
    //we do this at each thread to avoid the situation where the entire data structure 
    //resides in the same memory node
    for (i=0;i<d->num_add;++i) {
        key = my_random(&seeds[0],&seeds[1],&seeds[2]) & rand_max;
        DDPRINT("key is %u\n",key);
        //we make sure the insert was effective (as opposed to just updating an existing entry)
        if (bst_insert(key,root,d->id)!=TRUE) {
            i--;
        }
    }
    DDPRINT("added initial data\n",NULL);

    bool_t res;
    /* Init of local data if necessary */
    ticks t1,t2;
    /* Wait on barrier */
    barrier_cross(d->barrier);
    //start the test
    while (*running) {
        //generate a key (node that rand_max is expected to be a power of 2)
        key = my_random(&seeds[0],&seeds[1],&seeds[2]) & rand_max;
        //generate the operation
        op = my_random(&seeds[0],&seeds[1],&seeds[2]) & 0xff;
        DDPRINT("key is %lu\n",key);
        if (op < read_thresh) {
            //do a find operation
            //PF_START and PF_STOP can be used to do latency measurements of the operation
            //to enable them, DO_TIMINGS must be defined at compile time, otherwise they do nothing
            DDPRINT("starting find\n",NULL);
            PF_START(2);
            bst_find(key,root,d->id);
            PF_STOP(2);
            DDPRINT("finished find\n",NULL);
        } else if (op < write_thresh) {
            //do a write operation
            DDPRINT("starting insert\n",NULL);
            if (bst_insert(key,root, d->id)) {
                d->num_insert++;
            }
            DDPRINT("finished insert\n",NULL);
        } else {
            DDPRINT("starting remove\n",NULL);
            //do a delete operation
            if (bst_delete(key,root, d->id)) {
                d->num_remove++;
            }
            DDPRINT("finished remove\n",NULL);
        }
        d->num_operations++;
        //memory barrier to ensure no unwanted reporderings are happening
        MEM_BARRIER;
    }
    //summary of the fine grain measurements if enabled 
    PF_PRINT;
    return NULL;
}

void catcher(int sig)
{
    static int nb = 0;
    printf("CAUGHT SIGNAL %d\n", sig);
    if (++nb >= 3)
        exit(1);
}

int main(int argc, char* const argv[]) {
    //place thread on the first cpu
    set_cpu(the_cores[0]);
    //initialize the custom memory allocator
    ssalloc_init();
    pthread_t *threads;
    pthread_attr_t attr;
    barrier_t barrier;
    struct timeval start, end;
    struct timespec timeout;

    thread_data_t *data;
    sigset_t block_set;

    //initially, set parameters to their default values
    num_threads = DEFAULT_NUM_THREADS;
    seed=DEFAULT_SEED;
    max_key=DEFAULT_MAX_KEY;
    finds=DEFAULT_READS;
    inserts=DEFAULT_INSERTS;
    removes=DEFAULT_REMOVES;
    duration=DEFAULT_DURATION;

    //now read the parameters in case the user provided values for them 
    //we use getopt, the same skeleton may be used for other bechmarks,
    //though the particular parameters may be different
    struct option long_options[] = {
        // These options don't set a flag
        {"help",                      no_argument,       NULL, 'h'},
        {"duration",                  required_argument, NULL, 'd'},
        {"range",                     required_argument, NULL, 'r'},
        {"num-threads",               required_argument, NULL, 'n'},
        {"finds",             required_argument, NULL, 'f'},
        {"seed",                      required_argument, NULL, 's'},
        {NULL, 0, NULL, 0}
    };

    int i,c;

    //actually get the parameters form the command-line
    while(1) {
        i = 0;
        c = getopt_long(argc, argv, "hd:n:f:r:s", long_options, &i);

        if(c == -1)
            break;

        if(c == 0 && long_options[i].flag == 0)
            c = long_options[i].val;

        switch(c) {
            case 0:
                /* Flag is automatically set */
                break;
            case 'h':
                printf("lock stress test\n"
                        "\n"
                        "Usage:\n"
                        "  stress_test [options...]\n"
                        "\n"
                        "Options:\n"
                        "  -h, --help\n"
                        "        Print this message\n"
                        "  -d, --duration <int>\n"
                        "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
                        "  -f, --finds <int>\n"
                        "        Percentage of find operations (default=" XSTR(DEFAULT_READS) ")\n"
                        "  -r, --range <int>\n"
                        "        Key range (default=" XSTR(DEFAULT_MAX_KEY) ")\n"
                        "  -n, --num-threads <int>\n"
                        "        Number of threads (default=" XSTR(DEFAULT_NUM_THREADS) ")\n"
                        "  -s, --seed <int>\n"
                        "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
                      );
                exit(0);
            case 'd':
                duration = atoi(optarg);
                break;
            case 'f':
                finds = atoi(optarg);
                inserts = (100 - finds)/2;
                removes = (100 - finds)/2;
                break;
            case 'r':
                max_key = atoi(optarg);
                break;
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case '?':
                printf("Use -h or --help for help\n");
                exit(0);
            default:
                exit(1);
        }
    }
    //aligmenent in the custom memory allocator to a 64 byte boundary 
    ssalloc_align();
    //we round the max key up to the nearest power of 2, which makes our random key generation more efficient
    max_key = pow2roundup(max_key)-1;

    //initialization of the tree
    root = bst_initialize(num_threads);

    //initialize the data which will be passed to the threads
    if ((data = (thread_data_t *)malloc(num_threads * sizeof(thread_data_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    if ((threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    if (seed == 0)
        srand((int)time(NULL));
    else
        srand(seed);

    //flag signaling the threads until when to run
    *running = 1;

    //global barrier initialization (used to start the threads at the same time)
    barrier_init(&barrier, num_threads + 1);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    timeout.tv_sec = duration / 1000;
    timeout.tv_nsec = (duration % 1000) * 1000000;
    

    //set the data for each thread and create the threads
    for (i = 0; i < num_threads; i++) {
        data[i].id = i;
        data[i].num_operations = 0;
        data[i].total_time=0;
        data[i].num_insert=0;
        data[i].num_remove=0;
        data[i].num_search=0;
        data[i].num_add = max_key/(2 * num_threads); 
        data[i].seed = rand();
        data[i].barrier = &barrier;
        if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }
    pthread_attr_destroy(&attr);

    /* Catch some signals */
    if (signal(SIGHUP, catcher) == SIG_ERR ||
            signal(SIGINT, catcher) == SIG_ERR ||
            signal(SIGTERM, catcher) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    /* Start threads */
    barrier_cross(&barrier);
    gettimeofday(&start, NULL);
    if (duration > 0) {
        //sleep for the duration of the experiment
        nanosleep(&timeout, NULL);
    } else {
        sigemptyset(&block_set);
        sigsuspend(&block_set);
    }

    //signal the threads to stop
    *running = 0;
    gettimeofday(&end, NULL);

    /* Wait for thread completion */
    for (i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error waiting for thread completion\n");
            exit(1);
        }
    }
    DDPRINT("threads finshed\n",NULL);
    //compute the exact duration of the experiment
    duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
    
    //bst_print(root);

    unsigned long operations = 0;
    ticks total_ticks = 0;
    long reported_total = 2; //the tree contains two initial dummy nodes, INF1 and INF2
    //report some experiment statistics
    for (i = 0; i < num_threads; i++) {
        printf("Thread %d\n", i);
        printf("  #operations   : %lu\n", data[i].num_operations);
        operations += data[i].num_operations;
        total_ticks += data[i].total_time;
        reported_total = reported_total + data[i].num_add + data[i].num_insert - data[i].num_remove;
    }

    printf("Duration      : %d (ms)\n", duration);
    printf("#operations     : %lu (%f / s)\n", operations, operations * 1000.0 / duration);
    //printf("Operation latency %lu\n", total_ticks / operations);
    //make sure the tree is correct
    printf("Expected size: %ld Actual size: %lu\n",reported_total,bst_size(root));

    free(threads);
    free(data);

    return 0;

}

