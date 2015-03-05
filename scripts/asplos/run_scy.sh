#!/bin/bash

config_workload=./scripts/asplos/config/scy1.long
config_execs=./scripts/asplos/config/all


skip=0;
if [ "$1" = "skip" ];
then
    skip=1;
    echo "** Skipping check and proceeding to the experiment!";
    shift;
fi;

un=$(uname -n);
ub="bin/$un";
if [ ! -d "$ub" ];
then
    mkdir $ub;
fi;

MAKE=make;
if [ "$un" = ol-collab1 ];
then
    MAKE=gmake
fi;

# default data structures
lls="./$ub/sq-ll ./$ub/lb-ll_lazy ./$ub/lb-ll_pugh ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt";
hts="./$ub/sq-ht ./$ub/lb-ht_coupling_gl ./$ub/lb-ht_lazy_gl ./$ub/lb-ht_pugh_gl ./$ub/lb-ht_copy ./$ub/lf-ht_rcu ./$ub/lb-ht_java ./$ub/lb-ht_tbb ./$ub/lf-ht";
sls="./$ub/sq-sl ./$ub/lb-sl_pugh ./$ub/lb-sl_herlihy ./$ub/lf-sl_fraser  ./$ub/lf-sl_herlihy";
bsts="./$ub/sq-bst_internal ./$ub/sq-bst_external ./$ub/lb-bst_bronson ./$ub/lb-bst-drachsler ./$ub/lf-bst_ellen ./$ub/lf-bst-howley ./$ub/lf-bst-aravind";

