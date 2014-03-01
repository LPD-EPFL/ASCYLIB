#!/bin/bash

#param1 : which executables.config file to use
#param2 : pass any value to prohibit compilation

./scripts/heatmap_c_u_th.sh $@
./scripts/heatmap_lat_all.sh $@

ts=$(date +%Y_%m_%d_%H_%M)
mkdir -p archive/$ts

unm=$(uname -n);
cp data/${unm}* archive/$ts/

