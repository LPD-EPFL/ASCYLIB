#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit;
fi

core=$1

unm=$(uname -n);
./scripts/heatmap.sh ./bin/lb-ll ./bin/lf-ll u s -n${core} -d1000 > data/${unm}_ll_heatmap_${core}.csv
./scripts/heatmap.sh ./bin/lb-ht ./bin/lf-ht u s -n${core} -d1000 > data/${unm}_ht_heatmap_${core}.csv
./scripts/heatmap.sh ./bin/lb-sl ./bin/lf-sl u s -n${core} -d1000 > data/${unm}_sl_heatmap_${core}.csv
./scripts/heatmap.sh ./bin/lb-bst2 ./bin/lf-bst u s -n${core} -d1000 > data/${unm}_bst_heatmap_${core}.csv

#run the next only if R is installed
#r -f scripts/heatmap.r --args data/${unm}_ll_heatmap_${core}.csv data/${unm}_ll_heatmap_${core}.pdf "Linked list, lf/lb" "Key range" "Update ratio"
#r -f scripts/heatmap.r --args data/${unm}_sl_heatmap_${core}.csv data/${unm}_sl_heatmap_${core}.pdf "Skip-list, lf/lb" "Key range" "Update ratio"
#r -f scripts/heatmap.r --args data/${unm}_ht_heatmap_${core}.csv data/${unm}_ht_heatmap_${core}.pdf "Hash table, lf/lb" "Key range" "Update ratio"
#r -f scripts/heatmap.r --args data/${unm}_bst_heatmap_${core}.csv data/${unm}_bst_heatmap_${core}.pdf "BST, lf/lb" "Key range" "Update ratio"
