#!/bin/bash

structs="ll ht sl bst"
inits="256 1024 2048 8192 65536"
unm="lpdxeon2680"

for s in $structs; do
    for i in $inits; do
       R -f change_data.r --args ./data/${unm}_${s}_heatmap_uc_${i}.csv ./data/${unm}_${s}_heatmap_uc_scal_${i}.csv ./data/${unm}_${s}_heatmap_uc_lat_get_ratio_${i}.csv ./data/${unm}_${s}_heatmap_uc_lat_put_ratio_${i}.csv ./data/${unm}_${s}_heatmap_uc_lat_rem_ratio_${i}.csv ${unm}_${s}_${i}_merged.pdf "Stats $s $unm init=${i}" 
    done
done
