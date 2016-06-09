.PHONY:	all

BENCHS = src/bst-aravind src/bst-bronson src/bst-drachsler src/bst-ellen src/bst-howley src/bst-seq_internal src/bst-tk src/hashtable-copy src/hashtable-coupling src/hashtable-harris src/hashtable-java src/hashtable-java_optik src/hashtable-lazy src/hashtable-optik0 src/hashtable-optik1 src/hashtable-map_optik src/hashtable-pugh src/hashtable-rcu src/hashtable-seq src/hashtable-tbb  src/linkedlist-copy src/linkedlist-coupling src/linkedlist-gl_opt src/linkedlist-harris src/linkedlist-harris_opt src/linkedlist-lazy src/linkedlist-lazy_sp src/linkedlist-lazy_orig src/linkedlist-lazy_cache src/linkedlist-optik src/linkedlist-optik_gl src/linkedlist-optik_cache src/linkedlist-michael src/linkedlist-pugh src/linkedlist-seq src/noise src/skiplist-fraser src/skiplist-herlihy_lb src/skiplist-optik src/skiplist-optik1 src/skiplist-optik2 src/skiplist-herlihy_lf src/skiplist-pugh src/skiplist-seq src/priorityqueue-alistarh src/priorityqueue-lotanshavit_lf src/priorityqueue-alistarh-herlihyBased src/priorityqueue-alistarh-pughBased src/queue-ms_lb src/queue-ms_hybrid src/queue-ms_lf src/queue-optik0 src/queue-optik1 src/queue-optik2 src/queue-optik2a src/queue-optik3 src/queue-optik4 src/queue-optik5 src/stack-treiber src/stack-lock src/stack-optik src/stack-optik1 src/stack-optik2 src/map-lock src/map-optik
LBENCHS = src/linkedlist-coupling src/linkedlist-gl_opt src/linkedlist-lazy src/linkedlist-lazy_sp src/linkedlist-lazy_orig src/linkedlist-lazy_cache src/linkedlist-optik src/linkedlist-optik_gl src/linkedlist-optik_cache src/linkedlist-pugh src/linkedlist-copy src/hashtable-pugh src/hashtable-coupling src/hashtable-lazy src/hashtable-optik0 src/hashtable-optik1 src/hashtable-map_optik src/hashtable-java src/hashtable-java_optik src/hashtable-copy src/skiplist-herlihy_lb src/skiplist-optik src/skiplist-optik1 src/skiplist-optik2 src/skiplist-pugh src/bst-bronson src/bst-drachsler src/bst-tk/ src/priorityqueue-alistarh-pughBased src/queue-ms_lb src/queue-ms_hybrid src/queue-optik0 src/queue-optik1 src/queue-optik2 src/queue-optik2a src/queue-optik3 src/queue-optik4 src/queue-optik5 src/stack-lock src/stack-optik src/stack-optik1 src/stack-optik2 src/map-lock src/map-optik
LFBENCHS = src/linkedlist-harris src/linkedlist-harris_opt src/linkedlist-michael src/hashtable-harris src/skiplist-fraser src/skiplist-herlihy_lf src/bst-ellen src/bst-howley src/bst-aravind src/priorityqueue-alistarh src/priorityqueue-lotanshavit_lf src/priorityqueue-alistarh-herlihyBased src/queue-ms_lf src/stack-treiber
SEQBENCHS = src/linkedlist-seq src/hashtable-seq src/skiplist-seq src/bst-seq_internal src/bst-seq_external
EXTERNALS = src/hashtable-rcu src/hashtable-tbb
NOISE = src/noise
TESTS = src/tests src/optik_test
BSTS = src/bst-bronson src/bst-drachsler src/bst-ellen src/bst-howley src/bst-aravind src/bst-tk/

.PHONY:	clean all external $(BENCHS) $(LBENCHS) $(NOISE) $(TESTS) $(SEQBENCHS)

default: lockfree tas seq

all:	lockfree tas seq external

ppopp: mapppopp llppopp htppopp slppopp quppopp stppopp bstppopp

mutex:
	$(MAKE) "LOCK=MUTEX" $(LBENCHS)

spin:
	$(MAKE) "LOCK=SPIN" $(LBENCHS)

