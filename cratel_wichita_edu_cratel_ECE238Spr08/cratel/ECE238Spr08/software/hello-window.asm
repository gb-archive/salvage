; Hello Window
; modification of Hello Angle to show the function of the GB window
; John Harrison

; v1.1 - 2008-Apr-2: tweaked CALL_IF_NTH_TIME macro

; besides demonstrating use of the sine function
; this code shows a good main loop
; using the timer interrupt to set its speed

INCLUDE "gbhw.inc" ; standard hardware definitions from devrs.com
INCLUDE "ibmpc1.inc" ; ASCII character set from devrs.com
INCLUDE "standard-defs.inc" ; specific defs
INCLUDE "sprite.inc"

;***************************************************************************
;*
;* TIMER DEFINITIONS
;*
;***************************************************************************

TimerHertz	EQU	TACF_START|TACF_4KHZ

CALL_IF_NTH_TIME:	MACRO
			push	af
			push	bc
			ld	a,[\1]
			ld	b,a
			and	256-\2
			cp	b
			call	z,\3
			pop	bc
			pop	af
			ENDM

; create variables	
	LoByteVar	IFlags  ; flag var for interrupts
	LoByteVar	LoopCount		; count iterations through MainLoop
	LoByteVar	TimerClockDiv		; change speed of loop at run time
	LoByteVar	WindowLowScanLine	; turn off window at this line -- internal
	LoByteVar	WindowNewLowScanLine	; turn off window at this line -- your code uses this one
	LoByteVar	WindowHighScanLine	; turn on window at this line
	LoByteVar	DirectionFlag		; are we opening or closing the window?
	SpriteAttr	Sprite0 ; this is a structure of 4 variables. See hello-sprite.inc
	
CLOSE	EQU	1
OPEN	EQU	0

; IRQs
SECTION	"Vblank",HOME[$0040]
	jp	DMACODELOC ; *hs* update sprites every time the Vblank interrupt is called (~60Hz)
SECTION	"LCDC",HOME[$0048]
	jp	LCDCInterrupt
SECTION	"Timer_Overflow",HOME[$0050]
	jp	TimerInterrupt		; flag the timer interrupt
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
	LOAD_ROM_HEADER	Hello Window

INCLUDE "memory.asm"

; --
; -- Generate a 256 byte sine table with values between -64 and 64
; --
SINE_TABLE:
ANGLE   SET     0.0
        REPT    256+64
        DB      (MUL(64.0,SIN(ANGLE)))>>16
ANGLE   SET     ANGLE+256.0
        ENDR

GET_SINE:	MACRO
	push	hl
	push	bc
	ld	hl,SINE_TABLE
	ld	b,0
	ld	c,\1
	add	hl,bc
	ld	a,[hl]
	pop	bc
	pop	hl
	ENDM

GET_COSINE:	MACRO
	push	hl
	push	bc
	ld	hl,SINE_TABLE+64
	ld	b,0
	ld	c,\1
	xor	a
	sub	c
	ld	c,a
	add	hl,bc
	ld	a,[hl]
	pop	bc
	pop	hl
	ENDM


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

	ld	a,256-30
	ld	[TimerClockDiv],a	; divide clock by 30. You can change this at any time
					; for faster or slower game play.

; Initialize the window.
	ld	a,CLOSE			; start by closing the window
	ld	[DirectionFlag],a
	ld	a,7			; have window stop at line 7
	ld	[WindowLowScanLine],a
	ld	[WindowNewLowScanLine],a
	ld	a,SCRN_Y-8		; and start again at 8 before the end of the screen
	ld	[WindowHighScanLine],a

	call	initdma			; move sprite routine to HRAM
	ld	a, IEF_VBLANK|IEF_TIMER|IEF_LCDC ; comment out IEF_LCDC if you don't want the window to
					; ever be turned off
	ld	[rIE],a			; ENABLE VBLANK, TIMER (AND MAYBE IEF_LCDC) INTERRUPT
	ei				; LET THE INTS FLY

init:
	ld	a, %11100100 		; Window palette colors, from darkest to lightest
	ld	[rBGP], a		; set background and window pallette
	ldh	[rOBP0],a		; set sprite pallette 0 (choose palette 0 or 1 when describing the sprite)
	ldh	[rOBP1],a		; set sprite pallette 1

	ld	a,0			; SET SCREEN TO TO UPPER RIGHT HAND CORNER
	ld	[rSCX], a
	ld	[rSCY], a		
	call	StopLCD			; YOU CAN NOT LOAD $8000 WITH LCD ON
	ld	hl, TileData
	ld	de, _VRAM		; $8000
	ld	bc, 8*256 		; the ASCII character set: 256 characters, each with 8 bytes of display data
	call	mem_CopyMono	; load tile data

