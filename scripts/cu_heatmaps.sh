#!/bin/bash
unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8"
inits="256 1024 2048 8192 65536"
for unm in $unames
do
for initial in ${inits}
do

R -f ./scripts/heatmap.r --args ./data/${unm}_ll_heatmap_uc_${initial}.csv ./data/${unm}_ll_heatmap_uc_${initial}.pdf "Linked list, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
R -f ./scripts/heatmap.r --args ./data/${unm}_ht_heatmap_uc_${initial}.csv ./data/${unm}_ht_heatmap_uc_${initial}.pdf "Hash table, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
R -f ./scripts/heatmap.r --args ./data/${unm}_sl_heatmap_uc_${initial}.csv ./data/${unm}_sl_heatmap_uc_${initial}.pdf "Skip list, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
R -f ./scripts/heatmap.r --args ./data/${unm}_bst_heatmap_uc_${initial}.csv ./data/${unm}_bst_heatmap_uc_${initial}.pdf "BST, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
done
done
