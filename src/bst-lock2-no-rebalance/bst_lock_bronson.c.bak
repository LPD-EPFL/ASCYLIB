#include "bst_lock_bronson.h"
#include <pthread.h>

// node_t* root;

node_t* bst_initialize() {
	// printf("bst_initialize\n");
	node_t* root = (node_t*) ssalloc(sizeof(node_t));

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root
	root->key = 0;
	root->value = &root; //wanted to avoid using ssalloc for the dummy value stored in the root placeholder node.
	root->left = NULL;
	root->right = NULL;
	root->height = 0;
	root->version = 0;
	root->parent = NULL;
	INIT_LOCK(&(root->lock));
	
	// should we create an op pointer and flag it with NONE?
	return root;
}


bst_value_t* bst_get(bst_key_t k, node_t* root){
	// printf("bst_get\n");
	return bst_attempt_get(k, root, TRUE, 0);
}

bst_value_t* bst_attempt_get(bst_key_t k, node_t* node, bool_t is_right, bst_version_t node_v){
	// printf("bst_attempt_get\n");

	while(TRUE){
		node_t* child = CHILD(node, is_right);
		if (((node->version ^ node_v) & IGNORE_GROW) != 0) {
			return RETRY;
		}

		if (child == NULL) {
			return NULL;
		}

		if (k == child->key) {
			return child->value;
		} 

		bool_t next_is_right = k > child->key ? TRUE : FALSE;
		bst_version_t ch_v = child->version;

		if ((ch_v & SHRINKING) != 0) {
			//TODO define
			bst_wait_until_not_changing(child);
		} else if (ch_v != UNLINKED && child==CHILD(node, is_right)) {

			if (((node->version ^ node_v) & IGNORE_GROW) != 0) {
				return RETRY;
			}

			bst_value_t* p = bst_attempt_get(k, child, next_is_right, ch_v);
			if (p != RETRY) {
				return p;
			}
		}
	}
}

bst_value_t* bst_put(bst_key_t k, bst_value_t* v, node_t* root){
	// printf("bst_put\n");

	return bst_attempt_put(k, v, root, TRUE, 0);
}

bst_value_t* bst_attempt_put(bst_key_t k, bst_value_t* v, node_t* node, bool_t is_right, bst_version_t node_v){
	// printf("[%d] bst_attempt_put\n", pthread_self());

	bst_value_t* p = RETRY;
	do{

		node_t* child = CHILD(node, is_right);
		if (((node->version ^ node_v) & IGNORE_GROW) != 0) {
			return RETRY;
		}

		if (child == NULL){
			p = bst_attempt_insert(k, v, node, is_right, node_v);
		} else{

			if (k == child->key) {
				p = bst_attempt_update(child, v);
			} else{

				bool_t next_is_right = k > child->key ? TRUE : FALSE;
				bst_version_t ch_v = child->version;
				if ((ch_v & SHRINKING) != 0) {
					bst_wait_until_not_changing(child);
				} else if (ch_v != UNLINKED && child == CHILD(node, is_right)){

					if (((node->version ^ node_v) & IGNORE_GROW) != 0) {
						return RETRY;
					}

					p = bst_attempt_put(k, v, child, next_is_right, ch_v);


				}
			}	 
		}


	} while (p == RETRY);
	return p;
}

bst_value_t* bst_attempt_insert(bst_key_t k, bst_value_t* v, node_t* node, bool_t is_right, bst_version_t node_v){
	// printf("[%d] bst_attempt_insert\n", pthread_self());


	LOCK(&(node->lock));
	if (((node->version ^ node_v) & IGNORE_GROW) != 0 || CHILD(node, is_right) != NULL) {
		UNLOCK(&(node->lock));
		return RETRY;
	}	

	node_t* new_child = (node_t*) ssalloc(sizeof(node_t));
	new_child->height = 1;
	new_child->key = k;
	new_child->value = v;
	new_child->parent = node;
	new_child->version = 0;

	// was not in original code
	new_child->left = NULL;
	new_child->right = NULL;
	// end

	INIT_LOCK(&(new_child->lock));

	if(is_right) {
		node->right = new_child;
	} else {
		node->left = new_child;
	}

	UNLOCK(&(node->lock));
	bst_fix_height_and_rebalance(node);
	return NULL;

}

