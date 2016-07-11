#!/bin/bash

updates="20 50 100"
initials="512 1024 2048 8192"
def_duration=5000
nums="16 32"

rm ./data/locked_tsx_*

INIT=one make TSX=1 TSXSTATS=1 wf_tests

for n in ${nums}; do
for i in ${initials}; do
range=`echo "${i} * 2" | bc`
for u in ${updates}; do
    frac=$(./bin/lb-ll_lazy -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Fraction" | awk '{print $4}')
    echo "ll ${u} ${frac}" >> ./data/locked_tsx_${n}_${i}.txt
done

for u in ${updates}; do
    frac=$(./bin/lb-ht_lazy_gl -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Fraction" | awk '{print $4}')
    echo "ht ${u} ${frac}" >> ./data/locked_tsx_${n}_${i}.txt
done

for u in ${updates}; do
    frac=$(./bin/lb-sl_herlihy -n${n} -u${u} -i${i} -r${range} -d${def_duration}| grep "Fraction" | awk '{print $4}')
    echo "sl ${u} ${frac}" >> ./data/locked_tsx_${n}_${i}.txt
done

done

done
