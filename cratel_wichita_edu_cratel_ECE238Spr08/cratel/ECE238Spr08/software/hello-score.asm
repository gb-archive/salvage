; hello-score.asm
; demo for the easyScore generic score keeper.
; by John Harrison

; v1.0   2008-Apr-28 - original release
; v1.0a  2008-Apr-30 - added easyScoreReset demo
; v1.0b  2008-May-01 - added easyScoreSet demo

INCLUDE "gbhw.inc" ; standard hardware definitions from devrs.com
INCLUDE "ibmpc1.inc" ; ASCII character set from devrs.com
INCLUDE "standard-defs.inc"

; IRQs
SECTION	"Vblank",HOME[$0040]
	reti
SECTION	"LCDC",HOME[$0048]
	reti
SECTION	"Timer_Overflow",HOME[$0050]
	reti
SECTION	"Serial",HOME[$0058]
	reti
SECTION	"p1thru4",HOME[$0060]
	reti

; ****************************************************************************************
; boot loader jumps to here.
; ****************************************************************************************
SECTION	"start",HOME[$0100]
nop
jp	begin

; ****************************************************************************************
; ROM HEADER and ASCII character set
; ****************************************************************************************
; ROM header
	LOAD_ROM_HEADER	"hello-score"

INCLUDE "memory.asm"
INCLUDE "easyScore.asm"
INCLUDE "keypad.asm"

TileData:
	chr_IBMPC1	1,8 ; LOAD ENTIRE CHARACTER SET

; ****************************************************************************************
; Main code Initialization:
; set the stack pointer, enable interrupts, set the palette, set the screen relative to the window
; copy the ASCII character table, clear the screen
; ****************************************************************************************
begin:
	nop
	ld	a,IEF_VBLANK
	ld	[rIE],a		; turn on VBLANK interrupt as easy hack
				; for speed of MainLoop

	ld	a, %11100100 	; Window palette colors, from darkest to lightest
	ld	[rBGP], a		; CLEAR THE SCREEN

	ld	a,0			; SET SCREEN TO TO UPPER RIGHT HAND CORNER
	ld	[rSCX], a
	ld	[rSCY], a		
	call	StopLCD		; YOU CAN NOT LOAD $8000 WITH LCD ON
	ld	hl, TileData
	ld	de, _VRAM		; $8000
	ld	bc, 8*256 		; the ASCII character set: 256 characters, each with 8 bytes of display data
	call	mem_CopyMono	; load tile data
	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ16|LCDCF_OBJOFF|LCDCF_WIN9C00|LCDCF_WINON 
	ld	[rLCDC], a	
	ld	a, 32		; ASCII FOR BLANK SPACE
	ld	hl, _SCRN0
	ld	bc, SCRN_VX_B * SCRN_VY_B
	call	mem_SetVRAM
	call	easyScoreInit
	ld	hl,Title
	ld	de, _SCRN0+(SCRN_VY_B*1) ; 
	ld	bc, TitleEnd-Title
	call	mem_CopyVRAM

; ****************************************************************************************
; Main code:
; Print a character string in the middle of the screen
; ****************************************************************************************
MainLoop:
	halt
	call	GetKeys
	cp	PADF_DOWN
	push	af
	call	z,lowerScore
	pop	af
	push	af
	cp	PADF_UP
	call	z,raiseScore
	pop	af
	push	af
	cp	PADF_A
	call	z,easyScoreReset
	pop	af
	cp	PADF_B
	call	z,setScore
	jr	MainLoop

lowerScore:
	ld	a,1	; lower score by this amount
	call	easyScoreDrop
	ret

raiseScore:
	ld	a,1	; raise score by this amount
	call	easyScoreRaise
	ret
setScore:
	ld	hl,$4000
	call	easyScoreSet
	ret
		
; ****************************************************************************************
; Prologue
; Wait patiently 'til somebody kills you
; ****************************************************************************************
wait:
	halt
	nop
	jr	wait
	
; ****************************************************************************************
; hard-coded data
; ****************************************************************************************
Title:
;                00000000011111111112222222222333
;                12345678901234567890123456789012
	DB	"Generic Score Demo              "
	DB	"                                "
	DB	"Use the UP and DOWN             "
	DB	"Keys to raise and               "
	DB	"Lower the score.                "
	DB	"                                "
	DB	"Use the A key to                "
	DB	"reset the score.                "
	DB	"                                "
	DB	"Use the B key to set            "
	DB      "score to hard-coded             "
	DB	"value of 4000 BCD               "            
TitleEnd:

; ****************************************************************************************
; StopLCD:
; turn off LCD if it is on
; and wait until the LCD is off
; ****************************************************************************************
StopLCD:
        ld      a,[rLCDC]
        rlca                    ; Put the high bit of LCDC into the Carry flag
        ret     nc              ; Screen is off already. Exit.

; Loop until we are in VBlank

.wait:
        ld      a,[rLY]
        cp      145             ; Is display on scan line 145 yet?
        jr      nz,.wait        ; no, keep waiting

; Turn off the LCD

        ld      a,[rLCDC]
        res     7,a             ; Reset bit 7 of LCDC
        ld      [rLCDC],a

        ret
