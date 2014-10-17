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
#include "test.h"
#include <stdio.h>
#include "include/atomic_ops_if.h"
#include "util.h"
#include "allocator.h"
//#include "arch/atomic.h"

#define NOT_ENTERED 0
#define ENTERED 1
#define DONE 2

struct qsbr_globals {
    spinlock_t update_lock __attribute__ ((__aligned__ (CACHESIZE)));
    int global_epoch __attribute__ ((__aligned__ (CACHESIZE)));
};

struct qsbr_globals *qg __attribute__ ((__aligned__ (CACHESIZE)));

int update_epoch()
{
    int curr_epoch;
    int i;
    int old;
    int myTID = getTID();
    
    if (!spin_trylock(&qg->update_lock)) {
        /* Someone could be preempted while holding the update lock. Give
         * them the CPU. */
        /*cond_yield();*/
        return 0;
    }
    
    /* If any CPU hasn't advanced to the current epoch, abort the attempt. */
    curr_epoch = qg->global_epoch;
    for (i = 0; i < tg->nthreads; i++) {
        if (get_thread(i)->in_critical == 1 && 
            get_thread(i)->epoch != curr_epoch &&
            i != myTID) {
            spin_unlock(&qg->update_lock);
            /*cond_yield();*/
            return 0;
        }
    }
    
    /* Update the global epoch. 
     * 
     * I wanted to use CAS here, but that would be unsafe due to 
     * wraparound. */
    qg->global_epoch = (curr_epoch + 1) % N_EPOCHS;
    
    spin_unlock(&qg->update_lock);
    return 1;
}

void mr_init()
{
    int i, j;
    
    qg = (struct qsbr_globals *)mapmem(sizeof(struct qsbr_globals));

    for (i = 0; i < MAX_THREADS+1; i++) {
        get_thread(i)->epoch = 0;
        get_thread(i)->in_critical = 1;
        get_thread(i)->rcount = 0;
        for (j = 0; j < N_EPOCHS; j++)
            get_thread(i)->limbo_list[j] = NULL;
    }
    
    qg->global_epoch = 1;
    qg->update_lock = SPIN_LOCK_UNLOCKED;
}

void mr_thread_exit()
{
    this_thread()->in_critical = 0;

     while(this_thread()->rcount > 0) {
        quiescent_state(FUZZY);
        cond_yield();
    }
}

void mr_reinitialize()
{
    int i;
    
    for (i = 0; i < MAX_THREADS+1; i++) {
        get_thread(i)->epoch = 0;
        get_thread(i)->in_critical = 1;
        get_thread(i)->rcount = 0;
    }

    qg->global_epoch = 1;
    qg->update_lock = SPIN_LOCK_UNLOCKED;
}

/* Processes a list of callbacks.
 *
 * @list: Pointer to list of node_t's.
 */
void process_callbacks(node_t **list)
{
    node_t *next;
    node_t *dead;
    uint64_t num = 0;
    
    write_barrier(); // IGOR why you do this?
    
    for (; (*list) != NULL; (*list) = next) {
        next = (*list)->mr_next;
        free_node(*list);
        num++;
    }

    /* Update our accounting information. */
    this_thread()->rcount -= num;
}

/* 
 * Informs other threads that this thread has passed through a quiescent 
 * state.
 * If all threads have passed through a quiescent state since the last time
 * this thread processed it's callbacks, proceed to process pending callbacks.
 */
void quiescent_state (int blocking)
{
    struct per_thread *t = this_thread();
    int epoch;
    int orig;

 retry:    
    epoch = qg->global_epoch;
    if (t->epoch != epoch) { /* New epoch. */
        /* Process callbacks for old 'incarnation' of this epoch. */
        process_callbacks(&t->limbo_list[epoch]);
        t->epoch = epoch;
    } else {
        orig = t->in_critical;
        t->in_critical = 0;
        if (update_epoch()) {
            t->in_critical = orig; memory_barrier();
            epoch = qg->global_epoch;
            if (t->epoch != epoch) {
                process_callbacks(&t->limbo_list[epoch]);
                t->epoch = epoch;
            }
            return;
        } else if (blocking) {
            t->in_critical = orig; memory_barrier();
            cond_yield();
            goto retry;
        }
        t->in_critical = orig; memory_barrier();
    }
    
    return;
}

/* Links the node into the per-thread list of pending deletions.
 */
void free_node_later (node_t *q)
{
    struct per_thread *t = this_thread();
    q->mr_next = t->limbo_list[t->epoch];
    t->limbo_list[t->epoch] = q;
    t->rcount++;
}
