/*********************************************/
/* GameBoy cartridge reader and writer       */
/*									       	 */
/* original Read program    				 */
/* last edit 06-Nov-95 by Pascal Felber		 */
/*											 */
/* original EEPROM / Flash EPROM Programmer  */
/* last edit 15-Mar-97 by Jeff Frohwein 	 */
/*											 */
/* modified common ReadPlus version for      */
/* old (TTL Version with few modifications)  */
/* new (Altera EPLD EPM7064LC84) CARTIO HW   */
/* and new IO-56 hardware with minor change	 */
/* last edit 29-Sept-97 by Reiner Ziegler	 */
/*											 */
/* V2.51 change LED_OFF calls				 */
/* V2.5  change internal structure for       */
/*       new hardware support				 */
/*       common.h, io56.h, io48.h  			 */
/*       cartio.h, epld.h, newhw.h           */
/* V2.41 fix - W bug						 */
/* V2.4  add overwrite command for RAM		 */
/* V2.3  make RAM visible for Dump command 	 */
/*       -d 160 = base 0xA000                */
/*       some minor changes for IO-56 hw     */
/* V2.2  add support for IO-56 Hardware      */
/*       skip 10ms routine for programming   */ 
/*       add readout for 64in1 module        */
/*       speed up programming routine        */
/*       (input from Jeff Frohwein)			 */
/* V2.1  fix bug with filesize calculation,  */
/*       add default file extensions.		 */
/* V2.0  first common version, works...      */
/*       change erase routine, commands, ... */            
/*											 */
/* Compiled with Microsoft C++ 1.5 			 */
/*********************************************/

#include "cartio.h"   			/* defines for CARTIO hardware */

void usage(void)
{
	printf("\nReadPlus Version 2.51 from Reiner Ziegler\n");
#ifndef Smallprogram
	printf("Usage: readplus [-l int] [-w int] [-m int] [-n int]\n");
	printf("                [-t |-d int |-a |-p file |-s file |-c file |-v file\n");
	printf("				|-o int |-b file |-r file ]\n");
#else  
 	printf("Usage: readplus [-l int][-w int][-d int|-a|-p file|-s file|-c file|-v file\n");
 	printf("                |-o int|-b file|-r file]\n\n");
#endif	
	usage_hardware();
	printf("\t-w\tSpecify the time to wait between accesses to the cartridge\n");
#ifndef Smallprogram	
	printf("\t-t\tTest the cartridge reader\n");
#endif	
	printf("\t-d\tDump 256 bytes of chip to screen (default = 1)\n");
	printf("\t-a\tAnalyze the cartridge header\n");
#ifndef Smallprogram	
	printf("\n\t-m\tMode of programming (0=AMD Am29Fxxx,1=Data Valid)\n");
	printf("\t\t(default is 0 = AMD Am29Fxxx)\n");
	printf("\t-n\tNumber of bits (0=16K,1=32K,2=64K,3=128K,4=256K,5=512K,6=1M,\n");
	printf("\t\t7=2M,8=4M,9=8M,10=16M) (default is 8 = 4Mbits)");
#endif
	printf("\n\t-p\tProgram cartridge from file\n");
	printf("\t-s\tSave the cartridge into file\n");
  	printf("\t-c\tCombined module 64in1 save into file\n"); 
	printf("\t-v\tVerify cartridge matches file\n\n");
	printf("\t-o\tOverwrite SRAM with byte (default = 0)\n");
	printf("\t-b\tBackup the SRAM into file\n");
	printf("\t-r\tRestore the SRAM from file\n");
}

