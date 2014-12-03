/*   
 *   File: test.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   test.c is part of ASCYLIB
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

#include "intset.h"
#include "utils.h"

__thread unsigned long* seeds;

ALIGNED(64) uint8_t running[64];

typedef ALIGNED(64) struct thread_data 
{
  skey_t first;
  long range;
  int update;
  int alternate;
  int effective;
  int nb_threads;
  unsigned long nb_add;
  unsigned long nb_added;
  unsigned long nb_remove;
  unsigned long nb_removed;	
  unsigned long nb_contains;
  unsigned long nb_found;
  unsigned long nb_aborts;
  unsigned long nb_aborts_locked_read;
  unsigned long nb_aborts_locked_write;
  unsigned long nb_aborts_validate_read;
  unsigned long nb_aborts_validate_write;
  unsigned long nb_aborts_validate_commit;
  unsigned long nb_aborts_invalid_memory;
  unsigned long nb_aborts_double_write;
  unsigned long max_retries;
  unsigned int seed;
  intset_t *set;
  barrier_t *barrier;
  barrier_t *barrier_workers;
  unsigned long failures_because_contention;
  int id;
  uint8_t padding[48];
} thread_data_t;


void*
test(void *data) 
{
  PF_MSG(0, "rand_range");
  PF_MSG(1, "malloc");
  PF_MSG(2, "free");
  PF_MSG(3, "search");

  int unext, last = -1; 
  skey_t val = 0;
	
  thread_data_t *d = (thread_data_t *)data;
	
  set_cpu(the_cores[d->id]);
  /* Wait on barrier */
  ssalloc_init();
  PF_CORRECTION;

  seeds = seed_rand();

  barrier_cross(d->barrier);
	
  /* Is the first op an update? */
  unext = (rand_range_re(&d->seed, 100) - 1 < d->update);
	
  /* while (stop == 0)  */
  while (*running)
    {
      if (unext) { // update
			
	if (last < 0) { // add
		
	  val = rand_range_re(&d->seed, d->range);
	  if (set_add(d->set, val, TRANSACTIONAL)) {
	    d->nb_added++;
	    last = val;
	  } 				
	  d->nb_add++;
				
	} else { // remove
				
	  if (d->alternate) { // alternate mode (default)
	    if (set_remove(d->set, last)) {
	      d->nb_removed++;
	    } 
	    last = -1;
	  } else {
	    /* Random computation only in non-alternated cases */
	    val = rand_range_re(&d->seed, d->range);
	    /* Remove one random value */
	    if (set_remove(d->set, val)) {
	      d->nb_removed++;
	      /* Repeat until successful, to avoid size variations */
	      last = -1;
	    } 
	  }
	  d->nb_remove++;
	}
			
      } else { // read
				
	if (d->alternate) {
	  if (d->update == 0) {
	    if (last < 0) {
	      val = d->first;
	      last = val;
	    } else { // last >= 0
	      val = rand_range_re(&d->seed, d->range);
	      last = -1;
	    }
	  } else { // update != 0
	    if (last < 0) {
	      val = rand_range_re(&d->seed, d->range);
	      //last = val;
	    } else {
	      val = last;
	    }
	  }
	}	else val = rand_range_re(&d->seed, d->range);
			
	if (set_contains(d->set, val)) 
	  d->nb_found++;
	d->nb_contains++;
	
      }
		
      /* Is the next op an update? */
      if (d->effective) { // a failed remove/add is a read-only tx
	unext = ((100 * (d->nb_added + d->nb_removed))
		 < (d->update * (d->nb_add + d->nb_remove + d->nb_contains)));
      } else { // remove/add (even failed) is considered as an update
	unext = (rand_range_re(&d->seed, 100) - 1 < d->update);
      }
    }

  uint8_t t;
  for (t = 0; t < d->nb_threads; t++)
    {
      if (t == d->id)
	{
	  PF_PRINT;
	}
      barrier_cross(d->barrier_workers);
    }

  return NULL;
}

