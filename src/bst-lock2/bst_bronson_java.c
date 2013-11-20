#include "bst_bronson_java.h"
#include <pthread.h>

// node_t* root;

node_t* bst_initialize() {
	printf("bst_initialize\n");
	node_t* root = (node_t*) ssalloc(sizeof(node_t));

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root
	root->key = 0;
	root->value = FALSE; //wanted to avoid using ssalloc for the dummy value stored in the root placeholder node.
	root->left = NULL;
	root->right = NULL;
	root->height = 0;
	root->version = 0;
	root->parent = NULL;
	INIT_LOCK(&(root->lock));
	
	// should we create an op pointer and flag it with NONE?
	return root;
}

bool_t bst_contains(bst_key_t key, node_t* root) {
	while(TRUE) {
		node_t* right = root->right;

		if (right != NULL) {
			return FALSE;
		} else {
			bst_key_t right_cmp = key - right->key;

			if (right_cmp == 0) {
				return TRUE;
			}

			uint64_t ovl = right->version;
            if(IS_SHRINKING_OR_UNLINKED(ovl)){
                wait_until_not_changing(right);
            } else if(right == root->right){
            	// if right_cmp < 0, we should go left, otherwise right
                result_t vo = attempt_get(key, right, (right_cmp < 0 ? FALSE : TRUE), ovl);
                if (vo != RETRY) {
                    return vo == FOUND;
                }
            }
        }
    }
}

result_t attempt_get(bst_key_t k, node_t* node, bool_t is_right, uint64_t node_v) {

	while(TRUE){
        node_t* child = CHILD(node, is_right);

        if(child == NULL){
            if(node->version != node_v){
                return RETRY;
            }

            return NOT_FOUND;
        } else {
            bst_key_t child_cmp = k - child->key;
            if(child_cmp == 0){
            	//Verify that it's a value node
                return child->value ? FOUND : NOT_FOUND;
            }

            uint64_t child_ovl = child->version;
            if(IS_SHRINKING_OR_UNLINKED(child_ovl)){
                wait_until_not_changing(child);

                if(node->version != node_v){
                    return RETRY;
                }
            } else if(child != CHILD(node, is_right)){
                if(node->version != node_v){
                    return RETRY;
                }
            } else {
                if(node->version != node_v){
                    return RETRY;
                }

                result_t result = attempt_get(k, child, (child_cmp < 0 ? FALSE : TRUE), child_ovl);
                if(result != RETRY){
                    return result;
                }
            }
        }
    }
}


bool_t bst_add(bst_key_t key, node_t* root) {
    printf("bst add\n");
	return update_under_root(key, UPDATE_IF_ABSENT, FALSE, TRUE, root) == NOT_FOUND;
}

bool_t bst_remove(bst_key_t key, node_t* root) {
    printf("bst remove\n");

    return update_under_root(key, UPDATE_IF_PRESENT, TRUE, FALSE, root) == FOUND;
}

result_t update_under_root(bst_key_t key, function_t func, bst_value_t expected, bst_value_t new_value, node_t* holder) {


	while(TRUE){
        printf("while true update under root\n");

        node_t* right = holder->right;

        if(right == NULL){
            if(!SHOULD_UPDATE(func, FALSE)){
                return NO_UPDATE_RESULT(func);
            }

            if(!new_value || attempt_insert_into_empty(key, new_value, holder)){
                return UPDATE_RESULT(func);
            }
        } else {
            uint64_t ovl = right->version;

            if(IS_SHRINKING_OR_UNLINKED(ovl)){
                wait_until_not_changing(right);
            } else if(right == holder->right){
                result_t vo = attempt_update(key, func, expected, new_value, holder, right, ovl);
                if(vo != RETRY){
                    return vo;   
                }
            }
        }
    }
}