tas:
	$(MAKE) $(LBENCHS)

ticket:
	$(MAKE) "LOCK=TICKET" $(LBENCHS)

hticket:
	$(MAKE) "LOCK=HTICKET" $(LBENCHS)

clh:
	$(MAKE) "LOCK=CLH" $(LBENCHS)

bst:	seqbstint seqbstext
	$(MAKE) $(BSTS)

bstppopp: bst_aravind bst_bronson bst_tk 

bst_tk:
	$(MAKE) src/bst-tk/

bst_aravind:
	$(MAKE) "STM=LOCKFREE" src/bst-aravind

bst_howley:
	$(MAKE) "STM=LOCKFREE" src/bst-howley

bst_ellen:
	$(MAKE) "STM=LOCKFREE" src/bst

bst_drachsler:
	$(MAKE) src/bst-drachsler

bst_drachsler_no_ro:
	$(MAKE) "RO_FAIL=0" src/bst-drachsler

bst_bronson:
	$(MAKE) src/bst-bronson

sequential:
	$(MAKE) "STM=SEQUENTIAL" "SEQ_NO_FREE=1" $(SEQBENCHS)

seqgc:
	$(MAKE) "STM=SEQUENTIAL" $(SEQBENCHS)

seq:	sequential


seqht:
	$(MAKE) "STM=SEQUENTIAL" "SEQ_NO_FREE=1" src/hashtable-seq

seqhtgc:
	$(MAKE) "STM=SEQUENTIAL" "GC=1" src/hashtable-seq

seqsl:
	$(MAKE) "STM=SEQUENTIAL" "SEQ_NO_FREE=1" src/skiplist-seq

seqslgc:
	$(MAKE) "STM=SEQUENTIAL" "GC=1" src/skiplist-seq

lockfree:
	$(MAKE) "STM=LOCKFREE" $(LFBENCHS)

noise:
	$(MAKE) $(NOISE)

tests:
	$(MAKE) $(TESTS)


otppopp: optik_test0 optik_test1 optik_test2

optik_test0:
	$(MAKE) "OPTIK=0" "OPTIK_STATS=1" src/optik_test

optik_test1:
	$(MAKE) "OPTIK=1" "OPTIK_STATS=1" src/optik_test

optik_test2:
	$(MAKE) "OPTIK=2" "OPTIK_STATS=1" src/optik_test

tbb:
	$(MAKE) src/hashtable-tbb

lfsl:
	$(MAKE) "STM=LOCKFREE" src/skiplist

lfsl_fraser:
	$(MAKE) "STM=LOCKFREE" src/skiplist-fraser

lfsl_herlihy_lf:
	$(MAKE) "STM=LOCKFREE" src/skiplist-herlihy_lf

lbsl_pugh:
	$(MAKE) src/skiplist-pugh

lbsl_herlihy_lb:
	$(MAKE) src/skiplist-herlihy_lb

lbsl_optik:
	$(MAKE) src/skiplist-optik

lbsl_optik1:
	$(MAKE) src/skiplist-optik1

lbsl_optik2:
	$(MAKE) src/skiplist-optik2

sl:	seqsl lfsl_fraser lfsl_herlihy_lf lbsl_pugh lbsl_herlihy_lb lbsl_optik lbsl_optik1 lbsl_optik2

slppopp: lfsl_fraser lbsl_herlihy_lb lbsl_optik lbsl_optik1 lbsl_optik2


qu: lbqu_ms lfqu_ms lbqu_optik0 lbqu_optik1 lbqu_optik2 lbqu_optik2a lbqu_optik3 lbqu_optik4 lbqu_optik5

quppopp: lbqu_ms lfqu_ms lbqu_optik0 lbqu_optik1 lbqu_optik2 lbqu_optik3 

lbqu_ms:
	$(MAKE) "LOCK=MCS" src/queue-ms_lb

lbqu_ms_hybrid:
	$(MAKE) "LOCK=MCS" src/queue-ms_hybrid

lfqu_ms:
	$(MAKE) src/queue-ms_lf

lbqu_optik0:
	$(MAKE) "OPTIK=0" "LOCK=MCS" src/queue-optik0