void read_data(FILE *fp, int type, int sizekB)
{
	int bank, page, byte, nbbank;
	unsigned char val;
    unsigned chk;

	/* One bank is 16k bytes */
	nbbank = sizekB / 16;
    chk=0;

	for(bank = 0; bank < nbbank; bank++) {
		printf("Reading bank %d\n", bank);
		if(bank) { WRITE_BKSW(bank,0x2100); }
		
		for(page = (bank ? 0x40 : 0); page < (bank ? 0x80 : 0x40); page++) {
			printf(".");
			ADDRESS_HIGH(page);

			for(byte = 0; byte <= 0xFF; byte++) {
				ADDRESS_LOW(byte);
				val = READ(ROM);
                fputc(val, fp);
                chk += val;
			}
		}
		printf("\n");
	}
	chk -= checksum & 0xFF;
	chk -= (checksum >> 8);
	if(chk == checksum)
		printf("Checksum OK\n");
	else
		printf("Warning: checksum is wrong (waiting for %uX, got %uX)\n", checksum, chk);
}

void read_sram(FILE *fp, int type, int sizekB)
{
	int bank, page, byte, nbbank, banksize;

	if(type == MBC1) {
		/* One bank is 8k bytes */
		nbbank = sizekB / 8;
		if(nbbank == 0) {
			nbbank = 1;
			banksize = sizekB << 2; }
		else
			banksize = 0x20; } 
	else if(type == MBC2) {
			/* SRAM is 512 * 4 bits */
			nbbank = 1;
			banksize = 0x02; } 
		 else {
			printf("No RAM with battery\n");
			return; }		

	/* Initialize the bank-switch IC */
	WRITE_BKSW(0x0A,0x0000);

	for(bank = 0; bank < nbbank; bank++) {
		if(type == MBC1) {
			printf("Reading bank %d\n", bank);
			WRITE_BKSW(bank,0x4000);
		}
		for(page = 0xA0; page < 0xA0 + banksize; page++) {
			printf(".");
			ADDRESS_HIGH(page);
		
			for(byte = 0; byte <= 0xFF; byte++) {
				ADDRESS_LOW(byte);
				fputc(READ(RAM), fp);
			}
		}
        /* disable bank-switch IC */
		WRITE_BKSW(0x00,0x0000);
        printf("\n");

	}
}

void write_sram(FILE *fp, int type, int sizekB)
{
	int bank, page, byte, nbbank, banksize;

	if(type == MBC1) {
		/* One bank is 8k bytes */
		nbbank = sizekB / 8;
		if(nbbank == 0) {
			nbbank = 1;
			banksize = sizekB << 2; }
		else
			banksize = 0x20; } 
	else if(type == MBC2) {
			/* SRAM is 512 * 4 bits */
			nbbank = 1;
			banksize = 0x02; } 
		 else {
			printf("No RAM with battery\n");
			return; }		

	/* Initialize the bank-switch IC */
	WRITE_BKSW(0x0A,0x0000);

	for(bank = 0; bank < nbbank; bank++) {
		if(type == MBC1) {
			printf("Writing bank %d\n", bank);
			WRITE_BKSW(bank,0x4000);
		}
		for(page = 0xA0; page < 0xA0 + banksize; page++) {
			printf(".");
			ADDRESS_HIGH(page);

			for(byte = 0; byte <= 0xFF; byte++) {
				if(feof(fp)) {
					printf("Unexpected EOF\n");
					exit(1);
				}
				ADDRESS_LOW(byte);
				WRITE_MEM(fgetc(fp));
			}
		}
        /* disable bank-switch IC */
		WRITE_BKSW(0x00,0x0000);
		printf("\n");
	}
}
          
void clear_sram(int type, int sizekB, BYTE val)
{
	int bank, page, byte, nbbank, banksize;

	if(type == MBC1) {
		/* One bank is 8k bytes */
		nbbank = sizekB / 8;
		if(nbbank == 0) {
			nbbank = 1;
			banksize = sizekB << 2; }
		else
			banksize = 0x20; } 
	else if(type == MBC2) {
			/* SRAM is 512 * 4 bits */
			nbbank = 1;
			banksize = 0x02; } 
		 else {
			printf("No RAM with battery\n");
			return; }		

	/* Initialize the bank-switch IC */
	WRITE_BKSW(0x0A,0x0000);

	for(bank = 0; bank < nbbank; bank++) {
		if(type == MBC1) {
			printf("Writing bank %d\n", bank);
			WRITE_BKSW(bank,0x4000);
		}
		for(page = 0xA0; page < 0xA0 + banksize; page++) {
			printf(".");
			ADDRESS_HIGH(page);

			for(byte = 0; byte <= 0xFF; byte++) {
				ADDRESS_LOW(byte);
				WRITE_MEM(val);
			}
		}
        /* disable bank-switch IC */
		WRITE_BKSW(0x00,0x0000);
		printf("\n");
	}
}
          
