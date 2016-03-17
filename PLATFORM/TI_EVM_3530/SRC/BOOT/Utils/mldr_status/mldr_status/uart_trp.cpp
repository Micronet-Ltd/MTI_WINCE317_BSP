
#include "stdafx.h"
#include "mldr_status.h"


// Global Variables:
HANDLE  hPort = NULL;


//  Return Value:  0 -failue, 1 - process is stopped by "complete" pattern passed in COM port,
//                 99 - process is stopped by "ESQ" char;
//
//
DWORD ComOperationsThread (pfnHandleData fnHandleData)
{
	DWORD NumBytesWritten = 0;
	DWORD Retval	= 0;
	DWORD error = 0;
	BOOL process_completed =FALSE;

	DWORD i = 0;

	char	Buffer [MAX_SIZE_TO_READ];
	DWORD	NumOfReadBytes = 0;


	Sleep(100);
	do
	{
		DWORD dwErr = 0;
		COMSTAT cs = {0};

		Sleep(200);

		if(_kbhit())
		{
			if('\x1B' == _getch())
				return 99;
		}


		DWORD err = ClearCommError(hPort, &dwErr, &cs);

		if(!err)
		{
			printf ("Error on COM port %d\r\n",GetLastError());
			break;
		}

		//if(dwErr)
			//wprintf(L"%X\r\n", dwErr);


		if(cs.fCtsHold || cs.fDsrHold || cs.fRlsdHold || cs.fXoffHold)
			wprintf(L"transmission is hold\r\n", dwErr);
	

		if(cs.cbInQue < 1)
			continue;


		Retval = ReadFile (hPort, 
			               Buffer,
			               (cs.cbInQue< sizeof(Buffer)-1)? cs.cbInQue:(sizeof(Buffer)-1),
						    &NumOfReadBytes, 
							NULL);

		if (Retval == 0)
		{
	 		error = GetLastError ();
			CloseHandle (hPort);
			return 0;
		}
		Buffer [NumOfReadBytes] = '\0';

	//	if(strstr(Buffer, "quit"))
	//		break;
		if (!fnHandleData(Buffer,&process_completed))
		{
	 		error = GetLastError ();
			CloseHandle (hPort);
			return 0;
		}
		if (process_completed)
		{
			Buffer[0]='\0';
			break;
		}
			

	} while (1);//(NumOfReadBytes != 0);

	return 1;
	
}



DWORD OpenPort (TCHAR* PortName)
{
	hPort = INVALID_HANDLE_VALUE;
	DWORD  error = 0;
	
	// Open the serial port.
	 hPort = CreateFile (PortName,		// Pointer to the name of the port
						GENERIC_READ | GENERIC_WRITE,
										// Access (read-write) mode
						0,				// Share mode
						NULL,			// Pointer to the security attribute
						OPEN_EXISTING,	// How to open the serial port
						0,				// Port attributes
					NULL);			// Handle to port with attribute to copy

	
	

	

	if (hPort == INVALID_HANDLE_VALUE)
	{
		//error = GetLastError ();
		return 0;
	}

	DCB dcb;

	if (! GetCommState(hPort, &dcb)) {

		CloseHandle (hPort);
		return 0;
	}

	dcb.DCBlength = sizeof(dcb);
    dcb.fBinary           = 1;
    dcb.fParity           = 0;


    dcb.BaudRate          = CBR_115200;
    dcb.ByteSize          = DATABITS_8; 
 //   dcb.Parity            = ODDPARITY; 
    dcb.Parity            = NOPARITY; 
    dcb.StopBits          = ONESTOPBIT;


	
    if (! SetCommState(hPort, &dcb)) {
		CloseHandle (hPort);
		return 0;
    }


 return 1;
}


DWORD  ClosePort()
{
	if (CloseHandle (hPort) == 0)
	{
		//int error = GetLastError ();
		return 0;
	}
	
	return 1;
}
