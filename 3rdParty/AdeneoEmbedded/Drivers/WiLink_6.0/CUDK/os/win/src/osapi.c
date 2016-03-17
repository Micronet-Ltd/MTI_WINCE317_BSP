/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

/****************************************************************************
*
*   MODULE:  osapi.c
*   
*   PURPOSE: 
* 
*   DESCRIPTION:  
*   ============
*   OS Memory API for user mode application (CUDK)      
*
****************************************************************************/

/* includes */
/************/
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "ipc_sta.h"

#include "TWDriver.h"
#include "STADExternalIf.h"
#include "cu_os.h"

#include "cu_osapi.h"


/* defines */
/***********/
#define MAX_HOST_MESSAGE_SIZE   256

S32 ipc_pipe[2];

extern S32 user_main (S32 argc, PPS8 argv);

#define TI_DRIVER_PRINT_REDIRECTED_PORT 3421
static DWORD g_dwRedirectDrvOutput = FALSE;
static DWORD g_dwRedirectToSerial = FALSE;  
static HANDLE g_hReadThread = NULL;

VOID os_OsSpecificCmdParams(S32 argc, PS8* argv)
{
	int i;

	if( argc > 1 )
    {
        for( i=1; i < argc; i++ )
        {
			if (!strcmp(argv[i], "-r" ) )
            {
                g_dwRedirectDrvOutput = TRUE;
            }
			else if (!strcmp(argv[i], "-c" ) )
            {
                g_dwRedirectToSerial = TRUE;
            }
        }
    }
}


DWORD driverReadThreadFunc(LPVOID lpParam)
{
    SOCKET socket_id = 0;
    int result;
    struct sockaddr_in server_addr;
    WORD wVersionRequested;
    WSADATA wsaData;
    char buffer[512];

    wVersionRequested = MAKEWORD( 2, 2 );
     
    result = WSAStartup( wVersionRequested, &wsaData );
    if ( result != 0 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        fprintf(stderr, "Could not initializate WinSocks.\n" );
        return(1);
    }
     
    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions later    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */
     
    if ( LOBYTE( wsaData.wVersion ) != 2 ||
            HIBYTE( wsaData.wVersion ) != 2 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup( );
        fprintf(stderr, "Could not find a usable WinSock DLL.\n" );
        return(1);
    }
     
    /* The WinSock DLL is acceptable. Proceed. */
    /******************/
    /* Open a socket */
    /****************/

    socket_id = socket(PF_INET, SOCK_DGRAM, 0);

    if (!socket_id) {
        /* Error opening socket */
        fprintf(stderr, "%s: error opening socket socket.\n", __FUNCTION__);
        return(1);
    }
    /********************/
    /* Bind the socket */
    /******************/

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    server_addr.sin_port = htons(TI_DRIVER_PRINT_REDIRECTED_PORT);

    result = bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (result != 0) {
        /* Error binding socket */
        fprintf(stderr, "%s: error binding logger socket. Error %d\n", __FUNCTION__, WSAGetLastError());
        closesocket(socket_id);
        return(1);
    }
    do {
        result = recvfrom(socket_id, buffer, 512, 0, NULL, 0);
        if( (result != 0) && (result != SOCKET_ERROR)) {
            printf("%s", buffer);
        }
        else {
            if(result == SOCKET_ERROR) {
                fprintf(stderr, "%s: error is returned by recvfrom.\n", __FUNCTION__);
            }
            else {
                fprintf(stderr, "%s: zerro is returned by recvfrom.\n", __FUNCTION__);
            }
            break;
        }
    } while(TRUE);

    closesocket(socket_id);
    return(0); 
}


