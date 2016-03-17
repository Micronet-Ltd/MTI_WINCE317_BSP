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
#include <windows.h>
#include <winsock.h>

#include "cu_osapi.h"


BOOLEAN os_trans_create()
{
	WORD wVersionRequested;
    WSADATA wsaData;
	int res;

	wVersionRequested = MAKEWORD( 2, 2 );
     
    res = WSAStartup( wVersionRequested, &wsaData );
    if ( res != 0 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return FALSE;
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
		return FALSE;
    }

	return TRUE;
}


BOOLEAN os_socket (THandle* pSock)
{
	SOCKET socket_id;
	BOOL optval = TRUE;

	socket_id = socket(PF_INET, SOCK_STREAM, 0);

    if (!socket_id) {
		/* Error opening socket */
        os_error_printf(CU_MSG_ERROR, "%s: error opening socket.\n", __FUNCTION__);
        return (FALSE);
	}

	/*************************/
	/* Configure the socket */
	/***********************/

    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval)) == -1) {
		/* Error setting socket option */
        os_error_printf(CU_MSG_ERROR, "%s: error setting socket option. Error %d\n", __FUNCTION__, WSAGetLastError());
        closesocket(socket_id);
        return(FALSE);
    }
	
	*pSock = (THandle) socket_id;

	return TRUE;
}


BOOLEAN os_bind (THandle sock, U16 port)
{
	struct sockaddr_in server_addr;
	int result;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	server_addr.sin_port = htons(port);

	result = bind((SOCKET)sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (result != 0) {
		/* Error binding socket */
        os_error_printf(CU_MSG_ERROR, "%s: error binding socket. Error %d\n", __FUNCTION__, WSAGetLastError());
        closesocket((SOCKET)sock);
        return(FALSE);
	}

	return TRUE;
}


BOOLEAN os_sockWaitForConnection (THandle socket_id, THandle* pConnSock)
{
	struct sockaddr_in client_addr;
	int client_addr_len;
	int result;

	result = listen((SOCKET)socket_id, 5);

    if (result == -1) {
		/* Error listening to socket */
/*        os_error_printf(CU_MSG_ERROR, "%s: error listening to socket. Error %d\n", __FUNCTION__, WSAGetLastError());
		*pConnSock = NULL;
        closesocket((SOCKET)socket_id);
        return(FALSE);*/
	}

	/**********************/
	/* Accept connection */
	/********************/
	client_addr_len = sizeof(client_addr);

	/* We suppose to get new socket id after accept (blocking action) */
	result = (int)accept((SOCKET)socket_id, (struct sockaddr *)&client_addr, &client_addr_len);

    if (result == -1) {			
		*pConnSock = NULL;
        /* Error accepting connection */
        os_error_printf(CU_MSG_ERROR, "%s: error accepting connection. Error %d\n", __FUNCTION__, WSAGetLastError());
        closesocket((SOCKET)socket_id);
        return(FALSE);
    }

	*pConnSock = (THandle) result;

	return TRUE;
}


BOOLEAN os_sockSend (THandle socket_id, PS8 buffer, U32 bufferSize)
{
	int result;

	/* Write to the socket */
	result = send((SOCKET)socket_id, buffer, bufferSize, 0);

	if (result != bufferSize) {

		/**************************/
		/* Error writing to port */
		/************************/
		os_error_printf(CU_MSG_ERROR, "%s: Error writing to socket (result = %d), error %d\n", 
                                __FUNCTION__, result, WSAGetLastError());

		return FALSE;
	}

	return TRUE;
}

VOID os_trans_destroy()
{
	WSACleanup();
}