#!/bin/bash

# get into the script's folder
cd "$(dirname "$0")" || exit
cd ../../..

# params
build=$1
backend=$2

function syntax {
echo "syntax reminder: $0 <build type> <backend type>"
echo "build types: development, release, sanitized"
echo "backend types: software, glx, egl, vulkan"
}

# utilitary variables
tag=$(git tag --sort v:refname | tail -n 1)
output="make/output"
name_lib="globuf_wayland"

# ninja file variables
folder_ninja="build"
folder_objects="\$builddir/obj"
folder_globuf="globuf_bin_$tag"
folder_library="\$folder_globuf/lib/globuf"
folder_include="\$folder_globuf/include"
name="globuf_example_complex_wayland"
cmd="./\$name"
cc="gcc"
ld="gcc"
as="as"

# compiler flags
flags+=("-std=c99" "-pedantic")
flags+=("-Wall" "-Wextra" "-Werror=vla" "-Werror")
flags+=("-Wformat")
flags+=("-Wformat-security")
flags+=("-Wno-address-of-packed-member")
flags+=("-Wno-unused-parameter")
flags+=("-I\$folder_include")
flags+=("-Iexample/helpers")
flags+=("-Ires/cursoryx/include")
flags+=("-Ires/dpishit/include")
flags+=("-Ires/willis/include")
ldflags+=("-z noexecstack")
defines+=("-DGLOBUF_EXAMPLE_WAYLAND")
#defines+=("-DGLOBUF_EXAMPLE_LOG_ALL")

# common sources
src+=("res/wayland_headers/xdg-shell-protocol.c")
src+=("res/wayland_headers/xdg-decoration-protocol.c")
src+=("res/wayland_headers/kde-blur-protocol.c")
src+=("res/wayland_headers/zwp-relative-pointer-protocol.c")
src+=("res/wayland_headers/zwp-pointer-constraints-protocol.c")

# customize depending on the chosen build type
if [ -z "$build" ]; then
	build=development
fi

case $build in
	development)
flags+=("-g")
	;;

	release)
flags+=("-D_FORTIFY_SOURCE=2")
flags+=("-fstack-protector-strong")
flags+=("-fPIE")
flags+=("-fPIC")
flags+=("-O2")
ldflags+=("-z relro")
ldflags+=("-z now")
	;;

	sanitized_memory)
flags+=("-g")
flags+=("-O1")
flags+=("-fno-omit-frame-pointer")
flags+=("-fno-optimize-sibling-calls")

flags+=("-fsanitize=leak")
flags+=("-fsanitize-recover=all")

ldflags+=("-fsanitize=leak")
ldflags+=("-fsanitize-recover=all")
	;;

	sanitized_undefined)
flags+=("-g")
flags+=("-O1")
flags+=("-fno-omit-frame-pointer")
flags+=("-fno-optimize-sibling-calls")

flags+=("-fsanitize=undefined")
flags+=("-fsanitize-recover=all")

ldflags+=("-fsanitize=undefined")
ldflags+=("-fsanitize-recover=all")
	;;

	sanitized_address)
flags+=("-g")
flags+=("-O1")
flags+=("-fno-omit-frame-pointer")
flags+=("-fno-optimize-sibling-calls")

flags+=("-fsanitize=address")
flags+=("-fsanitize-address-use-after-scope")
flags+=("-fsanitize-recover=all")

ldflags+=("-fsanitize=address")
ldflags+=("-fsanitize-address-use-after-scope")
ldflags+=("-fsanitize-recover=all")
	;;

	sanitized_thread)
flags+=("-g")
flags+=("-O1")
flags+=("-fno-omit-frame-pointer")
flags+=("-fno-optimize-sibling-calls")

flags+=("-fsanitize=thread")
flags+=("-fsanitize-recover=all")

ldflags+=("-fsanitize=thread")
ldflags+=("-fsanitize-recover=all")
	;;

	*)
echo "invalid build type"
syntax
exit 1
	;;
esac

# customize depending on the chosen backend type
if [ -z "$backend" ]; then
	backend=software
fi

case $backend in
	software)
ninja_file=example_complex_wayland_software.ninja
src+=("example/complex/software.c")
libs+=("\$folder_library/globuf_elf_software.a")
	;;

	egl)
