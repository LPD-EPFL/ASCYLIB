#!/bin/bash

at=./scripts/ppopp

echo "## Plotting: begin";

have_gp=$(which gnuplot);
if [ "${have_gp}0" = 0 ];
then
    echo "## !! WARNING: no gnuplot installation found. The plots cannot be created."
    exit;
fi;

cd ${at}/plots;
make -k fixed;

figs=$(ls eps/fig*);
figss=$(echo "$figs" | sed 's/^eps\///g');

echo "## Plotting: end";
echo "## Figures in ${at}/plots/eps";
echo "## Available figures #############";
for f in $figss;
do
    printf "#    %-27s #\n" $f;
done
echo "##################################"
cd - > /dev/null;
