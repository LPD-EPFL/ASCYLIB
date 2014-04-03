#!/bin/bash

scp tadavid@lpdserver:./code/synchrobench/data/common_gp* .
scp tadavid@lpdserver:./code/synchrobench/data/extremes* .
scp tudor@parsasrv1:./synchrobench/data/common_gp* .
scp tudor@parsasrv1:./synchrobench/data/extremes* .
scp tudor@diassrv8:./synchrobench/data/common_gp* .
scp tudor@diassrv8:./synchrobench/data/extremes* .

gnuplot bst.gp
gnuplot ht.gp
gnuplot ll.gp
gnuplot sl.gp


