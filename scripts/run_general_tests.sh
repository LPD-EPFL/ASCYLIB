#!/bin/bash
MAKE=make
cores="socketandone"

#0 - median; 1 - max; 2 - min; 3 - avg
res_type=0

uname=$(uname -n);
if [ $unm = "ol-collab1" ];
then
    MAKE=gmake
fi;
if [ $# -le 1 ];		# pass any param to avoid compilation
then
    LATENCY=1 INIT=one GRANULARITY=GLOBAL_LOCK $MAKE -k tas
    LATENCY=1 INIT=one $MAKE -k tas
    LATENCY=1 INIT=one $MAKE -k lockfree
fi
source scripts/config;
source scripts/namemap.config
source scripts/lock_exec;

ll_algos="./bin/lb-ll ./bin/lb-ll_copy ./bin/lf-ll_harris ./bin/lf-ll_harris_opt ./bin/lf-ll_michael"
do_ll=1
sl_algos="./bin/lb-sl_herlihy ./bin/lf-sl"
do_sl=1
ht_algos="./bin/lb-ht_copy ./bin/lb-ht_tbb ./bin/lb-ht_copy_gl ./bin/lb-ht_gl ./bin/lb-ht ./bin/lb-ht_gl ./bin/lf-ht ./bin/lf-ht_rcu"
do_ht=1
bst_algos="./bin/lf-bst ./bin/lb-bst-drachsler ./bin/lf-bst-aravind ./bin/lf-bst-howley ./bin/lb-bst2"
do_bst=1

num_repetitions=7

#default duration
def_duration=300

#parameters for the common case experiment
base_initial=4096
base_range=8192
base_update=10

#parameters for the high contention experiment
high_initial=256
high_range=512
high_update=50
high_cores=20

#parameters for the low contention experiment
low_initial=8192
low_range=16384
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
    elif [ ${res_type} -eq 1]; then
        #max
        echo ${array1[${#array1[@]}-1]}
    elif [ ${res_type} -eq 2]; then
        #min
        echo ${array[0]}
    elif [ ${res_type} -eq 3]; then
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
        thr=$(${run_script} ${executable} ${params} | grep "Mops" | cut -d' ' -f2);
        array+=("$thr")
    done
    res=$(compute_val $array)
    echo $res
}

test_structure() {
    algos=$@
    for algo in ${algos}; do
        if [ -e $algo ]; then
        echo "starting ${algo} tests..."
#base case, all cores
        echo "   varying cores..."
        printf "      "
        for c in ${cores}; do
            printf "${c} "
            throughput=$(run_test ${algo} -d${def_duration} -n${c} -i${base_initial} -r${base_range} -u${base_update}) 
            echo "${uname} ${namemap[${algo}]} ${c} ${throughput}" >> ./data/common_${uname}.txt
        done
        printf "\n"
#hight contention case
        echo "   high contention..."
        throughput_one=$(run_test ${algo} -d${def_duration} -n1 -i${high_initial} -r${high_range} -u${high_update}) 
        throughput=$(run_test ${algo} -d${def_duration} -n${high_cores} -i${high_initial} -r${high_range} -u${high_update}) 
        scal=$(echo "${throughput}/${throughput_one}" | bc -l);
        echo "${uname} high ${namemap[${algo}]} ${throughput} ${scal}" >> ./data/extremes_${uname}.txt
#low contention case
        echo "   low contention..."
        throughput_one=$(run_test ${algo} -d${def_duration} -n1 -i${low_initial} -r${low_range} -u${low_update}) 
        throughput=$(run_test ${algo} -d${def_duration} -n${low_cores} -i${low_initial} -r${low_range} -u${low_update}) 
        scal=$(echo "${throughput}/${throughput_one}" | bc -l);
        echo "${uname} low ${namemap[${algo}]} ${throughput} ${scal}" >> ./data/extremes_${uname}.txt
        else 
            echo "$algo not found"
        fi
    done
}

echo "machine structure cores throughput" > ./data/common_${uname}.txt
echo "machine experiment structure throughput scalability" > ./data/extremes_${uname}.txt

if [ ${do_ll} -eq 1 ]; then
    test_structure ${ll_algos}
fi
if [ ${do_ht} -eq 1 ]; then
    test_structure ${ht_algos}
fi
if [ ${do_sl} -eq 1 ]; then
    test_structure ${sl_algos}
fi
if [ ${do_bst} -eq 1 ]; then
    test_structure ${bst_algos}
fi

source scripts/unlock_exec;
