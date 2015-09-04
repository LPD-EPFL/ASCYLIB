#!/bin/bash

INIT=one make ZIPF=1 STATS=1 wf_tests

updates="10"
initials="2048"
def_duration=5000

rm ./data/retries_zipf_*

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/retries_zipf_ll.txt
for u in ${updates}; do
    perc=$(./bin/lb-ll_lazy -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "parse" | awk '{print $3}');
    printf "%s%% " $perc >> ./data/retries_zipf_ll.txt
done
printf "\n" >> ./data/retries_zipf_ll.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/retries_zipf_sl.txt
for u in ${updates}; do
    perc=$(./bin/lb-sl_herlihy -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "parse" | awk '{print $3}');
    printf "%s%% " $perc >> ./data/retries_zipf_sl.txt
done
printf "\n" >> ./data/retries_zipf_sl.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/retries_zipf_ht.txt
for u in ${updates}; do
    perc=$(./bin/lb-ht_lazy_gl -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "parse" | awk '{print $3}');
    printf "%s%% " $perc >> ./data/retries_zipf_ht.txt
done
printf "\n" >> ./data/retries_zipf_ht.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/retries_zipf_bst.txt
for u in ${updates}; do
    perc=$(./bin/lb-bst_tk -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "parse" | awk '{print $3}');
    printf "%s%% " $perc >> ./data/retries_zipf_bst.txt
done
printf "\n" >> ./data/retries_zipf_bst.txt
done
