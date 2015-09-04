#include "bl-stack.h"

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

stack_t* create_stack(){
    volatile stack_t* new_stack;
#if GC == 1
    new_stack = (volatile stack_t*) ssmem_alloc(alloc,sizeof(stack_t));
#else
    new_stack = (volatile stack_t*) ssalloc(sizeof(stack_t));
#endif

    qnode_t* sentinel = create_qnode(NULL);

    new_stack->head = sentinel;
    new_stack->tail = sentinel;
    INIT_LOCK(&(new_stack->t_lock));

    MEM_BARRIER;

    return new_stack;
}

void push(stack_t* stack, qvalue_t* value){
    qnode_t* new_node = create_qnode(value);
    LOCK(&(stack->t_lock));
    stack->head->next = new_node;
    stack->head = new_node;
    UNLOCK(&(stack->t_lock));
}


qvalue_t* pop(stack_t* stack) {
    LOCK(&(stack->t_lock));
    qnode_t* node = stack->head;
    qnode_t* new_head = node->next;
    if (new_head == NULL) {
        UNLOCK(&(stack->t_lock));
        return NULL;
    }
    qvalue_t* ret = new_head->value;
    stack->head = new_head;
    UNLOCK(&(stack->t_lock));
#if GC==1
    ssmem_free(alloc, (void*)node);
#endif
    return ret;
}


void print_stack(stack_t* stack) {
    //sequential stack print method
    qnode_t* node = stack->head;
    qvalue_t* cur;
    if (node == NULL) {
        fprintf(stderr, "Empty stack\n");
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

int stack_size(stack_t* stack) {
    qnode_t* node = stack->head;
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
