#!/bin/bash

structs="ll ht sl bst"
inits="256 1024 2048 8192 65536"
unames="lpdxeon2680 lpd48core maglite diassrv8 parsasrv1.epfl.ch lpdpc4 ol-collab1"

if [ $# -ge 1 ];
then
    unames="$1";
    echo "**Creating plots for: $unames";
fi;

plots_folder=./plots

if [ $# -ge 2 ];
then
    plots_folder="$2";
    echo "**Merging plots in: $plots_folder";
fi;


for s in $structs; do
    for i in $inits; do
        for unm in $unames; do
           R -f scripts/multiplot.r  --args ./data/${unm}_${s}_heatmap_uc_${i}.csv ./data/${unm}_${s}_heatmap_uc_scal_${i}.csv ./data/${unm}_${s}_heatmap_uc_lat_get_ratio_${i}.csv ./data/${unm}_${s}_heatmap_uc_lat_put_ratio_${i}.csv ./data/${unm}_${s}_heatmap_uc_lat_rem_ratio_${i}.csv ${plots_folder}/${unm}_multi_${s}_${i}_merged.pdf "Stats $s $unm init=${i}" 
        done
    done
done
