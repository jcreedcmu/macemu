# Temporarily split this off to a Makefile that assumes
# Retro68 toolchain is in PATH. Should probably reunity with other Makefile

CC=m68k-apple-macos-gcc
CXX=m68k-apple-macos-g++
REZ=Rez

CFLAGS=-I/build/twelf/bin -g

Twelf.code.bin: twelf.o clikloop.o
	$(CC) $^ -o $@ $(LDFLAGS) -L/build/twelf/bin -L/build/gmp-m68k-INSTALL/lib -ltwelf -lgmp -lm

clikloop.o: clikloop.s
	$(CC) -ffunction-sections -c clikloop.s -o $@

Twelf.bin: Twelf.code.bin
	$(REZ) --copy $^ twelf.r -t "APPL" -c "TWLF" -o $@

.PHONY: clean
clean:
	rm -f Twelf.bin Twelf.APPL Twelf.dsk Twelf.code.bin Twelf.code.bin.gdb twelf.o clikloop.o
