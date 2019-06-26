// CPU speed tests

#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpstdio.h"
#include "gpfont.h"

GPDRAWSURFACE gpDraw;
char tmp_string[256];
char FileName[256];
#define TEXT_RED 0xE0
#define TEXT_BLACK 0

// pllset.exe was used to calculate the CPU divisor speed
// values below. It can be obtained from Samsung's website in
// the same place as you find the cpu datasheet on their site.
//
// Example:
//  pllset 2 12000000 99000000
//    (To find divisor value for 99MHz clock.)
//
// NOTE: Messing with the CPU clock speed also can change
// various other timings (besides just the CPU) so do not change
// the CPU speed for any standard application until you read the
// CPU data sheet and fully understand what is going on.

void GpError (char* text)
   {
   GpTextOut(NULL, &gpDraw, 0, 16, text, TEXT_RED);
	while(1);
   }

void delay (void)
   {
   int i;
   int j;
   for (i=0; i<10000; i++)
      for (j=0; j<10000; j++)
         {}
   }

void GpMain (void *arg)
   {
   int i,Fout;
   unsigned char * p_buf;
   unsigned long m_size;
   int Loop = 1;
   int Pressed = 0;
   int Value = 0;
   unsigned char keydata;
   int ExKey;
   int Redraw = 0;

//    ERR_CODE err_code;
   F_HANDLE h_file;

   int cy;
   int entry_start_idx;
   int FileCount;

   ERR_CODE err_code;
   GPDIRENTRY *p_list;
   unsigned long list_count = 0, read_count = 0;
   GPFILEATTR mAttr;

   i = GpLcdSurfaceGet(&gpDraw, 0);

   GpSurfaceSet(&gpDraw);

   GpRectFill(NULL, &gpDraw, 0, 0, 320, 240, 0xff);
   i = * (int *)0x14800004;
   sprintf (tmp_string, "MDIV=0x%x, PDIV=0x%x, SDIV=%d", i>>12, (i>>4)&0x3f, i&3);
   GpTextOut (NULL, &gpDraw, 0, 32, (char*)tmp_string, TEXT_RED);

   Fout = ( ((i>>12)+8)*12000000 ) / ( (((i>>4)&0x3f)+2) * (1<<(i&3)) );

   sprintf (tmp_string, "CPU speed = %d MHz", Fout);
   GpTextOut (NULL, &gpDraw, 0, 48, (char*)tmp_string, TEXT_BLACK);

   sprintf (tmp_string, "On next screen select speed with Up/Dwn.");
   GpTextOut (NULL, &gpDraw, 0, 64, (char*)tmp_string, TEXT_RED);
   sprintf (tmp_string, "Then press Start & count until change.");
   GpTextOut (NULL, &gpDraw, 0, 80, (char*)tmp_string, TEXT_RED);

   sprintf (tmp_string, "Press Up or Down to continue");
   GpTextOut (NULL, &gpDraw, 0, 96, (char*)tmp_string, TEXT_BLACK);

   while (Loop)
      {
      if (Redraw)
         {
         GpRectFill(NULL, &gpDraw, 0, 0, 320, 240, 0xff);
         sprintf (tmp_string, " %x  ", Value);
         GpTextOut (NULL, &gpDraw, 0, 16, (char*)tmp_string, TEXT_RED);
         Redraw = 0;
         }

      GpKeyGetEx(&ExKey);
      keydata = ExKey & 0xff;

      if (Pressed == 0)
         {
         if ( ( keydata & GPC_VK_DOWN ) == GPC_VK_DOWN )
            {
            Value++;
            if (Value == 9) Value = 0;
            Pressed = 1;
            Redraw = 1;
            }
         if ( ( keydata & GPC_VK_UP ) == GPC_VK_UP )
            {
            if (Value == 0) Value = 9; else Value--;
            Pressed = 1;
            Redraw = 1;
            }
         if ( (ExKey & GPC_VK_START) ||
              (ExKey & GPC_VK_SELECT) )
            {
            Loop = 0;
            }
         }
      else
         {
         if ( ( ( keydata & GPC_VK_DOWN ) == GPC_VK_DOWN ) ||
              ( ( keydata & GPC_VK_UP ) == GPC_VK_UP ) )
            Pressed = 1;
         else
            Pressed = 0;
         }

      }

   switch (Value)
      {      // All values in seconds below are ~
      case 0:     // default - 21 seconds
         break;
      case 1:
         GpClockSpeedChange (40000000, 0x48013, 1); // 35.5 seconds
         break;
      case 2:
         GpClockSpeedChange (59250000, 0x47022, 1); // 24 seconds
         break;
      case 3:
         GpClockSpeedChange (67500000, 0x25002, 1); // 21 seconds
         break;
      case 4:
         GpClockSpeedChange (80000000, 0x48012, 1); // 17 seconds
         break;
      case 5:
         GpClockSpeedChange (99000000, 0x3a002, 1); // 14.5 seconds
         break;
      case 6:
         GpClockSpeedChange(105000000, 0x1b001, 1); // crash
         break;
      case 7:
         GpClockSpeedChange(110000000, 0x2f011, 1);
         break;
      case 8:
         GpClockSpeedChange(120000000, 0x34011, 1);
         break;
      case 9:
         GpClockSpeedChange(132000000, 0x3a011, 1);
         break;
      }

   while (1)
      {
      GpRectFill(NULL, &gpDraw, 0, 0, 320, 240, 0xff);
      GpTextOut (NULL, &gpDraw, 0, 16, "////", TEXT_RED);
      delay ();
      GpRectFill(NULL, &gpDraw, 0, 0, 320, 240, 0xff);
      GpTextOut (NULL, &gpDraw, 0, 16, "\\\\\\\\", TEXT_RED);
      delay ();
      }

   while (1) {}
   }
