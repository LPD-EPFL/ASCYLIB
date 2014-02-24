#ifndef _BARRIER_H_
#define _BARRIER_H_

/* ################################################################### *
 * BARRIER
 * ################################################################### */

typedef struct barrier 
{
  pthread_cond_t complete;
  pthread_mutex_t mutex;
  int count;
  int crossing;
} barrier_t;

static inline void
 barrier_init(barrier_t *b, int n) 
{
  pthread_cond_init(&b->complete, NULL);
  pthread_mutex_init(&b->mutex, NULL);
  b->count = n;
  b->crossing = 0;
}

static inline void 
barrier_cross(barrier_t *b) 
{
  pthread_mutex_lock(&b->mutex);
  /* One more thread through */
  b->crossing++;
  /* If not all here, wait */
  if (b->crossing < b->count) {
    pthread_cond_wait(&b->complete, &b->mutex);
  } else {
    pthread_cond_broadcast(&b->complete);
    /* Reset for next time */
    b->crossing = 0;
  }
  pthread_mutex_unlock(&b->mutex);
}

#define EXEC_IN_DEC_ID_ORDER(id, nthr)		\
  { int __i;					\
  for (__i = nthr - 1; __i >= 0; __i--)		\
    {						\
  if (id == __i)				\
    {

#define EXEC_IN_DEC_ID_ORDER_END(barrier)	\
  }						\
    barrier_cross(barrier);			\
    }}

#endif	/* _BARRIER_H_ */
