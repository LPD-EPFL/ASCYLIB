/*
 * File:
 *   skiplist-lock.h
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Skip list implementation of an integer set
 *
 * Copyright (c) 2009-2010.
 *
 * skiplist-lock.h is part of Synchrobench
 * 
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#ifndef _SKIPLIST_LOCK_H_
#define _SKIPLIST_LOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL 1
#if SL == 1

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include "common.h"
#include "atomic_ops.h"
#include "lock_if.h"
#include "ssmem.h"

#define STRING_LENGTH						8

typedef intptr_t skey_t;
typedef intptr_t sval_t;

struct strkey
{
	char key[STRING_LENGTH];

	strkey& operator=( volatile strkey& other ) volatile {
		strncpy((char *) key, (char*) other.key, STRING_LENGTH);
		return (strkey&) *this;
	}
	strkey& operator=( const strkey& other ) const {
		strncpy((char *) key, (char*) other.key, STRING_LENGTH);
		return (strkey&) *this;
	}
	strkey() {}
	strkey(volatile strkey& other) {
x		strncpy((char *) key, (char*) other.key, STRING_LENGTH);
	}

	strkey(const char* data) {
		strncpy(key, data, STRING_LENGTH);
	}
};

typedef struct strkey strkey_t;

struct strval {
	char val[STRING_LENGTH];


	strval& operator=( volatile strval& other ) volatile {
		strncpy((char *) val, (char*) other.val, STRING_LENGTH);
		return (strval&) *this;
	}
	strval& operator=( const strval& other ) const {
		strncpy((char *) val, (char*) other.val, STRING_LENGTH);
		return (strval&) *this;
	}

	strval(volatile strval& other) {
		strncpy((char *) val, (char*) other.val, STRING_LENGTH);
	}

	strval(volatile strval* other) {
		strncpy((char *) val, (char*) other->val, STRING_LENGTH);
	}

	strval() {}
	strval(strval& other) {
		strncpy((char *) val, (char*) other.val, STRING_LENGTH);
	}

	strval(const char* data) {
		strncpy((char *) val, (char *) data, STRING_LENGTH);
	}
};

typedef struct strval strval_t;


// static inline char* STR_KEY_MAX() {
// 	char str[] = {CHAR_MIN, '\0'};
// 	return str;
// }

// inline const char* KEY_MIN() {
// 	std::string str = "";
// 	str.push_back(CHAR_MIN);
// 	return str.data();
// }

#define STR_KEY_MIN						""
#define STR_KEY_MAX						"zzz"

#define ALT_KEY_MIN						INT64_MIN
#define ALT_KEY_MAX						INT64_MAX

inline int strkey_compare_old(strkey_t k1, strkey_t k2) {

	//TODO write our own strcmp
	if ( strcmp(k1.key, k2.key) == 0) return 0;

	if ( strcmp(k1.key, STR_KEY_MIN) == 0) return -1;
	if ( strcmp(k2.key, STR_KEY_MIN) == 0 ) return +1;

	if ( strcmp(k1.key, STR_KEY_MAX) == 0 ) return +1;
	if ( strcmp(k2.key, STR_KEY_MAX) == 0 ) return -1;

	return strcmp(k1.key, k2.key);
}

inline int64_t strkey_compare(strkey_t k1, strkey_t k2) {
	return *(int64_t *)(k1.key) - *(int64_t *)(k2.key);
}

inline int64_t char8_to_int64(char c[]) {
	return *(int64_t *)c;
}

inline void int64_to_char8(char c[], int64_t v) {
	memcpy(c, &v, STRING_LENGTH);
}


extern unsigned int global_seed;
extern __thread ssmem_allocator_t* alloc;

extern unsigned int levelmax, size_pad_32;

typedef volatile struct sl_node
{
  strkey_t key;
  strval_t val; 
  int32_t toplevel;
#if !defined(LL_GLOBAL_LOCK)
  ptlock_t lock;
#endif
  volatile struct sl_node* next[1];
} sl_node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct sl_intset 
{
  sl_node_t *head;
#if defined(LL_GLOBAL_LOCK)
  volatile ptlock_t* lock;
#endif
} sl_intset_t;

int floor_log_2(unsigned int n);

/* 
 * Create a new node without setting its next fields. 
 */
sl_node_t* sl_new_simple_node(strkey_t key, strval_t val, int toplevel, int transactional);
/* 
 * Create a new node with its next field. 
 * If next=NULL, then this create a tail node. 
 */
sl_node_t *sl_new_node(strkey_t key, strval_t val, sl_node_t *next, int toplevel, int transactional);
void sl_delete_node(sl_node_t* n);
sl_intset_t* sl_set_new();
void sl_set_delete(sl_intset_t* set);
int sl_set_size(sl_intset_t* cset); 

#ifdef __cplusplus
}
#endif

#endif

#endif // _SKIPLIST_LOCK_H_
