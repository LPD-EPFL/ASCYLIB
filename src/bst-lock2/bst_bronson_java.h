#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "ssalloc.h"
#include "lock_if.h"

// TODO maybe change value of RETRY to 1?
// TODO maybe make retry a pointer to a dummy value (bst_value_t)
#define FOUND 1
#define NOT_FOUND 2
#define RETRY 3

#define UPDATE_IF_PRESENT 1
#define UPDATE_IF_ABSENT 2

#define UNLINKED_OVL 2

#define TRUE 1
#define FALSE 0

// checked
#define UNLINK_REQUIRED -1
#define REBALANCE_REQUIRED -2
#define NOTHING_REQUIRED -3

// Spin time for bst_wait_until_not_changing
#define SPIN_COUNT 100

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
   
// typedef uint32_t bst_height_t; 
// typedef uint64_t bst_version_t;
typedef uint8_t bool_t;
typedef uint8_t result_t;
typedef uint8_t function_t;

typedef uint32_t bst_key_t;
typedef bool_t bst_value_t;

typedef ALIGNED(64) struct node_t node_t;


// TODO define constants (Fig 3 on page 3)

struct node_t {
	// TODO should this be volatile?
	/*volatile*/ int height;
				 bst_key_t key;
				 bst_value_t value;
	/*volatile*/ uint64_t version;
	
	volatile node_t* parent;
	volatile node_t* left;
	volatile node_t* right;
	volatile ptlock_t lock;	
};

// bst functions
node_t* bst_initialize();
bool_t bst_contains(bst_key_t k, node_t* root);
bool_t bst_add(bst_key_t k, node_t* root);
bool_t bst_remove(bst_key_t k, node_t* root);

// private functions
void wait_until_not_changing(node_t* node);
bool_t attempt_unlink_nl(node_t* parent, node_t* node);
int node_conditon(node_t* node);
void fix_height_and_rebalance(node_t* node);
node_t* fix_height_nl(node_t* node);
node_t* rebalance_nl(node_t* n_parent, node_t* n);
node_t* rebalance_to_right_nl(node_t* n_parent, node_t* n, node_t* nl, int hr0);
node_t* rebalance_to_left_nl(node_t* n_parent, node_t* n, node_t* nr, int hl0);
node_t* rotate_right_nl(node_t* n_parent, node_t* n, node_t* nl, int hr, int hll, node_t* nlr, int hlr);
node_t* rotate_left_nl(node_t* n_parent, node_t* n, int hl, node_t* nr, node_t* nrl, int hrl, int hrr);
node_t* rotate_right_over_left_nl(node_t* n_parent, node_t* n, node_t* nl, int hr, int hll, node_t* nlr, int hlrl);
node_t* rotate_left_over_right_nl(node_t* n_parent, node_t* n, int hl, node_t* nr, node_t* nrl, int hrr, int hrlr);
void set_child(node_t* parent, node_t* child, bool_t is_right);
result_t attempt_node_update(function_t func, bst_value_t expected, bst_value_t new_value, node_t* parent, node_t* node);
result_t attempt_update(bst_key_t key, function_t func, bst_value_t expected, bst_value_t new_value, node_t* parent, node_t* node, uint64_t node_v);
node_t* new_node(int height, bst_key_t key, uint64_t version, bst_value_t value, node_t* parent, node_t* left, node_t* right);
bool_t attempt_insert_into_empty(bst_key_t key, bst_value_t value, node_t* holder);
result_t update_under_root(bst_key_t k, function_t func, bst_value_t expected, bst_value_t new_value, node_t* holder);
result_t attempt_get(bst_key_t k, node_t* node, bool_t is_right, uint64_t node_v);
void bst_print(node_t* node);

uint64_t bst_size(node_t* node);

// checked
static inline node_t* CHILD(node_t* parent, bool_t is_right) {
	return is_right ? parent->right : parent->left;
}

// checked
static inline uint64_t BEGIN_CHANGE(uint64_t ovl) {
	return (ovl | 1);
}

// checked
static inline uint64_t END_CHANGE(uint64_t ovl) {
	return (ovl | 3) + 1;
}

// checked
static inline int HEIGHT(node_t* node) {
	return node == NULL ? 0 : node->height;
}

// checked
static inline bool_t IS_SHRINKING(bst_value_t ovl) {
	return (bool_t)((ovl & 1) != 0);
}


// checked
static inline bool_t IS_UNLINKED(bst_value_t ovl) {
	return (bool_t)((ovl & 2) != 0);
}

// checked
static inline bool_t IS_SHRINKING_OR_UNLINKED(uint64_t ovl){
	return (bool_t)((ovl & 3) != 0L);
}

// checked
static inline bool_t SHOULD_UPDATE(function_t func, bool_t prev) {

	return func == UPDATE_IF_ABSENT ? !prev : prev;
}

// checked
static inline result_t UPDATE_RESULT(function_t func) {

	return func == UPDATE_IF_ABSENT ? NOT_FOUND : FOUND;
}

// checked
static inline result_t NO_UPDATE_RESULT(function_t func){
    return func == UPDATE_IF_ABSENT ? FOUND : NOT_FOUND;
}