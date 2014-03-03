#!/bin/bash
./scripts/heatmap_c_u_th.sh $@
./scripts/heatmap_lat_all.sh $1 # only 1 param, should not prohibit compilation
./scripts/lat_distribution_all.sh

if [ $# -gt 0 ];
then
    executable=$(echo $1 | sed 's/.*scripts\///g' | sed 's/\.config//g');
else
    executable=executable;
fi

unm=$(uname -n);
ts="${unm}_${executable}_$(date +%Y_%m_%d_%H_%M)"
mkdir -p archive/$ts

cp data/${unm}* archive/$ts/

