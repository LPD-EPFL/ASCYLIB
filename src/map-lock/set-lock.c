/*   
 *   File: linkedlist-lock.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   linkedlist-lock.c is part of ASCYLIB
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
#include "utils.h"

__thread ssmem_allocator_t* alloc;

intset_l_t*
set_new_l(size_t size)
{
  intset_l_t* set = ssalloc_aligned(CACHE_LINE_SIZE, sizeof(intset_l_t));
  assert(set != NULL);

  set->size = size;
  set->array = (key_val_t*) ssalloc_aligned(CACHE_LINE_SIZE, size * sizeof(key_val_t));
  assert(set->array != NULL);
  memset(set->array, 0, size * sizeof(key_val_t));

  set->lock = (volatile ptlock_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(ptlock_t));
  if (set->lock == NULL)
    {
      perror("malloc");
      exit(1);
    }
  INIT_LOCK_A(set->lock);
  MEM_BARRIER;
  return set;
}

void set_delete_l(intset_l_t *set)
{
  ssfree(set->array);
  ssfree((void*) set->lock);
  ssfree(set);
}

int set_size_l(intset_l_t *set)
{
  int size = 0;
  int i;
  for (i = 0; i < set->size; i++)
    {
      size += (set->array[i].key != 0);
    }
  return size;
}



	