VOID os_InitOsSpecificModules(VOID)
{
	DWORD dwRet;
	HKEY hKey;
	DWORD dwValue;

    const TCHAR cszConsole[] = TEXT("Drivers\\Console");
    const TCHAR cszOutputTo[] = TEXT("OutputTo");
    const TCHAR cszCOMSpeed[] = TEXT("COMSpeed");

	if(g_dwRedirectDrvOutput) {

		CuOs_RedirectDrvOutputToTicon(NULL);

		g_hReadThread = CreateThread(NULL, 0, driverReadThreadFunc, NULL, 0, NULL);
		if(g_hReadThread == NULL) {
			fprintf(stderr, "%s: error creating read thread.\n", __FUNCTION__);
		}

	}

	if(g_dwRedirectToSerial) {

		dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszConsole, 0, 0, &hKey);

		if (ERROR_SUCCESS != dwRet) {
			fprintf(stderr, "Could not open HKLM\\Console\\Driver\n" );
			return;
		}

		dwValue = 38400;
		dwRet = RegSetValueEx(hKey, cszCOMSpeed, 0, REG_DWORD, (PBYTE)&dwValue, sizeof(dwValue));

		if (ERROR_SUCCESS != dwRet) {
			fprintf(stderr, "Could not write HKLM\\Console\\Driver\\COMSpeed=dword:38400\n" );
			return;
		}

		dwValue = 1;
		dwRet = RegSetValueEx(hKey, cszOutputTo, 0, REG_DWORD, (PBYTE)&dwValue, sizeof(dwValue));

		if (ERROR_SUCCESS != dwRet) {
			fprintf(stderr, "Could not write HKLM\\Console\\Driver\\OutputTo=dword:1\n" );
			return;
		}

	}

}


VOID os_DeInitOsSpecificModules(VOID)
{
	HKEY hKey;
	DWORD dwRet;
	const TCHAR cszConsole[] = TEXT("Drivers\\Console");
	const TCHAR cszOutputTo[] = TEXT("OutputTo");
	DWORD dwValue;

	if(g_dwRedirectDrvOutput){
		CloseHandle(g_hReadThread);
	}

    if(g_dwRedirectToSerial) {
		dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszConsole, 0, 0, &hKey);
		if (ERROR_SUCCESS != dwRet)
		{
			os_error_printf(CU_MSG_ERROR, "Could not open HKLM\\Console\\Driver\n" );
		}
		
		dwValue = (DWORD)-1;
		dwRet = RegSetValueEx(hKey, cszOutputTo, 0, REG_DWORD, (PBYTE)&dwValue, sizeof(dwValue));
		if (ERROR_SUCCESS != dwRet)
		{
			os_error_printf(CU_MSG_ERROR, "Could not write HKLM\\Console\\Driver\\OutputTo=dword:4294967295 (-1)\n" );
		}
	}

}



/** 
 * \fn     main
 * \brief  Main entry point to a user-mode program 
 * 
 * This is the main() function for a user mode program, or the entry point
 * called by the OS, This calls an OS-abstracted main function
 * 
 * \param  argc - command line argument count
 * \param  argv - command line arguments
 * \return 0 on success, any other value indicates error 
 * \sa     user_main
 */ 
int main (int argc, char** argv)
{
    return  user_main (argc, (PPS8)argv);
}

/****************************************************************************************
 *                        os_error_printf()                                 
 ****************************************************************************************
DESCRIPTION:    This function prints a debug message

ARGUMENTS:      OsContext   -   our adapter context.
                arg_list - string to output with arguments

RETURN:         None
*****************************************************************************************/
VOID os_error_printf(S32 debug_level, const PS8 arg_list ,...)
{
    static int g_debug_level = CU_MSG_ERROR;
    va_list ap;
    S8 msg[MAX_HOST_MESSAGE_SIZE];

    if (debug_level < g_debug_level)
        return;

    
    /* Format the message */
    va_start(ap, arg_list);
    vsprintf(msg, arg_list, ap);
    va_end(ap);

    /* print the message */
    //fprintf(stderr, msg);
	RETAILMSG(1, (TEXT("%S"), msg));
}

