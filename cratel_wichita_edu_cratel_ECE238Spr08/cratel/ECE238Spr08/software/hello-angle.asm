; Hello Angle
; have a sprite move at a constant speed and different angles
; John Harrison

; v1.0: 2008-Mar-31

; besides demonstrating use of the sine function
; this code shows a good main loop
; using the timer interrupt to set its speed

INCLUDE "gbhw.inc" ; standard hardware definitions from devrs.com
INCLUDE "ibmpc1.inc" ; ASCII character set from devrs.com
INCLUDE "standard-defs.inc" ; specific defs
INCLUDE "sprite.inc"
; create variables
	LoByteVar	IFlags  ; flag var for interrupts
	SpriteAttr	Sprite0 ; this is a structure of 4 variables. See hello-sprite.inc

	
; IRQs
SECTION	"Vblank",HOME[$0040]
	jp	DMACODELOC ; *hs* update sprites every time the Vblank interrupt is called (~60Hz)
SECTION	"LCDC",HOME[$0048]
	reti
SECTION	"Timer_Overflow",HOME[$0050]
	jp	TimerInterrupt		; flag the timer interrupt
SECTION	"Serial",HOME[$0058]
	reti
SECTION	"p1thru4",HOME[$0060]
	reti

;***************************************************************************
;*
;* TIMER DEFINITIONS
;*
;***************************************************************************

TimerHertz	EQU	TACF_START|TACF_4KHZ
TimerClockDiv	EQU	256-30				; divide clock by 30


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
	LOAD_ROM_HEADER	HelloAngle

INCLUDE "memory.asm"
INCLUDE "keypad.asm"

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

; NEXT FOUR LINES FOR SETTING UP SPRITES *hs*
	call	initdma			; move routine to HRAM
	ld	a, IEF_VBLANK|IEF_TIMER
	ld	[rIE],a			; ENABLE VBLANK AND TIMER INTERRUPT
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

	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ8|LCDCF_OBJON ; *hs* see gbspec.txt lines 1525-1565 and gbhw.inc lines 70-86
	ld	[rLCDC], a	
	ld	a, 32		; ASCII FOR BLANK SPACE
	ld	hl, _SCRN0
	ld	bc, SCRN_VX_B * SCRN_VY_B
	call	mem_SetVRAM

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
	ld	a,TimerClockDiv		; load number of counts of timer
	ld	[rTMA],a		
	ld	a,TimerHertz		; load timer speed
	ld	[rTAC],a
	ld	a,IEF_TIMER
	ld	[IFlags],a
	pop	af			; restore a. Everything has been preserved.
	reti

; *hs* START
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
