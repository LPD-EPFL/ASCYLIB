#!/bin/bash

gc=1;
cpu=1;
power=1;
ro_fail=1;

locks="NONE TAS TTAS TICKET CLH MUTEX UPMUTEX4"

ds=bin/lb-ll_copy;

for l in $locks;
do
    make llcopy POWER=$power SET_CPU=$cpu GC=$gc RO_FAIL=$ro_fail LOCK=$l;
    mv $ds ${ds}_${l};
done
