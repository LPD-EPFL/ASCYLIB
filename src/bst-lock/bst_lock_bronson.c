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
	// bst_fix_height_and_rebalance(node);
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

		// bst_fix_height_and_rebalance(par);
	}
	return prev;
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

// Functions for fixing the height


// void bst_fix_height_and_rebalance(node_t* node){
// 	// printf("bst_fix_height_and_rebalance\n");
// 	while(node != NULL && node->parent != NULL){
// 		uint8_t condition = node_condition(node);
// 		if (condition == NOTHING_REQUIRED || node->version == UNLINKED){

// 			return;
// 		}

// 		if (condition == UNLINK_REQUIRED && condition != REBALANCE_REQUIRED) {

// 			LOCK(&(node->lock));
// 			node = fix_height_nl(node);
// 			UNLOCK(&(node->lock));
// 		} else {
// 			node_t* n_parent = node->parent; 
// 			LOCK(&(n_parent->lock));
// 			if ( n_parent->version != UNLINKED && node->parent == n_parent){
// 				LOCK(&(node->lock));
// 				node = rebalance_nl(n_parent, node);
// 				UNLOCK(&(node->lock));
// 			}
// 			UNLOCK(&(n_parent->lock));	
// 		}
// 	}
// }

// node_t* fix_height_nl(node_t* node){
// 	uint8_t c = node_condition(node);
// 	switch (c) {
//         case REBALANCE_REQUIRED:
//         case UNLINK_REQUIRED:
//             // can't repair
//             return node;
//         case NOTHING_REQUIRED:
//             // Any future damage to this node is not our responsibility.
//             return NULL;
//         default:
//             node->height = c;
//             // we've damaged our parent, but we can't fix it now
//             return node->parent;
//     }
// }

// node_t* rebalance_nl(node_t* n_parent, node_t* n){
// 	node_t* nl = unshared_left(n);
// 	node_t* nr = unshared_right(n);

// 	if((nl == NULL || nr == NULL) && n->value == NULL){
// 		if (attempt_unlink_nl(n_parent, n)){
// 			return fix_height_nl(n_parent);
// 		} else {
// 			return n;
// 		}
// 	}

// 	bst_height_t hn = n->height;
// 	bst_height_t hl0 = height(nl);
// 	bst_height_t hr0 = height(nr);
// 	bst_height_t hNRepl = 1 + max(hl0, hr0);
//     bst_height_t bal = hl0 - hr0;

//     if(bal > 1){
//     	return rebalance_to_right_nl(n_parent, n, nl, hr0);
//     } else if (bal < 1){
//     	return rebalance_to_left_nl(n_parent, n, nr, hl0);
//     } else if (hNRepl != hn){
//     	n->height = hNRepl;
//     	return fix_height_nl(n_parent);
//     } else {
//     	return NULL;
//     }

// }

// node_t* rebalance_to_right_nl(node_t* n_parent, node_t* n, node_t* nl, bst_height_t hr0){

// 	LOCK(&(nl->lock));
// 	bst_height_t hl = nl->height;
// 	if (hl - hr0 <= 1){
// 		UNLOCK(&(nl->lock));
// 		return n;
// 	} else {
// 		node_t* nlr = unshared_right(nl);
// 		bst_height_t hll0 = height(nl->left);
// 		bst_height_t hlr0 = height(nlr);
//         if (hll0 >= hlr0) {
//             // rotate right based on our snapshot of hLR
//             node_t* res = rotate_right_nl(n_parent, n, nl, hr0, hll0, nlr, hlr0);
//             UNLOCK(&(nl->lock));
//             return res;
//         } else {
//         	LOCK(&(nlr->lock));

//         	bst_height_t hlr = nlr->height;
//             if (hll0 >= hlr) {
//             	node_t* res = rotate_right_nl(n_parent, n, nl, hr0, hll0, nlr, hlr);
//             	UNLOCK(&(nlr->lock));
//             	UNLOCK(&(nl->lock));
//                 return res;
//             } else {
// 	            bst_height_t hlrl = height(nLR.left);
// 	            bst_height_t b = hll0 - hlrl;
// 	            if (b >= -1 && b <= 1 && !((hll0 == 0 || hlrl == 0) && nl->value == NULL)) {
// 	                // nParent.child.left won't be damaged after a double rotation
// 	                node_t* res = rotateRightOverLeft_nl(n_parent, n, nl, hr0, hll0, nlr, hlrl);
//                 	UNLOCK(&(nlr->lock));
//             		UNLOCK(&(nl->lock));
// 	                return res; 
// 				}
//         	}

//         	node_t* res = rebalance_to_left_nl(n, nl, nlr, nll0);
//         	UNLOCK(&(nl->lock));
//         	return res;
// 		}
// 	}
// 	UNLOCK(&(nl->lock));
// }

// node_t* rebalance_to_left_nl(node_t* n_parent, node_t* n, node_t* nr,bst_height_t hl0) {

