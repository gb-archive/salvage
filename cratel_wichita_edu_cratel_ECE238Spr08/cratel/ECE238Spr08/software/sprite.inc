;* sprite.inc
;* sprite defs
;
;* by John Harrison (so far)

;* 2008-Mar-31 --- V1.0
;* 2008-Apr-15 --- V1.1
;*                 added ability to read/write to sprites by sprite # as well
;*                 as by name.
;*                 added SetSpriteTileNum
;*                 added version checking macro
;* 2008-May-01 --- V1.1a
;*		   fixed check for register or # in GetSpriteXAddr and friends

        IF      !DEF(SPRITE_INC)
SPRITE_INC  SET  1

;***************************************************************************
REV_CHECK_SPRITE_INC: MACRO
;NOTE: REVISION NUMBER CHANGES MUST BE ADDED
;TO SECOND PARAMETER IN FOLLOWING LINE.
        IF      \1 > 1.1a      ;PUT REVISION NUMBER HERE
        FAIL    "Version \1 or later of 'sprite.inc' is required."
        ENDC
        ENDM

include "standard-defs.inc"

;***************************************************************************
;*
;* SPRITE DEFINITIONS
;*
;***************************************************************************

		RSSET	OAMDATALOC
SpriteAttr:	MACRO
\1YAddr		RB	1
\1XAddr		RB	1
\1TileNum	RB	1
\1Flags		RB	1
		ENDM


;***************************************************************************
; _FIND_SPRITE - locate sprite in memory by sprite #
; input: \1 is sprite #
; intended for internal use, so has a push without a pop
; output: hl = memory location pointing to sprite #
;***************************************************************************

_FIND_SPRITE:	MACRO
		push	hl
		ld	l,\1		; get sprite #
		sla	l
		sla	l		; multiply by 4
		ld	h,OAMDATALOCBANK
		ENDM

;***************************************************************************
;set XAddr of sprite.
;format:
;	PutSpriteXAddr	SpriteName,r8
;	PutSpriteXAddr	SpriteName,n8
PutSpriteXAddr:	MACRO
		push	af
		ld	a,\2
		add	8
		IF ((STRIN("0123456789$",STRSUB("\1",1,1))==0) && (STRIN("a",STRLWR("\1"))==0) && (STRIN("b",STRLWR("\1"))==0) && (STRIN("c",STRLWR("\1"))==0) && (STRIN("d",STRLWR("\1"))==0) && (STRIN("e",STRLWR("\1"))==0) && (STRIN("h",STRLWR("\1"))==0) && (STRIN("l",STRLWR("\1"))==0)) ; not a # or an 8-bit register
		ld	[\1XAddr],a
		ELSE
		_FIND_SPRITE	\1
		inc	hl	; point to X Addr
		ld	[hl],a
		pop	hl
		ENDC
		pop	af
		ENDM

;***************************************************************************
;set YAddr of sprite.
;format:
;	PutSpriteYAddr	SpriteName,r8
;	PutSpriteYAddr	SpriteName,n8
PutSpriteYAddr:	MACRO
		push	af
		ld	a,\2
		add	16	
		IF ((STRIN("0123456789$",STRSUB("\1",1,1))==0) && (STRIN("a",STRLWR("\1"))==0) && (STRIN("b",STRLWR("\1"))==0) && (STRIN("c",STRLWR("\1"))==0) && (STRIN("d",STRLWR("\1"))==0) && (STRIN("e",STRLWR("\1"))==0) && (STRIN("h",STRLWR("\1"))==0) && (STRIN("l",STRLWR("\1"))==0)) ; not a # or an 8-bit register
		ld	[\1YAddr],a
		ELSE
		_FIND_SPRITE	\1
		ld	[hl],a
		pop	hl
		ENDC
		pop	af
		ENDM

;***************************************************************************
;set TileNum of sprite.
;format:
;	SetSpriteTileNum	SpriteName,r8
;	SetSpriteTileNum	SpriteName,n8
;SpriteName may be either a SpriteName set with SpriteAttr or may be a
;sprite number, 0 being the sprite at $OAMDATALOC, 1 at $OAMDATALOC+4, etc.
SetSpriteTileNum:	MACRO
		push	af
		ld	a,\2
		IF ((STRIN("0123456789$",STRSUB("\1",1,1))==0) && (STRIN("a",STRLWR("\1"))==0) && (STRIN("b",STRLWR("\1"))==0) && (STRIN("c",STRLWR("\1"))==0) && (STRIN("d",STRLWR("\1"))==0) && (STRIN("e",STRLWR("\1"))==0) && (STRIN("h",STRLWR("\1"))==0) && (STRIN("l",STRLWR("\1"))==0)) ; not a # or an 8-bit register
		ld	[\1TileNum],a
		ELSE
		_FIND_SPRITE	\1
		inc	hl
		inc	hl		; point to the tile number
		ld	[hl],a
		pop	hl
		ENDC
		pop	af
		ENDM

;***************************************************************************
GetSpriteXAddr:	MACRO
		IF ((STRIN("0123456789$",STRSUB("\1",1,1))==0) && (STRIN("a",STRLWR("\1"))==0) && (STRIN("b",STRLWR("\1"))==0) && (STRIN("c",STRLWR("\1"))==0) && (STRIN("d",STRLWR("\1"))==0) && (STRIN("e",STRLWR("\1"))==0) && (STRIN("h",STRLWR("\1"))==0) && (STRIN("l",STRLWR("\1"))==0)) ; not a # or an 8-bit register
		ld	a,[\1XAddr]
		ELSE
		_FIND_SPRITE	\1
		inc	hl		; point to the X addr
		ld	a,[hl]
		pop	hl
		ENDC
		sub	8
		ENDM

;***************************************************************************
GetSpriteYAddr:	MACRO
		IF ((STRIN("0123456789$",STRSUB("\1",1,1))==0) && (STRIN("a",STRLWR("\1"))==0) && (STRIN("b",STRLWR("\1"))==0) && (STRIN("c",STRLWR("\1"))==0) && (STRIN("d",STRLWR("\1"))==0) && (STRIN("e",STRLWR("\1"))==0) && (STRIN("h",STRLWR("\1"))==0) && (STRIN("l",STRLWR("\1"))==0)) ; not a # or an 8-bit register
		ld	a,[\1YAddr]
		ELSE
		_FIND_SPRITE	\1
		ld	a,[hl]		; hl already points to y addr
		pop	hl
		ENDC
		sub	16
		ENDM

		ENDC