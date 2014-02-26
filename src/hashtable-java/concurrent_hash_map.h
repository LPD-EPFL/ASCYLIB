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

#define CHM_NUM_SEGMENTS_INIT           128

/* 
 * structures
 */

typedef volatile struct chm_node
{
  skey_t key;
  sval_t val;
  volatile struct chm_node* next;
} chm_node_t;

typedef struct ALIGNED(64) chm
{
  union
  {
    struct
    {
      size_t num_buckets;
      size_t hash;
      size_t num_segments;
      size_t hash_seg;
      chm_node_t** table;
      ptlock_t* locks;
    };
    uint8_t padding[CACHE_LINE_SIZE];
  };
} chm_t;

/* 
 * interface
 */

chm_t* chm_new();
sval_t chm_get(chm_t* set, skey_t key);
int chm_put(chm_t* set, skey_t key, sval_t val);
sval_t chm_rem(chm_t* set, skey_t key);
size_t chm_size(chm_t* set);

extern __thread ssmem_allocator_t* alloc;
extern size_t maxhtlength;
#endif	/* _CONCURRENT_HASH_MAP_H_ */
