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

/*
 * File:
 *   test.c
 * Author(s):
 * Description:
 *   Concurrent accesses of a hashtable
 *
 * Copyright (c) 2009-2010.
 *
 * test.c is part of HIDDEN
 * 
 * HIDDEN is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "intset.h"

/* Hashtable length (# of buckets) */
unsigned int maxhtlength;
__thread unsigned long* seeds;

ALIGNED(64) uint8_t running[64];

/* Hashtable seed */
#ifdef TLS
__thread unsigned int *rng_seed;
#else /* ! TLS */
pthread_key_t rng_seed_key;
#endif /* ! TLS */

/* 
 * Returns a pseudo-random value in [1;range).
 * Depending on the symbolic constant RAND_MAX>=32767 defined in stdlib.h,
 * the granularity of rand() could be lower-bounded by the 32767^th which might 
 * be too high for given values of range and initial.
 */

typedef ALIGNED(64) struct thread_data 
{
  skey_t first;
  long range;
  int update;
  int move;
  int snapshot;
  int unit_tx;
  int alternate;
  int effective;
  unsigned long nb_add;
  unsigned long nb_added;
  unsigned long nb_remove;
  unsigned long nb_removed;
  unsigned long nb_contains;
  unsigned long load_factor;	/* added for HashTables */
  unsigned long nb_move;
  unsigned long nb_moved;
  unsigned long nb_snapshot;
  unsigned long nb_snapshoted;	/* end: added for HashTables */
  unsigned long nb_found;
  unsigned int seed;
  ht_intset_t *set;
  barrier_t *barrier;
  unsigned long failures_because_contention;
  int id;
  uint8_t padding[16];
} thread_data_t;


void
*test(void *data) 
{
  PF_MSG(0, "rand_range");
  PF_MSG(1, "malloc");
  PF_MSG(2, "free");

  int val2, numtx, r, last = -1;
  skey_t key = 0;
  int unext = 0 , mnext = 0, cnext = 1;
	
  thread_data_t *d = (thread_data_t *)data;
	
  set_cpu(d->id);
  ssalloc_init();
  PF_CORRECTION;

  seeds = seed_rand();

  /* Wait on barrier */
  barrier_cross(d->barrier);
	
  while (*running)
    {
      if (unext) { // update
	    
	if (mnext) { // move
	      
	  if (last == -1) key = rand_range_re(&d->seed, d->range);
	  else key = last;
	  val2 = rand_range_re(&d->seed, d->range);
	  if (ht_move(d->set, key, val2, TRANSACTIONAL)) {
	    d->nb_moved++;
	    last = -1;
	  }
	  d->nb_move++;
	      
	} else if (last < 0) { // add
	      
	  key = rand_range_re(&d->seed, d->range);
	  if (ht_add(d->set, key, TRANSACTIONAL)) {
	    d->nb_added++;
	    last = key;
	  }
	  d->nb_add++;
	      
	} else { // remove
	      
	  if (d->alternate) { // alternate mode
	    if (ht_remove(d->set, last)) {
	      d->nb_removed++;
	      last = -1;
	    }
	  } else {
	    /* Random computation only in non-alternated cases */
	    key = rand_range_re(&d->seed, d->range);
	    /* Remove one random value */
	    if (ht_remove(d->set, key)) {
	      d->nb_removed++;
	      /* Repeat until successful, to avoid size variations */
	      last = -1;
	    }
	  }
	  d->nb_remove++;
	}
	    
      } else { // reads
	    
	if (cnext) { // contains (no snapshot)
				
	  if (d->alternate) {
	    if (d->update == 0) {
	      if (last < 0) {
		key = d->first;
		last = key;
	      } else { // last >= 0
		key = rand_range_re(&d->seed, d->range);
		last = -1;
	      }
	    } else { // update != 0
	      if (last < 0) {
		key = rand_range_re(&d->seed, d->range);
		//last = key;
	      } else {
		key = last;
	      }
	    }
	  }	else key = rand_range_re(&d->seed, d->range);
				
	  if (ht_contains(d->set, key))
	    d->nb_found++;
	  d->nb_contains++;
	} else { // snapshot
	      
	  if (ht_snapshot(d->set, TRANSACTIONAL))
	    d->nb_snapshoted++;
	  d->nb_snapshot++;
	      
	}
      }
	  
      /* Is the next op an update, a move, a contains? */
      if (d->effective) { // a failed remove/add is a read-only tx
	numtx = d->nb_contains + d->nb_add + d->nb_remove + d->nb_move + d->nb_snapshot;
	unext = ((100.0 * (d->nb_added + d->nb_removed + d->nb_moved)) < (d->update * numtx));
	mnext = ((100.0 * d->nb_moved) < (d->move * numtx));
	cnext = !((100.0 * d->nb_snapshoted) < (d->snapshot * numtx));
      } else { // remove/add (even failed) is considered as an update
	r = rand_range_re(&d->seed, 100) - 1;
	unext = (r < d->update);
	mnext = (r < d->move);
	cnext = (r >= d->update + d->snapshot);
      }
    }


  PF_PRINT;
	
  return NULL;
}


