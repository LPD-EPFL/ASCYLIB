#ifndef _LOCK_IF_H_
#define _LOCK_IF_H_

#include "utils.h"

#if defined(MUTEX)
typedef pthread_mutex_t ptlock_t;
#  define INIT_LOCK(lock)				pthread_mutex_init((pthread_mutex_t *) lock, NULL);
#  define DESTROY_LOCK(lock)			pthread_mutex_destroy((pthread_mutex_t *) lock)
#  define LOCK(lock)					pthread_mutex_lock((pthread_mutex_t *) lock)
#  define UNLOCK(lock)					pthread_mutex_unlock((pthread_mutex_t *) lock)
#elif defined(SPIN)		/* pthread spinlock */
typedef pthread_spinlock_t ptlock_t;
#  define INIT_LOCK(lock)				pthread_spin_init((pthread_spinlock_t *) lock, PTHREAD_PROCESS_PRIVATE);
#  define DESTROY_LOCK(lock)			pthread_spin_destroy((pthread_spinlock_t *) lock)
#  define LOCK(lock)					pthread_spin_lock((pthread_spinlock_t *) lock)
#  define UNLOCK(lock)					pthread_spin_unlock((pthread_spinlock_t *) lock)
#elif defined(TAS)			/* TAS */
typedef uint32_t ptlock_t;
#  define INIT_LOCK(lock)				tas_init((volatile uint32_t*) lock)
#  define DESTROY_LOCK(lock)			
#  define LOCK(lock)					tas_lock((volatile uint32_t*) lock)
#  define UNLOCK(lock)					tas_unlock((volatile uint32_t*) lock)

#  define TAS_FREE 0
#  define TAS_LCKD 1

static inline void
tas_init(volatile uint32_t* l)
{
  *l = TAS_FREE;
  MEM_BARRIER;
}

static inline uint32_t
tas_lock(volatile uint32_t* l)
{
  while (CAS_U32(l, TAS_FREE, TAS_LCKD) == TAS_LCKD)
    {
      PAUSE;
    }
  return 0;
}

static inline uint32_t
tas_unlock(volatile uint32_t* l)
{
  /* MEM_BARRIER; */
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

static inline void
ticket_init(volatile ptlock_t* l)
{
  l->ticket = l->curr = 0;
  MEM_BARRIER;
}

static inline uint32_t
ticket_lock(volatile ptlock_t* l)
{
  uint32_t ticket = FAI_U32(&l->ticket);

  PREFETCHW(l);
  while (ticket != l->curr)
    {
      PREFETCHW(l);
      PAUSE;
    }

  /* MEM_BARRIER; */
  
  return 0;
}

static inline uint32_t
ticket_unlock(volatile ptlock_t* l)
{
  MEM_BARRIER;
  PREFETCHW(l);
  l->curr++;
  /* FAI_U32(&l->curr); */
  return 0;
}

#endif

#endif	/* _LOCK_IF_H_ */
