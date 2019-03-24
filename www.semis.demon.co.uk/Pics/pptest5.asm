;Pic PP87 sample code for the PIC12C508
;S.G.Willis 17/1/97

        TITLE   "PP87TEST Ver 1:00"
        LIST    P=12C508,F=INHX8M

INDADDR EQU     00              ;Control registers.......
RTCC    EQU     01
STATUS  EQU     03
FSR     EQU     04
OSCCAL  EQU     05
PORTG   EQU     06              ;Six mode indicator LEDS

TEMP1   EQU     10              ;RAM addrs...

        ORG     0x000                   ;Reset vector
        GOTO    MAIN                   
        ORG     0x005                   ;Start of ROM 

;------------------------------- Main -------------------------------------
;
;Main programme
;
MAIN    DECF    TEMP1           ;Loop forever
        GOTO    MAIN
        
        END

