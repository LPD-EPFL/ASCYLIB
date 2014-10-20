/*
 * Definition of nodes for our performance tests.
 *
 */
#ifndef NODE_H
#define NODE_H

// #include "spinlock.h"
#include "common.h"

typedef ALIGNED(CACHE_LINE_SIZE) struct node {
    skey_t key;
    struct node *next;
    char padding[CACHE_LINE_SIZE - sizeof(skey_t) - sizeof(struct node *)];
} node_t;

#endif
