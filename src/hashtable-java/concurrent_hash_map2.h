#ifndef _CONCURRENT_HASH_MAP_H_
#define _CONCURRENT_HASH_MAP_H_

#include "utils.h"
#include "common.h"
#include "lock_if.h"
#include "ssmem.h"

#define DEFAULT_LOAD                    1
#define MAXHTLENGTH                     65536

/* 
 * parameters
 */

#define CHM_NUM_SEGMENTS                512
#define CHM_LOAD_FACTOR                 0.75
#define CHM_TRY_PREFETCH                0
#define CHM_MAX_SCAN_RETRIES            64
#define CHM_READ_ONLY_FAIL              RO_FAIL

/* 
 * structures
 */

typedef volatile struct chm_node
{
  skey_t key;
  sval_t val;
  uint8_t padding32[8];
  volatile struct chm_node* next;
} chm_node_t;

typedef volatile struct chm_seg
{
  union
  {
    struct
    {
      size_t num_buckets;
      size_t hash;
      ptlock_t lock;
      uint32_t modifications;
      uint32_t size;
      float load_factor;
      uint32_t size_limit;
      chm_node_t** table;
    };
    uint8_t padding[CACHE_LINE_SIZE];
  };
} chm_seg_t;

typedef struct ALIGNED(CACHE_LINE_SIZE) chm
{
  union
  {
    struct
    {
      size_t num_segments;
      size_t hash;
      int hash_seed;
      chm_seg_t** segments;
    };
    uint8_t padding[CACHE_LINE_SIZE];
  };
} chm_t;

/* 
 * interface
 */

chm_t* chm_new(size_t capacity, size_t concurrency);
sval_t chm_get(chm_t* set, skey_t key);
int chm_put(chm_t* set, skey_t key, sval_t val);
sval_t chm_rem(chm_t* set, skey_t key);
size_t chm_size(chm_t* set);

extern __thread ssmem_allocator_t* alloc;
extern size_t maxhtlength;
#endif	/* _CONCURRENT_HASH_MAP_H_ */
