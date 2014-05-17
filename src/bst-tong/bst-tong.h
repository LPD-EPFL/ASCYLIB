#ifndef _BST_TONG_H_INCLUDED_
#define _BST_TONG_H_INCLUDED_

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

typedef ALIGNED(64) struct node_t node_t;

struct node_t{
    skey_t key;
    union {
        sval_t value;
        volatile uint64_t leaf;
    }; //node is a leaf if it has a value
    volatile node_t* right;
    volatile node_t* left;
    volatile ptlock_t lock;
    volatile uint8_t marked;
    uint8_t padding[CACHE_LINE_SIZE - 39]; //pad to cache line size
};


typedef ALIGNED(CACHE_LINE_SIZE) struct seek_record_t{
    node_t* gp;
    node_t* p;
    node_t* n;
   uint8_t padding[8]; //pad to 32 bytes
} seek_record_t;


node_t* initialize_tree();
void bst_init_local();
node_t* create_node(skey_t k, sval_t value, int initializing);
seek_record_t * bst_seek(skey_t key, node_t* node_r);
sval_t bst_search(skey_t key, node_t* node_r);
bool_t bst_insert(skey_t key, sval_t val, node_t* node_r);
sval_t bst_remove(skey_t key, node_t* node_r);
bool_t bst_cleanup(skey_t key);
uint32_t bst_size(volatile node_t* r);

static inline uint64_t GETFLAG(volatile node_t* ptr) {
    return ((uint64_t)ptr) & 1;
}

static inline uint64_t GETTAG(volatile node_t* ptr) {
    return ((uint64_t)ptr) & 2;
}

static inline uint64_t FLAG(node_t* ptr) {
    return (((uint64_t)ptr)) | 1;
}

static inline uint64_t TAG(node_t* ptr) {
    return (((uint64_t)ptr)) | 2;
}

static inline uint64_t UNTAG(node_t* ptr) {
    return (((uint64_t)ptr) & 0xfffffffffffffffd);
}

static inline uint64_t UNFLAG(node_t* ptr) {
    return (((uint64_t)ptr) & 0xfffffffffffffffe);
}

static inline node_t* ADDRESS(volatile node_t* ptr) {
    return (node_t*) (((uint64_t)ptr) & 0xfffffffffffffffc);
}

#endif
