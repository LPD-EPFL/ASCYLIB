/*   
 *   File: intset.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   intset.c is part of ASCYLIB
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

#include "intset.h"

inline sval_t
queue_contains(queue_t *set, skey_t key)
{
  return queue_optik_find(set, key);
}

inline int
queue_add(queue_t *set, skey_t key, sval_t val)
{  
  return queue_optik_insert(set, key, val);
}

inline sval_t
queue_remove(queue_t *set)
{
  return queue_optik_delete(set);
}

inline int
queue_add_seq(queue_t *set, skey_t key, sval_t val)
{  
  return queue_optik_insert_seq(set, key, val);
}

inline sval_t
queue_remove_seq(queue_t *set)
{
  return queue_optik_delete_seq(set);
}
