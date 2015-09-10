#!/bin/bash

sf=./scripts/ppopp

pinclude=./include/utils.h;
echo "--> Will use pausing (defined in $pinclude):";
echo;

grep -A1 "define DO_PAUSE_TYPE" $pinclude;
grep "size_t pause_" $pinclude;

echo;
echo "--> And will run on the following cores:";
. ./scripts/ppopp/run.config;
cconfig=$cores;
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
