#!/bin/bash

name=( tic int sep );

for (( i=0; i<3; i++ ))
do
    make optik_test OPTIK=$i $@;
    mv bin/optik_test bin/optik_test_${name[$i]};
done