bool_t attempt_insert_into_empty(bst_key_t key, bst_value_t value, node_t* holder){

    printf("attempt_insert_into_empty\n");
    LOCK(&(holder->lock));


    if(!holder->right){
        holder->right = new_node(1, key, 0, value, holder, NULL, NULL);
        holder->height = 2;

        UNLOCK(&(holder->lock));
        return TRUE;
    } else {

    	UNLOCK(&(holder->lock));
        return FALSE;
    }
}


node_t* new_node(int height, bst_key_t key, uint64_t version, bst_value_t value, node_t* parent, node_t* left, node_t* right) {

	node_t* node = (node_t*) ssalloc(sizeof(node_t));

	node->height = height;
    node->key = key;
    node->version = version;
    node->value = value;
    node->parent = parent;
    node->left = left;
    node->right = right;
    INIT_LOCK(&(node->lock));

    return node;
}

result_t attempt_update(bst_key_t key, function_t func, bst_value_t expected, bst_value_t new_value, node_t* parent, node_t* node, uint64_t node_v) {

    printf("attempt_update\n");

	bst_key_t cmp = key - node->key;
    if(cmp == 0){
        return attempt_node_update(func, expected, new_value, parent, node);
    }

    while(TRUE){
        printf("while true attempt_update\n");

        node_t* child = CHILD(node, (cmp < 0 ? FALSE : TRUE));

        if(node->version != node_v){
            return RETRY;
        }

        if(child == NULL){

            if(!new_value){
                return NOT_FOUND;
            } else {
                bool_t success;
                node_t* damaged;

                {
                    // publish(node);
                    LOCK(&(node->lock)); 
                
                    if(node->version != node_v){
                        // releaseAll();
                        UNLOCK(&(node->lock));
                        return RETRY;
                    }

                    if(CHILD(node, (cmp < 0 ? FALSE : TRUE)) != NULL){
                        success = FALSE;
                        damaged = NULL;
                    } else {
                        if(!SHOULD_UPDATE(func, FALSE)){
                            // releaseAll();
                            UNLOCK(&(node->lock));
                            return NO_UPDATE_RESULT(func);
                        }

                        node_t* new_child = new_node(1, key, 0, TRUE, node, NULL, NULL);
                        set_child(node, new_child, cmp);

                        success = TRUE;
                        damaged = fix_height_nl(node);
                    }
                    
                    // releaseAll();
                    UNLOCK(&(node->lock));
                }

                if(success){
                    fix_height_and_rebalance(damaged);
                    return UPDATE_RESULT(func);
                }
            }

        } else {
            uint64_t child_v = child->version;

            if(IS_SHRINKING_OR_UNLINKED(child_v)){
                wait_until_not_changing(child);
            } else if(child != CHILD(node, (cmp < 0 ? FALSE : TRUE))){
                //RETRY
            } else {
                if(node->version != node_v){
                    return RETRY;
                }

                result_t vo = attempt_update(key, func, expected, new_value, node, child, child_v);
                if(vo != RETRY){
                    return vo;
                }
            }
        }
    }
}

