#include "stdafx.h"
#include "bench2.h"
#include <windows.h>

#define FLOWCONTROL_HARD		1
#define FLOWCONTROL_SOFT		2
#define FLOWCONTROL_OFF			3

HANDLE hPort;
DWORD dwBytesRead=0;
char szDebugOut[256];

//extern "C" BOOL VirtualCopy( 
//LPVOID lpvDest, 
//LPVOID lpvSrc, 
//DWORD cbSize, 
//DWORD fdwProtect 
//);

void *serial_open(TCHAR *name, unsigned long baud, unsigned long flow)
{
	int ret;
	void *vp;
	DCB dcb;
	COMMTIMEOUTS CommTimeouts;

	vp = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if ((void *)-1 == vp)
		return (void *)-1;
	__try
	{
		
		ret = GetCommState(vp, &dcb);
		if(!ret)
			__leave;

		dcb.DCBlength = sizeof(dcb);

		dcb.BaudRate		= baud;
		dcb.ByteSize		= DATABITS_8;
		dcb.StopBits		= ONESTOPBIT;
		dcb.fParity			= 0;
		dcb.Parity			= NOPARITY;
		dcb.fDsrSensitivity	= 0;
		dcb.fOutxDsrFlow	= 0;
		dcb.fDtrControl		= DTR_CONTROL_ENABLE;
    
		if(FLOWCONTROL_HARD == flow)
		{
			// Enable RTS/CTS Flow Control
			dcb.fRtsControl	= RTS_CONTROL_HANDSHAKE;
			dcb.fOutxCtsFlow= 1;
			dcb.fOutX		= 0;
			dcb.fInX		= 0;
		}
		else if(FLOWCONTROL_SOFT == flow)
		{
			// Enable XON/XOFF Flow Control
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
			dcb.fOutxCtsFlow= 0;
			dcb.fOutX		= 1;
			dcb.fInX		= 1;  
		}
		else
		{
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
			dcb.fOutxCtsFlow= 0;
			dcb.fOutX		= 0;
			dcb.fInX		= 0;
		}

		ret = SetCommState(vp, &dcb);
		if(!ret)
			__leave;

		GetCommTimeouts(vp, &CommTimeouts);

		CommTimeouts.ReadIntervalTimeout		= MAXDWORD;
		CommTimeouts.ReadTotalTimeoutMultiplier = 0;
		CommTimeouts.ReadTotalTimeoutConstant	= 0;
		CommTimeouts.WriteTotalTimeoutMultiplier= 0;
		CommTimeouts.WriteTotalTimeoutConstant	= 0;

		SetCommTimeouts(vp, &CommTimeouts);
	}
	__finally
	{
		if(!ret)
		{
			CloseHandle(vp);
			vp = (void *)-1;
		}
	}

	return vp;

}


void serial_close(void *vp)
{
	CloseHandle(vp);
}

int serial_put(void *vp, char *buf, int len)
{
	int ret;
	unsigned long bytesWritten;

	do
	{
		ret = WriteFile(vp, buf, len, &bytesWritten, 0);
		if(!ret)
			break;
		len -= bytesWritten;
		buf += bytesWritten;
	}while(len);

	return len;
}
