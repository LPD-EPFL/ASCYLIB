#include <iostream>
#include <fstream>
#include <stdint.h>
#include <setjmp.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include "/home/an/atomic_ops/include/atomic_ops.h"

// Most of these macros are not used in this algorithm
#define WINDOW_SIZE 10

#define MAX_PROCESSES 63 
#define PROCESS_BITS 6 
 
#define DELETE_WINDOW_SIZE 5 

#define PTRNODE_RESERVED_BITS 58
#define DATANODE_RESERVED_BITS 55
#define HELPING_THRESHOLD 1000

#define PREGEN_FACTOR 0.75
#define MAX_SLEEP_TIME 100
#define WAITFREE_THRESHOLD 1000
#define FAST_UPDATE_THRESHOLD 1000 // used in phase 3 of update algorithm

#define RECYCLED_VECTOR_RESERVE 5000000

#define GC_NODE_THRESHOLD 1000000

#define MOST_RECENT_COUNT 4

#define MAX_INSERT_DEPTH 9

#define CACHE_LINE_SIZE		64

#define NODE_SIZE 47
#define WORD_SIZE 64

#define WORD_BITS 5
#define KEY_BITS 29

#include <pthread.h>


#define MARK_BIT 1
#define FLAG_BIT 0


enum{INS,DEL};

enum {BLACK,RED};

enum {UNMARK,MARK};

enum {UNFLAG,FLAG};

typedef uintptr_t Word;
const unsigned WORD_RESERVED_BITS = 2;






typedef struct node{
	
	
	
	
	int key;
	AO_double_t child;
	
	
	#ifdef UPDATE_VAL
		long value;
	#endif
	
} node_t;





typedef struct seekRecord{

// SeekRecord structure

//node_t * leaf;
long leafKey;

node_t * parent;
AO_t pL;
bool isLeftL; // is L the left child of P?


node_t * lum;
AO_t lumC;
bool isLeftUM; // is  last unmarked node's child on access path the left child of  the last unmarked node?
//char padding[ CACHE_LINE_SIZE ];

} seekRecord_t;




typedef struct barrier {
  pthread_cond_t complete;
  pthread_mutex_t mutex;
  int count;
  int crossing;
} barrier_t;


typedef struct thread_data {
  //sigjmp_buf env;
  int id;
  unsigned long numThreads;
  unsigned long numInsert;
//  long numDelete;
//  unsigned long numActualInsert;
  unsigned long numActualDelete;
  unsigned long ops;
  unsigned int seed;
  double search_frac;
  double insert_frac;
  double delete_frac;
  long keyspace1_size;
 // long keyspace2_size;

 
  node_t* rootOfTree;
  barrier_t *barrier;
  barrier_t *barrier2;
  
  
  //int helpCount;
//  AO_t lastWord2;
 #ifdef DETAILED_STATS	
    double tot_read_time; 
  
   double tot_fastins_time;
  double tot_slowins_time;
  long tot_fastins_count;
  long tot_slowins_count;
  
  double tot_fastdel_time;
  double tot_slowdel_time;
  long tot_fastdel_count;
  long tot_slowdel_count;
    
  long tot_reads;
  long insertSeqNumber;
  
  long count;
  #endif
  
  std::vector<node_t *> recycledNodes;
//  std::vector<AO_t> recycledDataNodes;
  
  
 seekRecord_t * sr; // seek record
 seekRecord_t * ssr; // secondary seek record

} thread_data_t;


// Helping function declaration
int perform_one_delete_window_operation(thread_data_t* data, seekRecord_t * R, long key);

int perform_one_insert_window_operation(thread_data_t* data, seekRecord_t * R, long newKey);





/* ################################################################### *
 * Mapping Definitions
 * ################################################################### */



inline bool SetBit(volatile unsigned long *array, int bit) {

   // asm("bts %1,%0" :  "+m" (*array): "r" (bit));
     bool flag; 
     __asm__ __volatile__("lock bts %2,%1; setb %0" : "=q" (flag) : "m" (*array), "r" (bit)); return flag; 
   return flag;
}

bool mark_Node(volatile AO_t * word){
	return (SetBit(word, MARK_BIT));
}

//#define atomic_cas_full(addr, old_val, new_val) (AO_compare_and_swap_full((volatile AO_t *)(addr), (AO_t)(old_val), (AO_t)(new_val)))
#define atomic_cas_full(addr, old_val, new_val) __sync_bool_compare_and_swap(addr, old_val, new_val);


//-------------------------------------------------------------
#define create_child_word(addr, mark, flag) (((uintptr_t) addr << 2) + (mark << 1) + (flag))

#define is_marked(x) ( ((x >> 1) & 1)  == 1 ? true:false)

#define is_flagged(x) ( (x & 1 )  == 1 ? true:false)





#define get_addr(x) (x >> 2)



//#define extract_pid_from_ptrNode_field2(key) ((key & (MAX_PROCESSES << 4)) >> 4)

#define add_mark_bit(x) (x + 4UL)





#define is_free(x) (((x) & 3) == 0? true:false)



//-------------------------------------------------------------







