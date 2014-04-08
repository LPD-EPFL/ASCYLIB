#!/bin/bash

cores=$1;
shift;
progs=$1;
shift;

./scripts/latency_rep.sh "$cores" 1 max "$progs" $@