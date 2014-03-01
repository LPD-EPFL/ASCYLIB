#!/bin/bash
./scripts/heatmap_c_u_th.sh $1
./scripts/heatmap_lat_all.sh $1
./scripts/lat_distribution_all.sh

ts=$(date +%Y_%m_%d_%H_%M)
mkdir -p archive/$ts

unm=$(uname -n);
cp data/${unm}* archive/$ts/

