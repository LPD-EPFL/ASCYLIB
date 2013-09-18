#include <immintrin.h>
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

#define XSTR(s) #s

#define DEFAULT_READS 90
#define DEFAULT_INSERTS 5
#define DEFAULT_REMOVES 5

#define DEFAULT_NUM_THREADS 4
#define DEFAULT_SEED 0
#define DEFAULT_DURATION 10000
#define DEFAULT_MAX_KEY 255

#define DEBUG 1

int duration;
int num_threads;
uint32_t finds;
uint32_t removes;
uint32_t inserts;
uint32_t max_key;
int seed;

ticks correction;
static volatile int stop;
__thread unsigned long * seeds;

//the root of the binary search tree
node_t * root;

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

typedef struct thread_data {
    barrier_t *barrier;
    unsigned long num_operations;
//unsigned long num_entries;
    ticks total_time;
//    int* lock;
    unsigned int seed;
    uint64_t num_add;
    int id;
} thread_data_t;

void *test(void *data)
{
    DDPRINT("starting test\n",NULL);
    thread_data_t *d = (thread_data_t *)data;
    uint32_t read_thresh = 256 * finds / 100;
    uint32_t write_thresh = 256 * (finds + inserts) / 100;
    set_cpu(the_cores[d->id]);
    ssalloc_init();
    uint32_t rand_max;
    seeds = seed_rand();
    rand_max = max_key;
    uint32_t op;
    key_t key;
    int i;

    DDPRINT("staring initial insert\n",NULL);
    DDPRINT("number of inserts: %u up to %u\n",d->num_add,rand_max);
    //bst_print(root);
    for (i=0;i<d->num_add;++i) {
        key = my_random(&seeds[0],&seeds[1],&seeds[2]) & rand_max;
        DDPRINT("key is %u\n",key);
        if (bst_insert(key,root,d->id)!=TRUE) {
            i--;
        }
        //bst_print(root);
    }
    DDPRINT("added initial data\n",NULL);

    bool_t res;
    /* Init of local data if necessary */
    ticks t1,t2;
    /* Wait on barrier */
    barrier_cross(d->barrier);
    while (stop == 0) {
        key = my_random(&seeds[0],&seeds[1],&seeds[2]) & rand_max;
        op = my_random(&seeds[0],&seeds[1],&seeds[2]) & 0xff;
        //t1=getticks();
        DDPRINT("key is %lu\n",key);
        if (op < read_thresh) {
            DDPRINT("starting find\n",NULL);
            bst_find(key,root,d->id);
            DDPRINT("finished find\n",NULL);
        } else if (op < write_thresh) {
            DDPRINT("starting insert\n",NULL);
            bst_insert(key,root, d->id);
            DDPRINT("finished insert\n",NULL);
        } else {
            DDPRINT("starting remove\n",NULL);
            bst_delete(key,root, d->id);
            DDPRINT("finished remove\n",NULL);
        }
        //t2=getticks();
        d->num_operations++;
        MEM_BARRIER;
        //d->total_time+=t2-t1-correction;
    }
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
    set_cpu(the_cores[0]);
    ssalloc_init();
    pthread_t *threads;
    pthread_attr_t attr;
    barrier_t barrier;
    struct timeval start, end;
    struct timespec timeout;
  #ifdef DO_TIMINGS
    correction = getticks_correction_calc();
#endif

    thread_data_t *data;
    sigset_t block_set;
    num_threads = DEFAULT_NUM_THREADS;
    seed=DEFAULT_SEED;
    max_key=DEFAULT_MAX_KEY;
    finds=DEFAULT_READS;
    inserts=DEFAULT_INSERTS;
    removes=DEFAULT_REMOVES;
    duration=DEFAULT_DURATION;
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

    while(1) {
        i = 0;
        c = getopt_long(argc, argv, "h:d:n:f:r:s", long_options, &i);

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
    ssalloc_align();
    max_key = pow2roundup(max_key)-1;

    root = bst_initialize(num_threads);

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

    stop = 0;

    barrier_init(&barrier, num_threads + 1);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    timeout.tv_sec = duration / 1000;
    timeout.tv_nsec = (duration % 1000) * 1000000;

    for (i = 0; i < num_threads; i++) {
        data[i].id = i;
        data[i].num_operations = 0;
        data[i].total_time=0;
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
        nanosleep(&timeout, NULL);
    } else {
        sigemptyset(&block_set);
        sigsuspend(&block_set);
    }
    stop = 1;
    gettimeofday(&end, NULL);
    /* Wait for thread completion */
    for (i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error waiting for thread completion\n");
            exit(1);
        }
    }
    DDPRINT("threads finshed\n",NULL);
    duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
    
    //bst_print(root);

    unsigned long operations = 0;
    ticks total_ticks = 0;
    for (i = 0; i < num_threads; i++) {
        printf("Thread %d\n", i);
        printf("  #operations   : %lu\n", data[i].num_operations);
        operations += data[i].num_operations;
        total_ticks += data[i].total_time;
    }

    printf("Duration      : %d (ms)\n", duration);
    printf("#operations     : %lu (%f / s)\n", operations, operations * 1000.0 / duration);
    printf("Operation latency %lu\n", total_ticks / operations);


    free(threads);
    free(data);

    return 0;

}

