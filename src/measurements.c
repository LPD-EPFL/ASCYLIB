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

#ifdef DO_TIMINGS
#  if !defined(PLATFORM_MCORE_SSMP)
__thread ticks entry_time[ENTRY_TIMES_SIZE];
__thread enum timings_bool_t entry_time_valid[ENTRY_TIMES_SIZE] = {M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE, M_FALSE};
__thread ticks total_sum_ticks[ENTRY_TIMES_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__thread long long total_samples[ENTRY_TIMES_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__thread const char *measurement_msgs[ENTRY_TIMES_SIZE];
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
