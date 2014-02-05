#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit;
fi

core=$1

unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch"


for unm in $unames
do
r -f heatmap.r --args ${unm}_ll_heatmap_${core}.csv ${unm}_ll_heatmap_${core}.pdf "Linked list, lf/lb, $unm, $core cores" "Key range" "Update ratio"
r -f heatmap.r --args ${unm}_sl_heatmap_${core}.csv ${unm}_sl_heatmap_${core}.pdf "Skip-list, lf/lb, $unm, $core cores" "Key range" "Update ratio"
r -f heatmap.r --args ${unm}_ht_heatmap_${core}.csv ${unm}_ht_heatmap_${core}.pdf "Hash table, lf/lb, $unm, $core cores" "Key range" "Update ratio"
r -f heatmap.r --args ${unm}_bst_heatmap_${core}.csv ${unm}_bst_heatmap_${core}.pdf "BST, lf/lb, $unm, $core cores" "Key range" "Update ratio"
done
