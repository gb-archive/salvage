#pragma once

//#include <fcntl.h>
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif
#include <windows.h>

class CartIO {
public:
    enum {
        /*
        * Bits for MUX (C B A):
        *      ~AUTOFDXT ~SCLINT ~STROBE
        * Bits in CTRL register (7..0):
        *      N/A N/A N/A IRQEnable ~SCLINT INIT ~AUTOFDXT ~STROBE
        */
        MASK            = 0x00,
        nSTROBE         = 0x01,
        nAUTOFDXT       = 0x02,
        INIT            = 0x04,
        nSCLINT         = 0x08,

        /* CTRL MUX */
        MREQ            = ( MASK | INIT ),                          /* 0100 */
        GET_LOW         = ( MASK | nAUTOFDXT | nSCLINT | nSTROBE ), /* 1011 */
        GET_HIGH        = ( MASK | nAUTOFDXT | nSCLINT ),           /* 1010 */
        SET_DATA        = ( MASK | nAUTOFDXT           | nSTROBE ), /* 0011 */
        DATA_REG        = ( MASK | nAUTOFDXT ),                     /* 0010 */
        LOW_ADDR        = ( MASK |             nSCLINT | nSTROBE ), /* 1001 */
        HIGH_ADDR       = ( MASK |             nSCLINT ),           /* 1000 */
        NOT_USED        = ( MASK |                       nSTROBE ), /* 0001 */
        NOP             = MASK
    };

    /* cartridge defines */
    enum {
        NO_SRAM = 0,
        NO_MBC  = 0,           
        ROM		= 0,
        RAM     = 1,
        MBC1    = 1,
        MBC2    = 2
    } TYPE;
    unsigned DATA;
    unsigned CTRL;
    unsigned STATUS;

    void OUTPORT(short port, BYTE val) const;
    BYTE INPORT(short port) const;

    BYTE READ(BYTE Mode) const;
    void WRITE_BKSW(BYTE data, unsigned int address) const;
    void WRITE_FLASH(BYTE val) const;
    void ADDRESS_LOW(BYTE val) const;
    void ADDRESS_HIGH(BYTE val) const;
    void LOAD(BYTE reg) const;
    /* Put data on the bus */
    void WRITE(BYTE val) const;
    /* disable LED */
    void LED_OFF() const;
    void SET_PORT_ADDRESS(unsigned int port_address);
    CartIO(unsigned int port_address);
};
