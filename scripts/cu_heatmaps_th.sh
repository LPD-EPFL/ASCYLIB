#!/bin/bash

#param1: list of platoform(s), e.g., "maglite lpdpc4"
#param2: folder of data to create plots for, e.g., "archive/something"

unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8 lpdpc4 ol-collab1"
if [ $# -ge 1 ];
then
    unames="$1";
    echo "**Creating plots for: $unames";
fi;

plots_folder=plots
data_folder=data

if [ $# -ge 2 ];
then
    data_folder="$2";
    echo "**Creating plots for data files in: $data_folder";
    plots_folder=${data_folder}/plots;
    echo "**Creating plots in: $plots_folder";
fi;

[ -d $plots_folder ] || mkdir -p $plots_folder;

inits="256 1024 2048 8192 65536"

for unm in $unames
do
    for initial in ${inits}
    do

	data=./${data_folder}/${unm}_ll_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_ll_heatmap_uc_${initial}.pdf "Linked list, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ll_heatmap_uc_frac_${initial}.csv

	data=./${data_folder}/${unm}_ht_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_ht_heatmap_uc_${initial}.pdf "Hash table, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ht_heatmap_uc_frac_${initial}.csv

	data=./${data_folder}/${unm}_sl_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_sl_heatmap_uc_${initial}.pdf "Skip list, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_sl_heatmap_uc_frac_${initial}.csv

	data=./${data_folder}/${unm}_bst_heatmap_uc_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_th.r --args $data ./$plots_folder/${unm}_bst_heatmap_uc_${initial}.pdf "BST, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_bst_heatmap_uc_frac_${initial}.csv
	
	data=./${data_folder}/${unm}_ll_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_ll_heatmap_scal_uc_${initial}.pdf "LL, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ll_heatmap_uc_scal_frac_${initial}.csv

	data=./${data_folder}/${unm}_ht_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_ht_heatmap_scal_uc_${initial}.pdf "HT, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_ht_heatmap_uc_scal_frac_${initial}.csv

	data=./${data_folder}/${unm}_sl_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_sl_heatmap_scal_uc_${initial}.pdf "SL, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_sl_heatmap_uc_scal_frac_${initial}.csv

	data=./${data_folder}/${unm}_bst_heatmap_uc_scal_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap2_sc.r --args $data ./$plots_folder/${unm}_bst_heatmap_scal_uc_${initial}.pdf "BST, Scal, lf/lb, $unm, $initial initial" "Cores" "Update ratio" ./${data_folder}/${unm}_bst_heatmap_uc_scal_frac_${initial}.csv


    done
done

echo "**Created  plots for: $unames";
