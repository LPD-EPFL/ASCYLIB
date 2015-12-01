/*   
 *   File: set.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   set.h is part of ASCYLIB
 *
 * Copyright (c) 2015 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
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

#ifndef _H_LAZY_
#define _H_LAZY_
#include "set-lock.h"


#define LAZY_RO_FAIL RO_FAIL


sval_t optik_map_contains(map_t* map, skey_t key);
int optik_map_insert(map_t* map, skey_t key, sval_t val);
sval_t optik_map_remove(map_t* map, skey_t key);

#endif	/* _H_LAZY_ */
