/*   
 *   File: hashtable-lock.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: using one lazy linst per bucket
 *   hashtable-lock.h is part of ASCYLIB
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

#include "./lists/intset.h"

#define DEFAULT_MOVE                    0
#define DEFAULT_SNAPSHOT                0
#define DEFAULT_LOAD                    1
#define DEFAULT_ELASTICITY              1
#define DEFAULT_ALTERNATE               0
#define DEFAULT_EFFECTIVE               1

#define MAXHTLENGTH                     65536

/* Hashtable length (# of buckets) */
extern unsigned int maxhtlength;

/* ################################################################### *
 * HASH TABLE
 * ################################################################### */

typedef struct ht_intset 
{
  size_t hash;
  intset_l_t* buckets;
  ptlock_t* locks;
  uint8_t padding[CACHE_LINE_SIZE - 24];
} ht_intset_t;

void ht_delete(ht_intset_t *set);
int ht_size(ht_intset_t *set);
int floor_log_2(unsigned int n);
ht_intset_t *ht_new();
sval_t ht_contains(ht_intset_t* set, skey_t key);
int ht_add(ht_intset_t* set, skey_t key, sval_t val);
sval_t ht_remove(ht_intset_t* set, skey_t key);

/* 
 * Move an element in the hashtable (from one linked-list to another)
 */
int ht_move(ht_intset_t *set, int val1, int val2);
/* 
 * Read all elements of the hashtable (parses all linked-lists)
 * This cannot be consistent when used with move operation.
 * TODO: make a coarse-grain version of the snapshot.
 */
int ht_snapshot(ht_intset_t *set);

#define IO_FLUSH fflush(stdout)