/****************************************************************************************
 *                        os_strcpy()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS strcpy fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
 PS8 os_strcpy(PS8 dest, const PS8 src)
{
    return strcpy(dest, src);
}

/****************************************************************************************
 *                        os_strncpy()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS strncpy fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
PS8 os_strncpy(PS8 dest, const PS8 src, S32 n)
{
    return strncpy(dest, src, n);
}

/****************************************************************************************
 *                        os_sprintf()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS sprintf fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
S32 os_sprintf(PS8 str, const PS8 format, ...)
{
    return sprintf(str, format);
}

/****************************************************************************************
 *                        os_Printf()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS printf fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
S32 os_Printf(const PS8 buffer)
{
    return printf(buffer);
}


/****************************************************************************************
 *                        os_strcat()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS strcat fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
PS8 os_strcat(PS8 dest, const PS8 src)
{
    return strcat(dest, src);
}

/****************************************************************************************
 *                        os_strlen()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS strlen fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
U32 os_strlen(const PS8 s)
{
    return strlen(s);
}

/****************************************************************************************
 *                        os_memoryCAlloc()                                 
 ****************************************************************************************
DESCRIPTION:    Allocates an array in memory with elements initialized to 0.

ARGUMENTS:      OsContext   -   our adapter context.
                Number      -   Number of elements
                Size        -   Length in bytes of each element

RETURN:         Pointer to the allocated memory.
*****************************************************************************************/
PVOID os_MemoryCAlloc(U32 Number, U32 Size)
{
    return calloc(Number, Size);
}

/****************************************************************************************
 *                        os_memoryAlloc()                                 
 ****************************************************************************************
DESCRIPTION:    Allocates resident (nonpaged) system-space memory.

ARGUMENTS:      OsContext   - our adapter context.
                Size        - Specifies the size, in bytes, to be allocated.

RETURN:         Pointer to the allocated memory.
*****************************************************************************************/
PVOID os_MemoryAlloc(U32 Size)
{
    return malloc(Size);
}

/****************************************************************************************
 *                        os_memoryFree()                                 
 ****************************************************************************************
DESCRIPTION:    This function releases a block of memory previously allocated with the
                os_memoryAlloc function.


ARGUMENTS:      OsContext   -   our adapter context.
                pMemPtr     -   Pointer to the base virtual address of the allocated memory.
                                This address was returned by the os_memoryAlloc function.
                Size        -   Redundant, needed only for kernel mode.

RETURN:         None
*****************************************************************************************/
VOID os_MemoryFree(PVOID pMemPtr)
{
     free(pMemPtr);
}

/****************************************************************************************
 *                        os_memset()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS memset fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
PVOID os_memset(PVOID s, U8 c, U32 n)
{
    return memset(s, c, n);
}

/****************************************************************************************
 *                        os_memcpy()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS memcpy fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
PVOID os_memcpy(PVOID dest, const PVOID src, U32 n)
{
    return memcpy(dest, src, n);
}

/****************************************************************************************
 *                        os_memcmp()                                 
 ****************************************************************************************
DESCRIPTION:    wrapper to the OS memcmp fucntion

ARGUMENTS:      

RETURN:         
*****************************************************************************************/
S32 os_memcmp(const PVOID s1, const PVOID s2, S32 n)
{
    return memcmp(s1, s2, n);
}

/************************************************************************
 *                        os_strcmp                     *
 ************************************************************************
DESCRIPTION: wrapper to the OS strcmp fucntion

CONTEXT:  
************************************************************************/
S32 os_strcmp(const PS8 s1, const PS8 s2)
{
    return strcmp(s1,s2);
}


/************************************************************************
 *                        os_strncmp                        *
 ************************************************************************
DESCRIPTION: wrapper to the OS strcmp fucntion

CONTEXT:  
************************************************************************/
S32 os_strncmp(const PS8 s1, const PS8 s2, U32 n)
{
    return strncmp(s1,s2,n);
}

/************************************************************************
 *                        os_sscanf                     *
 ************************************************************************
DESCRIPTION: wrapper to the OS sscanf fucntion

CONTEXT:  
************************************************************************/
S32 os_sscanf(const PS8 str, const PS8 format, ...)
{
    return sscanf(str, format);
}

/************************************************************************
 *                        os_strchr                     *
 ************************************************************************
DESCRIPTION: wrapper to the OS strchr fucntion

CONTEXT:  
************************************************************************/
PS8 os_strchr(const PS8 s, S32 c)
{
    return strchr(s,c);
}

/************************************************************************
 *                        os_strtol                     *
 ************************************************************************
DESCRIPTION: wrapper to the OS strtol fucntion

CONTEXT:  
************************************************************************/
S32 os_strtol(const PS8 nptr, PPS8 endptr, S32 base)
{
    return strtol(nptr, (char**)endptr, base);
}

/************************************************************************
 *                        os_strtoul                        *
 ************************************************************************
DESCRIPTION: wrapper to the OS strtoul fucntion

CONTEXT:  
************************************************************************/
U32 os_strtoul(const PS8 nptr, PPS8 endptr, S32 base)
{
    return strtoul(nptr, (char**)endptr, base);
}

