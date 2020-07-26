; Hello Random
; use gbrandom.asm to generate random numbers
; give one example of generating a random seed
; John Harrison

; v1.0  2008-Mar-30
; v1.1  2008-Apr-20
;        - added call to MakeRandomSeedByTimingKeypress

INCLUDE "gbhw.inc" ; standard hardware definitions from devrs.com
INCLUDE "ibmpc1.inc" ; ASCII character set from devrs.com
INCLUDE "standard-defs.inc" ; standard defs

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
	LOAD_ROM_HEADER	HelloRandom

INCLUDE "memory.asm"
INCLUDE "gbrandom.asm"
INCLUDE "print-number.asm"
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
	di
init:
	ld	a, %11100100 		; Window palette colors, from darkest to lightest
	ld	[rBGP], a		; set background and window pallette

	ld	a,0			; SET SCREEN TO TO UPPER RIGHT HAND CORNER
	ld	[rSCX], a
	ld	[rSCY], a		
	call	StopLCD			; YOU CAN NOT LOAD $8000 WITH LCD ON
	ld	hl, TileData
	ld	de, _VRAM		; $8000
	ld	bc, 8*256 		; the ASCII character set: 256 characters, each with 8 bytes of display data
	call	mem_CopyMono	; load tile data

	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ8|LCDCF_OBJOFF |LCDCF_WIN9C00 |LCDCF_WINOFF ; see gbspec.txt lines 1525-1565 and gbhw.inc lines 70-86 
	ld	[rLCDC], a	
	call	ClearScreen

; ****************************************************************************************
; Main loop
; Demo the random number generator
; ****************************************************************************************
MainLoop:	
	ld	hl,Title
	ld	de, _SCRN0
	ld	bc, TitleEnd-Title
	call	mem_CopyVRAM

	call	MakeRandomSeedByTimingKeypress

; print the seed
	call	ClearScreen
	ld	hl,HL_TEXT
	ld	de,_SCRN0	
	ld	bc,HL_TEXTEnd-HL_TEXT
	call	mem_CopyVRAM

	ld	a,[RandSeed]
	ld	e,a
	ld	a,[RandSeed+1]
	ld	d,a
	ld	hl,_SCRN0+12
	call	PrintWord

	ld	hl,RAND_TEXT
	ld	de,_SCRN0+(SCRN_VY_B*2)
	ld	bc,RAND_TEXTEnd-RAND_TEXT
	call	mem_CopyVRAM

	ld	bc,14		; # of rows of random #s to generate
.printNumberLoop
	call	Rand16		; put rand # in hl
	push	hl
	pop	de		; de contains random #
	ld	hl,_SCRN0+(SCRN_VY_B*3)
	push	bc
	REPT	5	;2^5=32
	sla	c
	rl	b
	ENDR
	add	hl,bc
	pop	bc
	call	PrintWord

	REPT	3
	INC_HL
	push	hl
	call	Rand16
	push	hl
	pop	de
	pop	hl
	call	PrintWord
	ENDR

	dec	bc
	ld	a,c
	cp	0
	jr	nz,.printNumberLoop
	WaitForKeyrelease
	WaitForKeypress
	jp	MainLoop

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
;		 00000000011111111112222222222333
;		 12345678901234567890123456789012
	DB	"Random Numbers                  "	; 1
	DB	"Example Code:                   "	; 2
        DB      "                                "	; 3
        DB      "To generate random              "	; 4
	DB	"numbers, you need               "	; 5
	DB	"a random number seed.           "	; 6
        DB      "                                "	; 7
        DB      "This program                    "	; 8
        DB      "generates one by                "	; 9
        DB      "running a fast                  "	; 10
        DB      "counter between two             "	; 11
	DB	"keypresses.                     "	; 12
        DB      "                                "	; 13
	DB	"Imagine this might              "	; 14
	DB	"be your title screen.           "	; 15
        DB      "                                "	; 16
	DB	"Press any key                   "	; 17
	DB	"to continue...                  "	; 18
TitleEnd:

;		 00000000011111111112222222222333
;		 12345678901234567890123456789012
HL_TEXT:	DB	"Random seed:"
HL_TEXTEnd:

RAND_TEXT:	DB	"Pseudo-random #s:"
RAND_TEXTEnd:

ClearScreen:
	ld	a, 32		; ASCII FOR BLANK SPACE
	ld	hl, _SCRN0
	ld	bc, SCRN_VX_B * SCRN_VY_B
	call	mem_SetVRAM
	ret

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
