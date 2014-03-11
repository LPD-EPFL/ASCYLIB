#include "concurrent_hash_map.h"

__thread ssmem_allocator_t* alloc = NULL;
#ifdef DO_TSX
__thread size_t num_pause=0, num_total=0, num_xtest=0;
#endif
size_t maxhtlength = 0;


chm_t*
chm_new()
{
  chm_t* chm = memalign(CACHE_LINE_SIZE, sizeof(chm_t));
  assert(chm != NULL);
  chm->table = memalign(CACHE_LINE_SIZE, maxhtlength * sizeof(chm_t*));
  assert(chm->table != NULL);
  chm->locks = memalign(CACHE_LINE_SIZE, CHM_NUM_SEGMENTS_INIT * sizeof(ptlock_t));
  assert(chm->locks != NULL);

  chm->num_buckets = maxhtlength;
  chm->hash = maxhtlength - 1;
  chm->num_segments = CHM_NUM_SEGMENTS_INIT;
  chm->hash_seg = CHM_NUM_SEGMENTS_INIT - 1;

  int i;
  for (i = 0; i < chm->num_buckets; i++)
    {
      chm->table[i] = NULL;
    }
  for (i = 0; i < chm->num_segments; i++)
    {
      INIT_LOCK_A(&chm->locks[i]);
    }

  return chm;
}

static chm_node_t*
chm_node_new(skey_t key, sval_t val, chm_node_t* next)
{
  volatile chm_node_t* node;
#if GC == 1
  node = (volatile chm_node_t*) ssmem_alloc(alloc, sizeof(chm_node_t));
#else
  node = (volatile chm_node_t*) ssalloc(sizeof(chm_node_t));
#endif
  
  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->next = next;

#if defined(__tile__)
  /* on tilera you may have store reordering causing the pointer to a new node
     to become visible, before the contents of the node are visible */
  MEM_BARRIER;
#endif	/* __tile__ */

  return (chm_node_t*) node;
}

sval_t
chm_get(chm_t* set, skey_t key)
{
  chm_node_t** bucket = &set->table[key & set->hash];
  chm_node_t* curr = *bucket;

  while (curr != NULL)
    {
      if (curr->key == key)
	{
	  return curr->val;
	}
      curr = curr->next;
    }

  return 0;
}

int
chm_put(chm_t* set, skey_t key, sval_t val)
{
  chm_node_t** bucket = &set->table[key & set->hash];

  ptlock_t* seg_lock = &set->locks[key & set->hash_seg];
  LOCK_A(seg_lock);

  chm_node_t* curr = *bucket;

  chm_node_t* pred = NULL;
  while (curr != NULL)
    {
      if (curr->key == key)
	{
	  UNLOCK_A(seg_lock);
	  return 0;
	}
      pred = curr;
      curr = curr->next;
    }

  chm_node_t* n = chm_node_new(key, val, NULL);
  if (pred != NULL)
    {
      pred->next = n;
    }
  else
    {
      *bucket = n;
    }

  UNLOCK_A(seg_lock);
  return 1;
}

sval_t
chm_rem(chm_t* set, skey_t key)
{
  chm_node_t** bucket = &set->table[key & set->hash];

  ptlock_t* seg_lock = &set->locks[key & set->hash_seg];
  LOCK_A(seg_lock);

  chm_node_t* curr = *bucket;

  chm_node_t* pred = NULL;
  while (curr != NULL)
    {
      if (curr->key == key)
	{
	  /* do the remove */
	  if (pred != NULL)
	    {
	      pred->next = curr->next;
	    }
	  else
	    {
	      *bucket = curr->next;
	    }
#if GC == 1
	  ssmem_free(alloc, (void*) curr);
#endif
	  UNLOCK_A(seg_lock);
	  return curr->val;
	}
      pred = curr;
      curr = curr->next;
    }

  UNLOCK_A(seg_lock);
  return 0;
}



/* not linearizable */
size_t
chm_size(chm_t* set)
{
  size_t size = 0;
  int i;
  for (i = 0; i < set->num_buckets; i++)
    {
      chm_node_t* curr = set->table[i];
      while (curr != NULL)
	{
	  size++;
	  curr = curr->next;
	}
    }

  return size;
}
