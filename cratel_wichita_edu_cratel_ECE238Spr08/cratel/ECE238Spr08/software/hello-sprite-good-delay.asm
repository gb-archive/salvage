; Hello Sprite Good Delay 1.0
; February 19, 2007
; John Harrison

; using sprites WITH interrupts.
; look at this sprite example third. Before looking at this one, look at:
; 1. hello-sprite-no-interrupts.asm
; 2. hello-sprite.asm (uses interrupts)

; An extension of Hello World, based mostly from GALP

INCLUDE "gbhw.inc" ; standard hardware definitions from devrs.com
INCLUDE "ibmpc1.inc" ; ASCII character set from devrs.com
INCLUDE "hello-sprite-good-delay.inc" ; specific defs

; create variables
;	LoByteVar	xpos	; these are examples of how variables
;	LoByteVar	spry	; can be created. They are based on a
;	LoByteVar	sprx	; macro in hello-sprite.inc
	LoByteVar	TimerEvent ; flag that a timer interrupt has occurred.
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
	ROM_HEADER	ROM_NOMBC, ROM_SIZE_32KBYTE, RAM_SIZE_0KBYTE
INCLUDE "memory.asm"
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
	ld	sp, $ffff		; set the stack pointer to highest mem location + 1

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
; ****************************************************************************************
; Main code:
; Print a character string in the middle of the screen
; ****************************************************************************************
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
MainLoop:
	call	TimedDelay
	call	GetKeys
	push	af
	and	PADF_RIGHT
	call	nz,right
	pop	af
	push	af
	and	PADF_LEFT
	call	nz,left
	pop	af
	push	af
	and	PADF_UP
	call	nz,up
	pop	af
	push	af
	and	PADF_DOWN
	call	nz,down
	pop	af
	push	af
	and	PADF_START
	call	nz,Yflip
	pop	af
	jr	MainLoop

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
	DB	"Move the sprite by              "
        DB      "using the arrow keys            "
        DB      "one at a time, or               "
        DB      "in combination.                 "
        DB      "                                "
        DB      "Flip the sprite                 "
        DB      "vertically with the             "
        DB      "'start' key.                    "
TitleEnd:

right:
	GetSpriteXAddr	Sprite0
	cp		SCRN_X-8	; already on RHS of screen?
	ret		z
	inc		a
	PutSpriteXAddr	Sprite0,a
	ret
left:	GetSpriteXAddr	Sprite0
	cp		0		; already on LHS of screen?
	ret		z
	dec		a
	PutSpriteXAddr	Sprite0,a
	ret	
up:	GetSpriteYAddr	Sprite0
	cp		0		; already at top of screen?
	ret		z
	dec		a
	PutSpriteYAddr	Sprite0,a
	ret
down:	GetSpriteYAddr	Sprite0
	cp		SCRN_Y-8	; already at bottom of screen?
	ret		z
	inc		a
	PutSpriteYAddr	Sprite0,a
	ret
Yflip:	ld	a,[Sprite0Flags]
	xor	OAMF_YFLIP		; toggle flip of sprite vertically
	ld	[Sprite0Flags],a
	ret

; simpleDelay is no longer used in this code. We can't predict
; how long simpleDelay will delay since we do not know how fast the CPU
; is. GameBoy CPU speeds vary between models: 
simpleDelay:
	dec	bc
	ld	a,b
	or	c
	jr	nz, simpleDelay
	ret

TimedDelay:
	ld	a,TimerOFlowReset	; reset timer interrupt flag
	ld	[TimerEvent],a
	ld	a,TimerClockDiv		; load number of counts of timer
	ld	[rTMA],a		
	ld	a,TimerHertz		; load timer speed
	ld	[rTAC],a
.wait:	halt				; wait for an interrupt
	nop				; always follow HALT with NOP (gbspec.txt lines 514-578)
	ld	a,[TimerEvent]		; load timer flag
	cp	TimerOFlowSet		; was the interrupt caused by the timer?
	jr	nz,.wait		; nope. Keep waiting

; TimerInterrupt is the routine called when a timer interrupt occurs.
TimerInterrupt:	
	push	af			; save a
	ld	a,TimerOFlowSet		; load value representing that an interrupt occured.
	ld	[TimerEvent],a		; save value in a variable
	pop	af			; restore a. Everything has been preserved.
	reti

; GetKeys: adapted from APOCNOW.ASM and gbspec.txt
GetKeys:                 ;gets keypress
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
 	ret				; do we need to reset joypad? (gbspec line 1082)


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
