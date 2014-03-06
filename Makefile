.PHONY:	all

BENCHS = src/sftree src/linkedlist src/hashtable src/hashtable-rcu src/hashtable-java src/hashtable-tbb src/skiplist src/rbtree src/deque src/bst src/bst-howley src/noise/
LBENCHS = src/linkedlist-lock src/hashtable-lock src/hashtable-tbb src/hashtable-java src/skiplist-lock src/bst-lock2
LFBENCHS = src/linkedlist src/hashtable src/hashtable-rcu src/skiplist src/bst src/bst-howley
NOISE = src/noise

.PHONY:	clean all $(BENCHS) $(LBENCHS) $(NOISE)

all:	lockfree tas lbhtgl

mutex:
	$(MAKE) "LOCK=MUTEX" $(LBENCHS)

spin:
	$(MAKE) "LOCK=SPIN" $(LBENCHS)

tas:
	$(MAKE) "LOCK=TAS" $(LBENCHS)

ticket:
	$(MAKE) "LOCK=TICKET" $(LBENCHS)

hticket:
	$(MAKE) "LOCK=HTICKET" $(LBENCHS)

clh:
	$(MAKE) "LOCK=CLH" $(LBENCHS)

sequential:
	$(MAKE) "STM=SEQUENTIAL" $(BENCHS)

lockfree:
	$(MAKE) "STM=LOCKFREE" $(LFBENCHS)

noise:
	$(MAKE) $(NOISE)

tbb:
	$(MAKE) "LOCK=TAS" src/hashtable-tbb

lfsl:
	$(MAKE) "STM=LOCKFREE" src/skiplist

lfll:
	$(MAKE) "STM=LOCKFREE" src/linkedlist

lbll:
	$(MAKE) "LOCK=TAS" src/linkedlist-lock

lfht:
	$(MAKE) "STM=LOCKFREE" src/hashtable

lbht:
	$(MAKE) "LOCK=TAS" src/hashtable-lock

lbsl:
	$(MAKE) "LOCK=TAS" src/skiplist-lock

htjava:
	$(MAKE) "LOCK=TAS" src/hashtable-java

lbhtgl:
	$(MAKE) "LOCK=TAS" "G=GL" src/hashtable-lock


clean:
	$(MAKE) -C src/linkedlist clean	
	$(MAKE) -C src/skiplist clean
	$(MAKE) -C src/hashtable clean
	$(MAKE) -C src/hashtable-rcu clean
	$(MAKE) -C src/rbtree clean
	$(MAKE) -C src/linkedlist-lock clean
	$(MAKE) -C src/hashtable-lock clean
	$(MAKE) -C src/skiplist-lock clean
	$(MAKE) -C src/sftree clean
	$(MAKE) -C src/bst clean
	$(MAKE) -C src/bst-howley clean
	$(MAKE) -C src/bst-lock2 clean
	$(MAKE) -C src/deque clean
	$(MAKE) -C src/noise clean
	rm -rf build
#	$(MAKE) -C rbtree-boosted clean

# $(BENCHS):
# 	$(MAKE) -C $@ $(TARGET)

$(LBENCHS):
	$(MAKE) -C $@ $(TARGET)

$(LFBENCHS):
	$(MAKE) -C $@ $(TARGET)

$(NOISE):
	$(MAKE) -C $@ $(TARGET)
