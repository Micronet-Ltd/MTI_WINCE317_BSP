// uart.cpp : Defines the entry point for the application.
//



// bluetooth.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>

//#include "stdafx.h"
//#define MAX_LOADSTRING 100

// Global Variables:
HANDLE hPort = NULL;
#define KEY_PRESSED 99

#define MAX_SIZE_TO_READ  2048
#define MAX_SIZE_TO_STORE 3072

char	general_buffer [MAX_SIZE_TO_STORE];
DWORD   general_buffer_lentgh;

//#define pattern_str "DBG_BOOT:"

//#define pattern_str "Sent BOOTME"
#define pattern_str              "D:\\>"
#define process_complete_pattern "UPDATE_COMPLETED"
#define progress_str             "PROGRESS"

DWORD ParseData(char* buffer, BOOL* completeRequest)
{
	*completeRequest = FALSE;
	size_t str_size;

	if(strstr(buffer,pattern_str))
	{

		if(strstr(buffer,progress_str))
		{
			str_size = strlen(buffer);
			// get out last 2 chars - \r\n - from the string 
			buffer[str_size-1]='\0';
			printf("\r%s", buffer);
		}
		else
		{
			printf(buffer);
		}
		
		if (strstr(buffer,process_complete_pattern))
			*completeRequest = TRUE;
	}
	
	return 1;
}

DWORD HandleData(char* buffer,BOOL* process_completed)
{
	//static TCHAR	general_buffer [MAX_SIZE_TO_STORE];
	char	buff [MAX_SIZE_TO_STORE];
	char*   str1;
	


	if ((strlen(general_buffer) + strlen(buffer))>MAX_SIZE_TO_STORE)
	{
		general_buffer[0] = '\0';
	}
	
	strcat (general_buffer,buffer);
	do
	{
		str1 = strchr(general_buffer,'\n');
		if (str1)
		{
			strncpy(buff,general_buffer,(str1-general_buffer+1));
			buff[str1-general_buffer+1]= '\0';
			strcpy(general_buffer,&general_buffer[str1-general_buffer+1]);
			ParseData (buff,process_completed);

			if (*process_completed)
				break;
		}
		else
			break;
	} while (strlen(general_buffer)>0);

/*
	for (i=0; i<MAX_SIZE_TO_STORE;i++)
	{
		strchr(
		if (general_buffer[i]=='\n')
		{
			//general_buffer[i]='\0';
			wcsncpy(TBuffer,general_buffer,i+1);
			TBuffer[i]='\0';
			wcscpy(general_buffer,&general_buffer[i+1]);
			PrintData(TBuffer);
			i=0;
		}
		else if (general_buffer[i]=='\0')
		{
			break;
		}
	}
	}

	if (i==MAX_SIZE_TO_STORE)
	{
		general_buffer=NULL;
		printf (" Error: buffer is too small\r\n");
		return 0;
	}
	
*/
	return 1;		
}

//  Return Value:  0 -failue, 1 - process is stopped by "complete" pattern passed in COM port,
//                 99 - process is stopped by "ESQ" char;
//
//
DWORD ComOperationsThread ()
{
	DWORD NumBytesWritten = 0;
	DWORD Retval	= 0;
	DWORD error = 0;
//	char Data [512];
	BOOL process_completed =FALSE;

	DWORD i = 0;

	char	Buffer [MAX_SIZE_TO_READ];
	DWORD	NumOfReadBytes = 0;


	general_buffer[0] = '\0';
	general_buffer_lentgh=0;

	Sleep(100);
	do
	{
		DWORD dwErr = 0;
		COMSTAT cs = {0};

		if(_kbhit())
		{
			if('\x1B' == _getch())
				return 99;
		}

		Sleep(100);

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
						   sizeof(Buffer),
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
		if (!HandleData(Buffer,&process_completed))
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
	/*
	TCHAR Buff[]= L"COM4:";
	
    wprintf(L"Open %s\r\n", PortName);
	*/
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
//    dcb.fParity           = 1;
    dcb.fParity           = 0;


    dcb.BaudRate          = CBR_115200;
    dcb.ByteSize          = DATABITS_8; 
 //   dcb.Parity            = ODDPARITY; 
    dcb.Parity            = NOPARITY; 
    dcb.StopBits          = ONESTOPBIT;

    dcb.fOutxCtsFlow      = 0;
    dcb.fOutxDsrFlow      = 0;
	
    dcb.fDtrControl       = DTR_CONTROL_ENABLE;
    dcb.fRtsControl       = RTS_CONTROL_ENABLE;
//	dcb.fDtrControl       = DTR_CONTROL_DISABLE;
 //   dcb.fRtsControl       = RTS_CONTROL_DISABLE;
    dcb.fDsrSensitivity   = 0;
    dcb.fTXContinueOnXoff = 0;

    dcb.fOutX             = 0;
    dcb.fInX              = 0;
	dcb.XoffLim			  = 1024;
	dcb.XoffLim			  = 512;
	dcb.XonChar			  = 17	;
	dcb.XoffChar		  = 19	;	

    dcb.fErrorChar        = 0;  
    dcb.ErrorChar         = 0x0D;  
	dcb.EvtChar			  = 0x0D;	
    dcb.fNull             = 0;
    dcb.fAbortOnError     = 0;
	



    if (! SetCommState(hPort, &dcb)) {
		CloseHandle (hPort);
		return 0;
    }


    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 1 ;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
    CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0 ;
    CommTimeOuts.WriteTotalTimeoutConstant = 0 ;

    if (! SetCommTimeouts (hPort, &CommTimeOuts)) {
		
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
