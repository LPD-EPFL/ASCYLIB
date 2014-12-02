/*   
 *   File: bst_howley.h
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>, 
 *  	     Tudor David <tudor.david@epfl.ch>
 *   Description: Shane V Howley and Jeremy Jones. 
 *   A non-blocking internal binary search tree. SPAA 2012
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
#include "ssmem.h"
#include "common.h"

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
#define FOUND 0x0
#define NOT_FOUND_L 0x1
#define NOT_FOUND_R 0x2
#define ABORT 0x3

#define INF KEY_MAX

typedef uint8_t bool_t;

extern __thread ssmem_allocator_t* alloc;

extern const sval_t val_mask;

typedef ALIGNED(64) union operation_t operation_t;
typedef ALIGNED(64) struct node_t node_t;

typedef struct child_cas_op_t {
	bool_t is_left;
	node_t* expected;
	node_t* update;
	//char padding[40]; 

} child_cas_op_t;

typedef struct relocate_op_t {
	int /*volatile*/ state; // initialize to ONGOING every time a relocate operation is created
	node_t* dest;
	operation_t* dest_op;
	skey_t remove_key;
	sval_t remove_value;
	skey_t replace_key;
	sval_t replace_value;
	//char padding[32]; 

} relocate_op_t;


struct node_t {
	skey_t /*volatile*/ key; 
    sval_t value;
	operation_t* /*volatile*/ op;
	volatile node_t* /*volatile*/ left;
	volatile node_t* /*volatile*/ right;
    uint8_t padding[CACHE_LINE_SIZE-sizeof(sval_t)-sizeof(skey_t)-3*sizeof(uintptr_t)];
	// char padding[32];
};

union operation_t {
	child_cas_op_t child_cas_op;
	relocate_op_t relocate_op;
    uint8_t padding[CACHE_LINE_SIZE];
};

//BST functions
sval_t bst_contains(skey_t k, node_t* root);
sval_t bst_find(skey_t k, node_t** pred, operation_t** pred_op, node_t** curr, operation_t** curr_op, node_t* aux_root, node_t* root); 
//do we need * or ** for node_t? if only the 
//value pointed to by pred is modified, we need *; if the 
//place pred points to is modified and we want the modif
//to live outside the function call, we need **.  

node_t* bst_initialize();
bool_t bst_add(skey_t k, sval_t v, node_t* root);
void bst_help_child_cas(operation_t* op, node_t* dest, node_t* root);
sval_t bst_remove(skey_t k, node_t* root);
bool_t bst_help_relocate(operation_t* op, node_t* pred, operation_t* pred_op, node_t* curr, node_t* root);
void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr, node_t* root);
void bst_help(node_t* pred, operation_t* pred_op, node_t* curr, operation_t* curr_op, node_t* root );
unsigned long bst_size(node_t* node);
void bst_print(volatile node_t* node); 

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
static inline uint64_t ISNULL(volatile node_t* node){
	return (node == NULL) || (((uint64_t)node) & 1);
}

static inline uint64_t SETNULL(volatile node_t* node){
	return (((uint64_t)node) & 0xfffffffffffffffe) | 1;
}

#endif
