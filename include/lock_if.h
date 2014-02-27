#ifndef _LOCK_IF_H_
#define _LOCK_IF_H_

#include "utils.h"

#define PREFETCHW_LOCK(lock)                            PREFETCHW(lock)

#if defined(MUTEX)
typedef pthread_mutex_t ptlock_t;
#define PTLOCK_SIZE sizeof(ptlock_t)
#  define INIT_LOCK(lock)				pthread_mutex_init((pthread_mutex_t *) lock, NULL);
#  define DESTROY_LOCK(lock)			        pthread_mutex_destroy((pthread_mutex_t *) lock)
#  define LOCK(lock)					pthread_mutex_lock((pthread_mutex_t *) lock)
#  define UNLOCK(lock)					pthread_mutex_unlock((pthread_mutex_t *) lock)
/* GLOBAL lock */
#  define GL_INIT_LOCK(lock)				pthread_mutex_init((pthread_mutex_t *) lock, NULL);
#  define GL_DESTROY_LOCK(lock)			        pthread_mutex_destroy((pthread_mutex_t *) lock)
#  define GL_LOCK(lock)					pthread_mutex_lock((pthread_mutex_t *) lock)
#  define GL_UNLOCK(lock)				pthread_mutex_unlock((pthread_mutex_t *) lock)
#elif defined(SPIN)		/* pthread spinlock */
typedef pthread_spinlock_t ptlock_t;
#define PTLOCK_SIZE sizeof(ptlock_t)
#  define INIT_LOCK(lock)				pthread_spin_init((pthread_spinlock_t *) lock, PTHREAD_PROCESS_PRIVATE);
#  define DESTROY_LOCK(lock)			        pthread_spin_destroy((pthread_spinlock_t *) lock)
#  define LOCK(lock)					pthread_spin_lock((pthread_spinlock_t *) lock)
#  define UNLOCK(lock)					pthread_spin_unlock((pthread_spinlock_t *) lock)
/* GLOBAL lock */
#  define GL_INIT_LOCK(lock)				pthread_spin_init((pthread_spinlock_t *) lock, PTHREAD_PROCESS_PRIVATE);
#  define GL_DESTROY_LOCK(lock)			        pthread_spin_destroy((pthread_spinlock_t *) lock)
#  define GL_LOCK(lock)					pthread_spin_lock((pthread_spinlock_t *) lock)
#  define GL_UNLOCK(lock)				pthread_spin_unlock((pthread_spinlock_t *) lock)
#elif defined(TAS)			/* TAS */
#  define PTLOCK_SIZE 32		/* choose 8, 16, 32, 64 */
#  define PASTER(x, y, z) x ## y ## z
#  define EVALUATE(sz) PASTER(uint, sz, _t)
#  define UTYPE  EVALUATE(PTLOCK_SIZE)
#  define PASTER2(x, y) x ## y
#  define EVALUATE2(sz) PASTER2(CAS_U, sz)
#  define CAS_UTYPE EVALUATE2(PTLOCK_SIZE)
typedef volatile UTYPE ptlock_t;
#  define INIT_LOCK(lock)				tas_init(lock)
#  define DESTROY_LOCK(lock)			
#  define LOCK(lock)					tas_lock(lock)
#  define TRYLOCK(lock)					tas_trylock(lock)
#  define UNLOCK(lock)					tas_unlock(lock)
/* GLOBAL lock */
#  define GL_INIT_LOCK(lock)				tas_init(lock)
#  define GL_DESTROY_LOCK(lock)			
#  define GL_LOCK(lock)					tas_lock(lock)
#  define GL_UNLOCK(lock)              			tas_unlock(lock)

#  define TAS_FREE 0
#  define TAS_LCKD 1

static inline void
tas_init(ptlock_t* l)
{
  *l = TAS_FREE;
#if defined(__tile__)
  MEM_BARRIER;
#endif
}

static inline uint32_t
tas_lock(ptlock_t* l)
{
  while (CAS_UTYPE(l, TAS_FREE, TAS_LCKD) == TAS_LCKD)
    {
      PAUSE;
    }
  return 0;
}

static inline uint32_t
tas_trylock(ptlock_t* l)
{
  return (CAS_UTYPE(l, TAS_FREE, TAS_LCKD) == TAS_FREE);
}

static inline uint32_t
tas_unlock(ptlock_t* l)
{
#if defined(__tile__)
  MEM_BARRIER;
#endif
  *l = TAS_FREE;
  return 0;
}

#elif defined(TICKET)			/* ticket lock */

struct ticket_st
{
  uint32_t ticket;
  uint32_t curr;
  /* char padding[40]; */
};

typedef struct ticket_st ptlock_t;
#  define INIT_LOCK(lock)				ticket_init((volatile ptlock_t*) lock)
#  define DESTROY_LOCK(lock)			
#  define LOCK(lock)					ticket_lock((volatile ptlock_t*) lock)
#  define UNLOCK(lock)					ticket_unlock((volatile ptlock_t*) lock)
/* GLOBAL lock */
#  define GL_INIT_LOCK(lock)				ticket_init((volatile ptlock_t*) lock)
#  define GL_DESTROY_LOCK(lock)			
#  define GL_LOCK(lock)					ticket_lock((volatile ptlock_t*) lock)
#  define GL_UNLOCK(lock)				ticket_unlock((volatile ptlock_t*) lock)

