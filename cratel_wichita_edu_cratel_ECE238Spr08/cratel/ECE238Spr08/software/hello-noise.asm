;* Hello Noise
;* a brief demo of GameBoy audio...

;* by John Harrison

;* 2008-Mar-05 --- V1.0
;* 2008-Mar-26 --- V1.1 fixed minor bugs and some cosmetics
;*			now runs correctly on real GameBoy
;* 2008-Apr-01 --- V1.2 added demo code to show playing notes
;*                      using register values and lookup table
;*                      replaced hello-noise.inc with standard-defs.inc

INCLUDE "gbhw.inc" ; standard hardware definitions from devrs.com
INCLUDE "gbhw-snd.inc" ; hardware defs for sound
INCLUDE "ibmpc1.inc" ; ASCII character set from devrs.com
INCLUDE "standard-defs.inc" ; specific defs

;***************************************************************************
;* PRINT_TEXT
;* prints the next line of text
;***************************************************************************
TEXT_COUNTER	SET	0
PRINT_TEXT:	MACRO
		ld	hl,TEXT+(TEXT_COUNTER*32)
		ld	de, _SCRN0+(SCRN_VY_B*TEXT_COUNTER) ; 
		ld	bc, 32
		call	mem_CopyVRAM
TEXT_COUNTER	SET	TEXT_COUNTER + 1
		ENDM
	
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
; LOAD RAW AUDIO 4 BIT SAMPLES IN UPPER 16K OF 32K CARTRIDGE
; ****************************************************************************************

SECTION "raw-audio",CODE[$4000]
AUDIOBYTES	EQU	9130
AUDIOLEN	EQU	AUDIOBYTES/16
RAWAUDIO:
		db	AUDIOLEN-((AUDIOLEN/256)*256) 	; LOW BYTE
		db	AUDIOLEN/256			; HIGH BYTE
		INCBIN 	"TEST8BIT.GBW"

; ****************************************************************************************
; boot loader jumps to here.
; ****************************************************************************************
SECTION	"start",HOME[$0100]
	nop
	jp	begin

; ****************************************************************************************
; ROM HEADER and ASCII character set
; ****************************************************************************************

	LOAD_ROM_HEADER	Hello-Noise

INCLUDE "memory.asm"
INCLUDE	"snd-playback.asm"	; SOUND ROUTINE TO PLAY AUDIO SAMPLES ON SOUND CIRCUIT 3
TileData:
	chr_IBMPC1	1,8 	; LOAD ENTIRE CHARACTER SET

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

	ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ8|LCDCF_OBJON ; *hs* see gbspec.txt lines 1525-1565 and gbhw.inc lines 70-86
	ld	[rLCDC], a	
	ld	a, 32		; ASCII FOR BLANK SPACE
	ld	hl, _SCRN0
	ld	bc, SCRN_VX_B * SCRN_VY_B
	call	mem_SetVRAM


; ****************************************************************************************
; Main code:
; ****************************************************************************************

; ****************************************************************************************
; DEMO: CHANNEL 3
; NOTE: WITH THIS EXAMPLE, TIGHT DELAY LOOP PREVENTS MUCH ELSE FROM HAPPENING
; WHILE PLAYING THE AUDIO SAMPLE. THIS COULD BE REWRITTEN TO USE INTERRUPTS
; ****************************************************************************************
	PRINT_TEXT

	ld	hl, RAWAUDIO
	call	snd_Sample1
	call	WaitForKey


