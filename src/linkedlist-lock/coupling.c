/*
 * File:
 *   coupling.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Hand-over-hand lock-based linked list implementation of an integer set
 *
 * Copyright (c) 2009-2010.
 *
 * coupling.c is part of Synchrobench
 * 
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "coupling.h"

/* 
 * Similar algorithm for the delete, find, and insert:
 * Lock the first two elements (locking each before getting the copy of the element)
 * then unlock previous, keep ownership of the current, and lock next in a loop.
 */
int lockc_delete(intset_l_t *set, val_t val) {
	node_l_t *curr, *next;
	int found;
	
	GL_LOCK(set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
	LOCK(ND_GET_LOCK(set->head));
	curr = set->head;
	LOCK(ND_GET_LOCK(curr->next));
	next = curr->next;
	
	while (next->val < val) {
	  UNLOCK(ND_GET_LOCK(curr));
		curr = next;
		LOCK(ND_GET_LOCK(next->next));
		next = next->next;
	}
	found = (val == next->val);
	if (found) {
	  curr->next = next->next;
	  UNLOCK(ND_GET_LOCK(next));
	  node_delete_l(next);
	  UNLOCK(ND_GET_LOCK(curr));
	} else {
	  UNLOCK(ND_GET_LOCK(curr));
	  UNLOCK(ND_GET_LOCK(next));
	}  
	GL_UNLOCK(set->lock);

	return found;
}

int lockc_find(intset_l_t *set, val_t val) {
	node_l_t *curr, *next; 
	int found;
	
	GL_LOCK(set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
	LOCK(ND_GET_LOCK(set->head));
	curr = set->head;
	LOCK(ND_GET_LOCK(curr->next));
	next = curr->next;
	
	while (next->val < val) {
		UNLOCK(ND_GET_LOCK(curr));
		curr = next;
		LOCK(ND_GET_LOCK(next->next));
		next = curr->next;
	}	
	found = (val == next->val);
	GL_UNLOCK(set->lock);
	UNLOCK(ND_GET_LOCK(curr));
	UNLOCK(ND_GET_LOCK(next));
	return found;
}

int lockc_insert(intset_l_t *set, val_t val) {
	node_l_t *curr, *next, *newnode;
	int found;
	
	GL_LOCK(set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
	LOCK(ND_GET_LOCK(set->head));
	curr = set->head;
	LOCK(ND_GET_LOCK(curr->next));
	next = curr->next;
	
	while (next->val < val) {
		
		UNLOCK(ND_GET_LOCK(curr));
		curr = next;
		LOCK(ND_GET_LOCK(next->next));
		next = curr->next;
		
	}
	found = (val == next->val);
	if (!found) {
		newnode =  new_node_l(val, next, 0);
		curr->next = newnode;
	}
	GL_UNLOCK(set->lock);
	UNLOCK(ND_GET_LOCK(curr));
	UNLOCK(ND_GET_LOCK(next));
	return !found;
}
