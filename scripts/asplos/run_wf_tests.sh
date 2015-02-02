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
    INIT=one GRANULARITY=GLOBAL_LOCK $MAKE -k $LOCK
    INIT=one $MAKE -k $LOCK
    INIT=one $MAKE -k seq
    INIT=one $MAKE -k lockfree
    INIT=one $MAKE -k htrcu
fi

source scripts/config;
source scripts/namemap.config
source scripts/lock_exec;

ll_algos=./${ub}/lb-ll_lazy
do_ll=1
sl_algos=./${ub}/lb-sl_herlihy
do_sl=1
ht_algos=./${ub}/lb-ht_lazy_gl
do_ht=1
bst_algos=./${ub}/lb-bst_tk
do_bst=1

num_repetitions=11

#default duration
def_duration=5000

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

updates="0 10 50"
initials="512 2048 8192"

test_structure() {
    struct=$1
    shift
    algo=$1
    rm ./data/wf_common_${struct}_${uname}.txt
    for init in ${initials}; do
    range=`echo "${init} * 2" | bc`
    echo "cores 0% 10% 50%" >> ./data/wf_common_${struct}_${uname}.txt

    if [ -e $algo ]; then
        echo "starting ${algo} tests..."
        echo "   varying cores..."
        for c in ${cores}; do
        printf "%s " ${c} >> ./data/wf_common_${struct}_${uname}.txt
        for u in ${updates}; do
            printf "${algo} ${c} ${u} ${init} ${range}\n"
            throughput=$(run_test ${algo} -d${def_duration} -n${c} -i${init} -r${range} -u${u}) 
            echo $throughput
            printf "%s " ${throughput} >> ./data/wf_common_${struct}_${uname}.txt
        done
        printf "\n" >> ./data/wf_common_${struct}_${uname}.txt
        done
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