void read_header(unsigned char *h)
{
	int byte, index = 0;

	ADDRESS_HIGH(0x01);

	for(byte = 0; byte < 0x50; byte++) {
		ADDRESS_LOW(byte);
		h[index++] = READ(ROM);
	}
	printf("Game name     : %s\n", &h[0x34]);

	if(h[0x46] == 0x03)
		printf("              : Super GameBoy functions included\n");

	if(h[0x4A] == 0x01)
		printf("              : Non-Japanese Game\n"); 

	if(h[0x4A] == 0x00)
		printf("              : Japanese Game\n"); 

	if(h[0x47] == 0xFF) {
		printf("Cartridge type: %d(%s)\n", h[0x47], "ROM+HuC1+RAM+BATTERY");
	    h[0x47] == 0x03; 
    }
	else {    
		printf("Cartridge type: %d(%s)\n", h[0x47], type[h[0x47]]); }
		
	printf("ROM size      : %d(%d kB)\n", h[0x48], rom[h[0x48]]);

	if(sram[h[0x47]] == MBC2)
		printf("RAM size      : 512*4 Bit\n");
	else	
		printf("RAM size      : %d(%d kB)\n", h[0x49], ram[h[0x49]]);
	checksum = ((unsigned)h[0x4E] << 8) + h[0x4F];
}

/* Return actual chip size in bits */

LONG ChipSize (BYTE size)
   {
   BYTE i;
   LONG Length = 16384;

   for(i=0; i<size; i++)
      Length *= 2;

   return(Length);
   }

/* Print chip size */

void PrintChipSize (void)
   {
   LONG i = ChipSize(Size);
   printf("Chip size = %lu bits (%lu bytes)\n", i, i/8);
   }

/* Set chip address */

void SetAddr (LONG Addr)
{                     
	int bank, AdrReg;
 
   	Address = Addr;
                          
    bank = (int)((Address & 0x7ffff) >> 14);
 	WRITE_BKSW(bank,0x2100);
          
    AdrReg = (int)(Address & 0x3fff);      
   	if (Address > 0x3fff) AdrReg += 16384;
   	
	ADDRESS_LOW(AdrReg & 0x00ff);   
	ADDRESS_HIGH(AdrReg >> 8);
}

/* Increment chip address */

void IncAddr(void)
{
   	Address++;
   	SetAddr(Address);
}

/* Read a byte from chip */

BYTE ReadByte(void)
{
   	BYTE i;

	i = READ(ROM);
  	return(i);
}

/* Write a byte to chip */

void WriteByte(BYTE val)
{
   WRITE_FLASH(val); 
}

/* Wait for program byte completion */

BYTE ProgramDone (BYTE val)
   {
   BYTE Check;
   BYTE Pass = 0;

   switch (Algorithm)
      {
      case AMD29FXXX:
         Check = ReadByte();

         /* Loop until chip done programming */
         while ( (Check != val) &&
                 ((Check & 0x20)==0) )
            Check = ReadByte();

         if (Check == val)
            Pass = 1;
         else
            {
            Check = ReadByte();
            if (Check == val)
               Pass = 1;
            }
         break;
      case DATA_VALID:
         /* Loop until chip done programming */

         while (ReadByte() != val)
            {}

         Pass = 1;
         break;
     }
   return (Pass);
   }

/* Program a byte on chip */

