/************************************************/
/*												*/
/*	definition file for CARTIO hardware V1.1	*/
/*												*/
/************************************************/

#include "StdAfx.h"
#include "CartIO.h"

short _stdcall Inp32(short PortAddress);
void _stdcall Out32(short PortAddress, short data);

/*
 * Bits read (7..0):
 *      BUSY ~ACKPR PE SLCT ERROR N/A N/A N/A
 * Bits from REGISTER (D7..D4, D3..D0):
 *      SLCT PE BUSY ~ACK
 */

#define D7(val)             (val & 0x10) << 3
#define D6(val)             (val & 0x20) << 1
#define D5(val)             ((val ^ 0x80) & 0x80) >> 2
#define D4(val)             (val & 0x40) >> 2
#define D3(val)             (val & 0x10) >> 1
#define D2(val)             (val & 0x20) >> 3
#define D1(val)             ((val ^ 0x80) & 0x80) >> 6
#define D0(val)             (val & 0x40) >> 6

/* devel 0 functions */
CartIO::CartIO(unsigned int port_address) :
    DATA(port_address),
    CTRL(DATA + 2),
    STATUS(DATA + 1)
{}

void CartIO::OUTPORT(short port, BYTE val) const {
    Out32(port, val);
}

BYTE CartIO::INPORT(short port) const {
    return (BYTE)Inp32(port);
}

/* read data from RAM(1) or ROM(0) */
BYTE CartIO::READ(BYTE Mode) const 
{                           
	BYTE low,high, result;

    OUTPORT(CTRL,  GET_LOW | (Mode*MREQ)); 
    low = INPORT(STATUS);
    OUTPORT(CTRL, GET_HIGH | (Mode*MREQ));
    high = INPORT(STATUS);
    result = ( D7(high)|D6(high)|D5(high)|D4(high)|D3(low)|D2(low)|D1(low)|D0(low) );
    return result;
}       

/* Put data on the bus */
void CartIO::WRITE(BYTE val) const {
    OUTPORT(DATA, val);
}

void CartIO::LOAD(BYTE reg) const {
    /* Load a register: toggle CK (OFF and ON) */
    OUTPORT(CTRL, reg); 
    OUTPORT(CTRL, NOP);
}

/* write low address byte */
void CartIO::ADDRESS_LOW(BYTE val) const 
{
	WRITE(val);
	LOAD(LOW_ADDR);
}

/* write high address byte */
void CartIO::ADDRESS_HIGH(BYTE val) const
{
	WRITE(val);
	LOAD(HIGH_ADDR);
}

/* write data in Flash IC */
void CartIO::WRITE_FLASH(BYTE val) const 
{
	WRITE(val);
	LOAD(DATA_REG);

    WRITE(0x00); 
    OUTPORT(CTRL, SET_DATA); 
	WRITE(0x02); // toggle pin 31 only - FWR\
    //WRITE(0x03); // toggle BOTH in 31 and the normal write
	WRITE(0x00); 
	OUTPORT(CTRL, NOP);
}

/* write data in bank-switch IC */
void CartIO::WRITE_BKSW(BYTE val, unsigned int address) const 
{
	ADDRESS_HIGH(address >> 8); 
	ADDRESS_LOW(address & 0x00FF); 

	WRITE(val); 
	LOAD(DATA_REG);

	WRITE(0x00); 
	OUTPORT(CTRL, SET_DATA);
	WRITE(0x01); 
	WRITE(0x00); 
	OUTPORT(CTRL, NOP);
}
/* disable LED */
void CartIO::LED_OFF() const {
    WRITE(0x00); 
    OUTPORT(CTRL, SET_DATA);
}

void CartIO::SET_PORT_ADDRESS(unsigned int port_address){
    DATA = port_address;
    CTRL = DATA + 2;
    STATUS = DATA + 1;
};

/* testprogram for CARTIO */
#if 0
void test()
{
    printf("\nD0 - D7 data lines check !\n");
    printf("Check pins <22 up to 29> of GB connector\n");
    WRITE(0x00);
    LOAD(LOW_ADDR);
    LOAD(HIGH_ADDR);
    LOAD(DATA_REG);
    OUTPORT(CTRL, SET_DATA);
    printf("They should all be low. Press <RETURN>\n");
    getch();

    WRITE(0xFF);
    LOAD(DATA_REG);
    OUTPORT(CTRL, SET_DATA);
    printf("Now, they should all be high. Press <RETURN>\n");
    getch();

    printf("\n\n\nA0 - A7 address lines check !\n");
    printf("Check pins <6 up to 13> of GB connector\n");
    WRITE(0x00);
    LOAD(LOW_ADDR);
    printf("They should all be low. Press <RETURN>\n");
    getch();

    WRITE(0xFF);
    LOAD(LOW_ADDR);
    printf("Now, they should all be high. Press <RETURN>\n");
    getch();

    printf("\n\n\nA8 - A15 address lines check !\n");
    printf("Check pins <14 up to 21> of GB connector\n");
    WRITE(0x00);
    LOAD(HIGH_ADDR);
    printf("They should all be low. Press <RETURN>\n");
    getch();

    WRITE(0xFF);
    LOAD(HIGH_ADDR);
    printf("Now, they should all be high. Press <RETURN>\n");
    getch();

    printf("\n\n\nCS line check !\n");
    printf("check pin <5> of GB connector\n");
    OUTPORT(CTRL, MREQ);
    printf("Should be low. Press <RETURN>\n");
    getch();

    OUTPORT(CTRL, NOP);
    printf("Should be high. Press <RETURN>\n");
    getch();

    printf("\n\n\nRD line check !\n");
    printf("check pin <4> of GB connector\n");
    OUTPORT(CTRL, NOP);
    printf("Should be low. Press <RETURN>\n");
    getch();

    OUTPORT(CTRL, SET_DATA);
    printf("Should be high. Press <RETURN>\n");
    getch();

    printf("\n\n\nWR line check !\n");
    printf("check pin <3> of GB connector\n");
    WRITE(0x01);
    LOAD(DATA_REG);
    OUTPORT(CTRL, SET_DATA);
    printf("Should be low. Press <RETURN>\n");
    getch();

    OUTPORT(CTRL, NOP);
    printf("Should be high. Press <RETURN>\n");
    getch();

    printf("\n\n\nAudio IN line check !\n");
    printf("check pin <31> of GB connector\n");
    WRITE(0x02);
    LOAD(DATA_REG);
    OUTPORT(CTRL, SET_DATA);
    printf("Should be low. Press <RETURN>\n");
    getch();

    OUTPORT(CTRL, NOP);
    printf("Should be high. Press <RETURN>\n");
    getch();

    LED_OFF;
    printf("\n\n\nTest complete !\n\n");
}
#endif
