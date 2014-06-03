#include "timnat.h"

RETRY_STATS_VARS;

__thread ssmem_allocator_t* alloc;

node_t*
new_node(skey_t key, sval_t val, node_t *next, int initializing)
{
  volatile node_t *node;

#if GC == 1
  if (unlikely(initializing))
    {
      node = (volatile node_t *) ssalloc(sizeof(node_t));
    }
  else
    {
      node = (volatile node_t *) ssmem_alloc(alloc, sizeof(node_t));
    }

#else
  node = (volatile node_t *) ssalloc(sizeof(node_t));
#endif

  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->next = next;
  return (node_t*) node;
}

intset_t* 
set_new()
{
  intset_t *set;
  node_t *min, *max;
	
  if ((set = (intset_t*)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(intset_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }

  max = new_node(KEY_MAX, 0, NULL, 1);
  min = new_node(KEY_MIN, 0, max, 1);
  set->head = min;

  return set;
}

void set_delete(intset_t *set)
{
  node_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = node->next;
    free((void*) node);
    node = next;
  }
  free(set);
}


inline int
is_marked_ref(node_t* i) 
{
  return ((uintptr_t) i & 0x1L);
}

inline node_t*
get_unmarked_ref(node_t* w) 
{
  return (node_t*) ((uintptr_t) w & ~0x1L);
}

inline node_t*
get_marked_ref(node_t* w) 
{
  return (node_t*) ((uintptr_t) w | 0x1L);
}

static inline int
physical_delete_right(node_t* left_node, node_t* right_node) 
{
  node_t* new_next = get_unmarked_ref(right_node->next);
  node_t* res = CAS_PTR(&left_node->next, right_node, new_next);
  int removed = (res == right_node);
#if GC == 1
  if (likely(removed))
    {
      ssmem_free(alloc, (void*) res);
    }
#endif
  return removed;
}

static inline node_t* 
list_search(intset_t* set, skey_t key, node_t** left_node_ptr) 
{
  PARSE_TRY();
  node_t* left_node = set->head;
  node_t* right_node = set->head->next;
  while(1)
    {
      if (likely(!is_marked_ref(right_node->next)))
	{
	  if (unlikely(right_node->key >= key))
	    {
	      break;
	    }
	  left_node = right_node;
	}
      else 
	{
	  CLEANUP_TRY();
	  physical_delete_right(left_node, right_node);
	}
      right_node = get_unmarked_ref(right_node->next);
    }
  *left_node_ptr = left_node;
  return right_node;
}

sval_t
timnat_find(intset_t* the_list, skey_t key)
{
  node_t* node = the_list->head->next;
  PARSE_TRY();
  while(likely(node->key < key))
    {
      node = get_unmarked_ref(node->next);
    }
  /* node_t* l; */
  /* node_t* node = list_search(the_list, key, &l); */

  if (node->key == key && !is_marked_ref(node->next)) 
    {
      return node->val;
    }
  return 0;
}


int
timnat_insert(intset_t *the_list, skey_t key, sval_t val)
{
  do
    {
      UPDATE_TRY();
      node_t* left_node;
      node_t* right_node = list_search(the_list, key, &left_node);
      if (right_node->key == key) 
	{
	  return 0;
	}

      node_t* node_to_add = new_node(key, val, right_node, 0);

#ifdef __tile__
      MEM_BARRIER;
#endif
      // Try to swing left_node's unmarked next pointer to a new node

      if (CAS_PTR(&left_node->next, right_node, node_to_add) == right_node)
	{
	  return 1;
	}

#if GC == 1
      ssmem_free(alloc, (void*) node_to_add);
#endif
    } 
  while (1);
}

sval_t
timnat_delete(intset_t *the_list, skey_t key)
{
  node_t* cas_result;
  node_t* unmarked_ref;
  node_t* left_node;
  node_t* right_node;
  
  do
    {
      UPDATE_TRY();
      right_node = list_search(the_list, key, &left_node);

      if (right_node->key != key)
	{
	  return 0;
	}
    
      // Try to mark right_node as logically deleted
      unmarked_ref = get_unmarked_ref(right_node->next);
      node_t* marked_ref = get_marked_ref(unmarked_ref);
      cas_result = CAS_PTR(&right_node->next, unmarked_ref, marked_ref);
    } 
  while(cas_result != unmarked_ref);

  sval_t ret = right_node->val;

  physical_delete_right(left_node, right_node);

  return ret;
}

cas_descriptor* insert_generator(key_t key){
}

cas_descriptor* remove_generator(key_t key){
}

bool_t update_wrapup() {

}

bool_t search_wrapup() {
    
}       

int
set_size(intset_t *set)
{
  size_t size = 0;
  node_t* node;

  /* We have at least 2 elements */
  node = get_unmarked_ref(set->head->next);
  while (get_unmarked_ref(node->next) != NULL)
    {
      if (!is_marked_ref(node->next)) size++;
      node = get_unmarked_ref(node->next);
    }

  return size;
}
