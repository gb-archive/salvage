; Hello BigSprite John Harrison
; modify Justin Bollig's hello-sprite code
; now uses BigSprites and demonstrates collision detection
; the purpose of this code is to demonstrate the BigSprite library

; v1.0 2008-Mar-22
; v1.1 2008-Apr-05: fixed to operate correctly on real GameBoy:
;                   tiles were loading when screen was on: tiles were trashed
;                   collision/no-collision text not updating on VBLANK: trashed
; v1.2 2008-Apr-11: updated to no longer use GET_BS_LOC, GET_BB_UPR_L
;                   GET_BB_LWR_R macros (deprecated)
;                   shows background tiles touching corners of bounding boxes
;                   fixed small bug allowing sprite to leave top of screen
;                   updated to use standard includes
;                   ROM title is correct and no longer EXAMPLE
; v1.3 2008-Apr-15: added SetSpriteTileNum macro demo

INCLUDE "gbhw.inc" ; standard hardware definitions from devrs.com
INCLUDE "ibmpc1.inc" ; ASCII character set from devrs.com
INCLUDE "standard-defs.inc" ; LoRamVar and other friends
INCLUDE "sprite.inc" ; sprite defs
INCLUDE "powercat.z80"		; Justin's set of 8 tiles
				; describing the powercat logo

	REV_CHECK_SPRITE_INC	1.1 ; check for correct version of sprite.inc

; IRQs
SECTION	"Vblank",HOME[$0040]
	jp	DMACODELOC ; update sprites every time the Vblank interrupt is called (~60Hz)
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
;ROM header
	LOAD_ROM_HEADER	HelloBS

INCLUDE "memory.asm"
INCLUDE "bigsprite.asm"
INCLUDE "keypad.asm"
INCLUDE "print-number.asm"

TileData:
	chr_IBMPC1	1,8 ; LOAD ENTIRE CHARACTER SET

; ****************************************************************************************
; Initialization:
; set the stack pointer, enable interrupts, set the palette, set the screen relative to the window
; copy the ASCII character table, clear the screen
; AND MORE...
; ****************************************************************************************
begin:
	nop
	di
	ld	sp, $ffff		; set the stack pointer to highest mem location + 1

; NEXT FOUR LINES FOR SETTING UP SPRITES *hs*
	call	initdma			; move routine to HRAM
	ld	a, IEF_VBLANK
	ld	[rIE],a			; ENABLE ONLY VBLANK INTERRUPT
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

	ld      hl, powercat      ; You chose this name in the export dialog box of GBTD
        ld      de, _VRAM        ; _VRAM=$8000=tile pattern table 0. If you wish to load into tile pattern table 1, choose $8800
        ld      bc, 8*16		;16 tiles to load into video memory
        call    mem_Copy      ; load tile data 

	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ8|LCDCF_OBJON ; *hs* see gbspec.txt lines 1525-1565 and gbhw.inc lines 70-86
	ld	[rLCDC], a	

	INIT_BS			; set up BigSprites
				; you must call this after loading tiles
				; and before your main loop 

; load correct tiles for sprites.
; these tiles must be set AFTER a call to INIT_BS
; you can change tile numebers on sprites at any time, regardless
; of whether or not the screen is on. This is not true with the
; background or the window

; INIT_BS loads sprite 0, with tile 0, sprite 1 with tile 1, etc.
; In this example, this is what we want for sprites 0-7, but not
; for sprites 8-15, so we redefine the tiles loaded in sprites 8-15

	SetSpriteTileNum	8,0	; sprite #, tile #
	SetSpriteTileNum	9,1
	SetSpriteTileNum	10,2
	SetSpriteTileNum	11,3
	SetSpriteTileNum	12,4
	SetSpriteTileNum	13,5
	SetSpriteTileNum	14,6
	SetSpriteTileNum	15,7

; clear the screen

	ld	a, 32		; ASCII FOR BLANK SPACE
	ld	hl, _SCRN0
	ld	bc, SCRN_VX_B * SCRN_VY_B
	call	mem_SetVRAM

	ld		hl,ExplainNumbers
	ld		de,$9800+0+(SCRN_VY_B*15)
	ld		bc,64
	call		mem_CopyVRAM
	