result_t attempt_node_update(function_t func, bst_value_t expected, bst_value_t new_value, node_t* parent, node_t* node) {

	if(!new_value){
        if(!node->value){
            return NOT_FOUND;
        }
    }

    if(!new_value && (!node->left || !node->right)){
        
        bst_value_t prev;
        node_t* damaged;

        {
            // publish(parent);
            // scoped_lock parentLock(parent->lock);
            LOCK(&(parent->lock));
            
            if(IS_UNLINKED(parent->version) || node->parent != parent){
                // releaseAll();
                UNLOCK(&(parent->lock));
                return RETRY;
            }

            {
                // publish(node);
                // scoped_lock lock(node->lock);
                LOCK(&(node->lock));
                
                prev = node->value;

                if(!SHOULD_UPDATE(func, prev)){
                    // releaseAll();
                    UNLOCK(&(node->lock));
                    UNLOCK(&(parent->lock));
                    return NO_UPDATE_RESULT(func);
                }

                if(!prev){
                    // releaseAll();
                    UNLOCK(&(node->lock));
                    UNLOCK(&(parent->lock));
                    return UPDATE_RESULT(func);
                }

                if(!attempt_unlink_nl(parent, node)){
                    // releaseAll();
                    UNLOCK(&(node->lock));
                    UNLOCK(&(parent->lock));
                    return RETRY;
                }
            }
            
            // releaseAll();
            UNLOCK(&(node->lock));
            UNLOCK(&(parent->lock));
            damaged = fix_height_nl(parent);
        }

        fix_height_and_rebalance(damaged);

        return UPDATE_RESULT(func);
    } else {
        // publish(node);
        // scoped_lock lock(node->lock);
        LOCK(&(node->lock));

        if(IS_UNLINKED(node->version)){
            // releaseAll();
            UNLOCK(&(node->lock));
            return RETRY;
        }

        bst_value_t prev = node->value;
        if(!SHOULD_UPDATE(func, prev)){
			// releaseAll();
			UNLOCK(&(node->lock));
            return NO_UPDATE_RESULT(func);
        }

        if(!new_value && (node->left == NULL || node->right == NULL)){
            // releaseAll();
            UNLOCK(&(node->lock));
            return RETRY;
        }

        node->value = new_value;
        
        // releaseAll();
        UNLOCK(&(node->lock));
        return UPDATE_RESULT(func);
    }
}

void wait_until_not_changing(node_t* node) {
	uint64_t version = node->version;
	int i;

	if (IS_SHRINKING(version)) {
		for (i = 0; i < 100; ++i) {
			if (version != node->version) {
				return;
			}
		}

		LOCK(&(node->lock));
		UNLOCK(&(node->lock));
	}
}

bool_t attempt_unlink_nl(node_t* parent, node_t* node) {

	node_t* parent_l = parent->left;
    node_t* parent_r = parent->right;

    if(parent_l != node && parent_r != node){
        return FALSE;
    }

    node_t* left = node->left;
    node_t* right = node->right;

    if(left != NULL && right != NULL){
        return FALSE;
    }

    node_t* splice = left != NULL ? left : right;
    
    if(parent_l == node){
        parent->left = splice;
    } else {
        parent->right = splice;
    }

    if(splice != NULL){
        splice->parent = parent;
    }

    node->version = UNLINKED_OVL;
    node->value = FALSE;

    // hazard.releaseNode(node);

    return TRUE;
}

int node_conditon(node_t* node) {

    printf("node_conditon\n");
	node_t* nl = node->left;
    node_t* nr = node->right;

    // unlink is required
    if((nl == NULL || nr == NULL) && !node->value){
        
        printf("return 3 node_conditon\n");

        return UNLINK_REQUIRED;
    }

    int hn = node->height;
    int hl0 = HEIGHT(nl);
    int hr0 = HEIGHT(nr);
    int hnrepl = 1 + max(hl0, hr0);
    int bal = hl0 - hr0;

    // rebalance is required ?
    if(bal < -1 || bal > 1){
        printf("return 2 node_conditon\n");

        return REBALANCE_REQUIRED;
    }

    printf("return 1 node_conditon\n");

    return hn != hnrepl ? hnrepl : NOTHING_REQUIRED;
}

void fix_height_and_rebalance(node_t* node) {

    printf("fix_height_and_rebalance\n");
	while(node != NULL && node->parent != NULL){
        int condition = node_conditon(node);
        if(condition == NOTHING_REQUIRED || IS_UNLINKED(node->version)){
            printf("return\n");
            return;
        }

        if(condition != UNLINK_REQUIRED && condition != REBALANCE_REQUIRED){
            // publish(node);
            // scoped_lock lock(node->lock);
            printf("before mistery lock\n");
            LOCK(&(node->lock));
            
            node = fix_height_nl(node);

            UNLOCK(&(node->lock));
            // releaseAll();
        } else {
            node_t* n_parent = node->parent;
            // publish(n_parent);
            // scoped_lock lock(n_parent->lock);
            LOCK(&(n_parent->lock));

            if(!IS_UNLINKED(n_parent->version) && node->parent == n_parent){
                // publish(node);
                // scoped_lock nodeLock(node->lock);
                LOCK(&(node->lock));

                node = rebalance_nl(n_parent, node);
                UNLOCK(&(node->lock));
            }


            UNLOCK(&(n_parent->lock));
            
            // releaseAll();
        }
    }
}

