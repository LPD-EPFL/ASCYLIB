/*   
 *   File: ms.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description:  
 *   ms.c is part of ASCYLIB
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
mstack_lock_find(mstack_t* qu, skey_t key)
{ 
  return 1;
}

int
mstack_lock_insert(mstack_t* qu, skey_t key, sval_t val)
{
  mstack_node_t* node = mstack_new_node(key, val, NULL);
  LOCK_A(&qu->lock);
  node->next = qu->top;
  qu->top = node;
  UNLOCK_A(&qu->lock);
  return 1;
}

sval_t
mstack_lock_delete(mstack_t* qu)
{
  LOCK_A(&qu->lock);
  mstack_node_t* top = qu->top;
  if (unlikely(top == NULL))
    {
      UNLOCK_A(&qu->lock);
      return 0;
    }

  qu->top = top->next;
  UNLOCK_A(&qu->lock);

#if GC == 1
  ssmem_free(alloc, (void*) top);
#endif
  return top->val;
}
