/*   
 *   File: skiplist-lock.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   skiplist-lock.c is part of ASCYLIB
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

#include "queue-lockfree.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;


queue_node_t*
queue_new_node(skey_t key, sval_t val, queue_node_t* next)
{
#if GC == 1
  queue_node_t* node = ssmem_alloc(alloc, sizeof(*node));
#else
  queue_node_t* node = ssalloc(sizeof(*node));
#endif

  node->key = key;
  node->val = val;
  node->next = next;
	
#ifdef __tile__
  MEM_BARRIER;
#endif

  return node;
}

void
queue_delete_node(queue_node_t *n)
{
#if GC == 1
  ssmem_free(alloc, (void*) n);
#endif
}

queue_t*
queue_new()
{
  queue_t *set;
	
  if ((set = (queue_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(queue_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }

  queue_node_t* node = (queue_node_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(queue_node_t));
  node->next = NULL;
  set->head = node;
  set->tail = node;

  return set;
}

void
queue_delete(queue_t *set)
{
  printf("queue_delete - implement me\n");
}

int queue_size(queue_t *set)
{
  int size = 0;
  queue_node_t *node;
	
  /* We have at least 2 elements */
  node = set->head;
  while (node->next != NULL) 
    {
      size++;
      node = node->next;
    }
  return size;
}
