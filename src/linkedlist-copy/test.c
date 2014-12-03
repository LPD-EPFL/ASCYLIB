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
int verbose = 0;

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
  unsigned long max_retries;
  unsigned int seed;
  intset_l_t *set;
  barrier_t *barrier;
  int id;
  uint8_t padding[16];
} thread_data_t;


/* extern __thread size_t num_pause, num_total, num_xtest; */

void*
test(void *data) 
{
  PF_MSG(0, "rand_range");
  PF_MSG(1, "malloc");
  PF_MSG(2, "free");

  int unext, last = -1; 
  skey_t val = 0;
	
  thread_data_t *d = (thread_data_t *)data;
  int transactional = d->unit_tx;

  set_cpu(the_cores[d->id]);

  ssalloc_init();
  PF_CORRECTION;

  seeds = seed_rand();

#if defined(HTICKET)
  init_thread_htlocks(the_cores[d->id]);
#elif defined(CLH)
  init_clh_thread(&clh_local_p);
#endif



  /* Wait on barrier */
  barrier_cross(d->barrier);
	
  /* Is the first op an update? */
  unext = (rand_range_re(&d->seed, 100) - 1 < d->update);
		
  /* while (stop == 0) */
  while (*running)
    {
      if (unext) { // update
				
	if (last < 0) { // add
					
	  val = rand_range_re(&d->seed, d->range);
	  if (set_add_l(d->set, val, val, TRANSACTIONAL)) {
	    d->nb_added++;
	    last = val;
	  } 				
	  d->nb_add++;
					
	} else { // remove
					
	  if (d->alternate) { // alternate mode
						
	    if (set_remove_l(d->set, last, TRANSACTIONAL)) {
	      d->nb_removed++;
	    }
	    last = -1;
						
	  } else {
					
	    val = rand_range_re(&d->seed, d->range);
	    if (set_remove_l(d->set, val, TRANSACTIONAL)) {
	      d->nb_removed++;
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
				
	if (set_contains_l(d->set, val, TRANSACTIONAL)) 
	  d->nb_found++;
	d->nb_contains++;			
      }
			
      /* Is the next op an update? */
      if (d->effective) { // a failed remove/add is a read-only tx
	unext = ((100 * (d->nb_added + d->nb_removed))
		 < (d->update * (d->nb_add + d->nb_remove + d->nb_contains)));
      } else { // remove/add (even failed) is considered an update
	unext = (rand_range_re(&d->seed, 100) - 1 < d->update);
      }
			
    }	
 
  PF_PRINT;
  /* printf("[%d] tot: %10lu / pause: %10lu / ratio: %5.2f / xtests: %lu\n", */
  /* 	 d->id, num_total, num_pause, 1.0*num_pause/num_total, num_xtest); */

  return NULL;
}

int
main(int argc, char **argv)
{
#if defined(CLH)
  init_clh_thread(&clh_local_p);
#endif

  set_cpu(the_cores[0]);
  ssalloc_init();
  seeds = seed_rand();

  struct option long_options[] = {
    // These options don't set a flag
    {"help",                      no_argument,       NULL, 'h'},
    {"verbose",                   no_argument,       NULL, 'v'},
    {"duration",                  required_argument, NULL, 'd'},
    {"initial-size",              required_argument, NULL, 'i'},
    {"num-threads",               required_argument, NULL, 'n'},
    {"range",                     required_argument, NULL, 'r'},
    {"seed",                      required_argument, NULL, 's'},
    {"update-rate",               required_argument, NULL, 'u'},
    {"load-factor",               required_argument, NULL, 'l'},
    {"unit-tx",                   required_argument, NULL, 'x'},
    {NULL, 0, NULL, 0}
  };
	
  intset_l_t *set;
  int i, c, size;
  skey_t last = 0; 
  skey_t val = 0;
  unsigned long reads, effreads, updates, effupds;
#if defined(STM)
  unsigned long aborts, aborts_locked_read, aborts_locked_write,
    aborts_validate_read, aborts_validate_write, aborts_validate_commit,
    aborts_invalid_memory, max_retries;
#endif 
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
  int unit_tx = DEFAULT_LOCKTYPE;
  int alternate = DEFAULT_ALTERNATE;
  int effective = DEFAULT_EFFECTIVE;
  sigset_t block_set;
	
  while(1) {
    i = 0;
    c = getopt_long(argc, argv, "hvAf:d:i:n:r:s:u:l:x:"
		    ,long_options, &i);
		
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
	     "  -v, --verbose\n"
	     "        Print detailed (per core) results\n"
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
	     "  -s, --seed <int>\n"
	     "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
	     "  -u, --update-rate <int>\n"
	     "        Percentage of update transactions (default=" XSTR(DEFAULT_UPDATE) ")\n"
	     "  -x, --lock-based algorithm (default=1)\n"
	     "        Use lock-based algorithm\n"
	     "        1 = lock-coupling,\n"
	     "        2 = lazy algorithm\n"
	     , argv[0]);
      exit(0);
    case 'A':
      alternate = 1;
      break;
    case 'v':
      verbose = 1;
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
    case 'l':
      break;
    case 'x':
      unit_tx = atoi(optarg);
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
  assert(range > 0 && range >= initial);
  assert(update >= 0 && update <= 100);
	
  printf("Set type     : linked list\n");
  printf("Duration     : %d\n", duration);
  printf("Initial size : %d\n", initial);
  printf("Nb threads   : %d\n", nb_threads);
  printf("Value range  : %ld\n", range);
  printf("Seed         : %d\n", seed);
  printf("Update rate  : %d\n", update);
  printf("Lock alg     : %d\n", unit_tx);
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
	
  if (seed == 0)
    srand((int)time(0));
  else
    srand(seed);
	
  set = set_new_l();
	
  /* stop = 0; */
  *running = 1;
	
  /* printf("Initializing STM\n"); */
	
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
	  if (set_add_l(set, val, val, 0)) 
	    {
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
	  set_add_l(set, i, i, 0);
	}
    }
  printf("\n");
  size = set_size_l(set);
  printf("Set size     : %d\n", size);
	
  /* Access set from all threads */
  barrier_init(&barrier, nb_threads + 1);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  printf("Creating threads: ");
  for (i = 0; i < nb_threads; i++)
    {
      printf("%d,", i);
      data[i].first = last;
      data[i].range = range;
      data[i].update = update;
      data[i].alternate = alternate;
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
      data[i].max_retries = 0;
      data[i].seed = rand();
      data[i].set = set;
      data[i].barrier = &barrier;
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
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error waiting for thread completion\n");
      exit(1);
    }
  }
	
  duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
#if defined(STM)
  aborts = 0;
  aborts_locked_read = 0;
  aborts_locked_write = 0;
  aborts_validate_read = 0;
  aborts_validate_write = 0;
  aborts_validate_commit = 0;
  aborts_invalid_memory = 0;
  max_retries = 0;
#endif
  reads = 0;
  effreads = 0;
  updates = 0;
  effupds = 0;
  for (i = 0; i < nb_threads; i++)
    {
      if (verbose)
	{
	  printf("Thread %d\n", i);
	  printf("  #add        : %lu\n", data[i].nb_add);
	  printf("    #added    : %lu\n", data[i].nb_added);
	  printf("  #remove     : %lu\n", data[i].nb_remove);
	  printf("    #removed  : %lu\n", data[i].nb_removed);
	  printf("  #contains   : %lu\n", data[i].nb_contains);
	  printf("  #found      : %lu\n", data[i].nb_found);
	}
#if defined(STM)
      if (verbose)
	{
	  printf("  #aborts     : %lu\n", data[i].nb_aborts);
	  printf("    #lock-r   : %lu\n", data[i].nb_aborts_locked_read);
	  printf("    #lock-w   : %lu\n", data[i].nb_aborts_locked_write);
	  printf("    #val-r    : %lu\n", data[i].nb_aborts_validate_read);
	  printf("    #val-w    : %lu\n", data[i].nb_aborts_validate_write);
	  printf("    #val-c    : %lu\n", data[i].nb_aborts_validate_commit);
	  printf("    #inv-mem  : %lu\n", data[i].nb_aborts_invalid_memory);
	  printf("  Max retries : %lu\n", data[i].max_retries);
	}
      aborts += data[i].nb_aborts;
      aborts_locked_read += data[i].nb_aborts_locked_read;
      aborts_locked_write += data[i].nb_aborts_locked_write;
      aborts_validate_read += data[i].nb_aborts_validate_read;
      aborts_validate_write += data[i].nb_aborts_validate_write;
      aborts_validate_commit += data[i].nb_aborts_validate_commit;
      aborts_invalid_memory += data[i].nb_aborts_invalid_memory;
      if (max_retries < data[i].max_retries)
	max_retries = data[i].max_retries;
#endif
      reads += data[i].nb_contains;
      effreads += data[i].nb_contains + 
	(data[i].nb_add - data[i].nb_added) + 
	(data[i].nb_remove - data[i].nb_removed); 
      updates += (data[i].nb_add + data[i].nb_remove);
      effupds += data[i].nb_removed + data[i].nb_added; 
		
      //size += data[i].diff;
      size += data[i].nb_added - data[i].nb_removed;
    }
  int size_after = set_size_l(set);
  printf("Set size      : %d (expected: %d)\n", size_after, size);
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
	
#if defined(STM)
  printf("#aborts       : %lu (%f / s)\n", aborts, aborts * 1000.0 / duration);
  printf("  #lock-r     : %lu (%f / s)\n", aborts_locked_read, aborts_locked_read * 1000.0 / duration);
  printf("  #lock-w     : %lu (%f / s)\n", aborts_locked_write, aborts_locked_write * 1000.0 / duration);
  printf("  #val-r      : %lu (%f / s)\n", aborts_validate_read, aborts_validate_read * 1000.0 / duration);
  printf("  #val-w      : %lu (%f / s)\n", aborts_validate_write, aborts_validate_write * 1000.0 / duration);
  printf("  #val-c      : %lu (%f / s)\n", aborts_validate_commit, aborts_validate_commit * 1000.0 / duration);
  printf("  #inv-mem    : %lu (%f / s)\n", aborts_invalid_memory, aborts_invalid_memory * 1000.0 / duration);
  printf("Max retries   : %lu\n", max_retries);
#endif 

  /* Delete set */
  set_delete_l(set);
	
  free(threads);
  free(data);
	
  return 0;
}
