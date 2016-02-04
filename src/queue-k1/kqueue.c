/*   
 *   File: kqueue.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description:  
 *   kqueue.c is part of ASCYLIB
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

static inline queue_low_t*
pqueue_get_low_n(pqueue_t* pq, const int n)
{
  return pq->queues + n;
}


sval_t
pqueue_optik_find(pqueue_t* qu, skey_t key)
{ 
  return 1;
}

#define PQUEUE_K 2

int
pqueue_optik_insert(pqueue_t* qu, skey_t key, sval_t val)
{
  queue_low_t* ql = pqueue_get_low_n(qu, 0); //pqueue_get_low_local(qu);
  queue_low_push(ql, key, val);
  return 1;
}


sval_t
pqueue_optik_delete(pqueue_t* qu)
{
  /* queue_low_t* qls[PQUEUE_K]; */
  /* qls[0] = pqueue_get_low_local(qu); */
  /* skey_t key_min = queue_low_get_min(qls[0]); */
  /* int key_min_i = 0; */

  /* int k; */
  /* for (k = 1; k < PQUEUE_K && k < qu->size; k++) */
  /*   { */
  /*     int qi = mrand(seeds) % PQUEUE_K; */
  /*     qls[k] = qu->queues + qi; */
  /*     skey_t key = queue_low_get_min(qls[k]); */
  /*     if (key < key_min) */
  /* 	{ */
  /* 	  key_min = key; */
  /* 	  key_min_i = k; */
  /* 	} */
  /*   } */

  /* sval_t ret = queue_low_pop(qls[key_min_i]); */
  queue_low_t* ql = pqueue_get_low_n(qu, 0); //pqueue_get_low_local(qu);
  sval_t ret = queue_low_pop(ql);
  return ret;
}
