#!/bin/bash

GC=0;

make lbll_lazy POWER=1 GC=${GC} LOCK=MUTEX
mv bin/lb-ll_lazy bin/lb-ll_lazy_mutex

make lbll_lazy POWER=1 GC=${GC} LOCK=TAS
mv bin/lb-ll_lazy bin/lb-ll_lazy_tas

make lbll_lazy POWER=1 GC=${GC} LOCK=TTAS
mv bin/lb-ll_lazy bin/lb-ll_lazy_ttas

make lbll_lazy POWER=1 GC=${GC} LOCK=TICKET
mv bin/lb-ll_lazy bin/lb-ll_lazy_ticket

make lbll_lazy POWER=1 GC=0 LOCK=NONE
mv bin/lb-ll_lazy bin/lb-ll_lazy_no_lock
