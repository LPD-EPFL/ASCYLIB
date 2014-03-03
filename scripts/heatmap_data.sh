#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit;
fi

core=$1

source ./scripts/heatmap.config
source ./scripts/executables.config

unm=$(uname -n);
rm data/${unm}_*_heatmap_${core}.csv
./scripts/heatmap.sh ${lb_ll} ${lf_ll} u s -n${core} -d1000 > data/${unm}_ll_heatmap_${core}.csv
./scripts/heatmap.sh ${lb_ht} ${lf_ht} u s -n${core} -d1000 > data/${unm}_ht_heatmap_${core}.csv
./scripts/heatmap.sh ${lb_sl} ${lf_sl} u s -n${core} -d1000 > data/${unm}_sl_heatmap_${core}.csv
./scripts/heatmap.sh ${lb_bst} ${lf_bst} u s -n${core} -d1000 > data/${unm}_bst_heatmap_${core}.csv

