/*   
 *   File: stack-optik.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description:  
 *   stack-optik.c is part of ASCYLIB
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

int
mstack_optik_insert(mstack_t* qu, skey_t key, sval_t val)
{
  NUM_RETRIES();
  mstack_node_t* node = mstack_new_node(key, val, NULL);
  while (1)
    {
      COMPILER_NO_REORDER(optik_t version = qu->lock;);
      /* COMPILER_NO_REORDER(optik_t version = optik_get_version_wait(&qu->lock);); */
      node->next = qu->top;
      if (optik_trylock_version(&qu->lock, version))
  	{
  	  qu->top = node;
  	  optik_unlock(&qu->lock);
  	  break;
  	}

      DO_PAUSE();
    }
  return 1;
}

sval_t
mstack_optik_delete(mstack_t* qu)
{
  NUM_RETRIES();
  mstack_node_t* top;
  while (1)
    {
      COMPILER_NO_REORDER(optik_t version = qu->lock;);
      /* COMPILER_NO_REORDER(optik_t version = optik_get_version_wait(&qu->lock);); */

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

      DO_PAUSE();
    }

#if GC == 1
  ssmem_free(alloc, (void*) top);
#endif
  return top->val;
}
