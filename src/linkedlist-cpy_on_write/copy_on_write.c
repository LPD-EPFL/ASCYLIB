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

  if (removed)
    {
      set->array = all_new;
    }
  else
    {
      ssmem_free(alloc, (void*) all_new);
    }

  UNLOCK_A(set->lock);
  return removed;
}

int
cpy_insert(copy_on_write_t* set, volatile skey_t key, sval_t val) 
{
  LOCK_A(set->lock);

  volatile array_ll_t* all_old = set->array;
  array_ll_t* all_new = array_ll_new(all_old->size + 1);


  int i;
  for (i = 0; i < all_old->size; i++)
    {
      if (unlikely(all_old->kvs[i].key == key))
	{
	  ssmem_free(alloc, all_new);
	  UNLOCK_A(set->lock);
	  return 0;
	}
      all_new->kvs[i].key = all_old->kvs[i].key;
      all_new->kvs[i].val = all_old->kvs[i].val;
    }

  all_new->kvs[i].key = key;
  all_new->kvs[i].val = val;
  
  set->array = all_new;
  ssmem_free(alloc, (void*) all_old);

  UNLOCK_A(set->lock);
  return 1;
}

size_t
copy_on_write_size(copy_on_write_t* set)
{
  return set->array->size;
};
