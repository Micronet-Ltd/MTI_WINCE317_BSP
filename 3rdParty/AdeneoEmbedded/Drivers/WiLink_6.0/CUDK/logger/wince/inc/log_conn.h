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
#ifndef _LOG_CONN_H
#define _LOG_CONN_H

#include "cu_osapi.h"

#define REPORT_MODULES_OFFSET           17
#define SEVERITY_MODULES_TABLE_OFFSET   2
#define LOGGER_DEFAULT_PORT		        700
#define MESSAGE_HEADER_SIZE             6
#define MSG_PREFIX                      0x34

#define IS_MESSAGE_AFTER_CONNECT(X)         (X[0] == 0x00 && \
                                             X[1] == 0x35 && \
                                             X[2] == 0x31 && \
                                             X[3] == 0xff)

typedef struct {
	THandle listen_sock;
	THandle data_sock;
	U32	connections;
} LOG_CONN_DATA, *PLOG_CONN_DATA;

typedef struct {
	U16 port_num;
} TLoggerConnParams, *PTLoggerConnParams;

typedef enum {
	LOG_CONN_MEDIA_SERIAL,
	LOG_CONN_MEDIA_ETH
} ELoggerConnMedia;

THandle loggerConn_Create();

BOOLEAN loggerConn_init (THandle loggerConn, 
						 ELoggerConnMedia mediaType,		/*currently unused */
						 TLoggerConnParams* mediaParams
						 );

BOOLEAN loggerConn_waitForConn (THandle loggerConn);
BOOLEAN loggerConn_sendMsg (THandle loggerConn, PS8 buffer, U32 bufferSize);
VOID loggerConn_destroy (THandle loggerConn);

#endif