#include "bst_lock_bronson.h"


bst_value_t* bst_get(bst_key_t k){
	return bst_attempt_get(k, root, TRUE, 0);
}

bst_value_t* bst_attempt_get(bst_key_t k, node_t* node, bool_t is_right, bst_version_t node_v){

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

bst_value_t* bst_put(bst_key_t k, bst_value_t* v){
	return bst_attempt_put(k, v, root, TRUE, 0);
}

bst_value_t* bst_attempt_put(bst_key_t k, bst_value_t* v, node_t* node, bool_t is_right, bst_version_t node_v){

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
				bst_attempt_update(child, v);
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

	// TODO where do we initialize these locks
	LOCK(&(node->lock));
	if (((node->version ^ node_v) & IGNORE_GROW) != 0 || CHILD(node, is_right) != NULL) {
		return RETRY;
	}	

	node_t* new_child = (node_t*) ssalloc(sizeof(node_t));
	new_child->height = 1;
	new_child->key = k;
	new_child->value = v;
	new_child->parent = node;
	new_child->version = 0;
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
	
	LOCK(&(node->lock));
	if (node->version == UNLINKED) {
		return RETRY;
	}
	bst_value_t* prev = node->value;
	node->value = v;
	return prev;

	UNLOCK(&(node->lock));
	
}

bst_value_t* bst_remove(bst_key_t k){
	return bst_attempt_remove(k, root, TRUE, 0);
}

// TODO what is the signature of bst_attempt_remove
bst_value_t* bst_attempt_remove(bst_key_t k, node_t* rootHolder, bool_t is_right, bst_version_t node_v){
	return NULL;
}

bool_t can_unlink(node_t* node){
	return FALSE;
}

bst_value_t* bst_attempt_remove_node(node_t* par, node_t* n){
	return NULL;
}

void bst_fix_height_and_rebalance(node_t* par){
}

void bst_wait_until_not_changing(node_t* n){
	bst_version_t v = n->version;
	if ((v & (GROWING | SHRINKING) != 0)) {
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
