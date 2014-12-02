/*   
 *   File: copy_on_write.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Similar to Java's CopyOnWriteArrayList.
 *   http://docs.oracle.com/javase/7/docs/api/java/util/concurrent/CopyOnWriteArrayList.html
 *   copy_on_write.c is part of ASCYLIB
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

#include "copy_on_write.h"

__thread ssmem_allocator_t* alloc;


size_t array_ll_fixed_size;

inline void 
cpy_delete_copy(ssmem_allocator_t* alloc, array_ll_t* a)
{
#if CPY_ON_WRITE_USE_MEM_RELEAS == 1
  ssmem_release(alloc, (void*) a);
#else
  ssmem_free(alloc, (void*) a);
#endif
}

static inline volatile array_ll_t*
array_ll_new_init(size_t size)
{
  array_ll_t* all;
  all = ssalloc_aligned(CACHE_LINE_SIZE, sizeof(array_ll_t) + (array_ll_fixed_size * sizeof(kv_t)));
  assert(all != NULL);
  
  all->size = size;
  all->kvs = (kv_t*) ((uintptr_t) all + sizeof(array_ll_t));

  return all;
}

static inline array_ll_t*
array_ll_new(size_t size)
{
  array_ll_t* all;
  all = ssmem_alloc(alloc, sizeof(array_ll_t) + (array_ll_fixed_size * sizeof(kv_t)));
  assert(all != NULL);
  
  all->size = size;
  all->kvs = (kv_t*) ((uintptr_t) all + sizeof(array_ll_t));

  return all;
}


static copy_on_write_t*
copy_on_write_new_init()
{
  copy_on_write_t* cow;
  cow = ssalloc_aligned(CACHE_LINE_SIZE, sizeof(copy_on_write_t));
  assert(cow != NULL);
  cow->lock = ssalloc_aligned(CACHE_LINE_SIZE, sizeof(ptlock_t));
  assert(cow->lock != NULL);

  INIT_LOCK_A(cow->lock);

  cow->array = array_ll_new_init(0);
  return cow;
}

copy_on_write_t*
copy_on_write_new()
{
  return copy_on_write_new_init();
}


sval_t
cpy_search(copy_on_write_t* set, skey_t key) 
{
  array_ll_t* all_cur = (array_ll_t*) set->array;

  int i;
  for (i = 0; i < all_cur->size; i++)
    {
      if (unlikely(all_cur->kvs[i].key == key))
	{
	  return all_cur->kvs[i].val;
	}
    }

  return 0;
}

sval_t
cpy_delete(copy_on_write_t* set, skey_t key)
{
#if CPY_ON_WRITE_READ_ONLY_FAIL == 1
  if (cpy_search(set, key) == 0)
    {
      return 0;
    }
#endif

  sval_t removed = 0;

  LOCK_A(set->lock);

  volatile array_ll_t* all_old = set->array;
  array_ll_t* all_new = array_ll_new(all_old->size - 1);

  int i, n;
  for (i = 0, n = 0; i < all_old->size; i++, n++)
    {
      if (unlikely(all_old->kvs[i].key == key))
	{
	  removed = all_old->kvs[i].val;
	  n--;
	  continue;
	}

      all_new->kvs[n].key = all_old->kvs[i].key;
      all_new->kvs[n].val = all_old->kvs[i].val;
    }

#ifdef __tile__
  MEM_BARRIER;
#endif

  if (removed)
    {
      set->array = all_new;
      cpy_delete_copy(alloc, (void*) all_old);
    }
  else
    {
      cpy_delete_copy(alloc, (void*) all_new);
    }

  UNLOCK_A(set->lock);
  return removed;
}

int
cpy_insert(copy_on_write_t* set, skey_t key, sval_t val) 
{

#if CPY_ON_WRITE_READ_ONLY_FAIL == 1
  if (cpy_search(set, key) != 0)
    {
      return 0;
    }
#endif

  LOCK_A(set->lock);

  volatile array_ll_t* all_old = set->array;
  array_ll_t* all_new = array_ll_new(all_old->size + 1);


  int i;
  for (i = 0; i < all_old->size; i++)
    {
      if (unlikely(all_old->kvs[i].key == key))
	{
	  cpy_delete_copy(alloc, all_new);
	  UNLOCK_A(set->lock);
	  return 0;
	}
      all_new->kvs[i].key = all_old->kvs[i].key;
      all_new->kvs[i].val = all_old->kvs[i].val;
    }

  all_new->kvs[i].key = key;
  all_new->kvs[i].val = val;
  
#ifdef __tile__
  MEM_BARRIER;
#endif

  set->array = all_new;
  cpy_delete_copy(alloc, (void*) all_old);

  UNLOCK_A(set->lock);
  return 1;
}

size_t
copy_on_write_size(copy_on_write_t* set)
{
  return set->array->size;
};
