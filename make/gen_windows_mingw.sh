#!/bin/bash

# get into the right folder
cd "$(dirname "$0")"
cd ..

# generate makefile
cc=x86_64-w64-mingw32-gcc

src+=("src/globox.c")
src+=("src/globox_error.c")
src+=("src/windows/globox_windows.c")

example+=("res/icon/iconpix_pe.o")

flags+=("-std=c99" "-pedantic")
flags+=("-Wall" "-Wextra" "-Werror=vla" "-Werror")
flags+=("-Wno-address-of-packed-member")
flags+=("-Wno-unused-parameter")
flags+=("-Wno-implicit-fallthrough")
flags+=("-Wno-cast-function-type")
flags+=("-Wno-incompatible-pointer-types")

flags+=("-Isrc")

defines+=("-DGLOBOX_ERROR_LOG_BASIC")
defines+=("-DGLOBOX_ERROR_LOG_THROW")
defines+=("-DGLOBOX_ERROR_LOG_DEBUG")
#defines+=("-DGLOBOX_ERROR_SKIP")
#defines+=("-DGLOBOX_ERROR_ABORT")

defines+=("-DGLOBOX_PLATFORM_WINDOWS")
defines+=("-DGLOBOX_COMPATIBILITY_WINE")
defines+=("-DUNICODE")
defines+=("-D_UNICODE")
defines+=("-DWINVER=0x0A00")
defines+=("-D_WIN32_WINNT=0x0A00")
defines+=("-DCINTERFACE")
defines+=("-DCOBJMACROS")

ldlibs+=("-lgdi32")
ldlibs+=("-ldwmapi")
ldlibs+=("-mwindows")

# build type
read -p "optimize? ([1] optimize | [2] debug): " optimize

if [ $optimize -eq 1 ]; then
flags+=("-O2")
else
flags+=("-g")
fi

# context type
read -p "select context type ([1] software | [2] egl | [3] wgl): " context

case $context in
	[1]* )
# software context
makefile=makefile_windows_software
example_src+=("example/software.c")
src+=("src/windows/software/globox_windows_software.c")
defines+=("-DGLOBOX_CONTEXT_SOFTWARE")
	;;

	[2]* )
# egl context
makefile=makefile_windows_egl
example_src+=("example/egl.c")
src+=("src/windows/egl/globox_windows_egl.c")
flags+=("-Ires/egl_headers")
defines+=("-DGLOBOX_CONTEXT_EGL")
ldflags+=("-Lres/eglproxy/lib/mingw")
ldlibs+=("-leglproxy")
ldlibs+=("-lopengl32")

make/scripts/eglproxy_get.sh
	;;

	[3]* )
# wgl context
makefile=makefile_windows_wgl
example_src+=("example/wgl.c")
src+=("src/windows/wgl/globox_windows_wgl.c")
flags+=("-Ires/egl_headers")
defines+=("-DGLOBOX_CONTEXT_WGL")
ldlibs+=("-lgdi32")
ldlibs+=("-ldwmapi")
ldlibs+=("-mwindows")
ldlibs+=("-lopengl32")

make/scripts/egl_get.sh
	;;
esac

# create empty makefile
echo ".POSIX:" > $makefile
echo "NAME = globox" >> $makefile
echo "CMD = wine ./globox.exe" >> $makefile

# generate linking info
echo "" >> $makefile
echo "CC = $cc" >> $makefile
echo "LDFLAGS+= ${ldflags[@]}" >> $makefile
echo "LDLIBS+= ${ldlibs[@]}" >> $makefile

# generate compiler flags
echo "" >> $makefile
for file in ${flags[@]}; do
	echo "CFLAGS+= $file" >> $makefile
done

echo "" >> $makefile
for file in ${defines[@]}; do
	echo "CFLAGS+= $file" >> $makefile
done

# generate object list
echo "" >> $makefile
for file in ${src[@]}; do
	folder=$(dirname "$file")
	name=$(basename "$file" .c)
	echo "OBJ+= $folder/$name.o" >> $makefile
done

for file in ${example[@]}; do
	echo "EXAMPLE+= $file" >> $makefile
done

for file in ${example_src[@]}; do
	folder=$(dirname "$file")
	name=$(basename "$file" .c)
	echo "EXAMPLE+= $folder/$name.o" >> $makefile
done

# build eglproxy and get OpenGL header when needed
if [ $context -eq 2 ]; then
echo "" >> $makefile
cat make/templates/targets_windows_mingw_egl.make >> $makefile
fi

echo "" >> $makefile
cat make/templates/targets_windows_mingw.make >> $makefile

echo "" >> $makefile
cat make/templates/targets_windows_mingw_libs.make >> $makefile

# generate object targets
echo "" >> $makefile
for file in ${src[@]}; do
	$cc $defines -MM -MG $file >> $makefile
done

for file in ${example_src[@]}; do
	$cc $defines -MM -MG $file >> $makefile
done

# generate utilitary targets
echo "" >> $makefile
cat make/templates/targets_extra.make >> $makefile
