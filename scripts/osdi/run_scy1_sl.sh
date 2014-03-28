#!/bin/bash

configuration=./scripts/osdi/config/scy1.long

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
    configuration="$1";
    shift;
    echo "# Using configuration file: $configuration";
    if [ ! -f "$configuration" ];
    then
	echo "* File does not exist: $configuration";
	exit;
    fi;
fi;

source ${configuration};

out_folder=data;

un=$(uname -n);
ub="bin/$un";
if [ ! -d "$ub" ];
then
    mkdir $ub;
fi;


echo "## Will do measurements for:";
metrics=2;
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

do_ldi=1;
if [ "${skip_ldi}0" -eq 10 ];
then
    do_ldi=0;
else
    printf "  LATENCY DISTRIBUTION";
fi;
echo "";


# estimate the time to execute the experiment
ll_num=8;
ht_num=9;
sl_num=5;
i_num=$(echo $initials | wc -w);
u_num=$(echo $updates | wc -w);
source scripts/config;

c_num=$(echo "1 "${cores} | wc -w);
est_time_thr_lat=$(echo "${metrics}*${i_num}*${u_num}*(${sl_num})*${reps}*(${duration}/1000)*${c_num}/3600" | bc -l);

c_num_ldi=$(echo $cores_lat_dist | wc -w);
est_time_ldi=$(echo "${do_ldi}*${c_num_ldi}*${i_num}*${u_num}*(${sl_num})*(${duration}/1000)/3600" | bc -l);
est_time=$(echo "${est_time_thr_lat}+${est_time_ldi}" | bc -l);
printf "## Estimated time for the experiment: %6.3f h\n" $est_time;
printf "   Continue? [Y/n] ";
read cont;
if [ "$cont" = "n" ];
then
    exit;
fi;
#############################################


if [ ! "${skip_thr}0" -eq 10 ];
then

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Throughput";
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    # sl ##################################################################
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


if [ ! "${skip_lat}0" -eq 10 ];
then

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency average";
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    # sl ##################################################################
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

if [ ! "${skip_ldi}0" -eq 10 ];
then

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency distribution";
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    # sl ##################################################################
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
		./scripts/latency_raw_suc.sh $c "./$ub/sq-sl ./$ub/lb-sl_pugh ./$ub/lb-sl_herlihy ./$ub/lf-sl_fraser  ./$ub/lf-sl_herlihy" $params -v$LATENCY_POINTS -f$LATENCY_POINTS | tee $dat | head -n32; 
	    done;
	done;
    done;

fi;
