#ifndef _COPY_ON_WRITE_H_
#define _COPY_ON_WRITE_H_

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
#include "lock_if.h"

#include "common.h"
#include "utils.h"
#include "measurements.h"
#include "ssalloc.h"
#include "ssmem.h"

#define DEFAULT_ALTERNATE		0
#define DEFAULT_EFFECTIVE		1
#define DEFAULT_LOAD		        1

#define CPY_ON_WRITE_READ_ONLY_FAIL     1

static volatile int stop;
extern __thread ssmem_allocator_t* alloc;
extern size_t array_ll_fixed_size;

typedef volatile struct kv
{
  skey_t key;
  sval_t val;
} kv_t;


typedef struct array_ll
{
  size_t size;
  kv_t* kvs;
} array_ll_t;


typedef ALIGNED(CACHE_LINE_SIZE) struct copy_on_write
{
   union
   {
     struct
     {
       size_t num_buckets;
       size_t hash;
       volatile ptlock_t* lock;
       volatile array_ll_t** array;
     };
     uint8_t padding[CACHE_LINE_SIZE];
   };
} copy_on_write_t;

copy_on_write_t* copy_on_write_new();
size_t copy_on_write_size(copy_on_write_t* set);
sval_t cpy_search(copy_on_write_t* set, skey_t key);
int cpy_insert(copy_on_write_t* set, skey_t key, sval_t val);
sval_t cpy_delete(copy_on_write_t* set, skey_t key);

#endif	/* _COPY_ON_WRITE_H_ */
