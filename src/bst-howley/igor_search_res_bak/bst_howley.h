/*   
 *   File: bst_howley.h
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>,
 *   Description: 
 *   bst_howley.h is part of ASCYLIB
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


#ifndef _BST_HOWLEY_H_INCLUDED_
#define _BST_HOWLEY_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"

#define TRUE 1
#define FALSE 0

//Encoded in the operation pointer
#define STATE_OP_NONE 0
#define STATE_OP_MARK 1
#define STATE_OP_CHILDCAS 2
#define STATE_OP_RELOCATE 3

//In the relocate_op struct
#define STATE_OP_ONGOING 0
#define STATE_OP_SUCCESSFUL 1
#define STATE_OP_FAILED 2

//States for the result of a search operation
#define FOUND 0
#define NOT_FOUND_L 1
#define NOT_FOUND_R 2
#define ABORT 3

#define INF UINT32_MAX
//TODO min key; max key values

typedef uint32_t skey_t;
typedef uint8_t bool_t;

typedef /*ALIGNED(64)*/ union operation_t operation_t;
typedef ALIGNED(64) struct node_t node_t;

typedef struct child_cas_op_t {
	/*ALIGNED(64)*/ bool_t is_left;
	/*ALIGNED(64)*/ node_t* expected;
	/*ALIGNED(64)*/ node_t* update;

} child_cas_op_t;

typedef struct relocate_op_t {
	/*ALIGNED(64)*/ int state; //TODO initialize to ONGOING
	/*ALIGNED(64)*/ skey_t remove_key;
	/*ALIGNED(64)*/ skey_t replace_key; 
	/*ALIGNED(64)*/ node_t* dest;
	/*ALIGNED(64)*/ operation_t* dest_op;

} relocate_op_t;


struct node_t {
	/*ALIGNED(64)*/ skey_t key; //volatile? (for all variables)
	/*ALIGNED(64)*/ operation_t* op;
	/*ALIGNED(64)*/ node_t* left;
	/*ALIGNED(64)*/ node_t* right;
};

union operation_t {
	child_cas_op_t child_cas_op;
	relocate_op_t relocate_op;
};

typedef ALIGNED(64) struct bst_search_result_t {
	uint8_t result;
	node_t* pred;
	node_t* curr;
	operation_t* pred_op;
	operation_t* curr_op;
} bst_search_result_t;

bst_search_result_t** my_search_result;

//BST functions
node_t* bst_initialize(int num_proc);
void bst_init_local(int id);
bool_t bst_contains(skey_t k, node_t* root, int id);
bst_search_result_t* bst_find(skey_t k, node_t* aux_root, node_t* root, int id); 
bool_t bst_add(skey_t k, node_t* root, int id);
bool_t bst_remove(skey_t k, node_t* root, int id);

void bst_help_child_cas(operation_t* op, node_t* dest);
bool_t bst_help_relocate(operation_t* op, node_t* pred, operation_t* pred_op, node_t* curr);
void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr);
void bst_help(node_t* pred, operation_t* pred_op, node_t* curr, operation_t* curr_op);

// for debugging
unsigned long bst_size(node_t* node);
void bst_print(node_t* node); 

//Helper functions

static inline uint64_t GETFLAG(operation_t* ptr) {
    return ((uint64_t)ptr) & 3;
}

static inline uint64_t FLAG(operation_t* ptr, uint64_t flag) {
    return (((uint64_t)ptr) & 0xfffffffffffffffc) | flag;
}

static inline uint64_t UNFLAG(operation_t* ptr) {
    return (((uint64_t)ptr) & 0xfffffffffffffffc);
}


//Last bit of the node pointer will be set to 1 if the pointer is null 
static inline uint64_t ISNULL(node_t* node){
	return (node == NULL) || (((uint64_t)node) & 1);
}

static inline uint64_t SETNULL(node_t* node){
	return (((uint64_t)node) & 0xfffffffffffffffe) | 1;
}

//TODO QUESTION: do we need to set specifically the last bit to 0 when the pointer is not null, to make sure that we don't have it marked as null when in fact it is not null?

#endif