all: fbdevcube

lb1sf68.o: lb1sf68.S
	$(CC) -g -nostdlib \
		-m68000 \
		-D L_mulsi3 \
		-D L_divsi3 \
		-D L_udivsi3 \
		-ffunction-sections \
		-Os -c -o $@ $<

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
