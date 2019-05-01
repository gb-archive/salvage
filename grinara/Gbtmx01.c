/*************************************************************
*               program name: gbt15.c                        *
*  GBT MX version, 0.01	modified by Joshua Neal              *
*  use parallel EPP/SPP port to r/w game boy cartridge       *
*                                                            *
* ai[]=0 w a[7..0]                                           *
* ai[]=1 w a[15..8]                                          *
* ai[]=2 w control d7=rs,d6=spp,d1=xwe_en,d0=cs_en           *
* ai[]=3 r/w data                                            *
*                                                            *
* MBC1                                                       *
*   R/W A000~BFFF   RAM SWITCHING BANK(256Kbit) 4 BANKS OF 8Kbyte   *
*   R 4000~7FFF     ROM SWITCHING BANK(4Mbit) 32 BANKS OF 128Kbit   *
*   W 2000~3FFF     SET ROM BANK (5 BIT)                      *
*   R 0000~3FFF     FIX ROM BANK 0                           *
*   W 4000~5FFF     SET RAM BANK (2 BIT)                     *
*   W 0000~1FFF     SET 0A ENABLE RAM BANK                   *
*                                                            *
* MBC2                                                       *
*   R/W A000~BFFF   512 X 4 BIT RAM                          *
*   R 4000~7FFF     ROM SWITCHING BANK(2Mbit) 16 BANKS OF 128Kbit  *
*   W 2100          SET ROM BANK (4 BIT)                     *
*   R 0000~3FFF     FIX ROM BANK 0                           *
*   W 0000          SET 0A ENABLE RAM BANK                   *
*                                                            *
* MBC5                                                       *
*   R/W A000~BFFF   RAM SWITCHING BANK(1Mbit) 16 BANKS OF 64 Kbit    *
*   R 4000~7FFF     ROM SWITCHING BANK(64Mbit) 512 BANKS OF 128Kbit  *
*   W 3000~3FFF     SET ROM BANK1(BANK Q8)  TOTAL 9 BIT      *
*   W 2000~2FFF     SET ROM BANK0(BANK Q7~Q0)                *
*   R 0000~3FFF     FIX ROM BANK 0                           *
*   W 4000~7FFF     SET RAM BANK (4 BIT)                     *
*   W 0000~1FFF     SET 0A ENABLE RAM BANK                   *
*                                                            *
*************************************************************/
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
//#include <dir.h>
#include <string.h>

#define ai port_b
#define data port_c
#define trans_size 32768
//#define set_ai_write outportb(port_a,5); // ninit=1, nwrite=0
#define set_data_read outportb(port_a,0);  // nastb=1,nib_sel=0,ndstb=1,nwrite=1
#define set_data_write outportb(port_a,1); // nastb=1,nib_sel=0,ndstb=1,nwrite=0
//#define set_data_write outportb(port_a,1);    // ninit=0, nwrite=0
//#define set_data_read outportb(port_a,0);     // ninit=0, nwrite=1
#define set_normal outportb(port_a,4);          // ninit=1, nwrite=1
#define retry_time 3;
unsigned long time_out;
unsigned int port[2];
unsigned char port_no;
unsigned int port_8,port_9,port_a,port_b,port_c;
unsigned int bank,bank_size;

unsigned long maxfilesize;
char *file_name=NULL;
unsigned char cmd,eeprom_type;          // command
FILE *fptr;
union mix_buffer {
   unsigned char buffer[32768];
   unsigned int bufferx[16384];
}mix;
unsigned int i,j,idx,gcrc;
unsigned char temp,mbc1_exp;
unsigned long filesize;

unsigned char header_ok,cart_type,rom_size,ram_size,sram_bank_num;
char port_type=0;       // 0=epp, 1=spp
char epp_spp=0;
char pocket_camera=0;   // 0=not pocket camera sram(1Mbits)
char got_shark = 0;

void out_adr_data_32k(unsigned int adr,unsigned char data);

/**************************************
*               Subroutine            *
**************************************/
void disp_buffer(unsigned int disp_len)
{
   int i,j,x,y;
   for (i=0; i<disp_len ; i++)
      {
      if ((i & 0xf)==0)
	 printf("%04x : ",i&0xfff0);
      if ((i & 0xf)==8)
	 printf("- ");
      printf("%02x ",mix.buffer[i]);
      if ((i & 0xf)==0xf)
	 {
	 printf("-> ");
	 for (j=0; j<16 ; j++)
	    {
	    if (mix.buffer[(i&0xfff0)+j]<0x20 || mix.buffer[(i&0xfff0)+j]>0x80)
	       printf (".");
	    else
	       printf ("%c",mix.buffer[(i&0xfff0)+j]);
	    }
	 printf("\n");
	 }
      }

   y=disp_len&0xf;
   if (y)                               /* not equal 16*?? */
      {
      for (x=y;x<16;x++)
	 {
         if ((x & 0xf)==8)
            printf("- ");
         printf("   ");
         if ((x & 0xf)==0xf)
	    {
            printf("-> ");
            for (j=0; j<y ; j++)
               {
               if (mix.buffer[(i&0xfff0)+j]<0x20 || mix.buffer[(i&0xfff0)+j]>0x80)
                  printf (".");
               else
                  printf ("%c",mix.buffer[(i&0xfff0)+j]);
               }
            printf("\n");
            }
	 }
      }
}

/*void port_set_read(void)
{
   outportb(port_a,0);          // nastb=1,nib_sel=0,ndstb=1,nwrite=1
}
void port_set_write(void)
{
   outportb(port_a,1);          // nastb=1,nib_sel=0,ndstb=1,nwrite=0
}*/

void spp_set_ai(unsigned char _ai)
{
   set_data_write
//   outportb(port_a,1);                // nastb=1,nib_sel=0,ndstb=1,nwrite=0
   outportb(port_8,_ai);        // put ai at data bus
   outportb(port_a,9);          // nastb=0,nib_sel=0,ndstb=1,nwrite=0
   outportb(port_a,1);          // nastb=1,nib_sel=0,ndstb=1,nwrite=0
                                // nastb ~~~~|___|~~~~
}

void spp_write_data(unsigned char _data)
{
//   outportb(port_a,1);                // nastb=1,nib_sel=0,ndstb=1,nwrite=0
   outportb(port_8,_data);      // put data at data bus
   outportb(port_a,3);          // nastb=1,nib_sel=0,ndstb=0,nwrite=0
   outportb(port_a,1);          // nastb=1,nib_sel=0,ndstb=1,nwrite=0
				// ndstb ~~~~|___|~~~~
}

void spp_set_ai_data(unsigned char _ai,unsigned char _data)
{
   spp_set_ai(_ai);
   spp_write_data(_data);
}

char spp_read_data(void)
{
char low_nibble,high_nibble,temp;
   set_data_read
   outportb(port_a,2);          // nastb=1,nib_sel=0,ndstb=0,nwrite=1
   low_nibble=inportb(port_9);
   outportb(port_a,6);          // nastb=1,nib_sel=1,ndstb=0,nwrite=1
   high_nibble=inportb(port_9);
   outportb(port_a,0);          // nastb=1,nib_sel=0,ndstb=1,nwrite=1
				// nibble_sel ___|~~~ and ndstb ~~~~|___|~~~~
   temp=(((high_nibble<<1)&0xf0)|((low_nibble>>3)&0x0f));
//   printf("temp=%x",temp);
   return(temp);
}

void epp_set_ai(unsigned char _ai)
{
   set_data_write
   outportb(ai,_ai);
}

void epp_set_ai_data(unsigned char _ai,unsigned char _data)
{
   epp_set_ai(_ai);
   set_data_write
   outportb(data,_data);
}

void set_ai(unsigned char _ai)
{
   set_data_write
   if (port_type)
        spp_set_ai(_ai);
   else
	epp_set_ai(_ai);
}

void set_ai_data(unsigned char _ai,unsigned char _data)
{
   if (port_type)
        spp_set_ai_data(_ai,_data);     // spp mode
   else
        epp_set_ai_data(_ai,_data);     // epp mode
}

void write_data(unsigned char _data)
{
   if (port_type)
        spp_write_data(_data);          // spp write data
   else
        outportb(data,_data);           // epp write data
}

unsigned char read_data(void)
{
   if (port_type){
	return(spp_read_data());        // spp read data
   }
   else{
        return(inportb(data));          // epp read data
   }
}

void init_port(void)
{
   outportb(port_9,1);          // clear EPP time flag
   set_ai_data(2,0);            // rst=0, wei=0(dis.), rdi=0(dis.)
   set_ai_data(2,0x80);         // rst=1, wei=0(dis.), rdi=0(dis.)
}

void end_port(void)
{
   set_ai_data(2,0);            // rst=0, wei=0(dis.), rdi=0(dis.)
   set_normal                   // ninit=1, nWrite=1
}

unsigned char write_32k_file(void)
{
   if (fwrite((char *)mix.buffer,sizeof(char),trans_size,fptr)!=trans_size)
      {
      fclose(fptr);     /* write data error */
      return(1);
      }
//   printf(".");
   return(0);
}

unsigned char read_8k_file()
{
   if (fread((char *)mix.buffer,sizeof(char),0x2000,fptr)!=0x2000)
      {
      printf("read error!!!\07\n");
      fclose(fptr);     /* read data error */
      return(1);
      }
//   printf(".");
   return(0);
}

unsigned char read_16k_file()
{
   if (fread((char *)mix.buffer,sizeof(char),0x4000,fptr)!=0x4000)
      {
      printf("read error!!!\07\n");
      fclose(fptr);     /* read data error */
      return(1);
      }
//   printf(".");
   return(0);
}

unsigned char read_32k_file()
{
   if (fread((char *)mix.buffer,sizeof(char),trans_size,fptr)!=trans_size)
      {
      printf("read error!!!\07\n");
      fclose(fptr);     /* read data error */
      return(1);
      }
   printf(".");
   return(0);
}

