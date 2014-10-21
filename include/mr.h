/******************************************************************************
 *
 * Functions which all memory reclamation schemes must provide.
 *
 *****************************************************************************/

#ifndef MR_H
#define MR_H

// #include "node.h"
 #include "utils.h"

void mr_init_global(uint64_t nthreads);
void mr_init_local(uint64_t thread_index, uint64_t nthreads);
void mr_thread_exit();
void mr_reinitialize();

struct mr_node {
    void *actual_node;
    struct timeval created; 
    struct mr_node *mr_next;
    char padding[CACHE_LINE_SIZE - sizeof(void *) - sizeof(struct timeval) - sizeof(struct mr_node*)];
};

typedef ALIGNED(CACHE_LINE_SIZE) struct mr_node mr_node_t;

#endif
