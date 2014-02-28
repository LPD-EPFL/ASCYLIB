#!/bin/bash
./scripts/heatmap_c_u_th.sh
./scripts/heatmap_lat_all.sh
./scripts/lat_distribution_all.sh

ts=$(date +%Y_%m_%d_%H_%M)
mkdir -p archive/$ts

unm=$(uname -n);
cp data/${unm}* archive/$ts/

