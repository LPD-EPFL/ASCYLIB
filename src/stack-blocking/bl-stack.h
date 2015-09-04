/*
 *  File: bl-stack.h
 *  Author: Tudor David
 *
 *  Created on: February 20, 2015
 *
 *  Description: 
 *      blocking stack algorithm
 */

#ifndef _bl_stack_h_
#define _bl_stack_h_
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


typedef struct stack_t {
    volatile qnode_t* head;
    //TODO: padding?
    volatile qnode_t* tail;
    ptlock_t t_lock;
    char padding[8];
} stack_t;

extern __thread ssmem_allocator_t* alloc;   

qnode_t* create_qnode(qvalue_t* value);

stack_t* create_stack();

void push(stack_t* stack, qvalue_t* value);

qvalue_t* pop(stack_t* stack);

void print_stack(stack_t* stack);

int stack_size(stack_t* stack);




#ifdef __cplusplus
}
#endif

#endif


