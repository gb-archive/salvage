// include algorithm
//#include <algorithm>
//#include <iostream>
//using namespace std;

// include assertions

#include "stdafx.h"

//#include <wdm.h>
//#include <ntddk.h>
#include <winioctl.h>

#include <assert.h>
#include "ParallelPort.hpp"

void ErrorHandler(int i)
{
	char MsgBuf[255];
	DWORD result = FormatMessage( 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			i,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) MsgBuf,
			sizeof(MsgBuf),
			NULL 
	);
}

// Constructor
ParallelPort::ParallelPort (const std::string & portId):
	m_Handle (INVALID_HANDLE_VALUE),
    m_Name(portId)
{
}

// Destructor
ParallelPort::~ParallelPort()
{
	if(INVALID_HANDLE_VALUE != m_Handle)
		Close();
}

// Open the port
void ParallelPort::Open() //throw (InformationPortException)
{
	assert(INVALID_HANDLE_VALUE == m_Handle);

	// open the port
	m_Handle = ::CreateFile (
		m_Name.c_str(),
		GENERIC_READ | GENERIC_WRITE, 
		0, 
		0,
		OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED,
		0 
	);
	if (m_Handle == INVALID_HANDLE_VALUE) {
		TCHAR msg[255];
		DWORD ecode;
		ecode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			ecode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			msg,
			sizeof(msg),
			NULL
		);
//		throw InformationPortException (*this, string(msg));
	}
    char buffer[256];
    DWORD result;
    DeviceIoControl(
        m_Handle, //HANDLE hDevice,
        IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO, //DWORD dwIoControlCode,
        buffer, //LPVOID lpInBuffer,
        sizeof(buffer), //DWORD nInBufferSize,
        buffer, // LPVOID lpOutBuffer,
        sizeof(buffer), //DWORD nOutBufferSize,
        & result // LPDWORD lpBytesReturned,
        NULL// , LPOVERLAPPED lpOverlapped
    );

	// modify port parameters
	DCB dcb = getDCBParameters ();
	dcb.fDtrControl = 0;
	dcb.fRtsControl = 0;
	dcb.BaudRate = CBR_9600;
	dcb.StopBits = ONESTOPBIT;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.fParity = TRUE;
	dcb.EvtChar = 0x0d; // cr is the magic character that terminates an input message
	setDCBParameters (dcb);

	/**
	 * Save original comm timeouts and set new ones
	 */
	COMMTIMEOUTS timeouts;
	// we don't read the buffer until we detect a cr - so we should never timeout
	timeouts.ReadIntervalTimeout  = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;	// 1 millisecond
	timeouts.ReadTotalTimeoutConstant = 10;
	timeouts.WriteTotalTimeoutMultiplier = 1;
	timeouts.WriteTotalTimeoutConstant = 10;
	::SetCommTimeouts (m_Handle, &timeouts);

	/**
	 * set comm buffer sizes
	 */
	::SetupComm (m_Handle, 64, 64);

	// set events that interest us
	::SetCommMask(m_Handle, EV_ERR | EV_RXFLAG | EV_TXEMPTY);

	/**
	 * raise DTR
	 */
	if (!::EscapeCommFunction (m_Handle, SETDTR)) {
//		throw InformationPortException (*this, "Could not raise DTR.");
	}
}

// Close the port
void ParallelPort::Close()
{
	assert(INVALID_HANDLE_VALUE != m_Handle);
	// force WaitCommEvent to return
	::SetCommMask(m_Handle, EV_ERR | EV_RXFLAG | EV_TXEMPTY);
//	PurgeComm(_pHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
 	/**
	 * lower DTR
	 */
//	::EscapeCommFunction (_pHandle, CLRDTR);
	
	/**
	 * close the port
	 */
	::CloseHandle (m_Handle);
	m_Handle = INVALID_HANDLE_VALUE;
}

// retrieve the port parameters
DCB ParallelPort::getDCBParameters ()
{
	assert(m_Handle != INVALID_HANDLE_VALUE);
	// retrieve the current settings
	DCB dcb;
	::GetCommState (m_Handle, &dcb);
	return dcb;
}

// set the port parameters
void ParallelPort::setDCBParameters (const DCB params)
{
	assert(m_Handle != INVALID_HANDLE_VALUE);
	::SetCommState (m_Handle, const_cast<DCB *>(& params));
}

std::size_t ParallelPort::Read(void * address, std::size_t size){
    DWORD result = size;
    ReadFile(m_Handle,address,result, & result,NULL);
    return result;
}

std::size_t ParallelPort::Write(const void * address, std::size_t size){
    DWORD result = size;
    WriteFile(m_Handle, address, result, & result, NULL);
    return result;
}


#if 0
// write to the port
void ParallelPort::writeInformation (const std::string &str)
{
	assert(_pHandle != INVALID_HANDLE_VALUE);
	InformationPort::writeInformation(str);
	// wake up WaitCommEvent anyone waiting on this
	::SetCommMask(_pHandle, EV_ERR | EV_RXFLAG | EV_TXEMPTY);
}

