#!/bin/bash
INIT=one GRANULARITY=GLOBAL_LOCK make tas
INIT=one make tas
INIT=one make lockfree

inits="256 1024 2048 8129 65536"

for initial in ${inits}
do
range=$((2*${initial}));

unm=$(uname -n);
rm data/${unm}_*_heatmap_uc_${initial}.csv
./scripts/heatmap.sh ./bin/lb-ll ./bin/lf-ll u c -i${initial} -r${range} -d2000 > data/${unm}_ll_heatmap_uc_${initial}.csv
./scripts/heatmap.sh ./bin/lb-ht_gl ./bin/lf-ht u c -i${initial} -r${range} -d2000 > data/${unm}_ht_heatmap_uc_${initial}.csv
./scripts/heatmap.sh ./bin/lb-sl ./bin/lf-sl u c -i${initial} -r${range} -d2000 > data/${unm}_sl_heatmap_uc_${initial}.csv
./scripts/heatmap.sh ./bin/lb-bst2 ./bin/lf-bst-howley u c -i${initial} -r${range} -d2000 > data/${unm}_bst_heatmap_uc_${initial}.csv
done
