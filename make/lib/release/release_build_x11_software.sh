#!/bin/bash

# get into the right folder
cd "$(dirname "$0")"
cd ../../..

./make/lib/gen/gen_x11.sh release software
make -f makefile_lib_x11_software clean
make -f makefile_lib_x11_software
