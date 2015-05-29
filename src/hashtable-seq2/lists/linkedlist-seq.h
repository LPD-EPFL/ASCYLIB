/*   
 *   File: linkedlist-seq.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   linkedlist-seq.h is part of ASCYLIB
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

#ifndef _H_LINKEDLIST_LOCK_
#define _H_LINKEDLIST_LOCK_

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

#include <atomic_ops.h>
#include "lock_if.h"

#include "common.h"
#include "utils.h"
#include "measurements.h"
#include "ssmem.h"

#define HT_ALLOC_TYPE 1		/* 0: malloc,
				   1: ssmem
				   2: memalign
				   3: ssalloc 
				*/

#if HT_ALLOC_TYPE == 0
#  define ht_alloc(s)        malloc(s)
#  define ht_free(m)         free(m)
#elif HT_ALLOC_TYPE == 1
#  define ht_alloc(s)        ssmem_alloc(alloc, s)
#  define ht_free(m)         ssmem_free(alloc, m)
#elif HT_ALLOC_TYPE == 2
#  define ht_alloc(s)        memalign(s, s)
#  define ht_free(m)         free(m)
#elif HT_ALLOC_TYPE == 3
#  define ht_alloc(s)        ssalloc(s)
#  define ht_free(m)         ssfree(m)
#else
#  error give correct allocator type
#endif


#define DEFAULT_LOCKTYPE	  	2
#define DEFAULT_ALTERNATE		0
#define DEFAULT_EFFECTIVE		1

static volatile int stop;
extern __thread ssmem_allocator_t* alloc;

typedef ALIGNED(32) volatile struct node
{
  skey_t key;
  sval_t val;
  uint8_t padding32[8];
  volatile struct node* next;
  /* uint8_t padding[32]; */
} node_t;

typedef struct intset 
{
  node_t* head;
  /* uint8_t padding[CACHE_LINE_SIZE - sizeof(node_t*)]; */
} intset_t;

node_t* new_node(skey_t key, sval_t val, node_t* next);
void bucket_set_init(intset_t* set);
void set_delete(intset_t* set);
int set_size(intset_t* set);
void node_delete(node_t* node);

#endif	/* _H_LINKEDLIST_LOCK_ */
