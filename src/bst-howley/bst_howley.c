#include "bst_howley.h"

//TODO memory allocation (for whom?)
//define root?

bool_t bst_contains(bst_key_t k){
	
	fprintf(stderr, "bst contains\n");

	node_t* pred;
	node_t* curr;
	operation_t* pred_op;
	operation_t* curr_op;
	return bst_find(k, &pred, &pred_op, &curr, &curr_op, &root);
}

search_res_t bst_find(bst_key_t k, node_t** pred, operation_t** pred_op, node_t** curr, operation_t** curr_op, node_t* aux_root){
	fprintf(stderr, "bst find\n");

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
		if (aux_root == &root){
			bst_help_child_cas(((child_cas_op_t*)UNFLAG(*curr_op)), *curr);
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
			bst_help(*pred, *pred_op, *curr, *curr_op);
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
  
bool_t bst_add(bst_key_t k){
	fprintf(stderr, "bst add\n");
	node_t* pred;
	node_t* curr;
	node_t* new_node;
	operation_t* pred_op;
	operation_t* curr_op;
	operation_t* cas_op;
	search_res_t result;

	while(TRUE) {
		result = bst_find(k, &pred, &pred_op, &curr, &curr_op, &root);
		if (result == FOUND) {
			return FALSE;
		}
		// TODO allocate memory 
		// new_node = new Node(k);
		bool_t is_left = (result == NOT_FOUND_L);
		node_t* old;
		if (is_left) {
			old = curr->left;
		} else {
			old = curr->right;
		}
		// cas_op = new child_cas_op_t(is_left, old, new_node)
		// TODO allocate memory
		if (CAS_PTR(&curr->op, curr_op, FLAG(cas_op, STATE_OP_CHILDCAS))) {
			bst_help_child_cas((child_cas_op_t*)cas_op, curr);
			return TRUE;
		}
	}
}

void bst_help_child_cas(child_cas_op_t* op, node_t* dest){
	fprintf(stderr, "bst help child\n");
	node_t** address = NULL;
	if (op->is_left) {
		address = &(dest->left);
	} else {
		address = &(dest->right);
	}
	CAS_PTR(address, op->expected, op->update);
	CAS_PTR(&(dest->op), FLAG((operation_t*)op, STATE_OP_CHILDCAS), FLAG((operation_t*)op, STATE_OP_NONE));
}

bool_t bst_remove(bst_key_t k){
	fprintf(stderr, "bst remove\n");
	return 0;
}
bool_t bst_help_relocate(relocate_op_t* op, node_t* pred, operation_t* pred_op, node_t* curr){
	fprintf(stderr, "bst help relocate\n");
	return 0;
}

void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr){

	fprintf(stderr, "bst help marked\n");

}
void bst_help(node_t* pred, operation_t* pred_op, node_t* curr, operation_t* curr_op ){
	fprintf(stderr, "bst help\n");
}
