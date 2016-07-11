#!/bin/bash

updates="20 50 100"
initials="512 1024 2048 8192"
def_duration=5000
nums="16 32"

rm ./data/tsx_*

INIT=one make TSX=1 wf_tests
mv ./bin/lb-ll_lazy ./bin/tsx-lb-ll_lazy
mv ./bin/lb-ht_lazy_gl ./bin/tsx-lb-ht_lazy_gl
mv ./bin/lb-sl_herlihy ./bin/tsx-lb-sl_herlihy
mv ./bin/lb-bst_tk ./bin/tsx-lb-bst_tk
INIT=one make wf_tests

for n in ${nums}; do
for i in ${initials}; do
range=`echo "${i} * 2" | bc`
for u in ${updates}; do
    thr1=$(./bin/lb-ll_lazy -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Mops" | awk '{print $2}')
    thr2=$(./bin/tsx-lb-ll_lazy -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Mops" | awk '{print $2}')
    echo "ll ${u} ${thr1} ${thr2}" >> ./data/tsx_${n}_${i}.txt
done

for u in ${updates}; do
    thr1=$(./bin/lb-ht_lazy_gl -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Mops" | awk '{print $2}')
    thr2=$(./bin/tsx-lb-ht_lazy_gl -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Mops" | awk '{print $2}')
    echo "ht ${u} ${thr1} ${thr2}" >> ./data/tsx_${n}_${i}.txt
done

for u in ${updates}; do
    thr1=$(./bin/lb-sl_herlihy -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Mops" | awk '{print $2}')
    thr2=$(./bin/tsx-lb-sl_herlihy -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Mops" | awk '{print $2}')
    echo "sl ${u} ${thr1} ${thr2}" >> ./data/tsx_${n}_${i}.txt
done

done

done
