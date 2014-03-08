
source ./scripts/heatmap.config

num_repetitions=3
median=0

prog1=$1
shift;
prog2=$1
shift
y_axis=$1
shift;
x_axis=$1
shift;
params=$@

source scripts/config;
source scripts/lock_exec;

GREP=grep

unm=$(uname -n);
if [ $unm = "maglite" ];
then
    GREP=/opt/csw/bin/gegrep
fi;


case "${y_axis}" in
s)  quantity_1=${ranges}
    par_1="-r"
    par_1_name="range"
    ;;
u)  quantity_1=${updates}
    par_1="-u"
    par_1_name="updates"
    ;;
c)  quantity_1=${cores}
    par_1="-n"
    par_1_name="cores"
    ;;
*) echo "Format: ./heatmap.sh prog_1 prog_2 y_axis x_axis other_parameters"
    exit;
   ;;
esac

case "${x_axis}" in
s)  quantity_2=${ranges}
    par_2="-r"
    par_2_name="range"
    ;;
u)  quantity_2=${updates}
    par_2="-u"
    par_2_name="updates"
    ;;
c)  quantity_2=${cores}
    par_2="-n"
    par_2_name="cores"
    ;;
*) echo "Format: ./heatmap.sh prog_1 prog_2 y_axis x_axis other_parameters"
    exit;
   ;;
esac

function process_array()
{
    thearray=$1 
    #now sort the array
    thearray=( $(for i in ${thearray[@]}; do echo "$i"; done | sort -n) );
    if [ $median -eq 1 ]; then
        #median
        mid=$(( ${#thearray[@]}/2 ))
        if (( $#%2 == 0 )); then
            theres=$(( (${thearray[mid]}+${thearray[mid-1]})/2.0 ))
        else
            theres=${thearray[mid]}
        fi
    else
        #average
        total=$( IFS="+"; bc <<< "${thearray[*]}" )
        theres=$(echo "$total/${#thearray[@]}" | bc -l);
    fi
    echo "$theres";
}

temp_files="./data/lat_put_lb.txt ./data/lat_put_lf.txt ./data/lat_put_ratio.txt ./data/lat_get_lb.txt ./data/lat_get_lf.txt ./data/lat_get_ratio.txt ./data/lat_rem_lb.txt ./data/lat_rem_lf.txt ./data/lat_rem_ratio.txt"

for f in ${temp_files}; do
    rm "$f"
    printf "%s" ${par_1_name} >> "$f"
    printf "/%s," ${par_2_name} >> "$f"
done

for x in ${quantity_2}
do
for f in ${temp_files}; do
    printf "%-8.0f," $x >> "$f"
done
done

for f in ${temp_files}; do
    printf "\n" >> "$f"
done

for y in ${quantity_1}
do

for f in ${temp_files}; do
    printf "%-8.0f," $y >> "$f"
done

for x in ${quantity_2}
do
    init=""
    init_val=""
    if [ "${par_1}" = "-r" ]
    then
       init="-i"
       init_val=$((y/2)) 
    fi
    if [ "${par_2}" = "-r" ]
    then
       init="-i"
       init_val=$((x/2)) 
    fi

    prog=$prog1;
    
    #repeat the measurements a number of times
    array1_get=()
    array1_put=()
    array1_rem=()
    for i in `seq 1 ${num_repetitions}`
    do
        out=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val} | $GREP "srch_suc" -A 1 | grep -v "srch_suc")
        echo "$out"
        put=$(echo $out | awk '{ print $4}')
        get=$(echo $out | awk '{ print $2}')
        rem=$(echo $out | awk '{ print $6}')
        if [ $put -eq 0 ]; then
            put=1
        fi
        if [ $rem -eq 0 ]; then
            rem=1
        fi
        if [ $get -eq 0 ]; then
            get=1
        fi

        array1_get+=("$get")
        array1_put+=("$put")
        array1_rem+=("$rem")
    done
    
    lat1_get=$(process_array ${array1_get})
    lat1_put=$(process_array ${array1_put})
    lat1_rem=$(process_array ${array1_rem})
    
    
    prog=$prog2;
    #repeat the measurements a number of times
    array2_get=()
    array2_put=()
    array2_rem=()
    for i in `seq 1 ${num_repetitions}`
    do
        out=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val} | ${GREP} "srch_suc" -A 1 | grep -v "srch_suc")
        echo "$out"
        put=$(echo $out | awk '{ print $4}')
        get=$(echo $out | awk '{ print $2}')
        rem=$(echo $out | awk '{ print $6}')
        if [ $put -eq 0 ]; then
            put=1
        fi
        if [ $rem -eq 0 ]; then
            rem=1
        fi
        if [ $get -eq 0 ]; then
            get=1
        fi

        array2_get+=("$get")
        array2_put+=("$put")
        array2_rem+=("$rem")
    done
    
    lat2_get=$(process_array ${array2_get})
    lat2_put=$(process_array ${array2_put})
    lat2_rem=$(process_array ${array2_rem})

    ratio_get=$(echo "${lat2_get}/${lat1_get}" | bc -l);
    ratio_put=$(echo "${lat2_put}/${lat1_put}" | bc -l);
    ratio_rem=$(echo "${lat2_rem}/${lat1_rem}" | bc -l);
    
    printf " %.2f," $ratio_get >> ./data/lat_get_ratio.txt;
    printf " %.2f," $ratio_put >> ./data/lat_put_ratio.txt;
    printf " %.2f," $ratio_rem >> ./data/lat_rem_ratio.txt;
    printf " %.0f," $lat1_get >> ./data/lat_get_lb.txt;
    printf " %.0f," $lat1_put >> ./data/lat_put_lb.txt;
    printf " %.0f," $lat1_rem >> ./data/lat_rem_lb.txt;
    printf " %.0f," $lat2_get >> ./data/lat_get_lf.txt;
    printf " %.0f," $lat2_put >> ./data/lat_put_lf.txt;
    printf " %.0f," $lat2_rem >> ./data/lat_rem_lf.txt;

    echo "Heatmap latency status: done ${prog1} and ${prog2} with ${params} ${par_1}${y} ${par_2}${x} ${init}${init_val}"

done

for f in ${temp_files}; do
    printf "\n" >> "$f"
done
done

source scripts/unlock_exec;
