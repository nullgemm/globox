bin/$(NAME).exe: $(OBJ)
	mkdir -p $(@D)
	$(CC) -Febin/$(NAME) $^ -link $(LDFLAGS) $(LDLIBS)

res/icon/iconpix.bin:
	make/scripts/pixmap_bin.sh

res/icon/iconpix_pe.obj: res/icon/iconpix.bin
	objcopy -I binary -O pe-x86-64 -B i386:x86-64 \
	--redefine-syms=res/icon/syms.map \
	--rename-section .data=.iconpix \
	$< $@
