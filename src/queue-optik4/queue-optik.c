/*   
 *   File: optik.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description:  
 *   optik.c is part of ASCYLIB
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
 * 	     	      Tudor David <tudor.david@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * ASCYLIB is free software: you can redistribute it and/or
 * modify it under the teroptik of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "queue-optik.h"
#include "utils.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

static __thread rcu_node_t* __rcu_node = NULL;

static inline void
queue_rcu_op_on(queue_t* qu)
{
  if (unlikely(__rcu_node == NULL))
    {
      __rcu_node = memalign(CACHE_LINE_SIZE, sizeof(rcu_node_t));
      assert(__rcu_node != NULL);
      __rcu_node->next = SWAP_PTR(&qu->threads, __rcu_node);
    }
  __rcu_node->in_op = 1;
}

static inline void
queue_rcu_op_off()
{
  __rcu_node->in_op = 0;
}

static void
queue_rcu_wait_others(queue_t* qu)
{
  /* unsigned on[16] = {0}; */
  int num_on;
  do
    {
      num_on = 0;
      rcu_node_t* node;
      node = qu->threads;
      /* int o = 0; */
      while (node != NULL)
	{
	  num_on += node->in_op;
	  node = node->next;
	}
    }
  while (num_on > 1);
}

static inline int 
queue_is_empty(const size_t tail, const size_t head, const size_t hash)
{
  return (head >= tail) && ((tail - head) < hash);
}

static inline int 
queue_is_full(const size_t tail, const size_t head, const size_t hash)
{
  return ((tail - head) > (hash-1));
}

static inline int
queue_node_is_null(queue_node_t* node)
{
  return ((uintptr_t) node & 0x1);
}

static inline uintptr_t
queue_node_set_null(size_t node)
{
  return ((uintptr_t) node | 0x1);
}

static int
queue_optik_resize(queue_t* qu, size_t* tail_new)
{
  if (optik_trylock(&qu->lock))
    {
      queue_rcu_wait_others(qu);
      size_t size_new = qu->size << 1;
      size_t hash_new = size_new - 1;
      queue_node_t** array_new = (queue_node_t**) memalign(CACHE_LINE_SIZE, size_new * sizeof(queue_node_t*));
      assert(array_new != NULL);

      size_t hash = qu->hash;
      size_t head = qu->head_n & hash;
      int i, tot = 1;
      for (i = head + 1; i < (qu->size + head + 1); i++)
	{
	  queue_node_t* node = qu->array[i & hash];
	  if (!queue_node_is_null(node))
	    {
	      node->version = queue_node_set_null((tot - 1) & (hash_new));
	      array_new[tot++] = node;
	    }
	}

      size_t j;
      for (j = tot; j < size_new; j++)
	{
	  size_t v = ((j - 1) & (hash_new)) | 0x1;
	  array_new[j] = (queue_node_t*) v;
	}

      array_new[0] = (queue_node_t*) queue_node_set_null((-1) & (hash_new));
      tot--;

      qu->size = size_new;
      qu->hash = hash_new;
      qu->tail_n = tot + 1;
      qu->head_n = 0;
      queue_node_t** array_old = qu->array;
      qu->array = array_new;
      *tail_new = tot;

      optik_unlock(&qu->lock);
      free(array_old);
      return 1; 
    }
  else
    {
      FAD_U64(&qu->tail_n);
      return 0;
    }
}


static const int qpt = 1024;

static void UNUSED
queue_pause(int n)
{
  volatile int nn = n;
  while (nn-- > 0)
    {
       fflush(stdout);
    }
}


static inline void
queue_wait_resize(queue_t* qu)
{
  queue_rcu_op_on(qu);
  while(unlikely(optik_is_locked(qu->lock)))
    {
      queue_rcu_op_off(qu);
      /* queue_pause(4); */
    }
  queue_rcu_op_on(qu);
}


int
queue_optik_insert(queue_t* qu, skey_t key, sval_t val)
{
  NUM_RETRIES();
 restart:
  queue_wait_resize(qu);

  queue_node_t* node = queue_new_node(key, val);
  size_t hash = qu->hash;
  size_t tail = FAI_U64(&qu->tail_n);
  if (queue_is_full(tail, qu->head_n, hash))
    {
      /* printf(" !! full \n"); */
      if (!queue_optik_resize(qu, &tail))
	{
	  goto restart;
	}
      hash = qu->hash;
    }

  size_t spot = (tail + 1) & hash;
  size_t tailm = queue_node_set_null(tail);
  node->version = tailm;
  while (unlikely((uintptr_t) qu->array[spot]) != tailm)
    {
      DO_PAUSE();
    }

  qu->array[spot] = node;
 
  queue_rcu_op_off(qu);
  return 1;
}

sval_t
queue_optik_delete(queue_t* qu)
{
  NUM_RETRIES();
  queue_wait_resize(qu);
  size_t hash = qu->hash;
  while (1)
    {
      size_t head = qu->head_n;
      if (queue_is_empty(qu->tail_n, head, hash))
	{
	  return 0;
	}

      COMPILER_BARRIER();
      size_t spot = (head + 1) & hash;
      queue_node_t* node = qu->array[spot];
      if (unlikely(queue_node_is_null(node)))
	{
	  DO_PAUSE();
	  continue;
	}
      else if (unlikely(node->version != queue_node_set_null(head)))
	{
	  DO_PAUSE();
	  continue;
	}

      size_t spotm_next = queue_node_set_null(head + hash + 1);
      if ((CAS_U64(&qu->array[spot], node, spotm_next) == node))
	{
	  FAI_U64(&qu->head_n);
#if GC == 1
	  ssmem_free(alloc, (void*) node);
#endif
	  queue_rcu_op_off(qu);
	  return node->val;
	}

      DO_PAUSE();
    }

  return 1;			/* never happens */
}