BYTE ProgramByte(LONG Addr, BYTE val)
   {
   BYTE Pass = 0;

   if (Algorithm == AMD29FXXX)
      {
	  WRITE_BKSW(0x01,0x2100); 
      ADDRESS_LOW(0x55); ADDRESS_HIGH(0x55); WRITE_FLASH(0xAA);
      ADDRESS_LOW(0xAA); ADDRESS_HIGH(0x2A); WRITE_FLASH(0x55);
      ADDRESS_LOW(0x55); ADDRESS_HIGH(0x55); WRITE_FLASH(0xA0);
      }

   SetAddr(Addr);    WriteByte(val);

   /* Wait until chip done programming */
   Pass = ProgramDone(val);

   if (Pass == 0)
      printf("Write failed at Address %lx of cartridge.\n", Address);

   return (Pass);
   }


void dump(BYTE BaseAdr)
   {                            
   int i;        
   BYTE First = 1;
   BYTE val,ramrom;        
   BYTE Display[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

   if((BaseAdr >= 0xA0) & (BaseAdr < 0xC0)) 
   		{ramrom=1; WRITE_BKSW(0x0A,0x0000); WRITE_BKSW(0x00,0x4000); }
   else {ramrom=0; WRITE_BKSW(0x01,0x2100);	}
 
   ADDRESS_HIGH(BaseAdr);	

   for(i=0; i<256; i++)
      {
      if (First == 1)
         {
         if (i < 16) printf("0");
         printf("%hx - ",i);
         First = 0;
         }
	  ADDRESS_LOW(i);
      val =  READ(ramrom);
   
      if ((val > 31) & (val < 127))
       Display[i & 0xf] = val;
      else
       Display[i & 0xf] = 46;
        
      if (val < 16)
         printf("0");
      printf("%hx ",val);
      if ((i & 0xf)==0xf)
         {
         First = 1;  
         printf("   %3s",(&Display[0]));
         printf("\n");
         }
      }
   if(ramrom == 1) { WRITE_BKSW(0x00,0x0000); }
   }

/* Compare chip to a file */

void VerifyChip(FILE * fp)
   {
   LONG i = 0;
   BYTE val;
   BYTE CompareFail = 0;
   int c;

   PrintChipSize();
   printf("Verify GameBoy cartridge...\n");
  
   SetAddr(0);

   while ( (!feof(fp)) && (i < (ChipSize(Size)/8)) )
      {
      c = fgetc(fp);
      if (c != EOF)
         {
         val = ReadByte();
         IncAddr();
         if (c != val)
            {
            printf("Address %lx - Cartridge %hx : File %hx\n", i, val, c);
            CompareFail = 1;
            }
         i++;
         }
      }

      if (CompareFail == 0)
         printf("%lu bytes compared ok.\n", i);
   }

/* Erase AMD chips */

BYTE EraseAm29Fxxx(void)
   {
   BYTE Pass = 0;
   BYTE Val  = 0;

   ADDRESS_LOW(0x55); ADDRESS_HIGH(0x55); WRITE_FLASH(0xAA);
   ADDRESS_LOW(0xAA); ADDRESS_HIGH(0x2A); WRITE_FLASH(0x55);
   ADDRESS_LOW(0x55); ADDRESS_HIGH(0x55); WRITE_FLASH(0x80);
   ADDRESS_LOW(0x55); ADDRESS_HIGH(0x55); WRITE_FLASH(0xAA);
   ADDRESS_LOW(0xAA); ADDRESS_HIGH(0x2A); WRITE_FLASH(0x55);
   ADDRESS_LOW(0x55); ADDRESS_HIGH(0x55); WRITE_FLASH(0x10);
   ADDRESS_LOW(0x00); ADDRESS_HIGH(0x00);

   printf("Erasing GameBoy cartridge...");
   
   Val = ReadByte();
   /* Wait for erase to complete */
   while (((Val & 0x80)==0) && ((Val & 0x20)==0))
      {
      Val = ReadByte();
   	  }   
   if ((Val & 0xA0)==0xA0) Pass=1;
 
   if (Pass == 0)
      printf("Cartridge erase failed!\n");
   else
      printf("ok\n");    

   return (Pass);
   }

void ErrorExit(void)
   {
   LED_OFF; 	
   exit(1);
   }

/* Begin chip programming */

void ProgramChip(FILE * fp)
   {
   BYTE b;
   BYTE Pass = 1;
   int c;
   LONG Addr = 0;
  
   PrintChipSize();
  
   if (Algorithm == AMD29FXXX)
      {
      if (!EraseAm29Fxxx())   /* Chip erase */
         ErrorExit();
      }

   printf("Programming cartridge - |");
   while ( (!feof(fp)) &&
           (Addr < (ChipSize(Size)/8)) &&
           (Pass == 1) )
      {
      c = fgetc(fp);
      if (c != EOF)
         {
         b = c;
         Pass = ProgramByte(Addr, b);
         Addr++;
         switch (Addr & 0x3ff)
            {
            case 0:
               printf("\b/");
               break;
            case 0x100:
               printf("\b-");
               break;
            case 0x200:
               printf("\b\\");
               break;
            case 0x300:
               printf("\b|");
               break;
            }
         }
      }
   printf("\n");

   if (Pass == 0)
      {
      printf("Program error. Cartridge may be bad!\n");
      ErrorExit();
      }
   else
      printf("%lu bytes programmed.\n", Addr);
   }

void multimodul(FILE * fp)
{
	int bank, page, byte, nbbank;
	unsigned char val;
    BYTE i, Block;

    for (i=0;i<8;i++) {
		if (i>3) { Block = ( i & 0x03 ) + 0x08; } /* select 2.ROM */
		else     { Block = i; }
		
		WRITE_BKSW(Block,0x2081);
		printf("Block %d\n",i);
		    
		/* One bank is 16k bytes */
		nbbank = 16;
    
		for(bank = 0; bank < nbbank; bank++) {
			printf("Reading bank %d\n", bank);
			WRITE_BKSW(bank,0x2080);
		
			for(page = (bank ? 0x40 : 0); page < (bank ? 0x80 : 0x40); page++) {
				printf(".");
				ADDRESS_HIGH(page);

				for(byte = 0; byte <= 0xFF; byte++) {
					ADDRESS_LOW(byte);
    				val = READ(ROM);
                	fputc(val, fp);
				}
			}
  			printf("\n");
    	}
    }
}

void main(int argc, char **argv)
{
	int arg, fh;
	BYTE Base,Overwrite;
	FILE *fp;
 	char fname[20];
	unsigned char header[0x50];
	double Fsize;

	printf("\n");
	if(argc < 2) {
		usage();
		exit(1);
	}

    Algorithm = AMD29FXXX;  /* default to Am29Fxxx programming algorithm */
    Size = 8;               /* default to 4mbit chip */
 
    wait_delay = 10;
	init_port(1);

	for(arg = 1; arg < argc; arg++) {
		if(argv[arg][0] != '-') {
			usage();
			exit(1);
		}
		switch(toupper(argv[arg][1])) {
            case 'M':
                    Algorithm = atoi(argv[++arg]);
                    if (Algorithm > 1) {
                    	printf("Programming algorithm selection invalid!\n");
                       	exit(1);
                    }
                    break;
			case 'C':     
                    /* add extension .GB if none present */
         			strcpy(fname, argv[++arg]);
         			if (strchr(fname, '.') == NULL)
            		strcat(fname, ".gb");

					if((fp = fopen(fname, "wb")) == NULL) {
						printf("Error opening file %s\n", fname);
						exit(1);
					}
					multimodul(fp);
					fclose(fp);
					break;
            case 'D':
					if (argv[++arg] == NULL) Base = 1;
					else Base = (BYTE)(atoi(argv[arg]));
					printf("Base address : %hx\n",Base*256);
            		dump(Base);
                    break;
            case 'P':
                   /* add extension .GB if none present */
         			strcpy(fname, argv[++arg]);
         			if (strchr(fname, '.') == NULL)
            		strcat(fname, ".gb");
#ifdef Smallprogram	
               		if ((fh = _open( fname, _O_RDONLY))  == -1 ) { 
                       	printf("Error opening file %s\n", fname);
                       	exit(1);
                    }
   				    if (_filelength( fh ) <= 2048L) {
   				       Size = 0; }
   				    else {      
   					   Fsize = (log(_filelength( fh )/2048)/log(2));   
   					   printf("File size = %.0f\n",Fsize); 
   					   Size = (unsigned char)Fsize;
   					   _close( fh ); }
#endif
				  	if ((fp = fopen(fname, "rb")) == NULL) {
                       	printf("Error opening file %s\n", fname);
                       	exit(1);
                    }
                    ProgramChip(fp);
                    fclose(fp);
                    break;
#ifndef Smallprogram	
            case 'N':
                    Size = atoi(argv[++arg]);
                    if (Size>10) {
                      	printf("Chip size selection invalid!\n");
                       	exit(1);
                    }
                    break;
#endif
            case 'V':
	           		/* add extension .GB if none present */
         			strcpy(fname, argv[++arg]);
         			if (strchr(fname, '.') == NULL)
            		strcat(fname, ".gb");
#ifdef Smallprogram	
             		if ((fh = _open( fname, _O_RDONLY))  == -1 ) { 
                       	printf("Error opening file %s\n", fname);
                       	exit(1);
                    }
   				    if (_filelength( fh ) <= 2048L) {
   				       Size = 0; }
   				    else {      
   				       Fsize = (log(_filelength( fh )/2048)/log(2));
   					   printf("File size = %.0f\n",Fsize); 
   					   Size = (unsigned char)Fsize;
   					   _close( fh ); }
#endif
                    if ((fp = fopen(fname, "rb")) == NULL) {
                   		printf("Error opening file %s\n", fname);
                       	exit(1);
                    }
                    VerifyChip(fp);
                    fclose(fp);
                    break;
			case 'L':
					init_port(atoi(argv[++arg]));
			    	break;
			case 'W':
					wait_delay = atoi(argv[++arg]);
					break;
			case 'T':
					test();
					break;
			case 'A':
					read_header(header);
					break;
			case 'S':
                    /* add extension .GB if none present */
         			strcpy(fname, argv[++arg]);
         			if (strchr(fname, '.') == NULL)
            		strcat(fname, ".gb");

					if((fp = fopen(fname, "wb")) == NULL) {
						printf("Error opening file %s\n", fname);
						exit(1);
					}
					read_header(header);
					read_data(fp, mbc[header[0x47]], rom[header[0x48]]);
					fclose(fp);
					break;
			case 'O': 
					if (argv[++arg] == NULL) Overwrite = 0x00;
					else Overwrite = (BYTE)(atoi(argv[arg]));
			        read_header(header);
					clear_sram(sram[header[0x47]], ram[header[0x49]], Overwrite);
					break;
			case 'B':
                    /* add extension .SAV if none present */
         			strcpy(fname, argv[++arg]);
         			if (strchr(fname, '.') == NULL)
            		strcat(fname, ".sav");

					if((fp = fopen(fname, "wb")) == NULL) {
            			printf("Error opening file %s\n", fname);
						exit(1);
					}
					read_header(header);
					read_sram(fp, sram[header[0x47]], ram[header[0x49]]);
					fclose(fp);
					break;
			case 'R':   
                    /* add extension .SAV if none present */
         			strcpy(fname, argv[++arg]);
         			if (strchr(fname, '.') == NULL)
            		strcat(fname, ".sav");
       				
       				if((fp = fopen(fname, "rb")) == NULL) {
						printf("Error opening file %s\n", fname);
						exit(1);
					}        
   					read_header(header);
					write_sram(fp, sram[header[0x47]], ram[header[0x49]]);
					fclose(fp); 
					break;
			default:
					usage();
                    LED_OFF;
					exit(1);
		}
	}
    LED_OFF;
	exit(0);
}


