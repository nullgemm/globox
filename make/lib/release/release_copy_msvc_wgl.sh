#!/bin/bash

# get into the right folder
cd "$(dirname "$0")"
cd ../../..

tag=$(git tag --sort v:refname | tail -n 1)
release=globox_bin_$tag

mkdir -p "$release/lib/globox/windows"
mv bin/globox.lib $release/lib/globox/windows/globox_windows_wgl_msvc.lib
mv bin/globox.dll $release/lib/globox/windows/globox_windows_wgl_msvc.dll