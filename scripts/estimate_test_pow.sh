#!/bin/bash

duration=1000;

NC=$(nproc);
echo -- $NC cores / $duration ms;


echo "";
echo "static __attribute__ ((unused)) double eng_per_test_iter_nj[$NC][5] = ";
echo "  {";

for c in $(seq 1 1 $NC)
do
    res=$(sudo ./bin/test -d$duration -n$c);
    # echo "$res";
    pow_tot=$(echo "$res" | awk '/Total power/ { print $6 }');
    pow_pac=$(echo "$res" | awk '/Package power/ { print $6 }');
    pow_ppo=$(echo "$res" | awk '/PowerPlane0 power/ { print $6 }');
    pow_ram=$(echo "$res" | awk '/DRAM power/ { print $6 }');
    pow_rst=$(echo "$res" | awk '/Rest power/ { print $6 }');
    # echo $pow_tot $pow_pac $pow_ppo $pow_ram $pow_rst;

    thr=$(echo "$res" | grep "#txs" | cut -d'(' -f2);
    # echo $thr;

    ene_tot=$(echo 1000000000*$pow_tot/$thr | bc -l);
    ene_pac=$(echo 1000000000*$pow_pac/$thr | bc -l);
    ene_ppo=$(echo 1000000000*$pow_ppo/$thr | bc -l);
    ene_ram=$(echo 1000000000*$pow_ram/$thr | bc -l);
    ene_rst=$(echo 1000000000*$pow_rst/$thr | bc -l);
    echo "    {" $ene_tot, $ene_pac, $ene_ppo, $ene_ram, $ene_rst "},";
done;

echo "  };";
