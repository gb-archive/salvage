; 16-bit pseudo-random number generator

; John Harrison
; v1.0  2008-Mar-30
; v1.1  2008-Apr-20
;       - added MakeRandomSeedByTimingKeypress routine 

; Adapted for GB from http://map.tni.nl/sources/external/z80bits.html
; which was in turn taken from an old ZX Spectrum game and slightly optimised.

; Pseudo random numbers are generated with a linear congruential generator,
; perhaps the most common and established simple method of generating
; pseudo-random numbers.

; The random number generator has a period of 65536. The biggest challenge
; in using it is to pick a good seed. One method is to run a counter
; between user keypresses and use the results from the counter as your
; seed.

; To use this generator:
;   1. seed the generator by putting as random a word as you can find in
;      [RandSeed]

;   2. call Rand16 to generate a random word in HL
;
; inputs: none
; output: HL = random word
; preserves all registers, except HL which contains the random word

include "standard-defs.inc"
include	"incdecss.inc"
include "keypad.asm"

;***************************************************************************
;* Rand16: Generate pseudo-random word
;* input: none
;* output: hl = pseudo-random word
;* note: before first call to Rand16, set random seed word called RandSeed
;* with a reasonably random number, or you will always generate the same
;* pseudo-random numbers from this function
;***************************************************************************

	LoWordVar	RandSeed
Rand16	push	af
	push	de
	ld	a,[RandSeed]		; LSB first
	ld	e,a
	ld	a,[RandSeed+1]
	ld	d,a

	ld	h,e
	ld	l,253
	push	af			; save a
; hl = hl - de:
; do 2s complement of de and add:
	DEC_DE
	ld	a,d
	cpl
	ld	d,a
	ld	a,e
	cpl
	ld	e,a
	pop	af			; restore a
	add	hl,de
	sbc	a,0			; a = a - CY
	jr	nc,.nosubtract
	DEC_HL				; carry flag was set
.nosubtract:
	add	hl,de

	ld	d,0
	sbc	a,d
	ld	e,a		
	jr	nc,.nosub2
	DEC_HL
.nosub2
; hl = hl - de:
; do 2s complement of de and add:
	DEC_DE
	ld	a,d
	cpl
	ld	d,a
	ld	a,e
	cpl
	ld	e,a
	add	hl,de

	jr	nc,.Rand
	inc	hl
.Rand	ld	a,l
	ld	[RandSeed],a
	ld	a,h
	ld	[RandSeed+1],a
	pop	de
	pop	af
	ret

;***************************************************************************
;* MakeRandomSeedByTimingKeypress
;* run a tight loop, incrementing a counter, waiting for a keypress.
;* When the keypress happens, write the value of the counter as
;* the random seed, considering that value to be reasonably random.
;* input: none
;* output: none
;***************************************************************************

MakeRandomSeedByTimingKeypress:
	push	hl
	push	bc
	push	af
	ld	hl,0
	ld	b,20
.WaitForKeyrelease
	call	GetKeys
	INC_HL
	jr	nz,.WaitForKeyrelease
	dec	b
	jr	nz,.WaitForKeyrelease

	ld	b,20

.WaitForKeypress
	call	GetKeys
	INC_HL
	jr	z,.WaitForKeypress
	dec	b
	jr	nz,.WaitForKeypress

; save the seed
	ld	a,l
	ld	[RandSeed],a
	ld	a,h
	ld	[RandSeed+1],a
	pop	af
	pop	bc
	pop	hl
	ret