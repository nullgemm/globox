#!/bin/bash

# get into the script's folder
cd "$(dirname "$0")"
cd ../../..

# library makefile data
name="globox"
cc="gcc"

src+=("src/globox.c")
src+=("src/globox_error.c")
src+=("src/globox_private_getters.c")
src+=("src/x11/globox_x11.c")

flags+=("-std=c99" "-pedantic")
flags+=("-Wall" "-Wextra" "-Werror=vla" "-Werror")
flags+=("-Wno-address-of-packed-member")
flags+=("-Wno-unused-parameter")
flags+=("-Isrc")
flags+=("-fPIC")

#defines+=("-DGLOBOX_ERROR_ABORT")
#defines+=("-DGLOBOX_ERROR_SKIP")
defines+=("-DGLOBOX_ERROR_LOG_THROW")

# library platform
defines+=("-DGLOBOX_PLATFORM_X11")

# generated linker arguments
link+=("xcb")

# build type
read -p "select build type ([1] debug | [2] release): " build

case $build in
	[1]* ) # debug build
flags+=("-g")
defines+=("-DGLOBOX_ERROR_LOG_BASIC")
defines+=("-DGLOBOX_ERROR_LOG_DEBUG")
	;;

	[2]* ) # release build
flags+=("-O2")
	;;
esac

# context type
read -p "select context type ([1] software | [2] egl | [3] glx): " context

case $context in
	[1]* ) # software context
makefile=makefile_lib_x11_software
src+=("src/x11/software/globox_x11_software.c")
defines+=("-DGLOBOX_CONTEXT_SOFTWARE")
link+=("xcb-shm")
link+=("xcb-randr")
link+=("xcb-render")
	;;

	[2]* ) # egl context
makefile=makefile_lib_x11_egl
src+=("src/x11/egl/globox_x11_egl.c")
defines+=("-DGLOBOX_CONTEXT_EGL")
link+=("egl")
link+=("glesv2")
	;;

	[3]* ) # glx context
makefile=makefile_lib_x11_glx
src+=("src/x11/glx/globox_x11_glx.c")
defines+=("-DGLOBOX_CONTEXT_GLX")
link+=("glx")
link+=("glesv2")
link+=("x11 x11-xcb xrender")
	;;
esac

# add the libraries as default targets
default+=("bin/$name.a")
default+=("bin/$name.so")

# makefile start
echo ".POSIX:" > $makefile
echo "NAME = $name" >> $makefile
echo "CC = $cc" >> $makefile

# makefile linking info
echo "" >> $makefile
for flag in $(pkg-config ${link[@]} --cflags); do
	echo "LDFLAGS+= $flag" >> $makefile
done

echo "" >> $makefile
for flag in $(pkg-config ${link[@]} --libs); do
	echo "LDLIBS+= $flag" >> $makefile
done

# makefile compiler flags
echo "" >> $makefile
for flag in "${flags[@]}"; do
	echo "CFLAGS+= $flag" >> $makefile
done

echo "" >> $makefile
for define in "${defines[@]}"; do
	echo "CFLAGS+= $define" >> $makefile
done

# makefile object list
echo "" >> $makefile
for file in "${src[@]}"; do
	folder=$(dirname "$file")
	filename=$(basename "$file" .c)
	echo "OBJ+= $folder/$filename.o" >> $makefile
done

# makefile default target
echo "" >> $makefile
echo "default: ${default[@]}" >> $makefile

# makefile library targets
echo "" >> $makefile
cat make/lib/templates/targets_linux.make >> $makefile

# makefile object targets
echo "" >> $makefile
for file in "${src[@]}"; do
	$cc $defines -MM -MG $file >> $makefile
done

# makefile extra targets
echo "" >> $makefile
cat make/lib/templates/targets_extra.make >> $makefile
