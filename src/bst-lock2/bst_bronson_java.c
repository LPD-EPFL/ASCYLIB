#include "bst_bronson_java.h"
#include <pthread.h>

// node_t* root;

node_t* bst_initialize() {
	//printf("bst_initialize\n");
	node_t* root = (node_t*) ssalloc(sizeof(node_t));

	// assign minimum key to the root, actual tree will be 
	// the right subtree of the root
	root->key = 0;
	root->value = FALSE; 
	root->left = NULL;
	root->right = NULL;
	root->height = 0;
	root->version = 0;
	root->parent = NULL;
	INIT_LOCK(&(root->lock));
	
	// should we create an op pointer and flag it with NONE?
	return root;
}

// checked
bool_t bst_contains(bst_key_t key, node_t* root) {
	while(TRUE) {
		node_t* right = root->right;

		if (right == NULL) {
			return FALSE;
		} else {
			int right_cmp = key - right->key;
            // printf("cmp %d\n", right_cmp);
			if (right_cmp == 0) {
				return right->value;
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

// checked
result_t attempt_get(bst_key_t k, node_t* node, bool_t is_right, uint64_t node_v) {

	while(TRUE){
        node_t* child = CHILD(node, is_right);

        if(child == NULL){
            if(node->version != node_v){
                return RETRY;
            }

            return NOT_FOUND;
        } else {
            int child_cmp = k - child->key;
            // printf("cmp %d\n", child_cmp);
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

// checked
bool_t bst_add(bst_key_t key, node_t* root) {
    // printf("bst add %d\n", key);
	return update_under_root(key, UPDATE_IF_ABSENT, FALSE, TRUE, root) == NOT_FOUND;
}

// checked
bool_t bst_remove(bst_key_t key, node_t* root) {
    // printf("bst remove %d\n", key);
    return update_under_root(key, UPDATE_IF_PRESENT, TRUE, FALSE, root) == FOUND;
}

// checked
result_t update_under_root(bst_key_t key, function_t func, bst_value_t expected, bst_value_t new_value, node_t* holder) {

	while(TRUE){
       // printf("while true update under root\n");

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

// checked
bool_t attempt_insert_into_empty(bst_key_t key, bst_value_t value, node_t* holder){

    bst_key_t holder_key = holder->key;

    //printf("attempt_insert_into_empty\n");
    //printf("Lock node: %d\n", holder_key);
    ptlock_t* holder_lock = &holder->lock;
    LOCK(holder_lock);


    if(holder->right == NULL){
        holder->right = new_node(1, key, 0, value, holder, NULL, NULL);
        holder->height = 2;

        //printf("Unlock node: %d\n", holder_key);
        UNLOCK(holder_lock);
        return TRUE;
    } else {
        //printf("Unlock node: %d\n", holder_key);
    	UNLOCK(holder_lock);
        return FALSE;
    }
}

// checked
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

// checked
result_t attempt_update(bst_key_t key, function_t func, bst_value_t expected, bst_value_t new_value, node_t* parent, node_t* node, uint64_t node_v) {

    //printf("attempt_update\n");
	int cmp = key - node->key;
    //printf("cmp %d\n", cmp);
    if(cmp == 0){
        result_t res = attempt_node_update(func, expected, new_value, parent, node);
        return res;
    }

    bool_t is_right = cmp < 0 ? FALSE : TRUE ;
    
    while(TRUE){
        //printf("while true attempt_update\n");

        node_t* child = CHILD(node, is_right);

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
                    bst_key_t node_key = node->key;
                    //printf("Lock node: %d\n", node_key);
                    ptlock_t* node_lock = &node->lock;
                    LOCK(node_lock); 
                
                    if(node->version != node_v){
                        // releaseAll();
                        //printf("Unlock node: %d\n", node_key);
                        UNLOCK(node_lock);
                        
                        return RETRY;
                    }

                    if(CHILD(node, is_right) != NULL){
                        success = FALSE;
                        damaged = NULL;
                    } else {
                        if(!SHOULD_UPDATE(func, FALSE)){
                            // releaseAll();
                            //printf("Unlock node: %d\n", node_key);
                            UNLOCK(node_lock);
                            
                            return NO_UPDATE_RESULT(func);
                        }

                        node_t* new_child = new_node(1, key, 0, TRUE, node, NULL, NULL);
                        set_child(node, new_child, is_right);

                        success = TRUE;
                        damaged = fix_height_nl(node);
                    }
                    
                    // releaseAll();
                    //printf("Unlock node: %d\n", node_key);
                    UNLOCK(node_lock);
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
            } else if(child != CHILD(node, is_right)){
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

// checked
result_t attempt_node_update(function_t func, bst_value_t expected, bst_value_t new_value, node_t* parent, node_t* node) {


	if(!new_value){
        if(!node->value){
            
            return NOT_FOUND;
        }
    }

    if(!new_value && (node->left == NULL || node->right == NULL)){
        
        bst_value_t prev;
        node_t* damaged;

        {
            // publish(parent);
            // scoped_lock parentLock(parent->lock);
            bst_key_t parent_key = parent->key;
            //printf("Lock node: %d\n", parent->key);
            ptlock_t* parent_lock = &parent->lock;
            LOCK(parent_lock);
            
            if(IS_UNLINKED(parent->version) || node->parent != parent){
                // releaseAll();
                //printf("Unlock node: %d\n", parent_key);
                UNLOCK(parent_lock);
                return RETRY;
            }

            {
                // publish(node);
                // scoped_lock lock(node->lock);
                bst_key_t node_key = node->key;
                //printf("Lock node: %d\n", node_key);
                ptlock_t* node_lock = &node->lock;
                LOCK(node_lock);
                
                prev = node->value;

                if(!SHOULD_UPDATE(func, prev)){
                    // releaseAll();
                    //printf("Unlock node: %d\n", node_key);
                    UNLOCK(node_lock);
                    //printf("Unlock node: %d\n", parent_key);
                    UNLOCK(parent_lock);
                    return NO_UPDATE_RESULT(func);
                }

                if(!prev){
                    // releaseAll();
                    //printf("Unlock node: %d\n", node_key);
                    UNLOCK(node_lock);
                    //printf("Unlock node: %d\n", parent_key);
                    UNLOCK(parent_lock);
                    return UPDATE_RESULT(func);
                }

                //printf("before unlink\n");

                if(!attempt_unlink_nl(parent, node)){
                    // releaseAll();
                    //printf("Unlock node: %d\n", node_key);
                    UNLOCK(node_lock);
                    //printf("Unlock node: %d\n", parent_key);
                    UNLOCK(parent_lock);
                    return RETRY;
                }

                //printf("Unlock node: %d\n", node_key);
                // here the node has been unlinked, so why are we unlocking it??
                UNLOCK(node_lock);
            }
            
            // releaseAll();
            //printf("Unlock node: %d\n", parent_key);
            //printf(stderr, "after unlock parent_lock\n");
            damaged = fix_height_nl(parent);
            UNLOCK(parent_lock);
        }

        fix_height_and_rebalance(damaged);
        
        return UPDATE_RESULT(func);
    } else {
        // publish(node);
        // scoped_lock lock(node->lock);
        bst_key_t node_key = node->key;
        //printf("Lock node: %d\n", node_key);
        ptlock_t* node_lock = &node->lock;
        LOCK(node_lock);

        if(IS_UNLINKED(node->version)){
            // releaseAll();
            //printf("Unlock node: %d\n", node_key);
            UNLOCK(node_lock);
            return RETRY;
        }

        bst_value_t prev = node->value;
        if(!SHOULD_UPDATE(func, prev)){
			// releaseAll();
            //printf("Unlock node: %d\n", node_key);
			UNLOCK(node_lock);
            return NO_UPDATE_RESULT(func);
        }

        if(!new_value && (node->left == NULL || node->right == NULL)){
            // releaseAll();
            //printf("Unlock node: %d\n", node_key);
            UNLOCK(node_lock);
            return RETRY;
        }

        node->value = new_value;
        
        // releaseAll();
        //printf("Unlock node: %d\n", node_key);
        UNLOCK(node_lock);
        return UPDATE_RESULT(func);
    }
}

// checked (oana still has doubts?)
void wait_until_not_changing(node_t* node) {
    // printf("wait_until_not_changing\n");
	uint64_t version = node->version;
	int i;

	if (IS_SHRINKING(version)) {
		for (i = 0; i < SPIN_COUNT; ++i) {
			if (version != node->version) {
				return;
			}
		}

        bst_key_t node_key = node->key;
        //printf("Lock node: %d\n", node_key);
		ptlock_t* node_lock = &node->lock;
		LOCK(node_lock);
        //printf("Unlock node: %d\n", node_key);
		UNLOCK(node_lock);
	}
}

// checked
bool_t attempt_unlink_nl(node_t* parent, node_t* node) {

    // printf("attempt_unlink_nl\n");
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

    node_t* splice = (left != NULL) ? left : right;
    
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

// checked
int node_conditon(node_t* node) {

    // printf("node_conditon\n");
	node_t* nl = node->left;
    node_t* nr = node->right;

    // unlink is required
    if((nl == NULL || nr == NULL) && !node->value){
        
        // printf("UNLINK_REQUIRED\n");

        return UNLINK_REQUIRED;
    }

    int hn = node->height;
    int hl0 = HEIGHT(nl);
    int hr0 = HEIGHT(nr);
    int hnrepl = 1 + max(hl0, hr0);
    int bal = hl0 - hr0;

    // rebalance is required ?
    if(bal < -1 || bal > 1){
        // printf("REBALANCE_REQUIRED\n");

        return REBALANCE_REQUIRED;
    }

    // printf("hnrepl : NOTHING_REQUIRED\n");

    return hn != hnrepl ? hnrepl : NOTHING_REQUIRED;
}

// checked
node_t* fix_height_nl(node_t* node){

    //printf("fix_height_nl node key %d\n", node->key);
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

// checked
void fix_height_and_rebalance(node_t* node) {
    
    return;
    // printf("fix_height_and_rebalance\n");
    while(node != NULL && node->parent != NULL){
        
        
        int condition = node_conditon(node);
        if(condition == NOTHING_REQUIRED || IS_UNLINKED(node->version)){
            return;
        }

        if(condition != UNLINK_REQUIRED && condition != REBALANCE_REQUIRED){
            // publish(node);
            // scoped_lock lock(node->lock);

            bst_key_t node_key = node->key;
            //printf(stderr, "Lock node: %d\n", node_key);

            ptlock_t* node_lock = &node->lock;
            LOCK(node_lock);
            
            node = fix_height_nl(node);

            //printf("Unlock node: %d\n", node_key);
            UNLOCK(node_lock);

            // releaseAll();
        } else {

            node_t* n_parent = node->parent;
            // publish(n_parent);
            // scoped_lock lock(n_parent->lock);
            bst_key_t n_parent_key = n_parent->key;
            //printf("Lock node: %d\n", n_parent_key);
            ptlock_t* n_parent_lock = &n_parent->lock;
            LOCK(n_parent_lock);

            if(!IS_UNLINKED(n_parent->version) && node->parent == n_parent){
                // publish(node);
                // scoped_lock nodeLock(node->lock);
                bst_key_t node_key = node->key;
                //printf("Lock node: %d\n", node_key);
                ptlock_t* node_lock = &node->lock;
                LOCK(node_lock);

                node = rebalance_nl(n_parent, node);

                //printf("Unlock node: %d\n", node_key);
                UNLOCK(node_lock);
            }

            //printf("Unlock node: %d\n", n_parent_key);
            UNLOCK(n_parent_lock);
            // releaseAll();
        }
    }    
}

//checked
node_t* rebalance_nl(node_t* n_parent, node_t* n){
    //printf("rebalance_nl: n_parent key %d, n_key %d\n", n_parent->key, n->key);

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

// checked
node_t* rebalance_to_right_nl(node_t* n_parent, node_t* n, node_t* nl, int hr0) {

    //printf("rebalance_to_right_nl\n");
    
    bst_key_t nl_key = nl->key;
    ptlock_t* nl_lock = &nl->lock;
    //printf("Lock node: %d\n", nl_key);
	LOCK(nl_lock);

	int hl = nl->height;
    if(hl - hr0 <= 1){
        //printf("Unlock node: %d\n", nl_key);
    	UNLOCK(nl_lock);
        return n;
    } else {
        // publish(nl->right);
        node_t* nlr = nl->right;

        int hll0 = HEIGHT(nl->left);
        int hlr0 = HEIGHT(nlr);

        if(hll0 > hlr0){
        	node_t* res = rotate_right_nl(n_parent, n, nl, hr0, hll0, nlr, hlr0);
        	//printf("Unlock node: %d\n", nl_key);
            UNLOCK(nl_lock);
            return res ;
        } else {
            {
                // if(reinterpret_cast<long>(&nlr->lock) == 0x30){
                //     return n;
                // }
                // scoped_lock sublock(nlr->lock);
                bst_key_t nlr_key = nlr->key;
                //printf("Lock node: %d\n", nlr_key);
                ptlock_t* nlr_lock = &nlr->lock;
                LOCK(nlr_lock);

                int hlr = nlr->height;
                if(hll0 >= hlr){
                	node_t* res = rotate_right_nl(n_parent, n, nl, hr0, hll0, nlr, hlr);
                	
                    //printf("Unlock node: %d\n", nlr_key);
                    UNLOCK(nlr_lock);
                    //printf("Unlock node: %d\n",nl_key);
                    UNLOCK(nl_lock);
                    return res;
                } else {
                    int hlrl = HEIGHT(nlr->left);
                    int b = hll0 - hlrl;
                    // CHANGED: Java and C++ implementations differ
                    if(b >= -1 && b <= 1 && !((hll0 == 0 || hlrl == 0) && !nl->value)){
                    	node_t* res = rotate_right_over_left_nl(n_parent, n, nl, hr0, hll0, nlr, hlrl);
                    	//printf("Unlock node: %d\n", nlr_key);
                        UNLOCK(nlr_lock);
                        //printf("Unlock node: %d\n", nl_key);
                        UNLOCK(nl_lock);
                        return res;
                    }
                }

                // CHANGED!
                UNLOCK(nlr_lock);
            }

            node_t* res = rebalance_to_left_nl(n, nl, nlr, hll0);
            //printf("Unlock node: %d\n", nl_key);
            UNLOCK(nl_lock);
            return res;
        }
    }

    // probably not necessary
    // UNLOCK(nl_lock);
}

// checked
node_t* rebalance_to_left_nl(node_t* n_parent, node_t* n, node_t* nr, int hl0) {

    //printf("rebalance_to_left_nl");

	// publish(nR);
 //    scoped_lock lock(nR->lock);
    
    bst_key_t nr_key = nr->key;
	//printf("Lock node: %d\n", nr_key);
    ptlock_t* nr_lock = &nr->lock;
	LOCK(nr_lock);

    int hr = nr->height;
    if(hl0 - hr >= -1){
        //printf("Unlock node: %d\n", nr_key);
    	UNLOCK(nr_lock);
        return n;
    } else {
        node_t* nrl = nr->left;
        int hrl0 = HEIGHT(nrl);
        int hrr0 = HEIGHT(nr->right);

        if(hrr0 >= hrl0){

            node_t* res = rotate_left_nl(n_parent, n, hl0, nr, nrl, hrl0, hrr0);
            UNLOCK(nr_lock);
            return res;
        } else {
            {
                // publish(nrl);
                // scoped_lock sublock(nrl->lock);
                bst_key_t nrl_key = nrl->key;
                //printf("Lock node: %d\n", nrl_key);
                ptlock_t* nrl_lock = &nrl->lock;
                LOCK(nrl_lock);

                int hrl = nrl->height;
                if(hrr0 >= hrl){
                	node_t* res = rotate_left_nl(n_parent, n, hl0, nr, nrl, hrl, hrr0);
                	//printf("Unlock node: %d\n", nrl_key);
                    UNLOCK(nrl_lock);
                    //printf("Unlock node: %d\n", nr_key);
                	UNLOCK(nr_lock);
                    return res;
                } else {
                    int hrlr = HEIGHT(nrl->right);
                    int b = hrr0 - hrlr;
                    // CHANGED
                    if(b >= -1 && b <= 1 && !((hrr0 == 0 || hrlr == 0) && !nr->value)){
                    	node_t* res = rotate_left_over_right_nl(n_parent, n, hl0, nr, nrl, hrr0, hrlr);

                        //printf("Unlock node: %d\n", nrl_key);
                        UNLOCK(nrl_lock);
                        //printf("Unlock node: %d\n", nr_key);
                		UNLOCK(nr_lock);
                        return res;
                    }
                }
                //printf("Unlock node: %d\n", nrl_key);
                UNLOCK(nrl_lock);

            }
            node_t* res = rebalance_to_right_nl(n, nr, nrl, hrr0);
            //printf("Unlock node: %d\n", nr_key);
            UNLOCK(nr_lock);
            return res;
        }
    }

    // UNLOCK(nr_lock);
}

// checked
node_t* rotate_right_nl(node_t* n_parent, node_t* n, node_t* nl, int hr, int hll, node_t* nlr, int hlr) {

    //printf("rotate_right_nl\n");

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

    // CHANGED from Java code
    if ((nlr == NULL || hr == 0) && !n->value) {
            // we need to remove n and then repair
            return n;
    }

    int ball = hll - hnrepl;
    if(ball < -1 || ball > 1){
        return nl;
    }

    // CHANGED
    if (hll == 0 && !nl->value) {
            return nl;
    }

    return fix_height_nl(n_parent);
}

// checked
node_t* rotate_left_nl(node_t* n_parent, node_t* n, int hl, node_t* nr, node_t* nrl, int hrl, int hrr){

    //printf("rotate_left_nl");


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

    // CHANGED
    if ((nrl == NULL || hl == 0) && !n->value) {
            return n;
    }

    int balr = hrr - hnrepl;
    if(balr < -1 || balr > 1){
        return nr;
    }

    // CHANGED
    if (hrr == 0 && !nr->value) {
        return nr;
    }


    return fix_height_nl(n_parent);
}

// checked
node_t* rotate_right_over_left_nl(node_t* n_parent, node_t* n, node_t* nl, int hr, int hll, node_t* nlr, int hlrl){

    //printf("rotate_right_over_left_nl\n");


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

// checked
node_t* rotate_left_over_right_nl(node_t* n_parent, node_t* n, int hl, node_t* nr, node_t* nrl, int hrr, int hrlr){

    //printf("rotate_left_over_right_nl\n");


    uint64_t node_ovl = n->version;
    uint64_t right_ovl = nr->version;

    // CHANGED place
    n->version = BEGIN_CHANGE(node_ovl);
    nr->version = BEGIN_CHANGE(right_ovl);
    
    node_t* npl = n_parent->left;
    node_t* nrll = nrl->left;
    node_t* nrlr = nrl->right;
    int hrll = HEIGHT(nrll);


    n->right = nrll;
    if(nrll != NULL){
        nrll->parent = n;
    }

    nr->left = nrlr;
    if(nrlr != NULL){
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

    // CHANGED
    if ((nrll == NULL || hl == 0) && !n->value) {
        return n;
    }

    int balrl = hrrepl - hnrepl;
    if(balrl < -1 || balrl > 1){
        return nrl;
    }
    
    return fix_height_nl(n_parent);
}

// checked
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
	} else if (!node->value) {
		return bst_size(node->left) + bst_size(node->right);
	} else {
		return 1 + bst_size(node->left) + bst_size(node->right);
	}
}


void bst_print(node_t* node) {

    if (node == NULL) {
        return;
    }

    if (node->value == TRUE) {

        //printf("%d, ", node->key);
    }

    //printf("Left \n");
    bst_print(node->left);
    //printf("right \n");
    bst_print(node->right);

}
