/*   
 *   File: bst_howley.c
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>, 
 *  	     Tudor David <tudor.david@epfl.ch>
 *   Description: Shane V Howley and Jeremy Jones. 
 *   A non-blocking internal binary search tree. SPAA 2012
 *   bst_howley.c is part of ASCYLIB
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

#include "bst_howley.h"

RETRY_STATS_VARS;

__thread ssmem_allocator_t* alloc;

const sval_t val_mask = ~(0x3);

node_t* create_node(skey_t key, sval_t value, int initializing) {
    volatile node_t * new_node;
#if GC == 1
    if (unlikely(initializing)) {
        new_node = (volatile node_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(node_t));
    } else {
        new_node = (volatile node_t*) ssmem_alloc(alloc, sizeof(node_t));
    }
#else
    new_node = (volatile node_t*) ssalloc(sizeof(node_t));
#endif
    if (new_node==NULL) {
        perror("malloc in bst create node");
        exit(1);
    }
    new_node->key=key;
    new_node->value=value;
    new_node->op=NULL;
    new_node->right=NULL;
    new_node->left=NULL;

    asm volatile("" ::: "memory");
    return (node_t*) new_node;
}

operation_t* alloc_op() {
    volatile operation_t * new_op;
#if GC == 1
    new_op = (volatile operation_t*) ssmem_alloc(alloc, sizeof(operation_t));
#else
    new_op = (volatile operation_t*) ssalloc(sizeof(operation_t));
#endif
    if (new_op==NULL) {
        perror("malloc in bst create node");
        exit(1);
    }
    return (operation_t*) new_op;
}

node_t* bst_initialize() {

	// node_t* root = (node_t*) ssalloc(sizeof(node_t));
	node_t* root = create_node(0,0,1);

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root

	return root;
}


sval_t bst_contains(skey_t k, node_t* root){
	
	node_t* pred;
	node_t* curr;
	operation_t* pred_op;
	operation_t* curr_op;
    sval_t res = bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root);
    if (res & val_mask) return res;
    return 0;
}

sval_t bst_find(skey_t k, node_t** pred, operation_t** pred_op, node_t** curr, operation_t** curr_op, node_t* aux_root, node_t* root){

	sval_t result;
	skey_t curr_key;
	node_t* next;
	node_t* last_right;
	operation_t* last_right_op;

retry:
	PARSE_TRY();

	result = NOT_FOUND_R;
	*curr = aux_root;
	*curr_op = (*curr)->op;

	if(GETFLAG(*curr_op) != STATE_OP_NONE){
#ifdef __tile__
    MEM_BARRIER;
#endif
		if (aux_root == root){
			bst_help_child_cas((operation_t*)UNFLAG(*curr_op), *curr, root);
			goto retry;
		} else {
			return ABORT;
		}
	}


	next = (node_t*) (*curr)->right;
	last_right = *curr;
	last_right_op = *curr_op;

	while (!ISNULL(next)){
		*pred = *curr;
		*pred_op = *curr_op;
		*curr = next;
		*curr_op = (*curr)->op;


		if(GETFLAG(*curr_op) != STATE_OP_NONE){
			bst_help(*pred, *pred_op, *curr, *curr_op, root);
			goto retry;
		}
		curr_key = (*curr)->key;
		if(k < curr_key){
			result = NOT_FOUND_L;
			next = (node_t*) (*curr)->left;
		} else if(k > curr_key) {
			result = NOT_FOUND_R;
			next = (node_t*) (*curr)->right;
			last_right = *curr;
			last_right_op = *curr_op;
		} else{
			result = (*curr)->value;
			break;
		}
	}
	
	if ((!(result & val_mask)) && (last_right_op != last_right->op)) {
		goto retry;
	}

	if ((*curr)->op != *curr_op){
		goto retry;
	}

	return result;
} 
  
bool_t bst_add(skey_t k,sval_t v,  node_t* root){

	node_t* pred;
	node_t* curr;
	node_t* new_node = NULL;
	operation_t* pred_op;
	operation_t* curr_op;
	operation_t* cas_op;
	sval_t  result;

	while(TRUE) {
	  UPDATE_TRY();

		result = bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root);
		if (result & val_mask) {
#if GC == 1
            if (new_node!=NULL) {
                ssmem_free(alloc,new_node);
            }
#endif
			return FALSE;
		}
        
        if (new_node == NULL) {
		    new_node = create_node(k,v,0);
        }

		bool_t is_left = (result == NOT_FOUND_L);
		node_t* old;
		if (is_left) {
			old = (node_t*) curr->left;
		} else {
			old = (node_t*) curr->right;
		}

		cas_op = alloc_op();
		cas_op->child_cas_op.is_left = is_left;
		cas_op->child_cas_op.expected = old;
		cas_op->child_cas_op.update = new_node;

#if defined(__tile__)
		MEM_BARRIER;
#endif
		if (CAS_PTR(&curr->op, curr_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == curr_op) {

			bst_help_child_cas(cas_op, curr, root);
#if GC == 1
            //if (UNFLAG(curr_op)!=0) ssmem_free(alloc,(void*)UNFLAG(curr_op));
#endif
			return TRUE;
		} else {
#if GC == 1
            ssmem_free(alloc,cas_op);
#endif
        }
	}
}

void bst_help_child_cas(operation_t* op, node_t* dest, node_t* root){

  CLEANUP_TRY();

	node_t** address = NULL;
	if (op->child_cas_op.is_left) {
	  address = (node_t**) &(dest->left);
	} else {
		address = (node_t**) &(dest->right);
	}
	void* UNUSED dummy0 = CAS_PTR(address, op->child_cas_op.expected, op->child_cas_op.update);
#ifdef __tile__
    MEM_BARRIER;
#endif
	void* UNUSED dummy1 = CAS_PTR(&(dest->op), FLAG(op, STATE_OP_CHILDCAS), FLAG(op, STATE_OP_NONE));
}

sval_t bst_remove(skey_t k, node_t* root){

	node_t* pred;
	node_t* curr;
	node_t* replace;
    sval_t val;
	operation_t* pred_op;
	operation_t* curr_op;
	operation_t* replace_op;
	operation_t* reloc_op=NULL;

	while(TRUE) {
	  UPDATE_TRY();

        sval_t res = bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root);
		if (!(res & val_mask)) {
#if GC == 1
            //if (reloc_op!=NULL) ssmem_free(alloc,reloc_op);
#endif
			return 0;
		}

		if (ISNULL((node_t*) curr->right) || ISNULL((node_t*) curr->left)) { // node has less than two children
			if (CAS_PTR(&(curr->op), curr_op, FLAG(curr_op, STATE_OP_MARK)) == curr_op) {
				bst_help_marked(pred, pred_op, curr, root);
#if GC == 1
                //if (reloc_op!=NULL) ssmem_free(alloc,reloc_op);
                if (UNFLAG(curr->op)!=0) ssmem_free(alloc,(void*)UNFLAG(curr->op));
                ssmem_free(alloc,curr);
#endif
				return res;
			}
		} else { // node has two children
			val = bst_find(k, &pred, &pred_op, &replace, &replace_op, curr, root);
			if ((val == ABORT) || (curr->op != curr_op)) {
				continue;
			} 
            
            //if (reloc_op==NULL) {
			    reloc_op = alloc_op(); 
            //}
			reloc_op->relocate_op.state = STATE_OP_ONGOING;
			reloc_op->relocate_op.dest = curr;
			reloc_op->relocate_op.dest_op = curr_op;
			reloc_op->relocate_op.remove_key = k;
			reloc_op->relocate_op.remove_value = res;
			reloc_op->relocate_op.replace_key = replace->key;
			reloc_op->relocate_op.replace_value = replace->value;

#if defined(__tile__)
			MEM_BARRIER;
#endif
			if (CAS_PTR(&(replace->op), replace_op, FLAG(reloc_op, STATE_OP_RELOCATE)) == replace_op) {
#if GC == 1
                if (UNFLAG(replace_op)!=0) ssmem_free(alloc,(void*)UNFLAG(replace_op));
#endif
				if (bst_help_relocate(reloc_op, pred, pred_op, replace, root)) {
                    //if (UNFLAG(replace->op)!=0) ssmem_free(alloc,(void*)UNFLAG(replace->op));
#if GC == 1
                    //ssmem_free(alloc,replace);
#endif
					return res;
				}
			} else {
#if GC == 1
               ssmem_free(alloc,reloc_op);
             //   reloc_op=NULL;
#endif
            }
		}
	}
}

bool_t bst_help_relocate(operation_t* op, node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){
  CLEANUP_TRY();

	int seen_state = op->relocate_op.state;
	if (seen_state == STATE_OP_ONGOING) {
		// VCAS in original implementation
		operation_t* seen_op = CAS_PTR(&(op->relocate_op.dest->op), op->relocate_op.dest_op, FLAG(op, STATE_OP_RELOCATE));
		if ((seen_op == op->relocate_op.dest_op) || (seen_op == (operation_t *)FLAG(op, STATE_OP_RELOCATE))){
			CAS_U32(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_SUCCESSFUL);
			seen_state = STATE_OP_SUCCESSFUL;
            if (seen_op == op->relocate_op.dest_op) {
#if GC == 1
                if (UNFLAG(seen_op)!=0) ssmem_free(alloc,(void*)UNFLAG(seen_op));
#endif
            } 
		} else {
			// VCAS in original implementation
			seen_state = CAS_U32(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_FAILED);
		}
	}

	if (seen_state == STATE_OP_SUCCESSFUL) {

		skey_t UNUSED dummy0 = CAS_PTR(&(op->relocate_op.dest->key), op->relocate_op.remove_key, op->relocate_op.replace_key);
		skey_t UNUSED dummy1 = CAS_PTR(&(op->relocate_op.dest->value), op->relocate_op.remove_value, op->relocate_op.replace_value);
		void* UNUSED dummy2 = CAS_PTR(&(op->relocate_op.dest->op), FLAG(op, STATE_OP_RELOCATE), FLAG(op, STATE_OP_NONE));
	}

	bool_t result = (seen_state == STATE_OP_SUCCESSFUL);
	if (op->relocate_op.dest == curr) {
		return result;
	}

	void* UNUSED dummy = CAS_PTR(&(curr->op), FLAG(op, STATE_OP_RELOCATE), FLAG(op, result ? STATE_OP_MARK : STATE_OP_NONE));
	if (result) {
		if (op->relocate_op.dest == pred) {
			pred_op = (operation_t *)FLAG(op, STATE_OP_NONE);
		}
		bst_help_marked(pred, pred_op, curr, root);
	}
	return result;
}

void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){

  CLEANUP_TRY();

	node_t* new_ref;
	if (ISNULL((node_t*) curr->left)) {
		if (ISNULL((node_t*) curr->right)) {
			new_ref = (node_t*)SETNULL(curr);
		} else {
			new_ref = (node_t*) curr->right;
		}
	} else {
		new_ref = (node_t*) curr->left;
	}
	operation_t* cas_op = alloc_op();
	cas_op->child_cas_op.is_left = (curr == pred->left);
	cas_op->child_cas_op.expected = curr;
	cas_op->child_cas_op.update = new_ref;

#ifdef __tile__
    MEM_BARRIER;
#endif
	if (CAS_PTR(&(pred->op), pred_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == pred_op) {
		bst_help_child_cas(cas_op, pred, root);
#if GC == 1
        if (UNFLAG(pred_op)!=0) ssmem_free(alloc,(void*)UNFLAG(pred_op));
#endif
	} else {
#if GC == 1
        ssmem_free(alloc,cas_op);
#endif
    }
}

void bst_help(node_t* pred, operation_t* pred_op, node_t* curr, operation_t* curr_op, node_t* root ){
	

	if (GETFLAG(curr_op) == STATE_OP_CHILDCAS) {
		bst_help_child_cas((operation_t*)UNFLAG(curr_op), curr, root);
	} else if (GETFLAG(curr_op) == STATE_OP_RELOCATE) {
		bst_help_relocate((operation_t*)UNFLAG(curr_op), pred, pred_op, curr, root);
	} else if (GETFLAG(curr_op) == STATE_OP_MARK) {
		bst_help_marked(pred, pred_op, curr, root);
	}
}

unsigned long bst_size_rec(volatile node_t* node) {
  if (ISNULL((node_t*) node)) {
    return 0;
  } else if (GETFLAG(node->op) != STATE_OP_MARK) {
    return 1 + bst_size_rec(node->right) + bst_size_rec(node->left);
  } else {
    return bst_size_rec(node->right) + bst_size_rec(node->left);
  }
}

unsigned long bst_size(node_t* node) {
    return bst_size_rec(node) - 1; //do not count the root
}

void bst_print(volatile node_t* node){
	if (ISNULL(node)) {
		return;
	}
	fprintf(stderr, "key: %lu ", node->key);
	fprintf(stderr, "address %p ", node);
	fprintf(stderr, "left: %p; right: %p, op: %p \n", node->left, node->right, node->op);
	
	bst_print(node->left);
	bst_print(node->right);
}
