 /*A Lock Free Binary Search Tree
 
 * File:
 *   wfrbt.cpp
 * Author(s):
 *   Aravind Natarajan <natarajan.aravind@gmail.com>
 * Description:
 *   A Lock Free Binary Search Tree
 *
 * Copyright (c) 20013-2014.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

Please cite our PPoPP 2014 paper - Fast Concurrent Lock-Free Binary Search Trees by Aravind Natarajan and Neeraj Mittal if you use our code in your experiments	

 Features:
1. Insert operations directly install their window without injecting the operation into the tree. They help any conflicting operation at the injection point, before executing their window txn.
2. Delete operations are the same as that of the original algorithm.
 
 */



#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "wfrbt.h"
#include "operations.h" 


#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
/* Note: stdio is thread-safe */
#endif

/*
 * Useful macros to work with transactions. Note that, to use nested
 * transactions, one should check the environment returned by
 * stm_get_env() and only call sigsetjmp() if it is not null.
 */
 
#define RO                              1
#define RW                              0

#define DEFAULT_DURATION                2000
#define DEFAULT_TABLE_SIZE              10
#define DEFAULT_NB_THREADS              2
#define DEFAULT_SEED                    0
#define DEFAULT_SEARCH_FRAC             0.0
#define DEFAULT_INSERT_FRAC             0.5
#define DEFAULT_DELETE_FRAC             0.5
#define DEFAULT_READER_THREADS         	0
#define DEFAULT_KEYSPACE1_SIZE          10000
#define DEFAULT_KEYSPACE2_SIZE          1000000000
#define KEYSPACE1_PROB           1.0
#define KEYSPACE2_PROB           0.0
#define LOWERHALF 		 0.99

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

/* ################################################################### *
 * GLOBALS
 * ################################################################### */

 


 
static volatile AO_t stop = 0;
static volatile AO_t stop2 = 0;

long total_insert = 0;



timespec t;
/* STRUCTURES */


enum{
Front,
Back
};


long blackCount = -1;
long leafNodes = 0;



/* ################################################################### *
 * Correctness Checking
 * ################################################################### */
 
long in_order_visit(node_t * rootNode){
	
	

	
	long key = rootNode->key;
	
	if((node_t *)get_addr(rootNode->child.AO_val1) == NULL){
		leafNodes++;
		return (key);
	}
	
	node_t * lChild = (node_t *)get_addr(rootNode->child.AO_val1);
	node_t * rChild = (node_t *)get_addr(rootNode->child.AO_val2);
	
	if((lChild) != NULL){
	
		long lKey = in_order_visit(lChild);
		if(lKey >= key){
			std::cout << "Lkey is larger!!__" << lKey << "__ " << key << std::endl;
			std::cout << "Sanity Check Failed!!" << std::endl;
		}
	}
	
	
	if((rChild) != NULL){
	
		long rKey = in_order_visit(rChild);
		if(rKey < key){
			std::cout << "Rkey is smaller!!__" << rKey << "__ " << key <<  std::endl;
			std::cout << "Sanity Check Failed!!" << std::endl;
		}
	}
	return (key);
}




/*************************************************************************************************/
/*************************************************************************************************/







/* ################################################################### *
 * BARRIER
 * ################################################################### */



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

/* ################################################################### *
 * TEST
 * ################################################################### */



