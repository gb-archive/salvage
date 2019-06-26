
;*
;* Turn GameBoy screen On / Off
;*
;*  This code can be used on GB Classic, GB Pocket,
;* Super GB, and GBC.
;*
;* Compiled with RGBASM
;*

:* Note:
;*  One of the most common misconceptions
;* about turning the GB screen off is that you
;* can accomplish this simply by setting the
;* high bit of register LCDC to 0 at any
;* random time. This is actually a "dirty"
;* way of turning the screen off. In fact,
;* if you don't wait until vblank sometimes
;* your code will not work on a Super GB
;* game that uses Super GB features and the
;* game will just lock up. There might be
;* cases on the GBC as well when not using
;* the following code (or something similar)
;* will cause you problems.


; *** Turn screen on ***

ScreenOn:
        ld      hl,rLCDC
        set     7,(hl)
        ret


; *** Turn screen off ***

ScreenOff:
        ld      hl,rLCDC
        bit     7,(hl)          ; Is LCD already off?
        ret     z               ; yes, exit

        ld      a,[rIE]
        push    af
        res     0,a
        ld      [rIE],a         ; Disable vblank interrupt if enabled

.loop:  ld      a,[rLY]         ; Loop until in first part of vblank
        cp      145
        jr      nz,.loop

        res     7,(hl)          ; Turn the screen off

        pop     af
        ld      [rIE],a         ; Restore the state of vblank interrupt
        ret

