/*   
 *   File: bst-aravind-bl.c
 *   Author: Tudor David <tudor.david@epfl.ch>
 *   Description: Aravind Natarajan and Neeraj Mittal. 
 *   Fast Concurrent Lock-free Binary Search Trees. PPoPP 2014
 *   bst-aravind-bl.c is part of ASCYLIB
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

#include "bst-aravind-bl.h"

RETRY_STATS_VARS;

__thread seek_record_t* seek_record;
__thread ssmem_allocator_t* alloc;

node_t* initialize_tree(){
    node_t* r;
    node_t* s;
    node_t* inf0;
    node_t* inf1;
    node_t* inf2;
    r = create_node(INF2,0,1);
    s = create_node(INF1,0,1);
    inf0 = create_node(INF0,0,1);
    inf1 = create_node(INF1,0,1);
    inf2 = create_node(INF2,0,1);

    asm volatile("" ::: "memory");
    r->left = s;
    r->right = inf2;
    s->right = inf1;
    s->left= inf0;
    asm volatile("" ::: "memory");

    return r;
}

void bst_init_local() {
    seek_record = (seek_record_t*) memalign(CACHE_LINE_SIZE, sizeof(seek_record_t));
    assert(seek_record != NULL);
}

node_t* create_node(skey_t k, sval_t value, int initializing) {
    volatile node_t* new_node;
#if GC == 1
    if (unlikely(initializing)) {
        new_node = (volatile node_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(node_t));
    } else {
        new_node = (volatile node_t*) ssmem_alloc(alloc, sizeof(node_t));
    }
#else 
    new_node = (volatile node_t*) ssalloc(sizeof(node_t));
#endif
    if (new_node == NULL) {
        perror("malloc in bst create node");
        exit(1);
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->key = k;
    new_node->value = value;
    new_node->lock = 0;
    asm volatile("" ::: "memory");
    return (node_t*) new_node;
}

seek_record_t * bst_seek(skey_t key, node_t* node_r){
    PARSE_TRY();
    volatile seek_record_t seek_record_l;
    node_t* node_s = node_r->left;
    seek_record_l.grandparent = node_r;
    seek_record_l.parent = node_s;
    seek_record_l.leaf = node_s->left;

    node_t* current = seek_record_l.leaf->left;


    while (current != NULL) {
        seek_record_l.grandparent = seek_record_l.parent;
        seek_record_l.parent = seek_record_l.leaf;
        seek_record_l.leaf = current;
        if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    seek_record->grandparent=seek_record_l.grandparent;
    seek_record->parent=seek_record_l.parent;
    seek_record->leaf=seek_record_l.leaf;
    return seek_record;
}

sval_t bst_search(skey_t key, node_t* node_r) {
    bst_seek(key, node_r);
    if (seek_record->leaf->key == key) {
        return seek_record->leaf->value;
    } else {
        return 0;
    }
}


bool_t bst_insert(skey_t key, sval_t val, node_t* node_r) {
    node_t* new_internal = NULL;
    node_t* new_node = NULL;

    while (1) {
        UPDATE_TRY();

        bst_seek(key, node_r);
        if (seek_record->leaf->key == key) {
            return FALSE;
        }

        node_t* parent = seek_record->parent;
        node_t* leaf = seek_record->leaf;

        if (bst_lock_and_validate(parent, leaf) == FALSE) continue;


            new_internal=create_node(max(key,leaf->key),0,0);
            new_node = create_node(key,val,0);

        if ( key < leaf->key) {
            new_internal->left = new_node;
            new_internal->right = leaf; 
        } else {
            new_internal->right = new_node;
            new_internal->left = leaf;
        }
#ifdef __tile__
        MEM_BARRIER;
#endif
        if (key < parent->key) {
            parent->left = new_internal;
        } else {
            parent->right = new_internal;
        }
        bst_unlock(parent);
        return TRUE;
    }
}

sval_t bst_remove(skey_t key, node_t* node_r) {
    node_t* leaf;
    sval_t val = 0;
    while (1) {
        UPDATE_TRY();

        bst_seek(key, node_r);

        leaf = seek_record->leaf;

        if (leaf->key != key) {
            return 0;
        }

        node_t* parent = seek_record->parent;
        node_t* grandparent = seek_record->grandparent;

        bool_t result = bst_lock_and_validate(parent, leaf);
        if (result == FALSE) continue;
        result = bst_lock_and_validate(grandparent, parent);

        if (result == FALSE) {
            bst_unlock(parent);
            continue;
        }

        mark(parent);
        node_t* sibling;
        if (key < parent->key) {
            sibling = parent->right;
        } else {
            sibling = parent->left;
        }

        if (key < grandparent->key) {
            grandparent->left = sibling;
        } else {
            grandparent->right = sibling;
        }

        val = leaf->value;
        bst_unlock(grandparent);
        bst_unlock(parent);

#if GC == 1
        ssmem_free(alloc, leaf);
        ssmem_free(alloc, parent);
#endif
        return val;
    }

}

uint32_t bst_size(volatile node_t* node) {
    if (node == NULL) return 0; 
    if ((node->left == NULL) && (node->right == NULL)) {
        if (node->key < INF0 ) return 1;
    }
    uint32_t l = 0;
    uint32_t r = 0;
    if ( node->left!=NULL)  {
        l = bst_size(node->left);
    }
    if  (node->right!=NULL) { 
        r = bst_size(node->right);
    }
    return l+r;
}


