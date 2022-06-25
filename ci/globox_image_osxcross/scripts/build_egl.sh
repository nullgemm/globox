#!/bin/sh

git clone https://github.com/nullgemm/globox.git
cd ./globox || exit

# use macOS SDK
export PATH=/scripts/sdk/osxcross/target/bin/:/bin:/sbin:/usr/bin:/usr/sbin
export LD_LIBRARY_PATH=/scripts/sdk/osxcross/target/lib/

# test build
make makefile_release_lib_macos_egl
make build_lib_macos_egl
