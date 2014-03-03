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

rm ./data/temp1.txt
rm ./data/temp2.txt
rm ./data/temp3.txt
rm ./data/scal_temp.txt
rm ./data/scal_data.txt

printf "%s" ${par_1_name}
printf "/%s," ${par_2_name}
printf "%s" ${par_1_name} >> ./data/temp1.txt
printf "/%s," ${par_2_name} >> ./data/temp1.txt
printf "%s" ${par_1_name} >> ./data/temp2.txt
printf "/%s," ${par_2_name} >> ./data/temp2.txt
printf "%s" ${par_1_name} >> ./data/temp3.txt
printf "/%s," ${par_2_name} >> ./data/temp3.txt
printf "%s" ${par_1_name} >> ./data/scal_temp.txt
printf "/%s," ${par_2_name} >> ./data/scal_temp.txt
printf "%s" ${par_1_name} >> ./data/scal_data.txt
printf "/%s," ${par_2_name} >> ./data/scal_data.txt

for x in ${quantity_2}
do
    printf "%-8.0f," $x;
    printf "%-8.0f," $x >> ./data/temp1.txt;
    printf "%-8.0f," $x >> ./data/temp2.txt;
    printf "%-8.0f," $x >> ./data/temp3.txt;
    printf "%-8.0f," $x >> ./data/scal_temp.txt;
    printf "%-8.0f," $x >> ./data/scal_data.txt;
done
printf "\n" 
printf "\n" >> ./data/temp1.txt
printf "\n" >> ./data/temp2.txt
printf "\n" >> ./data/temp3.txt
printf "\n" >> ./data/scal_temp.txt
printf "\n" >> ./data/scal_data.txt

for y in ${quantity_1}
do
printf "%-8.0f," $y
printf "%-8.0f," $y >> ./data/temp1.txt
printf "%-8.0f," $y >> ./data/temp2.txt
printf "%-8.0f," $y >> ./data/temp3.txt
printf "%-8.0f," $y >> ./data/scal_temp.txt
printf "%-8.0f," $y >> ./data/scal_data.txt
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
    array1=()
    for i in `seq 1 ${num_repetitions}`
    do
        thrp1=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val}  | grep "Mops" | cut -d' ' -f2);
        array1+=("$thrp1")
    done
    #now sort the array
    array1=( $(for i in ${array1[@]}; do echo "$i"; done | sort -n) );
    if [ $median -eq 1 ]; then
        #median
        mid=$(( ${#array1[@]}/2 ))
        if (( $#%2 == 0 )); then
            thr=$(( (${array1[mid]}+${array1[mid-1]})/2.0 ))
        else
            thr=${array1[mid]}
        fi
    else
        #average
        total=$( IFS="+"; bc <<< "${array1[*]}" )
        thr=$(echo "$total/${#array1[@]}" | bc -l);
    fi

    if [ $x -eq 1 ]
    then
        thr_1_1=$thr
    fi
    scal1=$(echo "$thr/${thr_1_1}" | bc -l);

    prog=$prog2;
    #repeat the measurements a number of times
    array2=()
    for i in `seq 1 ${num_repetitions}`
    do
        thrp2=$(${run_script} $prog $params ${par_1}${y} ${par_2}${x} ${init}${init_val} | grep "Mops" | cut -d' ' -f2);
        array2+=("$thrp2")
    done
    #now sort the array
    array2=( $(for i in ${array2[@]}; do echo "$i"; done | sort -n) );
    if [ $median -eq 1 ]; then
        #median
        mid=$(( ${#array2[@]}/2 ))
        if (( $#%2 == 0 )); then
            thrt2=$(( (${array2[mid]}+${array2[mid-1]})/2.0 ))
        else
            thrt2=${array2[mid]}
        fi
    else
        #average
        total=$( IFS="+"; bc <<< "${array2[*]}" )
        thrt2=$(echo "$total/${#array2[@]}" | bc -l);
    fi

    if [ $x -eq 1 ]
    then
        thr_2_1=$thrt2
    fi
    scal2=$(echo "$thrt2/${thr_2_1}" | bc -l);

    ratio=$(echo "100* $thrt2/$thr" | bc -l);
    ratio_scal=$(echo "$scal2/$scal1" | bc -l);
    
    printf "%-8.0f," $ratio;
    printf " %.2f," $thr >> ./data/temp1.txt
    printf " %.2f," $thrt2 >> ./data/temp2.txt
    printf " %.1f/%.1f," $thrt2 $thr >> ./data/temp3.txt
    printf "%.1f," ${ratio_scal} >> ./data/scal_temp.txt;
    printf " %.1f/%.1f," $scal2 $scal1 >> ./data/scal_data.txt

done
printf "\n"
printf "\n" >> ./data/temp1.txt
printf "\n" >> ./data/temp2.txt
printf "\n" >> ./data/temp3.txt
printf "\n" >> ./data/scal_temp.txt
printf "\n" >> ./data/scal_data.txt
done

source scripts/unlock_exec;
