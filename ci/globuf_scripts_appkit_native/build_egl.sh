#!/bin/sh

git clone https://github.com/nullgemm/globuf.git
cd ./globuf || exit

# test build
export PATH=/scripts/sdk/target/bin/:/bin:/sbin:/usr/bin:/usr/sbin
export LD_LIBRARY_PATH=/scripts/sdk/target/lib/
./make/scripts/angle_release_get.sh
./make/scripts/build.sh release appkit egl none native
