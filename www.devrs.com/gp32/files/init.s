      ; GP32 Startup Boot Code (Init.o)
      ; REing by cba_

  AREA Init, CODE, READONLY

        IMPORT |Image$$RO$$Base|
        IMPORT |Image$$RO$$Limit|
        IMPORT |Image$$RW$$Base|
        IMPORT |Image$$RW$$Limit|
        IMPORT |Image$$ZI$$Base|
        IMPORT |Image$$ZI$$Limit|

        IMPORT asm_user_entry
        IMPORT GpSurfaceSet
        IMPORT GpSurfaceFlip
        IMPORT HEAPSTART
        IMPORT HEAPEND
        IMPORT _reg_io_key_a
        IMPORT _reg_io_key_b
        IMPORT _timepassed
        IMPORT Main


	; HEADER

        ENTRY
	B		_GpInit

        DCD   |Image$$RO$$Base|   ; Start of Read Only section
        DCD   |Image$$RO$$Limit|  ; End "
        DCD   |Image$$RW$$Base|   ; Start of Read/Write section
        DCD   |Image$$RW$$Limit|  ; End "
        DCD   |Image$$ZI$$Base|   ; Base of Zero Initialized section
        DCD   |Image$$ZI$$Limit|  ; End "

        DCD             0x44450011
        DCD             0x44450011

        DCD             0x01234567
        DCD             0x12345678
        DCD             0x23456789
	DCD		0x34567890
	DCD		0x45678901
	DCD		0x56789012
	DCD		0x23456789
	DCD		0x34567890
	DCD		0x45678901
	DCD		0x56789012
	DCD		0x23456789
	DCD		0x34567890
	DCD		0x45678901
	DCD		0x56789012

_GpInit
        mrs r0,CPSR
        orr r0,r0,#0xc0
        msr CPSR_fsxc,r0

; Call function in user_init.s
        bl  asm_user_entry

; Get pointer to GpSurfaceSet routine
        mov r0,#0
        swi 0xb
        ldr r1,=GpSurfaceSet
        ldr r2,=GpSurfaceFlip
        str r0,[r1]
        str r0,[r2]

; Get time passed
        mov r0,#6
        swi 0xb
        ldr r1,=_timepassed
        str r0,[r1]

; Get button stuff
        mov r0,#0
        swi 0x10
        ldr r2,=_reg_io_key_a
        ldr r3,=_reg_io_key_b
        str r0,[r2]
        str r1,[r3]

; Set heap start location
        ldr r0,=|Image$$ZI$$Limit|
        ldr r1,=HEAPSTART
        str r0,[r1]

; Set heap end location
        mov r0,#5
        swi 0xb
        ldr r1,=HEAPEND
        sub r0,r0,#255
        bic r0,r0,#3
        str r0,[r1]

; Set App Argument(?)
        swi 0x15

        mov r10,r0
        mov r11,r1

        mrs r0,CPSR
        bic r0,r0,#192
        orr r0,r0,#64
        msr CPSR_fsxc,r0

        mov r0,r10
        mov r1,r11

; Jump to Main ()
        ldr r3,=Main
        bx  r3

 	END
