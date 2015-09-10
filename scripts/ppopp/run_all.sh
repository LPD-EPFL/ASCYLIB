#!/bin/bash

sf=./scripts/ppopp
. ./scripts/ppopp/run.config;

pinclude=./include/utils.h;
echo "--> Settings in ./scripts/ppopp/run.config;"
echo "--> # Repetitions      : "$repetitions;
echo "--> Duration/rep (ms)  : "$duration;
echo "--> Pin threads        : "$set_cpu;
echo "--> On cores setting   : "$cores;
echo "--> Keep value         : "$keep;
echo "--> Will use pausing (defined in $pinclude):";
echo;

grep -A1 "define DO_PAUSE_TYPE" $pinclude;
grep "size_t pause_" $pinclude;

cconfig=$cores;
. ./scripts/config;

echo;
echo "--> And will run on the following cores:";

echo $cores;
echo "    (change it in ./scripts/config line 130 if it's not OK)";

printf "\n   Continue? [Y/n] ";
read cont;
if [ "$cont" = "n" ];
then
    exit 1;
fi;

ds="ot map ll ht sl qu st";
for d in $ds;
do
    printf "######################## %-3s ############################################################\n" $d;
    ${sf}/run_${d}.sh skip;
done;
