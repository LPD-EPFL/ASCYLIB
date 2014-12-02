/*   
 *   File: bst_howley.c
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>, 
 *  	     Tudor David <tudor.david@epfl.ch>
 *   Description: 
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


node_t* bst_initialize() {

	// node_t* root = (node_t*) ssalloc(sizeof(node_t));
        node_t* root = (node_t*) ssalloc_aligned(CACHE_LINE_SIZE, CACHE_LINE_SIZE);

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root
	root->key = 0;
	root->left = NULL;
	root->right = NULL;
	root->op = NULL;
	
	return root;
}


bool_t bst_contains(bst_key_t k, node_t* root){
	
	node_t* pred;
	node_t* curr;
	operation_t* pred_op;
	operation_t* curr_op;

	return bst_find2(k, &pred, &pred_op, &curr, &curr_op, root, root) == FOUND;
}


search_res_t bst_find2(bst_key_t k, node_t** pred, operation_t** pred_op, node_t** curr, operation_t** curr_op, node_t* aux_root, node_t* root){

	search_res_t result;
	bst_key_t curr_key;
	node_t* next;
	node_t* last_right;
	operation_t* last_right_op;

retry:
	result = NOT_FOUND_R;
	*curr = aux_root;
	*curr_op = (*curr)->op;

	next = (*curr)->right;
	last_right = *curr;
	last_right_op = *curr_op;

	while (!ISNULL(next)){
		*pred = *curr;
		*pred_op = *curr_op;
		*curr = next;
		*curr_op = (*curr)->op;

		curr_key = (*curr)->key;
		if(k < curr_key){
			result = NOT_FOUND_L;
			next = (*curr)->left;
		} else if(k > curr_key) {
			result = NOT_FOUND_R;
			next = (*curr)->right;
			last_right = *curr;
			last_right_op = *curr_op;
		} else{
			result = FOUND;
			break;
		}
	}
	return result;
} 


search_res_t bst_find(bst_key_t k, node_t** pred, operation_t** pred_op, node_t** curr, operation_t** curr_op, node_t* aux_root, node_t* root){

	search_res_t result;
	bst_key_t curr_key;
	node_t* next;
	node_t* last_right;
	operation_t* last_right_op;

retry:
	result = NOT_FOUND_R;
	*curr = aux_root;
	*curr_op = (*curr)->op;

	if(GETFLAG(*curr_op) != STATE_OP_NONE){

		if (aux_root == root){
			bst_help_child_cas((operation_t*)UNFLAG(*curr_op), *curr, root);
			goto retry;
		} else {
			return ABORT;
		}
	}


	next = (*curr)->right;
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
			next = (*curr)->left;
		} else if(k > curr_key) {
			result = NOT_FOUND_R;
			next = (*curr)->right;
			last_right = *curr;
			last_right_op = *curr_op;
		} else{
			result = FOUND;
			break;
		}
	}
	
	if ((result != FOUND) && (last_right_op != last_right->op)) {
		goto retry;
	}

	if ((*curr)->op != *curr_op){
		goto retry;
	}

	return result;
} 
  
bool_t bst_add(bst_key_t k, node_t* root){

	node_t* pred;
	node_t* curr;
	node_t* new_node;
	operation_t* pred_op;
	operation_t* curr_op;
	operation_t* cas_op;
	search_res_t result;

	while(TRUE) {

		result = bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root);
		if (result == FOUND) {
			return FALSE;
		}

		new_node = (node_t*) ssalloc(sizeof(node_t));
		new_node->key = k;
		new_node->op = NULL;
		new_node->left = NULL;
		new_node->right = NULL;

		bool_t is_left = (result == NOT_FOUND_L);
		node_t* old;
		if (is_left) {
			old = curr->left;
		} else {
			old = curr->right;
		}

		cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
		cas_op->child_cas_op.is_left = is_left;
		cas_op->child_cas_op.expected = old;
		cas_op->child_cas_op.update = new_node;

#if defined(__tile__)
		MEM_BARRIER;
#endif
		if (CAS_PTR(&curr->op, curr_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == curr_op) {

			bst_help_child_cas(cas_op, curr, root);
			return TRUE;
		}
	}
}

void bst_help_child_cas(operation_t* op, node_t* dest, node_t* root){

	node_t** address = NULL;
	if (op->child_cas_op.is_left) {
		address = &(dest->left);
	} else {
		address = &(dest->right);
	}
	CAS_PTR(address, op->child_cas_op.expected, op->child_cas_op.update);
	CAS_PTR(&(dest->op), FLAG(op, STATE_OP_CHILDCAS), FLAG(op, STATE_OP_NONE));
}

bool_t bst_remove(bst_key_t k, node_t* root){

	node_t* pred;
	node_t* curr;
	node_t* replace;
	operation_t* pred_op;
	operation_t* curr_op;
	operation_t* replace_op;
	operation_t* reloc_op;

	while(TRUE) {

		if (bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root) != FOUND) {
			return FALSE;
		}

		if (ISNULL(curr->right) || ISNULL(curr->left)) { // node has less than two children
			if (CAS_PTR(&(curr->op), curr_op, FLAG(curr_op, STATE_OP_MARK)) == curr_op) {
				bst_help_marked(pred, pred_op, curr, root);
				return TRUE;
			}
		} else { // node has two children

			if ((bst_find(k, &pred, &pred_op, &replace, &replace_op, curr, root) == ABORT) || (curr->op != curr_op)) {
				continue;
			} 


			reloc_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
			reloc_op->relocate_op.state = STATE_OP_ONGOING;
			reloc_op->relocate_op.dest = curr;
			reloc_op->relocate_op.dest_op = curr_op;
			reloc_op->relocate_op.remove_key = k;
			reloc_op->relocate_op.replace_key = replace->key;

#if defined(__tile__)
			MEM_BARRIER;
#endif
			if (CAS_PTR(&(replace->op), replace_op, FLAG(reloc_op, STATE_OP_RELOCATE)) == replace_op) {
				if (bst_help_relocate(reloc_op, pred, pred_op, replace, root)) {
					return TRUE;
				}
			}
		}
	}
}

bool_t bst_help_relocate(operation_t* op, node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){

	int seen_state = op->relocate_op.state;
	if (seen_state == STATE_OP_ONGOING) {
		// VCAS in original implementation
		operation_t* seen_op = CAS_PTR(&(op->relocate_op.dest->op), op->relocate_op.dest_op, FLAG(op, STATE_OP_RELOCATE));
		if ((seen_op == op->relocate_op.dest_op) || (seen_op == (operation_t *)FLAG(op, STATE_OP_RELOCATE))){
			CAS_PTR(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_SUCCESSFUL);
			seen_state = STATE_OP_SUCCESSFUL;
		} else {
			// VCAS in original implementation
			seen_state = CAS_PTR(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_FAILED);
		}
	}

	if (seen_state == STATE_OP_SUCCESSFUL) {

		CAS_PTR(&(op->relocate_op.dest->key), op->relocate_op.remove_key, op->relocate_op.replace_key);
		CAS_PTR(&(op->relocate_op.dest->op), FLAG(op, STATE_OP_RELOCATE), FLAG(op, STATE_OP_NONE));
	}

	bool_t result = (seen_state == STATE_OP_SUCCESSFUL);
	if (op->relocate_op.dest == curr) {
		return result;
	}

	CAS_PTR(&(curr->op), FLAG(op, STATE_OP_RELOCATE), FLAG(op, result ? STATE_OP_MARK : STATE_OP_NONE));
	if (result) {
		if (op->relocate_op.dest == pred) {
			pred_op = (operation_t *)FLAG(op, STATE_OP_NONE);
		}
		bst_help_marked(pred, pred_op, curr, root);
	}
	return result;
}

void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){

	node_t* new_ref;
	if (ISNULL(curr->left)) {
		if (ISNULL(curr->right)) {
			new_ref = (node_t*)SETNULL(curr);
		} else {
			new_ref = curr->right;
		}
	} else {
		new_ref = curr->left;
	}

	operation_t* cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
	cas_op->child_cas_op.is_left = (curr == pred->left);
	cas_op->child_cas_op.expected = curr;
	cas_op->child_cas_op.update = new_ref;

	if (CAS_PTR(&(pred->op), pred_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == pred_op) {
		bst_help_child_cas(cas_op, pred, root);
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

unsigned long bst_size(node_t* node) {
	if (ISNULL(node)) {
		return 0;
	} else if (GETFLAG(node->op) != STATE_OP_MARK) {
		return 1 + bst_size(node->right) + bst_size(node->left);
	} else {
		return bst_size(node->right) + bst_size(node->left);
	}
}

void bst_print(node_t* node){
	if (ISNULL(node)) {
		return;
	}
	fprintf(stderr, "key: %lu ", node->key);
	fprintf(stderr, "address %p ", node);
	fprintf(stderr, "left: %p; right: %p, op: %p \n", node->left, node->right, node->op);
	
	bst_print(node->left);
	bst_print(node->right);
}
