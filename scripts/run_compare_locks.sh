#!/bin/bash

p="lb-ll -x2 -u10";
./scripts/run_compare.sh $p
p="lb-ll -x2 -u10 -i128 -r256";
./scripts/run_compare.sh $p
p="lb-sl  -u10";
./scripts/run_compare.sh $p
p="-u10 -i128 -r256";
./scripts/run_compare.sh lb-sl $p
p="-u10 -l4";
./scripts/run_compare.sh lb-ht $p
p="-u10 -l4 -x2"
./scripts/run_compare.sh lb-ht $p