void set_adr(unsigned int adr)  // *****
{
//unsigned char temp;
   set_ai_data(0,(adr & 0xff));         // a[7..0]
   set_ai_data(1,((adr>>8) & 0xff));    // a[15..8]
   set_ai(3);
   set_data_read                // ninit=0, nWrite=1
}

int write_file_32k(void)
{
   if (fwrite((char *)mix.buffer,sizeof(char),trans_size,fptr)!=trans_size)
      {
      fclose(fptr);     /* write data error */
      return(-1);
      }
   printf(".");
   return(0);
}

char write_file_xxk(unsigned int write_size)
{
   if (fwrite((char *)mix.buffer,sizeof(char),write_size,fptr)!=write_size)
      {
      fclose(fptr);     /* write data error */
      return(-1);
      }
   printf(".");
   return(0);
}

void set_bank(unsigned int adr,unsigned char bank)
{
//   printf("adr=%x bank=%x\n",adr,bank);
   set_ai_data(2,0x80); // disable inc
   set_ai_data(0,(adr & 0xff));         // a[7..0]
   set_ai_data(1,((adr>>8) & 0x7f));    // a[15..8]
   set_ai_data(3,bank);         // write bank no
   set_data_read                // ninit=0, nWrite=1
}

void set_rom_bank(unsigned char bank)
{
// cart_type <4 is MCB1, other is MCB2
   if (cart_type < 4)
      set_bank(0x2000,bank);    // for MCB1
   else
      set_bank(0x2100,bank);    // for MCB2
}

void delay_10us()
{
long x;
   for (x=0;x<0x2000;x++);
}

void delay_100us()
{
long x;
   for (x=0;x<0x10000;x++);
}

void delay_20ms()
{
long x;
   for (x=0;x<0xfffff;x++);
}

void out_byte_eeprom(unsigned char d)
{
   set_ai_data(2,0x82);         // wei enable
   set_ai(3);                   // default write mode
//   set_data_read              // ninit=0, nWrite=1
   set_data_write
   write_data(d);               // out data
//   outportb(data,d);          // out data
   set_ai_data(2,0x80);         // wei disable
   set_ai(3);                   // default write mode
}

void out_byte(unsigned char d)
{
   set_ai(3);
//   set_data_read              // ninit=0, nWrite=1
   set_data_write
   write_data(d);               // out data
//   outportb(data,d);          // out data
}

void out_data(unsigned char h,unsigned char m,unsigned char l,unsigned char d)
{
// ai[]=2 w control d7=rs,d1=xwe_en,d0=cs_en
   h=((h<<2)|(m>>6))&0x1f;      // maximum bank is 1f
   if (h)
      m=(m & 0x3f)|0x40;        // >bank 0
   else
      m=m & 0x3f;               // bank 0

   set_adr(0x2000);             // write 2000:h
   set_data_write
   write_data(h);               // set rom bank value
//   outportb(data,h);          // set rom bank value
   set_ai_data(1,m);            // a[15..8]
   set_ai_data(0,l);            // a[7..0]
   out_byte_eeprom(d);                  // write data to eeprom
}

void out_adr2_data(unsigned long adr,unsigned char d)   // address shift 1 bit
{
unsigned char h,m,l;
   set_ai_data(2,0x80);         // disable wr/rd inc.
//j   adr<<=1;                     // adr x 2
   l=adr & 0xff;                // a7~a0
   m=(adr>>8) & 0x3f;           // a13~a8
   h=(adr>>14) & 0xff;          // a21~a13
   if (h)
      m|=0x40;  // >bank 0

//j   set_adr(0x2000);             // write 2000:h
   set_adr(0x3f00);             // write 2000:h
   set_data_write
//j  write_data(0x40);               // set rom bank value
   write_data(0x40+h);               // set rom bank value
   printf("Rom Bank Select: %x:%x:%x\n", 0x40+h,m,l);
//   outportb(data,h);          // set rom bank value
   set_ai_data(1,m);            // a[15..8]
   set_ai_data(0,l);            // a[7..0]
   out_byte_eeprom(d);                  // write data to eeprom
}

void out_adr_data(unsigned long adr,unsigned char d)    // real address
{
unsigned char xh,h,m,l;
   set_ai_data(2,0x80);         // disable wr/rd inc.
   l=adr & 0xff;                // a7~a0
   m=(adr>>8) & 0x3f;           // a13~a8
   h=(adr>>14) & 0xff;          // a21~a13
   xh=(adr>>22) & 0x7;          // max. 256Mbit
   if (h|xh)
      m|=0x40;  // >bank 0

   set_adr(0x3000);             // write 3000:sh
   set_data_write
   write_data(xh);              // set rom bank extend value
   set_adr(0x2000);             // write 2000:h
   set_data_write
   write_data(h);               // set rom bank value
//   outportb(data,h);          // set rom bank value
   set_ai_data(1,m);            // a[15..8]
   set_ai_data(0,l);            // a[7..0]
   out_byte_eeprom(d);                  // write data to eeprom
}

void set_adr_long(unsigned long adr)    // real address
{
unsigned char xh,h,m,l;
   set_ai_data(2,0x80);         // disable wr/rd inc.
   l=adr & 0xff;                // a7~a0
   m=(adr>>8) & 0x3f;           // a13~a8
   h=(adr>>14) & 0xff;          // a21~a13
   xh=(adr>>22) & 0x7;          // max. 256Mbit
   if (h|xh)
      m|=0x40;  // >bank 0

   set_adr(0x3000);             // write 3000:sh
   set_data_write
   write_data(xh);              // set rom bank extend value
   set_adr(0x2000);             // write 2000:h
   set_data_write
   write_data(h);               // set rom bank value
//   outportb(data,h);          // set rom bank value
   set_ai_data(1,m);            // a[15..8]
   set_ai_data(0,l);            // a[7..0]
}

unsigned char read_byte(void)
{
   set_ai(3);                   // default write mode
   set_data_read                // ninit=0, nWrite=1
   return (read_data());
//   return (inportb(data));
}

void enable_protection(void)
{
//   set_bank(0x2000,0);                // set bank 0
   out_data(0,0x55,0x55,0xaa);          /* adr2,adr1,adr0,data 05555:aa */
   out_data(0,0x2a,0xaa,0x55);
   out_data(0,0x55,0x55,0xa0);
}

void disable_protection(void)
{
   out_data(0,0x55,0x55,0xaa);          /* adr2,adr1,adr0,data 05555:aa */
   out_data(0,0x2a,0xaa,0x55);
   out_data(0,0x55,0x55,0x80);
   out_data(0,0x55,0x55,0xaa);
   out_data(0,0x2a,0xaa,0x55);
   out_data(0,0x55,0x55,0x20);
   delay_20ms();
}

int data_polling_data(unsigned char last_data)
{
   unsigned char loop;
   unsigned long timeout=0;
//   delay_10us();                      // call delay
   loop = 1;
   while ((timeout<0x07ffffff) && (loop))
      {
      if (((read_byte() ^ last_data) & 0x80)==0)        // end wait
	 loop = 0;                      // ready to exit the while loop
      timeout++;
      }
//   printf("timeout = %x\n",timeout);
   return(loop);
}

int data_polling(void)
{
   unsigned char loop,predata,currdata;
   unsigned long timeout=0;
//   delay_10us();                      // call delay
   loop = 1;
   predata = read_byte() & 0x40;
   while ((timeout<0x07ffffff) && (loop))
      {
      currdata = read_byte() & 0x40;
      if (predata == currdata)
	 loop = 0;                      // ready to exit the while loop
      predata = currdata;
      timeout++;
      }
//   printf("timeout = %x\n",timeout);
   return(loop);
}

void reset_to_read(void)        // return to read mode
{
//   out_adr2_data(0x5555,0xaa);          // 5555:aa adr2,adr1,adr0,data
//   out_adr2_data(0x2aaa,0x55);          // 2aaa:55
//   out_adr2_data(0x5555,0xf0);          // 5555:f0
   set_bank(0x3F00,0x41);   // Datel MBC Bank
   out_adr_data_32k(0x5555,0xaa);          /* 5555:aa softwave product ID entry */
   out_adr_data_32k(0x2aaa,0x55);          /* 2aaa:55 adr2,adr1,adr0,data */
   out_adr_data_32k(0x5555,0xf0);          /* 5555:90 adr2,adr1,adr0,data */

}

void read_status_reg_cmd(void)
{
   out_adr2_data(0x5555,0xaa);          // 5555:aa adr2,adr1,adr0,data
   out_adr2_data(0x2aaa,0x55);          // 2aaa:55
   out_adr2_data(0x5555,0x70);          // 5555:70
}

char wait_status(void)
{
   unsigned temp;
   temp=read_byte();                    // read first status byte
//      printf("temp=%x ",temp);

   while ((temp & 0xfc)!=0x80){
//      printf("temp=%x ",temp);
      if ((temp & 0x20)==0x20){
	 printf("Fail in erase!!!\07\n");
         return(-1);
      }
      if ((temp & 0x10)==0x10){
         printf("Fail in program!!!\07\n");
	 return(-2);
      }
      temp=read_data();
//      temp=inportb(data);
   }
//   reset_to_read();
   return(0);
}

char mx_erase(void)
{
   out_adr2_data(0x5555,0xaa);          // 5555:aa adr2,adr1,adr0,data
   out_adr2_data(0x2aaa,0x55);          // 2aaa:55
   out_adr2_data(0x5555,0x80);          // 5555:80
   out_adr2_data(0x5555,0xaa);          // 5555:aa
   out_adr2_data(0x2aaa,0x55);          // 2aaa:55
   out_adr2_data(0x5555,0x10);          // 5555:10

   delay_100us();
//   read_status_reg_cmd();             // send read status reg. cmd
   if(wait_status()==0){
      reset_to_read();
      printf("erase ok\n");
      return(0);
   }
   else{
      reset_to_read();
      printf("erase error!!!\07\n");
      return(-1);
   }
}

