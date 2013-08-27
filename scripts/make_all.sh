#!/bin/sh

make lockfree;
make ticket GRANULARITY=GLOBAL_LOCK;
./scripts/bins_add_suffix.sh gl_ticket lb
make ticket
./scripts/bins_add_suffix.sh ticket lb

