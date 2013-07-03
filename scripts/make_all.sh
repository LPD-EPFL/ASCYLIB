#!/bin/sh

make lockfree
make ticket GRANULARITY=GLOBAL_LOCK
./scripts/bins_add_suffix.sh gl lb
make ticket