char win_erase(void)
{
   out_data(0,0x55,0x55,0xaa);          /* adr2,adr1,adr0,data 05555:aa */
   out_data(0,0x2a,0xaa,0x55);
   out_data(0,0x55,0x55,0x80);
   out_data(0,0x55,0x55,0xaa);
   out_data(0,0x2a,0xaa,0x55);
   out_data(0,0x55,0x55,0x10);
   delay_20ms();

   if (data_polling()){
      printf("erase error!!!\n");
      return(-1);
   }
   else{
      printf("erase ok!\n");
      return(0);
   }
}

unsigned char intel_read_status(void)
{
   out_adr_data(0,0x70);                // read status command
   return(read_byte());

}

char intel_check_status(void)
{
   time_out=0x8000;
   while (!(intel_read_status() & 0x80)){
      time_out--;
      if (time_out==0){

	 printf("Intel read status time out\n");
	 printf("status = %x\n",intel_read_status());
	 out_adr_data(0,0x50);  // clear status register
	 return(-1);
      }
   }
   return(0);
}

char intel_block_erase(unsigned long block){
   time_out=0x8000;
   while ((intel_read_status())!=0x80){
      time_out--;
      if (time_out==0){
	 printf("Intel Block erase time out\n");
	 printf("status = %x\n",intel_read_status());
	 return(-1);
      }
   }
   out_adr_data(block,0x20);            // Block erase
   out_adr_data(block,0xd0);            // write confirm
   time_out=0x8000;
   while (!(intel_read_status() & 0x80)){
      time_out--;
      if (time_out==0){
	 printf("Intel Block erase time out at %lx\n",block);
	 printf("status = %x\n",intel_read_status());
	 out_adr_data(block,0x50);      // clear status register
	 printf("status = %x\n",intel_read_status());
	 return(-1);
      }
   }

   if ((intel_read_status())==0x80){
      printf("e");
      return(0);
   }
   else{
      printf("Intel Block erase error at %lx\n",block);
      printf("status = %x\n",intel_read_status());
      out_adr_data(block,0x50); // clear status register
      printf("status = %x\n",intel_read_status());
      out_adr_data(0x0000,0xff);        // read array
      return(-1);
   }
}

char intel_erase(void)
{

unsigned long block;
   for (block=0;block<64;block++){
      if (intel_block_erase(block*0x20000))
	 return(-1);            // something error
   }
   printf("\nErase completed\n");
   return(0);
}

char shark_erase(void)
{
	set_bank(0x3F00,0x41);   // Datel MBC Bank
	out_adr_data_32k(0x5555,0xaa);          /* 5555:aa softwave product ID entry */
	out_adr_data_32k(0x2aaa,0x55);          /* 2aaa:55 adr2,adr1,adr0,data */
	out_adr_data_32k(0x5555,0x80);          /* 5555:90 adr2,adr1,adr0,data */
	out_adr_data_32k(0x5555,0xaa);          /* 5555:aa softwave product ID entry */
	out_adr_data_32k(0x2aaa,0x55);          /* 2aaa:55 adr2,adr1,adr0,data */
	out_adr_data_32k(0x5555,0x10);          /* 5555:90 adr2,adr1,adr0,data */
	delay_20ms();

	if (data_polling()){
		printf("erase error!!!\n");
		return(-1);
	} else {
		printf("erase ok!\n");
		return(0);
	}

}

char erase(void)
{
   if (got_shark) {
	return shark_erase();
   }
   else {
	printf("Cart to program must be Shark MX!\07\n");
	return(-1);
   }

}

char sector_erase(unsigned long sector)
{
unsigned char temp;
   out_adr2_data(0x5555,0xaa);          // 5555:aa adr2,adr1,adr0,data
   out_adr2_data(0x2aaa,0x55);          // 2aaa:55
   out_adr2_data(0x5555,0x80);          // 5555:80
   out_adr2_data(0x5555,0xaa);          // 5555:aa
   out_adr2_data(0x2aaa,0x55);          // 2aaa:55
   out_adr2_data(sector,0x30);          // sector:30

   delay_100us();
//   read_status_reg_cmd();             // send read status reg. cmd
   if(wait_status()==0){
//      reset_to_read();
//      printf("erase ok\n");
      printf("s");
      return(0);
   }
   else{
      reset_to_read();
      printf("sector erase error!!!\07\n");
      return(-1);
   }
}

void mx_id(void)
{
   out_adr2_data(0x5555,0xaa);          /* softwave product ID entry */
   out_adr2_data(0x2aaa,0x55);          /* adr2,adr1,adr0,data */
   out_adr2_data(0x5555,0x90);          /* adr2,adr1,adr0,data */
//   delay_10us();
   set_adr(0);                  /* adr2,adr1,adr0 */
   printf("Manufacturer Code : %x\n",read_byte());
   set_adr(2);                  /* adr2,adr1,adr0 */
   printf("Device Code : %x\n",read_byte());
   set_adr(4);                  /* adr2,adr1,adr0 */
   printf("First 16k protection Code : %x\n",read_byte());
   reset_to_read();                     // reset to read mode
}

void win_id(void)
{
   out_data(0,0x55,0x55,0xaa);          /* softwave product ID entry */
   out_data(0,0x2a,0xaa,0x55);          /* adr2,adr1,adr0,data */
   out_data(0,0x55,0x55,0x80);          /* adr2,adr1,adr0,data */
   out_data(0,0x55,0x55,0xaa);          /* adr2,adr1,adr0,data */
   out_data(0,0x2a,0xaa,0x55);          /* adr2,adr1,adr0,data */
   out_data(0,0x55,0x55,0x60);          /* adr2,adr1,adr0,data */

   delay_10us();
   set_adr(0);                  /* adr2,adr1,adr0 */
   printf("Manufacturer Code : %x\n",read_byte());
   set_adr(1);                  /* adr2,adr1,adr0 */
   printf("Device Code : %x\n",read_byte());
//   set_adr(2);                        /* adr2,adr1,adr0 */
//   printf("First 16k protection Code : %x\n",read_byte());
//   set_bank(0x2000,0x1f);
//   set_adr(0x7ff2);                   /* adr2,adr1,adr0=0x7fff2 */
//   printf("Last 16k protection Code : %x\n",read_byte());

   out_data(0,0x55,0x55,0xaa);          /* softwave product ID exit */
   out_data(0,0x2a,0xaa,0x55);          /* adr2,adr1,adr0,data */
   out_data(0,0x55,0x55,0xf0);          /* adr2,adr1,adr0,data */
}

void intel_id(void)
{
   out_adr_data(0,0x98);                // Read Query
   for (i=0;i<128;i+=2){
      set_adr(i);                       /* adr2,adr1,adr0 */
      mix.buffer[i/2]=read_byte();
   }
//   disp_buffer(64);

   printf("Manufacture Code = %x\n",mix.buffer[0]);
   printf("Device Code = %x\n",mix.buffer[1]);
}

void disp_id(void)
{
   if (eeprom_type==4)
      win_id();
   if (eeprom_type==16)
      mx_id();
   if (eeprom_type==64)
      intel_id();
}

void out_adr_data_32k(unsigned int adr,unsigned char data)
{
   set_adr(adr);                // write adr:data
//   out_byte_eeprom(data);       // write data to eeprom
   out_byte(data);       // write data to eeprom
}

char check_eeprom(void)
{
   unsigned char mfgrcode, devcode;

   if (got_shark) return 1;
#if 0
   while (read_byte() != 0x66) {
     printf(".");
     set_adr(0x107);                  /* adr2,adr1,adr0 */
     delay_100us();
     delay_100us();
     delay_100us();
   }
   while (read_byte() == 0x66) {
     printf("-");
     set_adr(0x107);                  /* adr2,adr1,adr0 */
     delay_100us();
     delay_100us();
     delay_100us();
   }
   while (read_byte() != 0x66) {
     printf(".");
     set_adr(0x107);                  /* adr2,adr1,adr0 */
     delay_100us();
     delay_100us();
     delay_100us();
   }
#endif
   /* Send SST Software Product ID Entry Sequence */
   set_bank(0x3F00,0x41);   // Datel MBC Bank
   out_adr_data_32k(0x5555,0xaa);          /* 5555:aa softwave product ID entry */
   set_bank(0x3F00,0x40);   // Datel MBC Bank
   out_adr_data_32k(0x6aaa,0x55);          /* 2aaa:55 adr2,adr1,adr0,data */
   set_bank(0x3F00,0x41);   // Datel MBC Bank
   out_adr_data_32k(0x5555,0x90);          /* 5555:90 adr2,adr1,adr0,data */
//   set_adr(0x107);                  /* adr2,adr1,adr0 */
//   printf("Byte at 0x107 : %x\n",read_byte());

   set_adr(0);                  /* adr2,adr1,adr0 */
   mfgrcode = read_byte();
   set_adr(1);                  /* adr2,adr1,adr0 */
   devcode = read_byte();

//   printf("Manufacturer Code : %x\n",mfgrcode );
//   printf("Device Code : %x\n",devcode);
   set_adr(0);                  /* adr2,adr1,adr0 */
   reset_to_read();                     // reset to read mode
//   set_adr(0);                  /* adr2,adr1,adr0 */
//   printf("Byte at 0 : %x\n",read_byte());

   /* Send SST Software Product ID Exit Sequence */
	out_adr_data_32k(0x5555,0xAA);          /* 5555:aa softwave product ID entry */
	out_adr_data_32k(0x2aaa,0x55);          /* 2aaa:55 adr2,adr1,adr0,data */
	out_adr_data_32k(0x5555,0xF0);          /* 5555:90 adr2,adr1,adr0,data */
	delay_10us();
	reset_to_read();


   if (mfgrcode == 0xBF && devcode == 0xB6) {
	printf("Shark MX Found!\n");
	got_shark = 1;
	return(1);
   } else {
	printf("Shark MX Not Found!\n");
	got_shark = 0;
	return(0);
   }


// check_64m:
//    init_port();
//    out_adr_data(0x0000,0x98);           // Read Query
//    for (i=0;i<128;i+=2){
//       set_adr(i);                       /* adr2,adr1,adr0 */
//       mix.buffer[i/2]=read_byte();
//    }
//   disp_buffer(64);
//    if (mix.buffer[0]==0x89 && mix.buffer[1]==0x15 &&
//        mix.buffer[0x10]=='Q' && mix.buffer[0x11]=='R' &&
//        mix.buffer[0x12]=='Y'){
//       eeprom_type=64;
//       out_adr_data(0x0000,0xff);        // read array
//       return(0);
//    }
//    else{
//       return(1);
//   }

}

