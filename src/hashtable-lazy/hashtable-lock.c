/*   
 *   File: hashtable-lock.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: using one lazy linst per bucket
 *   hashtable-lock.c is part of ASCYLIB
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

#include "hashtable-lock.h"
#include "ssalloc.h"

unsigned int maxhtlength;

void
ht_delete(ht_intset_t *set) 
{
  node_l_t *node, *next;
  int i;
	
  for (i=0; i < maxhtlength; i++) 
    {
      node = set->buckets[i].head;
      while (node != NULL) 
	{
	  next = node->next;
	  /* free(node); */
	  ssfree((void*) node);		/* TODO: fix with ssmem */
	  node = next;
	}
    }
  free(set);
}

int
ht_size(ht_intset_t *set) 
{
  int size = 0;
  node_l_t *node;
  int i;
	
  for (i=0; i < maxhtlength; i++) 
    {
      node = set->buckets[i].head;
      while (node->next) 
	{
	  size++;
	  node = node->next;
	}
    }
  return size;
}

int
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

ht_intset_t*
ht_new() 
{
  ht_intset_t *set;
  int i;
	
  if ((set = (ht_intset_t *)ssalloc_aligned_alloc(1, CACHE_LINE_SIZE, sizeof(ht_intset_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }   

  set->hash = maxhtlength - 1;

  size_t bs = (maxhtlength + 1) * sizeof(intset_l_t);
  bs += CACHE_LINE_SIZE - (bs & CACHE_LINE_SIZE);
  if ((set->buckets = ssalloc_alloc(1, bs)) == NULL)
    {
      perror("malloc buckets");
      exit(1);
    }  

#if defined(LL_GLOBAL_LOCK)
  size_t ls = (maxhtlength + 1) * sizeof(ptlock_t);
  ls += CACHE_LINE_SIZE - (ls & CACHE_LINE_SIZE);
  if ((set->locks = ssalloc_alloc(1, ls)) == NULL)
    {
      perror("malloc locks");
      exit(1);
    }  
#endif

  for (i=0; i < maxhtlength; i++) 
    {
#if defined(LL_GLOBAL_LOCK)
      ptlock_t* l = &set->locks[i];
#else
      ptlock_t* l = NULL;
#endif
      bucket_set_init_l(&set->buckets[i], l);
    }
  return set;
}

sval_t
ht_contains(ht_intset_t *set, skey_t key) 
{
  int addr = key & set->hash;
  return set_contains_l(&set->buckets[addr], key);
}

int
ht_add(ht_intset_t *set, skey_t key, sval_t val) 
{
  int addr = key & set->hash;
  return set_add_l(&set->buckets[addr], key, val);
}

sval_t
ht_remove(ht_intset_t *set, skey_t key) 
{
  int addr = key & set->hash;
  return set_remove_l(&set->buckets[addr], key);
}

/* 
 * Move an element in the hashtable (from one linked-list to another)
 */
int
ht_move(ht_intset_t *set, int val1, int val2) 
{
  int result = 0;
  /* node_l_t *pred1, *curr1, *curr2, *pred2, *newnode; */
  /* int addr1, addr2, result = 0; */
	
/* #ifdef DEBUG */
/*   printf("++> ht_move(%d, %d)\n", (int) val1, (int) val2); */
/*   IO_FLUSH; */
/* #endif */
	
/*   if (val1 == val2) return 0; */
	
/*   // records pred and succ of val1 */
/*   addr1 = val1 % maxhtlength; */
/*   pred1 = set->buckets[addr1]->head; */
/*   curr1 = pred1->next; */
/*   while (curr1->val < val1) { */
/*     pred1 = curr1; */
/*     curr1 = curr1->next; */
/*   } */
/*   // records pred and succ of val2  */
/*   addr2 = val2 % maxhtlength; */
/*   pred2 = set->buckets[addr2]->head; */
/*   curr2 = pred2->next; */
/*   while (curr2->val < val2) { */
/*     pred2 = curr2; */
/*     curr2 = curr2->next; */
/*   } */
/*   // unnecessary move */
/*   if (pred1->val == pred2->val || curr1->val == pred2->val ||  */
/*       curr2->val == pred1->val || curr1->val == curr2->val)  */
/*     return 0; */
/*   // acquire locks in order */
/*   if (addr1 < addr2 || (addr1 == addr2 && val1 < val2)) { */
/*     LOCK(ND_GET_LOCK(pred1)); */
/*     LOCK(ND_GET_LOCK(curr1)); */
/*     LOCK(ND_GET_LOCK(pred2)); */
/*     LOCK(ND_GET_LOCK(curr2)); */
/*   } else { */
/*     LOCK(ND_GET_LOCK(pred2)); */
/*     LOCK(ND_GET_LOCK(curr2)); */
/*     LOCK(ND_GET_LOCK(pred1)); */
/*     LOCK(ND_GET_LOCK(curr1)); */
/*   } */
/*   // remove val1 and insert val2  */
/*   result = (parse_validate(pred1, curr1) && (val1 == curr1->val) && */
/* 	    parse_validate(pred2, curr2) && (curr2->val != val2)); */
/*   if (result) { */
/*     set_mark((uintptr_t*) &curr1->next); */
/*     pred1->next = curr1->next; */
/*     newnode = new_node_l(val2, val2, curr2, 0); */
/*     pred2->next = newnode; */
/*   } */
/*   // release locks in order */
/*   UNLOCK(ND_GET_LOCK(pred2)); */
/*   UNLOCK(ND_GET_LOCK(pred1)); */
/*   UNLOCK(ND_GET_LOCK(curr2)); */
/*   UNLOCK(ND_GET_LOCK(curr1)); */
		
  return result;
}

/* 
 * Read all elements of the hashtable (parses all linked-lists)
 * This cannot be consistent when used with move operation.
 */
int ht_snapshot_unmovable(ht_intset_t *set) {
	/* node_l_t *next, *curr; */
	/* int i; */
	int sum = 0;
	
	/* for (i=0; i < maxhtlength; i++) { */
	/* 	curr = set->buckets[i]->head; */
	/* 	next = set->buckets[i]->head->next; */
		
  	/* 	//pthread_mutex_lock((pthread_mutex_t *) &next)); */
	/* 	LOCK(ND_GET_LOCK(next)); */
	    
	/* 	while (next->next) { */
	/* 		UNLOCK(ND_GET_LOCK(next)); */
	/* 		curr = next; */
	/* 		if (!is_marked_ref((long) next)) sum += next->val; */
	/* 		next = curr->next; */
	/* 		LOCK(ND_GET_LOCK(next)); */
	/* 	} */
	/* 	UNLOCK(ND_GET_LOCK(next)); */
	/* } */
	
	return sum;
}


/* 
 * Read all elements of the hashtable (parses all linked-lists)
 */
int ht_snapshot(ht_intset_t *set) {
	/* node_l_t *next, *curr; */
	/* int i; */
	/* int sum = 0; */
	
	/* int m = maxhtlength; */
	
	/* for (i=0; i < m; i++) { */
	/*   do { */
	/*     LOCK(ND_GET_LOCK(set->buckets[i]->head)); */
	/*     LOCK(ND_GET_LOCK(set->buckets[i]->head->next)); */
	/*     curr = set->buckets[i]->head; */
	/*     next = set->buckets[i]->head->next; */
	/*   } while (!parse_validate(curr, next)); */

	/*   while (next->next) { */
	/*     while(1) { */
	/*       LOCK(ND_GET_LOCK(next->next)); */
	/*       curr = next; */
	/*       next = curr->next; */
	/*       if (parse_validate(curr, next)) { */
	/* 	if (!is_marked_ref((long) next)) { */
	/* 	  sum += curr->val; */
	/* 	} */
	/* 	break; */
	/*       } */
	/*     } */
	/*   } */
	/* } */
	
	/* for (i=0; i < m; i++) { */
	/*   curr = set->buckets[i]->head; */
	/*   next = set->buckets[i]->head->next; */
	  
	/*   UNLOCK(ND_GET_LOCK(curr)); */
	/*   UNLOCK(ND_GET_LOCK(next)); */
	/*   while (next->next) { */
	/*     curr = next; */
	/*     next = curr->next; */
	/*     UNLOCK(ND_GET_LOCK(next)); */
	/*   } */
	/* } */
	
	return 1;
}
