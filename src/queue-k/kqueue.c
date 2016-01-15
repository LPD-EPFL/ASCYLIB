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

#include "kqueue.h"
#include "utils.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

__thread size_t __pq_thread_id;

static inline queue_low_t*
pqueue_get_low_local(pqueue_t* pq)
{
  return pq->queues + __pq_thread_id;
}

sval_t
pqueue_optik_find(pqueue_t* qu, skey_t key)
{ 
  return 1;
}

int
pqueue_optik_insert(pqueue_t* qu, skey_t key, sval_t val)
{
  queue_low_t* ql = pqueue_get_low_local(qu);
  int ret = queue_low_push(ql, key, val);
  return ret;
}


sval_t
pqueue_optik_delete(pqueue_t* qu)
{
  queue_low_t* ql = pqueue_get_low_local(qu);
  sval_t ret = queue_low_pop(ql, 1);
  return ret;
}
