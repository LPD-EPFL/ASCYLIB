#!/bin/bash
unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8 lpdpc4 ol-collab1"
inits="256 1024 2048 8192 65536"

plots_folder=plots
[ -d foo ] || mkdir $plots_folder;

for unm in $unames
do
    for initial in ${inits}
    do

	data=./data/${unm}_ll_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2.r --args $data ./$plots_folder/${unm}_ll_heatmap_uc_${initial}.pdf "Linked list, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
	data=./data/${unm}_ht_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2.r --args $data ./$plots_folder/${unm}_ht_heatmap_uc_${initial}.pdf "Hash table, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
	data=./data/${unm}_sl_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2.r --args $data ./$plots_folder/${unm}_sl_heatmap_uc_${initial}.pdf "Skip list, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
	data=./data/${unm}_bst_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2.r --args $data ./$plots_folder/${unm}_bst_heatmap_uc_${initial}.pdf "BST, lf/lb, $unm, $initial initial" "Cores" "Update ratio"
    done
done
