/**************************************************/
/***                                            ***/
/*** TEST.c  -- test interface to inpout32.dll  ***/
/***  ( http://www.logix4u.net/inpout32.htm )   ***/
/***                                            ***/
/*** Copyright (C) 2003, Douglas Beattie Jr.    ***/
/***                                            ***/
/***    <beattidp@ieee.org>                     ***/
/***    http://www.hytherion.com/beattidp/      ***/
/***                                            ***/
/**************************************************/


/*******************************************************/
/*                                                     */
/*  Builds with Borland's Command-line C Compiler      */
/*    (free for public download from Borland.com, at   */
/*  http://www.borland.com/bcppbuilder/freecompiler )  */
/*                                                     */
/*   Compile with:                                     */
/*                                                     */
/*   BCC32 -IC:\BORLAND\BCC55\INCLUDE  TEST.C          */
/*                                                     */
/*                                                     */
/*  Be sure to change the Port addresses               */
/*  accordingly if your LPT port is addressed          */
/*  elsewhere.                                         */
/*                                                     */
/*******************************************************/




#include <stdio.h>
#include <conio.h>
#include <windows.h>


/* Definitions in the build of inpout32.dll are:            */
/*   short _stdcall Inp32(short PortAddress);               */
/*   void _stdcall Out32(short PortAddress, short data);    */


/* prototype (function typedef) for DLL function Inp32: */

     typedef short _stdcall (*inpfuncPtr)(short portaddr);
     typedef void _stdcall (*oupfuncPtr)(short portaddr, short datum);

int main(void)
{
     HINSTANCE hLib;
     inpfuncPtr inp32;
     oupfuncPtr oup32;

     short x;
     int i;

     /* Load the library */
     hLib = LoadLibrary("inpout32.dll");

     if (hLib == NULL) {
          printf("LoadLibrary Failed.\n");
          return -1;
     }

     /* get the address of the function */

     inp32 = (inpfuncPtr) GetProcAddress(hLib, "Inp32");

     if (inp32 == NULL) {
          printf("GetProcAddress for Inp32 Failed.\n");
          return -1;
     }


     oup32 = (oupfuncPtr) GetProcAddress(hLib, "Out32");

     if (oup32 == NULL) {
          printf("GetProcAddress for Oup32 Failed.\n");
          return -1;
     }


/***************************************************************/
/* now test the functions */

     /* Try to read 0x378..0x37F, LPT1:  */

     for (i=0x378; (i<0x380); i++) {

          x = (inp32)(i);

          printf("port read (%04X)= %04X\n",i,x);
     }



     /*****  Write the data register */

     i=0x378;
     x=0x77;

     (oup32)(i,x);

     printf("port write to 0x%X, datum=0x%2X\n" ,i ,x);

     /***** And read back to verify  */
     x = (inp32)(i);
     printf("port read (%04X)= %04X\n",i,x);



     /*****  One more time, different value */

     i=0x378;
     x=0xAA;

     (oup32)(i,x);

     printf("port write to 0x%X, datum=0x%2X\n" ,i ,x);

     /***** And read back to verify  */
     x = (inp32)(i);
     printf("port read (%04X)= %04X\n",i,x);




     FreeLibrary(hLib);
     return 0;
}