; ****************************************************************************************
; DEMO: CHANNEL 1
; SET THE TONES, ENVELOPES, SWEEP, AND GO DO SOMETHING ELSE.
; THE GAMEBOY HARDWARE DOES ALL THE HARD WORK
; ****************************************************************************************
	PRINT_TEXT		; CHANNEL 1 PLAYING AUDIO SAMPLES

	ld	a,AUD1SWEEPOFF | AUD1SWEEPUP | AUD1SWEEPSHIFT0
	ld	[rAUD1SWEEP],a		; no sweep

	xor	a
	SETAUD1SNDLEN	0		; max sound length
	or	AUD1DUTY50		; max sound length, square wave
	ld	[rAUD1LEN],a

	xor	a
	SETAUD1INITVAL	$f
	SETAUD1ENVSWP	0		; no envelope, constant tone
	ld	[rAUD1ENV],a

	SETAUDLOWHZ	440
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	440
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	call	WaitForKey
	PRINT_TEXT			; CHANGE THE TIMBRE

	ld	a,AUD1DUTY12
	ld	[rAUD1LEN],a

	call	WaitForKey
	PRINT_TEXT			; SOFTER

	xor	a
	SETAUD1INITVAL	1
	SETAUD1ENVSWP	0
	ld	[rAUD1ENV],a
	SETAUDLOWHZ	440
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	440
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	call	WaitForKey
	PRINT_TEXT			; Now an Octave Lower

	xor	a
	SETAUD1INITVAL	$f		; MAKE IT LOUD AGAIN
	SETAUD1ENVSWP	0
	ld	[rAUD1ENV],a

	ld	a,AUD1DUTY50		; GO BACK TO "NICER" SOUND
	ld	[rAUD1LEN],a

; We do not have to hardcode all frequencies we want to play
; at assemble time. Another option is to make a lookup table of
; frequencies we want to play. Here is an example, with a lookup
; table of size 1.

	ld	hl,OurFreq
	ld	a,[hl+]
	SETAUDLOWGB	a
	ld	[rAUD1LOW],a
	ld	a,[hl]
	SETAUDHIGHGB	a

	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	call	WaitForKey
	PRINT_TEXT			; SWEEP

	ld	a,AUD1SWEEP54MS|AUD1SWEEPSHIFT7|AUD1SWEEPUP
	ld	[rAUD1SWEEP],a
	SETAUDHIGHHZ	220
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	call	WaitForKey
	PRINT_TEXT			; SWEEP FASTER
	
	SETAUDLOWHZ	220
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	220
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	ld	a,AUD1SWEEP7MS|AUD1SWEEPSHIFT6|AUD1SWEEPUP
	ld	[rAUD1SWEEP],a

	call	WaitForKey
	PRINT_TEXT			; SWEEP DOWN

	ld	a,AUD1SWEEP7MS|AUD1SWEEPSHIFT6|AUD1SWEEPDOWN
	ld	[rAUD1SWEEP],a
	SETAUDLOWHZ	12000
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	12000
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	call	WaitForKey
	PRINT_TEXT	;With Decay

	xor	a
	SETAUD1INITVAL	$f
	SETAUD1ENVSWP	7
	or	AUD1ENVDOWN
	ld	[rAUD1ENV],a

	ld	a,AUD1SWEEP7MS|AUD1SWEEPSHIFT6|AUD1SWEEPDOWN
	ld	[rAUD1SWEEP],a

	SETAUDLOWHZ	12000
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	12000
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	call	WaitForKey
	PRINT_TEXT	;Getting louder instead

	xor	a
	SETAUD1INITVAL	$0
	SETAUD1ENVSWP	7
	or	AUD1ENVUP
	ld	[rAUD1ENV],a
	ld	a,AUD1SWEEP7MS|AUD1SWEEPSHIFT6|AUD1SWEEPDOWN
	ld	[rAUD1SWEEP],a

	SETAUDLOWHZ	12000
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	12000
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

	call	WaitForKey

; ****************************************************************************************
; DEMO: CHANNELS 1 & 2 TOGETHER
; ****************************************************************************************

	PRINT_TEXT	;Two Tones

; TONE 1
	xor	a
	SETAUD1INITVAL	$f		; MAKE IT LOUD AGAIN
	SETAUD1ENVSWP	7
	ld	[rAUD1ENV],a

	ld	a,AUD1DUTY50		; GO BACK TO "NICER" SOUND
	ld	[rAUD1LEN],a

	ld	a,AUD1SWEEPOFF
	ld	[rAUD1SWEEP],a

	SETAUDLOWHZ	220
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	220
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

