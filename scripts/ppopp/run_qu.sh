#!/bin/bash

ds=qu;

. ./scripts/ppopp/run.config

do_thr=1;
do_ldi=1;

skip=$#;

algos=( ./${ub}/lf-qu_ms ${ub}/lb-qu_ms ${ub}/lb-qu_optik0 ${ub}/lb-qu_optik1 ${ub}/lb-qu_optik2 ${ub}/lb-qu_optik3 );

param_i=65534;
params_p=( 40 50 60 );
params_nc=( 10 );		# for latency ditribution
np=${#params_p[*]};

cores_backup=$cores;
. ./scripts/config;

nc=$(echo "$cores" | wc -w);
dur_s=$(echo $duration/1000 | bc -l);
na=${#algos[@]};

dur_thr=$(echo "$do_thr*$na*$np*$nc*$repetitions*$dur_s" | bc -l);
nc_ldi=${#params_nc[@]};
dur_ldi=$(echo "$do_ldi*$na*$np*$nc_ldi*$repetitions*$dur_s" | bc -l);
dur_tot=$(echo "$dur_thr+$dur_ldi" | bc -l);

tf=( false true );
printf "#> measure throughput: %-5s / ldi: %-5s\n" ${tf[$do_thr]} ${tf[$do_ldi]}
printf "#> $na algos, $np params, $nc cores, $repetitions reps of %.2f sec = %.2f sec\n" $dur_s $dur_tot;
printf "#> = %.2f hours\n" $(echo $dur_tot/3600 | bc -l);

if [ $skip -eq 0 ];
then
    printf "   Continue? [Y/n] ";
    read cont;
    if [ "$cont" = "n" ];
    then
	exit;
    fi;
fi;

cores=$cores_backup;
algos_str="${algos[@]}";


if [ $do_thr -eq 1 ];
then
    echo "########################################### Throughput";

    if [ $do_compile -eq 1 ];
    then
	ctarget=${ds}ppopp;
	cflags="SET_CPU=$set_cpu";
	echo "----> Compiling" $ctarget " with flags:" $cflags;
	make $ctarget $cflags >> /dev/null;
	if [ $? -eq 0 ];
	then
	    echo "----> Success!"
	fi;
	echo "----> Moving binaries to $ub";
	mkdir $ub &> /dev/null;
	mv bin/*${ds}* $ub;
	if [ $? -eq 0 ];
	then
	    echo "----> Success!"
	fi;
    fi;

    for ((i=0; i < $np; i++))
    do
	initial=$param_i;
	put=${params_p[$i]};
	if [ $fixed_file_dat -ne 1 ];
	then
	    out="$unm.${ds}.thr.p$put.dat"
	else
	    out="data.${ds}.thr.p$put.dat"
	fi;
	echo "### params -i$initial -p$put / keep $keep of reps $repetitions of dur $duration" | tee ${uo}/$out;

	./scripts/scalability_rep.sh $cores $repetitions $keep "$algos_str" -d$duration -i$initial -p$put \
	    | tee -a ${uo}/$out;
    done;
fi;

if [ $do_ldi -eq 1 ];
then
    echo "########################################### Latency distribution";

    if [ $do_compile -eq 1 ];
    then
	ctarget=${ds}ppopp;
	cflags="SET_CPU=$set_cpu LATENCY=6";
	echo "----> Compiling" $ctarget " with flags:" $cflags;
	make $ctarget $cflags >> /dev/null;
	if [ $? -eq 0 ];
	then
	    echo "----> Success!"
	fi;
	echo "----> Moving binaries to $ub";
	mkdir $ub &> /dev/null;
	mv bin/*${ds}* $ub;
	if [ $? -eq 0 ];
	then
	    echo "----> Success!"
	fi;
    fi;

    for ((n=0; n < $nc_ldi; n++))
    do
	for ((i=0; i < $np; i++))
	do
	    initial=$param_i;
	    put=${params_p[$i]};
	    nc=${params_nc[$n]};
	    if [ $fixed_file_dat -ne 1 ];
	    then
		out="$unm.${ds}.ldi.n$nc.p$put.dat"
	    else
		out="data.${ds}.ldi.n$nc.p$put.dat"
	    fi;
	    echo "### params -i$initial -p$put -n$nc / dur $duration" | tee ${uo}/$out;

	    ./scripts/scalability_ldi.sh $nc "$algos_str" -d$duration -i$initial -p$put \
		| tee -a ${uo}/$out;
	done;
    done;
fi;
