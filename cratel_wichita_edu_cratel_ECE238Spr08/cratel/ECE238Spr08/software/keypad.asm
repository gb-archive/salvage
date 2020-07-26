;* keypad.asm
;* code for standard keypad stuff
;
;* by John Harrison (so far)

;* 2008-Mar-30 --- V1.0

        IF      !DEF(KEYPAD_ASM)
KEYPAD_ASM  SET  1

;***************************************************************************
;* ; GetKeys: adapted from APOCNOW.ASM and gbspec.txt
;* input: none
;* output: a = keypad number
;*         z flag reset if no key hit, set if key is hit
;* preserves all registers except A and F
;***************************************************************************

GetKeys:                 ;gets keypress
	push	bc
	ld 	a,P1F_5			; set bit 5
	ld 	[rP1],a			; select P14 by setting it low. See gbspec.txt lines 1019-1095
	ld 	a,[rP1]
 	ld 	a,[rP1]			; wait a few cycles
	cpl				; complement A. "You are a very very nice Accumulator..."
	and 	$0f			; look at only the first 4 bits
	swap 	a			; move bits 3-0 into 7-4
	ld 	b,a			; and store in b

 	ld	a,P1F_4			; select P15
 	ld 	[rP1],a
	ld	a,[rP1]
	ld	a,[rP1]
	ld	a,[rP1]
	ld	a,[rP1]
	ld	a,[rP1]
	ld	a,[rP1]			; wait for the bouncing to stop
	cpl				; as before, complement...
 	and $0f				; and look only for the last 4 bits
 	or b				; combine with the previous result
	pop	bc
 	ret				; do we need to reset joypad? (gbspec line 1082)


WaitForKeypress:	MACRO
	push	bc
	ld	b,20
.WaitForKeypress\@
	call	GetKeys
	jr	z,.WaitForKeypress\@
	dec	b
	jr	nz,.WaitForKeypress\@
	pop	bc
	ENDM

WaitForKeyrelease:	MACRO
	push	bc
	ld	b,20
.WaitForKeyrelease\@
	call	GetKeys
	jr	nz,.WaitForKeyrelease\@
	dec	b
	jr	nz,.WaitForKeyrelease\@
	pop	bc
	ENDM

	ENDC