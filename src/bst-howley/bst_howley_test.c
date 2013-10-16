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

int num_threads;

// TODO how do we know that threads are correctly set up on separate cpus?
// TODO seems to work correctly with num_threads = 10. WAT. wrap-around for set_cpu/the_cores?

//used to signal the threads when to stop
// ALIGNED(64) uint8_t running[64];

//the root of the binary search tree
node_t * root;

//data structure through which we send parameters to and get results from the worker threads
typedef ALIGNED(64) struct thread_data {
    //pointer to the global barrier
    // barrier_t *barrier;

    //counts the number of operations each thread performs
    // unsigned long num_operations;
    //total operation time (not used here)
    // ticks total_time;
    //seed (not used here)
    // unsigned int seed;
    //the number of elements each thread should add at the beginning of its execution
    // uint64_t num_add;
    //number of inserts a thread performs
    unsigned long num_insert;
    // number of removes a thread performs
    unsigned long num_remove;
    //number of searches a thread performs
    unsigned long num_search;
    //the id of the thread (used for thread placement on cores)
    int id;
} thread_data_t;

void *test(void *data) {
	fprintf(stderr, "Starting test\n");
	//get the per-thread data
    thread_data_t *d = (thread_data_t *)data;

    //place the thread on the apropriate cpu
    set_cpu(the_cores[d->id]);
    int op_count = 10000;

    ssalloc_init();


	int i;
	bool_t added;

	for ( i = 1; i <= op_count; i++){

		added = bst_add(i);
		fprintf(stderr, "[%d] Added %d? %d\n", d->id, i, added==TRUE);
		if (added == TRUE) {
			d->num_insert++;
		}
	}

	// printf("Root right node: %d", root->right->key);
	
	for ( i = 1; i <= op_count; i++){

		bool_t found = bst_contains(i);
		printf("Contains %d? %d\n", i, found==FOUND);
		if (found == FOUND) {
			d->num_search ++;
		} 
	}

	for ( i = 1; i <= op_count; i++){

		bool_t removed = bst_remove(i);
		printf("Removed %d? %d\n", i, removed==TRUE);
		if (removed == TRUE) {
			d->num_remove ++;
		}
	}

	// for ( i = 1; i < 10; i++){

	// 	bool_t found = bst_contains(i);
	// 	printf("Contains %d? %d\n", i, found==FOUND);
	// }


	return NULL;
}

int main(int argc, char* const argv[]) {

	num_threads = 10;
	int i;
    
	//place thread on the first cpu
    set_cpu(the_cores[7]);

	ssalloc_init();
	//alignment in the custom memory allocator to a 64 byte boundary 
    ssalloc_align();

	pthread_t *threads;
    pthread_attr_t attr;
    thread_data_t *data;


	node_t* root = bst_initialize();
	printf("Initialized tree\n");

	//initialize the data which will be passed to the threads
    if ((data = (thread_data_t *)malloc(num_threads * sizeof(thread_data_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    if ((threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t))) == NULL) {
        perror("malloc");
        exit(1);
    }
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (i = 0; i < num_threads; i++) {

    	data[i].id = i;
        data[i].num_insert=0;
        data[i].num_remove=0;
        data[i].num_search=0;

        if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }
    pthread_attr_destroy(&attr);

 	/* Wait for thread completion */
    for (i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error waiting for thread completion\n");
            exit(1);
        }
    }

    for (i = 0; i < num_threads; i++) {
        printf("Thread %d\n", i);
   
        printf("  #inserts   : %lu\n", data[i].num_insert);
        printf("  #searches   : %lu\n", data[i].num_search);
        printf("  #removes   : %lu\n", data[i].num_remove);
        
    }

	free(threads);
    free(data);

	return 0;
}