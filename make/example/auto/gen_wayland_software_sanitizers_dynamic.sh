#!/bin/bash

# get into the right folder
cd "$(dirname "$0")"
cd ../gen

echo -e "3\n1\n2\n" | ./gen_wayland.sh
