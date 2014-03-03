#!/bin/bash

#param1 : which executables.config file to use
#param2 : pass any value to prohibit compilation

./scripts/heatmap_c_u_th.sh $@
./scripts/heatmap_lat_all.sh $1 # only 1 param, should not prohibit compilation

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
