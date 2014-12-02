/* this file is taken from libslock (https://github.com/tudordavid/libslock) */

/*
 * File: htlock.c
 * Author:  Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *
 * Description: an numa-aware hierarchical ticket lock
 *  The htlock contains N local ticket locks (N = number of memory
 *  nodes) and 1 global ticket lock. A thread always tries to acquire
 *  the local ticket lock first. If there isn't any (local) available,
 *  it enqueues for acquiring the global ticket lock and at the same
 *  time it "gives" NB_TICKETS_LOCAL tickets to the local ticket lock, 
 *  so that if more threads from the same socket try to acquire the lock,
 *  they will enqueue on the local lock, without even accessing the
 *  global one.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Vasileios Trigonakis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "htlock.h"

__thread uint32_t my_node, my_id;

htlock_t* 
create_htlock()
{
  htlock_t* htl;
#ifdef __sparc__
  htl = memalign(CACHE_LINE_SIZE, sizeof(htlock_t));
  if (htl==NULL) 
    {
      fprintf(stderr,"Error @ memalign : create htlock\n");
    }
#else
  if (posix_memalign((void**) &htl, CACHE_LINE_SIZE, sizeof(htlock_t)) < 0)
    {
      fprintf(stderr, "Error @ posix_memalign : create_htlock\n");
    }
#endif    
  assert(htl != NULL);

#ifdef __sparc__
  htl->global = memalign(CACHE_LINE_SIZE, sizeof(htlock_global_t));
  if (htl==NULL) 
    {
      fprintf(stderr,"Error @ memalign : create htlock\n");
    }
#else
  if (posix_memalign((void**) &htl->global, CACHE_LINE_SIZE, sizeof(htlock_global_t)) < 0)
    {
      fprintf(stderr, "Error @ posix_memalign : create_htlock\n");
    }
#endif    
  assert(htl->global != NULL);


  uint32_t s;
  for (s = 0; s < NUMBER_OF_SOCKETS; s++)
    {
#if defined(__x86_64__) && defined(PLATFORM_NUMA)
      numa_set_preferred(s);
      htl->local[s] = (htlock_local_t*) numa_alloc_onnode(sizeof(htlock_local_t), s);
#else
      htl->local[s] = (htlock_local_t*) malloc(sizeof(htlock_local_t));
#endif
      assert(htl->local != NULL);
    }

#if defined(__x86_64__) && defined(PLATFORM_NUMA)
  numa_set_preferred(my_node);
#endif

  htl->global->cur = 0;
  htl->global->nxt = 0;
  uint32_t n;
  for (n = 0; n < NUMBER_OF_SOCKETS; n++)
    {
      htl->local[n]->cur = NB_TICKETS_LOCAL;
      htl->local[n]->nxt = 0;
    }

  return htl;
}


void
init_htlock(htlock_t* htl)
{
  assert(htl != NULL);
  htl->global->cur = 0;
  htl->global->nxt = 0;
  uint32_t n;
  for (n = 0; n < NUMBER_OF_SOCKETS; n++)
    {
      htl->local[n]->cur = NB_TICKETS_LOCAL;
      htl->local[n]->nxt = 0;
    }
}

void
init_alloc_htlock(htlock_t* htl)
{
  assert(htl != NULL);

#ifdef __sparc__
  htl->global = memalign(CACHE_LINE_SIZE, sizeof(htlock_global_t));
  if (htl==NULL) 
    {
      fprintf(stderr,"Error @ memalign : create htlock\n");
    }
#else
  if (posix_memalign((void**) &htl->global, CACHE_LINE_SIZE, sizeof(htlock_global_t)) < 0)
    {
      fprintf(stderr, "Error @ posix_memalign : create_htlock\n");
    }
#endif    
  assert(htl->global != NULL);


  uint32_t s;
  for (s = 0; s < NUMBER_OF_SOCKETS; s++)
    {
#if defined(__x86_64__) && defined(PLATFORM_NUMA)
      numa_set_preferred(s);
      htl->local[s] = (htlock_local_t*) numa_alloc_onnode(sizeof(htlock_local_t), s);
#else
      htl->local[s] = (htlock_local_t*) malloc(sizeof(htlock_local_t));
#endif
      assert(htl->local != NULL);
    }

#if defined(__x86_64__) && defined(PLATFORM_NUMA)
  numa_set_preferred(my_node);
#endif

  htl->global->cur = 0;
  htl->global->nxt = 0;
  uint32_t n;
  for (n = 0; n < NUMBER_OF_SOCKETS; n++)
    {
      htl->local[n]->cur = NB_TICKETS_LOCAL;
      htl->local[n]->nxt = 0;
    }

}


void
init_thread_htlocks(uint32_t phys_core)
{
  set_cpu(phys_core);

#ifdef XEON
  __sync_synchronize();
  uint32_t real_core_num = 0;
  uint32_t i;
  for (i = 0; i < (NUMBER_OF_SOCKETS * CORES_PER_SOCKET); i++) 
    {
      if (the_cores[i]==phys_core) 
	{
	  real_core_num = i;
	  break;
	}
    }
  MEM_BARRIER;
  my_id = real_core_num;
  my_node = get_cluster(phys_core);
#else
  my_id = phys_core;
  my_node = get_cluster(phys_core);
#endif
  /* printf("id %3d / phys_core %3d / node %d \n", my_id, phys_core, my_node); */
}

