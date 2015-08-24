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

#include "queue-lock.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;


queue_node_t*
queue_new_node(skey_t key, sval_t val)
{
  queue_node_t* node = ssmem_alloc(alloc, sizeof(queue_node_t));
  node->key = key;
  node->val = val;
	
#ifdef __tile__
  MEM_BARRIER;
#endif

  return node;
}

void
queue_delete_node(queue_node_t *n)
{
  DESTROY_LOCK(ND_GET_LOCK(n));
  ssfree_alloc(1, (void*) n);
}

#define QUEUE_SIZE_INIT (1L<<17)

queue_t*
queue_new()
{
  queue_t *set;
	
  if ((set = (queue_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(queue_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }

  set->array = (queue_node_t**) memalign(CACHE_LINE_SIZE, QUEUE_SIZE_INIT * sizeof(queue_node_t*));
  assert(set->array != NULL);

  unsigned i;
  for (i = 0; i < QUEUE_SIZE_INIT; i++)
    {
      size_t v = ((i - 1) & (QUEUE_SIZE_INIT - 1)) | 0x1;
      set->array[i] = (queue_node_t*) v;
    }

  set->threads = NULL;
  set->tail_n = 0;
  set->size = QUEUE_SIZE_INIT;
  set->hash = QUEUE_SIZE_INIT - 1;

  optik_init(&set->lock);
  optik_init(&set->head_lock);

  return set;
}

void
queue_delete(queue_t *set)
{
  printf("queue_delete - implement me\n");
}

int
queue_size(queue_t* qu)
{
  /* int size = qu->tail_n - qu->head_lock.version; */
  int size = qu->tail_n - optik_get_n_locked(qu->head_lock);
  return size;
}
