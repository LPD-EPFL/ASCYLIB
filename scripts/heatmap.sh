#!/bin/bash
prog1=$1
shift;
prog2=$1
shift
y_axis=$1
shift;
x_axis=$1
shift;
params=$@

unm=$(uname -n);
ranges="16 32 64 128 256 512 1024 2048 4096 8192 16384";
updates="0 1 10 20 50 100";
if [ $unm = "maglite" ];
then
cores=$(seq 8 8 64);
elif [ $unm = "parsasrv1.epfl.ch" ];
then
cores=$(seq 6 6 36);
elif [ $unm = "diassrv8" ];
then
cores=$(seq 10 10 80);
elif [ $unm = "lpdxeon2680" ];
then
cores=$(seq 10 10 20);
elif [ $unm = "lpdpc34" ];
then
cores=$(seq 2 1 8);
else
cores=$(seq 6 6 48);
fi;

if [ $unm = "parsasrv1.epfl.ch" ];
then
    run_script="./run"
fi;

source scripts/lock_exec;

case "${y_axis}" in
s)  quantity_1=${ranges}
    par_1="-r"
    ;;
u)  quantity_1=${updates}
    par_1="-u"
    ;;
c)  quantity_1=${cores}
    par_1="-n"
    ;;
*) echo "Format: ./heatmap.sh prog_1 prog_2 y_axis x_axis other_parameters"
    exit;
   ;;
esac

case "${x_axis}" in
s)  quantity_2=${ranges}
    par_2="-r"
    ;;
u)  quantity_2=${updates}
    par_2="-u"
    ;;
c)  quantity_2=${cores}
    par_2="-n"
    ;;
*) echo "Format: ./heatmap.sh prog_1 prog_2 y_axis x_axis other_parameters"
    exit;
   ;;
esac

for y in ${quantity_1}
do
for x in ${quantity_2}
do
    init=""
    init_val=""
    if [ "${par_1}" = "-r" ]
    then
       init="-i"
       init_val=$((y/2)) 
    fi
    if [ "${par_2}" = "-r" ]
    then
       init="-i"
       init_val=$((x/2)) 
    fi

    prog=$prog1;
    thr1=$thr1a;
    thr=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val}  | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);

    prog=$prog2;
    thr1=$thr1b;

    thrt2=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val} | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);

    ratio=$(echo "100* $thrt2/$thr" | bc -l);
    
    printf "%-8.0f," $ratio;

done
printf "\n"
done

source scripts/unlock_exec;