void *test2(void *data)
{
  int val, newval, last, flag = 1;
  thread_data_t *d = (thread_data_t *)data;
	
  set_cpu(d->id);
  /* Wait on barrier */
  barrier_cross(d->barrier);
	
  last = 0; // to avoid warning
  while (stop == 0) {
		
    val = rand_range_re(&d->seed, 100) - 1;
    /* added for HashTables */
    if (val < d->update) {
      if (val >= d->move) { /* update without move */
	if (flag) {
	  /* Add random value */
	  val = (rand_r(&d->seed) % d->range) + 1;
	  if (ht_add(d->set, val, TRANSACTIONAL)) {
	    d->nb_added++;
	    last = val;
	    flag = 0;
	  }
	  d->nb_add++;
	} else {
	  if (d->alternate) {
	    /* Remove last value */
	    if (ht_remove(d->set, last))  
	      d->nb_removed++;
	    d->nb_remove++;
	    flag = 1;
	  } else {
	    /* Random computation only in non-alternated cases */
	    newval = rand_range_re(&d->seed, d->range);
	    if (ht_remove(d->set, newval)) {  
	      d->nb_removed++;
	      /* Repeat until successful, to avoid size variations */
	      flag = 1;
	    }
	    d->nb_remove++;
	  }
	} 
      } else { /* move */
	val = rand_range_re(&d->seed, d->range);
	if (ht_move(d->set, last, val, TRANSACTIONAL)) {
	  d->nb_moved++;
	  last = val;
	}
	d->nb_move++;
      }
    } else {
      if (val >= d->update + d->snapshot) { /* read-only without snapshot */
	/* Look for random value */
	val = rand_range_re(&d->seed, d->range);
	if (ht_contains(d->set, val))
	  d->nb_found++;
	d->nb_contains++;
      } else { /* snapshot */
	if (ht_snapshot(d->set, TRANSACTIONAL))
	  d->nb_snapshoted++;
	d->nb_snapshot++;
      }
    }
  }
	
  return NULL;
}

void print_set(intset_t *set) {
	node_t *curr, *tmp;
	
	curr = set->head;
	tmp = curr;
	do {
		printf(" - v%d", (int) curr->key);
		tmp = curr;
		curr = tmp->next;
	} while (curr->key != KEY_MAX);
	printf(" - v%d", (int) curr->key);
	printf("\n");
}

void print_ht(ht_intset_t *set) {
  int i;
  for (i=0; i < *maxhtlength; i++) 
    {
      print_set(set->buckets[i]);
    }
}

int test_verbose = 0;

