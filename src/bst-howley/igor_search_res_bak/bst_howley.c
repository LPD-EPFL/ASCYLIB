/*   
 *   File: bst_howley.c
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>,
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

// TODO initialize structures similar to my_search_result from bst_ellen?



//node_t* root;

node_t* bst_initialize(int num_proc) {
        node_t* root = (node_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(node_t));

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root
	root->key = 0;
	root->left = NULL;
	root->right = NULL;
	root->op = NULL;

	my_search_result = (bst_search_result_t**) malloc(num_proc * sizeof(bst_search_result_t*));
	
	// should we create an op pointer and flag it with NONE?

	 // fprintf(stderr, "root address: %p, key address %p, left node addr: %p, right node addr: %p, op addr: %p\n", (unsigned long)root, &(root->key), &(root->left), &(root->right), &(root->op)
	 // );

	 // fprintf(stderr, "sizes in bytes: int = %d, skey_t = %d, operation_t* = %d, node_t* = %d, node_t = %d, bool_t = %d, child_cas_op_t = %d, relocate_op_t = %d, operation_t = %d, search_res_t = %d \n", sizeof(int), sizeof(skey_t), sizeof(operation_t*), sizeof(node_t*), sizeof(node_t), sizeof(bool_t), sizeof(child_cas_op_t), sizeof(relocate_op_t), sizeof(operation_t), sizeof(search_res_t));


	return root;
}

void bst_init_local(int id){
    my_search_result[id] = (bst_search_result_t*) malloc(sizeof(bst_search_result_t));
}


bool_t bst_contains(skey_t k, node_t* root, int id){
	
	//fprintf(stderr, "bst contains\n");

	// node_t* pred;
	// node_t* curr;
	// operation_t* pred_op;
	// operation_t* curr_op;


	// root is now a global pointer to a node, not a node
	return bst_find(k, /*&pred, &pred_op, &curr, &curr_op,*/ root, root, id)->result == FOUND;
	// return TRUE;
}

bst_search_result_t* bst_find(skey_t k, node_t* aux_root, node_t* root, int id){
	//fprintf(stderr, "bst find\n");
	bst_search_result_t* my_result = my_search_result[id];

	// search_res_t result;
	skey_t curr_key;
	node_t* next;
	node_t* last_right;
	operation_t* last_right_op;

retry:
	my_result->result = NOT_FOUND_R;
	my_result->curr = aux_root;
	my_result->curr_op = my_result->curr->op;

	if(GETFLAG(my_result->curr_op) != STATE_OP_NONE){
		fprintf(stderr, "Shouldn't be here\n");
		//root is now a pointer to a node, not a node
		if (aux_root == root){
			bst_help_child_cas((operation_t*)UNFLAG(my_result->curr_op), my_result->curr/*, aux_root*/);
			goto retry;
		} else {
			my_result->result = ABORT;
			return my_result;
		}
	}


	next = my_result->curr->right;
	last_right = my_result->curr;
	last_right_op = my_result->curr_op;

	while (!ISNULL(next)){

		my_result->pred = my_result->curr;
		my_result->pred_op = my_result->curr_op;
		my_result->curr = next;
		my_result->curr_op = (my_result->curr)->op;


		if(GETFLAG(my_result->curr_op) != STATE_OP_NONE){
			fprintf(stderr, "Shouldn't be here 2\n");
			bst_help(my_result->pred, my_result->pred_op, my_result->curr, my_result->curr_op/*, aux_root*/);
			goto retry;
		}
		curr_key = (my_result->curr)->key;
		if(k < curr_key){
			my_result->result = NOT_FOUND_L;
			next = (my_result->curr)->left;
		} else if(k > curr_key) {
			my_result->result = NOT_FOUND_R;
			next = (my_result->curr)->right;
			last_right = my_result->curr;
			last_right_op = my_result->curr_op;
		} else{
			my_result->result = FOUND;
			break;
		}
	}
	
	if ((my_result->result != FOUND) && (last_right_op != last_right->op)) {
		fprintf(stderr, "Shouldn't be here 3\n");
		goto retry;
	}

	if ((my_result->curr)->op != my_result->curr_op){
		fprintf(stderr, "Shouldn't be here 4\n");
		goto retry;
	}

	return my_result;
} 
  
