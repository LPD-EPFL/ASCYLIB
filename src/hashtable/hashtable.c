/*
 * File:
 *   hashtable.c
 * Author(s):
 * Description:
 *   Hashtable
 *   Implementation of an integer set using a stm-based/lock-free hashtable.
 *   The hashtable contains several buckets, each represented by a linked
 *   list, since hashing distinct keys may lead to the same bucket.
 *
 * Copyright (c) 2009-2010.
 *
 * hashtable.c is part of HIDDEN
 * 
 * HIDDEN is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hashtable.h"

void ht_delete(ht_intset_t *set) 
{
  node_t *node, *next;
  int i;
  
  for (i=0; i < maxhtlength; i++) 
    {
      node = set->buckets[i].head;
      while (node != NULL) 
	{
	  next = node->next;
	  free((void*) node);
	  node = next;
	}
    }
  free(set->buckets);
  free(set);
}

int
ht_size(ht_intset_t *set) 
{
  int size = 0;
  int i;
	
  for (i = 0; i < maxhtlength; i++) 
    {
      size += set_size(&set->buckets[i]);
    }
  return size;
}

int floor_log_2(unsigned int n) {
  int pos = 0;
  printf("n result = %d\n", n);
  if (n >= 1<<16) { n >>= 16; pos += 16; }
  if (n >= 1<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1<< 1) {           pos +=  1; }
  printf("floor result = %d\n", pos);
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

  size_t bs = (maxhtlength + 1) * sizeof(intset_t);
  bs += CACHE_LINE_SIZE - (bs & CACHE_LINE_SIZE);
  if ((set->buckets = ssalloc_alloc(1, bs)) == NULL)
    {
      perror("malloc");
      exit(1);
    }  

  for (i = 0; i < maxhtlength; i++) 
    {
      bucket_set_init(&set->buckets[i]);
    }
  return set;
}
