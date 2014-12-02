/*   
 *   File: barrier.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>, 
 *  	     Tudor David <tudor.david@epfl.ch>
 *   Description: 
 *   barrier.h is part of ASCYLIB
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

#ifndef _BARRIER_H_
#define _BARRIER_H_

/* ################################################################### *
 * BARRIER
 * ################################################################### */

typedef struct barrier 
{
  pthread_cond_t complete;
  pthread_mutex_t mutex;
  int count;
  int crossing;
} barrier_t;

static inline void
 barrier_init(barrier_t *b, int n) 
{
  pthread_cond_init(&b->complete, NULL);
  pthread_mutex_init(&b->mutex, NULL);
  b->count = n;
  b->crossing = 0;
}

static inline void 
barrier_cross(barrier_t *b) 
{
  pthread_mutex_lock(&b->mutex);
  /* One more thread through */
  b->crossing++;
  /* If not all here, wait */
  if (b->crossing < b->count) {
    pthread_cond_wait(&b->complete, &b->mutex);
  } else {
    pthread_cond_broadcast(&b->complete);
    /* Reset for next time */
    b->crossing = 0;
  }
  pthread_mutex_unlock(&b->mutex);
}

#define EXEC_IN_DEC_ID_ORDER(id, nthr)		\
  { int __i;					\
  for (__i = nthr - 1; __i >= 0; __i--)		\
    {						\
  if (id == __i)				\
    {

#define EXEC_IN_DEC_ID_ORDER_END(barrier)	\
  }						\
    barrier_cross(barrier);			\
    }}

#endif	/* _BARRIER_H_ */
