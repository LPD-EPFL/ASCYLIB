/* this file was taken from TM2C (https://github.com/trigonak/tm2c) */

/*
 *   File: measurements.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: a simple profiler
 *   This file is part of TM2C
 *
 *   Copyright (C) 2013  Vasileios Trigonakis
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#define EXINLINED
#include "measurements.h"
#include "latency.h"

__thread ticks getticks_correction = 0;
ticks getticks_correction_calc() 
{
#    define GETTICKS_CALC_REPS 1000000
  ticks t_dur = 0;
  uint32_t i;
  for (i = 0; i < GETTICKS_CALC_REPS; i++) {
    ticks t_start = getticks();
    ticks t_end = getticks();
    t_dur += t_end - t_start;
  }
  getticks_correction = (ticks)(t_dur / (double) GETTICKS_CALC_REPS);
  /* printf("(cor: %llu)", (unsigned long long int) getticks_correction); */
  return getticks_correction;
}

#ifdef DO_TIMINGS
#  if !defined(PLATFORM_MCORE_SSMP)
__thread ticks entry_time[ENTRY_TIMES_SIZE];
__thread enum timings_bool_t entry_time_valid[ENTRY_TIMES_SIZE] = {M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE};
__thread ticks total_sum_ticks[ENTRY_TIMES_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__thread long long total_samples[ENTRY_TIMES_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__thread const char *measurement_msgs[ENTRY_TIMES_SIZE];

#    if defined(DO_TIMINGS_TICKS) || defined(DO_TIMINGS_TICKS_SIMPLE)
void
prints_ticks_stats(int start, int end)
{
  int32_t i, mpoints = 0, have_output = 0;
  unsigned long long tsamples = 0;
  ticks tticks = 0;

  for (i = start; i < end; i++) 
    {
      if (total_samples[i]) 
	{
	  have_output = 1;
	  mpoints++;
	  tsamples += total_samples[i];
	  tticks += total_sum_ticks[i];
	}
    }
  
  if (have_output)
    {
      printf("(PROFILING) >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    }

  for (i = start; i < end; i++) 
    {
      if (total_samples[i] && total_sum_ticks[i]) 
	{
	  if (measurement_msgs[i] == NULL)
	    {
	      measurement_msgs[i] = "null";
	    }
	  printf("[%02d]%s:\n", i, measurement_msgs[i]);
	  double ticks_perc = 100 * ((double) total_sum_ticks[i] / tticks);
	  double secs = total_sum_ticks[i] / (REF_SPEED_GHZ * 1.e9);
	  int s = (int) trunc(secs);
	  int ms = (int) trunc((secs - s) * 1000);
	  int us = (int) trunc(((secs - s) * 1000000) - (ms * 1000));
	  int ns = (int) trunc(((secs - s) * 1000000000) - (ms * 1000000) - (us * 1000));
	  double secsa = (total_sum_ticks[i] / total_samples[i]) / (REF_SPEED_GHZ * 1.e9);
	  int sa = (int) trunc(secsa);
	  int msa = (int) trunc((secsa - sa) * 1000);
	  int usa = (int) trunc(((secsa - sa) * 1000000) - (msa * 1000));
	  int nsa = (int) trunc(((secsa - sa) * 1000000000) - (msa * 1000000) - (usa * 1000));
	  printf(" [%4.1f%%] samples: %-12llu | time: %3d %3d %3d %3d | avg: %3d %3d %3d %3d | ticks: %.1f\n",
		 ticks_perc, total_samples[i],
		 s, ms, us, ns,
		 sa, msa, usa, nsa,
		 (double) total_sum_ticks[i]/total_samples[i]);
	}
    }
  if (have_output)
    {
      /* printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< (PROFILING)\n"); */
      fflush(stdout);
    }
}
#    endif	/* DO_TIMINGS_TICK[_SIMPLE]?*/
#  endif  /* !SSMP */
#endif	/* DO_TIMINGS */

#if ECDF_CALC == 1
__thread ticks** __lat_op;
ticks** __lat_op_all[1024];

#include <math.h>

static double
sqr(double v)
{
  return v * v;
}

static int
ecdf_comp(const void *elem1, const void *elem2) 
{
  size_t f = *((size_t*)elem1);
  size_t s = *((size_t*)elem2);
  if (f > s) return  1;
  if (f < s) return -1;
  return 0;
}


ecdf_t*
ecdf_calc(const size_t* vals, const size_t val_n)
{
  ecdf_t* e = (ecdf_t*) calloc(1, sizeof(ecdf_t));
  assert(e != NULL);

  size_t* vals_sorted = (size_t*) malloc(val_n * sizeof(size_t));
  assert(vals_sorted != NULL);

  e->vals_sorted = vals_sorted;

  memcpy(vals_sorted, vals, val_n * sizeof(size_t));
  qsort(vals_sorted, val_n, sizeof(size_t), ecdf_comp);

  ecdf_pair_t* ps = (ecdf_pair_t*) malloc(val_n * sizeof(ecdf_pair_t));
  assert(ps != NULL);

  size_t ps_n = 0;

  size_t cur = vals_sorted[0];

  int i;
  for (i = 1; i < val_n; i++)
    {
      if (cur != vals_sorted[i])
	{
	  ps[ps_n].x = cur;
	  ps[ps_n].cdf = ((double) i / val_n) * 100.0;
	  ps_n++;
	  cur = vals_sorted[i];
	}
    }

  ps[ps_n].x = cur;
  ps[ps_n].cdf = 100.0;
  ps_n++;

  e->val_n = val_n;
  e->pair_n = ps_n;
  e->pairs = (ecdf_pair_t*) malloc(ps_n * sizeof(ecdf_pair_t));
  assert(e->pairs != NULL);

  for (i = 0; i < e->pair_n; i++)
    {
      e->pairs[i].x = ps[i].x;
      e->pairs[i].cdf = ps[i].cdf;
    }

  free(ps);
  return e;
}

ecdf_t*
ecdf_calc_plus(const size_t* vals, const size_t val_n)
{
  ecdf_t* e = (ecdf_t*) calloc(1, sizeof(ecdf_t));
  assert(e != NULL);

  size_t* vals_sorted = (size_t*) malloc(val_n * sizeof(size_t));
  assert(vals_sorted != NULL);

  memcpy(vals_sorted, vals, val_n * sizeof(size_t));
  qsort(vals_sorted, val_n, sizeof(size_t), ecdf_comp);

  ecdf_pair_t* ps = (ecdf_pair_t*) malloc(val_n * sizeof(ecdf_pair_t));
  assert(ps != NULL);

  size_t ps_n = 0;
  size_t csum = 0;
  size_t cur = vals_sorted[0];

  int i;
  for (i = 1; i < val_n; i++)
    {
      size_t cv = vals_sorted[i];
      csum += cv;

      if (cur != cv)
	{
	  ps[ps_n].x = cur;
	  ps[ps_n].cdf = ((double) i / val_n) * 100.0;
	  ps_n++;
	  cur = vals_sorted[i];
	}
    }

  double mean = (double) csum / val_n;
  e->plus.avg = mean;

  size_t stdsum = 0;
  for (i = 1; i < val_n; i++)
    {
      size_t cv = vals_sorted[i];
      stdsum += sqr(cv - mean);
    }

  double std = sqrt(stdsum / val_n);
  double stdp = 100 * (1 - ((mean - std) / mean));
  e->plus.stdev = std;
  e->plus.stdevp = stdp;


  ps[ps_n].x = cur;
  ps[ps_n].cdf = 100.0;
  ps_n++;

  e->val_n = val_n;
  e->pair_n = ps_n;
  e->pairs = (ecdf_pair_t*) malloc(ps_n * sizeof(ecdf_pair_t));
  assert(e->pairs != NULL);

  for (i = 0; i < e->pair_n; i++)
    {
      e->pairs[i].x = ps[i].x;
      e->pairs[i].cdf = ps[i].cdf;
    }

  free(ps);
  free(vals_sorted);
  return e;
}


void
ecdf_print(const ecdf_t* e)
{
  int cdf_cur = 0;
  int p;
  for (p = 0; p < e->pair_n; p++)
    {
      int cdf = (int) e->pairs[p].cdf;
      if (cdf > cdf_cur)
	{
	  printf("%-5d : %-5zu -> %f%%\n", p, e->pairs[p].x, e->pairs[p].cdf);
	  cdf_cur = cdf;
	}
    }
}

void
ecdf_print_boxplot(const ecdf_t* e, const double perc, const char* title)
{
  double target[5] = { 100 - perc, 25, 50, 75, perc };
  int target_cur = 0;

  printf("#ECDF  ");
  printf("%20s ", "min");
  int p;
  for (p = 0; p < 5; p++)
    {
      printf("%9.1f%% ", target[p]);
    }

  printf("%10s ", "max");
  printf("\n#ECDF-%-10s ", title);

  printf("%10zu ", e->pairs[0].x);
  for (p = 0; p < e->pair_n && target_cur < 5; p++)
    {
      double cdf = e->pairs[p].cdf;
      if (cdf >= target[target_cur])
	{
	  target_cur++;
	  printf("%10zu ", e->pairs[p].x);
	}
    }
  printf("%10zu \n", e->pairs[e->pair_n - 1].x);
}

void
ecdf_print_boxplot_limits(const ecdf_t* e, const double* limits, const char* title)
{
  const double* target = limits;

  int target_cur = 0;
  printf("#ECDF  %10s", "");
  int p;
  for (p = 0; p < ECDF_BOXPLOT_VALS; p++)
    {
      printf("%9.1f%% ", target[p]);
    }

  printf("\n#ECDF-%-10s ", title);

  for (p = 0; p < e->pair_n && target_cur < ECDF_BOXPLOT_VALS; p++)
    {
      double cdf = e->pairs[p].cdf;
      if (cdf >= target[target_cur])
	{
	  target_cur++;
	  printf("%10zu ", e->pairs[p].x);
	  p--;
	}
    }
  printf("\n");
  /* printf("#ECDF-%-10s ", title); */
  /* for (p = 0; p < ECDF_BOXPLOT_VALS; p++) */
  /*   { */
  /*     int ei = e->val_n * (target[p] / 100.0); */
  /*     printf("%10zu ", e->vals_sorted[ei]); */
  /*   } */
  /* printf("\n"); */

  
}

void
ecdf_boxplot_get(ecdf_boxplot_t* b, const ecdf_t* e, const double perc)
{
  double target[5] = { 100 - perc, 25, 50, 75, perc };
  int target_cur = 0;

  b->confidence = perc;
  b->values[target_cur] =  e->pairs[0].x;
  int p;
  for (p = 0; p < e->pair_n && target_cur < 5; p++)
    {
      double cdf = e->pairs[p].cdf;
      if (cdf >= target[target_cur])
	{
	  target_cur++;
	  b->values[target_cur] = e->pairs[p].x;
	  p--;
	}
    }
  b->values[ECDF_BOXPLOT_VALS - 1] = e->pairs[e->pair_n - 1].x;
}

size_t
ecdf_boxplot_get_median(ecdf_boxplot_t* b)
{
  return b->values[(ECDF_BOXPLOT_VALS / 2)];
}

size_t
ecdf_boxplot_get_min(ecdf_boxplot_t* b)
{
  return b->values[0];
}

void
ecdf_boxplot_diff(ecdf_boxplot_t* d, const ecdf_boxplot_t* a, const ecdf_boxplot_t* b)
{
  assert(a->confidence == b->confidence);
  d->confidence = a->confidence;
  int p;
  for (p = 0; p < ECDF_BOXPLOT_VALS; p++)
    {
      d->values[p] = b->values[p] - a->values[p];
    }
}

void
ecdf_boxplot_minus(ecdf_boxplot_t* d, const size_t minus)
{
  int p;
  for (p = 0; p < ECDF_BOXPLOT_VALS; p++)
    {
      if (d->values[p] >= minus)
	{
	  d->values[p] -= minus;
	}
      else
	{
	  d->values[p] = 0;
	}
    }
}

void
ecdf_boxplot_print(const ecdf_boxplot_t* b, const char* title)
{
  double target[5] = { 100 - b->confidence, 25, 50, 75, b->confidence };

  printf("#ECDF  ");
  printf("%20s ", "min");
  int p;
  for (p = 0; p < 5; p++)
    {
      printf("%9.1f%% ", target[p]);
    }

  printf("%10s ", "max");
  printf("\n#ECDF-%-10s ", title);

  for (p = 0; p < ECDF_BOXPLOT_VALS; p++)
    {
      printf("%10zu ", b->values[p]);
    }
  printf("\n");
}

void
ecdf_print_avg(const ecdf_t* e)
{
  printf("#AVG         %-10s %-10s %-10s\n", "mean", "stdev", "stdev%");
  printf("#AVGv        %-10.0f %-10.1f %-10.3f\n", e->plus.avg, e->plus.stdev, e->plus.stdevp);
}

void
ecdf_print_star_at(const int at)
{
  int x;
  for (x = 0; x < at; x++)
    {
      printf(" ");
    }
  printf("*\n");
}

void
ecdf_plot(const ecdf_t* e)
{
  const int x_step = 2;
  
  int x;
  for (x = 0; x <= 58; x++)
    {
      printf("_");
    }  
  printf("\n");
  for (x = 0; x <= 27; x++)
    {
      printf(" ");
    }
  printf("ECDF\n");
  printf("      ");
  for (x = 0; x <= 100; x += 10*x_step)
    {
      printf("%-10d", x);
    }
  printf("\n");
  for (x = 0; x <= 58; x++)
    {
      printf("-");
    }  
  printf("\n");

  size_t y_max = e->pairs[e->pair_n - 1].x;
  const double y_step = y_max / 25.0;

  int x_prev = 0;
  double y_cur = 0;

  int i;
  for (i = 0; i < e->pair_n; i++)
    {
      ecdf_pair_t* p = e->pairs + i;
      while (p->x > y_cur)
	{
	  y_cur += y_step;
	  printf("     |");
	  ecdf_print_star_at(x_prev);
	}
      printf("%4zu |", p->x);
      
      x_prev = (int)(p->cdf / x_step);
      ecdf_print_star_at(x_prev);
    }

  for (x = 0; x <= 58; x++)
    {
      printf("_");
    }  
  printf("\n");
}

ecdf_clustering_t* 
ecdf_cluster(const ecdf_t* e, const int clust_offset)
{
  printf(" **** cluster offset: %d\n", clust_offset);
  ecdf_clustering_t* c = (ecdf_clustering_t*) malloc(sizeof(ecdf_clustering_t));
  assert(c != NULL);

  ecdf_clust_t clusts[e->pair_n];
  int clust_n = 0;

  size_t x_cur = e->pairs[0].x;
  size_t x_first = e->pairs[0].x;

  int i;
  for (i = 1; i < e->pair_n; i++)
    {
      if ((e->pairs[i].x - x_cur) > clust_offset)
	{
	  printf(" ** Cluster group %2d: %-4zu - %-4zu\n", clust_n, x_first, e->pairs[i - 1].x);
	  clusts[clust_n].x_min = x_first;
	  clusts[clust_n].x_max = e->pairs[i - 1].x;

	  x_first = e->pairs[i].x;
	  clust_n++;
	}
      x_cur = e->pairs[i].x;
    }
  
  printf(" ** Cluster group %2d: %-4zu - %-4zu\n", clust_n, x_first, e->pairs[i - 1].x);
  clusts[clust_n].x_min = x_first;
  clusts[clust_n].x_max = e->pairs[i - 1].x;
  clust_n++;

  c->clust_n = clust_n;
  c->clusts = (ecdf_clust_t*) malloc(clust_n * sizeof(ecdf_clust_t));
  assert(c->clusts != NULL);

  for (i = 0; i < clust_n; i++)
    {
      c->clusts[i].x_min = clusts[i].x_min;
      c->clusts[i].x_max = clusts[i].x_max;
    }
  

  return c;
}

size_t
ecdf_clust_get_val(const ecdf_clustering_t* c, const size_t val)
{
  int i = 0;
  while (val > c->clusts[i].x_max)
    {
      i++;
    }

  return c->clusts[i].x_min;
}


void
ecdf_destroy(ecdf_t* e)
{
  free(e->pairs);
  free(e);
}

void
ecdf_clustering_destroy(ecdf_clustering_t* c)
{
  free(c->clusts);
  free(c);
}


#endif
