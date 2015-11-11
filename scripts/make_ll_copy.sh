#!/bin/bash

gc=1;
cpu=0;
power=0;
ro_fail=1;

locks="TAS TTAS TICKET MCS MUTEX"

ds=bin/lb-ll_copy;

for l in $locks;
do
    make llcopy POWER=$power SET_CPU=$cpu GC=$gc RO_FAIL=$ro_fail LOCK=$l;
    mv $ds ${ds}_${l};
done
