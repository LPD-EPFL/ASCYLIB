#!/bin/bash



ub="./bin/$(uname -n)";
uo="scripts/ppopp/data";

do_compile=1;
set_cpu=0;

skip=$#;

algos=( ${ub}/lf-ll_harris_opt ${ub}/lf-ht_harris  ${ub}/lf-sl_fraser  );
to_make="lfll_harris_opt lfht lfsl_fraser";

repetitions=11;
duration=5000;
keep=median; #max min median

dss=( ll ht sl );
params_i=( 128 512 2048 4096 8192 );
params_u=( 100 50  20   10   1 );
params_extra[1]="-l2";
np=${#params_i[*]};

cores=ppopp;

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
    ctarget="$to_make"
    cflags="SET_CPU=$set_cpu";
    echo "----> Compiling" $ctarget " with flags:" $cflags;
    make $ctarget $cflags >> /dev/null;
    if [ $? -eq 0 ];
    then
	echo "----> Success!"
    else
	echo "----> Compilation error!"; exit;
    fi;
    echo "----> Moving binaries to $ub";
    mkdir $ub &> /dev/null;
    for b in ${algos[@]};
    do
	bb=$(echo $b | sed "s/$(uname -n)//g");
	echo "------> $bb -> $b!"
	mv $bb $b;
	if [ $? -eq 0 ];
	then
	    echo "----> Success!"
	fi;
    done;
fi;

for ((i=0; i < $np; i++))
do
 for ((a=0; a < $na; a++))
 do
    initial=${params_i[$i]};
    update=${params_u[$i]};
    range=$((2*$initial));
    pextra=${params_extra[$a]};
    ds=${dss[$a]};
    algo=${algos[$a]};
    out="$unm.${ds}.lf.i$initial.u$update.dat"
    echo "### $algo / params -i$initial -r$range -u$update $pextra / keep $keep of reps $repetitions of dur $duration" \
	| tee ${uo}/$out;

    ./scripts/scalability_rep.sh $cores $repetitions $keep "$algo" -d$duration -i$initial -r$range -u$update \
    				 | tee -a ${uo}/$out;
 done;
done;