; *hs* erase sprite table
	ld	a,0
	ld	hl,OAMDATALOC
	ld	bc,OAMDATALENGTH
	call	mem_Set

	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ8|LCDCF_OBJON |LCDCF_WIN9C00 |LCDCF_WINON ; see gbspec.txt lines 1525-1565 and gbhw.inc lines 70-86 
	ld	[rLCDC], a	
	ld	a, 32		; ASCII FOR BLANK SPACE
	ld	hl, _SCRN0
	ld	bc, SCRN_VX_B * SCRN_VY_B
	call	mem_SetVRAM

	ld	hl,WindTitle
	ld	de, _SCRN1
	ld	bc, WindTitleEnd-WindTitle
	call	mem_CopyVRAM

; set the window to display at (0,0) on the screen

	ld	a,7		; X coordinate displaced by 7
	ld	[rWX],a
	xor	a
	ld	[rWY],a

	ld	a,[WindowLowScanLine]	; set LCDC interrupt to happen at low scan line
	ld	[rLYC],a

	ld	a,STATF_LYCF|STATF_LYC	; turn the LCDC interrupt on for when LCDC LY = LYC
	ld	[rSTAT],a

	ld	hl,Title
	ld	de, _SCRN0+(SCRN_VY_B*5) ; 
	ld	bc, TitleEnd-Title
	call	mem_CopyVRAM

sprsetup:
	PutSpriteYAddr	Sprite0,0	; set Sprite0 location to 0,0
	PutSpriteXAddr	Sprite0,0
 	ld	a,1		;	; ibmpc1.inc ASCII character 1 is happy face :-)
 	ld 	[Sprite0TileNum],a      ;sprite 1's tile address
 	ld	a,%00000000         	;set flags (see gbhw.inc lines 33-42)
 	ld	[Sprite0Flags],a        ;save flags

	call	TimerInterrupt		; turn on the timer

	ld	hl,0			; current location X * 64
	ld	de,0			; current location Y * 64
	ld	bc,0			; current angle (0-255)

; ****************************************************************************************
; Main loop
; Move a sprite around the screen
; ****************************************************************************************

MainLoop:
	halt
	nop
	ld	a,[IFlags]		; sit and wait
	cp	IEF_TIMER		; unless the timer caused the interrupt
	jr	nz,MainLoop
	xor	a			; reset the timer flag to catch the next
	ld	[IFlags],a		; timer interrupt

	ld	a,[LoopCount]
	inc	a
	ld	[LoopCount],a		; inc LoopCount timer
	CALL_IF_NTH_TIME	LoopCount,16,ChangeWindow	; var to check, iteration divisor, routine
								; iteration div must be multiple of 2

	push	bc			; save angle
	GET_COSINE	c		; returns a=cos(c). Range: -64 to 64
	ld	b,0
	ld	c,a
	cp	80
	jr	c,.bokx			; if c < 0
	dec	b			; then bc needs to be < 0 for add
.bokx:	add	hl,bc
	push	hl			; save new position
	REPT	6			; divide hl by 64 to get screen coords
	srl	h
	rr	l
	ENDR
	ld	a,l
	cp	SCRN_X+1
	jr	nz,.okx1
	pop	hl
	ld	hl,0
	push	hl
.okx1:	cp	-9
	jr	nz,.okx2
	pop	hl
	ld	hl,SCRN_X*64
	push	hl
	ld	l,SCRN_X
.okx2:	PutSpriteXAddr	Sprite0,l
	pop	hl
	pop	bc

	push	hl			; save hl
	push	de
	pop	hl			; ld hl,de
	push	bc			; save bc
	GET_SINE	c
	ld	b,0
	ld	c,a
	cp	80
	jr	c,.boky1
	dec	b
.boky1:	dec	bc
	ld	a,b
	cpl
	ld	b,a
	ld	a,c
	cpl
	ld	c,a
	add	hl,bc
	push	hl
	REPT	6
	srl	h
	rr	l
	ENDR
	ld	a,l
	cp	SCRN_Y+1
	jr	nz,.oky1
	pop	hl
	ld	hl,0
	push	hl
.oky1:	cp	-9
	jr	nz,.oky2
	pop	hl
	ld	hl,SCRN_Y*64
	push	hl
	ld	l,SCRN_Y
