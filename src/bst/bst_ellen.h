/*   
 *   File: bst_ellen.h
 *   Author: Tudor David <tudor.david@epfl.ch>
 *   Description: non-blocking binary search tree
 *      based on "Non-blocking Binary Search Trees"
 *      F. Ellen et al., PODC 2010
 *   bst_ellen.h is part of ASCYLIB
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

#ifndef _BST_ELLEN_H_INCLUDED_
#define _BST_ELLEN_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "common.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"
#include "ssmem.h"

//the states a node can have
//we avoid an enum to better control the size of the data structures
#define STATE_CLEAN 0
#define STATE_DFLAG 1
#define STATE_IFLAG 2
#define STATE_MARK 3

#define TRUE 1
#define FALSE 0

#define MIN_KEY 0



#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef uint8_t leaf_t;
typedef uint8_t bool_t;

#define INF2 (KEY_MAX + 1)
#define INF1 (KEY_MAX)
#define MAX_KEY KEY_MAX //MAX_KEY should be of the form 2^n-1 for increased random key generation performance

extern __thread ssmem_allocator_t* alloc;

typedef ALIGNED(64) struct node_t node_t;
typedef union info_t info_t;

typedef info_t* update_t;//FIXME quite a few CASes done on this data structure; make it cache line aligned?

typedef struct iinfo_t {
    node_t* p;
    node_t* new_internal;
    node_t* l;
} iinfo_t;

typedef struct dinfo_t {
    node_t* gp;
    node_t* p;
    node_t* l;
    update_t pupdate;
} dinfo_t;

union info_t {
    iinfo_t iinfo;
    dinfo_t dinfo;
    uint8_t padding[CACHE_LINE_SIZE];
};


struct node_t {
    skey_t key;
    sval_t value;
    update_t update;
    volatile node_t* left;
    volatile node_t* right;
    bool_t leaf;
    uint8_t padding[CACHE_LINE_SIZE - sizeof(sval_t) - sizeof(skey_t) - sizeof(update_t) - 2*sizeof(uintptr_t) - sizeof(bool_t)];
};

typedef ALIGNED(64) struct search_result_t {
    node_t* gp; 
    node_t* p;
    node_t* l;
    update_t pupdate;
    update_t gpupdate;
#ifdef DO_PAD
    char padding[24];
#endif
} search_result_t;

extern __thread search_result_t * last_result;

int bst_cas_child(node_t* parent, node_t* old, node_t* new);

void bst_help(update_t u);

void bst_help_marked(info_t* op);

bool_t bst_help_delete(info_t* op);

sval_t bst_delete(skey_t key, node_t* root);

void bst_help_insert(info_t * op);

bool_t bst_insert(skey_t key, sval_t value, node_t* root);

node_t* bst_find(skey_t key, node_t* root);

search_result_t* bst_search(skey_t key, node_t* root);

node_t* bst_initialize();

void bst_init_local();

static inline uint64_t GETFLAG(update_t ptr) {
    return ((uint64_t)ptr) & 3;
}

static inline uint64_t FLAG(update_t ptr, uint64_t flag) {
    return (((uint64_t)ptr) & 0xfffffffffffffffc) | flag;
}

static inline uint64_t UNFLAG(update_t ptr) {
    return (((uint64_t)ptr) & 0xfffffffffffffffc);
}

//for testing purposes
void bst_print(node_t* node);

//for testing purposes
size_t bst_size(node_t* node);

//#define SETFLAG(ptr,state) ptr= (ptr & ~(0x3)) |state
//#define FLAG(ptr,state) ((ptr & ~(0x3)) | state)
//#define GETFLAG(ptr) (ptr & 0x3)
//#define UNFLAG(ptr) (ptr & 0xfffffffffffffffc)
//#define UNFLAG(ptr) (ptr & ~(0x3))

#endif
