/*
 *  linkedlist.c
 *
 *  Description:
 *   Lock-free linkedlist implementation of Harris' algorithm
 *   "A Pragmatic Implementation of Non-Blocking Linked Lists" 
 *   T. Harris, p. 300-314, DISC 2001.
 * 
 * Documentation format : Doxygen
 */

#include "linkedlist.h"
/**@brief success is represented by 1*/
#define SUCCESS 1
/**@brief failure represented by 0*/
#define FAILURE 0

/**
 * @dev GET_NEXT(PTR)
 * 
 * @brief Maccro to avoid having to write casts and ->next
 * 
 * @param node_t* pointer
 **/
#define GET_NEXT(PTR)\
	(node_t*)get_unmarked_ref((long)((PTR)->next))
	
#define CAS(REG, OLD, NEW)\
	CAS_U64((REG), (OLD), (NEW))

/*
 * The five following functions handle the low-order mark bit that indicates
 * whether a node is logically deleted (1) or not (0).
 *  - is_marked_ref returns whether it is marked, 
 *  - (un)set_marked changes the mark,
 *  - get_(un)marked_ref sets the mark before returning the node.
 */
inline int
is_marked_ref(long i) 
{
  return (int) (i & 0x1L);
}

inline long
unset_mark(long i)
{
  i &= ~0x1L;
  return i;
}

inline long
set_mark(long i) 
{
  i |= 0x1L;
  return i;
}

inline long
get_unmarked_ref(long w) 
{
  return w & ~0x1L;
}

inline long
get_marked_ref(long w) 
{
  return w | 0x1L;
}

/**
 * list_search looks for value val, it
 *  - returns right_node owning val (if present) or its immediately higher 
 *    value present in the list (otherwise) and 
 *  - sets the left_node to the node owning the value immediately lower than val. 
 * Encountered nodes that are marked as logically deleted are physically removed
 * from the list, yet not garbage collected.
 */
node_t* 
list_search(intset_t* set, val_t val, node_t** left_node) 
{
  node_t* prev = set->head;
  node_t* iter = GET_NEXT(prev); /*This one cannot be marked*/
	
  for(; iter->val != INT_MAX; iter = GET_NEXT(iter))
    {
      /*We clean the list if we have to*/
      if(is_marked_ref((long)(iter->next)) &&
	 (CAS(&(prev->next), iter, GET_NEXT(iter)) || 1)){}
		
      else if(iter->val >= val)
	{
	  *left_node = prev;
	  return iter;
	}
      else
	{
	  prev = iter;
	}
    }
  *left_node = prev;
  return iter;
}

/**
 * @dev list_contains(intset_t* the_list, val_t val)
 * 
 * @brief Finds out if the list contains the specified value
 * 
 * @param intset_t* the_list
 * @param val_t val
 * 
 * @return int SUCCESS if contains the node, FAILURE otherwise 
 * */
/*
int list_contains(intset_t* the_list, val_t val)
{
  assert(the_list != NULL);
  int res = FAILURE;
  node_t* lft = NULL; 
  node_t* rght = list_search(the_list, val, &lft);
  
  if(rght->val == val && rght != &(the_list->max)){
		res = SUCCESS;
	}
  return res;
  }*/
 /*Version that doesn't use the list_search*/
int harris_find(intset_t* the_list, val_t val)
{
  node_t* iter = GET_NEXT(the_list->head);
  
  for(; iter->val <= val; iter = GET_NEXT(iter))
    {
      if(!is_marked_ref((long)(iter->next)) && iter->val == val)
	{
	  return SUCCESS;	
	}
    }
  return FAILURE;
}

/**
 * @dev list_add(intset_t *the_list, val_t val)
 * 
 * @brief Adds an element if it wasn't present, does nothing if it was.
 * 
 * @param intset_t* the_list
 * @param val_t val
 * 
 * @return int SUCCESS if added, 0 if already present
 **/

int
harris_insert(intset_t *the_list, val_t val)
{
  node_t* lft = NULL;
  node_t* rght = NULL;
	
  /*Create the new node*/
  node_t* new = new_node(val, NULL, 0);
	
  do
    {
      rght = list_search(the_list, val, &lft);
      if (rght->val == val)
	{
	  return FAILURE;
	}
      new->next = rght;
    }while(CAS(&(lft->next), rght, new) != rght);
	
  return SUCCESS;
}

/**
 * @dev list_remove(intset_t *the_list, val_t val)
 * 
 * @brief removes virtually a node from the list
 * 
 * @param intset_t* the_list
 * @param val_t val
 * 
 * @return int SUCCESS if removed, FAILURE otherwise
 * */
int 
harris_delete(intset_t *the_list, val_t val)
{
  node_t* lft = NULL;
  node_t* rght = NULL;
  node_t* next = NULL;
  
  do
    {
      rght = list_search(the_list, val, &lft);
		
      if(rght->val != val || rght->val == INT_MAX)
	{
	  return FAILURE;
	}	
		
      next = rght->next;
    
    }
  while(is_marked_ref((long)next) ||
	CAS(&(rght->next), next,(node_t*)set_mark((long)next)) != next);
	
  return SUCCESS;
}

