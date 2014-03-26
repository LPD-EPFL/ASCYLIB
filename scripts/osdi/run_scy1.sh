#!/bin/bash

core_lat_dist=20;

duration=2000;
initials="1024 4096 8192";
updates="0 1 10 20 50";
cores="all";

out_folder=data;

un=$(uname -n);
ub="bin/$un";

mkdir $ub;

echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Throughput";

make ll;
mv bin/*ll* $ub;

echo "~~~~~~~~~~~~ Working on ll";
# "./$ub/lb-ll -x1" "./$ub/lb-ll -x2" "./$ub/lb-ll -x3" ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt

for i in $initials;
do
    r=$((2*${i}));
    for u in $updates;
    do
	params="-i$i -r$r -u$u -d$duration";
	dat=$out_folder/scy1.thr.$un.i$i.u$u.dat;
	echo "~~~~~~~~ $params @ $dat";
	./scripts/scalability8.sh "$cores" ./$ub/sq-ll "./$ub/lb-ll -x1" "./$ub/lb-ll -x2" "./$ub/lb-ll -x3" ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt $params | tee $dat; 
    done;
done;

echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency average";

make ll LATENCY=1;
mv bin/*ll* $ub;

echo "~~~~~~~~~~~~ Working on ll";

for i in $initials;
do
    r=$((2*${i}));
    for u in $updates;
    do
	params="-i$i -r$r -u$u -d$duration";
	dat=$out_folder/scy1.lat.$un.i$i.u$u.dat;
	echo "~~~~~~~~ $params @ $dat";
	./scripts/latency8.sh "$cores" ./$ub/sq-ll "./$ub/lb-ll -x1" "./$ub/lb-ll -x2" "./$ub/lb-ll -x3" ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt $params | tee $dat; 
    done;
done;

echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency distribution";

make ll LATENCY=3;
mv bin/*ll* $ub;

echo "~~~~~~~~~~~~ Working on ll";

for i in $initials;
do
    r=$((2*${i}));
    for u in $updates;
    do
	params="-i$i -r$r -u$u -d$duration";
	dat=$out_folder/scy1.ldi.$un.i$i.u$u.dat;
	echo "~~~~~~~~ $params @ $dat";
	./scripts/latency_raw_suc8.sh $core_lat_dist ./$ub/sq-ll "./$ub/lb-ll -x1" "./$ub/lb-ll -x2" "./$ub/lb-ll -x3" ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt $params -v1000 -f1000 | tee $dat; 
    done;
done;


