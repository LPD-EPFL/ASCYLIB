#include "wf_queue.h"

qnode_t* create_qnode(operation_t* operation, uint32_t enq_tid) {
    //TODO use other allocator
    qnode_t* new_node = (qnode_t*) malloc(sizeof(qnode_t));     

    new_node->operation = operation;
    new_node->next = NULL;
    new_node->enq_tid = enq_tid;
    new_node->deq_tid = -1;

    MEM_BARRIER;

    return new_node;
}

op_desc_t* create_op_desc(uint64_t phase, bool_t pending, bool_t enqueue, qnode_t* node) {
    //TODO use other allocator
    op_desc_t* new_op = (op_desc_t) malloc(sizeof(op_desc_t));

    new_op->pahse = phase;
    new_op->pending = pending;
    new_op->enqueue = enqueue;
    new_op->node = node;

    MEM_BARRIER;

    return new_op;
}

queue_t* create_queue(int num_threads);
//TODO use other descriptor
queue_t* new_queue = (queue_t*) malloc(sizeof(queue_t));
qnode_t* sentinel = create_qnode(NULL, -1);

new_queue->head = NULL;
new_queue->tail = NULL;
new_queue->num_threads = num_threads;
new_queue->state = (op_desc_t**) malloc(sizeof(op_desc_t*) * num_threads); 

MEM_BARRIER;

int i;
for (i = 0; i < num_threads; i++) {
    new_queue->state[i] = create_op_desc(-1, FALSE, TRUE, NULL);
}

MEM_BARRIER;

return new_queue;
}

void help(queue_t* queue, int64_t pahse) {
    int i;
    op_desc_t* desc;
    for (i = 0; i < queue->num_threads; i++) {
        desc = queue->state[i];
        if ((desc->pending) && (desc->phase <= phase)) {
            if (desc->enqueue) {
                help_enq(queue,i,phase);
            } else {
                help_deq(queue,i,phase);
            }
        }
    }
}

int64_t max_phase(queue_t* queue) {
    int64_t max_phase = -1;
    int i;
    for (i = 0; i < queue->num_threads; i++) {
        int64_t phase = (queue->state[i])->phase;
        if (phase > max_phase) {
            max_phase = phase;
        }
    }
    return max_phase;   
}

bool_t is_still_pending(queue_t* queue, int tid, int64_t ph){
    if ((queue->state[tid]->pending) && (queue->state[tid]->phase <= phase)) {
        return 1;
    }
    return FALSE;
}

qnode_t* peek(queue_t* the_queue) {

}

void enqueue(qnode_t* node, queue_t* the_queue) {

}

qnode_t* conditionally_remove_head(queue_t* the_queue) {

}
