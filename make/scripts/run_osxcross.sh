#!/bin/bash

# get into the right folder
cd "$(dirname "$0")" || exit
cd ../..

./make/lib/gen/gen_macos.sh release software osxcross osxcross

make -f make/output/makefile_lib_macos_software clean

if [[ -v AR ]]; then
	make -f make/output/makefile_lib_macos_software -e AR="$AR"
else
	make -f make/output/makefile_lib_macos_software
fi