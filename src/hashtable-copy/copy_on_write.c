/*   
 *   File: copy_on_write.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Similar to Java's CopyOnWriteArrayList. One array per bucket. 
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
copy_on_write_new_init(size_t num_buckets)
{
  copy_on_write_t* cow;
  cow = ssalloc_aligned(CACHE_LINE_SIZE, sizeof(copy_on_write_t));
  assert(cow != NULL);
  
  cow->num_buckets = num_buckets;
  cow->hash = cow->num_buckets - 1;

#if defined(LL_GLOBAL_LOCK)
  cow->lock = ssalloc_aligned(CACHE_LINE_SIZE, sizeof(ptlock_t));
  assert(cow->lock != NULL);
  GL_INIT_LOCK(cow->lock);
#else
  cow->lock = ssalloc_aligned(CACHE_LINE_SIZE, cow->num_buckets * sizeof(ptlock_t));
  assert(cow->lock != NULL);
#endif

  cow->array = ssalloc_aligned(CACHE_LINE_SIZE, cow->num_buckets * sizeof(array_ll_t*));
  assert(cow->array != NULL);

  int i;
  for (i = 0; i < cow->num_buckets; i++)
    {
      INIT_LOCK(cow->lock + i);
      cow->array[i] = array_ll_new_init(0);
    }

  return cow;
}

copy_on_write_t*
copy_on_write_new(size_t num_buckets)
{
  return copy_on_write_new_init(num_buckets);
}


sval_t
cpy_search(copy_on_write_t* set, skey_t key) 
{
  size_t bucket = key & set->hash;

  volatile array_ll_t* all_cur = set->array[bucket];

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
cpy_array_search(array_ll_t* all_cur, skey_t key) 
{
  int i;
  for (i = 0; i < all_cur->size; i++)
    {
      if (unlikely(all_cur->kvs[i].key == key))
	{
	  return 1;
	}
    }

  return 0;
}

sval_t
cpy_delete(copy_on_write_t* set, skey_t key)
{
  size_t bucket = key & set->hash;
  array_ll_t* all_old;

#if CPY_ON_WRITE_READ_ONLY_FAIL == 1
  all_old = (array_ll_t*) set->array[bucket];
  if (cpy_array_search(all_old, key) == 0)
    {
      return 0;
    }
#endif

  sval_t removed = 0;

  GL_LOCK(set->lock);
  LOCK(set->lock + bucket);
  all_old = (array_ll_t*) set->array[bucket];
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

  if (removed)
    {
      set->array[bucket] = all_new;
      ssmem_free(alloc, (void*) all_old);
    }
  else
    {
      ssmem_free(alloc, (void*) all_new);
    }

  GL_UNLOCK(set->lock);
  UNLOCK(set->lock + bucket);
  return removed;
}

int
cpy_insert(copy_on_write_t* set, skey_t key, sval_t val) 
{
  size_t bucket = key & set->hash;
  array_ll_t* all_old;

#if CPY_ON_WRITE_READ_ONLY_FAIL == 1
  all_old = (array_ll_t*) set->array[bucket];
  if (cpy_array_search(all_old, key) == 1)
    {
      return 0;
    }
#endif

  GL_LOCK(set->lock);
  LOCK(set->lock + bucket);

  all_old = (array_ll_t*) set->array[bucket];
  array_ll_t* all_new = array_ll_new(all_old->size + 1);

  int i;
  for (i = 0; i < all_old->size; i++)
    {
      if (unlikely(all_old->kvs[i].key == key))
	{
	  ssmem_free(alloc, (void*) all_new);
	  GL_UNLOCK(set->lock);
	  UNLOCK(set->lock + bucket);
	  return 0;
	}
      all_new->kvs[i].key = all_old->kvs[i].key;
      all_new->kvs[i].val = all_old->kvs[i].val;
    }

  all_new->kvs[i].key = key;
  all_new->kvs[i].val = val;
  
  set->array[bucket] = all_new;
  ssmem_free(alloc, (void*) all_old);

  GL_UNLOCK(set->lock);
  UNLOCK(set->lock + bucket);
  return 1;
}

size_t
copy_on_write_size(copy_on_write_t* set)
{
  size_t s = 0;
  int i;
  for (i = 0; i < set->num_buckets; i++)
    {
      s += set->array[i]->size;
    }

  return s;
};


