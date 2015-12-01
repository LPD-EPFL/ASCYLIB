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

size_t elim_num = 0;
size_t elim_num_try = 0;


const int n_spin_elim = 1<<14;

int
mstack_optik_insert(mstack_t* qu, skey_t key, sval_t val)
{
  while (1)
    {
      COMPILER_NO_REORDER(optik_t version = qu->lock;);
      mstack_node_t* node = mstack_new_node(key, val, NULL);
      node->next = qu->top;
      if (optik_trylock_version(&qu->lock, version))
	{
	  qu->top = node;
	  optik_unlock(&qu->lock);
	  break;
	}

      ssmem_free(alloc, (void*) node);
      
      FAI_U64(&elim_num_try);

      int i;
      /* for (i = 0; i < MSTACK_NUM_ELIM; i++) */
      for (i = 0; i < (MSTACK_NUM_ELIM * MSTACK_NUM_ELIM); i += MSTACK_NUM_ELIM)
	{
	  mstack_req_t req = qu->elim[i];
	  if (req && !mstack_req_is_push(req))
	    {
	      if (CAS_PTR(&qu->elim[i], req, NULL) == req)
		{
		  /* printf("PUSH-[%-4zu] @ %d: found req\n", key, i); */
		  return 1;
		}
	    }
	  else if (req == NULL)
	    {
	      mstack_req_t node = (mstack_req_t) mstack_new_node(key, val, NULL);
	      mstack_req_t nodep = mstack_req_set_push(node);
	      if (CAS_PTR(&qu->elim[i], NULL, nodep) == NULL)
		{
		  /* printf("PUSH-[%-4zu] @ %d: inserted req\n", key, i); */
		  /* cpause(2048); */
		  volatile int nt = n_spin_elim;
		  while (qu->elim[i] == nodep && nt-- > 0)
		    {
		      OPTIK_PAUSE();
		    }
		  if (qu->elim[i] != nodep)
		    {
		      FAI_U64(&elim_num);
		      /* printf("PUSH-[%-4zu] @ %d: req consumed 0\n", key, i); */
		      return 1;
		    }
		  else if (CAS_PTR(&qu->elim[i], nodep, NULL) != nodep)
		    {
		      FAI_U64(&elim_num);
		      /* printf("PUSH-[%-4zu] @ %d: req consumed 1\n", key, i); */
		      return 1;
		    }
		  else
		    {
		      ssmem_free(alloc, (void*) node);
		      /* break; */
		      /* printf("PUSH-[%-4zu] @ %d: req retracted\n", key, i); */
		    }
		}
	    }
	}

      /* cpause(rand() % (++nr)); */
    }
  return 1;
}

sval_t
mstack_optik_delete(mstack_t* qu)
{
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

      FAI_U64(&elim_num_try);

      int i;
      for (i = 0; i < (MSTACK_NUM_ELIM * MSTACK_NUM_ELIM); i += MSTACK_NUM_ELIM)
      /* for (i = 0; i < MSTACK_NUM_ELIM; i++) */
	{
	  mstack_req_t req = qu->elim[i];
	  if (req && mstack_req_is_push(req))
	    {
	      if (CAS_PTR(&qu->elim[i], req, NULL) == req)
		{
		  /* printf("POP -[%-4zu] @ %d: found req\n", req->key, i); */
		  return 1;
		}
	    }
	  else if (req == NULL)
	    {
	      mstack_req_t node = (mstack_req_t) mstack_new_node(0, 0, NULL);
	      if (CAS_PTR(&qu->elim[i], NULL, node) == NULL)
		{
		  /* printf("POP -[%-4zu] @ %d: inserted req\n", 0, i); */
		  volatile int nt = n_spin_elim;
		  while (qu->elim[i] == node && nt-- > 0)
		    {
		      OPTIK_PAUSE();
		    }
		  if (qu->elim[i] != node)
		    {
		      FAI_U64(&elim_num);
		      /* printf("POP -[%-4zu] @ %d: req consumed 0\n", 0, i); */
		      return 1;
		    }
		  else if (CAS_PTR(&qu->elim[i], node, NULL) != node)
		    {
		      FAI_U64(&elim_num);
		      /* printf("POP -[%-4zu] @ %d: req consumed 1\n", 0, i); */
		      return 1;
		    }
		  else
		    {
		      ssmem_free(alloc, (void*) node);
		      /* break; */
		      /* printf("POP -[%-4zu] @ %d: req retracted\n", 0, i); */
		    }
		}
	    }
	}

      /* cpause(rand() % (nr++)); */
    }

#if GC == 1
  ssmem_free(alloc, (void*) top);
#endif
  return top->val;
}