uint32_t
is_free_hticket(htlock_t* htl)
{
  htlock_global_t* glb = htl->global;
#if defined(OPTERON_OPTIMIZE)
  PREFETCHW(glb);
#endif
  if (glb->cur == glb->nxt) 
    {
      return 1;
    }
  return 0;
}

static htlock_t* 
create_htlock_no_alloc(htlock_t* htl, htlock_local_t* locals[NUMBER_OF_SOCKETS], size_t offset)
{
#ifdef __sparc__
  htl->global = memalign(CACHE_LINE_SIZE, sizeof(htlock_global_t));
  if (htl==NULL) 
    {
      fprintf(stderr,"Error @ memalign : create htlock\n");
    }
#else
  if (posix_memalign((void**) &htl->global, CACHE_LINE_SIZE, sizeof(htlock_global_t)) < 0)
    {
      fprintf(stderr, "Error @ posix_memalign : create_htlock\n");
    }
#endif    
  assert(htl->global != NULL);


  uint32_t s;
  for (s = 0; s < NUMBER_OF_SOCKETS; s++)
    {
      htl->local[s] = locals[s] + offset;
    }


  htl->global->cur = 0;
  htl->global->nxt = 0;
  uint32_t n;
  for (n = 0; n < NUMBER_OF_SOCKETS; n++)
    {
      htl->local[n]->cur = NB_TICKETS_LOCAL;
      htl->local[n]->nxt = 0;
    }

  return htl;
}

htlock_t*
init_htlocks(uint32_t num_locks)
{
  htlock_t* htls;
#ifdef __sparc__
    htls = memalign(CACHE_LINE_SIZE, num_locks * sizeof(htlock_t));
    if (htls==NULL) {
      fprintf(stderr, "Error @ memalign : init_htlocks\n");
    }
#else
  if (posix_memalign((void**) &htls, 64, num_locks * sizeof(htlock_t)) < 0)
    {
      fprintf(stderr, "Error @ posix_memalign : init_htlocks\n");
    }
#endif   
  assert(htls != NULL);


  size_t alloc_locks = (num_locks < 64) ? 64 : num_locks;

  htlock_local_t* locals[NUMBER_OF_SOCKETS];
  uint32_t n;
  for (n = 0; n < NUMBER_OF_SOCKETS; n++)
    {
#if defined(__x86_64__) && defined(PLATFORM_NUMA)
      numa_set_preferred(n);
      _mm_mfence();
#endif
      locals[n] = (htlock_local_t*) calloc(alloc_locks, sizeof(htlock_local_t));
      //numa_alloc_onnode(num_locks * sizeof(htlock_local_t), n);
      assert(locals[n] != NULL);
    }

#if defined(__x86_64__) && defined(PLATFORM_NUMA)
  numa_set_preferred(my_node);
#endif

  uint32_t i;
  for (i = 0; i < num_locks; i++)
    {
      create_htlock_no_alloc(htls + i, locals, i);
    }

  return htls;
}


void 
free_htlocks(htlock_t* locks)
{
  free(locks);
}

static inline uint32_t
sub_abs(const uint32_t a, const uint32_t b)
{
  if (a > b)
    {
      return a - b;
    }
  else
    {
      return b - a;
    }
}


#define TICKET_BASE_WAIT 512
#define TICKET_MAX_WAIT  4095
#define TICKET_WAIT_NEXT 64


