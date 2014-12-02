#!/bin/bash
MAKE=make
cores="osdi"

#0 - median; 1 - max; 2 - min; 3 - avg
res_type=0

uname=$(uname -n);
ub="bin";

LOCK=tas

timeout="timeout 30"

if [ $uname = "ol-collab1" ];
then
    MAKE=gmake
    LOCK=ticket
fi;
if [ $uname = "parsasrv1.epfl.ch" ];
then
    timeout=""
fi;

if [ $# -eq 0 ];		# pass any param to avoid compilation
then
    INIT=one SET_CPU=0 GRANULARITY=GLOBAL_LOCK $MAKE -k $LOCK
    INIT=one SET_CPU=0 $MAKE -k $LOCK
    INIT=one SET_CPU=0 $MAKE -k seq
    INIT=one SET_CPU=0 $MAKE -k lockfree
fi

source scripts/config;
source scripts/namemap.config
source scripts/lock_exec;

cores=$(seq 1 10 201)

ll_algos="./${ub}/lb-ll_lazy ./${ub}/lb-ll_coupling ./${ub}/lb-ll_pugh ./${ub}/lb-ll_copy ./${ub}/lf-ll_harris ./${ub}/lf-ll_harris_opt ./${ub}/lf-ll_michael ./${ub}/sq-ll"
do_ll=1
sl_algos="./${ub}/lb-sl_herlihy ./${ub}/lb-sl_pugh ./${ub}/lf-sl_fraser ./${ub}/lf-sl_herlihy  ./${ub}/sq-sl"
do_sl=1
ht_algos="./${ub}/lb-ht_tbb ./${ub}/lb-ht_java ./${ub}/lb-ht_copy ./${ub}/lb-ht_lazy_gl ./${ub}/lb-ht_coupling_gl ./${ub}/lb-ht_pugh_gl ./${ub}/lf-ht_harris ./${ub}/lf-ht_rcu ./${ub}/sq-ht ./${ub}/lfhtm ./${ub}/hyhtm"
do_ht=0
bst_algos="./${ub}/lf-bst_ellen ./${ub}/lb-bst-drachsler ./${ub}/lf-bst-aravind ./${ub}/lf-bst-howley ./${ub}/lb-bst_bronson ./${ub}/sq-bst_external ./${ub}/sq-bst_internal"
do_bst=1

num_repetitions=11

#default duration
def_duration=3000

#parameters for the common case experiment
base_initial=8192
base_range=16384
base_update=10

compute_val() {
    array1=$1
    #sort the array
    array1=( $(for i in ${array1[@]}; do echo "$i"; done | sort -n) );
    if [ ${res_type} -eq 0 ]; then
        #median
        mid=$(( ${#array1[@]}/2 ))
        if (( $#%2 == 0 )); then
            thr=$(( (${array1[mid]}+${array1[mid-1]})/2.0 ))
        else
            thr=${array1[mid]}
        fi
        echo $thr
    elif [ ${res_type} -eq 1 ]; then
        #max
        echo ${array1[${#array1[@]}-1]}
    elif [ ${res_type} -eq 2 ]; then
        #min
        echo ${array[0]}
    elif [ ${res_type} -eq 3 ]; then
        #avg
        total=$( IFS="+"; bc <<< "${array1[*]}" )
        thr=$(echo "$total/${#array1[@]}" | bc -l);
        echo $thr
    else
        echo "0"
    fi

}

run_test() {
    executable=$1
    shift
    params=$@
    array=()
    for i in `seq 1 ${num_repetitions}`; do 
        thr=$(${timeout} ${run_script_timeout} ${executable} ${params} | grep "Mops" | cut -d' ' -f2);
        if [ $? -eq 0 ]
        then
            array+=("$thr")
        fi
    done
   res=$(compute_val $array)
    echo $res
}

test_structure() {
    struct=$1
    shift
    algos=$@
    echo "cores " > ./data/multithreading_${struct}_${uname}.txt
    for c in ${cores}; do
        echo "$c" >> ./data/multithreading_${struct}_${uname}.txt
    done
    for algo in ${algos}; do
        if [ -e $algo ]; then
        echo "starting ${algo} tests..."
#base case, all cores
        echo "   varying cores..."
        printf "      "
        echo "${namemap[${algo}]} " > ./data/temp
        for c in ${cores}; do
            printf "${c} "
            throughput=$(run_test ${algo} -d${def_duration} -n${c} -i${base_initial} -r${base_range} -u${base_update}) 
            echo "${throughput}" >> ./data/temp
        done
        paste -d' ' ./data/multithreading_${struct}_${uname}.txt ./data/temp > ./data/temp2
        mv ./data/temp2 ./data/multithreading_${struct}_${uname}.txt
        printf "\n"
        else 
            echo "$algo not found"
        fi
    done
}

if [ ${do_ll} -eq 1 ]; then
    test_structure ll ${ll_algos}
fi
if [ ${do_ht} -eq 1 ]; then
    test_structure ht ${ht_algos}
fi
if [ ${do_sl} -eq 1 ]; then
    test_structure sl ${sl_algos}
fi
if [ ${do_bst} -eq 1 ]; then
    test_structure bst ${bst_algos}
fi

source scripts/unlock_exec;
