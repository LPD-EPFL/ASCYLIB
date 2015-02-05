#!/bin/bash

INIT=one make SLOW=1 TH=1 wf_tests

updates="1 10 50"
initials="512 2048 8192"
def_duration=5000

rm ./data/throughputs_slow_*

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/throughputs_slow_ll.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-ll_lazy -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "Thrd" | awk '{print $3}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    dev=$(echo $res | cut -d' ' -f 5)
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/throughputs_slow_ll.txt
echo $devs >> ./data/throughputs_slow_ll.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/throughputs_slow_sl.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-sl_herlihy -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "Thrd" | awk '{print $3}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    dev=$(echo $res | cut -d' ' -f 5)
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/throughputs_slow_sl.txt
echo $devs >> ./data/throughputs_slow_sl.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/throughputs_slow_ht.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-ht_lazy_gl -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "Thrd" | awk '{print $3}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    dev=$(echo $res | cut -d' ' -f 5)
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/throughputs_slow_ht.txt
echo $devs >> ./data/throughputs_slow_ht.txt
done

for i in ${initials}; do
range=`echo "${i} * 2" | bc`
echo "1%Updates 10%Updates 50%Updates" >> ./data/throughputs_slow_bst.txt
thrs=""
devs=""
for u in ${updates}; do
    ./bin/lb-bst_tk -n20 -u${u} -i${i} -r${range} -d${def_duration}| grep "Thrd" | awk '{print $3}' > ./data/temp
    res=$(./scripts/podc/avg_std_dev.py -i ./data/temp)
    th=$(echo $res | cut -d' ' -f 2)
    dev=$(echo $res | cut -d' ' -f 5)
    thrs="$thrs $th"
    devs="$devs $dev"
done
echo $thrs >> ./data/throughputs_slow_bst.txt
echo $devs >> ./data/throughputs_slow_bst.txt
done


rm ./data/temp


