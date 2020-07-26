;*
;* GBHW-SND.INC - Gameboy Hardware Sound definitions for GALP
;* an extension of gbhw.inc, the Gameboy Hardware definitions for GALP
;*
;* by John Harrison. Inspired from work done by GABY, whoever that is...
;*
;* 2008-Mar-08 --- V1.0 - The original version
;* 2008-Apr-01 --- V1.1 - added MEMHZ and extended SETAUDLOWGB and
;*                        SETAUDHIGHGB to support MEMHZ and lookup tables

; If all of these are already defined, don't do it again.

        IF      !DEF(HWSND_INC)
HWSND_INC  SET  1

rev_Check_hw_snd_inc: MACRO
;NOTE: REVISION NUMBER CHANGES MUST BE ADDED
;TO SECOND PARAMETER IN FOLLOWING LINE.
        IF      \1 > 1.0      ;PUT REVISION NUMBER HERE
        WARN    "Version \1 or later of 'gbhw-snd.inc' is required."
        ENDC
        ENDM

;***************************************************************************
;*
;* Sound control registers
;*
;***************************************************************************

; --
; -- AUDVOL/NR50 ($FF24)
; -- Channel control / ON-OFF / Volume (R/W)
; --
; -- Bit 7   - Vin->SO2 ON/OFF (Vin??)
; -- Bit 6-4 - SO2 output level (volume) (# 0-7)
; -- Bit 3   - Vin->SO1 ON/OFF (Vin??)
; -- Bit 2-0 - SO1 output level (volume) (# 0-7)
; --
; rNR50 	EQU 	$FF24
; rAUDVOL	EQU 	rNR50
AUDVOLVINSO2	EQU	%10000000
AUDVOLVINSO1	EQU	%00001000

SETAUDIOVOLSO2:	MACRO
		swap	a
		or	\1
		swap	a
		nop
		ENDM

SETAUDIOVOLSO1:	MACRO
		or	\1
		ENDM	
		

; --
; -- AUDTERM/NR51 ($FF25)
; -- Selection of Sound output terminal (R/W)
; --
; -- Bit 7   - Output sound 4 to SO2 terminal
; -- Bit 6   - Output sound 3 to SO2 terminal
; -- Bit 5   - Output sound 2 to SO2 terminal
; -- Bit 4   - Output sound 1 to SO2 terminal
; -- Bit 3   - Output sound 4 to SO1 terminal
; -- Bit 2   - Output sound 3 to SO1 terminal
; -- Bit 1   - Output sound 2 to SO1 terminal
; -- Bit 0   - Output sound 0 to SO1 terminal
; --
; rNR51 	EQU 	$FF25
; rAUDTERM	EQU 	rNR51
AUDTERM4TOS02	EQU	%10000000
AUDTERM3TOS02	EQU	%01000000
AUDTERM2TOS02	EQU	%00100000
AUDTERM1TOS02	EQU	%00010000
AUDTERM4TOS01	EQU	%00001000
AUDTERM3TOS01	EQU	%00000100
AUDTERM2TOS01	EQU	%00000010
AUDTERM1TOS01	EQU	%00000001
AUDTERMALLTOBOTH EQU	%11111111
AUDTERM_OFF	EQU	0

; --
; -- AUDENA/NR52 ($FF26)
; -- Sound on/off (R/W)
; --
; -- Bit 7   - All sound on/off (sets all audio regs to 0!)
; -- Bit 3   - Sound 4 ON flag (doesn't work!)
; -- Bit 2   - Sound 3 ON flag (doesn't work!)
; -- Bit 1   - Sound 2 ON flag (doesn't work!)
; -- Bit 0   - Sound 1 ON flag (doesn't work!)
; --
; rNR52 	EQU 	$FF26
; rAUDENA	EQU 	rNR52
AUDENA_ON	EQU	%10000000
AUDENA_OFF	EQU	%00000000
AUDENA_ON4	EQU	%00001000
AUDENA_ON3	EQU	%00000100
AUDENA_ON2	EQU	%00000010
AUDENA_0N1	EQU	%00000001


MEMHZ:		MACRO
		db	(2048 - (131072/\1)) - ((2048 - (131072/\1))/256)*256
		db	(2048 - (131072/\1))/256
		ENDM

SETAUDLOWGB:	MACRO
		IF	STRIN("abcdehl",STRLWR("\1")) > 0
		ld	a,\1
		ELSE
		ld	a,(\1 & $ff)
		ENDC
		ENDM

SETAUDLOWHZ:	MACRO
		SETAUDLOWGB	(2048 - (131072/\1))
		ENDM
		
SETAUDHIGHGB:	MACRO
		IF	STRIN("qabcdehl",STRLWR("\1")) > 0
		ld	a,\1
		ELSE
		ld	a,(\1/256)
		ENDC
		ENDM

SETAUDHIGHHZ:	MACRO
		SETAUDHIGHGB	(2048 - (131072/\1))
		ENDM

		

;***************************************************************************
;*
;* SoundChannel #1 registers
;*
;***************************************************************************

; --
; -- AUD1SWEEP/NR10 ($FF10)
; -- Sweep register (R/W)
; --
; -- Bit 6-4 - Sweep Time
; -- Bit 3   - Sweep Increase/Decrease
; --           0: Addition    (frequency increases???)
; --           1: Subtraction (frequency increases???)
; -- Bit 2-0 - Number of sweep shift (# 0-7)
; -- Sweep Time: (n*7.8ms)
; --
; rNR10 	EQU 	$FF10
; rAUD1SWEEP	EQU 	rNR10
AUD1SWEEPOFF	EQU	%0
AUD1SWEEP7MS	EQU	%00010000
AUD1SWEEP15MS	EQU	%00100000
AUD1SWEEP23MS	EQU	%00110000
AUD1SWEEP31MS	EQU	%01000000
AUD1SWEEP39MS	EQU	%01010000
AUD1SWEEP46MS	EQU	%01100000
AUD1SWEEP54MS	EQU	%01110000

AUD1SWEEPUP	EQU	%00000000
AUD1SWEEPDOWN	EQU	%00001000

AUD1SWEEPSHIFT0	EQU	%00000000	
AUD1SWEEPSHIFT1	EQU	%00000001	
AUD1SWEEPSHIFT2	EQU	%00000010	
AUD1SWEEPSHIFT3	EQU	%00000011	
AUD1SWEEPSHIFT4	EQU	%00000100	
AUD1SWEEPSHIFT5	EQU	%00000101	
AUD1SWEEPSHIFT6	EQU	%00000110	
AUD1SWEEPSHIFT7	EQU	%00000111	

; --
; -- AUD1LEN/NR11 ($FF11)
; -- Sound length/Wave pattern duty (R/W)
; --
; -- Bit 7-6 - Wave Pattern Duty (00:12.5% 01:25% 10:50% 11:75%)
; -- Bit 5-0 - Sound length data (# 0-63)
; --
; rNR11 	EQU 	$FF11
; rAUD1LEN	EQU 	rNR11
AUD1DUTY12	EQU	%00000000
AUD1DUTY25	EQU	%01000000
AUD1DUTY50	EQU	%10000000
AUD1DUTY75	EQU	%11000000
SETAUD1SNDLEN:	MACRO
		or	\1
		ENDM

; --
; -- AUD1ENV/NR12 ($FF12)
; -- Envelope (R/W)
; --
; -- Bit 7-4 - Initial value of envelope
; -- Bit 3   - Envelope UP/DOWN
; --           0: Decrease
; --           1: Range of increase
; -- Bit 2-0 - Number of envelope sweep (# 0-7)
; --
; rNR12 	EQU 	$FF12
; rAUD1ENV	EQU 	rNR12
AUD1ENVUP	EQU	%00001000
AUD1ENVDOWN	EQU	%00000000
SETAUD1INITVAL:	MACRO
		swap	a
		or	\1
		swap	a
		ENDM
SETAUD1ENVSWP:	MACRO
		or	\1
		ENDM
AUD1NOENV	EQU	0

; --
; -- AUD1LOW/NR13 ($FF13)
; -- Frequency lo (W)
; --
; rNR13 	EQU 	$FF13
; rAUD1LOW	EQU 	rNR13

; --
; -- AUD1HIGH/NR14 ($FF14)
; -- Frequency hi (W)
; --
; -- Bit 7   - Initial (when set, sound restarts)
; -- Bit 6   - Counter/consecutive selection
; -- Bit 2-0 - Frequency's higher 3 bits
; --
; rNR14 	EQU 	$FF14
; rAUD1HIGH	EQU 	rNR14

AUD1RESTART	EQU	%10000000
AUD1CONT_ON	EQU	%00000000
AUD1CONT_OFF	EQU	%01000000


;***************************************************************************
;*
;* SoundChannel #2 registers
;*
;***************************************************************************

; --
; -- AUD2LEN/NR21 ($FF16)
; -- Sound Length; Wave Pattern Duty (R/W)
; --
; -- see AUD1LEN for info
; --
;rNR21 EQU $FF16
;rAUD2LEN	EQU rNR21

AUD2DUTY12	EQU	%00000000
AUD2DUTY25	EQU	%01000000
AUD2DUTY50	EQU	%10000000
AUD2DUTY75	EQU	%11000000
SETAUD2SNDLEN:	MACRO
		or	\1
		ENDM


; --
; -- AUD2ENV/NR22 ($FF17)
; -- Envelope (R/W)
; --
; -- see AUD1ENV for info
; --
;rNR22 EQU $FF17
;rAUD2ENV	EQU rNR22

AUD2ENVUP	EQU	%00001000
AUD2ENVDOWN	EQU	%00000000
SETAUD2INITVAL:	MACRO
		swap	a
		or	\1
		swap	a
		ENDM
SETAUD2ENVSWP:	MACRO
		or	\1
		ENDM
AUD2NOENV	EQU	0


; --
; -- AUD2LOW/NR23 ($FF18)
; -- Frequency lo (W)
; --
;rNR23 EQU $FF18
;rAUD2LOW	EQU rNR23


; --
; -- AUD2HIGH/NR24 ($FF19)
; -- Frequency hi (W)
; --
; -- see AUD1HIGH for info
; --
;rNR24 EQU $FF19
;rAUD2HIGH	EQU rNR24

AUD2RESTART	EQU	%10000000
AUD2CONT_ON	EQU	%00000000
AUD2CONT_OFF	EQU	%01000000


;***************************************************************************
;*
;* SoundChannel #3 registers
;*
;***************************************************************************

; --
; -- AUD3ENA/NR30 ($FF1A)
; -- Sound on/off (R/W)
; --
; -- Bit 7   - Sound ON/OFF (1EQUON,0EQUOFF)
; --
; rNR30 	EQU 	$FF1A
; rAUD3ENA	EQU 	rNR30
AUD3ENA_ON	EQU	%10000000
AUD3ENA_OFF	EQU	%00000000


; --
; -- AUD3LEN/NR31 ($FF1B)
; -- Sound length (R/W)
; --
; -- Bit 7-0 - Sound length
; --
;rNR31 		EQU 	$FF1B
;rAUD3LEN	EQU 	rNR31
SETAUD3LEN:	MACRO
		ld	a,\1
		ENDM

; --
; -- AUD3LEVEL/NR32 ($FF1C)
; -- Select output level
; --
; -- Bit 6-5 - Select output level
; --           00: 0/1 (mute)
; --           01: 1/1
; --           10: 1/2
; --           11: 1/4
; --
;rNR32 		EQU 	$FF1C
;rAUD3LEVEL	EQU 	rNR32
AUD3LEVMUTE	EQU	%00000000
AUD3LEVFULL	EQU	%00100000
AUD3LEVHALF	EQU	%00010000
AUD3LEVQTR	EQU	%00110000



; --
; -- AUD3LOW/NR33 ($FF1D)
; -- Frequency lo (W)
; --
; -- see AUD1LOW for info
; --
;rNR33 EQU $FF1D
;rAUD3LOW	EQU rNR33


; --
; -- AUD3HIGH/NR34 ($FF1E)
; -- Frequency hi (W)
; --
; -- see AUD1HIGH for info
; --
;rNR34 EQU $FF1E
;rAUD3HIGH	EQU rNR34


; --
; -- AUD4LEN/NR41 ($FF20)
; -- Sound length (R/W)
; --
; -- Bit 5-0 - Sound length data (# 0-63)
; --
;rNR41 EQU $FF20
;rAUD4LEN	EQU rNR41
SETAUD4SNDLEN:	MACRO
		ld	a,\1
		ENDM


; --
; -- AUD4ENV/NR42 ($FF21)
; -- Envelope (R/W)
; --
; -- see AUD1ENV for info
; --
;rNR42 EQU $FF21
;rAUD4ENV	EQU rNR42

AUD4ENVUP	EQU	%00001000
AUD4ENVDOWN	EQU	%00000000
SETAUD4INITVAL:	MACRO
		swap	a
		or	\1
		swap	a
		ENDM
SETAUD4ENVSWP:	MACRO
		or	\1
		ENDM
AUD4NOENV	EQU	0

; --
; -- AUD4POLY/NR42 ($FF22)
; -- Polynomial counter (R/W)
; --
; -- Bit 7-4 - Selection of the shift clock frequency of the (scf)
; --           polynomial counter (0000-1101)
; --           freqEQUdrf*1/2^scf (not sure)
; -- Bit 3 -   Selection of the polynomial counter's step
; --           0: 15 steps
; --           1: 7 steps
; -- Bit 2-0 - Selection of the dividing ratio of frequencies (drf)
; --           000: f/4   001: f/8   010: f/16  011: f/24
; --           100: f/32  101: f/40  110: f/48  111: f/56  (fEQU4.194304 Mhz)
; --
;rNR42_2 EQU $FF22
;rAUD4POLY	EQU rNR42_2
AUD4STEP15	EQU	%00000000
AUD4STEP7	EQU	%00001000
SETAUD4CLCKFRQ:	MACRO
		swap	a
		or	\1
		swap	a
		ENDM
SETAUD4DRF:	MACRO
		or	\1
		ENDM

; --
; -- AUD4GO/NR43 ($FF23)
; -- (has wrong name and value (ff30) in Dr.Pan's doc!)
; --
; -- Bit 7 -   Inital
; -- Bit 6 -   Counter/consecutive selection
; --
;rNR43 		EQU 	$FF23
;rAUD4GO	EQU 	rNR43	; silly name!
AUD4RESTART	EQU	%10000000
AUD4CONT_ON	EQU	%00000000
AUD4CONT_OFF	EQU	%01000000

		ENDC
