; easyScore.asm
; a generic score keeper.
; by John Harrison

; v1.0  2008-Apr-28
; v1.0a 2008-Apr-30 added easyScoreReset
; v1.0b 2008-May-01 added easyScoreSet

; shows a 4-digit score at the beginning of  the last line of the
; Game Boy screen.
; assumes you are doing nothing too out-of-the-ordinary:
;  - you aren't using the window for anything
;  - your background tile map is at $9800
;  - you have the standard ASCII tile map loaded

        IF      !DEF(EASY_SCORE_ASM)	; we have already defined?
EASY_SCORE_ASM  SET  1

;***************************************************************************
; Settings:
; the default settings below work for most cases
; and won't need to be changed
;***************************************************************************

ES_TEXT:
	DB	"Score: "	; CHANGE TO SAY SOMETHING ELSE IF YOU WANT
ES_TEXT_END:
ES_PRINT_LOC		EQU	0	; NUMBER OF COLUMNS WHICH
					; SCORE PRINTOUT WILL BE
					; INDENTED
ES_WINDOW_SCREEN	EQU	_SCRN1

;***************************************************************************
; End of Settings area
; Main code routines are below:
;***************************************************************************

include "standard-defs.inc"
include "print-number.asm"

	LoWordVar	easyScoreVar

;***************************************************************************
;* easyScoreInit:
;* sets up scoring mechanism
;* initializes score to 0
;* shows score readout at bottom of GB screen
;* input: none
;* output: none
;***************************************************************************

easyScoreInit:
; SAVE EVERYTHING
	push	hl
	push	de
	push	bc
	push	af

; CLEAR THE WINDOW SCREEN:
	ld	a, 32		; ASCII FOR BLANK SPACE
	ld	hl, _SCRN1
	ld	bc, SCRN_VX_B * SCRN_VY_B
	call	mem_SetVRAM

; SET THE WINDOW COORDINATES RELATIVE TO THE SCREEN
	ld	a,7
	ld	[rWX],a
	ld	a,144-8
	ld	[rWY],a

; INITIALIZE THE SCORE
	xor	a
	ld	[easyScoreVar],a
	ld	[easyScoreVar+1],a

; PRINT WHATEVER WE CALL THE SCORE
	ld	hl,ES_TEXT
	ld	de,ES_WINDOW_SCREEN+ES_PRINT_LOC
	ld	bc,ES_TEXT_END-ES_TEXT
	call	mem_CopyVRAM

; PRINT THE SCORE
	call	easyScorePrint

; RESTORE EVERYTHING AND BE DONE
	pop	af
	pop	bc
	pop	de
	pop	hl
	ret

;***************************************************************************
;* easyScorePrint:
;* prints the score
;* input: none
;* output: none
;***************************************************************************

easyScorePrint:
	push	af
	push	de
	push	hl
	ld	a,[easyScoreVar]
	ld	e,a
	ld	a,[easyScoreVar+1]
	ld	d,a
	ld	hl,ES_WINDOW_SCREEN+ES_PRINT_LOC+ES_TEXT_END-ES_TEXT
	call	PrintWord
	pop	hl
	pop	de
	pop	af
	ret

;***************************************************************************
;* easyScoreDrop:
;* lowers the score and prints the new score value
;* input: a = amount to drop score by
;* output: none
;***************************************************************************

easyScoreDrop:
	push	hl
	push	de
	ld	e,a
	push	af
	ld	a,[easyScoreVar+1]
	ld	h,a
	ld	a,[easyScoreVar]
	sub	e
	daa
	ld	l,a
	jr	nc,_easyScoreNoCarry
	ld	a,h
	sub	1
	daa
	ld	h,a
	jr	_easyScoreNoCarry

;***************************************************************************
;* easyScoreRaise:
;* raises the score and prints the new score value
;* input: a = amount to raise score by
;* output: none
;***************************************************************************

easyScoreRaise:
	push	hl
	push	de
	ld	e,a
	push	af
	ld	a,[easyScoreVar+1]
	ld	h,a
	ld	a,[easyScoreVar]
	add	a,e
	daa
	ld	l,a
	jr	nc,_easyScoreNoCarry
	ld	a,h
	add	a,1
	daa
	ld	h,a
_easyScoreNoCarry:
	ld	a,h
	ld	[easyScoreVar+1],a
	ld	a,l
	ld	[easyScoreVar],a
	call	easyScorePrint
	pop	af
	pop	de
	pop	hl
	ret

	ENDC

;***************************************************************************
;* easyScoreReset:
;* sets the score back to 0
;* input: none
;* output: none
;***************************************************************************
easyScoreReset:
	push	af
	xor	a
	ld	[easyScoreVar+1],a
	ld	[easyScoreVar],a
	call	easyScorePrint
	pop	af
	ret

;***************************************************************************
;* easyScoreSet:
;* sets the score back to BCD value stored in HL
;* input: HL = value to set score to in BCD
;* output: none
;***************************************************************************
easyScoreSet:
	push	af
	ld	a,h
	ld	[easyScoreVar+1],a
	ld	a,l
	ld	[easyScoreVar],a
	call	easyScorePrint
	pop	af
	ret