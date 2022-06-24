#!/bin/bash

# get into the right folder
cd "$(dirname "$0")" || exit
cd ../..

if [[ -v AR ]]; then
	make -f $1 -e AR="$AR"
else
	make -f $1
fi