void *testRW(void *data)
{
  
   double action;
  
   Word key;
   thread_data_t *d = (thread_data_t *)data;
 
  
  
  
  

  /* Wait on barrier */
   bool prepop = false;
   
restart:  
  barrier_cross(d->barrier);

while (stop == 0) {

	
	
     // determine what we're going to do
     action = (double)rand_r(&d->seed) / (double) RAND_MAX;
   
   	
     
   
  	key = rand_r(&d->seed) % (d->keyspace1_size);
	      while(key == 0){
	      	key = rand_r(&d->seed) % (d->keyspace1_size);
	      }
 
     if (action <= d->search_frac)
   	{
    //	auto search_start = std::chrono::high_resolution_clock::now();
	search(d, key);
//	auto search_end = std::chrono::high_resolution_clock::now();
	
	//auto search_dur = std::chrono::duration_cast<std::chrono::microseconds>(search_end - search_start);

   	
	#ifdef DETAILED_STATS
///	d->tot_read_time += search_dur.count();
	d->tot_reads++;
	#endif
     
     }
    
          
    else if (action > d->search_frac && action <= (d->search_frac + d->insert_frac))
     {
     	
	
	 bool slowOpn;
		//auto ins_start = std::chrono::high_resolution_clock::now();
		
		//long foundKey = fastsearch(d, key);
		
		//if(foundKey == key){
		//	slowOpn = false;			
		//}
		//else{	
		 slowOpn = insert(d,key);
		//}
		
		
		//auto ins_end = std::chrono::high_resolution_clock::now();
	
		//auto ins_dur = std::chrono::duration_cast<std::chrono::microseconds>(ins_end - ins_start);
   	#ifdef DETAILED_STATS
		if(slowOpn){
			//d->tot_slowins_time += ins_dur.count();
			d->tot_slowins_count++;
		}
		else{
			//d->tot_fastins_time += ins_dur.count();
			d->tot_fastins_count++;
		}
	#endif	
	
	
	

  
     }
     
     else
     {
    
        // Delete Operation
	
	
	
     
      bool slowOpn;
     //	auto del_start = std::chrono::high_resolution_clock::now();
     	
     	//long foundKey = fastsearch(d, key);
     	
     	//if(foundKey != key){
	//	slowOpn = false;			
	//}
	//else{
		slowOpn = delete_node(d,key);
	//}
	//auto del_end = std::chrono::high_resolution_clock::now();
	//auto del_dur = std::chrono::duration_cast<std::chrono::microseconds>(del_end - del_start);
   	#ifdef DETAILED_STATS
		if(slowOpn){
			//d->tot_slowdel_time += del_dur.count();
			d->tot_slowdel_count++;
		}
		else{
			//d->tot_fastdel_time += del_dur.count();
			d->tot_fastdel_count++;
		}	
	#endif
     }
     
     
     d->ops++;
     
     	
	
	
	
	
  }
	
	if(!prepop){
	
		barrier_cross(d->barrier2);
	}
	
	
	if(stop2 == 1){
		if(prepop){
			//Done.
				
			return NULL;
		}
		else{
			
			prepop = true;
			d->ops = 0;
			
			
		}
	
	}
	
	goto restart; 

  return NULL;
}

void catcher(int sig)
{
  printf("CAUGHT SIGNAL %d\n", sig);
}

