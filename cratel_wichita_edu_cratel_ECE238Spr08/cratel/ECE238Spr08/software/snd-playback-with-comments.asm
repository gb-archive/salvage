;*
;* Raw Sample Playback Routines
;* with tutorial comments
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
;* V1.3a- 24-Mar-08 : added tutorial comments
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

; The first thing you must do is enable sound. You do this with the
; AUDENA register. You can turn all sound voices on or off by writing
; AUDENA_ON as a bitmask to the AUDENA register.
; With the AUDENA register you can also check for what sound circuits
; are on, but you cannot enable/disable sound circuits individually.
; When you shut off sound with AUDENA_OFF, you save battery power.
 
        ld      a,AUDENA_ON
        ldh      [rAUDENA],a      ;enable sound


	
        ld      a,AUD3ENA_OFF
        ldh     [rAUD3ENA],a

; AUDTERM allows you to choose which circuit goes to which channel
	ld	a,AUDTERM_OFF
        ldh     [rAUDTERM],a

; AUDVOL sets the volume of each channel. The range is 0 (softest)
; to 7 (loudest).
	ld	a,0
	SETAUDIOVOLSO1	7		; MAX VOLUME CHANNEL 1
	SETAUDIOVOLSO2	7		; MAX VOLUME CHANNEL 2
        ldh     [rAUDVOL],a		; select speakers


        ld      a,AUDTERMALLTOBOTH
        ldh     [rAUDTERM],a       ;ENABLE ALL 4 SOUND CIRCUITS TO BOTH CHANNELS

; AUD3LEN allows you to set the length a sound will be played in circuit 3.
; The formula is (256 - x)*.5 seconds
        SETAUD3LEN	80
        ldh     [rAUD3LEN],a       ;sound length

; AUD3LEVEL allows you to set the volume of circuit 3. There are 4 levels:
; AUD3LEVMUTE, AUD3LEVQRTR, AUD3LEVHALF, AUD3LEVFULL
        ld	a,AUD3LEVFULL
        ldh     [rAUD3LEVEL],a       ;sound level high

; choose a frequency and write it to both AUD3LOW and AUD3HIGH. AUD3HIGH
; also triggers the voice to start so we write to AUD3LOW first and the last
; thing we do is write to AUD3HIGH.

        SETAUDLOWHZ	512
        ldh     [rAUD3LOW],a       ;sound freq low

; we now write to the wave ram.
.samp2:
        ld      de,_AUD3WAVERAM ;12
        push    bc              ;16
        ld      b,16            ;16

; We load AUD3ENA with AUD3ENA_OFF so that voice 3 is not currently on.
; This is because we cannot load wave ram when voice 3 is enabled.

        ld	a,AUD3ENA_OFF
        ldh     [rAUD3ENA],a
.samp3:
        ld      a,[hl+]         ;8
        ld      [de],a          ;8
        inc     de              ;8
        dec     b               ;4
        jr      nz,.samp3       ;12

; as soon as we are done loading the wave ram, we turn the voice back on
        ld      a,AUD3ENA_ON
        ldh     [rAUD3ENA],a

; set our frequency and trigger the sound to start
	SETAUDHIGHHZ	512	; SET FREQ TO 512 Hz
	or	AUD1RESTART
        ldh     [rAUD3HIGH],a

; now wait while the voice plays the wave table.
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

; shut off output of voice 3 to both channels while we reload the wave ram
        ld      a,AUDTERMALLTOBOTH  & ~AUDTERM3TOS02  & ~AUDTERM3TOS01
        ldh     [rAUDTERM],a       ;disable sound 3
        ret

        ENDC    ;sample1_asm

