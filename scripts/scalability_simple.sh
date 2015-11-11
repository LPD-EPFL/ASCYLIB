#!/bin/bash

cores="$1";
shift;

./scripts/scalability_rep_simple.sh "$cores" 1 max "$@";