void ParallelPort::run()
{
	OVERLAPPED oStatus;
	oStatus.Offset = 0;
	oStatus.OffsetHigh = 0;
	oStatus.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	OVERLAPPED oIO;
	oIO.Offset = 0;
	oIO.OffsetHigh = 0;
	oIO.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	unsigned long numRead = 0;
	char inputBuffer[64];
	const unsigned int READ_BUFFER_SIZE = sizeof(inputBuffer)/sizeof(inputBuffer[0]);
	char *pIB = inputBuffer;	// pointer to next place to put character in input buffer
	// initialize with a pending event
	DWORD commEvent;
	BOOL result;
	result = WaitCommEvent(_pHandle, &commEvent, &oStatus);
	if(FALSE == result){
		int i = GetLastError();
		if(ERROR_IO_PENDING != i){
			ErrorHandler(i);
			return;
		}
	}
	while (_running) {
		result = GetOverlappedResult(_pHandle, &oStatus, &numRead, TRUE);
		assert(TRUE == result);
		if(!isRunning())
			break;
		// immediatly issue a new status request so that one will always be pending
		result = WaitCommEvent(_pHandle, &commEvent, &oStatus);
		if(FALSE == result){
			int i = GetLastError();
			if(ERROR_IO_PENDING != i){
				ErrorHandler(i);
				return;
			}
		}
		InformationPort::Status status = 0;
		// not currently used
		#if 0
		if (0 != (EV_BREAK & commEvent))
			status |= InformationPort::ipEV_BREAK;
		if (0 != (EV_CTS & commEvent))
			status |= InformationPort::ipEV_CTS;
		if (0 != (EV_DSR & commEvent))
			status |= InformationPort::ipEV_DSR;
		if (0 != (EV_RING & commEvent))
			status |= InformationPort::ipEV_RING;
		if (0 != (EV_RLSD & commEvent))
			status |= InformationPort::ipEV_RLSD;
		if (0 != (EV_RXCHAR & commEvent))
			status |= InformationPort::ipEV_RXCHAR;
		#endif
		if (0 != (EV_ERR & commEvent))
			status |= InformationPort::ipEV_ERR;
		if (0 != (EV_RXFLAG & commEvent)){
			status |= InformationPort::ipEV_RXFLAG;
			assert(sizeof(inputBuffer) > (READ_BUFFER_SIZE - (pIB - inputBuffer) - 1));
			result = ::ReadFile(
				_pHandle,
				pIB,
				READ_BUFFER_SIZE - (pIB - inputBuffer) - 1,
				&numRead, 
				&oIO
			);
			if(FALSE == result){
				int i = GetLastError();
				if(ERROR_IO_PENDING != i){
					ErrorHandler(i);
					return;
				}
				result = GetOverlappedResult(_pHandle, &oIO, &numRead, TRUE);
				if(FALSE == result){
					i = GetLastError();
				}
			}
			if(0 < numRead){
				// figure end of buffer
				char *eBuf = pIB + numRead;
				// apend a null character to be sure strchr (below functions)
				*eBuf = '\0';
				// for each CR delimited string
				char *pCR;
				for(pIB = inputBuffer; ; pIB = pCR + 1){
					pCR = strchr(pIB, '\r');
					if(NULL == pCR)
						break;
					// process command
					notifyListeners(InformationPortEvent(InformationPortEvent::IPE_Read, string(pIB, pCR - pIB + 1)));
				}
				// shift pending characters to beginning of buffer
				memmove(inputBuffer, pIB, eBuf - pIB);
				pIB = inputBuffer;
			}
		}
		if(! _stringsBuffer.empty()){
			if(0 == (EV_TXEMPTY & commEvent)){
				DWORD errors;
				COMSTAT stat;
				ClearCommError(_pHandle, &errors, &stat);
				if(0 == stat.cbOutQue)
					commEvent |= EV_TXEMPTY;
			}
			if (0 != (EV_TXEMPTY & commEvent)){
				string *str = getNextString();
				DWORD written = 0;
				DWORD toWrite = str->size();
				char *buffer = (char *)str->c_str ();
				result = ::WriteFile (_pHandle, buffer, toWrite, &written, &oIO);
				if(FALSE == result){
					int i = GetLastError();
					if(ERROR_IO_PENDING != i){
						ErrorHandler(i);
						return;
					}
					result = GetOverlappedResult(_pHandle, &oIO, &numRead, TRUE);
					if(FALSE == result){
						i = GetLastError();
					}
				}
				notifyListeners(InformationPortEvent(InformationPortEvent::IPE_Write, *str));
				delete str;
			}
		}
	}
	::CloseHandle(oStatus.hEvent);
	::CloseHandle(oIO.hEvent);
}
#endif
