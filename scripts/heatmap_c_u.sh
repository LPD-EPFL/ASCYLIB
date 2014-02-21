#!/bin/bash

MAKE=make

unm=$(uname -n);
if [ $unm = "ol-collab1" ];
then
    MAKE=gmake
fi;

if [ $# -eq 0 ];		# pass any param to avoid compilation
then
    INIT=one GRANULARITY=GLOBAL_LOCK $MAKE -k tas
    INIT=one $MAKE -k tas
    INIT=one $MAKE -k lockfree
fi

inits="256 1024 2048 8129 65536"
duration=2000;

for initial in ${inits}
do
range=$((2*${initial}));

echo "## initial: $initial";

unm=$(uname -n);
rm data/${unm}_*_heatmap_uc_${initial}.csv
echo '#  ll';
./scripts/heatmap.sh ./bin/lb-ll ./bin/lf-ll u c -i${initial} -r${range} -d$duration | tee data/${unm}_ll_heatmap_uc_${initial}.csv
echo '#  ht';
./scripts/heatmap.sh ./bin/lb-ht_gl ./bin/lf-ht u c -i${initial} -r${range} -d$duration | tee data/${unm}_ht_heatmap_uc_${initial}.csv
echo '#  sl';
./scripts/heatmap.sh ./bin/lb-sl ./bin/lf-sl u c -i${initial} -r${range} -d$duration | tee data/${unm}_sl_heatmap_uc_${initial}.csv
echo '#  bst';
./scripts/heatmap.sh ./bin/lb-bst2 ./bin/lf-bst-howley u c -i${initial} -r${range} -d$duration | tee data/${unm}_bst_heatmap_uc_${initial}.csv
done