int
main(int argc, char **argv)
{
  set_cpu(0);
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
    {"move-rate",                 required_argument, NULL, 'm'},
    {"snapshot-rate",             required_argument, NULL, 'a'},
    {"elasticity",                required_argument, NULL, 'x'},
    {NULL, 0, NULL, 0}
  };
	
  ht_intset_t *set;
  int i, c, size;
  skey_t last = 0; 
  skey_t key = 0;
  unsigned long reads, effreads, updates, effupds, moves, moved, snapshots, snapshoted;

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
  int update = DEFAULT_UPDATE;
  int load_factor = DEFAULT_LOAD;
  int move = DEFAULT_MOVE;
  int snapshot = DEFAULT_SNAPSHOT;
  int unit_tx = DEFAULT_ELASTICITY;
  int alternate = DEFAULT_ALTERNATE;
  int effective = DEFAULT_EFFECTIVE;
  sigset_t block_set;
	
  while(1) 
    {
      i = 0;
      c = getopt_long(argc, argv, "hvAf:d:i:n:r:s:u:m:a:l:x:", long_options, &i);
		
      if(c == -1)
	break;
		
      if(c == 0 && long_options[i].flag == 0)
	c = long_options[i].val;
		
      switch(c) 
	{
	case 0:
	  // Flag is automatically set 
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
		 "  -m , --move-rate <int>\n"
		 "        Percentage of move transactions (default=" XSTR(DEFAULT_MOVE) ")\n"
		 "  -a , --snapshot-rate <int>\n"
		 "        Percentage of snapshot transactions (default=" XSTR(DEFAULT_SNAPSHOT) ")\n"
		 "  -l , --load-factor <int>\n"
		 "        Ratio of keys over buckets (default=" XSTR(DEFAULT_LOAD) ")\n"
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
	case 'm':
	  move = atoi(optarg);
	  break;
	case 'a':
	  snapshot = atoi(optarg);
	  break;
	case 'l':
	  load_factor = atoi(optarg);
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
  assert(range > 0);
  assert(update >= 0 && update <= 100);
  assert(move >= 0 && move <= update);
  assert(snapshot >= 0 && snapshot <= (100-update));
  assert(initial < MAXHTLENGTH);
  assert(initial >= load_factor);

  if (range < initial)
    {
      range = 2 * initial;
    }
	
  if (test_verbose)
    {
      printf("Set type     : hash table\n");
      printf("Duration     : %d\n", duration);
      printf("Initial size : %d\n", initial);
      printf("Nb threads   : %d\n", nb_threads);
      printf("Value range  : %ld\n", range);
      printf("Update rate  : %d\n", update);
      printf("Load factor  : %d\n", load_factor);
      printf("Move rate    : %d\n", move);
      printf("Snapshot rate: %d\n", snapshot);
      printf("Elasticity   : %d\n", unit_tx);
      printf("Alternate    : %d\n", alternate);	
      printf("Effective    : %d\n", effective);
      printf("Type sizes   : int=%d/long=%d/ptr=%d/word=%d\n",
	     (int)sizeof(int),
	     (int)sizeof(long),
	     (int)sizeof(void *),
	     (int)sizeof(uintptr_t));
    }
	
  timeout.tv_sec = duration / 1000;
  timeout.tv_nsec = (duration % 1000) * 1000000;
	
  if ((data = (thread_data_t *)memalign(64, nb_threads * sizeof(thread_data_t))) == NULL) 
    {
      perror("malloc");
      exit(1);
    }
  if ((threads = (pthread_t *)malloc(nb_threads * sizeof(pthread_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
	
  srand((int)time(0));
  maxhtlength = (unsigned int*) ssalloc(64);//memalign(64, 64);
  assert(maxhtlength != NULL);

  *maxhtlength = (unsigned int) initial / load_factor;
  set = ht_new();
	
  /* stop = 0; */
  *running = 1;
	
  // Init STM 
  if (test_verbose)
    {
      printf("Initializing STM\n");
    }	
	
  // Populate set 
  if (test_verbose)
    {
      printf("Adding %d entries to set\n", initial);
    }

  i = 0;
  while (i < initial) 
    {
      key = rand_range(range);
      if (ht_add(set, key, key)) 
	{
	  last = key;
	  i++;			
	}
    }

  if (test_verbose)
    {
      printf("Added %d entries to set\n", initial);
    }

  size = ht_size(set);
  if (test_verbose)
    {
      printf("Set size     : %d\n", size);
      printf("Bucket amount: %d\n", *maxhtlength);
      printf("Load         : %d\n", load_factor);
    }	

  // Access set from all threads 
  barrier_init(&barrier, nb_threads + 1);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  printf("Creating threads: ");
  for (i = 0; i < nb_threads; i++) 
    {
      printf("%2d, ", i);
      data[i].first = last;
      data[i].range = range;
      data[i].update = update;
      data[i].load_factor = load_factor;
      data[i].move = move;
      data[i].snapshot = snapshot;
      data[i].unit_tx = unit_tx;
      data[i].alternate = alternate;
      data[i].effective = effective;
      data[i].nb_add = 0;
      data[i].nb_added = 0;
      data[i].nb_remove = 0;
      data[i].nb_removed = 0;
      data[i].nb_move = 0;
      data[i].nb_moved = 0;
      data[i].nb_snapshot = 0;
      data[i].nb_snapshoted = 0;
      data[i].nb_contains = 0;
      data[i].nb_found = 0;
      data[i].seed = rand();
      data[i].set = set;
      data[i].barrier = &barrier;
      data[i].failures_because_contention = 0;
      data[i].id = i;

      if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) 
	{
	  fprintf(stderr, "Error creating thread\n");
	  exit(1);
	}
    }
  printf("\n");
  pthread_attr_destroy(&attr);
	
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
  //print_ht(set);
	
  // Wait for thread completion 
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error waiting for thread completion\n");
      exit(1);
    }
  }

  duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
  reads = 0;
  effreads = 0;
  updates = 0;
  effupds = 0;
  moves = 0;
  moved = 0;
  snapshots = 0;
  snapshoted = 0;
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
	  printf("  #move       : %lu\n", data[i].nb_move);
	  printf("  #moved      : %lu\n", data[i].nb_moved);
	  printf("  #snapshot   : %lu\n", data[i].nb_snapshot);
	  printf("  #snapshoted : %lu\n", data[i].nb_snapshoted);
	}
      reads += data[i].nb_contains;
      effreads += data[i].nb_contains + 
	(data[i].nb_add - data[i].nb_added) + 
	(data[i].nb_remove - data[i].nb_removed) + 
	(data[i].nb_move - data[i].nb_moved) +
	data[i].nb_snapshoted;
      updates += (data[i].nb_add + data[i].nb_remove + data[i].nb_move);
      effupds += data[i].nb_removed + data[i].nb_added + data[i].nb_moved; 
      moves += data[i].nb_move;
      moved += data[i].nb_moved;
      snapshots += data[i].nb_snapshot;
      snapshoted += data[i].nb_snapshoted;
      size += data[i].nb_added - data[i].nb_removed;
    }
  printf("Set size      : %d (expected: %d)\n", ht_size(set), size);
  printf("Duration      : %d (ms)\n", duration);
  printf("#txs          : %lu (%f / s)\n", reads + updates + snapshots, (reads + updates + snapshots) * 1000.0 / duration);
	
  if (test_verbose)
    {
      printf("#read txs     : ");
      if (effective) {
	printf("%lu (%f / s)\n", effreads, effreads * 1000.0 / duration);
	printf("  #cont/snpsht: %lu (%f / s)\n", reads, reads * 1000.0 / duration);
      } else printf("%lu (%f / s)\n", reads, reads * 1000.0 / duration);
	
      printf("#eff. upd rate: %f \n", 100.0 * effupds / (effupds + effreads));
	
      printf("#update txs   : ");
      if (effective) {
	printf("%lu (%f / s)\n", effupds, effupds * 1000.0 / duration);
	printf("  #upd trials : %lu (%f / s)\n", updates, updates * 1000.0 / 
	       duration);
      } else printf("%lu (%f / s)\n", updates, updates * 1000.0 / duration);
	
      printf("#move txs     : %lu (%f / s)\n", moves, moves * 1000.0 / duration);
      printf("  #moved      : %lu (%f / s)\n", moved, moved * 1000.0 / duration);
      printf("#snapshot txs : %lu (%f / s)\n", snapshots, snapshots * 1000.0 / duration);
      printf("  #snapshoted : %lu (%f / s)\n", snapshoted, snapshoted * 1000.0 / duration);
    }	
  // Delete set 
  /* ht_delete(set); */
	
  free(threads);
  free(data);
	
  return 0;
}
