
# path to RETRO68
RETRO68=/opt/Retro68-INSTALL

PREFIX=$(RETRO68)/m68k-apple-macos
CC=$(RETRO68)/bin/m68k-apple-macos-gcc
CXX=$(RETRO68)/bin/m68k-apple-macos-g++
REZ=$(RETRO68)/bin/Rez

CFLAGS=-I/build/twelf/bin -g
RINCLUDES=$(PREFIX)/RIncludes
REZFLAGS=-I$(RINCLUDES)

Twelf.bin: Twelf.code.bin
	$(REZ) $(REZFLAGS) --copy $^ twelf.r -t "APPL" -c "TWLF" -o $@

-include Makefile.deps

# remove multiversal-stubs.o if we use universal interface
OBJS = \
	 twelf.o \
	 about.o \
	 apple-event-handlers.o \
	 asmclikloop.o \
	 clikloop.o \
    debug.o \
	 dialogs.o \
    document.o \
	 events.o \
	 global-state.o \
	 handlers.o \
	 menus.o \
	 multiversal-stubs.o \
	 file-ops.o \
	 scrolling.o \
	 view-rects.o \
	 windows.o \
    console.o \
    framework.o \
	 #

Makefile.deps:
	gcc $(CFLAGS) -MM *.c > $@

Twelf.code.bin: $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) -L/build/twelf/bin -L/opt/gmp-m68k-INSTALL/lib -ltwelf -lgmp -lm

asmclikloop.o: asmclikloop.s
	$(CC) -ffunction-sections -c asmclikloop.s -o $@

.PHONY: clean
clean:
	rm -f Twelf.bin Twelf.APPL Twelf.dsk Twelf.code.bin Twelf.code.bin.gdb $(OBJS)
