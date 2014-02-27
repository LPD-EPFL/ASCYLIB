#!/bin/bash

source ./scripts/heatmap.config

prog1=$1
shift;
prog2=$1
shift
y_axis=$1
shift;
x_axis=$1
shift;
params=$@

source scripts/config;
source scripts/lock_exec;

case "${y_axis}" in
s)  quantity_1=${ranges}
    par_1="-r"
    par_1_name="range"
    ;;
u)  quantity_1=${updates}
    par_1="-u"
    par_1_name="updates"
    ;;
c)  quantity_1=${cores}
    par_1="-n"
    par_1_name="cores"
    ;;
*) echo "Format: ./heatmap.sh prog_1 prog_2 y_axis x_axis other_parameters"
    exit;
   ;;
esac

case "${x_axis}" in
s)  quantity_2=${ranges}
    par_2="-r"
    par_2_name="range"
    ;;
u)  quantity_2=${updates}
    par_2="-u"
    par_2_name="updates"
    ;;
c)  quantity_2=${cores}
    par_2="-n"
    par_2_name="cores"
    ;;
*) echo "Format: ./heatmap.sh prog_1 prog_2 y_axis x_axis other_parameters"
    exit;
   ;;
esac

printf "%s" ${par_1_name}
printf "/%s," ${par_2_name}
for x in ${quantity_2}
do
    printf "%-8.0f," $x;
done
printf "\n" 

for y in ${quantity_1}
do
printf "%-8.0f," $y
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
    thr=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val}  | grep "Mops" | cut -d' ' -f2);

    prog=$prog2;
    thr1=$thr1b;

    thrt2=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val} | grep "Mops" | cut -d' ' -f2);

    ratio=$(echo "100* $thrt2/$thr" | bc -l);
    
    printf "%-8.0f," $ratio;

done
printf "\n"
done

source scripts/unlock_exec;
