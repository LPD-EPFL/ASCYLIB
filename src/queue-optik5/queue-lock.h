/*   
 *   File: skiplist-lock.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   skiplist-lock.h is part of ASCYLIB
 *
n * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
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

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include "common.h"

#include <atomic_ops.h>
#include "lock_if.h"
#include "ssmem.h"
#include "optik.h"

extern unsigned int global_seed;
extern __thread ssmem_allocator_t* alloc;

typedef volatile struct queue_node
{
  volatile size_t version;
  skey_t key;
  sval_t val;
} queue_node_t;

typedef struct rcu_node
{
  volatile uint8_t in_op;
  uint8_t padding[CACHE_LINE_SIZE - sizeof(uint8_t) - sizeof(struct rcu_node*)];
  struct rcu_node* next;
} rcu_node_t;

STATIC_ASSERT(sizeof(rcu_node_t) == 64, "sizeof(rcu_node_t) == 64");


typedef ALIGNED(CACHE_LINE_SIZE) struct queue
{
  queue_node_t** array;
  optik_t lock;
  size_t hash;
  size_t size;
  rcu_node_t* threads;
  uint8_t padding1[CACHE_LINE_SIZE - 5 * sizeof(size_t)];
  optik_t head_lock;
  uint8_t padding2[CACHE_LINE_SIZE - 1 * sizeof(size_t)];
  size_t tail_n;
  uint8_t padding3[CACHE_LINE_SIZE - 1 * sizeof(size_t)];
} queue_t;

/* #warning test if putting head/tail on different CLs helps */
/* STATIC_ASSERT(sizeof(queue_t) == 128, "sizeof(queue_t) == 128"); */
STATIC_ASSERT(sizeof(queue_t) == 192, "sizeof(queue_t) == 192");


/* 
 * Create a new node without setting its next fields. 
 */
queue_node_t* queue_new_simple_node(skey_t key, sval_t val, int toplevel, int transactional);
/* 
 * Create a new node with its next field. 
 * If next=NULL, then this create a tail node. 
 */
queue_node_t* queue_new_node(skey_t key, sval_t val);
void queue_delete_node(queue_node_t* n);
queue_t* queue_new();
void queue_delete(queue_t* qu);
int queue_size(queue_t* cqu);
