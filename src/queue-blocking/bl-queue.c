#include "bl-queue.h"

__thread ssmem_allocator_t* alloc;

qnode_t* create_qnode(qvalue_t* value) {
    volatile qnode_t* new_node;
#if GC == 1
    new_node = (volatile qnode_t*) ssmem_alloc(alloc,sizeof(qnode_t));
#else
    new_node = (volatile qnode_t*) ssalloc(sizeof(qnode_t));
#endif

    new_node->value = value;
    new_node->next = NULL;
    MEM_BARRIER;
    return new_node;
}

queue_t* create_queue(){
    volatile queue_t* new_queue;
#if GC == 1
    new_queue = (volatile queue_t*) ssmem_alloc(alloc,sizeof(queue_t));
#else
    new_queue = (volatile queue_t*) ssalloc(sizeof(queue_t));
#endif

    qnode_t* sentinel = create_qnode(NULL);

    new_queue->head = sentinel;
    new_queue->tail = sentinel;
    INIT_LOCK(&(new_queue->t_lock));
    INIT_LOCK(&(new_queue->h_lock));

    MEM_BARRIER;

    return new_queue;
}

void enqueue(queue_t* queue, qvalue_t* value){
    qnode_t* new_node = create_qnode(value);
    LOCK(&(queue->t_lock));
    queue->tail->next = new_node;
    queue->tail = new_node;
    UNLOCK(&(queue->t_lock));
}


qvalue_t* dequeue(queue_t* queue) {
    LOCK(&(queue->h_lock));
    qnode_t* node = queue->head;
    qnode_t* new_head = node->next;
    if (new_head == NULL) {
        UNLOCK(&(queue->h_lock));
        return NULL;
    }
    qvalue_t* ret = new_head->value;
    queue->head = new_head;
    UNLOCK(&(queue->h_lock));
#if GC==1
    ssmem_free(alloc, (void*)node);
#endif
    return ret;
}


void print_queue(queue_t* queue) {
    //sequential queue print method
    qnode_t* node = queue->head;
    qvalue_t* cur;
    if (node == NULL) {
        fprintf(stderr, "Empty queue\n");
        return;
    }
    fprintf(stderr, "HEAD\n");
    fprintf(stderr, "====\n");
    node = node->next;
    while (node!=NULL) {
        if (node->value) {
            cur = node->value;
            fprintf(stderr, "%llu\n",*cur);
        }
        node = node->next;
    }
    fprintf(stderr, "====\n");
    fprintf(stderr, "TAIL\n");
    fprintf(stderr, "\n");
}

int queue_size(queue_t* queue) {
    qnode_t* node = queue->head;
    if (node == NULL) {
        return 0;
    }
    int size = 0;
    node = node->next;
    while (node!=NULL) {
        if (node->value) {
            size++;
        }
        node = node->next;
    }
    return size;
}
