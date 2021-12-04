bin/$(NAME).app: bin/$(NAME)
	mv bin/$(NAME) $@

bin/$(NAME): $(OBJ) $(OBJ_EXTRA)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/libEGL.dylib: res/angle/libs
	mkdir -p $(@D)
	cp res/angle/libs/*.dylib bin/

res/angle/libs:
	make/scripts/angle_dev_get.sh

res/objconv/objconv:
	make/scripts/objconv_make.sh

res/icon/iconpix.bin:
	make/scripts/pixmap_bin.sh

res/icon/iconpix_elf.o: res/icon/iconpix.bin res/objconv/objconv
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 \
	--redefine-syms=res/icon/syms.map \
	--rename-section .data=.iconpix \
	$< $@

res/shaders/gles2/square_vert_gles2_elf.o: res/shaders/gles2/square_vert_gles2.glsl res/objconv/objconv
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 \
	--redefine-syms=res/shaders/gles2/syms.map \
	--rename-section .data=.square_vert \
	$< $@

res/shaders/gles2/square_frag_gles2_elf.o: res/shaders/gles2/square_frag_gles2.glsl res/objconv/objconv
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 \
	--redefine-syms=res/shaders/gles2/syms.map \
	--rename-section .data=.square_frag \
	$< $@

res/shaders/gles2/decorations_vert_gles2_elf.o: res/shaders/gles2/decorations_vert_gles2.glsl res/objconv/objconv
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 \
	--redefine-syms=res/shaders/gles2/syms.map \
	--rename-section .data=.decorations_vert \
	$< $@

res/shaders/gles2/decorations_frag_gles2_elf.o: res/shaders/gles2/decorations_frag_gles2.glsl res/objconv/objconv
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 \
	--redefine-syms=res/shaders/gles2/syms.map \
	--rename-section .data=.decorations_frag \
	$< $@

res/icon/iconpix_mach.o: res/icon/iconpix_elf.o
	res/objconv/objconv -fmac64 -nu+ -v0 \
	res/icon/iconpix_elf.o $@

res/shaders/gles2/square_vert_gles2_mach.o: res/shaders/gles2/square_vert_gles2_elf.o
	res/objconv/objconv -fmac64 -nu+ -v0 \
	res/shaders/gles2/square_vert_gles2_elf.o $@

res/shaders/gles2/square_frag_gles2_mach.o: res/shaders/gles2/square_frag_gles2_elf.o
	res/objconv/objconv -fmac64 -nu+ -v0 \
	res/shaders/gles2/square_frag_gles2_elf.o $@

res/shaders/gles2/decorations_vert_gles2_mach.o: res/shaders/gles2/decorations_vert_gles2_elf.o
	res/objconv/objconv -fmac64 -nu+ -v0 \
	res/shaders/gles2/decorations_vert_gles2_elf.o $@

res/shaders/gles2/decorations_frag_gles2_mach.o: res/shaders/gles2/decorations_frag_gles2_elf.o
	res/objconv/objconv -fmac64 -nu+ -v0 \
	res/shaders/gles2/decorations_frag_gles2_elf.o $@

run: bin/$(NAME)
	cd bin && $(CMD)
