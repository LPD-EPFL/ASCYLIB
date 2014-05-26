#ifndef __inlcude_timnat_h_
#define  __inlcude_timnat_h_
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include <atomic_ops.h>
#include "atomic_ops_if.h"

#include "common.h"
#include "utils.h"
#include "measurements.h"
#include "ssalloc.h"
#include "ssmem.h"

#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
#endif

static volatile int stop;
extern __thread ssmem_allocator_t* alloc;

typedef volatile struct node 
{
  skey_t key;
  sval_t val;
  volatile struct node* next;
  uint8_t padding32[8];
#if defined(DO_PAD)
  uint8_t padding[CACHE_LINE_SIZE - sizeof(sval_t) - sizeof(skey_t) - sizeof(struct node*)];
#endif
} node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct intset 
{
  node_t *head;
} intset_t;

node_t *new_node(skey_t key, sval_t val, node_t *next, int initializing);
intset_t *set_new();
void set_delete(intset_t *set);

inline int is_marked_ref(long i);
inline long unset_mark(long i);
inline long set_mark(long i);
inline long get_unmarked_ref(long w);
inline long get_marked_ref(long w);

node_t* timnat_search(intset_t *set, skey_t key, node_t** left_node);
sval_t timnat_find(intset_t *set, skey_t key);
int timnat_insert(intset_t *set, skey_t key, sval_t val);
sval_t timnat_delete(intset_t *set, skey_t key);
int set_size(intset_t *set);

sval_t set_contains(intset_t *set, skey_t key);
int set_add(intset_t *set, skey_t key, skey_t val);
sval_t set_remove(intset_t *set, skey_t key);

#endif
