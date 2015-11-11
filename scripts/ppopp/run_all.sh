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
# echo "--> Will use pausing (defined in $pinclude):";
# echo;

# grep -A1 "define DO_PAUSE_TYPE" $pinclude;
# grep "size_t pause_" $pinclude;

cconfig=$cores;
. ./scripts/config;

echo;
echo "--> And will collect data on the following numbers of threads:";

echo $cores;
echo "    (change it in ./scripts/config line 130 if it's not OK)";


nc=$(echo $cores | wc -w);
nr=$repetitions;
na=$((3+2+6+6+4+6+3));
nw=$(echo "(1+2+5+5+5+3+3)/7" | bc -l); # avg num of workloads
ns=$(echo $duration/1000 | bc -l);
nst=$(echo "$nc*$nr*$na*$nw*$ns" | bc -l);
nht=$(echo "$nst/3600" | bc -l);

echo ""
echo "--> (assuming default workload settings)";
printf "!!> Running: %-10d   algorithms\n" $na;
printf "           : * %-10d repetitions\n" $nr;
printf "           : * %-10.2f avg #workloads/algo\n" $nw;
printf "           : * %-10.3f seconds\n" $ns;
printf "           : = %-10.0f seconds\n" $nst
printf "           : = %-10.2f hours\n" $nht;
echo "";
echo "    (you can adjust the #repetitions and the duration in ./scripts/ppopp/run.config;)";

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
