.text
.globl ASMCLIKLOOP

ASMCLIKLOOP:
			movem.l		%d1-%d2/%a1,-(%sp)
			clr.l			-(%sp)
			jsr			GETOLDCLIKLOOP
			movea.l		(%sp)+,%a0
			movem.l		(%sp)+,%d1-%d2/%a1

			jsr			(%a0)

			movem.l		%d1-%d2/%a1,-(%sp)
			jsr			PASCALCLIKLOOP
			movem.l		(%sp)+,%d1-%d2/%a1
			moveq			#1,%d0
			rts