void set_sram_bank(unsigned char bank)
{
   set_adr(0x4000);             // set sram adr
   out_byte(bank);              // sram bank 0
}

void read_eeprom_16k(unsigned int bank_16k)
{
   printf("r");
   idx=0;

   if (mbc1_exp){
      set_bank(0x6000,0);       // for MCB1 expand bank
      if ((bank_16k & 0x1f)==0){
         set_sram_bank((bank_16k>>5) & 0x3);    // use sram bank intend rom bank
//       printf("^");
      }
      bank_16k=bank_16k& 0x1f;
   }
//j   set_bank(0x2000,bank_16k);   // for MCB1 16k bank
   set_bank(0x3F00,0x40 + bank_16k);   // for MCB1 16k bank
   for (j=0;j<64;j++){          // 16k bytes = 64 x 256 bytes
      if (bank_16k)
	 set_ai_data(1,(j|0x40));       // set adr[15..8]
      else
	 set_ai_data(1,(j|0x40));       // set adr[15..8]
//j	 set_ai_data(1,j);              // a[15..0]
      set_ai_data(0,0);                 // a[7..0]
      set_ai_data(2,0x81);              // enable read inc.
      set_ai(3);                        // read/write data
      set_data_read
      for (i=0;i<256;i++) {             // page=256
//       set_ai_data(0,i);              // a[7..0]
//       mix.buffer[idx+i]=read_byte();
         mix.buffer[idx+i]=read_data();
//       mix.buffer[idx+i]=inportb(data);
      }
      idx=idx+256;
   }
//   printf(" ok\n");
}

char verify_eeprom_16k(unsigned int bank_16k)
{
   printf("v");
   idx=0;

   if (mbc1_exp){
      set_bank(0x6000,0);       // for MCB1 expand bank
      if ((bank_16k & 0x1f)==0){
	 set_sram_bank((bank_16k>>5) & 0x3);    // use sram bank intend rom bank
//       printf("^");
      }
      bank_16k=bank_16k& 0x1f;
   }

//   set_bank(0x3000,(bank_16k>>8)&0xff); // for MCB1 16k bank
   set_bank(0x3F00,0x40 + bank_16k);    // for MCB1 16k bank
   for (j=0;j<64;j++){          // 16k bytes = 64 x 256 bytes
	set_ai_data(1,(j|0x40)); /* set adr[15..8] */
      set_ai_data(0,0);                 // a[7..0]
      set_ai_data(2,0x81);              // enable read inc.
      set_ai(3);                        // read/write data
      set_data_read
      for (i=0;i<256;i++) {
	 temp=read_data();
//       temp=inportb(data);
         if(temp!=mix.buffer[idx+i]) {
//init_port();
            printf(" error at %lx!\07\n",(bank_16k*16384)+(j*256)+i);
            return(-1);
         }
      }
      idx=idx+256;
   }
//   printf(" ok\n");
   return(0);
}

void set_page_write(void)       // start page write command
{
   out_adr2_data(0x5555,0xaa);          // 5555:aa adr2,adr1,adr0,data
   out_adr2_data(0x2aaa,0x55);          // 2aaa:55
   out_adr2_data(0x5555,0xa0);          // 5555:a0
}

char page_write_128(unsigned int bank_16k,unsigned char hi_lo)
{
unsigned char retry,temp,verify_ok;
   retry=retry_time;
   while(retry){
         set_page_write();              // each page is 128 bytes
         set_bank(0x2000,bank_16k);     // for MCB1 16k bank
	 if (bank_16k)
	    set_ai_data(1,(j|0x40));    // set adr[15..8]
         else
            set_ai_data(1,j);

         set_ai_data(0,hi_lo);          // a[7..0]
         set_ai_data(2,0x83);           // enable flash write inc.
         set_ai(3);                     // read/write data
         for (i=0;i<128;i++){
//          outportb(port_8,mix.buffer[idx+i]);
//          outportb(port_a,0x03);      // ndstb=0
//          outportb(port_a,0x01);      // ndstb=1
            write_data(mix.buffer[idx+i]);// write data to eeprom
//          outportb(data,mix.buffer[idx+i]);// write data to eeprom
	 }
         set_ai_data(2,0x80);           // disable wr/rd inc.
         delay_10us();
//       delay_20ms();
//       set_ai_data(1,0x00);           // ce=lo
//       set_ai_data(0,hi_lo|0x7f);     // point to last address
         if (wait_status()){
            printf("write error !!!\n");
            return(-1);
         }

// verify data
         reset_to_read();               // return to read mode
//return(0);
         verify_ok=1;                   // verify ok
         set_bank(0x2000,bank_16k);     // for MCB1 16k bank
         if (bank_16k)
            set_ai_data(1,(j|0x40));    // set adr[15..8]
	 else
	    set_ai_data(1,j);

         set_ai_data(0,hi_lo);          // a[7..0]
         set_ai_data(2,0x81);           // enable inc.
	 set_ai(3);                     // read/write data
         set_data_read
         for (i=0;i<128;i++) {          // page=128
            temp=read_data();
//          temp=inportb(data);
	    if (temp!=mix.buffer[idx+i]){
//          printf(" %x(%x)[%x] ",i,temp,mix.buffer[idx+i]);
               verify_ok=0;             // verify error
	       i=128;
	    }
         }
         if (verify_ok)
	    break;
         else{
//          printf("%d",retry);
            retry--;
            if (retry==0){
	       printf("retry write error!!!\n");
//read_status_reg_cmd();
//wait_status();
//reset_to_read();
	       return(-1);
	    }
	 }
   }
   idx+=128;
//   printf("idx=%x",idx);
   return(0);
}

char win_write_eeprom_16k(unsigned int bank_16k)
{
   int wr_done,err_cnt;
   printf("w");
//   disable_protection();

   idx=0;

   for (j=0;j<64;j++){          // 16k bytes = 64 x 256 bytes
      err_cnt=16;               // retry write counter
      wr_done=1;
      while(wr_done){
//       set_ai_data(2,0x80);           // disable wr/rd inc.
	 enable_protection();
// write 256 byte
	 set_bank(0x2000,bank_16k);     // for MCB1 16k bank
	 if (bank_16k)
	    set_ai_data(1,(j|0x40));    // set adr[15..8]
	 else
	    set_ai_data(1,j);

	 set_ai_data(0,0);              // a[7..0]
//       set_ai_data(2,0x82);           // enable flash write
	 set_ai_data(2,0x83);           // enable flash write inc.
	 set_ai(3);                     // read/write data
//       set_ai_data(2,0x80);

	 for (i=0;i<256;i++){
//          set_ai_data(0,i);
//          set_ai(3);                  // read/write data
//          out_byte_eeprom(mix.buffer[idx+i]);// write data to eeprom
	    write_data(mix.buffer[idx+i]);// write data to eeprom
//          outportb(data,mix.buffer[idx+i]);// write data to eeprom
	 }
	 set_ai_data(2,0x80);           // disable wr/rd inc.
	 set_ai_data(0,0xff);           // point to xxff
	 if (data_polling())
	    printf("write error check(d6)!!!\n");

	 wr_done=0;
//   delay_20ms();

// verify 256 byte
      set_ai_data(0,0);                 // a[7..0]
      set_ai_data(2,0x81);              // enable read inc.
      set_ai(3);                        // read/write data
      set_data_read

	 for (i=0;i<256;i++){
//          set_ai_data(0,i);           // a[7..0]
	    temp=read_data();
//          temp=inportb(data);
//          printf("%x ",temp);
	    if(temp!=mix.buffer[idx+i]){
	       err_cnt--;
//init_port();
//printf("temp=%x buf=%x idx=%x j=%x i=%x\n",temp,mix.buffer[idx+i],idx,j,i);
//             printf("X");
//   delay_20ms();
	       wr_done=1;
	       i=256;
	    }
	 }
	 if (err_cnt==0){
	    printf("retry write error!!!\07\n");
	    return(-1);
	 }
      }
      idx=idx+256;
   }
//   printf(" ok\n");
//   enable_protection();
//   disable_protection();
//   delay_20ms();
   return(verify_eeprom_16k(bank_16k));
//   return(0);
}

char mx_write_eeprom_16k(unsigned int bank_16k)
{
   printf("w");
   idx=0;

   for (j=0;j<64;j++){          // 16k bytes = 64 x 256 bytes
      if (page_write_128(bank_16k,0))   // write first 128 bytes
	 return(-1);
      if (page_write_128(bank_16k,0x80))        // write second 128 bytes
	 return(-1);
   }
   reset_to_read();             // return to read mode
//   printf(" ok\n");
//   return(0);
   return(verify_eeprom_16k(bank_16k));
}

/*void dump_intel_data(void)
{
   out_adr_data(0,0xff);// read array command
   for (i=0;i<64;i++){  // read 0x100~0x150 to buffer
      set_adr(i);
      mix.buffer[i]=read_data();
   }
   disp_buffer(64);
}
*/

