#ifndef _BST_ARAVIND_H_INCLUDED_
#define _BST_ARAVIND_H_INCLUDED_

enum{INS,DEL};
enum {BLACK,RED};
enum {UNMARK,MARK};
enum {UNFLAG,FLAG};
typedef uintptr_t Word;
const unsigned WORD_RESERVED_BITS = 2;

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lock_if.h"
#include "common.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"
#include "ssmem.h"

#define TRUE 1
#define FALSE 0

#define MAX_KEY KEY_MAX
#define MIN_KEY 0

typedef uint8_t bool_t;

extern __thread ssmem_allocator_t* alloc;

typedef ALIGNED(64) struct node_t node_t;

struct node_t{
    skey_t key;
    sval_t value;
    node_t* right;
    node_t* left;
};


typedef struct seekRecord{
    // SeekRecord structure
    //node_t * leaf;
    skey_t leafKey;
    node_t * parent;
    AO_t pL;
    bool isLeftL; // is L the left child of P?
    node_t * lum;
    AO_t lumC;
    bool isLeftUM; // is  last unmarked node's child on access path the left child of  the last unmarked node?
    //char padding[ CACHE_LINE_SIZE ];
} seekRecord_t;


seekRecord_t * insseek(thread_data_t * data, skey_t key, int op);
seekRecord_t * delseek(thread_data_t * data, skey_t key, int op);
seekRecord_t * secondary_seek(thread_data_t * data, skey_t key, seekRecord_t * sr);
sval_t search(thread_data_t * data, skey_t key);
int help_conflicting_operation (thread_data_t * data, seekRecord_t * R);
int inject(thread_data_t * data, seekRecord_t * R, int op);
bool_t insert(thread_data_t * data, skey_t key, sval_t value);
sval_t delete_node(thread_data_t * data, skey_t key);
#endif