if [ $# -gt 0 ];
then
    config_workload="$1";
    echo "# Using workload configuration file   : $config_workload";

    if [ ! -f "$config_workload" ];
    then
	echo "* File does not exist: $config_workload";
	exit;
    fi;
fi;

if [ $# -gt 1 ];
then
    config_execs="$2";
    echo "# Using executable configuration file : $config_execs";
    if [ ! -f "$config_execs" ];
    then
	echo "* File does not exist: $config_execs";
	exit;
    fi;
fi;

out_folder=data;

source ${config_execs};
source ${config_workload};

if [ "${SCY}0" = "0" ];
then
    SCY=scy1;
fi;

printf "## Will compile with                : $COMPILE_FLAGS\n";
printf "## Will do measurements for         : $SCY\n";
printf "## Will measure                     : ";

metrics=3;
if [ "${skip_thr}0" -eq 10 ];
then
    metrics=$(($metrics-1));
else
    printf "  THROUGHPUT";
fi;
printf " / ";

if [ "${skip_lat}0" -eq 10 ];
then
    metrics=$(($metrics-1));
else
    printf "  LATENCY";
fi;
printf " / ";


if [ "${LATENCY_TYPE}0" != "0" ];
then
    if [ $LATENCY_TYPE -ge 4 ];
    then
	LATENCY_MERGE=1;
    fi;
fi;

if [ "${LATENCY_MERGE}0" -eq 10 ];
then
    ldi_print="merged";
    ldi_script=./scripts/latency_raw.sh
else
    ldi_print="seprte";
    ldi_script=./scripts/latency_raw_suc.sh
fi;

LATENCY_AVG_TYPE=1;
lat_script=./scripts/latency_rep.sh
if [ "${LATENCY_PARSE}0" -eq 10 ];
then
    LATENCY_AVG_TYPE=4;
    lat_script=./scripts/latency_parse.sh
fi;


do_ldi=1;
if [ "${skip_ldi}0" -eq 10 ];
then
    do_ldi=0;
else
    printf "  LATENCY DISTRIBUTION ($ldi_print)";
fi;
printf " / ";


if [ "${skip_pow}0" -eq 10 ];
then
    metrics=$(($metrics-1));
else
    printf "  POWER";
fi;

echo "";

printf "## On the following data structures : ";

if [ $do_ll -eq 1 ];
then
    printf "  LL";
fi;
printf " / ";

if [ $do_ht -eq 1 ];
then
    printf "  HT";
fi;
printf " / ";

if [ $do_sl -eq 1 ];
then
    printf "  SL";
fi;
printf " / ";

if [ $do_bst -eq 1 ];
then
    printf "  BST";
fi;
echo "";


# estimate the time to execute the experiment
lln=$(echo "$lls" | wc -w);
ll_num=$(($lln*$do_ll));
htn=$(echo "$hts" | wc -w);
ht_num=$(($htn*$do_ht));
sln=$(echo "$sls" | wc -w);
sl_num=$(($sln*$do_sl));
bstn=$(echo "$bsts" | wc -w);
bst_num=$(($bstn*$do_bst));
exec_num=$((${ll_num}+${ht_num}+${sl_num}+${bst_num}));
echo "## Number of executables            : $exec_num";

i_num=$(echo $initials | wc -w);
u_num=$(echo $updates | wc -w);
source scripts/config;

c_num=$(echo "1 "${cores} | wc -w);
est_time_thr_lat=$(echo "${metrics}*${i_num}*${u_num}*${exec_num}*${reps}*(${duration}/1000)*${c_num}/3600" | bc -l);

c_num_ldi=$(echo $cores_lat_dist | wc -w);
est_time_ldi=$(echo "${do_ldi}*${c_num_ldi}*${i_num}*${u_num}*${exec_num}*(${duration}/1000)/3600" | bc -l);

est_time=$(echo "${est_time_thr_lat}+${est_time_ldi}" | bc -l);
printf "## Estimated time for the experiment: %6.3f h\n" $est_time;

if [ $skip -eq 0 ];
then
    printf "   Continue? [Y/n] ";
    read cont;
    if [ "$cont" = "n" ];
    then
	exit;
    fi;
fi;

#############################################


if [ ! "${skip_pow}0" -eq 10 ];
then

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Power";
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    echo "** Please give sudo access to the script";
    sudo echo "";

if [ "${cores_pow}0" = "0" ];
then
    cores_pow=$cores;
fi;

    # ll ##################################################################
    if [ $do_ll -eq 1 ];
    then
	structure=ll;
	${MAKE} -k ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "$lls" $params | tee $dat; 
	    done;
	done;
    fi;


    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;

	if [ ! "${MAKE_CLHT}0" = 0 ];
	then
	    echo "~~~~~~ Compiling CLHT";
	    cd ${CLHT_PATH};
	    ${MAKE} -k clean hyht_res lfht_res hyht_mem lfht_mem ${COMPILE_FLAGS};
	    cd -;
	    cp ${CLHT_PATH}/hyht ${CLHT_PATH}/lfht_res ${CLHT_PATH}/hyhtm ${CLHT_PATH}/lfhtm $ub;
	fi;

	${MAKE} -k ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "$hts" $params | tee $dat; 
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
	${MAKE} -k ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "$sls" $params | tee $dat; 
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
	${MAKE} -k ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "$bsts" $params | tee $dat; 
	    done;
	done;
    fi;
fi;



if [ ! "${skip_thr}0" -eq 10 ];
then

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Throughput";
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    # ll ##################################################################
    if [ $do_ll -eq 1 ];
    then
	structure=ll;
	${MAKE} -k ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "$lls" $params | tee $dat; 
	    done;
	done;
    fi;


    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;

	if [ ! "${MAKE_CLHT}0" = 0 ];
	then
	    echo "~~~~~~ Compiling CLHT";
	    cd ${CLHT_PATH};
	    ${MAKE} -k clean hyht_res lfht_res hyht_mem lfht_mem ${COMPILE_FLAGS};
	    cd -;
	    cp ${CLHT_PATH}/hyht ${CLHT_PATH}/lfht_res ${CLHT_PATH}/hyhtm ${CLHT_PATH}/lfhtm $ub;
	fi;

	${MAKE} -k ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "$hts" $params | tee $dat; 
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
	${MAKE} -k ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "$sls" $params | tee $dat; 
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
	${MAKE} -k ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "$bsts" $params | tee $dat; 
	    done;
	done;
    fi;
fi;


if [ ! "${skip_lat}0" -eq 10 ];
then

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency average";
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    # ll ##################################################################
    if [ $do_ll -eq 1 ];
    then
	structure=ll;
	${MAKE} -k ${structure} LATENCY=${LATENCY_AVG_TYPE} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		${lat_script} "$cores" $reps $keep "$lls" $params | tee $dat; 
	    done;
	done;
    fi;

    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;

	if [ ! "${MAKE_CLHT}0" = 0 ];
	then
	    echo "~~~~~~ Compiling CLHT";
	    cd ${CLHT_PATH};
	    ${MAKE} -k clean hyht_res lfht_res hyht_mem lfht_mem ${COMPILE_FLAGS};
	    cd -;
	    cp ${CLHT_PATH}/hyht ${CLHT_PATH}/lfht_res ${CLHT_PATH}/hyhtm ${CLHT_PATH}/lfhtm $ub;
	fi;

	${MAKE} -k ${structure} LATENCY=${LATENCY_AVG_TYPE} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		${lat_script} "$cores" $reps $keep "$hts" $params | tee $dat; 
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
	${MAKE} -k ${structure} LATENCY=${LATENCY_AVG_TYPE} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		${lat_script} "$cores" $reps $keep "$sls" $params | tee $dat; 
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
	${MAKE} -k ${structure} LATENCY=${LATENCY_AVG_TYPE} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/${SCY}.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		${lat_script} "$cores" $reps $keep "$bsts" $params | tee $dat; 
	    done;
	done;
    fi;
fi;

if [ ! "${skip_ldi}0" -eq 10 ];
then

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency distribution";
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    # ll ##################################################################
    if [ $do_ll -eq 1 ];
    then
	structure=ll;
	${MAKE} -k ${structure} LATENCY=$LATENCY_TYPE ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for c in $cores_lat_dist
	do
	    for i in $initials;
	    do
		r=$((2*${i}));
		for u in $updates;
		do
		    params="-n$c -i$i -r$r -u$u -d$duration";
		    dat=$out_folder/${SCY}.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "$lls" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat;
		    head -n8 $dat; echo "..."; tail -n8 $dat;
		done;
	    done;
	done;
    fi;

    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;

	if [ ! "${MAKE_CLHT}0" = 0 ];
	then
	    echo "~~~~~~ Compiling CLHT";
	    cd ${CLHT_PATH};
	    ${MAKE} -k clean hyht_res lfht_res hyht_mem lfht_mem ${COMPILE_FLAGS};
	    cd -;
	    cp ${CLHT_PATH}/hyht ${CLHT_PATH}/lfht_res ${CLHT_PATH}/hyhtm ${CLHT_PATH}/lfhtm $ub;
	fi;

	make ${structure} LATENCY=$LATENCY_TYPE ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for c in $cores_lat_dist
	do
	    for i in $initials;
	    do
		r=$((2*${i}));
		for u in $updates;
		do
		    params="-n$c -i$i -r$r -u$u -d$duration";
		    dat=$out_folder/${SCY}.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "$hts" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat;
		    head -n8 $dat; echo "..."; tail -n8 $dat; 
		done;
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
	${MAKE} -k ${structure} LATENCY=$LATENCY_TYPE ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for c in $cores_lat_dist
	do
	    for i in $initials;
	    do
		r=$((2*${i}));
		for u in $updates;
		do
		    params="-n$c -i$i -r$r -u$u -d$duration";
		    dat=$out_folder/${SCY}.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "$sls" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat;
		    head -n8 $dat; echo "..."; tail -n8 $dat; 
		done;
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
	${MAKE} -k ${structure} LATENCY=$LATENCY_TYPE ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for c in $cores_lat_dist
	do
	    for i in $initials;
	    do
		r=$((2*${i}));
		for u in $updates;
		do
		    params="-n$c -i$i -r$r -u$u -d$duration";
		    dat=$out_folder/${SCY}.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "$bsts" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat; 
		    head -n8 $dat; echo "..."; tail -n8 $dat; 
		done;
	    done;
	done;
    fi;
fi;
