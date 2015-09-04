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

#include "queue-ms.h"
#include "utils.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

sval_t
queue_ms_find(queue_t* qu, skey_t key)
{ 
  return 1;
}

LOCK_LOCAL_DATA;

int
queue_ms_insert(queue_t* qu, skey_t key, sval_t val)
{
  queue_node_t* node = queue_new_node(key, val, NULL);
  LOCK_A(&qu->tail_lock);
  qu->tail->next = node;
  qu->tail = node; 
  UNLOCK_A(&qu->tail_lock);
  return 1;
}


sval_t
queue_ms_delete(queue_t* qu)
{
  NUM_RETRIES();
  queue_node_t* next, *head;
  while (1)
    {
      head = qu->head;
      queue_node_t* tail = qu->tail;
      next = head->next;
      if (likely(head == qu->head))
	{
	  if (head == tail)
	    {
	      if (next == NULL)
		{
		  return 0;
		}
	      UNUSED void* dummy = CAS_PTR(&qu->tail, tail, next);
	    }
	  else
	    {
	      if (CAS_PTR(&qu->head, head, next) == head)
		{
		  break;
		}
	    }
	}
      DO_PAUSE();
    }

  sval_t val = next->val;
#if GC == 1
  ssmem_free(alloc, (void*) head);
#endif
  return val;
}
