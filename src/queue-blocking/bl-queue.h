/*
 *  File: bl-queue.h
 *  Author: Tudor David
 *
 *  Created on: February 20, 2015
 *
 *  Description: 
 *      blocking queue algorithm
 */

#ifndef _bl_queue_h_
#define _bl_queue_h_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lock_if.h"
#include "common.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"
#include "ssmem.h"

#define TRUE 1
#define FALSE 0

typedef volatile ALIGNED(64) struct qnode_t qnode_t;
typedef uint8_t bool_t;

typedef uint64_t qvalue_t;

struct qnode_t {
    qvalue_t* value;
    volatile qnode_t* next;
    //padding up to 32 bytes
    char padding[16];
};


typedef struct queue_t {
    volatile qnode_t* head;
    //TODO: padding?
    volatile qnode_t* tail;
    ptlock_t h_lock;
    ptlock_t t_lock;
    char padding[8];
} queue_t;

extern __thread ssmem_allocator_t* alloc;   

qnode_t* create_qnode(qvalue_t* value);

queue_t* create_queue();

void enqueue(queue_t* queue, qvalue_t* value);

qvalue_t* dequeue(queue_t* queue);

void print_queue(queue_t* queue);

int queue_size(queue_t* queue);




#ifdef __cplusplus
}
#endif

#endif