; TONE 2
	xor	a
	SETAUD2SNDLEN	0		; max sound length
	or	AUD2DUTY50		; max sound length, square wave
	ld	[rAUD2LEN],a

	xor	a
	SETAUD2INITVAL	$f
	SETAUD2ENVSWP	7		; decay envelope
	or	AUD2ENVDOWN
	ld	[rAUD2ENV],a

	SETAUDLOWHZ	740
	ld	[rAUD2LOW],a
	SETAUDHIGHHZ	740
	or	AUD2RESTART|AUD2CONT_ON
	ld	[rAUD2HIGH],a

	call	WaitForKey
	PRINT_TEXT	;Stereo

; TONE 1
	xor	a
	SETAUD1INITVAL	$f		; MAKE IT LOUD AGAIN
	SETAUD1ENVSWP	0
	ld	[rAUD1ENV],a

	ld	a,AUD1DUTY50		; GO BACK TO "NICER" SOUND
	ld	[rAUD1LEN],a
	ld	a,AUD1SWEEPOFF
	ld	[rAUD1SWEEP],a

	SETAUDLOWHZ	220
	ld	[rAUD1LOW],a
	SETAUDHIGHHZ	220
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

; TONE 2
	xor	a
	SETAUD2SNDLEN	0		; max sound length
	or	AUD2DUTY50		; max sound length, square wave
	ld	[rAUD2LEN],a

	xor	a
	SETAUD2INITVAL	$f
	SETAUD2ENVSWP	0		; decay envelope
	or	AUD2ENVDOWN
	ld	[rAUD2ENV],a

	SETAUDLOWHZ	740
	ld	[rAUD2LOW],a
	SETAUDHIGHHZ	740
	or	AUD2RESTART|AUD2CONT_ON
	ld	[rAUD2HIGH],a

	ld	b,4
stereoLoop:
	push	bc
	ld	a,AUDTERM1TOS01|AUDTERM2TOS02
	ld	[rAUDTERM],a
	call	longDelay
	ld	a,AUDTERM1TOS02|AUDTERM2TOS01
	ld	[rAUDTERM],a
	call	longDelay
	pop	bc
	dec	b
	jr	nz, stereoLoop
	call	WaitForKey

; ****************************************************************************************
; DEMO: CHANNEL 4 -- the Noise channel
; ****************************************************************************************
	PRINT_TEXT	;Noise

	ld	a,AUDENA_OFF
	ld	[rAUDENA],a
	ld	a,AUDENA_ON
	ld	[rAUDENA],a
	ld	a,AUDTERMALLTOBOTH	; SET ALL SOUND CIRCUITS TO BOTH CHANNELS
	ld	[rAUDTERM],a
	
	xor	a
	SETAUDIOVOLSO1	7
	SETAUDIOVOLSO2	7
	ld	[rAUDVOL],a

	xor	a
	SETAUD4SNDLEN	0			; max sound length
	ld	[rAUD4LEN],a

	ld	a, AUD4ENVDOWN
	SETAUD4INITVAL	$f		; MAX VOLUME
	SETAUD4ENVSWP	$7		; MAX SWEEP TIME
	ld	[rAUD4ENV],a

	ld	a,AUD4STEP15
	SETAUD4CLCKFRQ	0
	SETAUD4DRF	0
	ld	[rAUD4POLY],a

	ld	a,AUD4CONT_ON|AUD4RESTART
	ld	[rAUD4GO],a

	call	WaitForKey
	PRINT_TEXT	;A different noise

	ld	a,AUD4STEP15
	SETAUD4CLCKFRQ	7
	SETAUD4DRF	7
	ld	[rAUD4POLY],a
	xor	a
	ld	a,AUD4CONT_ON|AUD4RESTART
	ld	[rAUD4GO],a
	
	call	WaitForKey
	PRINT_TEXT	;Noise with tones

