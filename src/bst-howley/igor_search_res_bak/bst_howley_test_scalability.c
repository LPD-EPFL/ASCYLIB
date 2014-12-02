/*   
 *   File: bst_howley_test_scalability.c
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>,
 *   Description: 
 *   bst_howley_test_scalability.c is part of ASCYLIB
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

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>


#include "bst_howley.h"
#include "measurements.h"
#include "utils.h"
#include "ssalloc.h"

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

//not used any more; TODO remove this parameter
#define DEFAULT_SEED 0

//default percentage of reads
#define DEFAULT_READS 80
#define DEFAULT_UPDATES 20

//default number of threads
#define DEFAULT_NUM_THREADS 1

//default experiment duration in miliseconds
#define DEFAULT_DURATION 1000

//the maximum value the key stored in the bst can take; defines the key range
#define DEFAULT_RANGE 2048

//#define DEBUG 1

int duration;
int num_threads;
uint32_t finds;
uint32_t updates;
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
    pthread_mutex_t *init_lock;
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
    //uint32_t write_thresh = 256 * (finds + inserts) / 100;
    //place the thread on the apropriate cpu
    set_cpu(the_cores[d->id]);
    //initialize the custom memeory allocator for this thread (we do not use malloc due to concurrency bottleneck issues)
    ssalloc_init();
    // ssalloc_align();
    bst_init_local(d->id);
    //for fine-grain latency measurements, we need to get the lenght of a getticks() function call, which is also counted
    //by default when we do getticks(); //code... getticks(); PF_START and PF_STOP use this when fine grain measurements are enabled
    PF_CORRECTION;
    uint32_t rand_max;
    //seed the custom random number generator
    seeds = seed_rand();
    rand_max = max_key;
    uint32_t op;
    skey_t key;
    int i;
    int last = -1;

    DDPRINT("staring initial insert\n",NULL);
    DDPRINT("number of inserts: %u up to %u\n",d->num_add,rand_max);
    //before starting the test, we insert a number of elements in the data structure
    //we do this at each thread to avoid the situation where the entire data structure 
    //resides in the same memory node

    // int num_elem = 0;
    // if (num_threads == 1){
    //     num_elem = max_key/2;
    // } else{
    //     num_elem = max_key/4;
    // }
    // pthread_mutex_lock(d->init_lock);
    // fprintf(stderr, "Starting critical section %d\n", d->id);
    for (i=0;i<max_key/4;++i) {
        key = my_random(&seeds[0],&seeds[1],&seeds[2]) & rand_max;
 
        DDPRINT("key is %u\n",key);
        //we make sure the insert was effective (as opposed to just updating an existing entry)
        if (d->id < 2) {
        if (bst_add(key,root, d->id)!=TRUE) {
            i--;
        }   }
    }

    // fprintf(stderr, "Exiting critical section %d\n", d->id);
    // pthread_mutex_unlock(d->init_lock);
    DDPRINT("added initial data\n",NULL);

    bool_t res;
    /* Init of local data if necessary */
    ticks t1,t2;
    /* Wait on barrier */
    // fprintf(stderr, "Waiting on barrier; thread %d\n", d->id);
    barrier_cross(d->barrier);
    //start the test
    while (*running) {
        //generate a key (node that rand_max is expected to be a power of 2)
        key = my_random(&seeds[0],&seeds[1],&seeds[2]) & rand_max;
        //generate the operation
        op = my_random(&seeds[0],&seeds[1],&seeds[2]) & 0xff;
        if (op < read_thresh) {
            //do a find operation
            //PF_START and PF_STOP can be used to do latency measurements of the operation
            //to enable them, DO_TIMINGS must be defined at compile time, otherwise they do nothing
            //PF_START(2);
            bst_contains(key,root, d->id);
            //PF_STOP(2);
        } else if (last == -1) {
            //do a write operation
            if (bst_add(key,root, d->id)) {
                d->num_insert++;
                last=1;
            }
        } else {
            //do a delete operation
            if (bst_remove(key,root, d->id)) {
                d->num_remove++;
                last=-1;
            }
        }
        d->num_operations++;
        //memory barrier to ensure no unwanted reporderings are happening
        //MEM_BARRIER;
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
    pthread_mutex_t init_lock;
    struct timeval start, end;
    struct timespec timeout;

    thread_data_t *data;
    sigset_t block_set;

    //initially, set parameters to their default values
    num_threads = DEFAULT_NUM_THREADS;
    seed=DEFAULT_SEED;
    max_key=DEFAULT_RANGE;
    updates=DEFAULT_UPDATES;
    finds=DEFAULT_READS;
    //inserts=DEFAULT_INSERTS;
    //removes=DEFAULT_REMOVES;
    duration=DEFAULT_DURATION;

    //now read the parameters in case the user provided values for them 
    //we use getopt, the same skeleton may be used for other bechmarks,
    //though the particular parameters may be different
    struct option long_options[] = {
        // These options don't set a flag
        {"help",                      no_argument,       NULL, 'h'},
        {"duration",                  required_argument, NULL, 'd'},
        {"range",                     required_argument, NULL, 'r'},
        {"initial",                     required_argument, NULL, 'i'},
        {"num-threads",               required_argument, NULL, 'n'},
        {"updates",             required_argument, NULL, 'u'},
        {"seed",                      required_argument, NULL, 's'},
        {NULL, 0, NULL, 0}
    };

    int i,c;

    //actually get the parameters form the command-line
    while(1) {
        i = 0;
        c = getopt_long(argc, argv, "hd:n:l:u:i:r:s", long_options, &i);

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
                        "  -u, --updates <int>\n"
                        "        Percentage of update operations (default=" XSTR(DEFAULT_UPDATES) ")\n"
                        "  -r, --range <int>\n"
                        "        Key range (default=" XSTR(DEFAULT_RANGE) ")\n"
                        "  -n, --num-threads <int>\n"
                        "        Number of threads (default=" XSTR(DEFAULT_NUM_THREADS) ")\n"
                        "  -s, --seed <int>\n"
                        "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
                      );
                exit(0);
            case 'd':
                duration = atoi(optarg);
                break;
            case 'u':
                updates = atoi(optarg);
                finds = 100 - updates;
                break;
            case 'r':
                max_key = atoi(optarg);
                break;
            case 'i':
                break;
            case 'l':
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

    max_key--;
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
    pthread_mutex_init(&init_lock, NULL);
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
        if (i< ((max_key/2)%num_threads)) data[i].num_add++;
        data[i].seed = rand();
        data[i].barrier = &barrier;
        data[i].init_lock = &init_lock;
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

    // seeds = seed_rand();
    // skey_t key;
    // for (i=0;i<max_key/2;++i) {
    //     key = my_random(&seeds[0],&seeds[1],&seeds[2]) & max_key;
    //     //we make sure the insert was effective (as opposed to just updating an existing entry)
    //     if (bst_add(key, root, 0)!=TRUE) {
    //         i--;
    //     }
    // }

    // bst_print(root);


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
    long reported_total = 1; //the tree contains two initial dummy nodes, INF1 and INF2
    //report some experiment statistics
    for (i = 0; i < num_threads; i++) {
        printf("Thread %d\n", i);
        printf("  #operations   : %lu\n", data[i].num_operations);
        printf("  #inserts   : %lu\n", data[i].num_insert);
        printf("  #removes   : %lu\n", data[i].num_remove);
        operations += data[i].num_operations;
        total_ticks += data[i].total_time;
        reported_total = reported_total + data[i].num_add + data[i].num_insert - data[i].num_remove;
    }

    printf("Duration      : %d (ms)\n", duration);
    printf("#txs     : %lu (%f / s)\n", operations, operations * 1000.0 / duration);
    //printf("Operation latency %lu\n", total_ticks / operations);
    //make sure the tree is correct
    printf("Expected size: %ld Actual size: %lu\n",reported_total,bst_size(root));

    free(threads);
    free(data);

    return 0;

}