node_t* fix_height_nl(node_t* node){

    printf("fix_height_nl\n");
	int c = node_conditon(node);

    switch(c){
        case REBALANCE_REQUIRED:
        case UNLINK_REQUIRED:
            return node;
        case NOTHING_REQUIRED:
            return NULL;
        default:
            node->height = c;
            return node->parent;
    }
}

node_t* rebalance_nl(node_t* n_parent, node_t* n){
    printf("rebalance_nl\n");

	node_t* nl = n->left;
    node_t* nr = n->right;

    if((nl == NULL || nr == NULL) && !n->value){
        if(attempt_unlink_nl(n_parent, n)){
            return fix_height_nl(n_parent);
        } else {
            return n;
        }
    }
    
    int hn = n->height;
    int hl0 = HEIGHT(nl);
    int hr0 = HEIGHT(nr);
    int hnrepl = 1 + max(hl0, hr0);
    int bal = hl0 - hr0;

    if(bal > 1){
        return rebalance_to_right_nl(n_parent, n, nl, hr0);
    } else if(bal < -1){
        return rebalance_to_left_nl(n_parent, n, nr, hl0);
    } else if(hnrepl != hn) {
        n->height = hnrepl;

        return fix_height_nl(n_parent);
    } else {
        return NULL;
    }
}

node_t* rebalance_to_right_nl(node_t* n_parent, node_t* n, node_t* nl, int hr0) {

    printf("rebalance_to_right_nl\n");

	LOCK(&(nl->lock));

	int hl = nl->height;
    if(hl - hr0 <= 1){
    	UNLOCK(&(nl->lock));
        return n;
    } else {
        // publish(nl->right);
        node_t* nlr = nl->right;

        int hll0 = HEIGHT(nl->left);
        int hlr0 = HEIGHT(nlr);

        if(hll0 > hlr0){
        	node_t* res = rotate_right_nl(n_parent, n, nl, hr0, hll0, nlr, hlr0);
        	UNLOCK(&(nl->lock));
            return res;
        } else {
            {
                // if(reinterpret_cast<long>(&nlr->lock) == 0x30){
                //     return n;
                // }
                // scoped_lock sublock(nlr->lock);
                LOCK(&(nlr->lock));

                int hlr = nlr->height;
                if(hll0 >= hlr){
                	node_t* res = rotate_right_nl(n_parent, n, nl, hr0, hll0, nlr, hlr);
                	
                    UNLOCK(&(nlr->lock));
                    UNLOCK(&(nl->lock));
                	
                    return res;
                } else {
                    int hlrl = HEIGHT(nlr->left);
                    int b = hll0 - hlrl;
                    if(b >= -1 && b <= 1){
                    	node_t* res = rotate_right_over_left_nl(n_parent, n, nl, hr0, hll0, nlr, hlrl);
                    	
                        UNLOCK(&(nlr->lock));
                        UNLOCK(&(nl->lock));
                        return res;
                    }
                }
            }

            node_t* res = rebalance_to_left_nl(n, nl, nlr, hll0);
            UNLOCK(&(nl->lock));
            return res;
        }
    }
}

