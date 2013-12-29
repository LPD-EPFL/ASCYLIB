#include "bst_howley.h"

// TODO initialize structures similar to my_search_result from bst_ellen?

node_t* bst_initialize() {

	MEM_BARRIER;
	// printf("[alloc] root\n");
	// node_t* root = (node_t*) ssalloc(sizeof(node_t));
	node_t* root = (node_t*) ssalloc(CACHE_LINE_SIZE);
	MEM_BARRIER;

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root
	root->key = 0;
	MEM_BARRIER;
	root->left = NULL;
	MEM_BARRIER;
	root->right = NULL;
	MEM_BARRIER;
	root->op = NULL;
	
	MEM_BARRIER;
	// should we create an op pointer and flag it with NONE?
	return root;
}


bool_t bst_contains(bst_key_t k, node_t* root){
	
	//fprintf(stderr, "bst contains\n");

	MEM_BARRIER;
	node_t* pred;
	MEM_BARRIER;
	node_t* curr;
	MEM_BARRIER;
	operation_t* pred_op;
	MEM_BARRIER;
	operation_t* curr_op;
	MEM_BARRIER;

	// root is now a global pointer to a node, not a node
	return bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root) == FOUND;
	// return TRUE;
}

search_res_t bst_find(bst_key_t k, node_t** pred, operation_t** pred_op, node_t** curr, operation_t** curr_op, node_t* aux_root, node_t* root){
	//fprintf(stderr, "bst find\n");

	MEM_BARRIER;
	search_res_t result;
	MEM_BARRIER;
	bst_key_t curr_key;
	MEM_BARRIER;
	node_t* next;
	MEM_BARRIER;
	node_t* last_right;
	MEM_BARRIER;
	operation_t* last_right_op;
	MEM_BARRIER;

retry:
	MEM_BARRIER;
	result = NOT_FOUND_R;
	MEM_BARRIER;
	*curr = aux_root;
	MEM_BARRIER;
	*curr_op = (*curr)->op;
	MEM_BARRIER;

	if(GETFLAG(*curr_op) != STATE_OP_NONE){
		//fprintf(stderr, "\nShouldn't be here\n");
		//root is now a global pointer to a node, not a node
		MEM_BARRIER;
		if (aux_root == root){
			MEM_BARRIER;
			bst_help_child_cas((operation_t*)UNFLAG(*curr_op), *curr, aux_root);
			MEM_BARRIER;
			goto retry;
		} else {
			MEM_BARRIER;
			return ABORT;
		}
	}

	MEM_BARRIER;
	next = (*curr)->right;
	MEM_BARRIER;
	last_right = *curr;
	MEM_BARRIER;
	last_right_op = *curr_op;
	MEM_BARRIER;

	while (!ISNULL(next)){
		MEM_BARRIER;
		*pred = *curr;
		MEM_BARRIER;
		*pred_op = *curr_op;
		MEM_BARRIER;
		*curr = next;
		MEM_BARRIER;
		*curr_op = (*curr)->op;
		MEM_BARRIER;

		if(GETFLAG(*curr_op) != STATE_OP_NONE){
			//fprintf(stderr, "\nShouldn't be here 2\n");
			MEM_BARRIER;
			bst_help(*pred, *pred_op, *curr, *curr_op, aux_root);
			MEM_BARRIER;
			goto retry;
		}
		MEM_BARRIER;
		curr_key = (*curr)->key;
		MEM_BARRIER;
		if(k < curr_key){
			MEM_BARRIER;
			result = NOT_FOUND_L;
			MEM_BARRIER;
			next = (*curr)->left;
			MEM_BARRIER;
		} else if(k > curr_key) {
			MEM_BARRIER;
			result = NOT_FOUND_R;
			MEM_BARRIER;
			next = (*curr)->right;
			MEM_BARRIER;
			last_right = *curr;
			MEM_BARRIER;
			last_right_op = *curr_op;
			MEM_BARRIER;
		} else{
			MEM_BARRIER;
			result = FOUND;
			MEM_BARRIER;
			break;
		}
	}
	
	MEM_BARRIER;
	if ((result != FOUND)) {
		MEM_BARRIER;
		if (last_right_op != last_right->op){
			//fprintf(stderr, "\nShouldn't be here 3\n");
			MEM_BARRIER;
			goto retry;
		}
	}

	MEM_BARRIER;
	if ((*curr)->op != *curr_op){
		//fprintf(stderr, "\nShouldn't be here 4\n");$
		MEM_BARRIER;
		goto retry;
	}

	MEM_BARRIER;
	return result;
} 
  
