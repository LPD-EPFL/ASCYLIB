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

#define UNLINK_REQUIRED = -1;
#define REBALANCE_REQUIRED = -2;
#define NOTHING_REQUIRED = -3;

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
	volatile int height;
	volatile uint64_t version;
	bst_key_t key;
	bst_value_t value;
	volatile node_t* parent;
	volatile node_t* left;
	volatile node_t* right;
	volatile ptlock_t lock;	
};

node_t* bst_initialize();
bool_t bst_contains(bst_key_t k, node_t* root)unsigned long bst_size(node_t* node);
bool_t bst_add(bst_key_t k, node_t* root);
bool_t bst_remove(bst_key_t k, node_t* root);

uint64_t bst_size(node_t* node);


static inline node_t* CHILD(node_t* parent, bool_t is_right) {
	return is_right ? parent->right : parent->left;
}


static inline uint64_t BEGIN_CHANGE(uint64_t ovl) {
	return (ovl | 1);
}

static inline uint64_t END_CHANGE(uint64_t ovl) {
	return (ovl | 3) + 1;
}

static inline int HEIGHT(node_t* node) {
	return node == NULL ? 0 : node->height;
}

static inline bool_t IS_SHRINKING(bst_value_t ovl) {
	return (bool_t)((ovl & 1) != 0);
}

static inline bool_t IS_UNLINKED(bst_value_t ovl) {
	return (bool_t)((ovl & 2) != 0);
}

static inline bool_t IS_SHRINKING_OR_UNLINKED(uint64_t ovl){
	return (bool_t)((ovl & 2) != 0);
}

static inline bool_t SHOULD_UPDATE(function_t func, bool_t prev) {

	return func == UPDATE_IF_ABSENT ? !prev : prev;
}

static inline result_t UPDATE_RESULT(function_t func) {

	return func == UPDATE_IF_ABSENT ? NOT_FOUND : FOUND;
}

static inline result_t NO_UPDATE_RESULT(function_t func){
    return func == UPDATE_IF_ABSENT ? FOUND : NOT_FOUND;
}

static inline HEIGHT(node_t* node) {
	return (node == NULL) ? 0 : node->height;
}