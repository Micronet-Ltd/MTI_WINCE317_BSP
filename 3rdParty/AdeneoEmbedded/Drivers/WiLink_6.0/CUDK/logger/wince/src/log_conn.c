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
#include <winsock.h>
#include "cu_osapi.h"
#include "ostitype.h"
#include "os_trans.h"
#include "log_conn.h"


static LOG_CONN_DATA LogConn;

THandle loggerConn_Create()
{
	LogConn.listen_sock = NULL;
	LogConn.data_sock = NULL;
	LogConn.connections = 0;

	return (THandle) &LogConn;
}


BOOLEAN loggerConn_init (THandle loggerConn, 
						 ELoggerConnMedia mediaType,		/*currently unused */
						 TLoggerConnParams* mediaParams
						 )
{
	PLOG_CONN_DATA LogConn = (PLOG_CONN_DATA) loggerConn;
	BOOLEAN rc;

	rc = os_socket(&LogConn->listen_sock);
	if(!rc) {
		return(FALSE);
	}

	LogConn->data_sock = NULL;

	rc = os_bind(LogConn->listen_sock, mediaParams->port_num);
	if(!rc) {
		return(FALSE);
	}

	return TRUE;
}


BOOLEAN loggerConn_waitForConn (THandle loggerConn)
{
	PLOG_CONN_DATA LogConn = (PLOG_CONN_DATA) loggerConn;
	BOOLEAN rc;
	
	rc = os_sockWaitForConnection(LogConn->listen_sock, &LogConn->data_sock);

	if(!rc)
		return FALSE;

	LogConn->connections++;
	return TRUE;

}


BOOLEAN loggerConn_sendMsg (THandle loggerConn, PS8 buffer, U32 bufferSize)
{
	PLOG_CONN_DATA LogConn = (PLOG_CONN_DATA) loggerConn;

	if(LogConn->data_sock == NULL)
		return TRUE;

	return os_sockSend(LogConn->data_sock, buffer, bufferSize);
}


VOID loggerConn_destroy (THandle loggerConn)
{
	PLOG_CONN_DATA LogConn = (PLOG_CONN_DATA) loggerConn;

	LogConn->listen_sock = NULL;
	LogConn->data_sock = NULL;

	LogConn->connections = 0;
}