lbqu_optik1:
	$(MAKE) "OPTIK=1" "LOCK=MCS" src/queue-optik1

lbqu_optik2:
	$(MAKE) src/queue-optik2

lbqu_optik2a:
	$(MAKE) src/queue-optik2a

lbqu_optik3:
	$(MAKE) "OPTIK=0" src/queue-optik3

lbqu_optik4:
	$(MAKE) src/queue-optik4

lbqu_optik5:
	$(MAKE) src/queue-optik5

lfst_treiber:
	$(MAKE) src/stack-treiber

lbst_lock:
	$(MAKE) "LOCK=MCS" src/stack-lock

lbst_lock_tas:
	$(MAKE) "LOCK=TAS" "G=GL" src/stack-lock

lbst_optik:
	$(MAKE) src/stack-optik

lbst_optik1:
	$(MAKE) src/stack-optik1

lbst_optik2:
	$(MAKE) src/stack-optik2

st: lfst_treiber lbst_lock lbst_optik lbst_optik1 lbst_optik2

stppopp: lfst_treiber lbst_lock lbst_lock_tas lbst_optik

lfll_harris:
	$(MAKE) "STM=LOCKFREE" src/linkedlist-harris

lfll_harris_opt:
	$(MAKE) "STM=LOCKFREE" src/linkedlist-harris_opt

lfll_michael:
	$(MAKE) "STM=LOCKFREE" src/linkedlist-michael

seqll:
	$(MAKE) "STM=SEQUENTIAL" "SEQ_NO_FREE=1" src/linkedlist-seq

seqllgc:
	$(MAKE) "STM=SEQUENTIAL" "GC=1" src/linkedlist-seq


lfll: lfll_harris lfll_michael lfll_harris_opt
lbll: seqll llcopy lbll_coupling lbll_gl lbll_pugh lbll_lazy lbll_lazy_sp lbll_lazy_orig lbll_lazy_cache lbll_lazy_no_ro lbll_optik lbll_optik_no_ro llcopy_no_ro lbll_pugh_no_ro
ll: seqll lfll llcopy lbll_coupling lbll_gl lbll_pugh lbll_lazy lbll_lazy_no_ro lbll_optik lbll_optik_no_ro llcopy_no_ro lbll_pugh_no_ro

llppopp: lfll_harris_opt lbll_lazy lbll_lazy_cache lbll_gl lbll_optik_gl lbll_optik lbll_optik_cache

optik: lbll_optik lbht_optik0 lbht_optik0_gl lbht_optik1 lbht_optik1_gl

llspaa: lbll_coupling_gl lbll_lazy_no_ro lbll_lazy lbll_lazy_sp lbll_lazy_orig lbll_gl lbll_optik_gl

lbht_coupling:
	$(MAKE) src/hashtable-coupling

lbht_pugh:
	$(MAKE) src/hashtable-pugh

lbht_lazy:
	$(MAKE) src/hashtable-lazy

lbht_optik0:
	$(MAKE) src/hashtable-optik0

lbht_optik0_gl:
	$(MAKE) "G=GL" src/hashtable-optik0

lbht_optik1:
	$(MAKE) src/hashtable-optik1

lbht_optik1_gl:
	$(MAKE) "G=GL" src/hashtable-optik1

lbht_map:
	$(MAKE) src/hashtable-map_optik


lbht_coupling_gl:
	$(MAKE) "G=GL" src/hashtable-coupling

lbht_pugh_gl:
	$(MAKE) "G=GL" src/hashtable-pugh

lbht_pugh_gl_no_ro:
	$(MAKE) "G=GL" "RO_FAIL=0" src/hashtable-pugh

lbht_lazy_gl:
	$(MAKE) "G=GL" src/hashtable-lazy

lbht_lazy_gl_no_ro:
	$(MAKE) "G=GL" "RO_FAIL=0" src/hashtable-lazy

lbll_coupling:
	$(MAKE) src/linkedlist-coupling

lbll_coupling_gl:
	$(MAKE) "G=GL" "LOCK=MCS" src/linkedlist-coupling

lbll_gl:
	$(MAKE) "LOCK=MCS" src/linkedlist-gl_opt

