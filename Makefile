all: fbdevcube

fbdevcube.o: fbdevcube.c
	$(CC) -g -nostdlib \
		-m68000 \
		-ffunction-sections \
		-include $(KDIR)/tools/include/nolibc/nolibc.h \
		-I easy-args/includes/ \
		-Os -c -o $@ $<

fbdevcube: fbdevcube.o
	$(CC) -g -nostdlib \
		-static \
		-Wl,-gc-sections \
		-Wl,-elf2flt=-rv \
		-o $@ $< -lgcc

clean:
	rm fbdevcube fbdevcube.o
