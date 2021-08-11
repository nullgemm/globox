#!/bin/bash

# get into the script's folder
cd "$(dirname "$0")"
cd ../../..

tag=$(git tag --sort v:refname | tail -n 1)

# versions
ver_windows=10
ver_windows_sdk=10.0.19041.0
ver_msvc=14.29.30037
ver_visual_studio=2019

# library makefile data
cc="\"/c/Program Files (x86)/Microsoft Visual Studio/\
$ver_visual_studio/BuildTools/VC/Tools/MSVC/\
$ver_msvc/bin/Hostx64/x64/cl.exe\""

lib="\"/c/Program Files (x86)/Microsoft Visual Studio/\
$ver_visual_studio/BuildTools/VC/Tools/MSVC/\
$ver_msvc/bin/Hostx64/x64/lib.exe\""

obj+=("res/icon/iconpix_pe.obj")

flags+=("-Zc:inline")
flags+=("-Iglobox_bin_$tag/include")
flags+=("-I\"/c/Program Files (x86)/Windows Kits/\
$ver_windows/Include/$ver_windows_sdk/ucrt\"")
flags+=("-I\"/c/Program Files (x86)/Windows Kits/\
$ver_windows/Include/$ver_windows_sdk/um\"")
flags+=("-I\"/c/Program Files (x86)/Windows Kits/\
$ver_windows/Include/$ver_windows_sdk/shared\"")
flags+=("-I\"/c/Program Files (x86)/Microsoft Visual Studio/\
$ver_visual_studio/BuildTools/VC/Tools/MSVC/$ver_msvc/include\"")

defines+=("-DGLOBOX_PLATFORM_WINDOWS")
defines+=("-DUNICODE")
defines+=("-D_UNICODE")
defines+=("-DWINVER=0x0A00")
defines+=("-D_WIN32_WINNT=0x0A00")
defines+=("-DCINTERFACE")
defines+=("-DCOBJMACROS")

# generated linker arguments
ldflags+=("-SUBSYSTEM:windows")
ldflags+=("-LIBPATH:\"/c/Program Files (x86)/Windows Kits/\
$ver_windows/Lib/$ver_windows_sdk/um/x64\"")
ldflags+=("-LIBPATH:\"/c/Program Files (x86)/Microsoft Visual Studio/\
$ver_visual_studio/BuildTools/VC/Tools/MSVC/$ver_msvc/lib/spectre/x64\"")
ldflags+=("-LIBPATH:\"/c/Program Files (x86)/Windows Kits/\
$ver_windows/Lib/$ver_windows_sdk/ucrt/x64\"")

ldlibs+=("Gdi32.lib")
ldlibs+=("User32.lib")
ldlibs+=("shcore.lib")
ldlibs+=("dwmapi.lib")

drmemory+=("-report_max -1")
drmemory+=("-report_leak_max -1")
drmemory+=("-batch")

# build type
read -p "select build type ([1] development | [2] release): " build

case $build in
	[1]* ) # development build
flags+=("-g")
defines+=("-DGLOBOX_ERROR_LOG_BASIC")
defines+=("-DGLOBOX_ERROR_LOG_DEBUG")
	;;

	[2]* ) # release build
flags+=("-O2")
	;;
esac

# context type
read -p "select context type ([1] software | [2] egl | [3] wgl): " context

case $context in
	[1]* ) # software context
makefile=makefile_example_windows_software_native
name="globox_example_windows_software_native"
globox="globox_windows_software"
src+=("example/software.c")
defines+=("-DGLOBOX_CONTEXT_SOFTWARE")
	;;

	[2]* ) # egl context
makefile=makefile_example_windows_egl_native
name="globox_example_windows_egl_native"
globox="globox_windows_egl"
src+=("example/egl.c")
flags+=("-Ires/egl_headers")
defines+=("-DGLOBOX_CONTEXT_EGL")
ldflags+=("-LIBPATH:res/eglproxy/lib/msvc")
ldlibs+=("eglproxy.lib")
ldlibs+=("opengl32.lib")
default+=("res/egl_headers")
default+=("bin/eglproxy.dll")
	;;

	[3]* ) # wgl context
makefile=makefile_example_windows_wgl_native
name="globox_example_windows_wgl_native"
globox="globox_windows_wgl"
src+=("example/wgl.c")
flags+=("-Ires/egl_headers")
defines+=("-DGLOBOX_CONTEXT_WGL")
ldlibs+=("opengl32.lib")
default+=("res/egl_headers")
esac

# link globox
ldflags+=("-LIBPATH:\"globox_bin_$tag/lib/globox/windows\"")
ldlibs+=($globox"_msvc.lib")

# default target
cmd="./$name.exe"
default+=("bin/\$(NAME)")

# makefile start
echo ".POSIX:" > $makefile
echo "NAME = $name" >> $makefile
echo "CMD = $cmd" >> $makefile
echo "CC = $cc" >> $makefile
echo "LIB = $lib" >> $makefile

# makefile linking info
echo "" >> $makefile
for flag in "${ldflags[@]}"; do
	echo "LDFLAGS+= $flag" >> $makefile
done

echo "" >> $makefile
for flag in "${ldlibs[@]}"; do
	echo "LDLIBS+= $flag" >> $makefile
done

# makefile compiler flags
echo "" >> $makefile
for flag in "${flags[@]}"; do
	echo "CFLAGS+= $flag" >> $makefile
done

echo "" >> $makefile
for define in ${defines[@]}; do
	echo "CFLAGS+= $define" >> $makefile
done

# makefile object list
echo "" >> $makefile
for file in ${src[@]}; do
	folder=$(dirname "$file")
	filename=$(basename "$file" .c)
	echo "OBJ+= $folder/$filename.obj" >> $makefile
done

echo "" >> $makefile
for prebuilt in ${obj[@]}; do
	echo "OBJ_EXTRA+= $prebuilt" >> $makefile
done

# generate Dr.Memory flags
echo "" >> $makefile
for flag in ${drmemory[@]}; do
	echo "DRMEMORY+= $flag" >> $makefile
done

# makefile default target
echo "" >> $makefile
echo "default: ${default[@]}" >> $makefile

# makefile linux targets
echo "" >> $makefile
cat make/example/templates/targets_windows_msvc.make >> $makefile

# makefile object targets
echo "" >> $makefile
for file in ${src[@]}; do
	x86_64-w64-mingw32-gcc $defines -MM -MG $file >> $makefile
done

# makefile extra targets
echo "" >> $makefile
cat make/example/templates/targets_extra.make >> $makefile