/************************************************************************
 *                        os_tolower                        *
 ************************************************************************
DESCRIPTION: wrapper to the OS tolower fucntion

CONTEXT:  
************************************************************************/
S32 os_tolower(S32 c)
{
    return tolower(c);
}

/************************************************************************
 *                        os_tolower                        *
 ************************************************************************
DESCRIPTION: wrapper to the OS tolower fucntion

CONTEXT:  
************************************************************************/
S32 os_isupper(S32 c)
{
    return isupper(c);
}

/************************************************************************
 *                        os_tolower                        *
 ************************************************************************
DESCRIPTION: wrapper to the OS tolower fucntion

CONTEXT:  
************************************************************************/
S32 os_toupper(S32 c)
{
    return toupper(c);

}

/************************************************************************
 *                        os_atoi                        *
 ************************************************************************
DESCRIPTION: wrapper to the OS atoi fucntion

CONTEXT:  
************************************************************************/
S32 os_atoi(const PS8 str)
{
	return (S32)atoi(str);
}

/************************************************************************
 *                        os_fopen                      *
 ************************************************************************
DESCRIPTION: wrapper to the OS fopen fucntion

CONTEXT:  
************************************************************************/
PVOID os_fopen(const PS8 path, os_fopen_mode_e mode)
{
    switch(mode)
    {
		case OS_FOPEN_READ:
			return fopen(path, "r");
		case OS_FOPEN_READ_BINARY:
			return fopen(path, "rb");
	    case OS_FOPEN_WRITE:
			return fopen(path, "w");
		case OS_FOPEN_WRITE_BINARY:
			return fopen(path, "wb");

		default:
		   return NULL;
    }   
}

/************************************************************************
 *                        os_getFileSize                      *
 ************************************************************************
DESCRIPTION: wrapper to the OS fopen fucntion

CONTEXT:  
************************************************************************/
S32 os_getFileSize (PVOID file)
{
    S32 size;

    if (fseek(file, 0, SEEK_END))
    {
        os_error_printf (CU_MSG_ERROR, "Cannot seek file to end\n");
        return -1;
    }
    size = ftell(file);
	fseek(file, 0, SEEK_SET);
    return size;
}

/************************************************************************
 *                        os_fgets                      *
 ************************************************************************
DESCRIPTION: wrapper to the OS fgets fucntion

CONTEXT:  
************************************************************************/
 PS8 os_fgets(PS8 s, S32 size, PVOID stream)
{
    return fgets(s, size, stream);
}

/************************************************************************
 *                        os_fread                      *
 ************************************************************************
DESCRIPTION: wrapper to the OS fread fucntion

CONTEXT:  
************************************************************************/
 S32 os_fread (PVOID ptr, S32 size, S32 nmemb, PVOID stream)
{
    return fread (ptr, size, nmemb, stream);
}

/************************************************************************
 *                        os_fwrite                      *
 ************************************************************************
DESCRIPTION: wrapper to the OS fwrite fucntion

CONTEXT:  
************************************************************************/
S32 os_fwrite (PVOID ptr, S32 size, S32 nmemb, PVOID stream)
{
    return fwrite (ptr, size, nmemb, stream);
}

/************************************************************************
 *                        os_fclose                     *
 ************************************************************************
DESCRIPTION: wrapper to the OS fclose fucntion

CONTEXT:  
************************************************************************/
 S32 os_fclose(PVOID stream)
{
    return fclose(stream);
}

/************************************************************************
 *                        os_getInputString                     *
 ************************************************************************
DESCRIPTION: get the input string for the console from the appropiate inputs

CONTEXT:  
************************************************************************/
S32 os_getInputString(PS8 inbuf, S32 len)
{
	if ( fgets(inbuf, len, stdin ) <= 0 )
		   return FALSE;

    return TRUE;
}

/************************************************************************
 *                        os_Catch_CtrlC_Signal                     *
 ************************************************************************
DESCRIPTION: register to catch the Ctrl+C signal

CONTEXT:  
************************************************************************/
VOID os_Catch_CtrlC_Signal(PVOID SignalCB)
{

}

