//#include <fcntl.h>
#include <conio.h>
//#include <windows.h>

/* Change those defines to match your compiler. */
/* delay 1 millisecond */
#define DELAY   //Sleep(1)
#define OUTPORT(port, val)      outp(port, val); DELAY
#define INPORT(port)            inp(port)