node_t* rebalance_to_left_nl(node_t* n_parent, node_t* n, node_t* nr, int hl0) {

    printf("rebalance_to_left_nl\n");

	// publish(nR);
 //    scoped_lock lock(nR->lock);
	LOCK(&(nr->lock));

    int hr = nr->height;
    if(hl0 - hr >= -1){
    	UNLOCK(&(nr->lock));
        return n;
    } else {
        node_t* nrl = nr->left;
        int hrl0 = HEIGHT(nrl);
        int hrr0 = HEIGHT(nr->right);

        if(hrr0 >= hrl0){
            return rotate_left_nl(n_parent, n, hl0, nr, nrl, hrl0, hrr0);
        } else {
            {
                // publish(nrl);
                // scoped_lock sublock(nrl->lock);
                LOCK(&(nrl->lock));

                int hrl = nrl->height;
                if(hrr0 >= hrl){
                	node_t* res = rotate_left_nl(n_parent, n, hl0, nr, nrl, hrl, hrr0);
                	UNLOCK(&(nrl->lock));
                	UNLOCK(&(nr->lock));
                    return res;
                } else {
                    int hrlr = HEIGHT(nrl->right);
                    int b = hrr0 - hrlr;
                    if(b >= -1 && b <= 1){
                    	node_t* res = rotate_left_over_right_nl(n_parent, n, hl0, nr, nrl, hrr0, hrlr);
                        UNLOCK(&(nrl->lock));
                		UNLOCK(&(nr->lock));
                        return res;
                    }
                }
            }
			//UNLOCK(&(nrl->lock));
            node_t* res = rebalance_to_right_nl(n, nr, nrl, hrr0);
            UNLOCK(&(nr->lock));
            return res;
        }
    }
}

node_t* rotate_right_nl(node_t* n_parent, node_t* n, node_t* nl, int hr, int hll, node_t* nlr, int hlr) {

	uint64_t node_ovl = n->version;
    node_t* npl = n_parent->left;
    n->version = BEGIN_CHANGE(node_ovl);

    n->left = nlr;
    if(nlr != NULL){
        nlr->parent = n;
    }

    nl->right = n;
    n->parent = nl;

    if(npl == n){
        n_parent->left = nl;
    } else {
        n_parent->right = nl;
    }
    nl->parent = n_parent;

    int hnrepl = 1 + max(hlr, hr);
    n->height = hnrepl;
    nl->height = 1 + max(hll, hnrepl);

    n->version = END_CHANGE(node_ovl);

    int baln = hlr - hr;
    if(baln < -1 || baln > 1){
        return n;
    }

    if ((nlr == NULL || hr == 0) && !n->value) {
            // we need to remove n and then repair
            return n;
    }

    int ball = hll - hnrepl;
    if(ball < -1 || ball > 1){
        return nl;
    }

    if (hll == 0 && !nl->value) {
            return nl;
    }

    return fix_height_nl(n_parent);
}

node_t* rotate_left_nl(node_t* n_parent, node_t* n, int hl, node_t* nr, node_t* nrl, int hrl, int hrr){

    uint64_t node_ovl = n->version;
    node_t* npl = n_parent->left;
    n->version = BEGIN_CHANGE(node_ovl);

    n->right = nrl;
    if(nrl != NULL){
        nrl->parent = n;
    }

    nr->left = n;
    n->parent = nr;

    if(npl == n){
        n_parent->left = nr;
    } else {
        n_parent->right = nr;
    }
    nr->parent = n_parent;

    int hnrepl = 1 + max(hl, hrl);
    n->height = hnrepl;
    nr->height = 1 + max(hnrepl, hrr);

    n->version = END_CHANGE(node_ovl);

    int baln = hrl - hl;
    if(baln < -1 || baln > 1){
        return n;
    }

    if ((nrl == NULL || hl == 0) && !n->value) {
            return n;
    }

    int balr = hrr - hnrepl;
    if(balr < -1 || balr > 1){
        return nr;
    }

    if (hrr == 0 && !nr->value) {
        return nr;
    }

    return fix_height_nl(n_parent);
}

