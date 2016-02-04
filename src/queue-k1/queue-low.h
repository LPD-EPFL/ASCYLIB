/*   
 *   File: queue-low.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   queue-low.h is part of ASCYLIB
 *
 * Copyright (c) 2016 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
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

#include "optik.h"

#ifndef _H_QUEUE_LOW
#define _H_QUEUE_LOW

typedef struct queue_node
{
  skey_t key;
  sval_t val;
  struct queue_node* next;
  optik_t lock;
} queue_node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct queue_low
{
  queue_node_t* head;
  uint8_t padding[CACHE_LINE_SIZE - sizeof(queue_node_t*)];
} queue_low_t;

queue_node_t* queue_node_new(skey_t key, sval_t val, queue_node_t* next);
void queue_low_init(queue_low_t* ql);
int queue_low_size(queue_low_t* ql);
int queue_low_push(queue_low_t *ql, skey_t key, sval_t val);
sval_t queue_low_pop(queue_low_t *ql);
sval_t queue_low_get_min(queue_low_t *ql);

#endif	/* _H_QUEUE_LOW */
