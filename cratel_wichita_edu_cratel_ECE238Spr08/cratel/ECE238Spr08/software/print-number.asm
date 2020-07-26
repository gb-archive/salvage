;* print-number.asm
;* code for printing numbers from byte and word values
;
;* by John Harrison (so far)

;* 2008-Mar-30 --- V1.0
;  2008-Apr-08 --- V1.1 added RET to PrintNibble...how did it ever work?

        IF      !DEF(PRINT_NUMBER_ASM)
PRINT_NUMBER_ASM  SET  1

; *********************************************************************
; PrintWord
; prints word to screen
; input: de = word to print
;        hl = video memory location to print word
; *********************************************************************
PrintWord:
	push	af
	ld	a,d
	call	PrintByte
	ld	a,e
	call	PrintByte
	pop	af
	ret

; *********************************************************************
; PrintByte
; print value of byte to the screen
; input: a = byte to print
;        hl = video memory location to print nibble
; output:hl points to next location in memory     
; *********************************************************************
PrintByte:
	push	af
	swap	a	; do high nibble first
	call	PrintNibble
	pop	af
	push	af
	call	PrintNibble
	pop	af
	ret

; *********************************************************************
; PrintNibble
; print value of nibble to the screen
; input: a bits 0-3 = nibble to print
;        hl = video memory location to print nibble     
; output:hl points to next location in memory
; *********************************************************************

PrintNibble:
	push	af
	push	bc
	and	$f	; apply a bitmask to a so that the new upper
			; nibble is all zeros.
	add	$30	; add the ascii value of '0' to A so it will
			; print the correct
			; ascii character to the screen
	cp	$3a
	jr	c,.PNinner1
	add	7
.PNinner1:
	ld	bc,1	; load the correct 16 bit register pair
			; with the number of times
   			; we want the upper nibble to be printed
	call	mem_SetVRAM
	pop	bc
	pop	af
	ret

	ENDC