char shark_write_eeprom_16k(unsigned int bank_16k)
{
	printf("w");
	idx=0;

	for (j=0; j<16384 ;j++){          // 16k bytes

		set_bank(0x3F00,0x41);   // Datel MBC Bank
		out_adr_data_32k(0x5555,0xaa);          /* 5555:aa softwave product ID entry */
		out_adr_data_32k(0x2aaa,0x55);          /* 2aaa:55 adr2,adr1,adr0,data */
		out_adr_data_32k(0x5555,0xA0);          /* 5555:90 adr2,adr1,adr0,data */
		set_bank(0x3F00,0x40+bank_16k);   // Datel MBC Bank
		out_adr_data_32k(0x4000+j,mix.buffer[j]);          /* 5555:aa softwave product ID entry */
		if (data_polling()){
			printf("write error!!!\n");
			return(-1);
		}

	}
	reset_to_read();

	return(0);
}

char intel_byte_write_32(unsigned long block_adr)
{
   for (i=0;i<32;i++){
      out_adr_data(block_adr+idx+i,0x40);// Write byte command
      out_adr_data(block_adr+idx+i,mix.buffer[idx+i]);// Write data
      time_out=0x8000;

      if (intel_check_status()){
	    printf("Intel byte write command time out\n");
            printf("status = %x\n",intel_read_status());
//          dump_intel_data();
            return(-1);
      }
   }
   if (intel_read_status()==0x80)
      return(0);
   else
      return(-1);       // error
}

char intel_buffer_write_32(unsigned long block_adr)
{
   out_adr_data(block_adr+idx,0xe8);// Write buffer command
   set_ai_data(2,0x82);         // wei enable
   set_ai(3);                   // default write mode
//   write_data(0xe8);          // Write buffer command
   set_data_read                // ninit=0, nWrite=1
   time_out=0x80000;
//   while (!(read_byte() & 0x80)){
   while (!(read_data() & 0x80)){
      time_out--;
      if (time_out==0){
	 printf("Intel buffer write command time out\n");
         printf("status = %x\n",intel_read_status());
//       dump_intel_data();
         return(-1);
      }
   set_data_write
   write_data(0xe8);            // out data
   set_data_read
//      out_byte_eeprom(0xe8);  // write buffer command
   }

//   out_byte_eeprom(0x1f);     // set write byte count command
   write_data(0x1f);            // out data

   set_ai_data(2,0x83);         // enable flash write inc.
   set_ai(3);
   for (i=0;i<32;i++){
      write_data(mix.buffer[idx+i]);// write data to eeprom
   }
   write_data(0xd0);                    // write confirm command
   return(0);
}

char intel_write_eeprom_16k(unsigned int bank_16k)
{
   unsigned long block_adr;

   block_adr=bank_16k;
   block_adr=block_adr << 14;   // convert to real address
   if ((bank_16k & 0x07)==0){
      if (intel_block_erase(block_adr)) return(-1);
   }
   printf("w");

//   set_adr_long(block_adr);   // set real address
   for (j=0;j<512;j++){         // 16k bytes = 512 X 32 bytes
//      time_out=0x8000;
      idx=j*32;
//      if (intel_byte_write_32(block_adr)) return(-1);
      if (intel_buffer_write_32(block_adr)){
         printf("write error\07\n");
	 printf("status = %x\n",intel_read_status());
         return(-1);
      }
   }

   if (intel_check_status()){
      printf("Intel buffer write command error\n");
      printf("status = %x\n",intel_read_status());
//      dump_intel_data();
      return(-1);
   }
   if (intel_read_status()!=0x80) return(-1);   // error

   out_adr_data(0,0xff);        // read array
   set_data_read
//   printf(" ok\n");
//   return(0);
   return(verify_eeprom_16k(bank_16k));
}

void chk_dsp_name(char chk)
{
   char game_name[17];
   for (i=0;i<16;i++)
      game_name[i]=mix.buffer[i+0x134];
   game_name[i]=0;
   printf("Game Name = %s\n",game_name);

   cart_type=mix.buffer[0x147];
   rom_size=mix.buffer[0x148];
   if(cart_type>0 && cart_type<4 && rom_size>4 && chk){ // mbc1 8M/16M
      mbc1_exp=1;
//      printf("mbc1_exp=1\n");
   }
}

char verify_cart_from_file(void)
{
struct ffblk ffblk;
int done;
   unsigned int page,num_page;
//   printf("Input filename : ");
//   scanf("%s",file_name);
   done = findfirst(file_name,&ffblk,0);
   if (done){
      printf("file not found!\07\n");
      return(-1);
   }
   filesize=ffblk.ff_fsize;
   printf("file length= %ld\n", filesize);

   if ((filesize<0x8000) || (filesize & 0x7fff) || (filesize>maxfilesize)){
      printf("filesize error!\07\n");
      return(-1);
   }
   num_page=(filesize/0x8000)*2;        // how many 16k banks
//   printf("num_page=%d\n",num_page);
   if((fptr=fopen(file_name,"rb"))==NULL){/* open error */
      printf("open error !!!\07\n");
      return(-1);
   }

   for (page=0;page<num_page;page++){
      if (read_16k_file()!=0){
         printf("load file error\n");
         fclose(fptr);
	 return(-1);
      }
      if (page==0)
	 chk_dsp_name(1);       // display game name and check mbc1_exp
      if (verify_eeprom_16k(page)){
         printf("verify cart error at bank=%x\n",page);
         fclose(fptr);
         return(-1);
      }
   }
   printf("verify cart ok\n");
   fclose(fptr);
   return(0);
}

char write_eeprom_16k(unsigned int bank_16k)
{
//   if (eeprom_type==4)          // winbond 4Mbits eeprom
//      return(win_write_eeprom_16k(bank_16k));
//   if (eeprom_type==16)         // MX 16Mbits eeprom
//      return(mx_write_eeprom_16k(bank_16k));
//   if (eeprom_type==64)         // Intel 64Mbits eeprom
//      return(intel_write_eeprom_16k(bank_16k));
	return(shark_write_eeprom_16k(bank_16k));
   return(-1);
}

char write_cart_from_file(void)
{
struct ffblk ffblk;
int done;
   unsigned int page,num_page;
//   printf("Input filename : ");
//   scanf("%s",file_name);
   done = findfirst(file_name,&ffblk,0);
   if (done){
      printf("file not found!\07\n");
      return(-1);
   }
   filesize=ffblk.ff_fsize;
   printf("file length= %ld\n", filesize);

   if ((filesize<0x8000) || (filesize & 0x7fff) || (filesize>maxfilesize)){
      printf("filesize error!\07\n");
//      return(-1);
   }
   num_page=(filesize/0x8000)*2;        // how many 16k banks
//   printf("num_page=%d\n",num_page);
   if((fptr=fopen(file_name,"rb"))==NULL){/* open error */
      printf("open error !!!\07\n");
      return(-1);
   }

   if (got_shark){        // erase 16M flash
      if (erase()){             // erase error
	 fclose(fptr);          // close file handle
	 return(-1);
      }
   }

   for (page=0;page<num_page;page++){
      if (read_16k_file()!=0){
	 printf("load file error\n");
	 fclose(fptr);
	 return(-1);
      }
      if (page==0)
	 chk_dsp_name(0);       // display game name only
      if (write_eeprom_16k(page)){
	 printf("write cart error at bank=%x\n",page);
	 fclose(fptr);
	 return(-1);
      }
   }
   printf("write cart ok\n");
   fclose(fptr);
   if((fptr=fopen(file_name,"rb"))==NULL){/* open error */
      printf("open error !!!\07\n");
      return(-1);
   }

   for (page=0;page<num_page;page++){
      if (read_16k_file()!=0){
	 printf("load file error\n");
	 fclose(fptr);
	 return(-1);
      }
      if (verify_eeprom_16k(page)){
	 printf("verify cart error at bank=%x\n",page);
	 fclose(fptr);
	 return(-1);
      }
   }
   printf("verify cart ok\n");
   fclose(fptr);
   return(0);
}

char backup_rom(void)   //no_page=how many 32K
// 0    "256kBit = 32kB = 2 banks",
// 1    "512kBit = 64kB = 4 banks",
// 2    "1MBit = 128kB = 8 banks",
// 3    "2MBit = 256kB = 16 banks",
// 4    "4MBit = 512kB = 32 banks"};
// 5    "8MBit = 1MB = 64 banks",
// 6    "16MBit = 2MB = 128 banks",
{
   unsigned int max_bank_define[]={2,4,8,16,32,64,128,256,512};
   unsigned int max_bank,rom_bank;
   max_bank=max_bank_define[rom_size];
   if (cmd=='B'){
      if (got_shark)
	 max_bank=16;	// backup 2M
      if (eeprom_type==4)
	 max_bank=32;   // backup 4M
      if (eeprom_type==16)
         max_bank=128;  // backup 16M
      if (eeprom_type==64)
	 max_bank=512;  // backup 64M
   }
//   printf("max_bank=%d\n",max_bank);
   for (rom_bank=0;rom_bank<max_bank;rom_bank++){
      read_eeprom_16k(rom_bank);
      if (verify_eeprom_16k(rom_bank)!=0) printf("X");

      if (write_file_xxk(0x4000)!=0){
         fclose(fptr);
         return(-1);
      }
   }
   fclose(fptr);
   return(0);
}

