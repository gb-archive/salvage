; hello-sprite.inc
; definitions used in hello-sprite.asm

TimerHertz	EQU	TACF_START|TACF_4KHZ
TimerClockDiv	EQU	256-100				; divide clock by 100

TimerOFlowSet	EQU	1
TimerOFlowReset	EQU	0

DMACODELOC	EQU	$ff80
OAMDATALOC	EQU	_RAM				; must be multiple of $100
OAMDATALOCBANK	EQU	OAMDATALOC/$100 
OAMDATALENGTH	EQU	$A0

LoRamBase       SET     OAMDATALOC + OAMDATALENGTH	; low area of RAM for storing variables

LoByteVar:	MACRO
\1		EQU	LoRamBase
LoRamBase	SET	LoRamBase+1
		ENDM

		printv	OAMDATALOCBANK
		printt	"\n"
		
		RSSET	OAMDATALOC
SpriteAttr:	MACRO
\1YAddr		RB	1
\1XAddr		RB	1
\1TileNum	RB	1
\1Flags		RB	1
		ENDM

;set XAddr of sprite.
;format:
;	PutSpriteXAddr	SpriteName,r8
;	PutSpriteXAddr	SpriteName,n8
PutSpriteXAddr:	MACRO
		push	af
		ld	a,\2
		add	8	
		ld	[\1XAddr],a
		pop	af
		ENDM

;set YAddr of sprite.
;format:
;	PutSpriteYAddr	SpriteName,r8
;	PutSpriteYAddr	SpriteName,n8
PutSpriteYAddr:	MACRO
		push	af
		ld	a,\2
		add	16	
		ld	[\1YAddr],a
		pop	af
		ENDM

GetSpriteXAddr:	MACRO
		ld	a,[\1XAddr]
		sub	8
		ENDM

GetSpriteYAddr:	MACRO
		ld	a,[\1YAddr]
		sub	16
		ENDM