bool_t bst_add(bst_key_t k, node_t* root){
	//fprintf(stderr, "bst add\n");
	MEM_BARRIER;
	node_t* pred;
	MEM_BARRIER;
	node_t* curr;
	MEM_BARRIER;
	node_t* new_node;
	MEM_BARRIER;
	operation_t* pred_op;
	MEM_BARRIER;
	operation_t* curr_op;
	MEM_BARRIER;
	operation_t* cas_op;
	MEM_BARRIER;
	search_res_t result;

	MEM_BARRIER;
	while(TRUE) {
		//root is now a global pointer to a node, not a node
		MEM_BARRIER;
		result = bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root);
		MEM_BARRIER;
		if (result == FOUND) {
			MEM_BARRIER;
			return FALSE;
		}
		// allocate memory 
		// new_node = new Node(k);
		// printf("[%d][alloc] node\n", node_id);

		MEM_BARRIER;
		new_node = (node_t*) ssalloc(sizeof(node_t));
		MEM_BARRIER;
		new_node->key = k;
		MEM_BARRIER;
		new_node->op = NULL;
		MEM_BARRIER;
		new_node->left = NULL;
		MEM_BARRIER;
		new_node->right = NULL;
		MEM_BARRIER;

		bool_t is_left = (result == NOT_FOUND_L);
		MEM_BARRIER;
		node_t* old;
		MEM_BARRIER;
		if (is_left) {
			MEM_BARRIER;
			old = curr->left;
		} else {
			MEM_BARRIER;
			old = curr->right;
		}
		MEM_BARRIER;
		// allocate memory
		//cas_op = new child_cas_op_t(is_left, old, new_node)
		// printf("[%d][alloc] cas_op\n", node_id);

		cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
		MEM_BARRIER;
		cas_op->child_cas_op.is_left = is_left;
		MEM_BARRIER;
		cas_op->child_cas_op.expected = old;
		MEM_BARRIER;
		cas_op->child_cas_op.update = new_node;

		MEM_BARRIER;
		if (CAS_PTR(&curr->op, curr_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == curr_op) {
			MEM_BARRIER;
			bst_help_child_cas(cas_op, curr, root);

			MEM_BARRIER;
			return TRUE;
		}
	}
	MEM_BARRIER;
}

void bst_help_child_cas(operation_t* op, node_t* dest, node_t* root){
	//fprintf(stderr, "bst help child cas\n");
	MEM_BARRIER;
	volatile node_t** address = NULL;
	MEM_BARRIER;
	if (op->child_cas_op.is_left) {
		MEM_BARRIER;
		address = &(dest->left);
	} else {
		MEM_BARRIER;
		address = &(dest->right);
	}
	MEM_BARRIER;
	CAS_PTR(address, op->child_cas_op.expected, op->child_cas_op.update);
	MEM_BARRIER;
	CAS_PTR(&(dest->op), FLAG(op, STATE_OP_CHILDCAS), FLAG(op, STATE_OP_NONE));
	MEM_BARRIER;
}

bool_t bst_remove(bst_key_t k, node_t* root){
	//fprintf(stderr, "bst remove\n");
	MEM_BARRIER;
	node_t* pred;
	MEM_BARRIER;
	node_t* curr;
	MEM_BARRIER;
	node_t* replace;
	MEM_BARRIER;
	operation_t* pred_op;
	MEM_BARRIER;
	operation_t* curr_op;
	MEM_BARRIER;
	operation_t* replace_op;
	MEM_BARRIER;
	operation_t* reloc_op;
	MEM_BARRIER;

	while(TRUE) {
		//root is now a global pointer to a node, not a node
		MEM_BARRIER;
		if (bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root) != FOUND) {
			MEM_BARRIER;
			return FALSE;
		}

		MEM_BARRIER;
		bool_t no_right_child = ISNULL(curr->right);
		MEM_BARRIER;
		bool_t no_left_child = ISNULL(curr->left);
		MEM_BARRIER;
		if (no_right_child || no_left_child) { // node has less than two children
			MEM_BARRIER;
			if (CAS_PTR(&(curr->op), curr_op, FLAG(curr_op, STATE_OP_MARK)) == curr_op) {
				MEM_BARRIER;
				bst_help_marked(pred, pred_op, curr, root);
				MEM_BARRIER;
				return TRUE;
			}
		} else { // node has two children
			MEM_BARRIER;
			if ((bst_find(k, &pred, &pred_op, &replace, &replace_op, curr, root) == ABORT) || (curr->op != curr_op)) {
				MEM_BARRIER;
				continue;
			} 

			//allocate memory
			//reloc_op = new RelocateOP(curr, curr_op, k, replace->key);
			
			// printf("[alloc] Relocate op\n");
			MEM_BARRIER;
			reloc_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
			MEM_BARRIER;
			reloc_op->relocate_op.state = STATE_OP_ONGOING;
			MEM_BARRIER;
			reloc_op->relocate_op.dest = curr;
			MEM_BARRIER;
			reloc_op->relocate_op.dest_op = curr_op;
			MEM_BARRIER;
			reloc_op->relocate_op.remove_key = k;
			MEM_BARRIER;
			reloc_op->relocate_op.replace_key = replace->key;

			MEM_BARRIER;
			if (CAS_PTR(&(replace->op), replace_op, FLAG(reloc_op, STATE_OP_RELOCATE)) == replace_op) {
				MEM_BARRIER;
				if (bst_help_relocate(reloc_op, pred, pred_op, replace, root)) {
					MEM_BARRIER;
					return TRUE;
				}
				MEM_BARRIER;
			}
			MEM_BARRIER;
		}
		MEM_BARRIER;
	}
	MEM_BARRIER;
}

