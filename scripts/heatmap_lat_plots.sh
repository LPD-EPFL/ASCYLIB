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

	data=./data/${unm}_ll_heatmap_uc_lat_put_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_ll_heatmap_uc_lat_put_ratio_${initial}.pdf "Linked list, lf/lb put latency, $unm, $initial initial" "Cores" "Update ratio"

    data=./data/${unm}_ll_heatmap_uc_lat_rem_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_ll_heatmap_uc_lat_rem_ratio_${initial}.pdf "Linked list, lf/lb rem latency, $unm, $initial initial" "Cores" "Update ratio"

	data=./data/${unm}_ll_heatmap_uc_lat_get_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_ll_heatmap_uc_lat_get_ratio_${initial}.pdf "Linked list, lf/lb get latency, $unm, $initial initial" "Cores" "Update ratio"

    data=./data/${unm}_ht_heatmap_uc_lat_put_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_ht_heatmap_uc_lat_put_ratio_${initial}.pdf "Hash table, lf/lb put latency, $unm, $initial initial" "Cores" "Update ratio"

    data=./data/${unm}_ht_heatmap_uc_lat_rem_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_ht_heatmap_uc_lat_rem_ratio_${initial}.pdf "Hash table, lf/lb rem latency, $unm, $initial initial" "Cores" "Update ratio"

	data=./data/${unm}_ht_heatmap_uc_lat_get_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_ht_heatmap_uc_lat_get_ratio_${initial}.pdf "Hash table, lf/lb get latency, $unm, $initial initial" "Cores" "Update ratio"

	data=./data/${unm}_sl_heatmap_uc_lat_put_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_sl_heatmap_uc_lat_put_ratio_${initial}.pdf "Skip-list, lf/lb put latency, $unm, $initial initial" "Cores" "Update ratio"

    data=./data/${unm}_sl_heatmap_uc_lat_rem_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_sl_heatmap_uc_lat_rem_ratio_${initial}.pdf "Skip-list, lf/lb rem latency, $unm, $initial initial" "Cores" "Update ratio"

	data=./data/${unm}_sl_heatmap_uc_lat_get_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_sl_heatmap_uc_lat_get_ratio_${initial}.pdf "Skip-list, lf/lb get latency, $unm, $initial initial" "Cores" "Update ratio"

	data=./data/${unm}_bst_heatmap_uc_lat_put_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_bst_heatmap_uc_lat_put_ratio_${initial}.pdf "BST, lf/lb put latency, $unm, $initial initial" "Cores" "Update ratio"

    data=./data/${unm}_bst_heatmap_uc_lat_rem_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_bst_heatmap_uc_lat_rem_ratio_${initial}.pdf "BST, lf/lb rem latency, $unm, $initial initial" "Cores" "Update ratio"

	data=./data/${unm}_bst_heatmap_uc_lat_get_ratio_${initial}.csv;
	[ -f $data ] && R -f ./scripts/heatmap_lat.r --args $data ./$plots_folder/${unm}_bst_heatmap_uc_lat_get_ratio_${initial}.pdf "BST, lf/lb get latency, $unm, $initial initial" "Cores" "Update ratio"


    done
done

echo "**Created  plots for: $unames";