bool_t bst_add(skey_t k, node_t* root, int id){
	//fprintf(stderr, "bst add\n");
	// node_t* pred;
	// node_t* curr;
	node_t* new_node;
	// operation_t* pred_op;
	// operation_t* curr_op;
	operation_t* cas_op;
	// search_res_t result;
	bst_search_result_t* my_result;

	while(TRUE) {
		//root is now a global pointer to a node, not a node
		my_result = bst_find(k, /*&pred, &pred_op, &curr, &curr_op, */root, root, id);
		if (my_result->result == FOUND) {
			return FALSE;
		}
		// allocate memory 
		// new_node = new Node(k);
		new_node = (node_t*) ssalloc(sizeof(node_t));
		new_node->key = k;
		new_node->op = NULL;
		new_node->left = NULL;
		new_node->right = NULL;

		// fprintf(stderr, "new_node address: %p, 64bit aligned: %d key address %p, left node addr: %p, right node addr: %p, op addr: %p\n", new_node, ((unsigned long)new_node & 7) == 0,&(new_node->key), &(new_node->left), &(new_node->right), &(new_node->op)
	 // );

		bool_t is_left = (my_result->result == NOT_FOUND_L);
		node_t* old;
		if (is_left) {
			old = my_result->curr->left;
		} else {
			old = my_result->curr->right;
		}

		// allocate memory
		//cas_op = new child_cas_op_t(is_left, old, new_node)
		cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
		cas_op->child_cas_op.is_left = is_left;
		cas_op->child_cas_op.expected = old;
		cas_op->child_cas_op.update = new_node;

		// fprintf(stderr, "cas_op address: %p, is_left address: %p, expected addr: %p, update addr: %p\n", (unsigned long)cas_op, &(cas_op->child_cas_op.is_left), &(cas_op->child_cas_op.expected), &(cas_op->child_cas_op.update)
	 // );

		if (CAS_PTR(&(my_result->curr->op), my_result->curr_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == my_result->curr_op) {
			// legit cast? YES!! verif
			bst_help_child_cas(cas_op, my_result->curr/*, root*/);
			return TRUE;
		}
	}
}

void bst_help_child_cas(operation_t* op, node_t* dest/*, node_t* root*/){
	//fprintf(stderr, "bst help child cas\n");
	node_t** address = NULL;
	if (op->child_cas_op.is_left) {
		address = &(dest->left);
	} else {
		address = &(dest->right);
	}
	CAS_PTR(address, op->child_cas_op.expected, op->child_cas_op.update);
	CAS_PTR(&(dest->op), FLAG(op, STATE_OP_CHILDCAS), FLAG(op, STATE_OP_NONE));
}

bool_t bst_remove(skey_t k, node_t* root, int id){
	//fprintf(stderr, "bst remove\n");
	// node_t* pred;
	// node_t* curr;
	node_t* replace = NULL;
	// operation_t* pred_op;
	// operation_t* curr_op;
	operation_t* replace_op = NULL;
	operation_t* reloc_op = NULL;
	bst_search_result_t* my_result;

	while(TRUE) {
		//root is now a global pointer to a node, not a node
		my_result = bst_find(k, /*&pred, &pred_op, &curr, &curr_op,*/ root, root, id);
		if (my_result->result != FOUND) {
			return FALSE;
		}

		if (ISNULL(my_result->curr->right) || ISNULL(my_result->curr->left)) { // node has less than two children
			if (CAS_PTR(&(my_result->curr->op), my_result->curr_op, FLAG(my_result->curr_op, STATE_OP_MARK)) == my_result->curr_op) {
				bst_help_marked(my_result->pred, my_result->pred_op, my_result->curr/*, root*/);
				return TRUE;
			}
		} else { // node has two children
			node_t* curr = my_result->curr;
			my_search_result[id]->pred = my_result->pred;
			my_search_result[id]->pred_op = my_result->pred_op;
			my_search_result[id]->curr = replace;
			my_search_result[id]->curr_op = replace_op;

			// my_result = bst_find(k, &pred, &pred_op, &replace, &replace_op, curr, root, id);
			my_result = bst_find(k, curr, root, id);
			if ((my_result->result == ABORT) || (my_result->curr->op != my_result->curr_op)) {
				continue;
			} 

			//allocate memory
			//reloc_op = new RelocateOP(curr, curr_op, k, replace->key);
			reloc_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
			reloc_op->relocate_op.state = STATE_OP_ONGOING;
			reloc_op->relocate_op.dest = my_result->curr;
			reloc_op->relocate_op.dest_op = my_result->curr_op;
			reloc_op->relocate_op.remove_key = k;
			reloc_op->relocate_op.replace_key = replace->key;

			// fprintf(stderr, "reloc_op address: %p, state address: %p, dest addr: %p, dest_op addr: %p, remove_key addr: %p, replace_key addr: %p \n", (unsigned long)reloc_op,  &(reloc_op->relocate_op.state), &(reloc_op->relocate_op.dest), &(reloc_op->relocate_op.dest_op), &(reloc_op->relocate_op.remove_key), &(reloc_op->relocate_op.replace_key)
	 	// 	);

			if (CAS_PTR(&(replace->op), replace_op, FLAG(reloc_op, STATE_OP_RELOCATE)) == replace_op) {
				if (bst_help_relocate(reloc_op, my_result->pred, my_result->pred_op, replace/*, root*/)) {
					return TRUE;
				}
			}
		}
	}
}

bool_t bst_help_relocate(operation_t* op, node_t* pred, operation_t* pred_op, node_t* curr/*, node_t* root*/){
	//fprintf(stderr, "bst help relocate\n");
	int seen_state = op->relocate_op.state;
	if (seen_state == STATE_OP_ONGOING) {
		//VCAS in original implementation
		operation_t* seen_op = CAS_PTR(&(op->relocate_op.dest->op), op->relocate_op.dest_op, FLAG(op, STATE_OP_RELOCATE));
		if ((seen_op == op->relocate_op.dest_op) || (seen_op == (operation_t *)FLAG(op, STATE_OP_RELOCATE))){
			CAS_PTR(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_SUCCESSFUL);
			seen_state = STATE_OP_SUCCESSFUL;
		} else {
			// VCAS
			seen_state = CAS_PTR(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_FAILED);
		}
	}

	if (seen_state == STATE_OP_SUCCESSFUL) {
		// TODO not clear in the paper code
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
		bst_help_marked(pred, pred_op, curr/*, root*/);
	}
	return result;
}

void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr/*, node_t* root*/){

	//fprintf(stderr, "bst help marked\n");
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

	// allocate memory
	// operation_t* cas_op = new child_cas_op(curr==pred->left, curr, new_ref);
	operation_t* cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
	cas_op->child_cas_op.is_left = (curr == pred->left);
	cas_op->child_cas_op.expected = curr;
	cas_op->child_cas_op.update = new_ref;

		// fprintf(stderr, "cas_op address: %p, is_left address: %p, expected addr: %p, update addr: %p\n", (unsigned long)cas_op, &(cas_op->child_cas_op.is_left), &(cas_op->child_cas_op.expected), &(cas_op->child_cas_op.update)
	 // );

	if (CAS_PTR(&(pred->op), pred_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == pred_op) {
		bst_help_child_cas(cas_op, pred/*, root*/);
	}
}

void bst_help(node_t* pred, operation_t* pred_op, node_t* curr, operation_t* curr_op/*, node_t* root */){
	
	//fprintf(stderr, "bst help\n");
	if (GETFLAG(curr_op) == STATE_OP_CHILDCAS) {
		bst_help_child_cas((operation_t*)UNFLAG(curr_op), curr/*, root*/);
	} else if (GETFLAG(curr_op) == STATE_OP_RELOCATE) {
		bst_help_relocate((operation_t*)UNFLAG(curr_op), pred, pred_op, curr/*, root*/);
	} else if (GETFLAG(curr_op) == STATE_OP_MARK) {
		bst_help_marked(pred, pred_op, curr/*, root*/);
	}
}

unsigned long bst_size(node_t* node) {
	if (ISNULL(node)) {
		return 0;
	} else {
		// fprintf(stderr, "node %p ; left: %p; right: %p\n", node, node->left, node->right);
		return 1 + bst_size(node->right) + bst_size(node->left);
	}
}

void bst_print(node_t* node){
	if (ISNULL(node)) {
		return;
	}
	fprintf(stderr, "key: %lu ", node->key);
	fprintf(stderr, "address %p ", node);
	fprintf(stderr, "left: %p; right: %p \n", node->left, node->right);
	
	bst_print(node->left);
	bst_print(node->right);
}
