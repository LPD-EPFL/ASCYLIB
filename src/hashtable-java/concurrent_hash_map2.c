#include "concurrent_hash_map2.h"

__thread ssmem_allocator_t* alloc = NULL;
size_t maxhtlength = 0;


static chm_seg_t*
chm_seg_new(size_t capacity, double load_factor)
{
  chm_seg_t* seg = memalign(CACHE_LINE_SIZE, sizeof(chm_seg_t));
  assert(seg != NULL);

  seg->table = memalign(CACHE_LINE_SIZE, capacity * sizeof(chm_t*));
  assert(seg->table != NULL);

  seg->num_buckets = capacity;
  seg->hash = capacity - 1;
  seg->modifications = 0;
  seg->size = 0;
  seg->size_limit = load_factor * capacity;

  int i;
  for (i = 0; i < seg->num_buckets; i++)
    {
      seg->table[i] = NULL;
    }

  INIT_LOCK(&seg->lock);
  return seg;
}

chm_t*
chm_new()
{
  chm_t* chm = memalign(CACHE_LINE_SIZE, sizeof(chm_t));
  assert(chm != NULL);
  chm->segments = memalign(CACHE_LINE_SIZE, CHM_NUM_SEGMENTS * sizeof(chm_seg_t*));
  assert(chm->segments != NULL);

  chm->num_segments = CHM_NUM_SEGMENTS;
  chm->hash =  CHM_NUM_SEGMENTS - 1;

  assert(maxhtlength % CHM_NUM_SEGMENTS == 0);
  size_t capacity_seg = maxhtlength / CHM_NUM_SEGMENTS;
  
  int s;
  for (s = 0; s < CHM_NUM_SEGMENTS; s++)
    {
      chm->segments[s] = chm_seg_new(capacity_seg, CHM_LOAD_FACTOR);
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
  chm_seg_t* seg = set->segments[key & set->hash];
  chm_node_t** bucket = &seg->table[key & seg->hash];
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
  chm_seg_t* seg = set->segments[key & set->hash];
  chm_node_t** bucket = &seg->table[key & seg->hash];

  ptlock_t* seg_lock = &seg->lock;
  LOCK(seg_lock);

  chm_node_t* curr = *bucket;

  chm_node_t* pred = NULL;
  while (curr != NULL)
    {
      if (curr->key == key)
	{
	  UNLOCK(seg_lock);
	  return 0;
	}
      pred = curr;
      curr = curr->next;
    }

  chm_node_t* n = chm_node_new(key, val, NULL);

  uint32_t sizepp = seg->size + 1;
  if (sizepp == seg->size_limit)
    {
      /* printf("-- seg size limit %u :: resize\n", seg->size_limit); */
    }

  if (pred != NULL)
    {
      pred->next = n;
    }
  else
    {
      *bucket = n;
    }

  seg->size = sizepp;
  UNLOCK(seg_lock);
  return 1;
}

sval_t
chm_rem(chm_t* set, skey_t key)
{
  chm_seg_t* seg = set->segments[key & set->hash];
  chm_node_t** bucket = &seg->table[key & seg->hash];

  ptlock_t* seg_lock = &seg->lock;
  LOCK(seg_lock);

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
	  seg->size--;
	  UNLOCK(seg_lock);
	  return curr->val;
	}
      pred = curr;
      curr = curr->next;
    }

 UNLOCK(seg_lock);
 return 0;
}



/* not linearizable */
size_t
chm_size(chm_t* set)
{
  size_t size = 0;
  int s;
  for (s = 0; s < set->num_segments; s++)
    {
      chm_seg_t* seg = set->segments[s];
      int i;
      for (i = 0; i < seg->num_buckets; i++)
	{
	  chm_node_t* curr = seg->table[i];
	  while (curr != NULL)
	    {
	      size++;
	      curr = curr->next;
	    }
	}
    }

  return size;
}
