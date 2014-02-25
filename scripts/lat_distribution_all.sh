#!/bin/bash

MAKE=make

unm=$(uname -n);
if [ $unm = "ol-collab1" ];
then
    MAKE=gmake
fi;

if [ $# -eq 0 ];		# pass any param to avoid compilation
then
    LATENCY=2 INIT=one GRANULARITY=GLOBAL_LOCK $MAKE -k tas
    LATENCY=2 INIT=one $MAKE -k tas
    LATENCY=2 INIT=one $MAKE -k lockfree
fi

./scripts/lat_distribution.sh ${unm}_lat_lf_ll ./bin/lf-ll -d1000 -f1024 -v1024
./scripts/lat_distribution.sh ${unm}_lat_lb_ll ./bin/lb-ll -d1000 -f1024 -v1024
./scripts/lat_distribution.sh ${unm}_lat_lf_ht ./bin/lf-ht -d1000 -f1024 -v1024
./scripts/lat_distribution.sh ${unm}_lat_lb_ht ./bin/lb-ht_gl -d1000 -f1024 -v1024
./scripts/lat_distribution.sh ${unm}_lat_lf_sl ./bin/lf-sl -d1000 -f1024 -v1024
./scripts/lat_distribution.sh ${unm}_lat_lb_sl ./bin/lb-sl -d1000 -f1024 -v1024
./scripts/lat_distribution.sh ${unm}_lat_lf_bst ./bin/lf-bst-howley -d1000 -f1024 -v1024
./scripts/lat_distribution.sh ${unm}_lat_lb_bst ./bin/lb-bst2 -d1000 -f1024 -v1024
