/*   
 *   File: hashtable-seq.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   hashtable-seq.c is part of ASCYLIB
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

#include "hashtable-seq.h"
#include "ssalloc.h"

unsigned int maxhtlength;

void
ht_delete(ht_intset_t *set) 
{
  printf("ht_delete(ht_intset_t *set)  not implemented :-(\n");
}

int
ht_size(ht_intset_t *set) 
{
  int size = 0;
  node_t *node;
  int i;
	
  for (i=0; i < maxhtlength; i++) 
    {
      node = set->buckets[i].head;
      while (node != NULL) 
	{
	  size++;
	  node = node->next;
	}
    }
  return size;
}

ht_intset_t*
ht_new() 
{
  ht_intset_t *set;
  int i;
	
  if ((set = (ht_intset_t*) ht_alloc(sizeof(ht_intset_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }   

  set->hash = maxhtlength - 1;
  set->buckets = ht_alloc((maxhtlength + 1) * sizeof(intset_t));
  if (set->buckets == NULL)
    {
      perror("malloc buckets");
      exit(1);
    }  

  for (i=0; i < maxhtlength; i++) 
    {
      bucket_set_init(&set->buckets[i]);
    }
  return set;
}

sval_t
ht_contains(ht_intset_t *set, skey_t key) 
{
  int addr = key & set->hash;
  return set_contains(&set->buckets[addr], key);
}

int
ht_add(ht_intset_t *set, skey_t key, sval_t val) 
{
  int addr = key & set->hash;
  return set_add(&set->buckets[addr], key, val);
}

sval_t
ht_remove(ht_intset_t *set, skey_t key) 
{
  int addr = key & set->hash;
  return set_remove(&set->buckets[addr], key);
}

