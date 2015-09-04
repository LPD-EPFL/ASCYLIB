#!/bin/bash

INIT=one make TH=1 wf_tests

updates="20 50 100"
initials="512 2048 8192"
def_duration=5000
num=16

rm ./data/tsx_${num}*

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/tsx_${num}_ll.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-ll_lazy -n${num} -u${u} -i${i} -r${range} -d${def_duration}| grep "Thrd" | awk '{print $3}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    dev=$(echo $res | cut -d' ' -f 5)
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/tsx_${num}_ll.txt
echo $devs >> ./data/tsx_${num}_ll.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/tsx_${num}_sl.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-sl_herlihy -n${num} -u${u} -i${i} -r${range} -d${def_duration}| grep "Thrd" | awk '{print $3}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    dev=$(echo $res | cut -d' ' -f 5)
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/tsx_${num}_sl.txt
echo $devs >> ./data/tsx_${num}_sl.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/tsx_${num}_ht.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-ht_lazy_gl -n${num} -u${u} -i${i} -r${range} -d${def_duration}| grep "Thrd" | awk '{print $3}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    dev=$(echo $res | cut -d' ' -f 5)
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/tsx_${num}_ht.txt
echo $devs >> ./data/tsx_${num}_ht.txt
done


rm ./data/temp