; ***************NOISE **********************
	SETAUD4SNDLEN	0			; max sound length
	ld	[rAUD4LEN],a
	ld	a, AUD4ENVDOWN
	SETAUD4INITVAL	$f		; MAX VOLUME
	SETAUD4ENVSWP	$7		; MAX SWEEP TIME
	ld	[rAUD4ENV],a

	ld	a,AUD4STEP15
	SETAUD4CLCKFRQ	0
	SETAUD4DRF	0
	ld	[rAUD4POLY],a

	ld	a,AUD4CONT_ON|AUD4RESTART
	ld	[rAUD4GO],a

; ***************TONE 1 ************************
; TONE 1
	xor	a
	SETAUD1INITVAL	$f		; MAKE IT LOUD AGAIN
	SETAUD1ENVSWP	7
	ld	[rAUD1ENV],a

	ld	a,AUD1DUTY50		; GO BACK TO "NICER" SOUND
	ld	[rAUD1LEN],a

	ld	a,AUD1SWEEP31MS|AUD1SWEEPSHIFT5|AUD1SWEEPUP
	ld	[rAUD1SWEEP],a

	xor	a
	SETAUDLOWHZ	220
	ld	[rAUD1LOW],a

	xor	a
	SETAUDHIGHHZ	220
	or	AUD1RESTART|AUD1CONT_ON
	ld	[rAUD1HIGH],a

; ***************TONE 2 ************************
; TONE 2
	xor	a
	SETAUD2SNDLEN	0		; max sound length
	or	AUD2DUTY50		; max sound length, square wave
	ld	[rAUD2LEN],a

	xor	a
	SETAUD2INITVAL	$f
	SETAUD2ENVSWP	7		; decay envelope
	or	AUD2ENVDOWN
	ld	[rAUD2ENV],a

	SETAUDLOWHZ	740
	ld	[rAUD2LOW],a
	SETAUDHIGHHZ	740
	or	AUD2RESTART|AUD2CONT_ON
	ld	[rAUD2HIGH],a
	
	call	WaitForKey
	PRINT_TEXT	;Oh what fun that was...
	jr	wait

WaitForKey:
	call	longDelay	; sloppy hack to make sure we don't read 1 keypress as 2
  	ld    a, P1F_5   ; with either P14 or P15 low, we read the low line. In this case we read P14 since P15 is high
	ldh    [rP1],a    ; tell the P1 register we want to read input from the keypad
.noInput:
	ldh    a, [rP1]   ; read the input. Because of bouncing we shouldread this
                        ; several times if we really care what specific key we read.
                        ; In this case, we only care that a key was read at all.
    	cpl                ; invert a
    	and    $0f          ; look only at the lower nibble and set the flags.
    	jr    z, .noInput   ; no key was pressed
.input:	ld	a, [rP1]
	cpl
	and	$0f
	jr	nz,.input
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
OurFreq:	MEMHZ	220	; macro to store frequencies

TEXT:
;		"12345678901234567890            " ; count the columns
	DB	"Wave Samples (#3)               "
	DB	"An 'A' (#1)                     "
	DB	"Change the timbre               "
	DB	"Softer                          "
	DB	"Now an Octave Lower             "
	DB	"Sweep                           "
	DB      "Faster Sweep                    "
        DB	"Sweep down                      "
	DB	"With Decay                      "
	DB	"Getting louder                  "
	DB	"Two Tones (1&2)                 "
	DB	"Stereo                          "
	DB	"Noise w/ Decay (4)              "
	DB	"A different noise               "
	DB	"Noise&tones (1,2,4)             "
	DB	"Fun while it lasted.            "
; simpleDelay is no longer used in this code. We can't predict
; how long simpleDelay will delay since we do not know how fast the CPU
; is. GameBoy CPU speeds vary between models: 
longDelay
	ld	bc,$ffff
simpleDelay:
	dec	bc
	ld	a,b
	or	c
	jr	nz, simpleDelay
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