ninja_file=example_complex_wayland_egl.ninja
src+=("example/complex/opengl.c")
link+=("egl")
link+=("glesv2")
link+=("wayland-egl")
obj+=("\$folder_objects/res/shaders/gl1/shaders.o")
libs+=("\$folder_library/globuf_elf_opengl.a")
defines+=("-DGLOBUF_EXAMPLE_EGL")
	;;

	vulkan)
ninja_file=example_complex_wayland_vulkan.ninja
src+=("example/complex/vulkan.c")
src+=("example/helpers/vulkan_helpers.c")
link+=("vulkan")
obj+=("\$folder_objects/res/shaders/vk1/shaders.o")
libs+=("\$folder_library/globuf_elf_vulkan.a")
	;;

	*)
echo "invalid backend"
syntax
exit 1
	;;
esac

link+=("wayland-client")
link+=("wayland-cursor")
link+=("xkbcommon")
ldlibs+=("-lpthread")

# additional object files
obj+=("\$folder_objects/res/icon/iconpix.o")
obj+=("\$folder_objects/res/cursor/cursorpix.o")
libs+=("\$folder_library/wayland/$name_lib""_$backend.a")
libs+=("\$folder_library/wayland/$name_lib""_common.a")
libs+=("\$folder_library/globuf_elf.a")
libs+=("res/cursoryx/lib/cursoryx/wayland/cursoryx_wayland.a")
libs+=("res/cursoryx/lib/cursoryx/cursoryx_elf.a")
libs+=("res/dpishit/lib/dpishit/wayland/dpishit_wayland.a")
libs+=("res/dpishit/lib/dpishit/dpishit_elf.a")
libs+=("res/willis/lib/willis/wayland/willis_wayland.a")
libs+=("res/willis/lib/willis/willis_elf.a")

# default target
default+=("\$builddir/\$name")

# valgrind flags
valgrind+=("--show-error-list=yes")
valgrind+=("--show-leak-kinds=all")
valgrind+=("--track-origins=yes")
valgrind+=("--leak-check=full")
valgrind+=("--suppressions=../res/valgrind.supp")

# objcopy flags
objcopy+=("-I binary")
objcopy+=("-O elf64-x86-64")
objcopy+=("-B i386:x86-64")

# ninja start
mkdir -p "$output"

{ \
echo "# vars"; \
echo "builddir = $folder_ninja"; \
echo "folder_objects = $folder_objects"; \
echo "folder_globuf = $folder_globuf"; \
echo "folder_library = $folder_library"; \
echo "folder_include = $folder_include"; \
echo "name = $name""_$backend"; \
echo "cmd = $cmd"; \
echo "cc = $cc"; \
echo "ld = $ld"; \
echo "as = $as"; \
echo ""; \
} > "$output/$ninja_file"

# ninja flags
echo "# flags" >> "$output/$ninja_file"

echo -n "flags =" >> "$output/$ninja_file"
for flag in "${flags[@]}"; do
	echo -ne " \$\n$flag" >> "$output/$ninja_file"
done
echo -e "\n" >> "$output/$ninja_file"

echo -n "defines =" >> "$output/$ninja_file"
for define in "${defines[@]}"; do
	echo -ne " \$\n$define" >> "$output/$ninja_file"
done
echo -e "\n" >> "$output/$ninja_file"

echo -n "ldflags =" >> "$output/$ninja_file"
for flag in $(pkg-config "${link[@]}" --cflags) "${ldflags[@]}"; do
	echo -ne " \$\n$flag" >> "$output/$ninja_file"
done
echo -e "\n" >> "$output/$ninja_file"

echo -n "ldlibs =" >> "$output/$ninja_file"
for flag in $(pkg-config "${link[@]}" --libs) "${ldlibs[@]}"; do
	echo -ne " \$\n$flag" >> "$output/$ninja_file"
done
echo -e "\n" >> "$output/$ninja_file"

echo -n "valgrind =" >> "$output/$ninja_file"
for flag in "${valgrind[@]}"; do
	echo -ne " \$\n$flag" >> "$output/$ninja_file"
done
echo -e "\n" >> "$output/$ninja_file"

echo -n "objcopy =" >> "$output/$ninja_file"
for flag in "${objcopy[@]}"; do
	echo -ne " \$\n$flag" >> "$output/$ninja_file"
