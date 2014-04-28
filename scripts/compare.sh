#!/bin/bash

f=$1;

awk '// { st+=($5/$2); i++; print $2, $5, $5/$2 }; END { print "Avg: " st/i }' $f;
