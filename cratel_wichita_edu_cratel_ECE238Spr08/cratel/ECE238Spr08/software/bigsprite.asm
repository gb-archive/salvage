;*
;* BIGSPRITE.ASM - Gameboy Sprite extensions
;* supports BigSprites, which are compositions of multiple 8x8 sprites
;* also supports collision detection of BigSprites
;*
;* by John Harrison.
;*
;* 2008-Mar-22 --- V0.1 - The original version -- not complete or fully tested
;* 2008-Apr-08 ----V0.1a- Fixed a few typos in the comments
;* 2008-Apr-11 --- V0.2 - replaced several macros with get_bs_param
;*                        added ability to determine background tile
;*                        under specified BB corner

; If all of these are already defined, don't do it again.

        IF      !DEF(BIGSPRITE_INC)
BIGSPRITE_INC  SET  1

;***************************************************************************
;* SETTINGS
;* for basic use alter only the stuff in this code block
;* to make good use of the BigSprite library
;***************************************************************************

HARDCODEDBSES	EQU	2		; number of BigSprites you are hardcoding

; 2 example hardcoded BSes.
BIGSPRITES:				; leave this label intact
;* BigSprite 0:
	db	0		; BS ID (Any # OK but make the IDs unique)
	db	0,0		; location of upper left hand corner
	db	0,0,32,16	; bounding boxes (LUX,LUY,RLX,RLY)
	db	4,2		; dimensions in 8x8 sprites (X,Y)
	db	0,1,4,5		; row 1 sprite/tile numbers (4 in this case)
	db	2,3,6,7		; row 2 sprite/tile numbers (4 more)
	ds	22-8		; 22 possible sprite/tile numbers and we used 8
	db	0		; flags

;* BigSprite 1:
	db	1		; BS ID
	db	40,40		; location of upper left hand corner
	db	0,0,32,16	; bounding boxes (LUX,LUY,RLX,RLY)
	db	4,2		; dimensions in 8x8 sprites (X,Y)
	db	8,9,12,13	; row 1 sprite/tile numbers
	db	10,11,14,15	; row 2 sprite/tile numbers
	ds	22-8		; 22 possible sprite/tile numbers and we used 8
	db	0		; flags

;***************************************************************************
;* END OF SETTINGS
;* for common use, no more changes need to be made to this file
;***************************************************************************
include "sprite.inc"
include "memory.asm"

INITTILE	EQU	0	; offset for correlating tiles with sprites
BSHOME		EQU	$dc00	; BSes start memory location
BSEMPTY		EQU	$ff	; signifies no BS at this location
MAXBSES		EQU	32
BSSIZE		EQU	32	; each BS is 32 bytes

; CONSTANTS REPRESENT 2 OFFSETS FOR WHERE INFORMATION IS STORE IN A BS
BB_UPPER_LEFT	EQU	$34
BB_UPPER_RIGHT	EQU	$54
BB_LOWER_LEFT	EQU	$36
BB_LOWER_RIGHT	EQU	$56
BS_LOCATION	EQU	$12

		LoByteVar	BSLocX
		LoByteVar	BSDimX



;***************************************************************************
;* INIT_BS - intiialize BigSprites
;* must be executed before BigSprites are used
;*
;* input:
;*    none
;***************************************************************************
INIT_BS:	MACRO
	INIT_SPRITES		; blank out all sprites
	SET_SPRITES_4_BS	; create sprites with corresponding tile #s
	ld	hl,BSHOME+HARDCODEDBSES*BSSIZE		; init BigSprites
	ld	bc,(MAXBSES-HARDCODEDBSES)*BSSIZE
	ld	a,BSEMPTY
	call	mem_Set
	ld	hl,BIGSPRITES				; load in hardcoded BigSprites
	ld	de,BSHOME
	ld	bc,HARDCODEDBSES*BSSIZE
	call	mem_Copy
	call	update_bs
	ENDM

;***************************************************************************
;* INIT_SPRITES
;* zero out (initialize) OAM memory
;***************************************************************************
INIT_SPRITES:	MACRO
	ld	hl,OAMDATALOC
	ld	bc,OAMDATALENGTH
	ld	a,0
	call	mem_Set
	ENDM

;***************************************************************************
;* SET_SPRITES_4_BS
;* set each sprite to its corresponding tile #
;* i.e. sprite 0 = tile #INITTILE
;*      sprite 1 = tile #INITTILE+1
;*      ...
;*      sprite 31= tile #INITTILE+31
;***************************************************************************
SET_SPRITES_4_BS:	MACRO
	ld	hl,OAMDATALOC+2		; tile # for sprite
	ld	a,INITTILE		; load in offset
.loop\@:
	ld	[hl+],a			; load sprite with tile #
	inc	hl
	inc	hl
	inc	hl
	inc	a			; go to next sprite and tile #
	cp	32			; only do sprites 0-31
	jr	nz,.loop\@
	ENDM

;***************************************************************************
;* CREATE_BS
;* create a BigSprite in the BigSprite list
;* if there is no space in the list, return error
;* output:
;*   hl <-- memory location of new sprite
;***************************************************************************
CREATE_BS:	MACRO
	ld	a,BSEMPTY
	call	findbs
	ENDM

;***************************************************************************
;* find_bs
;* find a BigSprite by looking up its ID #
;* if there is no space in the list, return error
;* input:
;*   a = BigSprite ID #
;* output:
;*   hl <-- memory location of matching BigSprite
;***************************************************************************
find_bs:
	push	bc			; preserve bc and de
	push	de
	ld	hl, BSHOME		; point to BS list
	ld	c,a			; get BS #
	ld	b,MAXBSES		; countdown
	ld	de,BSSIZE		; skip by BSSIZE at a time
.loop:
	ld	a,[hl]
	cp	c			; does the BS# match?
	jr	z,.foundmatch
	add	hl,de			; nope. jump to next BS
	dec	b			; countdown
	jr	nz,.loop
	ld	hl,BSNOTFOUND		; we never found a matching ID
	jp	bs_error		; report error and quit
.foundmatch:
	pop	de			; restore de and bc
	pop	bc
	ret


;***************************************************************************
;* get_bs_param
;* return X and Y coordinates of a specified parameter of a BigSprite
;* input:
;*   a: BS ID
;*   b: specified parameter
;* output:
;*   d: x coordinate
;*   e: y coordinate
;***************************************************************************
get_bs_param:
	push	af
	push	hl		; don't destroy hl
	push	bc		; store parameter
	call	find_bs
	push	hl		; save where BS is
	swap	b		; do high order nibble first
	ld	a,b
	and	$f
	ld	b,a
.loopx	inc	hl		; move to X loc of parameter
	dec	b
	jr	nz,.loopx
	ld	d,[hl]		; load X loc of parameter
	pop	hl		; go back to where BS is
	pop	bc		; get parameter
	push	bc		; save again (to preserve c)
	ld	a,b
	and	$f
	ld	b,a
.loopy	inc	hl
	dec	b
	jr	nz,.loopy
	ld	e,[hl]
	pop	bc
	pop	hl
	pop	af
	ret

;***************************************************************************
;* GET_BS_LOC
;* DEPRECATED. USE get_bs_param INSTEAD.
;* return the X and Y coordinates of a specified BigSprite
;* input:
;*   \1: BS ID
;* output:
;*   d: x coordinate
;*   e: y coordinate
;***************************************************************************

GET_BS_LOC:	MACRO
	push	bc
	push	hl
	ld	a,\1
	ld	b,BS_LOCATION
	call	get_bs_param
	pop	hl
	pop	bc
	ENDM

;***************************************************************************
;* SET_BS_LOC
;* set the X and Y coordinates of a specified BigSprite
;* input:
;*   \1: BS ID
;*   \2: X coordinate
;*   \3: Y coordinate
;***************************************************************************
SET_BS_LOC:	MACRO
	ld	a,\1
	call	find_bs
	inc	hl
	ld	[hl],\2
	inc	hl
	ld	[hl],\3
	call	update_bs
	ENDM

;***************************************************************************
;* PUT_SPRITE
;* set the X and Y coordinates of sprites used for BigSprites
;* input:
;*   \1: sprite #
;*   \2: X coordinate
;*   \3: Y coordinate
;***************************************************************************
PUT_SPRITE:	MACRO
	push	hl		; preserve hl,bc,af (de isn't used)
	push	bc
	push	af
	ld	hl,OAMDATALOC	; point to sprites
	ld	c,\1		; sprite #
	sla	c
	sla	c		; multiply by 4
	ld	b,0
	add	hl,bc		; now pointing to sprite loc of Y
	ld	a,\3
	add	16
	ld	[hl+],a
	ld	a,\2
	add	8
	ld	[hl],a
	pop	af
	pop	bc
	pop	hl
	ENDM

;***************************************************************************
;* update_bs
;* update sprites with info from BigSprites
;* so that sprites will print correctly on the GB screen
;***************************************************************************
update_bs:
	ld	hl,BSHOME
.processsprite:
	push	hl		; store BS beginning location
	ld	a,[hl]
	cp	BSEMPTY		; all BSes done
	jr	z,.done
	inc	hl
	ld	a,[hl+]		; location: x-coordinate
	ld	[BSLocX],a	; save for later
	ld	d,a
	ld	a,[hl+]		; location: y-coordinate
	ld	e,a
	inc	hl		; skip bounding boxes
	inc	hl
	inc	hl
	inc	hl
	ld	a,[hl+]		; X dimension in b
	ld	[BSDimX],a
	ld	b,a
	ld	c,[hl]		; Y dimension in c
.morex
	inc	hl		; point to number of next sprite #
	ld	a,[hl]
	PUT_SPRITE	a,d,e	;af,bc,de,hl preserved
	dec	b
	jr	z,.nomorex
	ld	a,d
	add	8
	ld	d,a
	jr	.morex
.nomorex:
	dec	c
	jr	z,.nextsprite
	ld	a,[BSLocX]
	ld	d,a
	ld	a,[BSDimX]
	ld	b,a
	ld	a,e
	add	8
	ld	e,a
	jr	.morex
.nextsprite:
	pop	hl		; point to beginning of BS
	ld	de,BSSIZE
	add	hl,de		; point to beginning of next BS
	jr	.processsprite	
.done:
	pop	hl
	ret


;***************************************************************************
;* GET_BB_UPR_L
;* DEPRECATED. USE get_bs_param INSTEAD.
;* return the X and Y dims of upper L corner of bounding box
;* for a specified BigSprite
;* input:
;*   \1: BS ID
;* output:
;*   d: x dimension
;*   e: y dimension
;***************************************************************************

GET_BB_UPR_L:	MACRO
	push	bc
	push	hl
	ld	a,\1
	ld	b,BB_UPPER_LEFT
	call	get_bs_param
	pop	hl
	pop	bc
	ENDM

;***************************************************************************
;* GET_BB_LWR_R
;* DEPRECATED. USE get_bs_param INSTEAD.
;* return the X and Y dims of lower R corner of bounding box
;* for a specified BigSprite
;* input:
;*   \1: BS ID
;* output:
;*   d: x coordinate
;*   e: y coordinate
;***************************************************************************
GET_BB_LWR_R:	MACRO
	push	bc
	push	hl
	ld	a,\1
	ld	b,BB_LOWER_RIGHT
	call	get_bs_param
	pop	hl
	pop	bc
	ENDM

;***************************************************************************
;* BSCheck4Clison
;* test if 2 BigSprites are colliding
;* if ((1st BS LWR_R) > (2nd BS UPR_L)) && ((2nd BS LWR_R) > (1st BS UPR_L))
;* then we have a collision 
;* input:
;*   h: BigSprite ID #
;*   l: BigSprite ID #
;* output:
;*   c: set if sprites collide
;*   c: reset if sprites do NOT collide
;***************************************************************************
BSCheck4Clison:
; get 1st BS LRW_R:
	push		hl	; save #IDs of BSes
	ld		a,h
	ld		b,BS_LOCATION
	call		get_bs_param
	push		de	; save X,Y of 1st BS
	ld		b,BB_LOWER_RIGHT
	call		get_bs_param	; de = LWR_R BB
	pop		hl	; hl = current X,Y loc of 1st BS
	ld		a,h
	add		d
	ld		h,a	; h = current X loc of LWR_R BB
	ld		a,l
	add		e
	ld		l,a	; l = current Y loc of LRW_R BB
	pop		de	; get BS IDs (stack now empty for this routine)
	push		hl	; save current X and Y of LRW_R BB
	push		de	; hl <==> de
	pop		hl	; HL = BS IDs
	push		hl

; get 2nd BS UPR_L
	ld		a,l
	ld		b,BS_LOCATION
	call		get_bs_param
	push		de	; save X,Y of 2nd BS 
	ld		b,BB_UPPER_LEFT
	call		get_bs_param	; de = UPR_L BB 2nd BS
	pop		hl	; hl = current X,Y of 2nd BS
	ld		a,h
	add		d
	ld		h,a
	ld		a,l
	add		e
	ld		l,a	; hl = X,Y of loc of UPR_L BB 2nd BS
	pop		bc	; bc = BS IDs
	pop		de	; de = X,Y of loc of LWR_R BB 1st BS
; (1st BS LWR_R) > (2nd BS UPR_L) ?
	
	cp		e	; l > e ? l-e > 0
	ret		nc	; l >= e so no collision
	ld		a,h
	cp		d
	ret		nc
; now test for ((2nd BS LWR_R) > (1st BS UPR_L))
; get 2nd BS UPR_R:
	push		bc
	pop		hl	; ld hl,bc
	push		hl
	ld		a,l
	ld		b,BS_LOCATION
	call		get_bs_param
	push		de
	ld		b,BB_LOWER_RIGHT
	call		get_bs_param	; de = LWR_R BB
	pop		hl	; hl = current X,Y loc of 1st BS
	ld		a,h
	add		d
	ld		h,a	; h = current X loc of LWR_R BB
	ld		a,l
	add		e
	ld		l,a	; l = current Y loc of LRW_R BB
	pop		de	; get BS IDs
	push		hl	; save current X and Y of LRW_R BB
	push		de
	pop		hl	; HL <--> DE

; get 2nd BS UPR_L
	ld		a,h
	ld		b,BS_LOCATION
	call		get_bs_param
	push		de
	ld		b,BB_UPPER_LEFT
	call		get_bs_param
	pop		hl	; hl = current X,Y of 2nd BS
	ld		a,h
	add		d
	ld		h,a
	ld		a,l
	add		e
	ld		l,a	; hl = X,Y of loc of UPR_L BB 2nd BS
	pop		de	; de = X,Y of loc of LRW_R BB 1st BS
; (1st BS LWR_R) > (2nd BS UPR_L) ?
	
	cp		e	; l > e ? l-e > 0
	ret		nc	; l < e so no collision
	ld		a,h
	cp		d
	ret


;***************************************************************************
;* bs_get_background_tile
;* return the background tile hitting a specified corner of a specified sprite
;* for a corner of the bounding box (x,y) and z = $9800 or $9c00
;* do the formula:
;* memory location = (x+rSCX)/8+((y+rSCY)/8)*SCRN_VY_B+z
;* input:
;*   a: BigSprite ID #
;*   b: corner to check (BB_UPPER_LEFT, BB_UPPER_RIGHT,
;*                       BB_LOWER_LEFT, BB_LOWER_RIGHT)
;*   hl: z
;* output:
;*   a: tile number hitting the corner
;***************************************************************************
bs_get_background_tile:
	push	bc		; preserve bc
	push	de		; preserve de
	push	hl		; preserve hl
	call	get_bs_param	; de = (x,y) of bounding box
	push	de
	ld	b,BS_LOCATION
	call	get_bs_param	; de = (x,y) of sprite location
	pop	hl		; hl = (x,y) of bounding box
	add	hl,de		; hl = (x,y) in formula in comments above
	ld	a,[rSCX]
	add	a,h		; x + rSCX
	ld	h,a		; h = x+rSCX
	ld	a,[rSCY]
	add	a,l
	ld	l,a		; l = y+rSCY
	REPT	3		; divide h and l by 8
	srl	h
	srl	l
	ENDR

	ld	e,h		; store h for a sec
	ld	h,0

	REPT	5		; mult hl by 32 (SCRN_VY_B)
	sla	l
	rl	h
	ENDR			; hl = ((y+rSCY)/8)*SCRN_VY_B
	ld	d,0
	add	hl,de		; hl = hl + (x+rSCX)/8
	push	hl
	pop	de		; de = hl
	pop	hl		; hl = z
	push	hl		; save hl
	add	hl,de		; hl = hl + z
	lcd_WaitVRAM
	ld	a,[hl]		; tile #
	pop	hl		; restore the registers
	pop	de
	pop	bc
	ret

;***************************************************************************
;* bs_error
;* report error and hang forever
;* input:
;*   HL <-- memory location with error text
;***************************************************************************
bs_error:
        ld      de,$9800+0+(SCRN_VY_B*17)
        ld      bc,32
        call    mem_Copy
.loop	jr	.loop

;			 00000000011111111112222222222333
;			 12345678901234567890123456789012
BSNOTFOUND:	db	"BS ID not found                 "

	ENDC	;BIGSPRITE_INC