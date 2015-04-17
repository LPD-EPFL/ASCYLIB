#!/bin/bash

gc=1;
cpu=1;


make lbht_coupling_gl POWER=1 SET_CPU=$cpu GC=${gc} LOCK=MUTEX
mv bin/lb-ht_coupling_gl bin/lb-ht_coupling_gl_mutex

make lbht_coupling_gl POWER=1 SET_CPU=$cpu GC=${gc} LOCK=TAS
mv bin/lb-ht_coupling_gl bin/lb-ht_coupling_gl_tas

make lbht_coupling_gl POWER=1 SET_CPU=$cpu GC=${gc} LOCK=TTAS
mv bin/lb-ht_coupling_gl bin/lb-ht_coupling_gl_ttas

make lbht_coupling_gl POWER=1 SET_CPU=$cpu GC=${gc} LOCK=TICKET
mv bin/lb-ht_coupling_gl bin/lb-ht_coupling_gl_ticket

make lbht_coupling_gl POWER=1 SET_CPU=$cpu GC=0 LOCK=NONE
mv bin/lb-ht_coupling_gl bin/lb-ht_coupling_gl_no_lock
