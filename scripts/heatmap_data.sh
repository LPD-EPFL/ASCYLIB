#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit;
fi

core=$1

source ./scripts/heatmap.config

unm=$(uname -n);
rm data/${unm}_*_heatmap_${core}.csv
./scripts/heatmap.sh ./bin/lb-ll ./bin/lf-ll u s -n${core} -d1000 > data/${unm}_ll_heatmap_${core}.csv
./scripts/heatmap.sh ./bin/lb-ht-gl ./bin/lf-ht u s -n${core} -d1000 > data/${unm}_ht_heatmap_${core}.csv
./scripts/heatmap.sh ./bin/lb-sl ./bin/lf-sl u s -n${core} -d1000 > data/${unm}_sl_heatmap_${core}.csv
./scripts/heatmap.sh ./bin/lb-bst2 ./bin/${lftree} u s -n${core} -d1000 > data/${unm}_bst_heatmap_${core}.csv