// 	LOCK(&(nr->lock));
// 	bst_height_t hr = nr->height;
// 	if (hl0 - hr >= -1) {
// 		UNLOCK(&(nr->lock));
// 		return n;
// 	} else {
// 		node_t* nrl = unshared_left(nr);
// 		bst_height_t hrl0 = height(nrl);
// 		bst_height_t hrr0 = height(nr->right);
// 		if (hrr0 >= hrl0) {
// 			node_t* res = rotate_left_nl(n_parent, n, hl0, nr, nrl, hrl0, hrr0);
// 			UNLOCK(&(nr->lock));
// 			return res;
// 		} else {
// 			LOCK(&(nrl->lock));
// 			bst_height_t hrl = nrl->height;
// 			if (hrr0 >= hrl) {
// 				node_t* res = rotate_left_nl(n_parent, n, hl0, nr, nrl, hrl, hrr0);
// 				UNLOCK(&(nrl->lock));
// 				UNLOCK(&(nr->lock));
// 				return res;
// 			} else {
//                 bst_height_t hrlr = height(nrl->right);
//                 bst_height_t b = hrr0 - hrlr;
//                 if (b >= -1 && b <= 1 && !((hrr0 == 0 || hrlr == 0) && nr->value == NULL)) {
//                 	node_t* res = rotate_left_over_right_nl(n_parent, n, hl0, nr, nrl, hrr0, hrlr);
// 					UNLOCK(&(nrl->lock));
// 					UNLOCK(&(nr->lock));
//                     return res;
//                 }
// 			}
// 			UNLOCK(&(nrl->lock));
// 			node_t* res = rebalance_to_right_nl(n, nr, nrl, hrr0);
// 			UNLOCK(&(nr->lock));
// 			return res;
// 		}
// 	}
// 	UNLOCK(&(nr->lock));
// }

// node_t* rotate_right_nl(node_t* n_parent, node_t* n, node_t* nl, bst_height_t hr, bst_height_t hll, bst_height_t nlr, bst_height_t hlr) {

// 	bst_version_t node_ovl = n->version;
// 	node_t* npl = n_parent->left;
// 	n->version = begin_change(node_ovl);
// 	n->left = nlr;
// 	if (nlr != NULL) {
// 		nlr->parent = n;
// 	}

// 	nl->right = n;
// 	n->parent = nl;

// 	if (npl == n) {
// 		n_parent->left = nl;
// 	} else {
// 		n_parent->right = nl;
// 	}
// 	nl->parent = n_parent;

// 	bst_height_t hn_repl = 1 + max(hlr, hr);
// 	n->height = hn_repl;
// 	nl->height = 1 + max(hll, hn_repl);

// 	n->version = end_change(node_ovl);

// 	bst_height_t bal_n = hlr - hr;
// 	if (bal_n < -1 || bal_n > 1) {
// 		return n;
// 	}

// 	if ((nlr == NULL || hr == 0) && n->value == NULL) {
// 		return n;
// 	}

// 	bst_height_t bal_l = hll - hn_repl;
// 	if (bal_l < -1 || bal_l > 1) {
// 		return nl;
// 	}

// 	if (hll == 0 && nl->value == NULL) {
// 		return nl;
// 	}

// 	return fix_height_nl(n_parent);
// }

// node_t* rotate_left_nl(node_t* n_parent, node_t* n, bst_height_t hl, node_t* nr, node_t* nrl, bst_height_t hrl, bst_height_t hrr) {

//     bst_version_t node_ovl = n->version;

//     node_t* npl = n_parent->left;

//     n->version = begin_change(node_ovl);

//     // fix up n links, careful to be compatible with concurrent traversal for all but n
//     n->right = nrl;
//     if (nrl != NULL) {
//         nrl->parent = n;
//     }

//     nr->left = n;
//     n->parent = nr;

//     if (npl == n) {
//         n_parent->left = nr;
//     } else {
//         n_parent->right = nr;
//     }
//     nr->parent = n_parent;

//     // fix up heights
//     bst_height_t hn_repl = 1 + max(hl, hrl);
//     n->height = hn_repl;
//     n->height = 1 + max(hn_repl, hrr);

//     n->version = endChange(node_ovl);

//     bst_height_t bal_n = hrl - hl;
//     if (bal_n < -1 || bal_n > 1) {
//         return n;
//     }

//     if ((nrl == NULL || hl == 0) && n->value == NULL) {
//         return n;
//     }

//     bst_height_t bal_r = hrr - hn_repl;
//     if (bal_r < -1 || bal_r > 1) {
//         return nr;
//     }

//     if (hrr == 0 && nr->value == NULL) {
//         return nr;
//     }

//     return fix_height_nl(n_parent);
// }

// TODO:
// begin_change(node_ovl);
// end_change(node_ovl);
// node_condition(node_t*);
// unshared_left(n);
// unshared_right(n);
// attempt_unlink_nl(n_parent, n);

// (static inline) height(nl);