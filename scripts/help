# pre
# string format
# strings
# post
function print_n()
{
    printf "$1";
    for s in $3;
    do
	printf "$2" $s;
    done;
    printf "$4";
}

# pre
# repetitions
# string
# post
function print_rep()
{
    printf "$1";
    for s in $(seq 1 1 $2);
    do
	printf "$3";
    done;
    printf "$4";
}

# position
# string
# e.g. get_n 3 "1 2 X 4"; returns X
function get_n()
{
    str=$(echo "$2" | sed -e 's/^\ //g' -e 's/\ \ */ /g' -e 's/\ $//g');
    echo $(echo "$str" | cut -d" " -f$1);
}

# template
# change
function get_tmp()
{
    echo $(echo $1 | sed "s/XXX/$2/g");
}
