/*   
 *   File: bst_bronson_java.h
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>, 
 *  	     Tudor David <tudor.david@epfl.ch>
 *   Description: Nathan G. Bronson, Jared Casper, Hassan Chafi
 *   , and Kunle Olukotun. A Practical Concurrent Binary Search Tree. 
 *   PPoPP 2010.
 *   bst_bronson_java.h is part of ASCYLIB
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

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "ssalloc.h"
#include "ssmem.h"
#include "lock_if.h"
#include "common.h"

#define FOUND 1
#define NOT_FOUND 2
#define RETRY 3

#define UPDATE_IF_PRESENT 1
#define UPDATE_IF_ABSENT 2

#define UNLINKED_OVL 2

#define TRUE 1
#define FALSE 0

#define UNLINK_REQUIRED -1
#define REBALANCE_REQUIRED -2
#define NOTHING_REQUIRED -3

// Spin time for bst_wait_until_not_changing
#define SPIN_COUNT 100

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
   
// typedef uint32_t bst_height_t; 
// typedef uint64_t bst_version_t;
typedef uint8_t bool_t;
typedef uint8_t function_t;

extern __thread ssmem_allocator_t* alloc;

typedef ALIGNED(64) union node_t node_t;

union node_t {
	
	struct {
		volatile int height; 
		volatile skey_t key; 
		volatile sval_t value; 
		volatile uint64_t version; 
		
		volatile node_t* parent; 
		volatile node_t* left; 
		volatile node_t* right; 
		volatile ptlock_t lock;				
	};
	// Compute the node padding depending on the type of lock used
	char padding[64*((48+sizeof(ptlock_t))/64+1)];
};

STATIC_ASSERT(sizeof(node_t) == 64, "sizeof(node_t) == 64");

// bst interface functions
volatile node_t* bst_initialize();
sval_t bst_contains(skey_t k, volatile node_t* root);
bool_t bst_add(skey_t k, sval_t v, volatile node_t* root);
sval_t bst_remove(skey_t k, volatile node_t* root);

// bst private functions
void wait_until_not_changing(volatile node_t* node);

bool_t attempt_unlink_nl(volatile node_t* parent, volatile node_t* node);

int node_conditon(volatile node_t* node);

void fix_height_and_rebalance(volatile node_t* node);

volatile node_t* fix_height_nl(volatile node_t* node);

volatile node_t* rebalance_nl(volatile node_t* n_parent, volatile node_t* n);

volatile node_t* rebalance_to_right_nl(volatile node_t* n_parent, volatile node_t* n, volatile node_t* nl, int hr0);

volatile node_t* rebalance_to_left_nl(volatile node_t* n_parent, volatile node_t* n, volatile node_t* nr, int hl0);

volatile node_t* rotate_right_nl(volatile node_t* n_parent, volatile node_t* n, volatile node_t* nl, int hr, int hll, volatile node_t* nlr, int hlr);

volatile node_t* rotate_left_nl(volatile node_t* n_parent, volatile node_t* n, int hl, volatile node_t* nr, volatile node_t* nrl, int hrl, int hrr);

volatile node_t* rotate_right_over_left_nl(volatile node_t* n_parent, volatile node_t* n, volatile node_t* nl, int hr, int hll, volatile node_t* nlr, int hlrl);

volatile node_t* rotate_left_over_right_nl(volatile node_t* n_parent, volatile node_t* n, int hl, volatile node_t* nr, volatile node_t* nrl, int hrr, int hrlr);

void set_child(volatile node_t* parent, volatile node_t* child, bool_t is_right);

sval_t attempt_node_update(function_t func, sval_t new_value, volatile node_t* parent, volatile node_t* node);

sval_t attempt_update(skey_t key, function_t func, sval_t new_value, volatile node_t* parent, volatile node_t* node, uint64_t node_v);

volatile node_t* new_node(int height, skey_t key, uint64_t version, sval_t value, volatile node_t* parent, volatile node_t* left, volatile node_t* right, bool_t initializing);

bool_t attempt_insert_into_empty(skey_t key, sval_t value, volatile node_t* holder);

sval_t update_under_root(skey_t k, function_t func, sval_t new_value, volatile node_t* holder);

sval_t attempt_get(skey_t k, volatile node_t* node, bool_t is_right, uint64_t node_v);

void bst_print(volatile node_t* node);

uint64_t bst_size(volatile node_t* node);


//Helper functions

static inline volatile node_t* CHILD(volatile node_t* parent, bool_t is_right) {
	return is_right ? parent->right : parent->left;
}

static inline uint64_t BEGIN_CHANGE(volatile uint64_t ovl) {
	return (ovl | 1);
}

static inline uint64_t END_CHANGE(volatile uint64_t ovl) {
	return (ovl | 3) + 1;
}

static inline int HEIGHT(volatile node_t* node) {
	return node == NULL ? 0 : node->height;
}

static inline bool_t IS_SHRINKING(volatile uint64_t ovl) {
	return (bool_t)((ovl & 1) != 0);
}

static inline bool_t IS_UNLINKED(volatile uint64_t ovl) {
	return (bool_t)((ovl & 2) != 0);
}

static inline bool_t IS_SHRINKING_OR_UNLINKED(volatile uint64_t ovl){
	return (bool_t)((ovl & 3) != 0L);
}

static inline bool_t SHOULD_UPDATE(function_t func, sval_t prev) {

	return func == UPDATE_IF_ABSENT ? prev == 0 : prev != 0;
}

static inline sval_t UPDATE_RESULT(function_t func, sval_t prev) {

	return func == UPDATE_IF_ABSENT ? NOT_FOUND : prev;
}

static inline sval_t NO_UPDATE_RESULT(function_t func, sval_t prev){
    return func == UPDATE_IF_ABSENT ? prev : NOT_FOUND;
}