; ****************************************************************************************
; Main Loop:
; ****************************************************************************************
MainLoop:
	halt				; add timer interrupt for much faster game play
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
	ld	h,01			; sprite ID 1
	ld	l,00			; sprite ID 0
	call	BSCheck4Clison		; are sprite IDs 1 and 0 colliding?
	push	af			; save flags
	call	c,ShowCollision		; if c is set, they are colliding
	pop	af
	call	nc,ShowNoCollision
	call	PrintBackgroundTiles
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
CollisionText:
;                00000000011111111112222222222333
;                12345678901234567890123456789012
	db	"Sprite: Collision               "
NoCollisionText:
	db	"Sprite: No Collision            "
ExplainNumbers:
	db	"bkgd tiles of crnrs:            "
	db	"UprL UprR LwrL LwrR             "

ShowCollision
	ld		hl,CollisionText
	ld		de,$9800+0+(SCRN_VY_B*13)
	ld		bc,32
	call		mem_CopyVRAM
	ret
ShowNoCollision
	ld		hl,NoCollisionText
	ld		de,$9800+0+(SCRN_VY_B*13)
	ld		bc,32
	call		mem_CopyVRAM
	ret

PrintBackgroundTiles:
	xor		a
	ld		b,BB_UPPER_LEFT
	ld		hl,_SCRN0
	call		bs_get_background_tile
	ld		hl,$9800+1+(SCRN_VY_B*17)
	call		PrintByte

	xor		a
	ld		b,BB_UPPER_RIGHT
	ld		hl,_SCRN0
	call		bs_get_background_tile
	ld		hl,$9800+6+(SCRN_VY_B*17)
	call		PrintByte

	xor		a
	ld		b,BB_LOWER_LEFT
	ld		hl,_SCRN0
	call		bs_get_background_tile
	ld		hl,$9800+11+(SCRN_VY_B*17)
	call		PrintByte

	xor		a
	ld		b,BB_LOWER_RIGHT
	ld		hl,_SCRN0
	call		bs_get_background_tile
	ld		hl,$9800+16+(SCRN_VY_B*17)
	call		PrintByte

	ret

right:
	ld		b,BS_LOCATION
	xor		a
	call		get_bs_param	; d=X coord, e=Y coord of sprite 0
	push		de
	ld		b,BB_LOWER_RIGHT
	call		get_bs_param	; d=X dim, e=Y dims of LWR_R
					; of BB sprite 0
	pop		bc		; bc contains current coords
	ld		a,b		; X coord
	add		d		; 
	sub		SCRN_X
	ret		z		; if (X coord=(SCRN_X + LWR_R(X))) then ret
	inc		b		; move 1 to right
	SET_BS_LOC	0,b,c		; set new location
	ret

left:	
	ld		b,BS_LOCATION
	xor		a
	call		get_bs_param	; d=X coord, e=Y coord of sprite 0
	push		de
	ld		b,BB_UPPER_LEFT
	call		get_bs_param	; d=X dim, e=Y dims of LWR_R
					; of BB sprite 0
	pop		bc		; b=X coord of Bounding Box
	ld		a,b		; X coord of bounding box
	add		d		; bounding box X coord - current X
	ret		z		; if (X coord=(UPR_L(X))) then ret
	dec		b
	SET_BS_LOC	0,b,c
	ret

down:
	ld		b,BS_LOCATION
	xor		a
	call		get_bs_param	; d=X coord, e=Y coord of sprite 0
	push		de
	ld		b,BB_LOWER_RIGHT
	call		get_bs_param	; d=X dim, e=Y dims of LWR_R
					; of BB sprite 0
	pop		bc		; bc contains current coords
	ld		a,c		; Y coord
	add		e
	sub		SCRN_Y
	ret		z		; if (Y coord=(SCRN_Y + LWR_R(Y))) then ret
	inc		c
	SET_BS_LOC	0,b,c
	ret

up:
	ld		b,BS_LOCATION
	xor		a
	call		get_bs_param	; d=X coord, e=Y coord of sprite 0
	push		de
	ld		b,BB_UPPER_LEFT
	call		get_bs_param	; d=X dim, e=Y dims of LWR_R
					; of BB sprite 0
	pop		bc		; c=Y coord of Bounding Box
	ld		a,c		; Y coord of bounding box
	add		e		; bounding box Y coord - current Y
	ret		z		; if (Y coord=(UPR_L(Y))) then ret
	dec		c
	SET_BS_LOC	0,b,c
	ret
	
simpleDelay:
	dec	bc
	ld	a,b
	or	c
	jr	nz, simpleDelay
	ret

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