done
echo -e "\n" >> "$output/$ninja_file"

# ninja rules
{ \
echo "# rules"; \
echo "rule cc"; \
echo "    deps = gcc"; \
echo "    depfile = \$out.d"; \
echo "    command = \$cc \$flags \$defines -MMD -MF \$out.d -c \$in -o \$out"; \
echo "    description = cc \$out"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule ld"; \
echo "    command = \$ld \$ldflags -o \$out \$in \$ldlibs"; \
echo "    description = ld \$out"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule pixmap"; \
echo "    command = make/scripts/pixmap.sh"; \
echo "    description = pixmap \$out"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule icon_object"; \
echo "    command = \$as -Ires/icon -c res/icon/iconpix_elf.S -o \$out"; \
echo "    description = \$as \$out"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule cursor_object"; \
echo "    command = \$as -Ires/cursor -c res/cursor/cursorpix_elf.S -o \$out"; \
echo "    description = \$as \$out"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule shaders_object_gl1"; \
echo "    command = \$as -Ires/shaders/gl1 -c res/shaders/gl1/shaders_elf.S -o \$out"; \
echo "    description = \$as \$out"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule shaders_object_vk1"; \
echo "    command = \$as -Ires/shaders/vk1 -c res/shaders/vk1/shaders_elf.S -o \$out"; \
echo "    description = \$as \$out"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule leak"; \
echo "    command = cd \$builddir \$"; \
echo "    && valgrind \$valgrind 2> valgrind.log \$cmd \$"; \
echo "    && less valgrind.log"; \
echo "    description = running valgrind \$in"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule run"; \
echo "    command = cd \$builddir && \$cmd"; \
echo "    description = running \$in"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule clean"; \
echo "    command = make/scripts/clean.sh"; \
echo "    description = cleaning repo"; \
echo ""; \
} >> "$output/$ninja_file"

{ \
echo "rule generator"; \
echo "    command = make/lib/wayland_$backend.sh $build $backend"; \
echo "    description = re-generating the ninja build file"; \
echo ""; \
} >> "$output/$ninja_file"

# ninja targets
## compile sources
echo "# compile sources" >> "$output/$ninja_file"
for file in "${src[@]}"; do
	folder=$(dirname "$file")
	filename=$(basename "$file" .c)
	obj+=("\$folder_objects/$folder/$filename.o")
	{ \
	echo "build \$folder_objects/$folder/$filename.o: \$"; \
	echo "cc $file"; \
	echo ""; \
	} >> "$output/$ninja_file"
done

## main targets
{ \
echo "# main targets"; \
echo "build res/icon/iconpix.bin: pixmap"; \
echo "build res/cursor/cursorpix.bin: pixmap"; \
echo "build \$folder_objects/res/icon/iconpix.o: \$"; \
echo "icon_object res/icon/iconpix.bin"; \
echo "build \$folder_objects/res/cursor/cursorpix.o: \$"; \
echo "cursor_object res/cursor/cursorpix.bin"; \
echo ""; \
echo "build \$folder_objects/res/shaders/gl1/shaders.o: \$"; \
echo "shaders_object_gl1 res/shaders/gl1/square_vert_gl1.glsl res/shaders/gl1/square_frag_gl1.glsl"; \
echo ""; \
echo "build \$folder_objects/res/shaders/vk1/shaders.o: \$"; \
echo "shaders_object_vk1 res/shaders/vk1/square_vert_vk1.spv res/shaders/vk1/square_frag_vk1.spv"; \
echo ""; \
} >> "$output/$ninja_file"

echo "# archive objects" >> "$output/$ninja_file"
echo -n "build \$builddir/\$name: ld" >> "$output/$ninja_file"
for file in "${obj[@]}" "${libs[@]}"; do
	echo -ne " \$\n$file" >> "$output/$ninja_file"
done
echo -e "\n" >> "$output/$ninja_file"

## special targets
{ \
echo "# run special targets"; \
echo "build leak: leak \$builddir/\$name"; \
echo "build run: run \$builddir/\$name"; \
echo "build regen: generator"; \
echo "build clean: clean"; \
echo "default" "${default[@]}"; \
} >> "$output/$ninja_file"
