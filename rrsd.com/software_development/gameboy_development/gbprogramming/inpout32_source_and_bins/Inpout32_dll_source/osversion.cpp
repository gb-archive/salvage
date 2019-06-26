#include "stdafx.h"
#include "hwinterfacedrv.h"

int SystemVersion()

{   
	
	   OSVERSIONINFOEX osvi;
       BOOL bOsVersionInfoEx;

	   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
       osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
		{
          osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
         
		  if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
			  return 0;  
		}

	      switch (osvi.dwPlatformId)
			  {      
				case VER_PLATFORM_WIN32_NT:

					return 2;		//WINNT

					break;

				case VER_PLATFORM_WIN32_WINDOWS:
						
					return 1;		//WIN9X

					break;

					}   
		  return 0; 
}