int main(int argc, char **argv)
{
  struct option long_options[] = {
    // These options don't set a flag
    {"help",                      no_argument,       NULL, 'h'},
    {"table-size",                required_argument, NULL, 't'},
    {"duration",                  required_argument, NULL, 'd'},
    {"num-threads",               required_argument, NULL, 'n'},
    {"seed",                      required_argument, NULL, 's'},
    {"search-fraction",           required_argument, NULL, 'r'},
    {"insert-update-fraction",    required_argument, NULL, 'i'},
    {"delete-fraction",           required_argument, NULL, 'x'},
    {"keyspace1-size",             required_argument, NULL, 'k'},
    {"keyspace2-size",             required_argument, NULL, 'l'},
    {"gc-node-threshold",             required_argument, NULL, 'g'},
    {"reader-threads",             required_argument, NULL, 'a'},
    {NULL, 0, NULL, 0}
  };



  int i, c;
  char *s;
  unsigned long inserts, deletes, actInsert, actDelete;
  thread_data_t *data;
  pthread_t *threads;
  pthread_attr_t attr;
  barrier_t barrier;
   barrier_t barrier2;
  
  struct timeval start, end, end1;
  struct timespec timeout;
  
  struct timespec gc_timeout;
  
  int duration = DEFAULT_DURATION;
  
  int tablesize = DEFAULT_TABLE_SIZE;
  int nb_threads = DEFAULT_NB_THREADS;
  double search_frac = DEFAULT_SEARCH_FRAC;
  double insert_frac = DEFAULT_INSERT_FRAC;
  double delete_frac = DEFAULT_DELETE_FRAC;
  long keyspace1_size = DEFAULT_KEYSPACE1_SIZE;
  long keyspace2_size = DEFAULT_KEYSPACE2_SIZE;
  long gcThr = GC_NODE_THRESHOLD;
  int reader_threads = DEFAULT_READER_THREADS;
  int seed = DEFAULT_SEED;
  sigset_t block_set;
  

  while(1) {
    i = 0;
    c = getopt_long(argc, argv, "ht:d:n:s:r:i:x:k:l:g:a:", long_options, &i);

    if(c == -1)
      break;

    if(c == 0 && long_options[i].flag == 0)
      c = long_options[i].val;

    switch(c) {
     case 0:
       /* Flag is automatically set */
       break;
     case 'h':
       printf("Balanced Search Tree\n"
              "\n"
              "Usage:\n"
              "  BST [options...]\n"
              "\n"
              "Options:\n"
              "  -h, --help\n"
              "        Print this message\n"
              "  -t, --table-size <int>\n"
              "        Number of cells in the hash table (default=" XSTR(DEFAULT_TABLE_SIZE) ")\n"
              "  -d, --duration <int>\n"
              "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
              "  -n, --num-threads <int>\n"
              "        Number of threads (default=" XSTR(DEFAULT_NB_THREADS) ")\n"
              "  -s, --seed <int>\n"
              "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
	      "  -r, --search-fraction <int>\n"
              "        Number of search Threads (default=" XSTR(DEFAULT_SEARCH_FRAC) ")\n"
              "  -i, --insert-update-fraction <int>\n"
              "        Number of insert/update Threads (default=" XSTR(DEFAULT_INSERT_FRAC) ")\n"
              "  -x, --delete-fraction <double>\n"
              "        Fraction of delete operations (default=" XSTR(DEFAULT_DELETE_FRAC) ")\n"
              "  -a, --reader-threads <int>\n"
              "        Number of reader threads (default=" XSTR(DEFAULT_READER_THREADS) ")\n"
              "  -k, --keyspace-size <int>\n"
               "       Number of possible keys (default=" XSTR(DEFAULT_KEYSPACE1_SIZE) ")\n"
              "  -l, --keyspace-size <int>\n"
               "       Number of possible keys (default=" XSTR(DEFAULT_KEYSPACE2_SIZE) ")\n"
              "  -g, --gc <int>\n"
               "       garbage collection threshold (default=" XSTR(GC_NODE_THRESHOLD) ")\n"
         );
       exit(0);
     case 't':
       tablesize = atoi(optarg);
       break;
     case 'd':
       duration = atoi(optarg);
       break;
     case 'n':
       nb_threads = atoi(optarg);
       break;
     case 's':
       seed = atoi(optarg);
       break;
     case 'r':
       search_frac = atof(optarg);
       break;
     case 'i':
       insert_frac = atof(optarg);
       break;
     case 'x':
       delete_frac = atof(optarg);
       break;
     case 'a':
       reader_threads = atoi(optarg);
       break;  
    case 'k':
       keyspace1_size = atoi(optarg);
       break;
     case 'l':
       keyspace2_size = atoi(optarg);
       break;
     case 'g':
       gcThr = atoi(optarg);
       break;
     case '?':
       printf("Use -h or --help for help\n");
       exit(0);
     default:
       exit(1);
    }
  }

  assert(duration >= 0);
  assert(tablesize >= 2);
  assert(nb_threads > 0);
 
  assert(reader_threads < nb_threads);


	
  printf("Duration       : %d\n", duration);
  printf("Nb threads     : %d\n", nb_threads);
  printf("Seed           : %d\n", seed);
  printf("Search frac    : %f\n", search_frac); 	
  printf("Insert frac    : %f\n", insert_frac);
  printf("Delete frac    : %f\n", delete_frac);
  printf("Reader Threads : %d\n", reader_threads);
  printf("Update Threads : %d\n", (nb_threads - reader_threads));   
  printf("Keyspace1 size : %d\n", keyspace1_size);
  printf("GC Threshold   : %ld\n", gcThr);


  timeout.tv_sec = duration / 1000;
  timeout.tv_nsec = (duration % 1000) * 1000000;
  
  
  data = new thread_data_t[nb_threads];


  if ((threads = (pthread_t *)malloc(nb_threads * sizeof(pthread_t))) == NULL) {
    perror("malloc");
    exit(1);
  }

  if (seed == 0)
    srand((int)time(0));
  else
    srand(seed);


 

node_t * newRT = new node_t;
 node_t * newLC = new node_t;
 node_t * newRC = new node_t;
 
 newRT->key = keyspace1_size+2;
 newLC->key = keyspace1_size+1;
 newRC->key = keyspace1_size+2;
 
 
 newRT->child.AO_val1 = create_child_word(newLC,UNMARK, UNFLAG);
 newRT->child.AO_val2 = create_child_word(newRC,UNMARK, UNFLAG);
 
  
  
 	
	#ifdef UPDATEVAL
		newLC->value = 0;
		newRC->value = 0;
	#endif
 	
 
 
  stop = 0;

//Pre-populate Tree-------------------------------------------------
int pre_inserts = 2;
	int i1 = 0;
data[i1].id = i1+1;
    data[i1].numThreads = nb_threads;
    data[i1].numInsert = 0;
    data[i1].numActualDelete = 0;
    data[i1].seed = rand();
    data[i1].search_frac = 0.0;
    data[i1].insert_frac = 1.0;
    data[i1].delete_frac = 0.0;
    data[i1].keyspace1_size = keyspace1_size;
    data[i1].ops = 0;
    data[i1].rootOfTree = newRT;
    
    	
    
    data[i1].barrier = &barrier;
    
    #ifdef DETAILED_STATS
     data[i1].tot_reads = 0;
       data[i1].tot_fastins_time = 0;
    data[i1].tot_slowins_time = 0;
    data[i1].tot_fastins_count = 0;
    data[i1].tot_slowins_count = 0;
    
    data[i1].tot_fastdel_time = 0;
    data[i1].tot_slowdel_time = 0;
    data[i1].tot_fastdel_count = 0;
    data[i1].tot_slowdel_count = 0;
     data[i1].count = 0;
     #endif
  
	
	data[i1].recycledNodes.reserve(RECYCLED_VECTOR_RESERVE);
	
    	
    	data[i].sr = new seekRecord_t;
    	data[i].ssr = new seekRecord_t;
    	
    	 Word key;
    	
   
   	while(data[0].numInsert < (keyspace1_size/2))	
    	{
	key = rand_r(&data[0].seed) % (keyspace1_size);
	        while(key == 0){
	      		key = rand_r(&data[0].seed) % (keyspace1_size);
	      	} 
    			
		insert(&data[i1],key);
    	} 
    	pre_inserts+= data[0].numInsert; 
    		//------------------------------------------------------------------- 
    		std::cout << "pre_inserts = " << pre_inserts << std::endl; 	
  
 
  barrier_init(&barrier, nb_threads + 1);

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
 
    
  for (i = 0; i < nb_threads; i++) {

    data[i].id = i+1;
    data[i].numThreads = nb_threads;
    data[i].numInsert = 0;
    data[i].numActualDelete = 0;
    data[i].seed = rand();
    data[i].search_frac = search_frac;
    data[i].insert_frac = insert_frac;
    data[i].delete_frac = delete_frac;
    data[i].keyspace1_size = keyspace1_size;

    data[i].ops = 0;
    data[i].rootOfTree = newRT;
    data[i].barrier = &barrier;
    data[i].barrier2 = &barrier2;
   
    #ifdef DETAILED_STATS	 
    data[i].tot_reads = 0;
     
   data[i].tot_read_time = 0;
    
     data[i].tot_reads = 0;
   
     data[i].tot_fastins_time = 0;
    data[i].tot_slowins_time = 0;
    data[i].tot_fastins_count = 0;
    data[i].tot_slowins_count = 0;
    
    data[i].tot_fastdel_time = 0;
    data[i].tot_slowdel_time = 0;
    data[i].tot_fastdel_count = 0;
    data[i].tot_slowdel_count = 0;
    data[i].insertSeqNumber = 0;
    data[i].count = 0;
  #endif
	
	data[i].recycledNodes.reserve(RECYCLED_VECTOR_RESERVE);
	
	
    	
    	
    	data[i].sr = new seekRecord_t;
    	data[i].ssr = new seekRecord_t;
    	
    	
	
    
    	if (pthread_create(&threads[i], &attr, testRW, (void *)(&data[i])) != 0) {
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
  
   bool startsim = false;
    
  while(!startsim){  
     leafNodes = 0;
    barrier_cross(&barrier);
    // let the threads run for a while.
    sleep(2);
    barrier_init(&barrier2, nb_threads + 1);	
    AO_store_full(&stop, 1);
    // check the size of the tree. Within 5% of steady state size
  
    in_order_visit(newRT);
   
    
    if((leafNodes > ((1.05)*(insert_frac*keyspace1_size)/(insert_frac+delete_frac))) || (leafNodes < ((0.95)*(insert_frac*keyspace1_size)/(insert_frac+delete_frac)))){
	
	 
	  barrier_init(&barrier, nb_threads + 1);
  	  barrier_cross(&barrier2);
    	   AO_store_full(&stop, 0);

    }
    else{
    	// prepopulation done
    	
    	barrier_init(&barrier, nb_threads + 1);
    	AO_store_full(&stop2, 1);
    	barrier_cross(&barrier2);
    	startsim = true;
  	  AO_store_full(&stop, 0);
    }	
    
    	
} 
   
   
    
    barrier_cross(&barrier);

  
  gettimeofday(&start, NULL);
  if (duration > 0) {
	nanosleep(&timeout, NULL);
	
  } 
  else {
    sigemptyset(&block_set);
    sigsuspend(&block_set);
    }
    
    	AO_store_full(&stop, 1);	
    gettimeofday(&end, NULL);
  
    
  /* Wait for thread completion */
  

   
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error waiting for thread completion\n");
      exit(1);
    }
  } 
  
  // CONSISTENCY CHECK
  
  
	
   leafNodes = 0;	
 
  
  
   long rootkey =  in_order_visit((newRT));
 
   unsigned long tot_inserts = 0;
   unsigned long tot_deletes = 0;
   unsigned long tot_ops = 0;
   
   #ifdef DETAILED_STATS
   unsigned long tot_reads = 0;
    double tot_read_time = 0;
   
   
   
   double tot_fastins_time = 0;
   
   double tot_slowins_time = 0;
   
   double tot_fastdel_time = 0;
   
   double tot_slowdel_time = 0;
   

   unsigned long tot_fastins_count = 0;
   unsigned long tot_slowins_count = 0;
   
   unsigned long tot_fastdel_count = 0;
   unsigned long tot_slowdel_count = 0;


   	uint64_t inscases = 0;
   	
	uint64_t inswin = 0;
#endif

   for(int i = 0; i < nb_threads; i++){
   	tot_inserts += data[i].numInsert;
   	tot_deletes += data[i].numActualDelete;
   	
   	tot_ops += data[i].ops;
   	
   	#ifdef DETAILED_STATS
   	tot_reads += data[i].tot_reads;
   	tot_read_time += (data[i].tot_read_time);
   	
   	tot_fastins_time += (data[i].tot_fastins_time);
   	tot_slowins_time += (data[i].tot_slowins_time);
   	
   	tot_fastdel_time += (data[i].tot_fastdel_time);
   	tot_slowdel_time += (data[i].tot_slowdel_time);
   	
   	tot_fastins_count += data[i].tot_fastins_count; 
   	 tot_slowins_count += data[i].tot_slowins_count;
   	 
   	 tot_fastdel_count += data[i].tot_fastdel_count; 
   	 tot_slowdel_count += data[i].tot_slowdel_count;
	#endif
	
   }
   
   tot_inserts+= pre_inserts;
     unsigned long tot_updates = 0;
 for(int i = reader_threads; i < nb_threads; i++){
 	tot_updates += data[i].ops;
 	
 }
    std::cout << "Total Number of Nodes ( " << tot_inserts << ", " << tot_deletes << " ) = " << tot_inserts - tot_deletes << std::endl;
   std::cout << "Total Number of leaf nodes (sanity check) = " << leafNodes << std::endl;
   
   if((tot_inserts - tot_deletes) != leafNodes){
   	std::cout << "ERROR. MISMATCH" << std::endl;
   	exit(0);
   } 
   
  duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
  
  std::cout << "*--PERFORMANCE STATISTICS--------*" << std::endl;
  std::cout << "Total Operations per sec = " << (tot_ops * 1000)/duration << std::endl; 
  #ifdef DETAILED_STATS
  std::cout << "Total reads = " << tot_reads << std::endl;
  std::cout << "Total updates = " << tot_updates << std::endl;
  std::cout << "AVG read time(usec) = " << tot_read_time/tot_reads << std::endl; 
  std::cout << "*--------------------------------------*" << std::endl;
 
  std::cout << "Number of Slow Insert opns = " << tot_slowins_count<< std::endl;
  std::cout << "Avg. Slow Insert Time(usec) = " << tot_slowins_time/tot_slowins_count<< std::endl;

  std::cout << "\n";
  std::cout << "Number of Fast Insert opns = " << tot_fastins_count<< std::endl;
  std::cout << "Avg. Fast Insert Time(usec) = " << tot_fastins_time/tot_fastins_count<< std::endl;
  std::cout << "\n";
  std::cout << "Number of Slow Delete opns = " << tot_slowdel_count<< std::endl;
  std::cout << "Avg. Slow Delete Time(usec) = " << tot_slowdel_time/tot_slowdel_count<< std::endl;
  std::cout << "\n";

  std::cout << "Number of Fast Delete opns = " << tot_fastdel_count<< std::endl;
  std::cout << "Avg. Fast Delete Time(usec) = " << tot_fastdel_time/tot_fastdel_count<< std::endl;

  #endif

  
  
  free(threads);
  delete [] data;
  return 0;
}
