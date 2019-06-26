;*****************************************************************************
;*                                                                           *
;* Code for the kernel of the operating system. This handles task switches,  *
;* launching new tasks, memory allocation etc                                *
;* TH/3/3/96 Tom Hammersley - tomh@globalnet.co.uk                           *
;*                                                                           *
;*****************************************************************************
;*****************************************************************************
;*                                                                           *
;* This switches the bank at A000-C000 to the one spec'd in A                *
;*                                                                           *
;*****************************************************************************

bankswitch:
        and     $03
        ld      ($4000), a
        ret

;*****************************************************************************
;*                                                                           *
;* This code switches the current task.                                      *
;* It does so by using the data field in the current RAM bank, and figuring  *
;* out what to switch to.                                                    *
;* Data field:                                                               *
;*                                                                           *
;* A000 : BankNo       DB      0                                             *
;* A001 : ActiveTask   DB      0                                             *
;* A002 : RegAF        DW      0                                             *
;* A004 : RegBC        DW      0                                             *
;* A006 : RegDE        DW      0                                             *
;* A008 : RegHL        DW      0                                             *
;* A00A : RegSP        DW      0                                             *
;* A00B : RegPC        DW      0                                             *
;* A00C : FreeBase     DW      0                                             *
;* A00D : TaskName     DB      16 dup(0)                                     *
;*                                                                           *
;*****************************************************************************
taskswitch:
        push    af              ; save this 4 l8r
        push    hl              ; now save all regs in2 the data field ...
        ld      hl, $A004       ; fetch BC ...
        ld      (hl), c         ; lobyte
        inc     hl              ;
        ld      (hl), b         ; hibyte
        inc     hl              ;
        ld      (hl), e         ; fetch DE
        inc     hl              ;
        ld      (hl), d         ;
        inc     hl              ;
        ld      d, h            ; DE is saved, so we can use it for mem
        ld      e, l            ;
        pop     hl              ; restore HL
        ld      a, h            ; now copy HL in2 (de) thru A
        ld      (de), a         ;
        inc     de              ;
        ld      a, l            ;
        ld      (de), a         ;
        ld      ($A00A), sp     ; all regs saved.
        pop     de              ; get AF
        ld      hl, $A002       ; index in2 block
        ld      (hl), e         ; save lo byte
        inc     hl              ;
        ld      (hl), d         ; save hibyte
        add     sp, 2
        pop     de              ; put old IP in2 DE
        ld      hl, $A00B       ;
        ld      (hl), e         ;
        inc     hl              ;
        ld      (hl), d         ; save old IP
        ld      a, ($A000)      ; get current bank
        inc     a               ; point to next bank
        and     3               ; clip to 4 banks
        ld      hl, $A001       ;
find_next:                      ;
        ld      ($4000), a      ; switch bank
        ld      b, (hl)         ; check for active task
        ld      c, a            ; save bank ...
        ld      a, b            ; get task status
        or      a               ; update flags
        jr      nz, found_task  ; if != 0, then have found task
        ld      a, c            ; get back bank
        inc     a               ; next bank
        and     3               ; clip
        jr      find_next       ; continue search
found_task:                     ; load up regs
        ld      hl, ($A002)     ; point to block
        ld      e, (hl)         ; now I gotta get AF & regs ...
        inc     hl              ;
        ld      d, (hl)         ;
        inc     hl              ;
        push    de              ;
        pop     af              ; get the AF regs
        ld      c, (hl)         ; load up BC, DE
        inc     hl              ;
        ld      b, (hl)         ;
        inc     hl              ;
        ld      e, (hl)         ;
        inc     hl              ;
        ld      d, (hl)         ; now for the tricky part ...
        inc     hl              ;
        push    af              ; save these
        push    de              ;
        ld      d, h            ;
        ld      e, l            ; get hl ...
        ld      a, (de)         ;
        ld      l, a            ;
        inc     de              ;
        ld      a, (de)         ;
        ld      h, a            ;
        pop     de              ;
        pop     af              ; put back old regs
        ld      sp, ($A00A)     ; now load SP
        ret                     ; just use a normal ret :- this code is called

;  And there we have it. A simple way to switch tasks on the Gameboys Z80 chip
; Not very elegant, I admit, but theres not exactly much room to work with
; on this system. I have decided that each 'task' will be given its own 8kb
; RAM bank, and within that bank they will be self contained. Also, I am gonna
; set up a kind of 'DOS int 21h' type service, thru a CALL, so we can provide
; some basic OS services (write text, draw window, cls, etc). Then we should
; have ourselves a gameboy operating system !!!!
;*****************************************************************************
