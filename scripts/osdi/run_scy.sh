#!/bin/bash

config_workload=./scripts/osdi/config/scy1.long
config_execs=./scripts/osdi/config/all

# example configuration ###################################

# cores_lat_dist="5 10 20"; 
# LATENCY_TYPE=3			# 2 or 3
# LATENCY_POINTS=128
# reps=3;
# keep=median 			# min, median, or max
# duration=500;
# initials="1024 4096 8192";
# updates="0 1 10 20 50";
# cores="three";
# skip_thr=1;
# skip_lat=1;


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

source ${config_workload};
source ${config_execs};

out_folder=data;

un=$(uname -n);
ub="bin/$un";
if [ ! -d "$ub" ];
then
    mkdir $ub;
fi;

printf "## Will do measurements for         : ";

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

if [ "${LATENCY_MERGE}0" -eq 10 ];
then
    ldi_print="merged";
    ldi_script=./scripts/latency_raw.sh
else
    ldi_print="seprte";
    ldi_script=./scripts/latency_raw_suc.sh
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
ll_num=$((8*$do_ll));
ht_num=$((9*$do_ht));
sl_num=$((5*$do_sl));
bst_num=$((7*$do_bst));
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
printf "   Continue? [Y/n] ";
read cont;
if [ "$cont" = "n" ];
then
    exit;
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
	make ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "./$ub/sq-ll ./$ub/lb-ll_coupling ./$ub/lb-ll_lazy ./$ub/lb-ll_pugh ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt" $params | tee $dat; 
	    done;
	done;
    fi;


    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;
	make ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "./$ub/sq-ht ./$ub/lb-ht_coupling_gl ./$ub/lb-ht_lazy_gl ./$ub/lb-ht_pugh_gl ./$ub/lb-ht_copy ./$ub/lf-ht_rcu ./$ub/lb-ht_java ./$ub/lb-ht_tbb ./$ub/lf-ht" $params | tee $dat; 
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
	make ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "./$ub/sq-sl ./$ub/lb-sl_pugh ./$ub/lb-sl_herlihy ./$ub/lf-sl_fraser ./$ub/lf-sl_herlihy" $params | tee $dat; 
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
	make ${structure} POWER=1 ${COMPILE_FLAGS} SET_CPU=1;
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.pow.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/power.sh "$cores_pow" $reps $keep "./$ub/sq-bst_internal ./$ub/sq-bst_external ./$ub/lb-bst2 ./$ub/lb-bst-drachsler ./$ub/lf-bst  ./$ub/lf-bst-howley ./$ub/lf-bst-aravind" $params | tee $dat; 
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
	make ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "./$ub/sq-ll ./$ub/lb-ll_coupling ./$ub/lb-ll_lazy ./$ub/lb-ll_pugh ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt" $params | tee $dat; 
	    done;
	done;
    fi;


    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;
	make ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "./$ub/sq-ht ./$ub/lb-ht_coupling_gl ./$ub/lb-ht_lazy_gl ./$ub/lb-ht_pugh_gl ./$ub/lb-ht_copy ./$ub/lf-ht_rcu ./$ub/lb-ht_java ./$ub/lb-ht_tbb ./$ub/lf-ht" $params | tee $dat; 
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
	make ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "./$ub/sq-sl ./$ub/lb-sl_pugh ./$ub/lb-sl_herlihy ./$ub/lf-sl_fraser  ./$ub/lf-sl_herlihy" $params | tee $dat; 
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
	make ${structure} ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";
	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.thr.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/scalability_rep.sh "$cores" $reps $keep "./$ub/sq-bst_internal ./$ub/sq-bst_external ./$ub/lb-bst2 ./$ub/lb-bst-drachsler ./$ub/lf-bst  ./$ub/lf-bst-howley ./$ub/lf-bst-aravind" $params | tee $dat; 
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
	make ${structure} LATENCY=1 ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/latency_rep.sh "$cores" $reps $keep "./$ub/sq-ll ./$ub/lb-ll_coupling ./$ub/lb-ll_lazy ./$ub/lb-ll_pugh ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt" $params | tee $dat; 
	    done;
	done;
    fi;

    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;
	make ${structure} LATENCY=1 ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/latency_rep.sh "$cores" $reps $keep "./$ub/sq-ht ./$ub/lb-ht_coupling_gl ./$ub/lb-ht_lazy_gl ./$ub/lb-ht_pugh_gl ./$ub/lb-ht_copy ./$ub/lf-ht_rcu ./$ub/lb-ht_java ./$ub/lb-ht_tbb ./$ub/lf-ht" $params | tee $dat; 
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
	make ${structure} LATENCY=1 ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/latency_rep.sh "$cores" $reps $keep "./$ub/sq-sl ./$ub/lb-sl_pugh ./$ub/lb-sl_herlihy ./$ub/lf-sl_fraser  ./$ub/lf-sl_herlihy" $params | tee $dat; 
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
	make ${structure} LATENCY=1 ${COMPILE_FLAGS};
	mv bin/*${structure}* $ub;

	echo "~~~~~~~~~~~~ Working on ${structure}";

	for i in $initials;
	do
	    r=$((2*${i}));
	    for u in $updates;
	    do
		params="-i$i -r$r -u$u -d$duration";
		dat=$out_folder/scy1.${structure}.lat.$un.i$i.u$u.dat;
		echo "~~~~~~~~ $params @ $dat";
		./scripts/latency_rep.sh "$cores" $reps $keep "./$ub/sq-bst_internal ./$ub/sq-bst_external ./$ub/lb-bst2 ./$ub/lb-bst-drachsler ./$ub/lf-bst  ./$ub/lf-bst-howley ./$ub/lf-bst-aravind" $params | tee $dat; 
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
		    dat=$out_folder/scy1.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "./$ub/sq-ll ./$ub/lb-ll_coupling ./$ub/lb-ll_lazy ./$ub/lb-ll_pugh ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat;
		    head -n8 $dat; echo "..."; tail -n8 $dat;
		done;
	    done;
	done;
    fi;

    # ht ##################################################################
    if [ $do_ht -eq 1 ];
    then
	structure=ht;
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
		    dat=$out_folder/scy1.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "./$ub/sq-ht ./$ub/lb-ht_coupling_gl ./$ub/lb-ht_lazy_gl ./$ub/lb-ht_pugh_gl ./$ub/lb-ht_copy ./$ub/lf-ht_rcu ./$ub/lb-ht_java ./$ub/lb-ht_tbb ./$ub/lf-ht" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat;
		    head -n8 $dat; echo "..."; tail -n8 $dat; 
		done;
	    done;
	done;
    fi;

    # sl ##################################################################
    if [ $do_sl -eq 1 ];
    then
	structure=sl;
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
		    dat=$out_folder/scy1.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "./$ub/sq-sl ./$ub/lb-sl_pugh ./$ub/lb-sl_herlihy ./$ub/lf-sl_fraser  ./$ub/lf-sl_herlihy" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat;
		    head -n8 $dat; echo "..."; tail -n8 $dat; 
		done;
	    done;
	done;
    fi;

    # bst ##################################################################
    if [ $do_bst -eq 1 ];
    then
	structure=bst;
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
		    dat=$out_folder/scy1.${structure}.ldi.$un.c$c.i$i.u$u.dat;
		    echo "~~~~~~~~ $params @ $dat";
		    ${ldi_script} $c "./$ub/sq-bst_internal ./$ub/sq-bst_external ./$ub/lb-bst2 ./$ub/lb-bst-drachsler ./$ub/lf-bst  ./$ub/lf-bst-howley ./$ub/lf-bst-aravind" $params -v$LATENCY_POINTS -f$LATENCY_POINTS > $dat; 
		    head -n8 $dat; echo "..."; tail -n8 $dat; 
		done;
	    done;
	done;
    fi;
fi;