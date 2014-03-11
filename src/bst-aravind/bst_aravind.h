#ifndef _BST_ARAVIND_H_INCLUDED_
#define _BST_ARAVIND_H_INCLUDED_

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

typedef uint8_t bool_t;

#define INF2 (KEY_MAX + 1)
#define INF1 (KEY_MAX)
#define MAX_KEY KEY_MAX //MAX_KEY should be of the form 2^n-1 for increased random key generation performance

extern __thread ssmem_allocator_t* alloc;

typedef ALIGNED(64) struct node_t node_t;

struct node_t {
    skey_t key;
    sval_t value;
    node_t* left;
    node_t* right;
    uint8_t padding[CACHE_LINE_SIZE - sizeof(sval_t) - sizeof(skey_t) - 2*sizeof(uintptr_t)];
};

typedef struct seek_record_t {
    node_t* ancestor;
    node_t* successor;
    node_t* parent;
    node_t* leaf;
} seek_record_t;

extern __thread seek_record_t seek_record;

va

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

#endif
