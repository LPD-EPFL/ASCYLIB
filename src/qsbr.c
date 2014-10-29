/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (c) Thomas E. Hart.
 */

#include "qsbr.h"
#include "mr.h"
// #include "test.h"
#include <stdio.h>
#include "atomic_ops_if.h"
#include "utils.h"
#include "ssalloc.h"
#include "lock_if.h"
#include "linkedlist-qsbr-hart/node.h"

#define NOT_ENTERED 0
#define ENTERED 1
#define DONE 2

struct qsbr_globals {
    ptlock_t update_lock ALIGNED(CACHE_LINE_SIZE);
    int global_epoch ALIGNED(CACHE_LINE_SIZE);
    // OANA IGOR Should we pad here? It's not clear how big the padding should be...
};

struct qsbr_globals *qg ALIGNED(CACHE_LINE_SIZE);

struct qsbr_data {
    /* EBR per-thread data:
     *  limbo_list: three lists of nodes awaiting physical deletion, one
     *              for each epoch
     *  in_critical: flag telling us whether we're in a critical section
     *               with respect to memory reclamation
     *  entries_since_update: the number of times we have entered a critical 
     *                        section in the current epoch since trying to
     *                        update the global epoch
     *  epoch: the local epoch
     */
    mr_node_t *limbo_list [N_EPOCHS];
    int entries_since_update;
    int epoch;
    int in_critical;
    int rcount;
    char padding[CACHE_LINE_SIZE - 4 * sizeof(int) - N_EPOCHS * sizeof(mr_node_t*)];
};

struct qsbr_aux_data {

    uint64_t thread_index;
    uint64_t nthreads;
    char padding[CACHE_LINE_SIZE - 2*sizeof(uint64_t)];
};

typedef ALIGNED(CACHE_LINE_SIZE) struct qsbr_data qsbr_data_t;
typedef ALIGNED(CACHE_LINE_SIZE) struct qsbr_aux_data qsbr_aux_data_t;

qsbr_data_t *qd;
__thread qsbr_aux_data_t qad;


int update_epoch()
{
    int curr_epoch;
    int i;
    
    // if (!spin_trylock(&qg->update_lock)) {
    if (!TRYLOCK(&qg->update_lock)) {
        /* Someone could be preempted while holding the update lock. Give
         * them the CPU. */
        /*cond_yield();*/
        return 0;
    }
    
    /* If any CPU hasn't advanced to the current epoch, abort the attempt. */
    curr_epoch = qg->global_epoch;
    for (i = 0; i < qad.nthreads; i++) {
        if (qd[i].in_critical == 1 && 
            qd[i].epoch != curr_epoch &&
            i != qad.thread_index) {
            UNLOCK(&qg->update_lock);
            /*cond_yield();*/
            return 0;
        }
    }
    
    /* Update the global epoch. 
     * 
     * I wanted to use CAS here, but that would be unsafe due to 
     * wraparound. */
    qg->global_epoch = (curr_epoch + 1) % N_EPOCHS;
    
    UNLOCK(&(qg->update_lock));
    return 1;
}

void mr_init_local(uint64_t thread_index, uint64_t nthreads) {
    qad.thread_index = thread_index;
    qad.nthreads = nthreads;
}

void mr_init_global(uint64_t nthreads) {
    int i, j;
    
    qg = (struct qsbr_globals *)malloc(sizeof(struct qsbr_globals));

    qd = (qsbr_data_t *)malloc(nthreads * sizeof(qsbr_data_t));

    for (i = 0; i < nthreads; i++) {
        qd[i].epoch = 0;
        qd[i].in_critical = 1;
        qd[i].rcount = 0;
        for (j = 0; j < N_EPOCHS; j++)
            qd[i].limbo_list[j] = NULL;
    }
    
    qg->global_epoch = 1;
    INIT_LOCK(&(qg->update_lock));
}

void mr_thread_exit()
{
    qd[qad.thread_index].in_critical = 0;

    while(qd[qad.thread_index].rcount > 0) {
        quiescent_state(FUZZY);
        // cond_yield();
        sched_yield();
    }
}

void mr_reinitialize()
{
    int i;
    
    for (i = 0; i < qad.nthreads; i++) {
        qd[i].epoch = 0;
        qd[i].in_critical = 1;
        qd[i].rcount = 0;
    }

    qg->global_epoch = 1;
    INIT_LOCK(&(qg->update_lock));
}

/* Processes a list of callbacks.
 *
 * @list: Pointer to list of node_t's.
 */
void process_callbacks(mr_node_t **list)
{
    mr_node_t *next;
    uint64_t num = 0;
    
    // write_barrier(); // IGOR why you do this?
    MEM_BARRIER;

    for (; (*list) != NULL; (*list) = next) {
        next = (*list)->mr_next;
        
        ((node_t *)((*list)->actual_node))->key = 10000;
        // ssfree_alloc(0, (*list)->actual_node);
        // ssfree_alloc(1, *list);
        num++;
    }

    /* Update our accounting information. */
    qd[qad.thread_index].rcount -= num;
}

/* 
 * Informs other threads that this thread has passed through a quiescent 
 * state.
 * If all threads have passed through a quiescent state since the last time
 * this thread processed it's callbacks, proceed to process pending callbacks.
 */
void quiescent_state (int blocking)
{
    // struct per_thread *t = this_thread();
    qsbr_data_t *t = &(qd[qad.thread_index]);
    int epoch;
    int orig;

 retry:    
    epoch = qg->global_epoch;
    if (t->epoch != epoch) { /* New epoch. */
        /* Process callbacks for old 'incarnation' of this epoch. */
        process_callbacks(&(t->limbo_list[epoch]));
        t->epoch = epoch;
    } else {
        orig = t->in_critical;
        t->in_critical = 0;
        int res = update_epoch();
        // fprintf(stderr, "Update epoch returned %d\n", res);
        if (res) {
            t->in_critical = orig; 
            MEM_BARRIER;
            epoch = qg->global_epoch;
            if (t->epoch != epoch) {
                process_callbacks(&(t->limbo_list[epoch]));
                t->epoch = epoch;
            }
            return;
        } else if (blocking) {
            t->in_critical = orig; 
            MEM_BARRIER;
            sched_yield();
            goto retry;
        }
        t->in_critical = orig; 
        MEM_BARRIER;//memory_barrier();

    }
    
    return;
}

/* Links the node into the per-thread list of pending deletions.
 */
void free_node_later (void *q)
{
    qsbr_data_t *t = &(qd[qad.thread_index]);

    mr_node_t* wrapper_node = ssalloc_alloc(1, sizeof(mr_node_t));
    wrapper_node->actual_node = q;

    wrapper_node->mr_next = t->limbo_list[t->epoch];
    t->limbo_list[t->epoch] = wrapper_node;
    t->rcount++;
}
