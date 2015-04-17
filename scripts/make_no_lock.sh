#!/bin/bash

lock=MUTEX;
testf=normal;

list=1;
ht=0;

if [ $list -eq 1 ];
then
    make -k seqll POWER=1 TEST=${testf} GC=0;

    make -k lbll_lazy POWER=1 TEST=${testf} LOCK=NONE GC=0;
    mv bin/lb-ll_lazy bin/lb-ll_lazy_no_lock;

    make -k lbll_lazy POWER=1 TEST=${testf} LOCK=${lock} GC=0;
    make -k lbll_lazy_no_ro POWER=1 TEST=${testf} LOCK=${lock} GC=0;

    make -k lbll_pugh POWER=1 TEST=${testf} LOCK=NONE GC=0;
    mv bin/lb-ll_pugh bin/lb-ll_pugh_no_lock;

    make -k lbll_pugh POWER=1 TEST=${testf} LOCK=${lock} GC=0;
fi;
##
#make -k lbll_lazy POWER=1 TEST=${testf} LOCK=PROF GC=0;

if [ $ht -eq 1 ];
then
    # make -k seqht POWER=1 TEST=${testf} GC=0;

    # make -k lbht_lazy_gl POWER=1 TEST=${testf} LOCK=NONE GC=0;
    # mv bin/lb-ht_lazy_gl bin/lb-ht_lazy_gl_no_lock;

    # make -k lbht_lazy_gl POWER=1 TEST=${testf} LOCK=${lock} GC=0;
    # make -k lbht_lazy_gl_no_ro POWER=1 TEST=${testf} LOCK=${lock} GC=0;

    make -k htjava POWER=1 TEST=${testf} LOCK=NONE GC=0;
    mv bin/lb-ht_java bin/lb-ht_java_no_lock;

    # make -k htjava POWER=1 TEST=${testf} LOCK=${lock} GC=0;
    make -k htjava_no_ro POWER=1 TEST=${testf} LOCK=${lock} GC=0;
fi;
