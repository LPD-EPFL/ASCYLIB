#include "clh.h"

__thread clh_local_params clh_local_p;

clh_qnode* 
clh_acquire(clh_lock *L, clh_qnode* I) 
{
  I->locked=1;
#ifndef  __tile__
  clh_qnode_ptr pred = (clh_qnode*) SWAP_PTR((volatile void*) (L), (void*) I);
#else
  MEM_BARRIER;
  clh_qnode_ptr pred = (clh_qnode*) SWAP_PTR( L, I);
#endif
  if (pred == NULL) 		/* lock was free */
    return NULL;
#if defined(OPTERON_OPTIMIZE)
  PREFETCHW(pred);
#endif	/* OPTERON_OPTIMIZE */
  while (pred->locked != 0) 
    {
      PAUSE;
#if defined(OPTERON_OPTIMIZE)
      pause_rep(23);
      PREFETCHW(pred);
#endif	/* OPTERON_OPTIMIZE */
    }

  return (clh_qnode*) pred;
}

clh_qnode* 
clh_release(clh_qnode *my_qnode, clh_qnode * my_pred) 
{
  my_qnode->locked=0;
  return my_pred;
}

clh_global_params* 
init_clh_locks(uint32_t num_locks) 
{
  clh_global_params* the_params;
  the_params = (clh_global_params*)malloc(num_locks * sizeof(clh_global_params));
  uint32_t i;
  for (i=0;i<num_locks;i++) {
    the_params[i].the_lock=(clh_lock*)malloc(sizeof(clh_lock));
    clh_qnode * a_node = (clh_qnode *) malloc(sizeof(clh_qnode));
    a_node->locked=0;
    *(the_params[i].the_lock) = a_node;
  }
  return the_params;
}

void
init_alloc_clh(clh_lock_t* lock) 
{
  clh_qnode* a_node = (clh_qnode *) memalign(CACHE_LINE_SIZE, sizeof(clh_qnode));
  a_node->locked=0;
  lock->the_lock = (clh_lock*) a_node;
}

void
destroy_free_clh(clh_lock* lock) 
{
}

void
init_clh_thread(clh_local_params* local_params)
{
  /* set_cpu(thread_num); */
  //init its qnodes
  /* local_params = (clh_local_params*)malloc(sizeof(clh_local_params)); */
  local_params->my_qnode = (clh_qnode*) memalign(CACHE_LINE_SIZE, sizeof(clh_qnode));
  local_params->my_qnode->locked=0;
  local_params->my_pred = NULL;
}

void end_thread_clh(clh_local_params* the_params, uint32_t size)
{
  free(the_params);
}

void end_clh(clh_global_params* the_locks, uint32_t size) {
    uint32_t i;
    for (i = 0; i < size; i++) {
        free(the_locks[i].the_lock);
    }
    free(the_locks);
}