lbll_pugh:
	$(MAKE) src/linkedlist-pugh

lbll_pugh_no_ro:
	$(MAKE) "RO_FAIL=0" src/linkedlist-pugh

lbll_lazy:
	$(MAKE) src/linkedlist-lazy

lbll_lazy_sp:
	$(MAKE) src/linkedlist-lazy_sp

lbll_lazy_orig:
	$(MAKE) src/linkedlist-lazy_orig

lbll_lazy_no_ro:
	$(MAKE) "RO_FAIL=0" src/linkedlist-lazy

lbll_lazy_cache:
	$(MAKE) "CACHE=1" src/linkedlist-lazy_cache

lbll_optik:
	$(MAKE) src/linkedlist-optik

lbll_optik_no_ro:
	$(MAKE) "RO_FAIL=0" src/linkedlist-optik

lbll_optik_cache:
	$(MAKE) src/linkedlist-optik_cache

lbll_optik_cache_no_ro:
	$(MAKE) "RO_FAIL=0" src/linkedlist-optik_cache

lbll_optik_gl:
	$(MAKE) src/linkedlist-optik_gl

llcopy:
	$(MAKE) "LOCK=MCS" src/linkedlist-copy

llcopy_no_ro:
	$(MAKE) "LOCK=MCS" "RO_FAIL=0" src/linkedlist-copy

htcopy:
	$(MAKE) src/hashtable-copy

htcopy_no_ro:
	$(MAKE) "RO_FAIL=0" src/hashtable-copy

htcopygl:
	$(MAKE) "LOCK=CLH" "G=GL" src/hashtable-copy

lfht:
	$(MAKE) "STM=LOCKFREE" src/hashtable-harris

htjava:
	$(MAKE) src/hashtable-java

htjava_no_ro:
	$(MAKE) "RO_FAIL=0" src/hashtable-java

htjava_optik:
	$(MAKE) src/hashtable-java_optik

htrcu:
	$(MAKE) "GC=0" src/hashtable-rcu

htrcugc:
	$(MAKE) src/hashtable-rcu

ht:	seqht lfht lbht htjava htjava_optik tbb htcopy htrcu lbht_coupling lbht_lazy lbht_pugh lbht_coupling_gl lbht_lazy_gl lbht_pugh_gl lbht_lazy_gl_no_ro lbht_pugh_gl_no_ro htcopy_no_ro htjava_no_ro

htppopp: lbht_lazy_gl htjava htjava_optik lbht_optik0 lbht_optik1 lbht_map

seqbstint:
	$(MAKE) "STM=SEQUENTIAL" "SEQ_NO_FREE=1" src/bst-seq_internal

seqbstext:
	$(MAKE) "STM=SEQUENTIAL" "SEQ_NO_FREE=1" src/bst-seq_external

seqbstintgc:
	$(MAKE) "STM=SEQUENTIAL" "GC=1" src/bst-seq_internal

seqbstextgc:
	$(MAKE) "STM=SEQUENTIAL" "GC=1" src/bst-seq_external

seqbstgc: seqbstextgc seqbstintgc

external:
	$(MAKE) "STM=LOCKFREE" $(EXTERNALS)

mapppopp: lbmap_lock lbmap_optik

lbmap_lock:
	$(MAKE) "LOCK=MCS" src/map-lock

lbmap_optik:
	$(MAKE) src/map-optik

lfpq_alistarh:
	$(MAKE) "STM=LOCKFREE" src/priorityqueue-alistarh

lfpq_alistarh_herlihy:
	$(MAKE) "STM=LOCKFREE" src/priorityqueue-alistarh-herlihyBased

lbpq_alistarh_pugh:
	$(MAKE) "LOCK=TAS" src/priorityqueue-alistarh-pughBased

lfpq_lotanshavit:
	$(MAKE) "STM=LOCKFREE" src/priorityqueue-lotanshavit_lf

pq: lfpq_alistarh lfpq_alistarh_herlihy lbpq_alistarh_pugh lfpq_lotanshavit

