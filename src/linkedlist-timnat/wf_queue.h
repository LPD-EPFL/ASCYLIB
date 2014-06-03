/*
 *  Wait-free queue
 *      based on the paper "Wait-free queues with multiple enqueuers and dequeuers", PPoPP 2011  
 *      modified as seen in the paper "A practical wait-free simulation for lock-free data structures", PPoPP 2014
 */

#ifdef _WF_QUEUE_H_
#define _WF_QUEUE_H_

#define TRUE 1
#define FALSE 0

typedef ALIGNED(64) struct qnode_t qnode_t;
typedef uint8_t bool_t;

struct qnode_t {
    operation_t* operation;
    qnode_t* next;
    int32_t enq_tid;
    int32_t deq_tid;
}

typedef struct op_desc_t {
    int64_t phase;
    bool_t pending;
    bool_t enqueue;
    qnode_t* node;
} op_desc_t;

//returns the current head of the queue without removing it  
qnode_t* peek(queue_t* the_queue);

//enqueues a value to the tail of the queue
void enqueue(qnode_t* node, queue_t* the_queue);

//receives a value it expects to find at the head of the queue and removes it (dequeues it) only if this value is found at the head
qnode_t* conditionally_remove_head(queue_t* the_queue);

#endif