bst_value_t* bst_attempt_update(node_t* node, bst_value_t* v){
	// printf("[%d] bst_attempt_update\n", pthread_self());
	
	LOCK(&(node->lock));
	if (node->version == UNLINKED) {
		UNLOCK(&(node->lock));
		return RETRY;
	}
	bst_value_t* prev = node->value;
	node->value = v;
	UNLOCK(&(node->lock));
	return prev;
	
}

bst_value_t* bst_remove(bst_key_t k, node_t* root){
	// printf("bst_remove\n");

	return bst_attempt_remove(k, root, TRUE, 0);
}

bst_value_t* bst_attempt_remove(bst_key_t k, node_t* node, bool_t is_right, bst_version_t node_v){
	// printf("bst_attempt_remove\n");

	bst_value_t* p = RETRY;
	do{

		node_t* child = CHILD(node, is_right);
		if (((node->version ^ node_v) & IGNORE_GROW) != 0) {
			return RETRY;
		}

		if (child == NULL){
			return NULL;
		} else{

			if (k == child->key) {
				p = bst_attempt_remove_node(node, child);

			} else {
				bool_t next_is_right = k > child->key ? TRUE : FALSE;
				bst_version_t ch_v = child->version;
				if ((ch_v & SHRINKING) != 0) {
					bst_wait_until_not_changing(child);
				} else if (ch_v != UNLINKED && child == CHILD(node, is_right)){

					if (((node->version ^ node_v) & IGNORE_GROW) != 0) {
						return RETRY;
					}

					p = bst_attempt_remove(k, child, next_is_right, ch_v);
				}
			}	 
		}
	} while (p == RETRY);
	return p;
}

bool_t can_unlink(node_t* node){
	// printf("can_unlink\n");

	return node->left == NULL || node->right == NULL;
}

bst_value_t* bst_attempt_remove_node(node_t* par, node_t* n){
	// printf("bst_attempt_remove_node\n");

	if (n->value == NULL) {
		return NULL;
	}

	bst_value_t* prev;
	if (!can_unlink(n)) {
		LOCK(&(n->lock));

		if (n->version == UNLINKED || can_unlink(n)) {
			UNLOCK(&(n->lock));
			return RETRY;
		}

		prev = n->value;
		n->value = NULL;
		
		UNLOCK(&(n->lock));
	} else {
		LOCK(&(par->lock));

		if (par->version == UNLINKED || n->parent != par || n->version == UNLINKED) {
			UNLOCK(&(par->lock));
			return RETRY;
		}
		
		LOCK(&(n->lock));

		prev = n->value;
		n->value = NULL;
		if (can_unlink(n)) {
			node_t* c = (n->left == NULL) ? n->right : n->left;
			if (par->left == n) {
				par->left = c;
			} else {
				par->right = c;
			}

			if (c != NULL) c->parent = par;
			n->version = UNLINKED;
		}

		UNLOCK(&(n->lock));

		UNLOCK(&(par->lock));

		bst_fix_height_and_rebalance(par);
	}
	return prev;
}

void bst_fix_height_and_rebalance(node_t* par){
	// printf("bst_fix_height_and_rebalance\n");
}

void bst_wait_until_not_changing(node_t* n){
	// printf("bst_wait_until_not_changing\n");

	bst_version_t v = n->version;
	if ((v & (GROWING | SHRINKING)) != 0) {
		int i = 0;
		while (n->version == v && i < SPIN_COUNT) {
			i++;
		}
		if (i == SPIN_COUNT) {
			LOCK(&(n->lock));
			UNLOCK(&(n->lock));
		} 
	}
}

unsigned long bst_size(node_t* node) {
	if (node == NULL) {
		return 0;
	} else if (node->value != NULL){
		// fprintf(stderr, "node %p ; left: %p; right: %p\n", node, node->left, node->right);
		return 1 + bst_size(node->right) + bst_size(node->left);
	} else {
		return bst_size(node->right) + bst_size(node->left);
	}
}