clean:
	$(MAKE) -C src/bst-aravind clean
	$(MAKE) -C src/bst-bronson clean
	$(MAKE) -C src/bst-drachsler clean
	$(MAKE) -C src/bst-ellen clean
	$(MAKE) -C src/bst-howley clean
	$(MAKE) -C src/bst-seq_external clean
	$(MAKE) -C src/bst-seq_internal clean
	$(MAKE) -C src/bst-tk clean
	$(MAKE) -C src/hashtable-copy clean
	$(MAKE) -C src/hashtable-coupling clean
	$(MAKE) -C src/hashtable-harris clean
	$(MAKE) -C src/hashtable-java clean
	$(MAKE) -C src/hashtable-java_optik clean
	$(MAKE) -C src/hashtable-lazy clean
	$(MAKE) -C src/hashtable-optik0 clean
	$(MAKE) -C src/hashtable-optik1 clean
	$(MAKE) -C src/hashtable-map_optik clean
	$(MAKE) -C src/hashtable-pugh clean
	$(MAKE) -C src/hashtable-rcu clean
	$(MAKE) -C src/hashtable-seq clean
	$(MAKE) -C src/hashtable-tbb clean
	$(MAKE) -C src/linkedlist-copy clean
	$(MAKE) -C src/linkedlist-coupling clean
	$(MAKE) -C src/linkedlist-gl_opt clean
	$(MAKE) -C src/linkedlist-harris clean
	$(MAKE) -C src/linkedlist-harris_opt clean
	$(MAKE) -C src/linkedlist-lazy clean
	$(MAKE) -C src/linkedlist-lazy_sp clean
	$(MAKE) -C src/linkedlist-lazy_orig clean
	$(MAKE) -C src/linkedlist-lazy_cache clean
	$(MAKE) -C src/linkedlist-optik clean
	$(MAKE) -C src/linkedlist-optik_cache clean
	$(MAKE) -C src/linkedlist-michael clean
	$(MAKE) -C src/linkedlist-pugh clean
	$(MAKE) -C src/linkedlist-seq clean
	$(MAKE) -C src/noise clean
	$(MAKE) -C src/skiplist-fraser clean
	$(MAKE) -C src/skiplist-herlihy_lb clean
	$(MAKE) -C src/skiplist-optik clean
	$(MAKE) -C src/skiplist-optik1 clean
	$(MAKE) -C src/skiplist-optik2 clean
	$(MAKE) -C src/skiplist-herlihy_lf clean
	$(MAKE) -C src/skiplist-pugh clean
	$(MAKE) -C src/skiplist-seq clean
	$(MAKE) -C src/queue-ms_lb clean
	$(MAKE) -C src/queue-ms_lf clean
	$(MAKE) -C src/queue-ms_hybrid clean
	$(MAKE) -C src/queue-optik0 clean
	$(MAKE) -C src/queue-optik1 clean
	$(MAKE) -C src/queue-optik2 clean
	$(MAKE) -C src/queue-optik2a clean
	$(MAKE) -C src/queue-optik3 clean
	$(MAKE) -C src/queue-optik4 clean
	$(MAKE) -C src/queue-optik5 clean
	$(MAKE) -C src/stack-treiber clean
	$(MAKE) -C src/stack-lock clean
	$(MAKE) -C src/stack-optik clean
	$(MAKE) -C src/stack-optik1 clean
	$(MAKE) -C src/stack-optik2 clean
	$(MAKE) -C src/map-lock clean
	$(MAKE) -C src/map-optik clean
	$(MAKE) -C src/priorityqueue-alistarh clean
	$(MAKE) -C src/priorityqueue-alistarh-herlihyBased clean
	$(MAKE) -C src/priorityqueue-alistarh-pughBased clean
	$(MAKE) -C src/priorityqueue-lotanshavit_lf clean
	$(MAKE) -C src/tests clean
	rm -rf build

$(SEQBENCHS):
	$(MAKE) -C $@ $(TARGET)

$(LBENCHS):
	$(MAKE) -C $@ $(TARGET)

$(LFBENCHS):
	$(MAKE) -C $@ $(TARGET)

$(NOISE):
	$(MAKE) -C $@ $(TARGET)

$(TESTS):
	$(MAKE) -C $@ $(TARGET)

$(EXTERNALS):
	$(MAKE) -C $@ $(TARGET)

