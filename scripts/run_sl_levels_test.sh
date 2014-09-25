#!/bin/sh

out_file="data/sl_28_levels_"$(date | gawk '// {print $2"_"$3}');
echo "Will write results in"$out_file;
updates="0 1 10";
test="../bin/lb-sl_pugh"

touch $out_file;
rm -f $out_file;
touch $out_file;


for u in $updates
do
	settings="-n19 -d30000 -i200000000 -u$u";
	echo $settings | tee -a $out_file;
	for i in "1 2 3 4 5"
	do
		$test $settings | grep Mops | tee -a $out_file;
	done;
done;