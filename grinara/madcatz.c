#include <stdio.h>
#include <pc.h>

#define lptbase 0x378 
#define DATA	lptbase+0
#define STATUS	lptbase+1
#define CONTROL lptbase+2

unsigned char buffer[0x1760];


int main (int argc, char *argv[])
{
	int inbyte, count = 0;
	

	fprintf(stderr, "Resetting device.\n");
	fflush(stderr);
	do {	
		outportb(CONTROL, 0x24);
		while ((inportb(STATUS) & 0x20) == 0) { }
		inbyte = inportw(DATA) & 0xF;
		printf("%x\n",inbyte);
	} while (inbyte != 4);	
	outportb(CONTROL, 0x22);
	while (inportb(STATUS) & 0x20 != 0) { }	
	outportb(CONTROL, 0x26);
	fprintf(stderr, "Receiving data:\n");
	fflush(stderr);
	do {	
		outportb(CONTROL, 0x26);
		while ((inportb(STATUS) & 0x20) == 0) { }
		inbyte = inportw(DATA) & 0xF;
		outportb(CONTROL, 0x22);
		while ((inportb(STATUS) & 0x20) != 0) { }	
		outportb(CONTROL, 0x26);
		while ((inportb(STATUS) & 0x20) == 0) { }
		inbyte |= (inportw(DATA) & 0xF) << 4;
		printf("%2x  ",inbyte);
		outportb(CONTROL, 0x22);
		while ((inportb(STATUS) & 0x20) != 0) { }	
		buffer[count++] = inbyte;
#if 0
	} while (count < 40);
#endif
	} while (1);
	
	printf("\n");
	for (count = 0; count < 40; count++) printf("%2x  ", buffer[count]);
	printf("\n");


}
