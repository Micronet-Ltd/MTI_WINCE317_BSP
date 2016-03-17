// mldr_status.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mldr_status.h"

//extern ostream cout;
//extern int a;

char	general_buffer [MAX_SIZE_TO_STORE]="";

DWORD ParseData(char* buffer, BOOL* completeRequest)
{
	*completeRequest = FALSE;
	size_t str_size;

	if(strstr(buffer,pattern_str))
	{

		if(strstr(buffer,progress_str))
		{
			str_size = strlen(buffer);
			// get out last  char - "\n" - from the string 
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

	return 1;		
}

static void usage()
{
	printf("H:\\>\r\n");
	printf("H:\\>Usage: mldr_status  <ComPort:>  where\r\n");
	printf("H:\\>       ComPort is a COM number used.\r\n");
	printf("H:\\>\r\n");
	Sleep(20000);
}

// currently we support only UART transport
// if we want to change the transport we need
// to make changes here

DWORD OperateTransport (pfnHandleData fnHandleData)
{
	return ComOperationsThread(fnHandleData);
}

DWORD InitTransport (TCHAR* devName)
{
	return OpenPort (devName);
}

DWORD CloseTransport()
{
	return ClosePort();
}

int _tmain(int argc, _TCHAR* argv[])
{

	TCHAR dev_name[10];
	DWORD ret;

	if( argc <= 1  ||  argc >= 5)
	{
		usage();
		return 0;
	}

	wcscpy_s(dev_name,10,argv[1]);

    //printf(L"The COM port to use is %s\r\n",port_name);
	do{
		if(_kbhit())
		{
			if('\x1B' == _getch())
				break;
		}
		printf ("H:\\>Trying to connect...\r\n");

		if (!InitTransport (dev_name))
		{
			wprintf (L"H:\\>ERROR: Can't connet to port %s. ErrNo is %d ...  exiting in few sec.\r\n",dev_name,GetLastError());
			Sleep(10000);
			return 0;
		}
		if (!(ret = OperateTransport (HandleData)))
		{
			wprintf (L"H:\\>ERROR: COM operation is aborted. ErrNo is %d ...  exiting in few sec.\r\n",GetLastError());
			Sleep(10000);
			return 0;
		}
		if (ret == KEY_PRESSED)
		{
			CloseTransport();
			break;	
		}

		// Finally close the handle.
		CloseTransport();
	}while (1);
	
	return 1;

}



