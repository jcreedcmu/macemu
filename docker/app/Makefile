# path to RETRO68
RETRO68=/Retro68-build/toolchain

PREFIX=$(RETRO68)/m68k-apple-macos
CC=$(RETRO68)/bin/m68k-apple-macos-gcc
CXX=$(RETRO68)/bin/m68k-apple-macos-g++
REZ=$(RETRO68)/bin/Rez

CFLAGS=-I/root/libtwelf
RINCLUDES=$(PREFIX)/RIncludes
REZFLAGS=-I$(RINCLUDES)

Twelf.code.bin: twelf.o
	$(CC) $< -o $@ $(LDFLAGS) -L/root/libtwelf -lgmp -ltwelf

Twelf.bin: Twelf.code.bin
	$(REZ) $(REZFLAGS) --copy $^ twelf.r -t "APPL" -c "TWLF" -o $@

.PHONY: clean
clean:
	rm -f Twelf.bin Twelf.APPL Twelf.dsk Twelf.code.bin Twelf.code.bin.gdb twelf.o
