;***************************************************************************
;* intmath.asm
;* integer math routines for Game Boy
;* by John Harrison (so far)
;
; very incomplete...routines written on an "as needed" basis
;
; 2008-Apr-1 --- V0.1 

        IF      !DEF(INTMATH_ASM)
INTMATH_ASM  SET  1

include "standard-defs.inc"
	LoWordVar	_MD16temp
	LoByteVar	_MD16count

;***************************************************************************
;* divide16
;* divide 16 bits register with 16 bit register
;* adapted from code found at http://www.devrs.com/gb/asmcode.php#asmmath
;* and posted by Jeff Frohwein
;* input:  DE = divisor
;*         BC = dividend (i.e. calculating DE/BC)
;* output: DE = result (DE/BC)
;*         BC = remainder
;***************************************************************************

divide16:
	push	af
	push	hl

        ld      hl,_MD16temp
        ld      [hl],c
        inc     hl
        ld      [hl],b
        inc     hl
        ld      [hl],17
        ld      bc,0
.nxtbit:
        ld      hl,_MD16count
        ld      a,e
        rla
        ld      e,a
        ld      a,d
        rla
        ld      d,a
        dec     [hl]
        jr     z,.done
        ld      a,c
        rla
        ld      c,a
        ld      a,b
        rla
        ld      b,a
        dec     hl
        dec     hl
        ld      a,c
        sub     [hl]
        ld      c,a
        inc     hl
        ld      a,b
        sbc     a,[hl]
        ld      b,a
        jr      nc,.noadd

        dec     hl
        ld      a,c
        add     a,[hl]
        ld      c,a
        inc     hl
        ld      a,b
        adc     a,[hl]
        ld      b,a
.noadd:
        ccf
        jr      .nxtbit
.done	pop	hl
	pop	af
	ret

	ENDC