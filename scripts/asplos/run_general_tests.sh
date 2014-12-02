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

ll_algos="./${ub}/lb-ll_lazy ./${ub}/lb-ll_coupling ./${ub}/lb-ll_pugh ./${ub}/lb-ll_copy ./${ub}/lf-ll_harris ./${ub}/lf-ll_harris_opt ./${ub}/lf-ll_michael ./${ub}/sq-ll"
do_ll=0
sl_algos="./${ub}/lb-sl_herlihy ./${ub}/lb-sl_pugh ./${ub}/lf-sl_fraser ./${ub}/lf-sl_herlihy  ./${ub}/sq-sl"
do_sl=1
ht_algos="./${ub}/lb-ht_tbb ./${ub}/lb-ht_java ./${ub}/lb-ht_copy ./${ub}/lb-ht_lazy_gl ./${ub}/lb-ht_coupling_gl ./${ub}/lb-ht_pugh_gl ./${ub}/lf-ht_harris ./${ub}/lf-ht_rcu ./${ub}/sq-ht"
do_ht=0
bst_algos="./${ub}/lf-bst_ellen ./${ub}/lb-bst-drachsler ./${ub}/lf-bst-aravind ./${ub}/lf-bst-howley ./${ub}/lb-bst_bronson ./${ub}/sq-bst_external ./${ub}/sq-bst_internal"
do_bst=0

num_repetitions=11

#default duration
def_duration=5000

#parameters for the common case experiment
base_initial=4096
base_range=8192
base_update=10

#parameters for the high contention experiment
high_initial=512
high_range=1024
high_update=25
high_cores=20

#parameters for the low contention experiment
low_initial=16384
low_range=32768
low_update=10
low_cores=20

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
    echo "cores " > ./data/common_gp_${struct}_${uname}.txt
    for c in ${cores}; do
        echo "$c" >> ./data/common_gp_${struct}_${uname}.txt
    done
    echo "machine structure cores throughput" > ./data/common_${struct}_${uname}.txt
    echo "machine experiment structure throughput scalability" > ./data/extremes_${struct}_${uname}.txt
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
            echo "${uname} ${namemap[${algo}]} ${c} ${throughput}" >> ./data/common_${struct}_${uname}.txt
            echo "${throughput}" >> ./data/temp
        done
        paste -d' ' ./data/common_gp_${struct}_${uname}.txt ./data/temp > ./data/temp2
        mv ./data/temp2 ./data/common_gp_${struct}_${uname}.txt
        printf "\n"
#hight contention case
        echo "   high contention..."
        throughput_one=$(run_test ${algo} -d${def_duration} -n1 -i${high_initial} -r${high_range} -u${high_update}) 
        throughput=$(run_test ${algo} -d${def_duration} -n${high_cores} -i${high_initial} -r${high_range} -u${high_update}) 
        scal=$(echo "${throughput}/${throughput_one}" | bc -l);
        echo "${uname} high ${namemap[${algo}]} ${throughput} ${scal}" >> ./data/extremes_${struct}_${uname}.txt
#low contention case
        echo "   low contention..."
        throughput_one=$(run_test ${algo} -d${def_duration} -n1 -i${low_initial} -r${low_range} -u${low_update}) 
        throughput=$(run_test ${algo} -d${def_duration} -n${low_cores} -i${low_initial} -r${low_range} -u${low_update}) 
        scal=$(echo "${throughput}/${throughput_one}" | bc -l);
        echo "${uname} low ${namemap[${algo}]} ${throughput} ${scal}" >> ./data/extremes_${struct}_${uname}.txt
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
