#include <stdio.h>
#include <stdlib.h>
#include "qsbr_smr_hybrid.h"
#include "mr.h"
#include "ssalloc.h"
#include "atomic_ops_if.h"
#include "utils.h"
#include "lock_if.h"



struct qsbr_globals *qg ALIGNED(CACHE_LINE_SIZE);

shared_thread_data_t *shtd;

__thread local_thread_data_t ltd;

int compare (const void *a, const void *b);

void mr_init_local(uint64_t thread_index, uint64_t nthreads) {
    ltd.thread_index = thread_index;
    ltd.nthreads = nthreads;
    ltd.rcount = 0;
    ltd.plist = (mr_node_t **) malloc(sizeof(mr_node_t *) * K * nthreads);
}

// TODO IGOR OANA add NULL verification after memory allocation
void mr_init_global(uint64_t nthreads) {
    int i, j;
    
    qg = (struct qsbr_globals *) malloc(sizeof(struct qsbr_globals));
    qg->global_epoch = 1;
    INIT_LOCK(&(qg->update_lock));

    shtd = (shared_thread_data_t *) malloc(nthreads * sizeof(shared_thread_data_t));

    for (i = 0; i < nthreads; i++) {
        shtd[i].epoch = 0;
        shtd[i].in_critical = 1;
        for (j = 0; j < N_EPOCHS; j++)
            shtd[i].limbo_list[j] = NULL;
    }

    HP = (hazard_pointer_t *)malloc(sizeof(hazard_pointer_t) * K*nthreads);

    for (i = 0; i < K*(nthreads); i++) {
        HP[i].p = NULL;
    }

    fallback.flag = 0;
    
}

void mr_thread_exit()
{
    // Hazard pointer-style exit every time just to be safe
    int i;
    
    for (i = 0; i < K; i++)
        HP[K * ltd.thread_index + i].p = NULL;
    
    while (ltd.rcount > 0) {
        scan();
        sched_yield();
    }

    fprintf(stderr, "Thread [%d] exiting. Fallback flag is [%d]\n", ltd.thread_index, fallback.flag);
}

void mr_reinitialize()
{
}

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
    for (i = 0; i < ltd.nthreads; i++) {
        if (shtd[i].in_critical == 1 && 
            shtd[i].epoch != curr_epoch &&
            i != ltd.thread_index) {
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
        ssfree_alloc(0, (*list)->actual_node);
        ssfree_alloc(1, *list);
        num++;
    }

    /* Update our accounting information. */
    ltd.rcount -= num;
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
    uint64_t my_index = ltd.thread_index;
    int epoch;
    int orig;

 retry:    
    epoch = qg->global_epoch;
    if (shtd[my_index].epoch != epoch) { /* New epoch. */
        /* Process callbacks for old 'incarnation' of this epoch. */
        process_callbacks(&(shtd[my_index].limbo_list[epoch]));
        shtd[my_index].epoch = epoch;
    } else {
        orig = shtd[my_index].in_critical;
        shtd[my_index].in_critical = 0;
        int res = update_epoch();
        if (res) {
            shtd[my_index].in_critical = orig; 
            MEM_BARRIER;
            epoch = qg->global_epoch;
            if (shtd[my_index].epoch != epoch) {
                process_callbacks(&(shtd[my_index].limbo_list[epoch]));
                shtd[my_index].epoch = epoch;
            }
            return;
        } else if (blocking) {
            shtd[my_index].in_critical = orig; 
            MEM_BARRIER;
            sched_yield();
            goto retry;
        }
        shtd[my_index].in_critical = orig; 
        MEM_BARRIER;
    }
    
    return;
}

/* Links the node into the per-thread list of pending deletions.
 */
void free_node_later (void *q)
{
    uint64_t my_index = ltd.thread_index;

    mr_node_t* wrapper_node = ssalloc_alloc(1, sizeof(mr_node_t));
    wrapper_node->actual_node = q;

    wrapper_node->mr_next = shtd[my_index].limbo_list[shtd[my_index].epoch];
    shtd[my_index].limbo_list[shtd[my_index].epoch] = wrapper_node;
    ltd.rcount++;
}

// Hazard Pointers Specific
void scan()
{
    /* Iteratation variables. */
    mr_node_t *cur;
    int i,j;
    uint64_t my_index = ltd.thread_index;

    /* List of SMR callbacks. */
    mr_node_t *tmplist;

    /* List of hazard pointers, and its size. */
    mr_node_t **plist = ltd.plist;
    uint64_t psize;

    /*
     * Make sure that the most recent node to be deleted has been unlinked
     * in all processors' views.
     */
    // write_barrier();
    MEM_BARRIER;

    /* Stage 1: Scan HP list and insert non-null values in plist. */
    psize = 0;
    for (i = 0; i < H; i++) {
        if (HP[i].p != NULL)
            plist[psize++] = HP[i].p;
    }
    
    /* Stage 2: Sort the plist. */
    qsort(plist, psize, sizeof(mr_node_t *), compare);

    /* Stage 3: Free non-harzardous nodes. */
    //OANA Modified Scan (a lot)
    ltd.rcount = 0;
    for (j = 0; j < N_EPOCHS; j++){
        tmplist = shtd[my_index].limbo_list[j];
        shtd[my_index].limbo_list[j] = NULL;

        //tmplist = this_thread()->rlist;
        //this_thread()->rlist = NULL;
        //this_thread()->rcount = 0;
        while (tmplist != NULL) {
            /* Pop cur off top of tmplist. */
            cur = tmplist;
            tmplist = tmplist->mr_next;

            if (bsearch(&cur, plist, psize, sizeof(mr_node_t *), compare)) {
                //cur->mr_next = this_thread()->rlist;
                //this_thread()->rlist = cur;

                cur->mr_next = shtd[my_index].limbo_list[j];
                shtd[my_index].limbo_list[j] = cur;
                ltd.rcount++;
            } else {
                ssfree_alloc(0, cur->actual_node);
                ssfree_alloc(1, cur);
            }
        }
    }
}

// UTILITY FUNCTIONS

/* 
 * Comparison function for qsort.
 *
 * We just need any total order, so we'll use the arithmetic order 
 * of pointers on the machine.
 *
 * Output (see "man qsort"):
 *  < 0 : a < b
 *    0 : a == b
 *  > 0 : a > b
 */
int compare (const void *a, const void *b)
{
  return ( *(mr_node_t **)a - *(mr_node_t **)b );
}