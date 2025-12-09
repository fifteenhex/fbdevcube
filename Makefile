fbdevcube: fbdevcube.c
	$(CC) -ffunction-sections -Wl,-gc-sections -nostdlib -static -m68000 \
		-include $(KDIR)/tools/include/nolibc/nolibc.h -Os -o $@ $< -lgcc
