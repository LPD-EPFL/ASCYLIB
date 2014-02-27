#include "concurrent_hash_map2.h"

__thread ssmem_allocator_t* alloc = NULL;


/* ********************************************************************************
 * help functions 
 ******************************************************************************** */

static int
floor_log_2(unsigned int n) 
{
  int pos = 0;
  if (n >= 1<<16) { n >>= 16; pos += 16; }
  if (n >= 1<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1<< 1) {           pos +=  1; }
  return ((n == 0) ? (-1) : pos);
}

static inline int
hash(skey_t key, int hash_seed)
{
  return (key >> hash_seed);
}

/* ********************************************************************************
 * create functions
 ******************************************************************************** */

static chm_seg_t*
chm_seg_new(size_t capacity, float load_factor)
{
  chm_seg_t* seg = memalign(CACHE_LINE_SIZE, sizeof(chm_seg_t));
  assert(seg != NULL);

  seg->table = memalign(CACHE_LINE_SIZE, capacity * sizeof(chm_node_t*));
  assert(seg->table != NULL);

  seg->num_buckets = capacity;
  seg->hash = capacity - 1;
  seg->modifications = 0;
  seg->size = 0;
  seg->load_factor = load_factor;
  seg->size_limit = seg->load_factor * capacity;
  if (seg->size_limit == 0)
    {
      seg->size_limit = 1;
    }

  int i;
  for (i = 0; i < seg->num_buckets; i++)
    {
      seg->table[i] = NULL;
    }

  INIT_LOCK(&seg->lock);
  return seg;
}

chm_t*
chm_new(size_t capacity, size_t num_segments)
{
  chm_t* chm = memalign(CACHE_LINE_SIZE, sizeof(chm_t));
  assert(chm != NULL);

  if (capacity < num_segments)
    {
      capacity = num_segments;
    }

  chm->num_segments = num_segments;
  chm->segments = memalign(CACHE_LINE_SIZE,  chm->num_segments * sizeof(chm_seg_t*));
  assert(chm->segments != NULL);

  chm->hash =  chm->num_segments - 1;
  int hs = floor_log_2(chm->num_segments);
  chm->hash_seed = hs;

  assert(capacity % chm->num_segments == 0);
  size_t capacity_seg = capacity / chm->num_segments;
#if defined(DEBUG)
  printf("#seg: %5zu, #bu/seg: %zu\n", chm->num_segments, capacity_seg);
#endif
  int s;
  for (s = 0; s < chm->num_segments; s++)
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


void
chm_seg_rehash(chm_t* set, int seg_num, chm_node_t* new)
{
  chm_seg_t** seg_oldp = set->segments + seg_num;
  chm_seg_t* seg_old = *seg_oldp;
  chm_seg_t* seg_new = chm_seg_new(seg_old->num_buckets << 1, seg_old->load_factor);

  int mask_new = seg_new->hash;

  int b;
  for (b = 0; b < seg_old->num_buckets; b++)
    {
      chm_node_t* curr = seg_old->table[b];
      if (curr != NULL)
	{
	  chm_node_t* next = curr->next;
	  int idx = hash(curr->key, set->hash_seed) & mask_new;
	  if (next == NULL)	/* single node on list */
	    {
	      seg_new->table[idx] = curr;
	    }
	  else
	    {			/* reuse consecutive sequence at same slot */
	      chm_node_t* last_run = curr;
	      int last_idx = idx;
	      chm_node_t* last;
	      for (last = next; last != NULL; last = last->next)
		{
		  int k = hash(last->key, set->hash_seed) & mask_new;
		  if (k != last_idx)
		    {
		      last_idx = k;
		      last_run = last;
		    }
		}
	      seg_new->table[last_idx] = last_run;
	      /* clone remaining */
	      chm_node_t* p;
	      for (p = curr; p != last_run; p = p->next)
		{
		  int k = hash(p->key, set->hash_seed) & mask_new;
		  chm_node_t* n = chm_node_new(p->key, p->val, seg_new->table[k]);
		  seg_new->table[k] = n;
#if GC == 1
		  ssmem_free(alloc, (void*) p);
#endif
		}
	      
	    }
	}
    }

  int new_idx = hash(new->key, set->hash_seed) & mask_new; /* add the new node */
  new->next = seg_new->table[new_idx];
  seg_new->table[new_idx] = new;

  seg_new->size = seg_old->size + 1;
  set->segments[seg_num] = seg_new;

#if GC == 1
  ssmem_release(alloc, (void*) seg_old);
  ssmem_release(alloc, seg_old->table);
#endif
}


/* ********************************************************************************
 * hash table interface
 ******************************************************************************** */

sval_t
chm_get(chm_t* set, skey_t key)
{
  chm_seg_t* seg = set->segments[key & set->hash];
  chm_node_t** bucket = &seg->table[hash(key, set->hash_seed) & seg->hash];
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
  int seg_num = key & set->hash;

  volatile chm_seg_t* seg;
  volatile ptlock_t* seg_lock;
  do 
    {
      seg = set->segments[seg_num];
      seg_lock = &seg->lock;
    }
  while (!TRYLOCK(seg_lock));

  chm_node_t** bucket = &seg->table[hash(key, set->hash_seed) & seg->hash];
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
  if (unlikely(sizepp >= seg->size_limit))
    {
#if defined(DEBUG)
      /* printf("-[%3d]- seg size limit %u :: resize\n", seg_num, seg->size_limit); */
#endif
      chm_seg_rehash(set, seg_num, n);
    }
  else
    {
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
    }

  return 1;
}

sval_t
chm_rem(chm_t* set, skey_t key)
{
  int seg_num = key & set->hash;

  volatile chm_seg_t* seg;
  volatile ptlock_t* seg_lock;
  do 
    {
      seg = set->segments[seg_num];
      seg_lock = &seg->lock;
    }
  while (!TRYLOCK(seg_lock));

  chm_node_t** bucket = &seg->table[hash(key, set->hash_seed) & seg->hash];
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