static inline void
htlock_wait_ticket(htlock_local_t* lock, const uint32_t ticket)
{

#if defined(OPTERON_OPTIMIZE)
  uint32_t wait = TICKET_BASE_WAIT;
  uint32_t distance_prev = 1;

  while (1)
    {
      PREFETCHW(lock);
      int32_t lock_cur = lock->cur;
      if (lock_cur == ticket)
	{
	  break;
	}
      uint32_t distance = sub_abs(lock->cur, ticket);
      if (distance > 1)
	{
	  if (distance != distance_prev)
	    {
	      distance_prev = distance;
	      wait = TICKET_BASE_WAIT;
	    }

	  nop_rep(distance * wait);
	  wait = (wait + TICKET_BASE_WAIT) & TICKET_MAX_WAIT;
	}
      else
	{
	  nop_rep(TICKET_WAIT_NEXT);
	}
    }  
#else
  while (lock->cur != ticket)
    {
      uint32_t distance = sub_abs(lock->cur, ticket);
      if (distance > 1)
      	{
      	  nop_rep(distance * TICKET_BASE_WAIT);
      	}
      else
	{
	  PAUSE;
	}
    }
#endif	/* OPTERON_OPTIMIZE */
}

static inline void
htlock_wait_global(htlock_local_t* lock, const uint32_t ticket)
{
  while (lock->cur != ticket)
    {
      uint32_t distance = sub_abs(lock->cur, ticket);
      if (distance > 1)
      	{
	  wait_cycles(distance * 256);
      	}
      else
	{
	  PAUSE;
	}
    }
}

void
htlock_lock(htlock_t* l)
{
  htlock_local_t* localp = l->local[my_node];
  int32_t local_ticket;

  /* _mm_clflush(localp); */

 again_local:
  /* if (my_id == 32) */
  /*   { */
  /*     s = getticks(); */
  /*   } */
  local_ticket = DAF_U32((volatile uint32_t*) &localp->nxt);
  /* if (my_id == 32) */
  /*   { */
  /*     e = getticks(); */
  /*     printf("%5llu - ", e - s - 74); */
  /*   } */
  /* only the guy which gets local_ticket == -1 is allowed to share tickets */
  if (local_ticket < -1)	
    {
      PAUSE;
      wait_cycles(-local_ticket * 120);
      PAUSE;
      goto again_local;
    }

  if (local_ticket >= 0)	/* local grabing successful */
    {
      htlock_wait_ticket((htlock_local_t*) localp, local_ticket);
    }
  else				/* no local ticket available */
    {
      do
	{
#if defined(OPTERON_OPTIMIZE)
	  PREFETCHW(localp);
#endif
	} while (localp->cur != NB_TICKETS_LOCAL);
      localp->nxt = NB_TICKETS_LOCAL; /* give tickets to the local neighbors */

      htlock_global_t* globalp = l->global;
      uint32_t global_ticket = FAI_U32(&globalp->nxt);

      /* htlock_wait_ticket((htlock_local_t*) globalp, global_ticket); */
      htlock_wait_global((htlock_local_t*) globalp, global_ticket);
    }
}

void
htlock_release(htlock_t* l)
{
  htlock_local_t* localp = l->local[my_node];
#if defined(OPTERON_OPTIMIZE)
  PREFETCHW(localp);
#endif
  int32_t local_cur = localp->cur;
  int32_t local_nxt = CAS_U32((volatile uint32_t*) &localp->nxt, local_cur, 0);
  if (local_cur == 0 || local_cur == local_nxt) /* global */
    {
#if defined(OPTERON_OPTIMIZE)
      PREFETCHW((l->global));
      PREFETCHW(localp);
#endif
      localp->cur = NB_TICKETS_LOCAL;
      l->global->cur++;
    }
  else				/* local */
    {
#if defined(OPTERON_OPTIMIZE)
      PREFETCHW(localp);
#endif
      localp->cur = local_cur - 1;
    }
}

uint32_t 
htlock_trylock(htlock_t* l)
{
  htlock_global_t* globalp = l->global;
  PREFETCHW(globalp);  
  uint32_t global_nxt = globalp->nxt;

  htlock_global_t tmp = 
    {
      .nxt = global_nxt, 
      .cur = global_nxt
    };
  htlock_global_t tmp_new = 
    {
      .nxt = global_nxt + 1, 
      .cur = global_nxt
    };

  uint64_t tmp64 = *(uint64_t*) &tmp;
  uint64_t tmp_new64 = *(uint64_t*) &tmp_new;

  if (CAS_U64((uint64_t*) globalp, tmp64, tmp_new64) == tmp64)
    {
      return 1;
    }

  return 0;
}


inline void
htlock_release_try(htlock_t* l)	/* trylock rls */
{
  PREFETCHW((l->global));
  l->global->cur++;
}

