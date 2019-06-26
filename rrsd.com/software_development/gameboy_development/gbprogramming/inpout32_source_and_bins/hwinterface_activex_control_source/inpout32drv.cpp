// inpout32drv.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "hwinterfacedrv.h"
#include "resource.h"
#include "conio.h"
#include "Winsvc.h"
#include "drvutil.h"

int ver;
HINSTANCE hinst;
int inst();
int start();

HANDLE hdriver;
char path[MAX_PATH];
SECURITY_ATTRIBUTES sa;



/***********************************************************************/

void Closedriver(void)
{
    CloseHandle(hdriver);
}

void  Out32(short PortAddress, short data)
{
	switch(ver)
	{
	case 1:
			_outp( PortAddress,data);
	break;

	case 2:
			unsigned int error;
			DWORD BytesReturned;        
			BYTE Buffer[3];
			unsigned short * pBuffer;
			pBuffer = (unsigned short *)&Buffer[0];
			*pBuffer = LOWORD(PortAddress);
			Buffer[2] = LOBYTE(data);

			error = DeviceIoControl(hdriver,
                            IOCTL_WRITE_PORT_UCHAR,
                            &Buffer,
                            3,
                            NULL,
                            0,
                            &BytesReturned,
							NULL);
	break;
	}

	
}

/*********************************************************************/

short Inp32(short PortAddress)
{
	byte retdata;
	switch(ver)
	{

	case 1:
		retdata = _inp(PortAddress);
		/*
_asm
{
;xor         eax,eax
mov         dx,PortAddress
in          al,dx
mov retdata,al 
}
*/
		return  (short)retdata;
return _inp(PortAddress);
	case 2:
		unsigned int error;
		DWORD BytesReturned;
		unsigned char Buffer[3];
		unsigned short * pBuffer;
		pBuffer = (unsigned short *)&Buffer;
		*pBuffer = LOWORD(PortAddress);
		Buffer[2] = 0;
		error = DeviceIoControl(hdriver,
                            IOCTL_READ_PORT_UCHAR,
                            &Buffer,
                            2,
                            &Buffer,
                            1,
                            &BytesReturned,
                            NULL);

		return((int)Buffer[0]);

	break;
	}
return 0;
}

/*********************************************************************/

int Opendriver(void)
{
	ver = SystemVersion();
	if(ver == 1)
	{
		return 0;
	}
    hdriver = CreateFile("\\\\.\\hwinterface", 
                                 GENERIC_READ | GENERIC_WRITE, 
                                 0, 
                                 NULL,
                                 OPEN_EXISTING, 
                                 FILE_ATTRIBUTE_NORMAL, 
                                 NULL);
	
	if(hdriver == INVALID_HANDLE_VALUE) 
		{
		
		
		if(start())
		{
			inst();
			start();

			 hdriver = CreateFile("\\\\.\\hwinterface", 
                                 GENERIC_READ | GENERIC_WRITE, 
                                 0, 
                                 NULL,
                                 OPEN_EXISTING, 
                                 FILE_ATTRIBUTE_NORMAL, 
                                 NULL);

		}
		
		
		
		return 1;
		}
return 0;
}

/***********************************************************************/

int inst()
{

    SC_HANDLE  Mgr;
    SC_HANDLE  Ser;

	hinst = AfxGetInstanceHandle();
	GetSystemDirectory(path , sizeof(path));
	HRSRC hResource = FindResource(hinst, MAKEINTRESOURCE(IDR_BIN1), "bin");
	if(hResource)
	{
		HGLOBAL binGlob = LoadResource(hinst, hResource);
	
		if(binGlob)
		{
			void *binData = LockResource(binGlob);
		
			if(binData)
			{
				HANDLE file;
				strcat(path,"\\Drivers\\hwinterface.sys");
				
				file = CreateFile(path,
								  GENERIC_WRITE,
								  0,
								  NULL,
								  CREATE_ALWAYS,
								  0,
								  NULL);

				if(file)
				{
					DWORD size, written;

					size = SizeofResource(hinst, hResource);
					WriteFile(file, binData, size, &written, NULL);
					CloseHandle(file);

				}
			}
		}
	}


	Mgr = OpenSCManager (NULL, NULL,SC_MANAGER_ALL_ACCESS);
	    if (Mgr == NULL)
		{							//No permission to create service
			if (GetLastError() == ERROR_ACCESS_DENIED) 
			{
				return 5;  // error access denied
			}
		}	
		else
		{
		   Ser = CreateService (Mgr,                      
                                "hwinterface",                        
                                "hwinterface",                        
                                SERVICE_ALL_ACCESS,                
                                SERVICE_KERNEL_DRIVER,             
                                SERVICE_SYSTEM_START,               
                                SERVICE_ERROR_NORMAL,               
                                "System32\\Drivers\\hwinterface.sys",  
                                NULL,                               
                                NULL,                              
                                NULL,                               
                                NULL,                              
                                NULL                               
                                );




		}

CloseServiceHandle(Ser);
CloseServiceHandle(Mgr);

	return 0;
}
/**************************************************************************/
int start(void)
{
    SC_HANDLE  Mgr;
    SC_HANDLE  Ser;

	Mgr = OpenSCManager (NULL, NULL,SC_MANAGER_ALL_ACCESS);

	    if (Mgr == NULL)
		{							//No permission to create service
			if (GetLastError() == ERROR_ACCESS_DENIED) 
			{
				Mgr = OpenSCManager (NULL, NULL,GENERIC_READ);
				Ser = OpenService(Mgr,"hwinterface",GENERIC_EXECUTE);
				if (Ser)
				{    // we have permission to start the service
					if(!StartService(Ser,0,NULL))
					{
						CloseServiceHandle (Ser);
						return 4; // we could open the service but unable to start
					}
					
				}

			}
		}
		else
		{// Successfuly opened Service Manager with full access

				Ser = OpenService(Mgr,"hwinterface",GENERIC_EXECUTE);
				if (Ser)
				{
					if(!StartService(Ser,0,NULL))
					{
						CloseServiceHandle (Ser);
						return 3; // opened the Service handle with full access permission, but unable to start
					}
					else
					{
						CloseServiceHandle (Ser);
						return 0;
					}

				}

		}

return 1;
}