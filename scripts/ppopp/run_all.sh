#!/bin/bash

sf=./scripts/ppopp

echo "--> Will use pausing:";
echo;

grep -A1 "define DO_PAUSE_TYPE" ./include/utils.h;
grep "size_t pause_" ./include/utils.h;

echo;
echo "--> And will run on the following cores:";
cores=ppopp;
. ./scripts/config;

echo $cores;
echo "    (change it in ./scripts/config line 130 if it's not OK)";

printf "\n   Continue? [Y/n] ";
read cont;
if [ "$cont" = "n" ];
then
    exit;
fi;

ds="ot map ll ht sl qu st";
for d in $ds;
do
    printf "######################## %-3s ############################################################\n" $d;
    ${sf}/run_${d}.sh skip;
done;
