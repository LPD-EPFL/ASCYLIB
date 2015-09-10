#!/bin/bash

at=./scripts/ppopp

echo "## Running experiments: begin";

${at}/run_all.sh;

echo "## Running experiments: end";
echo "## Plotting: begin";

cd ${at}/plots;
make -k fixed;

figs=$(ls eps/fig*);
figss=$(echo "$figs" | sed 's/^eps\///g');

echo "## Plotting: end";
echo "## Figures in ${at}/plots/eps";
echo "## Available figures #############";
for f in $figs;
do
    printf "#    %-27s #\n" $f;
done
echo "##################################"
cd -;

