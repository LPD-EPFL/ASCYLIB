#include "intset.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;

queue_node_t*
queue_node_new(skey_t key, sval_t val, queue_node_t* next)
{
  volatile queue_node_t* node;
#if GC == 1
  node = (volatile queue_node_t *) ssmem_alloc(alloc, sizeof(queue_node_t));
#else
  node = (volatile queue_node_t *) ssalloc(sizeof(queue_node_t));
#endif
  
  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->next = next;

  optik_init(&node->lock);

#if defined(__tile__)
  MEM_BARRIER;
#endif	/* __tile__ */

  return (queue_node_t*) node;
}

static inline void
queue_node_delete(queue_node_t* node)
{
#if GC == 1
  ssmem_free(alloc, (void*) node);
#endif
}

queue_node_t*
queue_node_new_init(skey_t key, sval_t val, queue_node_t* next)
{
  volatile queue_node_t* node;
  node = (volatile queue_node_t *) ssalloc(sizeof(queue_node_t));
  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->next = next;

  optik_init(&node->lock);

#if defined(__tile__)
  MEM_BARRIER;
#endif	/* __tile__ */

  return (queue_node_t*) node;
}

queue_low_t* queue_low_new()
{
  queue_low_t *set;
  queue_node_t *min, *max;

  if ((set = (queue_low_t *)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(queue_low_t))) == NULL) 
    {
      perror("malloc");
      exit(1);
    }

  max = queue_node_new(KEY_MAX, 0, NULL);
  min = queue_node_new(KEY_MIN, 0, max);
  set->head = min;

  MEM_BARRIER;
  return set;
}

void
queue_low_init(queue_low_t* ql)
{
  queue_node_t *min, *max;

  max = queue_node_new_init(KEY_MAX, 0, NULL);
  min = queue_node_new_init(KEY_MIN, 0, max);
  ql->head = min;
  MEM_BARRIER;
}

int
queue_low_size(queue_low_t* ql)
{
  int size = 0;
  volatile queue_node_t* node;

  /* We have at least 2 elements */
  node = ql->head->next;
  while (node->next != NULL) 
    {
      size++;
      node = node->next;
    }

  return size;
}

int
queue_low_push(queue_low_t *ql, skey_t key, sval_t val)
{
  optik_t pred_ver = OPTIK_INIT;
	
 restart:
  PARSE_TRY();
  queue_node_t* curr = ql->head, *pred;

  do
    {
      COMPILER_NO_REORDER(optik_t curr_ver = curr->lock;);
	  
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
    }
  while (likely(curr->key <= key));

  UPDATE_TRY();

  queue_node_t* newnode = queue_node_new(key, val, curr);

  if ((!optik_trylock_version(&pred->lock, pred_ver)))
    {
      queue_node_delete(newnode);
      goto restart;
    }

  pred->next = newnode;
  optik_unlock(&pred->lock);

  return true;
}

sval_t
queue_low_pop(queue_low_t *ql)
{
 restart:
  PARSE_TRY();

  queue_node_t* pred = ql->head;
  COMPILER_NO_REORDER(optik_t pred_ver = pred->lock;);
  queue_node_t* curr = pred->next;
  if (unlikely(curr == NULL || curr->key == KEY_MAX))
    {
      return 0;
    }
  COMPILER_NO_REORDER(optik_t curr_ver = curr->lock;);

  UPDATE_TRY();

  queue_node_t* cnxt = curr->next;

  if (unlikely(!optik_trylock_version(&pred->lock, pred_ver)))
    {
      goto restart;
    }

  if (unlikely(!optik_trylock_version(&curr->lock, curr_ver)))
    {
      optik_revert(&pred->lock);
      goto restart;
    }

  pred->next = cnxt;
  optik_unlock(&pred->lock);
      
  sval_t result = curr->val;
  queue_node_delete(curr);
  return result;
}


inline sval_t
queue_low_get_min(queue_low_t *ql)
{
  queue_node_t* pred = ql->head;
  queue_node_t* curr = pred->next;
  if (unlikely(curr == NULL || curr->key == KEY_MAX))
    {
      return KEY_MAX;
    }
  return curr->key;
}
