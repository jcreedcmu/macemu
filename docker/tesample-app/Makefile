# path to RETRO68
RETRO68=/opt/Retro68-INSTALL

PREFIX=$(RETRO68)/m68k-apple-macos
CC=$(RETRO68)/bin/m68k-apple-macos-gcc
CXX=$(RETRO68)/bin/m68k-apple-macos-g++
REZ=$(RETRO68)/bin/Rez

CFLAGS=-I/build/twelf/bin -g
RINCLUDES=$(PREFIX)/RIncludes
REZFLAGS=-I$(RINCLUDES)

TESample.bin: TESample.code.bin
	$(REZ) $(REZFLAGS) --copy $^ TESample.r -t "APPL" -c "TSMP" -o $@

TESample.code.bin: TESample.o
	$(CC) $^ -o $@ $(LDFLAGS) -L/build/twelf/bin -L/opt/gmp-m68k-INSTALL/lib -ltwelf -lgmp -lm

.PHONY: clean
clean:
	rm -f TESample.bin TESample.APPL TESample.dsk TESample.code.bin TESample.code.bin.gdb TESample.o
