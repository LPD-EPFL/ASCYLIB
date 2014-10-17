/******************************************************************************
 *
 * Functions which all memory reclamation schemes must provide.
 *
 *****************************************************************************/

#ifndef MR_H
#define MR_H

#include "node.h"

void mr_init_global();
void mr_init_local();
void mr_thread_exit();
void mr_reinitialize();

struct mr_node {
    void *actual_node;
    struct mr_node *mr_next;
};

typedef ALIGNED(CACHE_LINE_SIZE) struct mr_node mr_node_t;

#endif
