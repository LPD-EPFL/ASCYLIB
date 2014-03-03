#!/bin/bash

unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8 lpdpc4 ol-collab1"
if [ $# -ge 1 ];
then
    unames="$@";
    echo "**Creating plots for: $unames";
fi;

inits="256 1024 2048 8192 65536"

plots_folder=plots
data_folder=data
[ -d foo ] || mkdir $plots_folder;

for unm in $unames
do
    for initial in ${inits}
    do

	data=./data/${unm}_ll_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_ll_heatmap_uc_${initial}.pdf "Linked list, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ll_heatmap_uc_frac_${initial}.csv

	data=./data/${unm}_ht_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_ht_heatmap_uc_${initial}.pdf "Hash table, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ht_heatmap_uc_frac_${initial}.csv

	data=./data/${unm}_sl_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_sl_heatmap_uc_${initial}.pdf "Skip list, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_sl_heatmap_uc_frac_${initial}.csv

	data=./data/${unm}_bst_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_bst_heatmap_uc_${initial}.pdf "BST, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_bst_heatmap_uc_frac_${initial}.csv
	
	data=./data/${unm}_ll_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_ll_heatmap_scal_uc_${initial}.pdf "LL, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ll_heatmap_uc_scal_frac_${initial}.csv

	data=./data/${unm}_ht_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_ht_heatmap_scal_uc_${initial}.pdf "HT, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ht_heatmap_uc_scal_frac_${initial}.csv

	data=./data/${unm}_sl_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_sl_heatmap_scal_uc_${initial}.pdf "SL, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_sl_heatmap_uc_scal_frac_${initial}.csv

	data=./data/${unm}_bst_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_bst_heatmap_scal_uc_${initial}.pdf "BST, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_bst_heatmap_uc_scal_frac_${initial}.csv


    done
done

echo "**Created  plots for: $unames";