static inline void
ticket_init(volatile ptlock_t* l)
{
  l->ticket = l->curr = 0;
#if defined(__tile__)
  MEM_BARRIER;
#endif
}

#define TICKET_BASE_WAIT 512
#define TICKET_MAX_WAIT  4095
#define TICKET_WAIT_NEXT 128

static inline uint32_t
ticket_lock(volatile ptlock_t* l)
{
  uint32_t ticket = FAI_U32(&l->ticket);

#if defined(OPTERON_OPTIMIZE)
  uint32_t wait = TICKET_BASE_WAIT;
  uint32_t distance_prev = 1;
  while (1)
    {
      PREFETCHW(l);
      uint32_t cur = l->curr;
      if (cur == ticket)
	{
	  break;
	}
      uint32_t distance = (ticket > cur) ? (ticket - cur) : (cur - ticket);

      if (distance > 1)
      	{
	  if (distance != distance_prev)
	    {
	      distance_prev = distance;
	      wait = TICKET_BASE_WAIT;
	    }

	  nop_rep(distance * wait);
	  wait = (wait + TICKET_BASE_WAIT) & TICKET_MAX_WAIT;
      	}
      else
	{
	  nop_rep(TICKET_WAIT_NEXT);
	}

      if (distance > 20)
      	{
      	  sched_yield();
      	}
    }

#else  /* !OPTERON_OPTIMIZE */
  PREFETCHW(l);
  while (ticket != l->curr)
    {
      PREFETCHW(l);
      PAUSE;
    }

#endif
  return 0;
}

static inline uint32_t
ticket_unlock(volatile ptlock_t* l)
{
#if defined(__tile__)
  MEM_BARRIER;
#endif
  PREFETCHW(l);
  l->curr++;
  return 0;
}

#elif defined(HTICKET)		/* Hierarchical ticket lock */

#  include "htlock.h"

#  define INIT_LOCK(lock)				init_alloc_htlock((htlock_t*) lock)
#  define DESTROY_LOCK(lock)			
#  define LOCK(lock)					htlock_lock((htlock_t*) lock)
#  define UNLOCK(lock)					htlock_release((htlock_t*) lock)
/* GLOBAL lock */
#  define GL_INIT_LOCK(lock)				init_alloc_htlock((htlock_t*) lock)
#  define GL_DESTROY_LOCK(lock)			
#  define GL_LOCK(lock)					htlock_lock((htlock_t*) lock)
#  define GL_UNLOCK(lock)				htlock_release((htlock_t*) lock)

#elif defined(CLH)		/* CLH lock */

#  include "clh.h"

#  define INIT_LOCK(lock)				init_alloc_clh((clh_lock_t*) lock)
#  define DESTROY_LOCK(lock)			
#  define LOCK(lock)					clh_local_p.my_pred = \
    clh_acquire((volatile struct clh_qnode **) lock, clh_local_p.my_qnode);
#  define UNLOCK(lock)					clh_local_p.my_qnode = \
    clh_release(clh_local_p.my_qnode, clh_local_p.my_pred);
/* GLOBAL lock */
#  define GL_INIT_LOCK(lock)				init_alloc_clh((clh_lock_t*) lock)
#  define GL_DESTROY_LOCK(lock)			
#  define GL_LOCK(lock)					clh_local_p.my_pred = \
    clh_acquire((volatile struct clh_qnode **) lock, clh_local_p.my_qnode);
#  define GL_UNLOCK(lock)				clh_local_p.my_qnode = \
    clh_release(clh_local_p.my_qnode, clh_local_p.my_pred);
#endif



/* --------------------------------------------------------------------------------------------------- */
/* GLOBAL LOCK --------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */


#if defined(LL_GLOBAL_LOCK)
#  define ND_GET_LOCK(nd)                 nd /* LOCK / UNLOCK are not defined in any case ;-) */

#  undef INIT_LOCK
#  undef DESTROY_LOCK
#  undef LOCK
#  undef UNLOCK
#  undef PREFETCHW_LOCK

#  define INIT_LOCK(lock)
#  define DESTROY_LOCK(lock)			
#  define LOCK(lock)
#  define UNLOCK(lock)
#  define PREFETCHW_LOCK(lock)

#else  /* !LL_GLOBAL_LOCK */
#  define ND_GET_LOCK(nd)                 &nd->lock

#  undef GL_INIT_LOCK
#  undef GL_DESTROY_LOCK
#  undef GL_LOCK
#  undef GL_UNLOCK

#  define GL_INIT_LOCK(lock)
#  define GL_DESTROY_LOCK(lock)			
#  define GL_LOCK(lock)
#  define GL_UNLOCK(lock)

#endif

#endif	/* _LOCK_IF_H_ */
