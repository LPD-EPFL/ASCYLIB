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

extern ALIGNED(64) unsigned int levelmax;
__thread unsigned long* seeds;

ALIGNED(64) uint8_t running[64];

typedef ALIGNED(64) struct thread_data 
{
  skey_t first;
  long range;
  int update;
  int unit_tx;
  int alternate;
  int effective;
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
  sl_intset_t *set;
  barrier_t *barrier;
  unsigned long failures_because_contention;
  int id;
} thread_data_t;


void print_skiplist(sl_intset_t *set) 
{
  sl_node_t *curr;
  int i, j;
  int arr[levelmax];
	
  for (i=0; i< sizeof arr/sizeof arr[0]; i++) arr[i] = 0;
	
  curr = set->head;
  do {
    printf("%d", (int) curr->val);
    for (i=0; i<curr->toplevel; i++) {
      printf("-*");
    }
    arr[curr->toplevel-1]++;
    printf("\n");
    curr = curr->next[0];
  } while (curr); 
  for (j=0; j<levelmax; j++)
    printf("%d nodes of level %d\n", arr[j], j);
}


void*
test(void *data) 
{
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
	
  /* while (stop == 0) { */
  while (*running)
    {		
      if (unext) { // update
			
	if (last < 0) { // add
				
	  val = rand_range_re(&d->seed, d->range);
	  if (sl_add(d->set, val, val)) {
	    d->nb_added++;
	    last = val;
	  } 				
	  d->nb_add++;
				
	} else { // remove
				
	  if (d->alternate) { // alternate mode (default)
	    if (sl_remove(d->set, last)) {
	      d->nb_removed++;
	    } 
	    last = -1;
	  } else {
	    /* Random computation only in non-alternated cases */
	    val = rand_range_re(&d->seed, d->range);
	    /* Remove one random value */
	    if (sl_remove(d->set, val)) {
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
			
	PF_START(2);
	if (sl_contains(d->set, val)) 
	  d->nb_found++;
	PF_STOP(2);	
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
	
  PF_PRINT;

  return NULL;
}

void catcher(int sig)
{
	printf("CAUGHT SIGNAL %d\n", sig);
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
    {"seed",                      required_argument, NULL, 's'},
    {"update-rate",               required_argument, NULL, 'u'},
    {"elasticity",                required_argument, NULL, 'x'},
    {"nothing",                   required_argument, NULL, 'l'},
    {NULL, 0, NULL, 0}
  };
	
  sl_intset_t *set;
  int i, c, size;
  skey_t last = 0; 
  skey_t val = 0;
  unsigned long reads, effreads, updates, effupds, aborts, aborts_locked_read, aborts_locked_write,
    aborts_validate_read, aborts_validate_write, aborts_validate_commit,
    aborts_invalid_memory, aborts_double_write, max_retries, failures_because_contention;
  thread_data_t *data;
  pthread_t *threads;
  pthread_attr_t attr;
  barrier_t barrier;
  struct timeval start, end;
  struct timespec timeout;
  int duration = DEFAULT_DURATION;
  int initial = DEFAULT_INITIAL;
  int nb_threads = DEFAULT_NB_THREADS;
  long range = DEFAULT_RANGE;
  int seed = 0;
  int update = DEFAULT_UPDATE;
  int unit_tx = DEFAULT_ELASTICITY;
  int alternate = DEFAULT_ALTERNATE;
  int effective = DEFAULT_EFFECTIVE;
  sigset_t block_set;
	
  while(1) {
    i = 0;
    c = getopt_long(argc, argv, "hAf:d:i:n:r:s:u:x:l:", long_options, &i);
		
    if(c == -1)
      break;
		
    if(c == 0 && long_options[i].flag == 0)
      c = long_options[i].val;
		
    switch(c) {
    case 0:
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
	     "  -A, --Alternate\n"
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
	     "  -s, --seed <int>\n"
	     "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
	     "  -u, --update-rate <int>\n"
	     "        Percentage of update transactions (default=" XSTR(DEFAULT_UPDATE) ")\n"
	     "  -x, --elasticity (default=4)\n"
	     "        Use elastic transactions\n"
	     "        0 = non-protected,\n"
	     "        1 = normal transaction,\n"
	     "        2 = read elastic-tx,\n"
	     "        3 = read/add elastic-tx,\n"
	     "        4 = read/add/rem elastic-tx,\n"
	     "        5 = fraser lock-free\n"
	     , argv[0]);
      exit(0);
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
    case 's':
      seed = atoi(optarg);
      break;
    case 'u':
      update = atoi(optarg);
      break;
    case 'x':
      unit_tx = atoi(optarg);
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
  assert(update >= 0 && update <= 100);

  if (range < initial)
    {
      range = 2 * initial;
    }
	
  printf("Set type     : skip list\n");
  printf("Duration     : %d\n", duration);
  printf("Initial size : %u\n", initial);
  printf("Nb threads   : %d\n", nb_threads);
  printf("Value range  : %ld\n", range);
  printf("Seed         : %d\n", seed);
  printf("Update rate  : %d\n", update);
  printf("Elasticity   : %d\n", unit_tx);
  printf("Alternate    : %d\n", alternate);
  printf("Efffective   : %d\n", effective);
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
	
  if (seed == 0)
    srand((int)time(0));
  else
    srand(seed);
	
  levelmax = floor_log_2((unsigned int) initial);
  set = sl_set_new();

  /* stop = 0; */
  *running = 1;

  // Populate set 
  printf("Adding %d entries to set\n", initial);
  i = 0;
	
  while (i < initial) 
    {
      val = rand_range_re(NULL, range);
      if (sl_add(set, val, val)) 
	{
	  last = val;
	  i++;
	}
    }
  size = sl_set_size(set);
  printf("Set size     : %d\n", size);
  printf("Level max    : %d\n", levelmax);
	
  // Access set from all threads 
  barrier_init(&barrier, nb_threads + 1);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  printf("Creating threads: ");
  for (i = 0; i < nb_threads; i++)
    {
      printf("%d, ", i);
      data[i].first = last;
      data[i].range = range;
      data[i].update = update;
      data[i].unit_tx = unit_tx;
      data[i].alternate = alternate;
      data[i].effective = effective;
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
      data[i].failures_because_contention = 0;
      data[i].id = i;
      if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) {
	fprintf(stderr, "Error creating thread\n");
	exit(1);
      }
    }
  printf("\n");
  pthread_attr_destroy(&attr);
	
  // Catch some signals 
  if (signal(SIGHUP, catcher) == SIG_ERR ||
      //signal(SIGINT, catcher) == SIG_ERR ||
      signal(SIGTERM, catcher) == SIG_ERR) {
    perror("signal");
    exit(1);
  }
	
  // Start threads 
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
	
  // Wait for thread completion 
  for (i = 0; i < nb_threads; i++) {
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
  for (i = 0; i < nb_threads; i++) {
    printf("Thread %d\n", i);
    printf("  #add        : %lu\n", data[i].nb_add);
    printf("    #added    : %lu\n", data[i].nb_added);
    printf("  #remove     : %lu\n", data[i].nb_remove);
    printf("    #removed  : %lu\n", data[i].nb_removed);
    printf("  #contains   : %lu\n", data[i].nb_contains);
    printf("  #found      : %lu\n", data[i].nb_found);
    printf("  #aborts     : %lu\n", data[i].nb_aborts);
    printf("    #lock-r   : %lu\n", data[i].nb_aborts_locked_read);
    printf("    #lock-w   : %lu\n", data[i].nb_aborts_locked_write);
    printf("    #val-r    : %lu\n", data[i].nb_aborts_validate_read);
    printf("    #val-w    : %lu\n", data[i].nb_aborts_validate_write);
    printf("    #val-c    : %lu\n", data[i].nb_aborts_validate_commit);
    printf("    #inv-mem  : %lu\n", data[i].nb_aborts_invalid_memory);
    printf("    #dup-w    : %lu\n", data[i].nb_aborts_double_write);
    printf("    #failures : %lu\n", data[i].failures_because_contention);
    printf("  Max retries : %lu\n", data[i].max_retries);
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

  size_t size_after = sl_set_size(set);
  printf("Set size      : %lu (expected: %d)\n", size_after, size);
  assert(size_after == size);
  printf("Duration      : %d (ms)\n", duration);
  printf("#txs          : %lu (%f / s)\n", reads + updates, (reads + updates) * 1000.0 / duration);
	
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
	
  printf("#aborts       : %lu (%f / s)\n", aborts, aborts * 1000.0 / duration);
  printf("  #lock-r     : %lu (%f / s)\n", aborts_locked_read, aborts_locked_read * 1000.0 / duration);
  printf("  #lock-w     : %lu (%f / s)\n", aborts_locked_write, aborts_locked_write * 1000.0 / duration);
  printf("  #val-r      : %lu (%f / s)\n", aborts_validate_read, aborts_validate_read * 1000.0 / duration);
  printf("  #val-w      : %lu (%f / s)\n", aborts_validate_write, aborts_validate_write * 1000.0 / duration);
  printf("  #val-c      : %lu (%f / s)\n", aborts_validate_commit, aborts_validate_commit * 1000.0 / duration);
  printf("  #inv-mem    : %lu (%f / s)\n", aborts_invalid_memory, aborts_invalid_memory * 1000.0 / duration);
  printf("  #dup-w      : %lu (%f / s)\n", aborts_double_write, aborts_double_write * 1000.0 / duration);
  printf("  #failures   : %lu\n",  failures_because_contention);
  printf("Max retries   : %lu\n", max_retries);
	
  // Delete set 
  sl_set_delete(set);
	
  free(threads);
  free(data);
	
  return 0;
}

