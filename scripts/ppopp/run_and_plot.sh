#!/bin/bash

at=./scripts/ppopp

have_gp=$(which gnuplot);
if [ "${have_gp}0" = 0 ];
then
    echo "## !! WARNING: no gnuplot installation found. The plots cannot be created."
    echo "               you can later create the graphs manually using ./scipts/ppopp/plot.sh";
    echo "";
fi;

echo "## Running experiments: begin";

${at}/run_all.sh;

if [ $? -ne 0 ];
then
    echo "## Abort!"; exit;
fi;


echo "## Running experiments: end";

${at}/plot.sh;
