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

sval_t
queue_optik_find(queue_t* qu, skey_t key)
{ 
  return 1;
}


UNUSED static void
queue_list_print(queue_node_t* node)
{
  size_t len = 0;
  queue_node_t* cur = node;
  while (cur != NULL && len++ < 1024)
    {
      printf("[%zu] -> ", cur->key);
      cur = cur->next;
    }
  if (len > 1000)
    {
      printf("[[[ extreme size ]]] ");
    }
  printf("NULL\n");
}

UNUSED static size_t
queue_list_len(queue_node_t* node)
{
  size_t len = 0;
  queue_node_t* cur = node;
  while (cur != NULL)
    {
      len++;
      cur = cur->next;
    }
  return len;
}

UNUSED static void
queue_list_head_tail_assert(queue_node_t* node, queue_node_t* head, queue_node_t* tail)
{
  size_t len = 0;
  queue_node_t* cur = node, *pred = NULL;
  assert(head == cur);
  while (cur != NULL)
    {
      len++;
      pred = cur;
      cur = cur->next;
    }
  assert(pred == tail);
}

int
queue_optik_insert(queue_t* qu, skey_t key, sval_t val)
{
  queue_node_t* node = queue_new_node(key, val, NULL);

  if (optik_num_queued(qu->tail_lock) > 2)
    {
      node->next = SWAP_PTR(&qu->overflow, node);

      if (node->next == NULL)
	{
	  optik_lock_backoff(&qu->tail_lock);
	  queue_node_t* oh = SWAP_PTR(&qu->overflow, NULL);
	  qu->tail->next = oh;
	  qu->tail = node;
	  optik_unlock(&qu->tail_lock);
	}
      else
	{
	  volatile queue_node_t* qn;
	  do
	    {
	      COMPILER_NO_REORDER(qn = qu->overflow;);
	      pause_rep(16);
	    }
	  while (qn != NULL);
	}

      return 1;
    }      

  optik_lock_backoff(&qu->tail_lock);
  qu->tail->next = node;
  qu->tail = node; 
  optik_unlock(&qu->tail_lock);
  return 1;
}

sval_t
queue_optik_delete(queue_t* qu)
{
  NUM_RETRIES();
 restart:
  COMPILER_NO_REORDER(const optik_t version = qu->head_lock;);
  const queue_node_t* node = qu->head;
  const queue_node_t* head_new = node->next;
  if (head_new == NULL)
    {
      return 0;
    }

  if (!optik_trylock_version(&qu->head_lock, version))
    {
      DO_PAUSE();
      goto restart;
    }

  qu->head = (queue_node_t*) head_new;
  optik_unlock(&qu->head_lock);

#if GC == 1
  ssmem_free(alloc, (void*) node);
#endif

  return head_new->val;
}