char check_card(void)
{
   unsigned int sum=0xd3;       // magic code = 0xd3
   unsigned char cts[64];       // cart type string
   unsigned char check_header[48]={
      0xce,0xed,0x66,0x66,0xcc,0x0d,0x00,0x0b,
      0x03,0x73,0x00,0x83,0x00,0x0c,0x00,0x0d,
      0x00,0x08,0x11,0x1f,0x88,0x89,0x00,0x0e,
      0xdc,0xcc,0x6e,0xe6,0xdd,0xdd,0xd9,0x99,
      0xbb,0xbb,0x67,0x63,0x6e,0x0e,0xec,0xcc,
      0xdd,0xdc,0x99,0x9f,0xbb,0xb9,0x33,0x3e};
   char *cart_type_define[]={
        "ROM ONLY",             //0
        "ROM+MBC1",             //1
        "ROM+MBC1+RAM",         //2
        "ROM+MBC1+RAM+BATTERY", //3
        "",                     //4
        "ROM+MBC2",             //5
        "ROM+MBC2+BATTERY"};    //6
   char *rom_size_define[]={
        "256kBit = 32kB = 2 banks",
        "512kBit = 64kB = 4 banks",
        "1MBit = 128kB = 8 banks",
        "2MBit = 256kB = 16 banks",
        "4MBit = 512kB = 32 banks",
	"8MBit = 1MB = 64 banks",
        "16MBit = 2MB = 128 banks",
        "32MBit = 4MB = 256 banks",
        "64MBit = 8MB = 512 banks"};
   char *ram_size_define[]={
        "None",
        "16kBit = 2kB = 1 bank",
        "64kBit = 8kB = 1 bank",
        "256kBit = 32kB = 4 banks"};
   char game_name[17];

   printf("checking rom information...\n");
   header_ok=1;
   for (i=0;i<48;i++){
      if (mix.buffer[i+4]!=check_header[i]){
	 header_ok=0;
	 break;
      }
   }
//   if (!header_ok){
//      printf("no GB data present!\07\n");
//      disp_buffer(0x50);
//      return(-1);
//   }
   for (i=0;i<16;i++)
      game_name[i]=mix.buffer[i+0x34];
   game_name[i]=0;
   printf("Game Name = %s\n",game_name);

   cart_type=mix.buffer[0x47];

   switch (cart_type){
      case 0x00:strcpy(cts,"ROM only");break;   // cart type string
      case 0x01:strcpy(cts,"ROM+MBC1");break;
      case 0x02:strcpy(cts,"ROM+MBC1+RAM");break;
      case 0x03:strcpy(cts,"ROM+MBC1+RAM+BATTERY");break;
      case 0x05:strcpy(cts,"ROM+MBC");break;
      case 0x06:strcpy(cts,"ROM+MBC2+BATTERY");break;
      case 0x08:strcpy(cts,"ROM+RAM");break;
      case 0x09:strcpy(cts,"ROM+RAM+BATTERY");break;
      case 0x0B:strcpy(cts,"ROM+MMM01");break;
      case 0x0C:strcpy(cts,"ROM+MMM01+SRAM");break;
      case 0x0D:strcpy(cts,"ROM+MMM01+SRAM+BATTERY");break;
      case 0x0F:strcpy(cts,"ROM+MBC3+TIMER+BATTERY");break;
      case 0x10:strcpy(cts,"ROM+MBC3+TIMER+RAM+BATTERY");break;
      case 0x11:strcpy(cts,"ROM+MBC3");break;
      case 0x12:strcpy(cts,"ROM+MBC3+RAM");break;
      case 0x13:strcpy(cts,"ROM+MBC3+RAM+BATTERY");break;
      case 0x19:strcpy(cts,"ROM+MBC5");break;
      case 0x1A:strcpy(cts,"ROM+MBC5+RAM");break;
      case 0x1B:strcpy(cts,"ROM+MBC5+RAM+BATTERY");break;
      case 0x1C:strcpy(cts,"ROM+MBC5+RUMBLE");break;
      case 0x1D:strcpy(cts,"ROM+MBC5+RUMBLE+SRAM");break;
      case 0x1E:strcpy(cts,"ROM+MBC5+RUMBLE+SRAM+BATTERY");break;
      case 0x1F:strcpy(cts,"Pocker Camera");break;
      case 0xfd:strcpy(cts,"Bandai TAMA5");break;
      case 0xfe:strcpy(cts,"Hudson HuC-3");break;
      case 0xff:strcpy(cts,"Hudson HuC-1");break;

      default : strcpy(cts,"Not define");break;
   }
   printf("Cartridge type(%d) = %s\n",cart_type,cts);
   rom_size=mix.buffer[0x48];
   if (rom_size>8){
      printf("Rom size error!!! (%d)\n",rom_size);
      return(-1);
   }
   else
      printf("Rom size(%d) = %s\n",rom_size,rom_size_define[rom_size]);
   ram_size=mix.buffer[0x49];
   if (ram_size>4){
      printf("Ram size error!!! (%d)\n",ram_size);
      return(-1);
   }
   else
      printf("Ram size(%d) = %s\n",ram_size,ram_size_define[ram_size]);

   if(cart_type>0 && cart_type<4 && rom_size>4){        // mbc1 8M/16M
      mbc1_exp=1;
//      printf("mbc1_exp=1\n");
   }

//******************* check sum **********************
   gcrc=(mix.buffer[0x4e]<<8)|mix.buffer[0x4f];
   for (i=0x4;i<0x4d;i++)
      sum+=mix.buffer[i];
   if ((sum+mix.buffer[0x4d])&0xff){
      printf("Error Complement($%x), Correct Complement($%x)\07\n",mix.buffer[0x4d],(0x100-(sum&0xff)));

   }
   return(0);
}

char open_read()
{
struct ffblk ffblk;
int done;
   done = findfirst(file_name,&ffblk,0);
   if (done){
      printf("file not found!\07\n");
      return(-1);
   }
   filesize=ffblk.ff_fsize;
   printf("file length= %ld\n", filesize);

   if ((filesize<0x8000) || (filesize & 0x7fff) || (filesize>maxfilesize)){
      printf("filesize error!\07\n");
      return(-1);
   }
//   num_page=(filesize/0x8000)*2;      // how many 16k banks
//   printf("num_page=%d\n",num_page);
   if((fptr=fopen(file_name,"rb"))==NULL){/* open error */
      printf("open error !!!\07\n");
      return(-1);
   }

   if (read_16k_file()!=0){
      printf("load file error\n");
      fclose(fptr);
      return(-1);
   }
   fclose(fptr);
   return(0);
}

char check_header(char mode)

{
   if (mode){                   // check game cart
      for (i=0x100;i<0x150;i++){        // read 0x100~0x150 to buffer
         set_adr(i);
         mix.buffer[i-0x100]=read_data();
//       mix.buffer[i-0x100]=inportb(data);
      }
   }
   else{

      if (open_read())
         return(-1);
      fclose(fptr);                     // close file
      for (i=0;i<0x50;i++){
         mix.buffer[i]=mix.buffer[i+0x100];
      }

   }
   return(check_card());
}

char chk_gcrc(void)
{
struct ffblk ffblk;
int done;
unsigned int sum_gcrc;
   unsigned int page,num_page;
   done = findfirst(file_name,&ffblk,0);
   if (done){
      printf("file not found!\07\n");
      return(-1);
   }
   filesize=ffblk.ff_fsize;
//   printf("file length= %ld\n", filesize);

   if ((filesize<0x8000) || (filesize & 0x7fff) || (filesize>maxfilesize)){
      printf("filesize error!\07\n");
      return(-1);
   }
   num_page=(filesize/0x8000)*2;        // how many 16k banks
//   printf("num_page=%d\n",num_page);
   if((fptr=fopen(file_name,"rb"))==NULL){/* open error */
      printf("open error !!!\07\n");
      return(-1);
   }

   sum_gcrc=0;
   for (page=0;page<num_page;page++){
      if (read_16k_file()!=0){
         printf("load file error\n");
         fclose(fptr);
         return(-1);
      }
      if (page==0){
         for(i=0;i<0x4000;i++)
            if (i<0x14e || i>0x14f)     // skip gcrc
               sum_gcrc+=mix.buffer[i];
      }
      else{
         for(i=0;i<0x4000;i++)
            sum_gcrc+=mix.buffer[i];
      }
   }
   fclose(fptr);
   if (gcrc!=sum_gcrc)
      printf("Error Checksum($%x), Correct Checksum($%x)\07\n",gcrc,sum_gcrc);
   return(0);
}

void backup(void)
{
   unsigned char ch;            // single key function
   if (cmd != 'B' && check_header(1) ){                // something error
//      disp_buffer(0x50);
      return;
   }
   if((fptr=fopen(file_name,"w+b"))==NULL)
      {/* open error */
      printf("open error !!!\07\n");
      return;
      }
   if (backup_rom())
      printf("Backup Error!!!\07\n");
   else
      printf("Backup Rom success\n");
   return;
}

void SetSramBank(void)
{
   if (pocket_camera){
      bank_size=16;
      return;
   }
   bank_size=0;
   if (eeprom_type==4)
      bank_size=4;              // 4 x 8K =32K
   if (eeprom_type==16 || eeprom_type==64)
      bank_size=16;             // 16 x 8K =128K
}

void enable_sram_bank(void)
{
   init_port();
   set_adr(0x0);                // write 0000:0x0a default read mode
   out_byte(0x0a);              // enable sram
   out_byte(0xc0);              // disable sram
   set_adr(0xa000);
   out_byte(0xa0);              // ctr index
   set_adr(0xa100);
//   out_byte(0x00);            // ram_off,ram_bank_disable,MBC1
   out_byte(0xc0);              // ram_on,ram_bank_enable,MBC1

   set_adr(0x0);                // write 0000:0x0a
   out_byte(0x0a);              // enable sram
}

void gen_pat(unsigned int offset)
{
   for (i=0; i<0x2000; i++){    // 8k word = 16k bytes
      mix.bufferx[i]=i+offset;
   }
}

