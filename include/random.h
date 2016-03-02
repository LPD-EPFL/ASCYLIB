/*   
 *   File: random.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Tudor David <tudor.david@epfl.ch>
 *   Description: 
 *   random.h is part of ASCYLIB
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
 * 	     	      Tudor David <tudor.david@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * ASCYLIB is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _H_RANDOM_
#define _H_RANDOM_

#include <malloc.h>
#include <stdio.h>
#include "measurements.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"

#define LOCAL_RAND

#if defined(LOCAL_RAND)
extern __thread unsigned long* seeds; 
#endif


#define mrand(x) xorshf96(&x[0], &x[1], &x[2])
#define my_random xorshf96

//fast but weak random number generator for the sparc machine
static inline uint32_t
fast_rand() 
{
  return ((getticks()&4294967295UL)>>4);
}


static inline unsigned long* 
seed_rand() 
{
  unsigned long* seeds;
  /* seeds = (unsigned long*) ssalloc_aligned(64, 64); */
  seeds = (unsigned long*) memalign(64, 64);
  seeds[0] = getticks() % 123456789;
  seeds[1] = getticks() % 362436069;
  seeds[2] = getticks() % 521288629;
  return seeds;
}

//Marsaglia's xorshf generator
static inline unsigned long
xorshf96(unsigned long* x, unsigned long* y, unsigned long* z)  //period 2^96-1
{
  unsigned long t;
  (*x) ^= (*x) << 16;
  (*x) ^= (*x) >> 5;
  (*x) ^= (*x) << 1;

  t = *x;
  (*x) = *y;
  (*y) = *z;
  (*z) = t ^ (*x) ^ (*y);

  return *z;
}

static inline long
rand_range(long r) 
{
  /* PF_START(0); */
#if defined(LOCAL_RAND)
  long v = xorshf96(seeds, seeds + 1, seeds + 2) % r;
  v++;
#else
  int m = RAND_MAX;
  long d, v = 0;
	
  do {
    d = (m > r ? r : m);
    v += 1 + (long)(d * ((double)rand()/((double)(m)+1.0)));
    r -= m;
  } while (r > 0);
#endif
  /* PF_STOP(0); */
  return v;
}

/* Re-entrant version of rand_range(r) */
static inline long
rand_range_re(unsigned int *seed, long r) 
{
  /* PF_START(0); */
#if defined(LOCAL_RAND)
  long v = xorshf96(seeds, seeds + 1, seeds + 2) % r;
  v++;
#else
  int m = RAND_MAX;
  long d, v = 0;
	
  do {
    d = (m > r ? r : m);		
    v += 1 + (long)(d * ((double)rand_r(seed)/((double)(m)+1.0)));
    r -= m;
  } while (r > 0);
#endif
  /* PF_STOP(0); */
  return v;
}

//===========================================================================
//=  Functions to generate Zipf (power law) distributed random variables    =
//=    - Inputs: alpha and max_val                                          =
//===========================================================================
// modified from http://www.csee.usf.edu/~christen/tools/genzipf.c

/* ZIPF related settings */
#define ZIPF_ALPHA             0.9
#define ZIPF_ARR_SIZE_MUL      16 /* pre-allocate an array with skewed vals
				  of size (rand_rage)*ZIPF_ARR_SIZE_MUL */
#define ZIPF_ARR_SIZE_MUL_MIN  3
#define ZIPF_STATS             0


#if ZIPF_STATS == 1
#  define ZIPF_STATS_DO(x) x
#else
#  define ZIPF_STATS_DO(x)
#endif

struct zipf_arr
{
  size_t size;
  int max;
  int i;
  ZIPF_STATS_DO(int* stats);
  int vals[0];
};

extern __thread double __zipf_norm_constant;
extern __thread int __zipf_initialized;
extern __thread unsigned long* __zipf_seeds;
extern __thread struct zipf_arr* __zipf_arr;
#define ZIPF_RAND_DECLARATIONS()		\
  __thread double __zipf_norm_constant = 0;	\
  __thread int __zipf_initialized = 0;		\
  __thread unsigned long* __zipf_seeds = NULL;	\
  __thread struct zipf_arr* __zipf_arr;
  
