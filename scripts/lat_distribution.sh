
file_prefix=$1
shift;
prog=$1
shift;
params=$@

source scripts/lat_distribution.config
source scripts/lock_exec;
source scripts/config;

data_folder="./data"
rm ${data_folder}/${file_prefix}*

for i in $initials; do
    r=$((i*2)) 
    for u in $updates; do
	for c in ${max_cores}; do
        echo "running $prog $params -i${i} -n${c} -u${u} -r${r}"
	    out=$(${run_script} $prog $params -i${i} -n${c} -u${u} -r${r});
	    if [ $u -ne 0 ]
	    then
		echo "$out" | grep latency_put | cut -d' ' -f2-| tr ',' '\n' | sort -n | head -n -1 >> ${data_folder}/${file_prefix}_${i}_${u}_${c}_put.csv
		echo "$out" | grep latency_rem | cut -d' ' -f2-| tr ',' '\n' | sort -n | head -n -1 >> ${data_folder}/${file_prefix}_${i}_${u}_${c}_rem.csv
	    fi
	    if [ $u -ne 100 ]
	    then
		echo "$out" | grep latency_get | cut -d' ' -f2-| tr ',' '\n' | sort -n | head -n -1 >> ${data_folder}/${file_prefix}_${i}_${u}_${c}_get.csv
	    fi
	done
    done
done

source scripts/unlock_exec;
