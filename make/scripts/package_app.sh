#!/bin/bash

# get in the right folder
path="$(pwd)/$0"
folder=$(dirname "$path")
cd "$folder"/../.. || exit

# copy template app
cp -r res/app/globox.app "$1.app"

# copy the binary in the app
mkdir "$1.app/Contents/MacOS"
cp "$1" "$1.app/Contents/MacOS/globox"

# copy the icons bundle in the app
mkdir "$1.app/Contents/Resources"
cp res/app/icon.icns "$1.app/Contents/Resources/globox.icns"

# install the ANGLE dylibs in the app (and copy in the build folder)
if [ "$2" == "egl" ]; then
build=$(dirname "$1")
cp res/angle/libs/* "$build"
cp res/angle/libs/* "$1.app/Contents/MacOS/"

if [ "$3" == "native" ]; then
install_name_tool -change ./libEGL.dylib @executable_path/libEGL.dylib "$1.app/Contents/MacOS/globox"
install_name_tool -change ./libGLESv2.dylib @executable_path/libGLESv2.dylib "$1.app/Contents/MacOS/globox"
else
x86_64-apple-darwin21.4-install_name_tool -change ./libEGL.dylib @executable_path/libEGL.dylib "$1.app/Contents/MacOS/globox"
x86_64-apple-darwin21.4-install_name_tool -change ./libGLESv2.dylib @executable_path/libGLESv2.dylib "$1.app/Contents/MacOS/globox"
fi

fi

# install the MoltenVK dylibs in the app (and copy in the build folder)
if [ "$2" == "vulkan" ]; then
build=$(dirname "$1")
cp res/moltenvk/libs/* "$build"
cp res/moltenvk/libs/* "$1.app/Contents/MacOS/"

if [ "$3" == "native" ]; then
install_name_tool -change ./libMoltenVK.dylib @executable_path/libMoltenVK.dylib "$1.app/Contents/MacOS/globox"
else
x86_64-apple-darwin21.4-install_name_tool -change ./libMoltenVK.dylib @executable_path/libMoltenVK.dylib "$1.app/Contents/MacOS/globox"
fi

fi