int test_verbose = 0;

int
main(int argc, char **argv) 
{
  set_cpu(the_cores[0]);
  ssalloc_init();
  seeds = seed_rand();

  struct option long_options[] = 
    {
      // These options don't set a flag
      {"help",                      no_argument,       NULL, 'h'},
      {"verbose",                   no_argument,       NULL, 'v'},
      {"duration",                  required_argument, NULL, 'd'},
      {"initial-size",              required_argument, NULL, 'i'},
      {"num-threads",               required_argument, NULL, 'n'},
      {"range",                     required_argument, NULL, 'r'},
      {"seed",                      required_argument, NULL, 's'},
      {"update-rate",               required_argument, NULL, 'u'},
      {"elasticity",                required_argument, NULL, 'x'},
      {"nothing",                   required_argument, NULL, 'l'},
      {NULL, 0, NULL, 0}
    };
	
  intset_t *set;
  int i, c, size;
  skey_t last = 0; 
  skey_t val = 0;
  unsigned long reads, effreads, updates, effupds, aborts, aborts_locked_read, 
    aborts_locked_write, aborts_validate_read, aborts_validate_write, 
    aborts_validate_commit, aborts_invalid_memory, aborts_double_write, 
    max_retries, failures_because_contention;
  thread_data_t *data;
  pthread_t *threads;
  pthread_attr_t attr;
  barrier_t barrier, barrier_workers;
  struct timeval start, end;
  struct timespec timeout;
  int duration = DEFAULT_DURATION;
  int initial = DEFAULT_INITIAL;
  int nb_threads = DEFAULT_NB_THREADS;
  long range = DEFAULT_RANGE;
  int update = DEFAULT_UPDATE;
  int alternate = DEFAULT_ALTERNATE;
  int effective = DEFAULT_EFFECTIVE;
  sigset_t block_set;
	
  while(1) {
    i = 0;
    c = getopt_long(argc, argv, "hvAf:d:i:n:r:s:u:x:l:", long_options, &i);
		
    if(c == -1)
      break;
		
    if(c == 0 && long_options[i].flag == 0)
      c = long_options[i].val;
		
    switch(c) {
    case 0:
      /* Flag is automatically set */
      break;
    case 'h':
      printf("ASCYLIB -- stress test "
	     "\n"
	     "\n"
	     "Usage:\n"
	     "  %s [options...]\n"
	     "\n"
	     "Options:\n"
	     "  -h, --help\n"
	     "        Print this message\n"
	     "  -A, --alternate (default="XSTR(DEFAULT_ALTERNATE)")\n"
	     "        Consecutive insert/remove target the same value\n"
	     "  -f, --effective <int>\n"
	     "        update txs must effectively write (0=trial, 1=effective, default=" XSTR(DEFAULT_EFFECTIVE) ")\n"
	     "  -d, --duration <int>\n"
	     "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
	     "  -i, --initial-size <int>\n"
	     "        Number of elements to insert before test (default=" XSTR(DEFAULT_INITIAL) ")\n"
	     "  -n, --num-threads <int>\n"
	     "        Number of threads (default=" XSTR(DEFAULT_NB_THREADS) ")\n"
	     "  -r, --range <int>\n"
	     "        Range of integer values inserted in set (default=" XSTR(DEFAULT_RANGE) ")\n"
	     "  -u, --update-rate <int>\n"
	     "        Percentage of update transactions (default=" XSTR(DEFAULT_UPDATE) ")\n"
	     , argv[0]);
      exit(0);
    case 'v':
      test_verbose = 1;
      break;
    case 'A':
      alternate = 1;
      break;
    case 'f':
      effective = atoi(optarg);
      break;
    case 'd':
      duration = atoi(optarg);
      break;
    case 'i':
      initial = atoi(optarg);
      break;
    case 'n':
      nb_threads = atoi(optarg);
      break;
    case 'r':
      range = atol(optarg);
      break;
    case 'u':
      update = atoi(optarg);
      break;
    case 'l':
      break;
    case '?':
      printf("Use -h or --help for help\n");
      exit(0);
    default:
      exit(1);
    }
  }
	
  assert(duration >= 0);
  assert(initial >= 0);
  assert(nb_threads > 0);
  assert(range > 0);
  if (range < initial)
    {
      range = 2 * initial;
    }
  assert(update >= 0 && update <= 100);
	
  printf("Bench type   : linked list\n");
  printf("Duration     : %d\n", duration);
  printf("Initial size : %d\n", initial);
  printf("Nb threads   : %d\n", nb_threads);
  printf("Value range  : %ld\n", range);
  printf("Update rate  : %d\n", update);
  printf("Alternate    : %d\n", alternate);
  printf("Effective    : %d\n", effective);
  printf("Type sizes   : int=%d/long=%d/ptr=%d/word=%d\n",
	 (int)sizeof(int),
	 (int)sizeof(long),
	 (int)sizeof(void *),
	 (int)sizeof(uintptr_t));
	
  timeout.tv_sec = duration / 1000;
  timeout.tv_nsec = (duration % 1000) * 1000000;
	
  if ((data = (thread_data_t *)malloc(nb_threads * sizeof(thread_data_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
  if ((threads = (pthread_t *)malloc(nb_threads * sizeof(pthread_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
	
  srand((int)time(0));

  set = set_new();
  /* stop = 0; */
  *running = 1;
	
  /* Init STM */
  printf("Initializing STM\n");
	
  size_t ten_perc = initial / 10, tens = 1;
  size_t ten_perc_nxt = ten_perc;

  /* Populate set */
  printf("Adding %d entries to set\n", initial);

  if (initial < 10000)
    {
      i = 0;
      while (i < initial) 
	{
	  val = rand_range(range);
	  if (set_add(set, val, val)) 
	    {
	      last = val;
	      if (i == ten_perc_nxt)
		{
		  printf("%02lu%%  ", tens * 10); fflush(stdout);
		  tens++;
		  ten_perc_nxt = tens * ten_perc;
		}
	      i++;
	    }
	}
    }
  else
    {
      for (i = initial; i > 0; i--)
	{
	  set_add(set, i, val);
	}
    }
  printf("\n");
  size = set_size(set);
  printf("Set size     : %d\n", size);
	
  /* Access set from all threads */
  barrier_init(&barrier, nb_threads + 1);
  barrier_init(&barrier_workers, nb_threads);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  printf("Creating threads: ");
  for (i = 0; i < nb_threads; i++) 
    {
      printf("%2d, ", i);
      data[i].first = last;
      data[i].range = range;
      data[i].update = update;
      data[i].alternate = alternate;
      data[i].effective = effective;
      data[i].nb_threads = nb_threads;
      data[i].nb_add = 0;
      data[i].nb_added = 0;
      data[i].nb_remove = 0;
      data[i].nb_removed = 0;
      data[i].nb_contains = 0;
      data[i].nb_found = 0;
      data[i].nb_aborts = 0;
      data[i].nb_aborts_locked_read = 0;
      data[i].nb_aborts_locked_write = 0;
      data[i].nb_aborts_validate_read = 0;
      data[i].nb_aborts_validate_write = 0;
      data[i].nb_aborts_validate_commit = 0;
      data[i].nb_aborts_invalid_memory = 0;
      data[i].nb_aborts_double_write = 0;
      data[i].max_retries = 0;
      data[i].seed = rand();
      data[i].set = set;
      data[i].barrier = &barrier;
      data[i].barrier_workers = &barrier_workers;
      data[i].failures_because_contention = 0;
      data[i].id = i;
      if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) {
	fprintf(stderr, "Error creating thread\n");
	exit(1);
      }
    }
  printf("\n");
  pthread_attr_destroy(&attr);
	
  /* Start threads */
  barrier_cross(&barrier);
	
  printf("STARTING...\n");
  gettimeofday(&start, NULL);
  if (duration > 0) {
    nanosleep(&timeout, NULL);
  } else {
    sigemptyset(&block_set);
    sigsuspend(&block_set);
  }
	
  /* AO_store_full(&stop, 1); */
  *running = 0;

  gettimeofday(&end, NULL);
  printf("STOPPING...\n");
	
  /* Wait for thread completion */
  for (i = 0; i < nb_threads; i++) 
    {
      if (pthread_join(threads[i], NULL) != 0) {
	fprintf(stderr, "Error waiting for thread completion\n");
	exit(1);
      }
    }
	
  duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
  aborts = 0;
  aborts_locked_read = 0;
  aborts_locked_write = 0;
  aborts_validate_read = 0;
  aborts_validate_write = 0;
  aborts_validate_commit = 0;
  aborts_invalid_memory = 0;
  aborts_double_write = 0;
  failures_because_contention = 0;
  reads = 0;
  effreads = 0;
  updates = 0;
  effupds = 0;
  max_retries = 0;
  for (i = 0; i < nb_threads; i++) 
    {
      if (test_verbose)
	{
	  printf("Thread %d\n", i);
	  printf("  #add        : %lu\n", data[i].nb_add);
	  printf("    #added    : %lu\n", data[i].nb_added);
	  printf("  #remove     : %lu\n", data[i].nb_remove);
	  printf("    #removed  : %lu\n", data[i].nb_removed);
	  printf("  #contains   : %lu\n", data[i].nb_contains);
	  printf("    #found    : %lu\n", data[i].nb_found);
	}
      aborts += data[i].nb_aborts;
      aborts_locked_read += data[i].nb_aborts_locked_read;
      aborts_locked_write += data[i].nb_aborts_locked_write;
      aborts_validate_read += data[i].nb_aborts_validate_read;
      aborts_validate_write += data[i].nb_aborts_validate_write;
      aborts_validate_commit += data[i].nb_aborts_validate_commit;
      aborts_invalid_memory += data[i].nb_aborts_invalid_memory;
      aborts_double_write += data[i].nb_aborts_double_write;
      failures_because_contention += data[i].failures_because_contention;
      reads += data[i].nb_contains;
      effreads += data[i].nb_contains + 
	(data[i].nb_add - data[i].nb_added) + 
	(data[i].nb_remove - data[i].nb_removed); 
      updates += (data[i].nb_add + data[i].nb_remove);
      effupds += data[i].nb_removed + data[i].nb_added; 
      size += data[i].nb_added - data[i].nb_removed;
      if (max_retries < data[i].max_retries)
	max_retries = data[i].max_retries;
    }
  printf("Set size      : %d (expected: %d)\n", set_size(set), size);
  printf("Duration      : %d (ms)\n", duration);
  printf("#txs          : %lu (%f / s)\n", reads + updates, 
	 (reads + updates) * 1000.0 / duration);
	
  printf("#read txs     : ");
  if (effective) {
    printf("%lu (%f / s)\n", effreads, effreads * 1000.0 / duration);
    printf("  #contains   : %lu (%f / s)\n", reads, reads * 1000.0 / duration);
  } else printf("%lu (%f / s)\n", reads, reads * 1000.0 / duration);
	
  printf("#eff. upd rate: %f \n", 100.0 * effupds / (effupds + effreads));
	
  printf("#update txs   : ");
  if (effective) {
    printf("%lu (%f / s)\n", effupds, effupds * 1000.0 / duration);
    printf("  #upd trials : %lu (%f / s)\n", updates, updates * 1000.0 / 
	   duration);
  } else printf("%lu (%f / s)\n", updates, updates * 1000.0 / duration);
	
	
  /* Delete set */
  /* set_delete(set); */
	
  free(threads);
  free(data);
	
  return 0;
}