char test_sram_v(void)
{
   enable_sram_bank();

   for (bank=0;bank<bank_size;bank++){
      idx=0;
      set_sram_bank(bank);
      gen_pat(bank);
      for (j=0;j<0x20;j++){     // 32 x 256 = 8192(8kbytes)
         set_ai_data(1,(0xa0+j));       // sram at 0xa000~bfff
         set_ai_data(0,0);      // a[7..0]=0
         set_ai_data(2,0x81);   // enable inc
         set_ai(3);             // point to data r/w port
	 set_data_read
	 for (i=0;i<256;i++){
            temp=read_data();
//          temp=inportb(data);
            if (mix.buffer[i+idx]!=temp){
               printf("sram verify error!\07\n");
//             printf("sram verify error! bank=%x j=%x i=%x temp=%x pat=%x\n",bank,j,i,temp,mix.buffer[idx+i]);
               return(-1);
            }
         }
	 set_ai_data(2,0x80);   // disable inc
         idx=idx+256;
      }
   }
   if (bank_size==4)
      printf("256k sram verify ok\n");
   if (bank_size==16)
      printf("1M sram verify ok\n");
   return(0);
}

char test_sram_wv(void)
{
   enable_sram_bank();
   for (bank=0;bank<bank_size;bank++){
      idx=0;
//      printf("w");
      set_sram_bank(bank);
      gen_pat(bank);
//      disp_buffer(0x10);
      for (j=0;j<0x20;j++){     // 32 x 256 = 8192(8kbytes)
         set_ai_data(1,(0xa0+j));       // sram at 0xa000~bfff
         set_ai_data(0,0);      // a[7..0]=0
	 set_ai_data(2,0x81);   // enable inc
         set_ai(3);             // point to data r/w port
	 set_data_write
         for (i=0;i<256;i++){
            write_data(mix.buffer[i+idx]);
//          outportb(data,mix.buffer[i+idx]);
         }
         set_ai_data(2,0x80);   // disable inc
         idx=idx+256;
      }
   }
   if (bank_size==4)
      printf("256k sram pattern written\n");
   if (bank_size==16)
      printf("1M sram pattern written\n");
   return(test_sram_v());
}

char test_all(void)
{
   if (write_cart_from_file())
      return(-1);
   return (test_sram_wv());
}

void usage(char *progname)
{
//   fprintf(stderr, "Usage: %s [-option] <Filename>\n", progname);
   fprintf(stderr, "Usage: gbtmx [-option] <Filename>\n");
   fprintf(stderr, "-l   : load ROM file to GB Card.\n");
   fprintf(stderr, "-lsa : load 256k/1Mbits sram from PC to GB Card.\n");
   fprintf(stderr, "-lsn : load 64kbits sram file from PC to specific sram bank(-n) in GB card.\n");
   fprintf(stderr, "-lsc : load 1Mbits sram file from PC to Pocket Camera.\n");
   fprintf(stderr, "-b   : auto-detect size and backup entire GB Card to PC.\n");
   fprintf(stderr, "-ba  : backup full 4Mbits/16Mbits GB Card to PC.\n");
   fprintf(stderr, "-bsa : retrieve all sram data (256k/1Mbits) from GB Card to PC.\n");
   fprintf(stderr, "-bsn : retrieve specific bank(-n) sram data(64kbits) from GB Card to PC .\n");
   fprintf(stderr, "-bsc : retrieve 1Mbits sram from Pocket Camera to PC.\n");
   fprintf(stderr, "-v   : verify file in PC with GB Card.\n");
   fprintf(stderr, "-e   : erase Flash rom.\n");
   fprintf(stderr, "-c   : check ROM file header.\n");
   end_port();
   exit(2);
}

char check_port_xpp(void)
{
   init_port();
   set_ai_data(1,0x12);
   set_ai_data(0,0x34);
   set_ai(1);
   set_data_read                // ninit=0, nwrite=1
   if (read_data()!=0x12) return(1);
   set_ai(0);
   set_data_read                // ninit=0, nwrite=1
   if (read_data()!=0x34) return(1);
   end_port();
   return(0);
}

char check_port(void)
{
   port_type=1;         // 0=epp, 1=spp
   if (check_port_xpp()==0){
      epp_spp=1;                // epp port present
   }
   if (port_8==0x3bc){          // if port=0x3bc skip epp test
      return(!epp_spp);
   }
   port_type=0;
   if (check_port_xpp()){
      if (epp_spp){
	 port_type=1;
         end_port();
         return(0);
      }
      else
         return(1);             // no port found
   }
   return(0);
}

char open_read_sram()
{
struct ffblk ffblk;
int done;
   done = findfirst(file_name,&ffblk,0);
   if (done){
      printf("file not found!\07\n");
      return(-1);
   }
   filesize=ffblk.ff_fsize;
   printf("file length= %ld\n", filesize);
//   printf("bank_size=%d\n",bank_size);

   if (cmd=='L'){       // check file size 256k/1Mbits
      if ((filesize/0x2000)>bank_size){
            printf("filesize error!\07\n");
            return(-1);
         }

   }
   else{
      if (filesize!=0x2000){    // check file size 8kBytes
         printf("filesize error!\07\n");
         return(-1);
      }
   }

   if((fptr=fopen(file_name,"rb"))==NULL){/* open error */
      printf("open error !!!\07\n");
      return(-1);
   }

   bank_size=filesize/0x2000;           //********* over write bank of size
   if (cmd=='L'){
      return(0);
   }
   else{
      if (read_8k_file()!=0){
         printf("load file error\n");
         fclose(fptr);
         return(-1);
      }
   }
   fclose(fptr);
   return(0);
}

char write_sram_xxk(unsigned int length)
{
   if((fptr=fopen(file_name,"w+b"))==NULL)
      {/* open error */
      printf("open error !!!\07\n");
      return(-1);
      }
   if (write_file_xxk(length)!=0){
      fclose(fptr);
      return(-1);
   }
      fclose(fptr);
   return(0);
}

char read_all_sram_to_file(void)
{
   enable_sram_bank();
   if((fptr=fopen(file_name,"w+b"))==NULL)
      {/* open error */
      printf("open error !!!\07\n");
      return(-1);
      }

   for (bank=0;bank<bank_size;bank++){
      if ((bank & 3)==0) idx=0;
      set_sram_bank(bank);
      for (j=0;j<0x20;j++){     // 32 x 256 = 8192(8kbytes)
	 set_ai_data(1,(0xa0+j));       // sram at 0xa000~bfff
         set_ai_data(0,0);      // a[7..0]=0
         set_ai_data(2,0x81);   // enable inc
         set_ai(3);             // point to data r/w port
         set_data_read
         for (i=0;i<256;i++){
            mix.buffer[i+idx]=read_data();
//          mix.buffer[i+idx]=inportb(data);
         }
         set_ai_data(2,0x80);   // disable inc
         idx=idx+256;
      }
      if ((bank & 3)==3){
	 if (write_file_xxk(0x8000)!=0){
            printf("write file error\07\n");
	    fclose(fptr);
            return(-1);
         }
      }
   }
   fclose(fptr);

   if (bank_size==4)
      printf("sram 256kbits saved\n");
   if (bank_size==16)
      printf("sram 1Mbits saved\n");

   return(0);
}

char read_8k_sram_to_file(void)
{
   if (bank_size==4){
      if (sram_bank_num>3){
         printf("bank number error!!!\07\n");
         return(-1);
      }
   }
   else{
      if (sram_bank_num>15){
         printf("bank number error!!!\07\n");
	 return(-1);
      }
   }

   enable_sram_bank();

   idx=0;
   bank=sram_bank_num;
   {
      set_sram_bank(bank);
      for (j=0;j<0x20;j++){     // 32 x 256 = 8192(8kbytes)
         set_ai_data(1,(0xa0+j));       // sram at 0xa000~bfff
         set_ai_data(0,0);      // a[7..0]=0
         set_ai_data(2,0x81);   // enable inc
	 set_ai(3);             // point to data r/w port
	 set_data_read
         for (i=0;i<256;i++){
	    mix.buffer[i+idx]=read_data();
//          mix.buffer[i+idx]=inportb(data);
	 }
         set_ai_data(2,0x80);   // disable inc
         idx=idx+256;
      }
   }

   if (write_sram_xxk(0x2000)!=0){
      printf("write file 64kbits error\07\n");
      return(-1);
   }
   else
      printf("sram 64kbits saved from bank %d\n",sram_bank_num);
   return(0);
}

char write_all_sram_from_file(void)
{
   enable_sram_bank();
   if (open_read_sram()!=0)     // read sram data from file to buffer
      return(-1);
   for (bank=0;bank<bank_size;bank++){
      if ((bank & 3)==0){
         idx=0;
	 if (read_32k_file()!=0){
            printf("load file error\n");
            fclose(fptr);
	    return(-1);
         }

      }
//      printf("w");
      set_sram_bank(bank);
//      disp_buffer(0x10);
      for (j=0;j<0x20;j++){     // 32 x 256 = 8192(8kbytes)
         set_ai_data(1,(0xa0+j));       // sram at 0xa000~bfff
         set_ai_data(0,0);      // a[7..0]=0
         set_ai_data(2,0x81);   // enable inc
         set_ai(3);             // point to data r/w port
         set_data_write
         for (i=0;i<256;i++){
            write_data(mix.buffer[i+idx]);
//          outportb(data,mix.buffer[i+idx]);
         }
	 set_ai_data(2,0x80);   // disable inc
         idx=idx+256;
      }
   }
   fclose(fptr);

   if (bank_size==4)
      printf("write sram 256kbits ok\n");
   if (bank_size==16)
      printf("write sram 1Mbits ok\n");
   return(0);
}

