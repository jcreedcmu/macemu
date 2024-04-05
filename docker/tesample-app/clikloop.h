/*

See clikloop.s for implementation.

The reason why we have a "clikloop" function at all is that we want to
install a callback while the TextEdit toolbox routines are handling
selection-drag-scrolling, so that we can update the scrollbar position.

The reason why it is in assembly is that the required calling
convention requires exact register control.

The reason why it is in an external assembly file and not
something like
```
pascal void AsmClikLoop() {
  asm (
         "movem.l		%d1-%d2/%a1,-(%sp)"
         ...
  );
}
```
is that declaring C functions and putting assembly in them adds a
function prologue and epilogue. The attribute `__attribute__((naked))`
(which would disable prologue/epilogue) does not seem to be supported
by Retro68's gcc. (and this isn't too shocking; support for `naked` even
in 2024 seems to be sporadic across compilers and architectures)

The reason why it is in an external assembly file and not something like
```
extern pascal void AsmClikLoop();

asm(
  "ASMCLIKLOOP:"
         "movem.l		%d1-%d2/%a1,-(%sp)"
         ...
  );
```
is that at the end of the day I judged that it wasn't really buying much
to have it all in the same file when the fact that the equivalence
    assembly ASMCLIKLOOP = C AsmClikLoop
was still being obscured by the need to declare symbol inside the
inline assembly block.

*/

extern pascal void AsmClikLoop();
