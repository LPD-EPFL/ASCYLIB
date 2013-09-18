.PHONY:	all

BENCHS = src/sftree src/linkedlist src/hashtable src/skiplist src/rbtree src/deque src/bst
LBENCHS = src/linkedlist-lock src/hashtable-lock src/skiplist-lock
LFBENCHS = src/linkedlist src/hashtable src/skiplist src/bst 

.PHONY:	clean all $(BENCHS) $(LBENCHS)

all:	ticket

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

clean:
	$(MAKE) -C src/linkedlist clean	
	$(MAKE) -C src/skiplist clean
	$(MAKE) -C src/hashtable clean
	$(MAKE) -C src/rbtree clean
	$(MAKE) -C src/linkedlist-lock clean
	$(MAKE) -C src/hashtable-lock clean
	$(MAKE) -C src/skiplist-lock clean
	$(MAKE) -C src/sftree clean
	$(MAKE) -C src/bst clean
	$(MAKE) -C src/deque clean
	rm -rf build
#	$(MAKE) -C rbtree-boosted clean

$(BENCHS):
	$(MAKE) -C $@ $(TARGET)

$(LBENCHS):
	$(MAKE) -C $@ $(TARGET)
