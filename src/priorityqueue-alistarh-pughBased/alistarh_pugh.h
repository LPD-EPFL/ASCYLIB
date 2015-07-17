/*   
 *   File: alistarh_pugh.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *  	     Egeyar Bagcioglu <egeyar.bagcioglu@epfl.ch>
 *   Description: D. Alistarh, J. Kopinsky, J. Li, N. Shavit. The SprayList:
 *   A Scalable Relaxed Priority Queue. In Proceedings of the 20th ACM SIGPLAN
 *   Symposium on Principles and Practice of Parallel Programming (PPoPP 2015), 2015.
 *   alistarh_pugh.h is part of ASCYLIB
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

#include "skiplist-lock.h"

sval_t optimistic_find(sl_intset_t *set, skey_t key);
int optimistic_insert(sl_intset_t *set, skey_t key, sval_t val);
sval_t optimistic_delete(sl_intset_t *set, skey_t key);

void alistarh_init(int _num_threads, sl_intset_t* set, int padding);
sval_t alistarh_deleteMin(sl_intset_t *set);
skey_t alistarh_spray(sl_intset_t *set);