#define likely(x)       __builtin_expect((x), 1)
#define unlikely(x)     __builtin_expect((x), 0)

static inline double
zipf_init(const double alpha, const int max)
{
  if (unlikely(__zipf_initialized == 0))
    {
      __zipf_seeds = seed_rand();

      int i;
      for (i=1; i <= max; i++)
	{
	  __zipf_norm_constant = __zipf_norm_constant + (1.0 / pow((double) i, alpha));
	}
      __zipf_norm_constant = 1.0 / __zipf_norm_constant;
      __zipf_initialized = 1;
    }
  return __zipf_norm_constant;
}


static inline int 
zipf(double alpha, const int max)
{
  static double c = 0;          // Normalization constant
  double z;                     // Uniform random number (0 < z < 1)
  double sum_prob;              // Sum of probabilities
  double zipf_value = 0;        // Computed exponential value to be returned

  // Compute normalization constant on first call only
  c = zipf_init(alpha, max);

  // Pull a uniform random number (0 < z < 1)
  do
    {
      z = my_random(&(__zipf_seeds[0]),&(__zipf_seeds[1]),&(__zipf_seeds[2])) / (double) ((unsigned long) (-1));
    }
  while ((z == 0) || (z == 1));

  // Map z to the value
  sum_prob = 0;
  int i;
  for (i = 1; i <= max; i++)
    {
      sum_prob = sum_prob + c / pow((double) i, alpha);
      if (sum_prob >= z)
	{
	  zipf_value = i;
	  break;
	}
    }

  // Assert that zipf_value is between 1 and N
  /* assert((zipf_value >=1) && (zipf_value <= max)); */

  return (zipf_value - 1);
}

/* Create and return an array of num_vals zipf random values.
   @param num_vals: if num_vals == 0: automatically infers the num_vals to allocate
 */
static inline struct zipf_arr*
zipf_get_rand_array(double zipf_alpha,
		    size_t num_vals,
		    const int max,
		    const int id)
{
  if (num_vals == 0)
    {
      int log2 = log2f((double) max);
      int multi = ZIPF_ARR_SIZE_MUL - log2;
      if (multi < ZIPF_ARR_SIZE_MUL_MIN)
	{
	  multi = ZIPF_ARR_SIZE_MUL_MIN;
	}
      num_vals = multi * max;
    }


  struct zipf_arr* za = malloc(sizeof(struct zipf_arr) + num_vals * sizeof(int));
  assert(za != NULL);
  za->size = num_vals;
  za->max = max;
  za->i = 0;
  
  char fname[128];
  sprintf(fname, "data/zipf_rand_%d_%zu_%d.dat", id, num_vals, max);
  //  printf("--- %s\n", fname);
  FILE* rand_file = fopen(fname, "r");
  if (rand_file == NULL)
    {
      printf("--- [%-2d] Creating rand file with %zu vals\n", id, num_vals);
      rand_file = fopen(fname, "w+");
      int file_ok = (rand_file != NULL);
      int i;
      for (i = 0; i < num_vals; i++)
	{
	  za->vals[i] = zipf(zipf_alpha, max);
	  if (file_ok)
	    {
	      fprintf(rand_file, "%d\n", za->vals[i]);
	    }
	}
    }
  else
    {
      int i;
      for (i = 0; i < num_vals; i++)
	{
	  int val;
	  __attribute__((unused)) int ret = fscanf(rand_file, "%d", &val);
	  za->vals[i] = val;
	}      
    }

  if (rand_file != NULL)
    {
      fclose(rand_file);
    }

#if ZIPF_STATS == 1
  za->stats = calloc(max, sizeof(int));
  assert(za->stats != NULL);
#endif

  return za;
}

static inline int
zipf_get_next(struct zipf_arr* za)
{
  return za->vals[(za->i++) % za->size];
}

static inline void
zipf_print_stats(struct zipf_arr* za)
{
#if ZIPF_STATS == 1
  int i;
  for (i = 0; i < za->max; i++)
    {
      printf("%-4d : %d\n", i, za->stats[i]);
    }
#endif
}


#endif
