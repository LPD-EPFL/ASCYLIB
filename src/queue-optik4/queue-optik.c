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
  /* printf("ins %zu\n", key); */
  /* queue_node_t* node = queue_new_node(key, val, (queue_node_t*) 1, NULL); */
  queue_node_t* node = queue_new_node(key, val, NULL, NULL);
  register queue_node_t* tail = SWAP_PTR(&qu->tail, node);
  node->next = tail;
  if (tail == NULL)
    {
      qu->head = node;
    }
  else
    {
      tail->pred = node;
    }
  return 1;
}

sval_t
queue_optik_delete(queue_t* qu)
{
  optik_lock(&qu->head_lock);
  /* if (qu->head == NULL) */
  if (qu->tail == NULL)
    {
      optik_unlock(&qu->head_lock);
      return 0;
    }

  while (qu->head == NULL);

  queue_node_t* head = qu->head;
  queue_node_t* head_new = head->pred;

  if (head_new == NULL)
    {
      // find the new head -- if any
      head_new = SWAP_PTR(&qu->tail, NULL);

      queue_node_t* pred = head_new;
      while (head_new && head_new != head)
  	{
  	  pred = head_new;
  	  head_new = head_new->next;
  	}
      head_new = pred;
    }

  qu->head = head_new;


  optik_unlock(&qu->head_lock);

#if GC == 1
  ssmem_free(alloc, (void*) head);
#endif

  return head->val;
}
