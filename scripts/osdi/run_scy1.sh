#!/bin/bash

cores_lat_dist="5 10 20";
LATENCY_TYPE=2			# 2 or 3
LATENCY_POINTS=16384

reps=8;
keep=median 			# min, median, or max
duration=1000;
initials="1024 4096 8192";
updates="0 1 10 20 50";
cores="all";

out_folder=data;

un=$(uname -n);
ub="bin/$un";

mkdir $ub;


ll_num=8;
ht_num=9;
i_num=$(echo $initials | wc -w);
u_num=$(echo $updates | wc -w);
source scripts/config;
c_num=$max_cores;
est_time=$(echo "2*${i_num}*${u_num}*(${ll_num}+${ht_num})*${reps}*(${duration}/1000)*${c_num}/3600" | bc -l);
printf "## Estimated time for the experiment (hours): %.3f\n" $est_time;
sleep 2;

echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Throughput";
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

# ll ##################################################################
structure=ll;
make ${structure};
mv bin/*${structure}* $ub;

echo "~~~~~~~~~~~~ Working on ${structure}";
for i in $initials;
do
    r=$((2*${i}));
    for u in $updates;
    do
	params="-i$i -r$r -u$u -d$duration";
	dat=$out_folder/scy1.${structure}.thr.$un.i$i.u$u.dat;
	echo "~~~~~~~~ $params @ $dat";
	./scripts/scalability_rep8.sh "$cores" $reps $keep ./$ub/sq-ll "./$ub/lb-ll -x1" "./$ub/lb-ll -x2" "./$ub/lb-ll -x3" ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt $params | tee $dat; 
    done;
done;

# ht ##################################################################
structure=ht;
make ${structure};
mv bin/*${structure}* $ub;

echo "~~~~~~~~~~~~ Working on ${structure}";
for i in $initials;
do
    r=$((2*${i}));
    for u in $updates;
    do
	params="-i$i -r$r -u$u -d$duration";
	dat=$out_folder/scy1.${structure}.thr.$un.i$i.u$u.dat;
	echo "~~~~~~~~ $params @ $dat";
	./scripts/scalability_rep9.sh "$cores" $reps $keep ./$ub/sq-ht "./$ub/lb-ht_gl -x1" "./$ub/lb-ht_gl -x2" "./$ub/lb-ht_gl -x3" ./$ub/lb-ht_copy ./$ub/lf-ht_rcu "./$ub/lb-ht_java -c512" ./$ub/lb-ht_tbb ./$ub/lf-ht $params | tee $dat; 
    done;
done;



echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency average";
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

# ll ##################################################################
structure=ll;
make ${structure} LATENCY=1;
mv bin/*${structure}* $ub;

echo "~~~~~~~~~~~~ Working on ${structure}";

for i in $initials;
do
    r=$((2*${i}));
    for u in $updates;
    do
	params="-i$i -r$r -u$u -d$duration";
	dat=$out_folder/scy1.${structure}.lat.$un.i$i.u$u.dat;
	echo "~~~~~~~~ $params @ $dat";
	./scripts/latency_rep8.sh "$cores" $reps $keep ./$ub/sq-ll "./$ub/lb-ll -x1" "./$ub/lb-ll -x2" "./$ub/lb-ll -x3" ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt $params | tee $dat; 
    done;
done;

# ht ##################################################################
structure=ht;
make ${structure} LATENCY=1;
mv bin/*${structure}* $ub;

echo "~~~~~~~~~~~~ Working on ${structure}";

for i in $initials;
do
    r=$((2*${i}));
    for u in $updates;
    do
	params="-i$i -r$r -u$u -d$duration";
	dat=$out_folder/scy1.${structure}.lat.$un.i$i.u$u.dat;
	echo "~~~~~~~~ $params @ $dat";
	./scripts/latency_rep9.sh "$cores" $reps $keep ./$ub/sq-ht "./$ub/lb-ht_gl -x1" "./$ub/lb-ht_gl -x2" "./$ub/lb-ht_gl -x3" ./$ub/lb-ht_copy ./$ub/lf-ht_rcu "./$ub/lb-ht_java -c512" ./$ub/lb-ht_tbb ./$ub/lf-ht $params | tee $dat; 
    done;
done;



echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
echo "~~~~~~~~~~~~ ~~~~~~~~~~~~ Latency distribution";
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

# ll ##################################################################
structure=ll;
make ${structure} LATENCY=$LATENCY_TYPE;
mv bin/*${structure}* $ub;

echo "~~~~~~~~~~~~ Working on ${structure}";

for c in $cores_lat_dist
do
    for i in $initials;
    do
	r=$((2*${i}));
	for u in $updates;
	do
	    params="-i$i -r$r -u$u -d$duration -n$c";
	    dat=$out_folder/scy1.${structure}.ldi.$un.c$c.i$i.u$u.dat;
	    echo "~~~~~~~~ $params @ $dat";
	    ./scripts/latency_raw_suc8.sh $c ./$ub/sq-ll "./$ub/lb-ll -x1" "./$ub/lb-ll -x2" "./$ub/lb-ll -x3" ./$ub/lb-ll_copy ./$ub/lf-ll_harris ./$ub/lf-ll_michael ./$ub/lf-ll_harris_opt $params -v$LATENCY_POINTS -f$LATENCY_POINTS $params | tee $dat; 
	done;
    done;
done;

# ht ##################################################################
structure=ht;
make ${structure} LATENCY=$LATENCY_TYPE;
mv bin/*${structure}* $ub;

echo "~~~~~~~~~~~~ Working on ${structure}";

for c in $cores_lat_dist
do
    for i in $initials;
    do
	r=$((2*${i}));
	for u in $updates;
	do
	    params="-i$i -r$r -u$u -d$duration -n$c";
	    dat=$out_folder/scy1.${structure}.ldi.$un.c$c.i$i.u$u.dat;
	    echo "~~~~~~~~ $params @ $dat";
	    ./scripts/latency_raw_suc8.sh $c ./$ub/sq-ht "./$ub/lb-ht_gl -x1" "./$ub/lb-ht_gl -x2" "./$ub/lb-ht_gl -x3" ./$ub/lb-ht_copy ./$ub/lf-ht_rcu "./$ub/lb-ht_java -c512" ./$ub/lb-ht_tbb ./$ub/lf-ht -v$LATENCY_POINTS -f$LATENCY_POINTS $params | tee $dat; 
	done;
    done;
done;

