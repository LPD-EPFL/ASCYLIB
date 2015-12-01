/*   
 *   File: stack-treiber.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   Treiber stack with OPTIK and some sort elimination --  
 *   DOES NOT work properly
 *   stack-treiber.c is part of ASCYLIB
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

#include "stack-lock.h"
#include "utils.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

LOCK_LOCAL_DATA;

sval_t
mstack_optik_find(mstack_t* qu, skey_t key)
{ 
  return 1;
}

size_t __elim_push = 0;
size_t __elim_pop  = 0;

const int nrr = 1023;

extern __thread unsigned long * seeds;

int
mstack_optik_insert(mstack_t* qu, skey_t key, sval_t val)
{
  size_t nr = nrr;
  while (1)
    {
      mstack_node_t* node = mstack_new_node(key, val, NULL);
      COMPILER_NO_REORDER(optik_t version = qu->lock;);
      node->next = qu->top;
      if (optik_trylock_version(&qu->lock, version))
	{
	  qu->top = node;
	  optik_unlock(&qu->lock);
	  break;
	}

      int spot = my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) & (ELIM_SPOTS-1);
      volatile mstack_node_t* nodee = SWAP_PTR(&qu->pope[spot], NULL);
      if (nodee != NULL)
	{
	  nodee->key = key;
	  nodee->val = val;
	  /* printf("push: elim!\n"); */
	  //ssmem_free(alloc, (void*) node);
	  ++__elim_push;
	  return 1;
	}

      nodee = qu->pushe[spot];
      int try_elim = (nodee == NULL && (CAS_PTR(&qu->pushe[spot], NULL, node) == NULL));
      /* cpause(my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) & nr); */
      cpause(rand() & nr);

      if (try_elim && (CAS_PTR(&qu->pushe[spot], node, NULL) != node))
	{
	  ++__elim_push;
	  return 1;
	}

      //ssmem_free(alloc, (void*) node);
    }
  return 1;
}

sval_t
mstack_optik_delete(mstack_t* qu)
{
  size_t nr = nrr;
  mstack_node_t* top;
  while (1)
    {
      COMPILER_NO_REORDER(optik_t version = qu->lock;);

      top = qu->top;
      if (unlikely(top == NULL))
	{
	  return 0;
	}

      if (optik_trylock_version(&qu->lock, version))
	{
	  qu->top = top->next;
	  optik_unlock(&qu->lock);
	  break;
	}

      int spot = my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) & (ELIM_SPOTS-1);
      volatile mstack_node_t* nodee = SWAP_PTR(&qu->pushe[spot], NULL);
      if (nodee != NULL)
	{
	  /* printf("pop:  elim!\n"); */
	  //ssmem_free(alloc, (void*) nodee);
	  ++__elim_pop;
	  return nodee->val;
	}

      int try_elim = 0;
      nodee = qu->pope[spot];
      if (nodee == NULL)
	{
	  nodee = mstack_new_node(0, 0, NULL);
	  try_elim = (CAS_PTR(&qu->pope[spot], NULL, nodee) == NULL);
	}

      /* cpause(my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) & nr);/ */
      cpause(rand() & nr);

      if (try_elim && (CAS_PTR(&qu->pope[spot], nodee, NULL) != nodee))
	{
	  //ssmem_free(alloc, (void*) nodee);
	  while (!nodee->val)
	    {
	      OPTIK_PAUSE();
	    }
	  ++__elim_pop;
	  return nodee->val;
	}

      //ssmem_free(alloc, (void*) nodee);
    }

#if GC == 1
  //ssmem_free(alloc, (void*) top);
#endif
  return top->val;
}
