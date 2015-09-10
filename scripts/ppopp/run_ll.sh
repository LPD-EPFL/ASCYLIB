#!/bin/bash

ds=ll;

. ./scripts/ppopp/run.config

skip=$#;

algos=( ${ub}/lf-ll_harris_opt ${ub}/lb-ll_lazy ${ub}/lb-ll_gl ${ub}/lb-ll_optik_gl ${ub}/lb-ll_optik ${ub}/lb-ll_optik_cache );

params_i=( 128 512 2048 4096 8192 );
params_u=( 100 50  20   10   1 );
np=${#params_i[*]};

cores_backup=$cores;
. ./scripts/config;

nc=$(echo "$cores" | wc -w);
dur_s=$(echo $duration/1000 | bc -l);
na=${#algos[@]};

dur_tot=$(echo "$na*$np*$nc*$repetitions*$dur_s" | bc -l);

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
    initial=${params_i[$i]};
    update=${params_u[$i]};
    range=$((2*$initial));
    if [ $fixed_file_dat -ne 1 ];
    then
	out="$unm.${ds}.i$initial.u$update.dat"
    else
	out="data.${ds}.i$initial.u$update.dat"
    fi;

    echo "### params -i$initial -r$range -u$update / keep $keep of reps $repetitions of dur $duration" | tee ${uo}/$out;

    ./scripts/scalability_rep.sh $cores $repetitions $keep "$algos_str" -d$duration -i$initial -r$range -u$update \
				 | tee -a ${uo}/$out;
done;