char write_8k_sram_from_file(void)
{
   if (bank_size==4){
      if (sram_bank_num>3){
         printf("bank number error!!!\07\n");
         return(-1);
      }

   }
   else{
      if (sram_bank_num>15){
         printf("bank number error!!!\07\n");
         return(-1);
      }

   }

   enable_sram_bank();
   if (open_read_sram()!=0)     // read sram data from file to buffer
      return(-1);

   idx=0;
   bank=sram_bank_num;
   {
//      printf("w");
      set_sram_bank(bank);
//      disp_buffer(0x10);
      for (j=0;j<0x20;j++){     // 32 x 256 = 8192(8kbytes)
         set_ai_data(1,(0xa0+j));       // sram at 0xa000~bfff
         set_ai_data(0,0);      // a[7..0]=0
	 set_ai_data(2,0x81);   // enable inc
         set_ai(3);             // point to data r/w port
	 set_data_write
         for (i=0;i<256;i++){
	    write_data(mix.buffer[i+idx]);
//          outportb(data,mix.buffer[i+idx]);
         }
         set_ai_data(2,0x80);   // disable inc
         idx=idx+256;
      }
   }
   printf("write sram 64kbits at bank %d ok\n",sram_bank_num);
   return(0);
}

void try_read(void)
{
   set_ai_data(0,0);      // a[7..0]=0
   set_ai_data(1,0x40);
   set_ai_data(2,0x81); // enable inc
   set_ai(3);           // point to data r/w port
   for (i=0;i<16;i++)
//      mix.buffer[i]=inportb(data);
      mix.buffer[i]=read_data();
   disp_buffer(16);
}

void try_read0(void)
{

   set_rom_bank(1);
   for (j=0;j<4;j++){
      set_sram_bank(j);
      try_read();
   }
   set_bank(0x6000,1);
   printf("6000:1\n");
   for (j=0;j<4;j++){
      set_sram_bank(j);
      try_read();
   }
   set_bank(0x6000,0);
   printf("6000:0\n");
   for (j=0;j<4;j++){
      set_sram_bank(j);
      try_read();
   }
}

void test_intel(void)
{
   out_adr2_data(0x0000,0x90);          /* softwave product ID entry */
   for (i=0;i<128;i+=2){
      set_adr(i);                       /* adr2,adr1,adr0 */
      mix.buffer[i/2]=read_byte();
   }
   disp_buffer(64);

   out_adr2_data(0x0000,0x70);          // read status register

   printf("status register = %x\n",read_byte());

}

/*************************************************
*                  MAIN ENTRY                    *
*************************************************/
void main(int argc, char *argv[])
{
   char card_present;
   unsigned char ch,i;  // single key function
   char *progname=argv[0];

   pocket_camera=0;
   mbc1_exp=0;          // 0=4M, 1=8M/16M expand mode
   eeprom_type=0;       // unknown type
   maxfilesize=524288*16;       // 64M files

   init_port();
   printf("\n--- Parallel EPP/SPP Port to GB Xchanger, Shark MX v0.01 ---\n");
   port[0]=peek(0x40,8);                // lpt1 base address
   port[1]=peek(0x40,10);               // lpt2 base address
   if (port[0]==0){
      printf("No Printer Port Avialable!\07\n");
      exit(1);
   }
   if (port[1]==0)
      port_no=1;                // only one printer port
   else
      port_no=2;                // two printer port
   card_present=0;
   for (i=0;i<port_no;i++){
      port_8=port[i];
      port_9=port_8+1;
      port_a=port_9+1;
      port_b=port_a+1;
      port_c=port_b+1;
      if (check_port()==0){
	 card_present=1;
	 break;
      }
   }

   cmd=NULL;
   if (argc==1) usage(progname);

//    file_name = argv[2];
//    printf("%s\n",file_name);
   cmd='w';             // default write mode
   for( i=1; i<argc; i++ ) {
      argv[i]=strlwr(argv[i]);
      if( argv[i][0] == '-' ) {
	 char *c = argv[i]+1;
	 char cmd_l = strlen(argv[i]);

	 if(*c=='l'){
	    if (cmd_l==2)
	       continue;                        // cmd='w'
	    if( *(c+1)=='s'&& *(c+2)=='a' && cmd_l==4){ // -lsa
	       cmd='L';                 // cmd='L'
	       continue;                // check next cmd
	    }
	    if( *(c+1)=='s'&& *(c+2)=='c' && cmd_l==4){ // -lsc
	       pocket_camera=1;                 // sram size=1Mbits
	       cmd='L';                 // cmd='L'
	       continue;                // check next cmd
	    }
	    if( *(c+1)=='s' && cmd_l>3){        // -lsn

	       if( *(c+2)>='0' && *(c+2)<':'){  // 0,1..8,9,:
		  sram_bank_num=*(c+2)-'0';
		  if( *(c+3)>='0' && *(c+3)<':'){       // 0,1..8,9,:
		     sram_bank_num=sram_bank_num*10 + (*(c+3)-'0');
		  }
	       }
	       cmd='l';
//             printf("cmd=%c %d\n",cmd,sram_bank_num);
	       continue;                // check next cmd
	    }
	 }else if(*c=='b'){
	    if (cmd_l==2){                      // -b
	       cmd='b';
	       continue;                        // cmd='b'
	    }
	    if(*(c+1)=='a' && cmd_l==3){        // -ba
	       cmd='B';                         // cmd='B'
	       continue;                        // check next cmd
	    }
	    if(*(c+1)=='s' && *(c+2)=='a' && cmd_l==4){ // -bsa
	       cmd='R';                         // cmd='R'
	       continue;                        // check next cmd
	    }
	    if(*(c+1)=='s' && *(c+2)=='c' && cmd_l==4){ // -bsc
	       pocket_camera=1;                 // sram size=1Mbits
	       cmd='R';                         // cmd='R'
	       continue;                        // check next cmd
	    }
	    if( *(c+1)=='s' && cmd_l>3){        // -bsn
	       if( *(c+2)>='0' && *(c+2)<':'){  // 0,1..8,9,:
		  sram_bank_num=*(c+2)-'0';
		  if( *(c+3)>='0' && *(c+3)<':'){       // 0,1..8,9,:
		     sram_bank_num=sram_bank_num*10 + (*(c+3)-'0');
		  }
	       }
	       cmd='r';                         // cmd='r'
//             printf("cmd=%c %d\n",cmd,sram_bank_num);
	       continue;                // check next cmd
	    }

	 }else if(*c=='t'){
	    if(*(c+1)=='s' && cmd_l==3){        // -ts
	       cmd='t';                         // cmd='t'
	       continue;                        // check next cmd
	    }
	    if(*(c+1)=='a' && cmd_l==3){        // -ta
	       cmd='A';                         // cmd='A'
	       continue;                        // check next cmd
	    }
	    if(*(c+1)=='s' && *(c+2)=='f' && cmd_l==4){ // -tsf
	       cmd='T';                         // cmd='T'
	       continue;                        // check next cmd
	    }
	 }

	 if( *(c+1) != '\0' ) usage(progname);
	 switch( *c ) {
	    case 'v' : cmd='v'; break;
	    case 'i' : cmd='i';break;
	    case 'e' : cmd='e';break;
	    case 'c' : cmd='c'; break;
	    default:
	       usage(progname);
	  }
      }
      else {
	 if( file_name == NULL ) {
	     file_name = argv[i];
	 }
	 else {
	     usage(progname);
	  }
      }
   }

//   try_read0();
//   exit(0);

   if (cmd=='c'){
      if (check_header(0)==0)
	 exit(chk_gcrc());
//      exit(0);
   }
   else {
      if (card_present==0){
	 printf("\nNo GBX card present!!!\07\n\n");
	 exit(1);
      }
      else{
	 printf("GBX card found at port%d",port_no);
	 if (port_type)
	    printf("(SPP)\n");
	 else{
	    if (epp_spp==1)
	       printf("(EPP and SPP)\n");
	    else
	       printf("(EPP)\n");
	 }
      }
      init_port();
   }

/*set_adr(0);
set_ai(3);
set_data_write
while(time_out--)
write_data(0xa);
exit(0);
*/

/*   set_adr(0x4000);           // set sram adr
   out_byte(0xff);              // sram bank 0
   time_out=0x800000;
   while (time_out--){

   }
   printf("test rumble\n");
   set_adr(0x4000);             // set sram adr
   out_byte(0x0);               // sram bank 0
*/

//   test_intel();

   check_eeprom();      // check 4M/16M
   if (eeprom_type==4)
      maxfilesize=524288;       // 4M files
   if (eeprom_type==16)
      maxfilesize=524288*4;     // 16M files
   if (eeprom_type==64)
      maxfilesize=524288*16;    // 64M files
   if (got_shark)
      maxfilesize=256*1024;	// 2M files

   if (cmd=='b' || cmd=='B'){
      backup();
      end_port();
      exit(0);
   }
   if (cmd=='v'){
      verify_cart_from_file();
      end_port();
      exit(0);
   }

   if (!check_eeprom()){
      bank_size=4;              // 4x8k=32k sram
      SetSramBank();    // set bank_size=4/16 banks of 8k
      switch(cmd) {             // for game cart only
         case 'R' : read_all_sram_to_file(); break;
         case 'r' : read_8k_sram_to_file(); break;
         case 'L' : write_all_sram_from_file(); break;
         case 'l' : write_8k_sram_from_file(); break;

      case 'w' :
      case 'i' :
      case 'e' :
      printf("Flash memory not present!!!\07\n");
      break;

      }
      end_port();
      exit(0);
   }

/*   if (check_eeprom()){
      printf("Flash memory not present!!!\07\n");
      end_port();
      exit(0);
   }
*/

   SetSramBank();       // set bank_size=4/16 banks of 8k
   switch(cmd) {        // for GB card only
      case 'w' : write_cart_from_file(); break;
      case 'R' : read_all_sram_to_file(); break;
      case 'r' : read_8k_sram_to_file(); break;
      case 'L' : write_all_sram_from_file(); break;
      case 'l' : write_8k_sram_from_file(); break;
//      case 'r' : read_eeprom_16k(0); break;
      case 'i' : disp_id();break;
      case 'e' : erase();break;
      case 't' : test_sram_v();break;
      case 'T' : test_sram_wv();break;
      case 'A' : test_all();break;
   }
   end_port();
   exit(0);
}
