#!/bin/bash

if [ $# -eq 0 ];
then
    echo "Usage: $0 \"cores\" \"executable1 excutable2 ...\" [params]";
    echo "  where \"cores\" can be the actual thread num to use, such as \"1 10 20\", or"
    echo "  one of the predefined specifications for that platform (e.g., socket -- see "
    echo "  scripts/config)";
    exit;
fi;

cores=$1;
shift;
progs=$1;
shift;

./scripts/scalability_rep.sh "$cores" 1 max "$progs" $@
