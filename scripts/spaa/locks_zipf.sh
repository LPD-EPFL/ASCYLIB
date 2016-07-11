#!/bin/bash

INIT=one make ZIPF=1 WSTATS=1 wf_tests

updates="10"
initials="2048"
def_duration=5000
#cycles_duration=`echo "${def_duration} * 2800000" | bc`
cycles_duration=100

rm ./data/locks_zipf_*

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/locks_zipf_ll.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-ll_lazy -n20 -u${u} -i${i} -r${range} -d${def_duration} | grep "waiting" | awk '{print $7}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    th=`echo "${th} * 100 / ${cycles_duration}" | bc`
    dev=$(echo $res | cut -d' ' -f 5)
    dev=`echo "${dev} * 100 / ${cycles_duration}" | bc`
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/locks_zipf_ll.txt
echo $devs >> ./data/locks_zipf_ll.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/locks_zipf_sl.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-sl_herlihy -n20 -u${u} -i${i} -r${range} -d${def_duration} | grep "waiting" | awk '{print $7}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    th=`echo "${th} * 100 / ${cycles_duration}" | bc`
    dev=$(echo $res | cut -d' ' -f 5)
    dev=`echo "${dev} * 100 / ${cycles_duration}" | bc`
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/locks_zipf_sl.txt
echo $devs >> ./data/locks_zipf_sl.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/locks_zipf_ht.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-ht_lazy_gl -n20 -u${u} -i${i} -r${range} -d${def_duration} | grep "waiting" | awk '{print $7}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    th=`echo "${th} * 100 / ${cycles_duration}" | bc`
    dev=$(echo $res | cut -d' ' -f 5)
    dev=`echo "${dev} * 100 / ${cycles_duration}" | bc`
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/locks_zipf_ht.txt
echo $devs >> ./data/locks_zipf_ht.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/locks_zipf_bst.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-bst_tk -n20 -u${u} -i${i} -r${range} -d${def_duration} | grep "waiting" | awk '{print $7}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    th=`echo "${th} * 100 / ${cycles_duration}" | bc`
    dev=$(echo $res | cut -d' ' -f 5)
    dev=`echo "${dev} * 100 / ${cycles_duration}" | bc`
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/locks_zipf_bst.txt
echo $devs >> ./data/locks_zipf_bst.txt
done


rm ./data/temp