bool_t bst_help_relocate(operation_t* op, node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){
	//fprintf(stderr, "bst help relocate\n");
	MEM_BARRIER;
	int seen_state = op->relocate_op.state;
	MEM_BARRIER;
	if (seen_state == STATE_OP_ONGOING) {
		//VCAS in original implementation
		MEM_BARRIER;
		operation_t* seen_op = CAS_PTR(&(op->relocate_op.dest->op), op->relocate_op.dest_op, FLAG(op, STATE_OP_RELOCATE));
		MEM_BARRIER;
		bool_t is_relocating_op = (seen_op == op->relocate_op.dest_op);
		MEM_BARRIER;
		bool_t is_relocating_flag = (seen_op == (operation_t *)FLAG(op, STATE_OP_RELOCATE));
		MEM_BARRIER;
		if (is_relocating_op || is_relocating_flag){

			MEM_BARRIER;
			CAS_PTR(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_SUCCESSFUL);
			MEM_BARRIER;
			seen_state = STATE_OP_SUCCESSFUL;
			MEM_BARRIER;
		} else {
			// VCAS
			MEM_BARRIER;
			seen_state = CAS_PTR(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_FAILED);
			MEM_BARRIER;
		}
		MEM_BARRIER;
	}

	if (seen_state == STATE_OP_SUCCESSFUL) {
		// TODO not clear in the paper code
		MEM_BARRIER;
		CAS_PTR(&(op->relocate_op.dest->key), op->relocate_op.remove_key, op->relocate_op.replace_key);
		MEM_BARRIER;
		CAS_PTR(&(op->relocate_op.dest->op), FLAG(op, STATE_OP_RELOCATE), FLAG(op, STATE_OP_NONE));
		MEM_BARRIER;
	}
	MEM_BARRIER;
	bool_t result = (seen_state == STATE_OP_SUCCESSFUL);
	MEM_BARRIER;
	if (op->relocate_op.dest == curr) {
		MEM_BARRIER;
		return result;
	}

	MEM_BARRIER;
	CAS_PTR(&(curr->op), FLAG(op, STATE_OP_RELOCATE), FLAG(op, result ? STATE_OP_MARK : STATE_OP_NONE));
	MEM_BARRIER;
	if (result) {
		MEM_BARRIER;
		if (op->relocate_op.dest == pred) {
			MEM_BARRIER;
			pred_op = (operation_t *)FLAG(op, STATE_OP_NONE);
			MEM_BARRIER;
		}
		MEM_BARRIER;
		bst_help_marked(pred, pred_op, curr, root);
		MEM_BARRIER;
	}
	MEM_BARRIER;
	return result;
}

void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){

	//fprintf(stderr, "bst help marked\n");
	MEM_BARRIER;
	node_t* new_ref;
	MEM_BARRIER;
	if (ISNULL(curr->left)) {
		MEM_BARRIER;
		if (ISNULL(curr->right)) {
			MEM_BARRIER;
			new_ref = (node_t*)SETNULL(curr);
			MEM_BARRIER;
		} else {
			MEM_BARRIER;
			new_ref = curr->right;
			MEM_BARRIER;
		}
	} else {
		MEM_BARRIER;
		new_ref = curr->left;
		MEM_BARRIER;
	}

	MEM_BARRIER;

	// allocate memory
	// operation_t* cas_op = new child_cas_op(curr==pred->left, curr, new_ref);

	// printf("[alloc] cas_op\n");
	MEM_BARRIER;
	operation_t* cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
	MEM_BARRIER;
	cas_op->child_cas_op.is_left = (curr == pred->left);
	MEM_BARRIER;
	cas_op->child_cas_op.expected = curr;
	MEM_BARRIER;
	cas_op->child_cas_op.update = new_ref;

	MEM_BARRIER;
	if (CAS_PTR(&(pred->op), pred_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == pred_op) {
		MEM_BARRIER;
		bst_help_child_cas(cas_op, pred, root);
		MEM_BARRIER;
	}
	MEM_BARRIER;
}

void bst_help(node_t* pred, operation_t* pred_op, node_t* curr, operation_t* curr_op, node_t* root ){
	
	//fprintf(stderr, "bst help\n");
	MEM_BARRIER;
	if (GETFLAG(curr_op) == STATE_OP_CHILDCAS) {
		MEM_BARRIER;
		bst_help_child_cas((operation_t*)UNFLAG(curr_op), curr, root);
	} else if (GETFLAG(curr_op) == STATE_OP_RELOCATE) {
		MEM_BARRIER;
		bst_help_relocate((operation_t*)UNFLAG(curr_op), pred, pred_op, curr, root);
	} else if (GETFLAG(curr_op) == STATE_OP_MARK) {
		MEM_BARRIER;
		bst_help_marked(pred, pred_op, curr, root);
	}
	MEM_BARRIER;
}

unsigned long bst_size(node_t* node) {
	MEM_BARRIER;
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