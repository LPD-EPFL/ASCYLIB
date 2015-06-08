/*   
 *   File: skiplist-lock.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   skiplist-lock.h is part of ASCYLIB
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

typedef struct mstack_node
{
  skey_t key;
  sval_t val; 
  struct mstack_node* next;
} mstack_node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct mstack
{
  mstack_node_t* top;
  ptlock_t lock;
  /* uint8_t padding1[CACHE_LINE_SIZE - sizeof(mstack_node_t*) - sizeof(ptlock_t)]; */
  /* mstack_node_t* tail; */
  /* ptlock_t tail_lock; */
  uint8_t padding2[CACHE_LINE_SIZE - sizeof(ptlock_t) - sizeof(mstack_node_t*)];
} mstack_t;

int floor_log_2(unsigned int n);

/* 
 * Create a new node without setting its next fields. 
 */
mstack_node_t* mstack_new_simple_node(skey_t key, sval_t val, int toplevel, int transactional);
/* 
 * Create a new node with its next field. 
 * If next=NULL, then this create a tail node. 
 */
mstack_node_t *mstack_new_node(skey_t key, sval_t val, mstack_node_t *next);
void mstack_delete_node(mstack_node_t* n);
mstack_t* mstack_new();
void mstack_delete(mstack_t* qu);
int mstack_size(mstack_t* cqu);
