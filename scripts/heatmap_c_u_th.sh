
MAKE=make

unm=$(uname -n);
if [ $unm = "ol-collab1" ];
then
    MAKE=gmake
fi;

if [ $# -le 1 ];		# pass any param to avoid compilation
then
    INIT=one GRANULARITY=GLOBAL_LOCK $MAKE -k tas
    INIT=one $MAKE -k tas
    INIT=one $MAKE -k lockfree
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
    rm data/${unm}_*_heatmap_uc_*${initial}.csv
    if [ $do_ll -eq 1 ];
    then
	echo "#  ll (${lb_ll} vs. ${lf_ll})";
	./scripts/heatmap_avg.sh "${lb_ll}" "${lf_ll}" u c -i${initial} -r${range} -d$duration | tee data/${unm}_ll_heatmap_uc_${initial}.csv
	cp data/temp1.txt data/${unm}_ll_heatmap_uc_lb_${initial}.csv
	cp data/temp2.txt data/${unm}_ll_heatmap_uc_lf_${initial}.csv
	cp data/temp3.txt data/${unm}_ll_heatmap_uc_frac_${initial}.csv
	cp data/scal_temp.txt data/${unm}_ll_heatmap_uc_scal_${initial}.csv
	cp data/scal_data.txt data/${unm}_ll_heatmap_uc_scal_frac_${initial}.csv
    fi

    if [ $do_ht -eq 1 ];
    then
	echo "#  ht (${lb_ht} vs. ${lf_ht})";
	./scripts/heatmap_avg.sh "${lb_ht}" "${lf_ht}" u c -i${initial} -r${range} -d$duration | tee data/${unm}_ht_heatmap_uc_${initial}.csv
	cp data/temp1.txt data/${unm}_ht_heatmap_uc_lb_${initial}.csv
	cp data/temp2.txt data/${unm}_ht_heatmap_uc_lf_${initial}.csv
	cp data/temp3.txt data/${unm}_ht_heatmap_uc_frac_${initial}.csv
	cp data/scal_temp.txt data/${unm}_ht_heatmap_uc_scal_${initial}.csv
	cp data/scal_data.txt data/${unm}_ht_heatmap_uc_scal_frac_${initial}.csv
    fi

    if [ $do_sl -eq 1 ];
    then
	echo "#  sl (${lb_sl} vs. ${lf_sl})";
	./scripts/heatmap_avg.sh "${lb_sl}" "${lf_sl}" u c -i${initial} -r${range} -d$duration | tee data/${unm}_sl_heatmap_uc_${initial}.csv
	cp data/temp1.txt data/${unm}_sl_heatmap_uc_lb_${initial}.csv
	cp data/temp2.txt data/${unm}_sl_heatmap_uc_lf_${initial}.csv
	cp data/temp3.txt data/${unm}_sl_heatmap_uc_frac_${initial}.csv
	cp data/scal_temp.txt data/${unm}_sl_heatmap_uc_scal_${initial}.csv
	cp data/scal_data.txt data/${unm}_sl_heatmap_uc_scal_frac_${initial}.csv
    fi

    if [ $do_bst -eq 1 ];
    then
	echo "#  bst (${lb_bst} vs. ${lf_bst})";
	./scripts/heatmap_avg.sh "${lb_bst}" "${lf_bst}" u c -i${initial} -r${range} -d$duration | tee data/${unm}_bst_heatmap_uc_${initial}.csv
	cp data/temp1.txt data/${unm}_bst_heatmap_uc_lb_${initial}.csv
	cp data/temp2.txt data/${unm}_bst_heatmap_uc_lf_${initial}.csv
	cp data/temp3.txt data/${unm}_bst_heatmap_uc_frac_${initial}.csv
	cp data/scal_temp.txt data/${unm}_bst_heatmap_uc_scal_${initial}.csv
	cp data/scal_data.txt data/${unm}_bst_heatmap_uc_scal_frac_${initial}.csv
    fi
done
