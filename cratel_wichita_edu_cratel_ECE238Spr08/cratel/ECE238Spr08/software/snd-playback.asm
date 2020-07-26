;*
;* Raw Sample Playback Routines
;*
;*   Started 14-Jun-97
;*
;* Initials: JF = Jeff Frohwein
;*           JH = John Harrison
;*
;* V1.0 - 16-Jun-97 : Original Release - JF
;* V1.1 - 18-Aug-97 : Length included in data - JF
;* V1.2 - 29-Nov-97 : Interrupt handling taken out - JF
;* V1.3 - 04-Mar-08 : change for gbhw.inc, gbhw-snd.inc, and other cosmetics- JH
;*
;* Library Subroutines:
;*
;* snd_Sample1 -
;*    Playback raw sound sample at HL at 8192Hz rate.
;*
;*      The first DW at HL defines the length of the sample
;      in samples/32 or bytes/16. The format of the data at
;*     HL following the DW is 4-bit samples, 2 samples per
;*     byte, upper 4-bits played before lower 4 bits.
;*

; If all of these are already defined, don't do it again.

        IF      !DEF(SAMPLE1_ASM)
SAMPLE1_ASM  SET  1

rev_Check_sample1_asm: MACRO
;NOTE: REVISION NUMBER CHANGES MUST BE ADDED
;TO SECOND PARAMETER IN FOLLOWING LINE.
        IF      \1 > 1.2      ; <--- PUT REVISION NUMBER HERE
        WARN    "Version \1 or later of 'pan1.asm' is required."
        ENDC
        ENDM

; Definition includes

        INCLUDE "gbhw.inc"          ;Hardware defines
	INCLUDE "gbhw-snd.inc"

; Make sure include files are useable revisions.

        rev_Check_hardware_inc   1.0

        SECTION "Sample1 Code",HOME

snd_Sample1::
        ld      a,[hl+]         ;get sample length
        ld      c,a
        ld      a,[hl+]
        ld      b,a

        ld      a,AUDENA_ON
        ldh      [rAUDENA],a      ;enable sound

	
        ld      a,AUD3ENA_OFF
        ldh     [rAUD3ENA],a
	ld	a,AUDTERM_OFF
        ldh     [rAUDTERM],a

	ld	a,0
	SETAUDIOVOLSO1	7		; MAX VOLUME CHANNEL 1
	SETAUDIOVOLSO2	7		; MAX VOLUME CHANNEL 2
        ldh     [rAUDVOL],a		; select speakers
        ld      a,AUDTERMALLTOBOTH
        ldh     [rAUDTERM],a       ;ENABLE ALL 4 SOUND CIRCUITS TO BOTH CHANNELS

        SETAUD3LEN	80
        ldh     [rAUD3LEN],a       ;sound length
        ld	a,AUD3LEVFULL
        ldh     [rAUD3LEVEL],a       ;sound level high

        SETAUDLOWHZ	512
        ldh     [rAUD3LOW],a       ;sound freq low

.samp2:
        ld      de,_AUD3WAVERAM ;12
        push    bc              ;16
        ld      b,16            ;16

        ld	a,AUD3ENA_OFF
        ldh     [rAUD3ENA],a
.samp3:
        ld      a,[hl+]         ;8
        ld      [de],a          ;8
        inc     de              ;8
        dec     b               ;4
        jr      nz,.samp3       ;12

        ld      a,AUD3ENA_ON
        ldh     [rAUD3ENA],a

	SETAUDHIGHHZ	512	; SET FREQ TO 512 Hz
	or	AUD1RESTART
        ldh     [rAUD3HIGH],a


        ld      bc,558          ;delay routine
.samp4:
        dec     bc              ;8
        ld      a,b             ;4
        or      c               ;4
        jr      nz,.samp4       ;12

        ld      a,0             ;more delay
        ld      a,0
        ld      a,0

        pop     bc              ;12
        dec     bc              ;8
        ld      a,b             ;4
        or      c               ;4
        jr      nz,.samp2       ;12

        ld      a,AUDTERMALLTOBOTH  & ~AUDTERM3TOS02  & ~AUDTERM3TOS01
        ldh     [rAUDTERM],a       ;disable sound 3
        ret

        ENDC    ;sample1_asm