node_t* rotate_right_over_left_nl(node_t* n_parent, node_t* n, node_t* nl, int hr, int hll, node_t* nlr, int hlrl){

    uint64_t node_ovl = n->version;
    uint64_t left_ovl = nl->version;

    node_t* npl = n_parent->left;
    node_t* nlrl = nlr->left;
    node_t* nlrr = nlr->right;
    int hlrr = HEIGHT(nlrr);

    n->version = BEGIN_CHANGE(node_ovl);
    nl->version = BEGIN_CHANGE(left_ovl);

    n->left = nlrr;
    if(nlrr != NULL){
        nlrr->parent = n;
    }

    nl->right = nlrl;
    if(nlrl != NULL){
        nlrl->parent = nl;
    }

    nlr->left = nl;
    nl->parent = nlr;
    nlr->right = n;
    n->parent = nlr;

    if(npl == n){
        n_parent->left = nlr;
    } else {
        n_parent->right = nlr;
    }
    nlr->parent = n_parent;

    int hnrepl = 1 + max(hlrr, hr);
    n->height = hnrepl;

    int hlrepl = 1 + max(hll, hlrl);
    nl->height = hlrepl;

    nlr->height = 1 + max(hlrepl, hnrepl);

    n->version = END_CHANGE(node_ovl);
    nl->version = END_CHANGE(left_ovl);

    int baln = hlrr - hr;
    if(baln < -1 || baln > 1){
        return n;
    }

    if ((nlrr == NULL || hr == 0) && !n->value) {
        // repair involves splicing out n and maybe more rotations
        return n;
    }

    int ballr = hlrepl - hnrepl;
    if(ballr < -1 || ballr > 1){
        return nlr;
    }
    
    return fix_height_nl(n_parent);
}

node_t* rotate_left_over_right_nl(node_t* n_parent, node_t* n, int hl, node_t* nr, node_t* nrl, int hrr, int hrlr){

    uint64_t node_ovl = n->version;
    uint64_t right_ovl = nr->version;

    node_t* npl = n_parent->left;
    node_t* nrll = nrl->left;
    node_t* nrlr = nrl->right;
    int hrll = HEIGHT(nrll);

    n->version = BEGIN_CHANGE(node_ovl);
    nr->version = BEGIN_CHANGE(right_ovl);

    n->right = nrll;
    if(nrll){
        nrll->parent = n;
    }

    nr->left = nrlr;
    if(nrlr){
        nrlr->parent = nr;
    }

    nrl->right = nr;
    nr->parent = nrl;
    nrl->left = n;
    n->parent = nrl;

    if(npl == n){
        n_parent->left = nrl;
    } else {
        n_parent->right = nrl;
    }
    nrl->parent = n_parent;

    int hnrepl = 1 + max(hl, hrll);
    n->height = hnrepl;
    int hrrepl = 1 + max(hrlr, hrr);
    nr->height = hrrepl;
    nrl->height = 1 + max(hnrepl, hrrepl);

    n->version = END_CHANGE(node_ovl);
    nr->version = END_CHANGE(right_ovl);

    int baln = hrll - hl;
    if(baln < -1 || baln > 1){
        return n;
    }

    if ((nrll == NULL || hl == 0) && !n->value) {
        return n;
    }

    int balrl = hrrepl - hnrepl;
    if(balrl < -1 || balrl > 1){
        return nrl;
    }
    
    return fix_height_nl(n_parent);
}

void set_child(node_t* parent, node_t* child, bool_t is_right) {
	if (is_right) {
		parent->right = child;
	} else {
		parent->left = child;
	}
}

uint64_t bst_size(node_t* node) {
	if (node == NULL || node->version == UNLINKED_OVL) {
		return 0;
	} else if (node->value) {
		return 1 + bst_size(node->left) + bst_size(node->right);
	} else {
		return bst_size(node->left) + bst_size(node->right);
	}
}


