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

#include "smr.h"
//#include "arch/atomic.h"
#include "ssalloc.h"
// #include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "linkedlist-smr-hart/node.h"

// IGOR: SSALLOC allocator convention:
// 0 is for actual nodes
// 1 is for m_nodes

__thread smr_data_t sd;

void mr_init_global(uint64_t nthreads){
  /* Allocate HP array. Over-allocate since the parent has pid 32. */
  // HP = (hazard_pointer *)mapmem(sizeof(hazard_pointer) * K*(MAX_THREADS+1));
  HP = (hazard_pointer_t *)malloc(sizeof(hazard_pointer_t) * K*nthreads);
  if (HP == NULL) {
    fprintf(stderr, "SMR mr_init: out of memory\n");
    exit(-1);
  }

  /* Initialize the hazard pointers. */
  int i;
  for (i = 0; i < K*(nthreads); i++)
    HP[i].p = NULL;
}

void mr_init_local(uint64_t thread_index, uint64_t nthreads){
  sd.rlist = NULL;
  sd.rcount = 0;
  sd.thread_index = thread_index;
  sd.nthreads = nthreads;
  sd.plist = (mr_node_t **) malloc(sizeof(mr_node_t *) * K * sd.nthreads);
}

// void mr_init()
// {
//   int i;


//   /* Initialize rlists. */
//   for (i = 0; i < MAX_THREADS+1; i++) {
//     get_thread(i)->rlist = NULL;
//     get_thread(i)->rcount = 0;
//     get_thread(i)->plist = (node_t **)
//         malloc(sizeof(node_t *) * K * tg->nthreads);
//   }

// }

void mr_thread_exit()
{
    int i;
    
    for (i = 0; i < K; i++)
        HP[K * sd.thread_index + i].p = NULL;
    
    while (sd.rcount > 0) {
        scan();
        sched_yield();
    }
}

void mr_reinitialize()
{
}

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

/* Debugging function. Leave it around. */
inline mr_node_t *ssearch(mr_node_t **list, int size, mr_node_t *key) {
    int i;
    for (i = 0; i < size; i++)
        if (list[i] == key)
            return list[i];
    return NULL;
}

void scan()
{
    /* Iteratation variables. */
    mr_node_t *cur;
    int i;

    /* List of SMR callbacks. */
    mr_node_t *tmplist;

    /* List of hazard pointers, and its size. */
    mr_node_t **plist = sd.plist;
    uint64_t psize;

    /*
     * Make sure that the most recent node to be deleted has been unlinked
     * in all processors' views.
     *
     * Good:
     *   A -> B -> C ---> A -> C ---> A -> C
     *                    B -> C      B -> POISON
     *
     * Illegal:
     *   A -> B -> C ---> A -> B      ---> A -> C
     *                    B -> POISON      B -> POISON
     */
    //write_barrier();
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
    tmplist = sd.rlist;
    sd.rlist = NULL;
    sd.rcount = 0;
    while (tmplist != NULL) {
        /* Pop cur off top of tmplist. */
        cur = tmplist;
        tmplist = tmplist->mr_next;

        if (bsearch(&cur, plist, psize, sizeof(mr_node_t *), compare)) {
            cur->mr_next = sd.rlist;
            sd.rlist = cur;
            sd.rcount++;
        } else {
            ((node_t *)(cur->actual_node))->key = 10000;      
            // ssfree_alloc(0, cur->actual_node);
            // ssfree_alloc(1, cur);
        }
    }
}

void free_node_later(void *n)
{
    // OANA IGOR add timestamp around here
    mr_node_t* wrapper_node = ssalloc_alloc(1, sizeof(mr_node_t));
    wrapper_node->actual_node = n;

    wrapper_node->mr_next = sd.rlist;
    sd.rlist = wrapper_node;
    sd.rcount++;

    if (sd.rcount >= R) {
        scan();
    }
}