.oky2:	PutSpriteYAddr	Sprite0,l
	pop	de
	pop	bc
	pop	hl
	
	call	GetKeys
	push	af
	and	PADF_RIGHT
	call	nz,right
	pop	af
	and	PADF_LEFT
	call	nz,left
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
	DB	"Use the left and                "
	DB	"right arrow keys                "
        DB      "to change the angle             "
        DB      "of the sprite.                  "
        DB      "                                "
        DB      "The velocity                    "
        DB      "remains constant                "
TitleEnd:

WindTitle:
;		 01234567890123456789
;		 01234567890123456789012345678901"
	DB	"Window Line 0-456789012345678901"	;0
	DB	"Window Line 1-456789012345678901"	;1
	DB	"                                "	;2
	DB	"This is text from               "	;3
	DB	"the window and NOT              "	;4
	DB	"from the background.            "	;5
	DB	"                                "	;6
	DB	"Window tiles are                "	;7
	DB	"always on top of                "	;8
	DB	"background tiles                "	;9
	DB	"and are never                   "	;10
	DB	"transparent.                    "	;11
	DB	"                                "	;12
	DB	"Sprites can still               "	;13
	DB	"appear on top of                "	;14
	DB	"a window.                       "	;15
	DB      "                                "	;16
	DB	"Window Line 17-56789012345678901"	;17
	DB	"Window Line 18-56789012345678901"	;18
	DB	"Window Line 19-56789012345678901"	;19
WindTitleEnd:

right:
	ld	a,c
	dec	bc
	ret
left:
	ld	a,c
	inc	bc
	ret

; TimerInterrupt is the routine called when a timer interrupt occurs.
TimerInterrupt:	
	push	af			; save a
	ld	a,[TimerClockDiv]	; load number of counts of timer
	ld	[rTMA],a		
	ld	a,TimerHertz		; load timer speed
	ld	[rTAC],a
	ld	a,IEF_TIMER
	ld	[IFlags],a
	pop	af			; restore a. Everything has been preserved.
	reti

; this interrupt routine is called with LYC = LCDC LY
LCDCInterrupt:
	push	af
	push	bc
	ld	a,[WindowLowScanLine]
	ld	b,a
	ld	a,[rLY]			; are we on the low scan line?
	cp	b
	jr	z,.setHighScanLine	; yes. Set the interrupt for the high scan line, then.
	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ8|LCDCF_OBJON |LCDCF_WIN9C00 |LCDCF_WINON ; see gbspec.txt lines 1525-1565 and gbhw.inc lines 70-86 	
	ld	[rLCDC],a
	ld	a,[WindowNewLowScanLine]
	ld	[WindowLowScanLine],a	; change the low scan line, if WindowNewLowScanLine has changed
	jr	.seta
.setHighScanLine:
	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ8|LCDCF_OBJON |LCDCF_WIN9C00 |LCDCF_WINOFF ; see gbspec.txt lines 1525-1565 and gbhw.inc lines 70-86 	
	ld	[rLCDC],a
	ld	a,[WindowHighScanLine]
.seta:	ld	[rLYC],a
	pop	bc
	pop	af
	reti

; GetKeys: adapted from APOCNOW.ASM and gbspec.txt
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

ChangeWindow:
	push	af
	ld	a, [DirectionFlag]
	cp	CLOSE
	ld	a,[WindowNewLowScanLine]
	jr	nz,.go_open
	cp	(SCRN_Y/2)-1
	jr	z,.change2open
	inc	a
	ld	[WindowNewLowScanLine],a
	ld	a,[WindowHighScanLine]
	dec	a
	ld	[WindowHighScanLine],a
.done	pop	af
	ret
.change2open
	ld	a,OPEN
.finishChange
	ld	[DirectionFlag],a
	jr	.done
.go_open	cp	0
	jr	z,.change2close
	dec	a
	ld	[WindowNewLowScanLine],a
	ld	a,[WindowHighScanLine]
	inc	a
	ld	[WindowHighScanLine],a
	jr	.done
.change2close
	ld	a,CLOSE
	jr	.finishChange

initdma:
	ld	de, DMACODELOC
	ld	hl, dmacode
	ld	bc, dmaend-dmacode
	call	mem_CopyVRAM			; copy when VRAM is available
	ret
dmacode:
	push	af
	ld	a, OAMDATALOCBANK		; bank where OAM DATA is stored
	ldh	[rDMA], a			; Start DMA
	ld	a, $28				; 160ns
dma_wait:
	dec	a
	jr	nz, dma_wait
	pop	af
	reti
dmaend:
; *hs* END

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
