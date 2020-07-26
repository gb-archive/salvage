;* standard-defs.inc
;* definitions that every program seems to need
;
;* by John Harrison (so far)

;* 2008-Mar-30 --- V1.0

        IF      !DEF(STANDARD_DEFS_INC)
STANDARD_DEFS_INC  SET  1

DMACODELOC	EQU	$ff80
OAMDATALOC	EQU	_RAM				; must be multiple of $100
OAMDATALOCBANK	EQU	OAMDATALOC/$100 
OAMDATALENGTH	EQU	$A0

LoRamBase       SET     OAMDATALOC + OAMDATALENGTH	; low area of RAM for storing variables

LoByteVar:	MACRO
\1		EQU	LoRamBase
LoRamBase	SET	LoRamBase+1
		ENDM

LoWordVar:	MACRO
\1		EQU	LoRamBase
LoRamBase	SET	LoRamBase+2
		ENDM

;***************************************************************************
;* LOAD_ROM_HEADER
;* load the Nintendo Logo and ROM header
;* input:
;*   \1: cartridge name (max 15 bytes)
;***************************************************************************
; ROM header
LOAD_ROM_HEADER:	MACRO
			NINTENDO_LOGO
CName:		DB 	"\1" 				; Cart name: 15 bytes
CNameEnd:
		REPT 	15-(CNameEnd-CName)
		DB	0
		ENDR	
 		DB 	0                       ; $143
	 	DB 	0,0			; $144 - Licensee code (not important)
 		DB 	0        		; $146 - SGB Support indicator
 		DB 	ROM_NOMBC       	; $147 - Cart type
 		DB 	ROM_SIZE_32KBYTE	; $148 - ROM Size
 		DB	RAM_SIZE_0KBYTE         ; $149 - RAM Size
 		DB 	1              		; $14a - Destination code
 		DB 	$33                  	; $14b - Old licensee code
 		DB 	0                    	; $14c - Mask ROM version
 		DB 	0                    	; $14d - Complement check (important)
 		DW 	0                    	; $14e - Checksum (not important)
		ENDM

		ENDC	; STANDARD_DEFS_INC