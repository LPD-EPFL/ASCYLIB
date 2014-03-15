/*
 *  File: bst-drachsler.h
 *  Author: Tudor David
 *
 *  Created on: March 11, 2014
 *
 *  Description: 
 *      concurrent binary search tree
 *      based on "Practical Concurrent Binary Search Trees via Logical Ordering"
 *      D. Drachsler et al., PPOPP 2014
 */

#ifndef _BST_DRACHLER_H_INCLUDED_
#define _BST_DRACHLER_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lock_if.h"
#include "common.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"
#include "ssmem.h"

//#define DO_DRACHSLER_REBALANCE 1

#define TRUE 1
#define FALSE 0

#define MAX_KEY KEY_MAX
#define MIN_KEY 0

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


typedef uint8_t bool_t;

extern __thread ssmem_allocator_t* alloc;

typedef ALIGNED(64) struct node_t node_t;

struct node_t {
    node_t* left;
    node_t* right;
    node_t* parent;
    node_t* succ;
    node_t* pred;
#ifdef DO_DRACHSLER_REBALANCE
    int32_t left_height;
    int32_t right_height;
#endif
    ptlock_t tree_lock;
    ptlock_t succ_lock;
    skey_t key;
    sval_t value;
    bool_t mark;
};

node_t* initialize_tree();

node_t* bst_search(skey_t key, node_t* root);

sval_t bst_contains(skey_t k, node_t* root);

bool_t bst_insert(skey_t k, sval_t v, node_t* root);

node_t* choose_parent(node_t* pred, node_t* succ, node_t* first_cand);

void insert_to_tree(node_t* parent, node_t* new_node, node_t* root);

node_t* lock_parent(node_t* node);

sval_t bst_remove(skey_t key, node_t* root);

bool_t acquire_tree_locks(node_t* n);

void remove_from_tree(node_t* n, bool_t has_two_children, node_t* root);

void update_child(node_t *parent, node_t* old_ch, node_t* new_ch);

uint32_t bst_size(node_t* node);

#ifdef DO_DRACHSLER_REBALANCE

void bst_rebalance(node_t* node, node_t* child, node_t* root);

void bst_rotate(node_t* child, node_t* n, node_t* parent, bool_t left_rotation);

#endif

#endif
