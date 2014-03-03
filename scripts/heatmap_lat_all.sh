
MAKE=make

unm=$(uname -n);
if [ $unm = "ol-collab1" ];
then
    MAKE=gmake
fi;

if [ $# -le 1 ];		# pass any param to avoid compilation
then
    LATENCY=1 INIT=one GRANULARITY=GLOBAL_LOCK $MAKE -k tas
    LATENCY=1 INIT=one $MAKE -k tas
    LATENCY=1 INIT=one $MAKE -k lockfree
fi

inits="256 1024 2048 8192 65536"
duration=1000;

source ./scripts/heatmap.config

if [ $# -ge 1 ];
then
    source "$1";
else
    source ./scripts/executables.config
fi;

for initial in ${inits}
do
    range=$((2*${initial}));

    echo "## initial: $initial";

    unm=$(uname -n);
    rm data/${unm}_*_heatmap_uc_lat_*_${initial}.csv
    if [ $do_ll -eq 1 ];
    then
	echo "#  ll (${lb_ll} vs. ${lf_ll})";
	./scripts/heatmap_lat.sh "${lb_ll}" "${lf_ll}" u c -i${initial} -r${range} -d$duration
	cp data/lat_put_lb.txt data/${unm}_ll_heatmap_uc_lat_put_lb_${initial}.csv
	cp data/lat_put_lf.txt data/${unm}_ll_heatmap_uc_lat_put_lf_${initial}.csv
	cp data/lat_put_ratio.txt data/${unm}_ll_heatmap_uc_lat_put_ratio_${initial}.csv
	cp data/lat_get_lb.txt data/${unm}_ll_heatmap_uc_lat_get_lb_${initial}.csv
	cp data/lat_get_lf.txt data/${unm}_ll_heatmap_uc_lat_get_lf_${initial}.csv
	cp data/lat_get_ratio.txt data/${unm}_ll_heatmap_uc_lat_get_ratio_${initial}.csv
	cp data/lat_rem_lb.txt data/${unm}_ll_heatmap_uc_lat_rem_lb_${initial}.csv
	cp data/lat_rem_lf.txt data/${unm}_ll_heatmap_uc_lat_rem_lf_${initial}.csv
	cp data/lat_rem_ratio.txt data/${unm}_ll_heatmap_uc_lat_rem_ratio_${initial}.csv
    fi

    if [ $do_ht -eq 1 ];
    then
	echo "#  ht (${lb_ht} vs. ${lf_ht})";
	./scripts/heatmap_lat.sh "${lb_ht}" "${lf_ht}" u c -i${initial} -r${range} -d$duration
	cp data/lat_put_lb.txt data/${unm}_ht_heatmap_uc_lat_put_lb_${initial}.csv
	cp data/lat_put_lf.txt data/${unm}_ht_heatmap_uc_lat_put_lf_${initial}.csv
	cp data/lat_put_ratio.txt data/${unm}_ht_heatmap_uc_lat_put_ratio_${initial}.csv
	cp data/lat_get_lb.txt data/${unm}_ht_heatmap_uc_lat_get_lb_${initial}.csv
	cp data/lat_get_lf.txt data/${unm}_ht_heatmap_uc_lat_get_lf_${initial}.csv
	cp data/lat_get_ratio.txt data/${unm}_ht_heatmap_uc_lat_get_ratio_${initial}.csv
	cp data/lat_rem_lb.txt data/${unm}_ht_heatmap_uc_lat_rem_lb_${initial}.csv
	cp data/lat_rem_lf.txt data/${unm}_ht_heatmap_uc_lat_rem_lf_${initial}.csv
	cp data/lat_rem_ratio.txt data/${unm}_ht_heatmap_uc_lat_rem_ratio_${initial}.csv
    fi

    if [ $do_sl -eq 1 ];
    then
	echo "#  sl (${lb_sl} vs. ${lf_sl})";
	./scripts/heatmap_lat.sh "${lb_sl}" "${lf_sl}" u c -i${initial} -r${range} -d$duration
	cp data/lat_put_lb.txt data/${unm}_sl_heatmap_uc_lat_put_lb_${initial}.csv
	cp data/lat_put_lf.txt data/${unm}_sl_heatmap_uc_lat_put_lf_${initial}.csv
	cp data/lat_put_ratio.txt data/${unm}_sl_heatmap_uc_lat_put_ratio_${initial}.csv
	cp data/lat_get_lb.txt data/${unm}_sl_heatmap_uc_lat_get_lb_${initial}.csv
	cp data/lat_get_lf.txt data/${unm}_sl_heatmap_uc_lat_get_lf_${initial}.csv
	cp data/lat_get_ratio.txt data/${unm}_sl_heatmap_uc_lat_get_ratio_${initial}.csv
	cp data/lat_rem_lb.txt data/${unm}_sl_heatmap_uc_lat_rem_lb_${initial}.csv
	cp data/lat_rem_lf.txt data/${unm}_sl_heatmap_uc_lat_rem_lf_${initial}.csv
	cp data/lat_rem_ratio.txt data/${unm}_sl_heatmap_uc_lat_rem_ratio_${initial}.csv
    fi

    if [ $do_bst -eq 1 ];
    then
	echo "#  bst (${lb_bst} vs. ${lf_bst})";
	./scripts/heatmap_lat.sh "${lb_bst}" "${lf_bst}" u c -i${initial} -r${range} -d$duration
	cp data/lat_put_lb.txt data/${unm}_bst_heatmap_uc_lat_put_lb_${initial}.csv
	cp data/lat_put_lf.txt data/${unm}_bst_heatmap_uc_lat_put_lf_${initial}.csv
	cp data/lat_put_ratio.txt data/${unm}_bst_heatmap_uc_lat_put_ratio_${initial}.csv
	cp data/lat_get_lb.txt data/${unm}_bst_heatmap_uc_lat_get_lb_${initial}.csv
	cp data/lat_get_lf.txt data/${unm}_bst_heatmap_uc_lat_get_lf_${initial}.csv
	cp data/lat_get_ratio.txt data/${unm}_bst_heatmap_uc_lat_get_ratio_${initial}.csv
	cp data/lat_rem_lb.txt data/${unm}_bst_heatmap_uc_lat_rem_lb_${initial}.csv
	cp data/lat_rem_lf.txt data/${unm}_bst_heatmap_uc_lat_rem_lf_${initial}.csv
	cp data/lat_rem_ratio.txt data/${unm}_bst_heatmap_uc_lat_rem_ratio_${initial}.csv
    fi
done

#remove data for u0 from put and rem files
sed -i '2d' data/${unm}_*_heatmap_uc_lat_rem_*.csv
sed -i '2d' data/${unm}_*_heatmap_uc_lat_put_*.csv
#remove data for u100 form get files
sed -i '7d' data/${unm}_*_heatmap_uc_lat_get_*.csv
