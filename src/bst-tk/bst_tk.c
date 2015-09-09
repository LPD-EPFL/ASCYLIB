/*   
 *   File: bst_tk.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Asynchronized Concurrency: The Secret to Scaling Concurrent
 *    Search Data Structures, Tudor David, Rachid Guerraoui, Vasileios Trigonakis,
 *   ASPLOS '15
 *   bst_tk.c is part of ASCYLIB
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
 * 	     	      Tudor David <tudor.david@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * ASCYLIB is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "bst_tk.h"

#ifdef USE_TSX
#define TSX_NUM_RETIRES 10
#endif

RETRY_STATS_VARS;

    sval_t
bst_tk_delete(intset_t* set, skey_t key)
{

#ifdef USE_TSX
    int num_retries = TSX_NUM_RETRIES;
#if TSX_STATS == 1
    tried++;
#endif
#endif
    node_t* curr;
    node_t* pred = NULL;
    node_t* ppred = NULL;
    volatile uint64_t curr_ver = 0;
    uint64_t pred_ver = 0, ppred_ver = 0, right = 0, pright = 0;

#if RETRY_STATS ==1 
    int done = 0;
#endif
retry:
#if RETRY_STATS == 1
    if (done < 2) { 
        PARSE_TRY();
        UPDATE_TRY();
        done++;
    }
#endif

    curr = set->head;

    do
    {
        curr_ver = curr->lock.to_uint64;

        ppred = pred;
        ppred_ver = pred_ver;
        pright = right;

        pred = curr;
        pred_ver = curr_ver;

        if (key < curr->key)
        {
            right = 0;
            curr = (node_t*) curr->left;
        }
        else
        {
            right = 1;
            curr = (node_t*) curr->right;
        }
    }
    while(likely(!curr->leaf));


    if (curr->key != key)
    {
        return 0;
    }


    volatile tl_t ppred_v = (volatile tl_t) ppred_ver;
    uint16_t version = ppred_v.lr[right].version;
    if (unlikely(version != ppred_v.lr[right].ticket))
    {
        goto retry;
    }

    volatile tl_t pred_v = (volatile tl_t) pred_ver;
    uint16_t v0 = pred_v.lr[0].version;
    uint16_t v1 = pred_v.lr[1].version;

    if (unlikely(v0 != pred_v.lr[0].ticket || v1 != pred_v.lr[1].ticket))
    {
        goto retry;
    }

#ifdef USE_TSX
    if (num_retries > 0) { 
        num_retries --;
        if (_xbegin() ==_XBEGIN_STARTED) {
            version = ppred_v.lr[right].version;
            tl32_t tlo = { .version = version, .ticket = version };
            /*uint64_t tlob = (uint64_t) pred_ver ;*/

            if ((version != ppred_v.lr[right].ticket) || (ppred->lock.lr[right].to_uint32 != tlo.to_uint32))
            {
                _xabort(0xff);
                goto retry;
            }

            if ((v0 != pred_v.lr[0].ticket) || (v1 != pred_v.lr[1].ticket) || (pred->lock.to_uint64 != pred_ver))
            {
                _xabort(0xff);
                goto retry;
            }
            if (pright)
            {
                if (right)
                {
                    ppred->right = pred->left;
                }
                else
                {
                    ppred->right = pred->right;
                }
            }
            else
            {
                if (right)
                {
                    ppred->left = pred->left;
                }
                else
                {
                    ppred->left = pred->right;
                }
            }

            tl32_t tln = {.version = (version + 1), .ticket = (version + 1)};
            ppred->lock.lr[right].to_uint32 = tln.to_uint32;
            pred->lock.to_uint64 = TLN_REMOVED;
            _xend();
            return curr->val;
        } else {

            goto retry;
        }
    }

#if TSX_STATS == 1
    locked++;
#endif
#endif

    if ((!tl_trylock_version(&ppred->lock, (volatile tl_t*) &ppred_ver, pright)))
    {
        goto retry;
    }

    if ((!tl_trylock_version_both(&pred->lock, (volatile tl_t*) &pred_ver)))
    {
        tl_revert(&ppred->lock, pright);
        goto retry;
    }

    if (pright)
    {
        if (right)
        {
            ppred->right = pred->left;
        }
        else
        {
            ppred->right = pred->right;
        }
    }
    else
    {
        if (right)
        {
            ppred->left = pred->left;
        }
        else
        {
            ppred->left = pred->right;
        }
    }

#if SLOW_CORE==1
    if (slow_thread == 1) {
        uint32_t num =  my_random(&(seeds[0]), &(seeds[1]), &(seeds[2]));
        if (num % SLOW_RATE == 0) {
            ticks del = (num % 999) * DELAY_US;
            MEM_BARRIER;
            cpause(del);              
            MEM_BARRIER;
        }
    }
