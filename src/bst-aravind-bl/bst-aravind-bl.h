/*   
 *   File: bst-aravind-bl.h
 *   Author: Tudor David <tudor.david@epfl.ch>
 *   Description: Aravind Natarajan and Neeraj Mittal. 
 *   Fast Concurrent Lock-free Binary Search Trees. PPoPP 2014
 *   bst-aravind-bl.h is part of ASCYLIB
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

#ifndef _BST_ARAVIND_BL_H_INCLUDED_
#define _BST_ARAVIND_BL_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lock_if.h"
#include "common.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"
#include "ssmem.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })



#define TRUE 1
#define FALSE 0

#define INF2 (KEY_MAX + 2)
#define INF1 (KEY_MAX + 1)
#define INF0 (KEY_MAX)

#define MAX_KEY KEY_MAX
#define MIN_KEY 0

typedef uint8_t bool_t;

extern __thread ssmem_allocator_t* alloc;
#ifdef USE_TSX
extern __thread uint64_t locked;
#endif

typedef ALIGNED(64) struct node_t node_t;

struct node_t{
    skey_t key;
    sval_t value;
    volatile node_t* right;
    volatile node_t* left;
    volatile uint64_t lock;
  uint8_t padding[24];
};

#ifndef __tile__
#ifndef __sparc__

static inline void set_bit(volatile uintptr_t* *array, int bit) {
    asm("bts %1,%0" : "+m" (*array) : "r" (bit));
}
static inline bool_t set_bit2(volatile uintptr_t *array, int bit) {

   // asm("bts %1,%0" :  "+m" (*array): "r" (bit));
     bool_t flag; 
     __asm__ __volatile__("lock bts %2,%1; setb %0" : "=q" (flag) : "m" (*array), "r" (bit)); return flag; 
   return flag;
}
#endif
#endif


typedef ALIGNED(CACHE_LINE_SIZE) struct seek_record_t{
    node_t* grandparent;
    node_t* parent;
    node_t* leaf;
  uint8_t padding[40];
} seek_record_t;

//extern __thread seek_record_t* seek_record;

node_t* initialize_tree();
void bst_init_local();
node_t* create_node(skey_t k, sval_t value, int initializing);
seek_record_t * bst_seek(skey_t key, node_t* node_r);
sval_t bst_search(skey_t key, node_t* node_r);
bool_t bst_insert(skey_t key, sval_t val, node_t* node_r);
sval_t bst_remove(skey_t key, node_t* node_r);
bool_t bst_cleanup(skey_t key);
uint32_t bst_size(volatile node_t* r);


static inline bool_t bst_lock_and_validate(node_t* node, node_t* child) {
#ifdef USE_TSX
#if TSX_STATS  == 1
    tried++;
#endif
    int num_retries = TSX_NUM_RETRIES;
    while (num_retries >= 0) {
        num_retries--;
        if (likely(_xbegin() == _XBEGIN_STARTED)) {
            if ((node->lock == 0) && ((node->left == child) || (node->right == child))) {
                return TRUE;
            }
            _xabort(0xff);
         }
    PAUSE;
    PAUSE;
    PAUSE;
    }
    PAUSE;
    PAUSE;
    PAUSE;

#if TSX_STATS  == 1
    locked++;
#endif
#endif
    uint64_t old_u;
    old_u = node->lock & 1;
    while (CAS_U64(&(node->lock), old_u, (old_u | 2)) != old_u) {
        old_u = node->lock & 1;
        PAUSE;
    }
        if (node->lock & 1) { 
            node->lock = 1;
            return FALSE;
        }
        if ((node->left != child) && (node->right != child))  {
            node->lock = node->lock & 1;
             return FALSE;
        }

    return TRUE;
        //if (CAS_U64(&(node->lock), 0, 2) == 0) {
            //if (node->lock!=2) fprintf(stderr, "error lock\n");
            //return TRUE;
        //}
        //sched_yield();
    //}
}


static inline int bst_unlock(node_t* node) {
#ifdef USE_TSX
        if (likely((node->lock & 2) == 0)) {
        _xend();
        return 0;
    } else {
        MEM_BARRIER;
        node->lock = node->lock & 0x1;
        return 0;
    }
    return 0;
#else
    asm volatile("" ::: "memory");
    MEM_BARRIER;
    node->lock = node->lock & 0x1;
    return 0;
#endif
}

static inline void mark(node_t* node) {
    asm volatile("" ::: "memory");
    node->lock = node->lock | 1;
}

#endif
