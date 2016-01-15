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

pqueue_t*
pqueue_new(size_t size)
{
  pqueue_t* pq = (pqueue_t*) memalign(CACHE_LINE_SIZE, sizeof(pqueue_t));
  assert(pq != NULL);

  pq->size = size;
  int i;
  for (i = 0; i < size; i++)
    {
      queue_low_init(pq->queues + i);
    }
  return pq;
}

void
pqueue_delete(pqueue_t* pq)
{
  printf("pqueue_delete - implement me\n");
}

int pqueue_size(pqueue_t* pq)
{
  int size = 0;
  int i;
  for (i = 0; i < pq->size; i++)
    {
      size += queue_low_size(pq-> queues + i);
    }
  return size;
}