#endif

    tl_unlock(&ppred->lock, pright);

#if GC == 1
    ssmem_free(alloc, curr);
    ssmem_free(alloc, pred);
#endif

    return curr->val;
}

    sval_t
bst_tk_find(intset_t* set, skey_t key) 
{
    PARSE_TRY();

    node_t* curr = set->head;

    while (likely(!curr->leaf))
    {
        if (key < curr->key)
        {
            curr = (node_t*) curr->left;
        }
        else
        {
            curr = (node_t*) curr->right;
        }
    }

    if (curr->key == key)
    {
        return curr->val;
    }  

    return 0;
}

    int
bst_tk_insert(intset_t* set, skey_t key, sval_t val) 
{
#ifdef USE_TSX
    int num_retries = TSX_NUM_RETRIES;
#if TSX_STATS == 1
    tried++;
#endif
#endif
    node_t* curr;
    node_t* pred = NULL;
    volatile uint64_t curr_ver = 0;
    uint64_t pred_ver = 0, right = 0;

#if RETRY_STATS ==1 
    int done = 0;
#endif
retry:
#if RETRY_STATS == 1
    if (done < 2) { 
        PARSE_TRY();
        UPDATE_TRY();
        done++;
    }
#endif

    curr = set->head;

    do
    {
        curr_ver = curr->lock.to_uint64;

        pred = curr;
        pred_ver = curr_ver;

        if (key < curr->key)
        {
            right = 0;
            curr = (node_t*) curr->left;
        }
        else
        {
            right = 1;
            curr = (node_t*) curr->right;
        }
    }
    while(likely(!curr->leaf));


    if (curr->key == key)
    {
        return 0;
    }

    node_t* nn = new_node(key, val, NULL, NULL, 0);
    node_t* nr = new_node_no_init();



    volatile tl_t pred_v = (volatile tl_t) pred_ver;
    uint16_t version = pred_v.lr[right].version;
    if (unlikely(version != pred_v.lr[right].ticket))
    {
        ssmem_free(alloc, nn);
        ssmem_free(alloc, nr);
        goto retry;
    }
#ifdef USE_TSX
    if (num_retries > 0) { 
        num_retries --;
        if (_xbegin() ==_XBEGIN_STARTED) {

            version = pred_v.lr[right].version;
            tl32_t tlo = {.version =  version, .ticket = version };
            if ((version != pred_v.lr[right].ticket) || (pred->lock.lr[right].to_uint32 != tlo.to_uint32))
            {
                ssmem_free(alloc, nn);
                ssmem_free(alloc, nr);
                _xabort(0xff);
                fprintf(stderr, "restarting\n");
                goto retry;
            }

            if (key < curr->key)
            {
                nr->key = curr->key;
                nr->left = nn;
                nr->right = curr;
            }
            else
            {
                nr->key = key;
                nr->left = curr;
                nr->right = nn;
            }

            if (right)
            {
                pred->right = nr;
            }
            else
            {
                pred->left = nr;
            }

            /*tl32_t tln = {.version = (version + 1), .ticket = (version + 1)};*/
            pred->lock.lr[right].version = version + 1;
            pred->lock.lr[right].ticket = version + 1;
            _xend();
            return 1;
        } else {
            ssmem_free(alloc, nn);
            ssmem_free(alloc, nr);
            goto retry;
        }
    }

#if TSX_STATS == 1
    locked++;
#endif
#endif
    if ((!tl_trylock_version(&pred->lock, (volatile tl_t*) &pred_ver, right)))
    {
        ssmem_free(alloc, nn);
        ssmem_free(alloc, nr);
        goto retry;
    }

    if (key < curr->key)
    {
        nr->key = curr->key;
        nr->left = nn;
        nr->right = curr;
    }
    else
    {
        nr->key = key;
        nr->left = curr;
        nr->right = nn;
    }

#if defined(__tile__)
    /* on tilera you may have store reordering causing the pointer to a new node
       to become visible, before the contents of the node are visible */
    MEM_BARRIER;
#endif	/* __tile__ */

    if (right)
    {
        pred->right = nr;
    }
    else
    {
        pred->left = nr;
    }

#if SLOW_CORE==1
    if (slow_thread == 1) {
        uint32_t num =  my_random(&(seeds[0]), &(seeds[1]), &(seeds[2]));
        if (num % SLOW_RATE == 0) {
            ticks del = (num % 999) * DELAY_US;
            MEM_BARRIER;
            cpause(del);              
            MEM_BARRIER;
        }
    }
#endif

    tl_unlock(&pred->lock, right);

    return 1;
}
