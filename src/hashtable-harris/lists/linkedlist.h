/*   
 *   File: linkedlist.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   linkedlist.h is part of ASCYLIB
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

/*
 *  linkedlist.h
 *  
 *
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
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

#include <atomic_ops.h>
#include "atomic_ops_if.h"

#include "common.h"
#include "utils.h"
#include "measurements.h"
#include "ssalloc.h"
#include "ssmem.h"

#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
/* Note: stdio is thread-safe */
#endif

#define DEFAULT_ALTERNATE		0
#define DEFAULT_EFFECTIVE		1

static volatile int stop;
extern __thread ssmem_allocator_t* alloc;

#define TRANSACTIONAL                   4

typedef volatile struct node 
{
  skey_t key;
  sval_t val;
  volatile struct node* next;
  uint8_t padding32[8];
#if defined(DO_PAD)
  uint8_t padding[CACHE_LINE_SIZE - sizeof(val_t) - sizeof(key_t) - sizeof(struct node*)];
#endif
} node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct intset 
{
  node_t *head;
} intset_t;

node_t *new_node(skey_t key, sval_t val, node_t *next, int initializing);
void bucket_set_init(intset_t* set);
void set_delete(intset_t *set);
