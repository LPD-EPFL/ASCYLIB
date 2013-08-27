//clh lock
#ifndef _CLH_H_
#define _CLH_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#if defined(PLATFORM_NUMA)
#include <numa.h>
#endif
#include <pthread.h>
#include "utils.h"
#include "atomic_ops.h"

typedef struct clh_qnode 
{
  volatile uint8_t locked;
#ifdef ADD_PADDING
  uint8_t padding[CACHE_LINE_SIZE - 1];
#endif
} clh_qnode;

typedef volatile clh_qnode* clh_qnode_ptr;
typedef clh_qnode_ptr clh_lock;

typedef struct clh_local_params 
{
  clh_qnode* my_qnode;
  clh_qnode* my_pred;
} clh_local_params;


typedef struct clh_global_params 
{
  clh_lock* the_lock;
#ifdef ADD_PADDING
  uint8_t padding[CACHE_LINE_SIZE - 8];
#endif
} clh_global_params;

typedef clh_global_params clh_lock_t;
typedef clh_lock_t ptlock_t;


extern __thread clh_local_params clh_local_p;

//lock
clh_qnode* clh_acquire(clh_lock* the_lock, clh_qnode* my_qnode);

//unlock
clh_qnode* clh_release(clh_qnode* my_qnode, clh_qnode* my_pred);

clh_global_params* init_clh_locks(uint32_t num_locks);
void init_alloc_clh(clh_lock_t* lock);
void init_clh_thread(clh_local_params* local_params);

void end_thread_clh(clh_local_params* the_params, uint32_t size);
void end_clh(clh_global_params* the_locks, uint32_t size);

#endif
