/*
 * File:
 *   skiplist.h
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Stress test of the skip list implementation.
 *
 * Copyright (c) 2009-2010.
 *
 * skiplist.h is part of Synchrobench
 * 
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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

#include "tm.h"
#include "ssalloc.h"

#define DEFAULT_DURATION                1000
#define DEFAULT_INITIAL                 1024
#define DEFAULT_NB_THREADS              1
#define DEFAULT_RANGE                   (2 * DEFAULT_INITIAL)
#define DEFAULT_SEED                    0
#define DEFAULT_UPDATE                  20
#define DEFAULT_ELASTICITY              4
#define DEFAULT_ALTERNATE               0
#define DEFAULT_EFFECTIVE               1

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

extern volatile AO_t stop;
extern unsigned int global_seed;
#ifdef TLS
extern __thread unsigned int *rng_seed;
#else /* ! TLS */
extern pthread_key_t rng_seed_key;
#endif /* ! TLS */
extern unsigned int levelmax;

#define TRANSACTIONAL                   d->unit_tx

typedef intptr_t val_t;
typedef intptr_t level_t;
#define VAL_MIN                         INT_MIN
#define VAL_MAX                         INT_MAX

typedef struct sl_node
{
  val_t val;
  intptr_t deleted;
  int toplevel;
  struct sl_node *next[1];
} sl_node_t;

typedef struct sl_intset
{
  sl_node_t *head;
} sl_intset_t;

int get_rand_level();
int floor_log_2(unsigned int n);

sl_node_t *sl_new_simple_node(val_t val, int toplevel, int transactional);
sl_node_t *sl_new_node(val_t val, sl_node_t *next, int toplevel, int transactional);
void sl_delete_node(sl_node_t *n);

sl_intset_t *sl_set_new();
void sl_set_delete(sl_intset_t *set);
int sl_set_size(sl_intset_t *set);

inline long rand_range(long r); /* declared in test.c */
