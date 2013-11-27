#include "bst_howley.h"

// TODO initialize structures similar to my_search_result from bst_ellen?



//node_t* root;

bst_key_t glob_key = 1;

node_t* bst_initialize() {


	printf("[alloc] root\n");

	node_t* root = (node_t*) ssalloc(sizeof(node_t));

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root
	root->key = 0;
	root->left = NULL;
	root->right = NULL;
	root->op = NULL;
	
	// should we create an op pointer and flag it with NONE?
	return root;
}


bool_t bst_contains(bst_key_t k, node_t* root){
	
	//fprintf(stderr, "bst contains\n");

	node_t* pred;
	node_t* curr;
	operation_t* pred_op;
	operation_t* curr_op;


	// root is now a global pointer to a node, not a node
	return bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root) == FOUND;
	// return TRUE;
}

search_res_t bst_find(bst_key_t k, node_t** pred, operation_t** pred_op, node_t** curr, operation_t** curr_op, node_t* aux_root, node_t* root){
	//fprintf(stderr, "bst find\n");

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
		//fprintf(stderr, "\nShouldn't be here\n");
		//root is now a global pointer to a node, not a node
		if (aux_root == root){
			bst_help_child_cas((operation_t*)UNFLAG(*curr_op), *curr, aux_root);
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
			//fprintf(stderr, "\nShouldn't be here 2\n");
			bst_help(*pred, *pred_op, *curr, *curr_op, aux_root);
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
		//fprintf(stderr, "\nShouldn't be here 3\n");
		goto retry;
	}

	if ((*curr)->op != *curr_op){
		//fprintf(stderr, "\nShouldn't be here 4\n");
		goto retry;
	}

	return result;
} 
  
bool_t bst_add(bst_key_t k, node_t* root, int node_id){
	//fprintf(stderr, "bst add\n");
	node_t* pred;
	node_t* curr;
	node_t* new_node;
	operation_t* pred_op;
	operation_t* curr_op;
	operation_t* cas_op;
	search_res_t result;

	while(TRUE) {
		//root is now a global pointer to a node, not a node
		result = bst_find(k, &pred, &pred_op, &curr, &curr_op, root, root);
		if (result == FOUND) {
			return FALSE;
		}
		// allocate memory 
		// new_node = new Node(k);
		printf("[%d][alloc] node\n", node_id);

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

		// allocate memory
		//cas_op = new child_cas_op_t(is_left, old, new_node)
		printf("[%d][alloc] cas_op\n", node_id);

		cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
		cas_op->child_cas_op.is_left = is_left;
		cas_op->child_cas_op.expected = old;
		cas_op->child_cas_op.update = new_node;

		if (CAS_PTR(&curr->op, curr_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == curr_op) {
			// legit cast? YES!! verif
			bst_help_child_cas(cas_op, curr, root);
			// if (k == glob_key) {
			printf("[%d]key %d was allocated at address: %p\n",node_id, k, new_node);
			// } 
			return TRUE;
		}
	}
}

void bst_help_child_cas(operation_t* op, node_t* dest, node_t* root){
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

bool_t bst_remove(bst_key_t k, node_t* root){
	//fprintf(stderr, "bst remove\n");
	node_t* pred;
	node_t* curr;
	node_t* replace;
	operation_t* pred_op;
	operation_t* curr_op;
	operation_t* replace_op;
	operation_t* reloc_op;

	while(TRUE) {
		//root is now a global pointer to a node, not a node
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

			//allocate memory
			//reloc_op = new RelocateOP(curr, curr_op, k, replace->key);
			
			printf("[alloc] Relocate op\n");
			reloc_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
			reloc_op->relocate_op.state = STATE_OP_ONGOING;
			reloc_op->relocate_op.dest = curr;
			reloc_op->relocate_op.dest_op = curr_op;
			reloc_op->relocate_op.remove_key = k;
			reloc_op->relocate_op.replace_key = replace->key;

			if (CAS_PTR(&(replace->op), replace_op, FLAG(reloc_op, STATE_OP_RELOCATE)) == replace_op) {
				if (bst_help_relocate(reloc_op, pred, pred_op, replace, root)) {
					return TRUE;
				}
			}
		}
	}
}

bool_t bst_help_relocate(operation_t* op, node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){
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
		bst_help_marked(pred, pred_op, curr, root);
	}
	return result;
}

void bst_help_marked(node_t* pred, operation_t* pred_op, node_t* curr, node_t* root){

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

	printf("[alloc] cas_op\n");
	operation_t* cas_op = (operation_t*) ssalloc_alloc(1, sizeof(operation_t));
	cas_op->child_cas_op.is_left = (curr == pred->left);
	cas_op->child_cas_op.expected = curr;
	cas_op->child_cas_op.update = new_ref;

	if (CAS_PTR(&(pred->op), pred_op, FLAG(cas_op, STATE_OP_CHILDCAS)) == pred_op) {
		bst_help_child_cas(cas_op, pred, root);
	}
}

void bst_help(node_t* pred, operation_t* pred_op, node_t* curr, operation_t* curr_op, node_t* root ){
	
	//fprintf(stderr, "bst help\n");
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
