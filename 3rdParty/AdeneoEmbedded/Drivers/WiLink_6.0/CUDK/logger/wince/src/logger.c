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
#include <stdio.h>
#include <string.h>
#include <Winsock.h>

#include "cu_osapi.h"
#include "ipc_event.h"
#include "ipc_sta.h"

#include "TWDriver.h"
#include "STADExternalIf.h"

#include "osdot11.h"
#include "cu_os.h"

#include "os_trans.h"
#include "log_conn.h"
#include "ti_ipc_api.h"

#include "console.h"
#include "cu_cmd.h"
#include "cu_common.h"


DWORD dbgReadThreadFunc(LPVOID lpParam);

unsigned char logger_welcome_message[] = {'W', 2, 0, 2, 200};

UCHAR gdata[2000];

THandle hLogConn, IpcEvent;

/* HEX DUMP for BDs !!! Debug code only !!! */
void HexDumpData (UINT8 *data, int datalen)
{
#ifdef TI_DBG
int j,dbuflen=0;
char dbuf[50];
static char hexdigits[16] = "0123456789ABCDEF";

	for(j=0; j < datalen;)
	{
		/* Add a byte to the line*/
		dbuf[dbuflen] =  hexdigits[(data[j] >> 4)&0x0f];
		dbuf[dbuflen+1] = hexdigits[data[j] & 0x0f];
		dbuf[dbuflen+2] = ' ';
		dbuf[dbuflen+3] = '\0';
		dbuflen += 3;
		j++;
		if((j % 16) == 0)
		{
			/* Dump a line every 16 hex digits*/
			os_error_printf(CU_MSG_ERROR, "%04.4x  %s\n", j-16, dbuf);
			dbuflen = 0;
		}
	}
	/* Flush if something has left in the line*/
	if(dbuflen)
		os_error_printf(CU_MSG_ERROR, "%04.4x  %s\n", j & 0xfff0, dbuf);
#endif
}


void ProcessPacket(THandle CuCmd, char *input_string, short len)
{
	ConParm_t param;
    PS8 localInput_string = input_string;
    S16 numOfModules = len - REPORT_MODULES_OFFSET - 1;

   if(IS_MESSAGE_AFTER_CONNECT(localInput_string))
    { 
        len -= MESSAGE_HEADER_SIZE;
        localInput_string += MESSAGE_HEADER_SIZE;
    } 

   if (len < REPORT_SEVERITY_MAX) return; 

    if(localInput_string[0] != MSG_PREFIX) {
        os_error_printf(CU_MSG_ERROR, "Missing prefix. prefix=%d\n", localInput_string[0]);
        return;
    }

	switch (localInput_string[1]) {
	case '2':
        param.value = (U32)&localInput_string[2];
        CuCmd_LogReportSeverityLevel(CuCmd, &param, REPORT_SEVERITY_MAX);
        if (numOfModules > 0)
        {
    		param.value = (U32)&localInput_string[REPORT_MODULES_OFFSET];
    		CuCmd_LogAddReport(CuCmd, &param, numOfModules);
        }
		break;
	case '3':
	case '4':
	case '5':
		os_error_printf(CU_MSG_ERROR, "CMD_DEBUG. code=%c(0x%X)\n", localInput_string[2], localInput_string[2]);
		break;
	default:
		os_error_printf(CU_MSG_ERROR, "Unknown debug code (0x%X)\n", localInput_string[2]);
		break;
	}
}


void user_main()
{
#ifdef ETH_SUPPORT
	TLoggerConnParams LogConnParms;
    int sres;
#endif    
	THandle CuCmd;
	BOOLEAN rc;
	
	CuCmd = CuCmd_Create("tiwlan0", NULL, TRUE, "");
	if(CuCmd == NULL) {
			os_error_printf(CU_MSG_ERROR, "%s: Failed to create object. code=%d\n", __FUNCTION__, WSAGetLastError());
			return;
	}

	rc = os_trans_create();

	if(!rc) {
		os_error_printf(CU_MSG_ERROR, "%s: os_trans_create failed code=%d\n", __FUNCTION__, WSAGetLastError());
		return;
	}

	IpcEvent = IpcEvent_Create();

	if(IpcEvent == NULL)
		return;


#ifdef ETH_SUPPORT
	hLogConn = loggerConn_Create();
	LogConnParms.port_num = LOGGER_DEFAULT_PORT;

	rc = loggerConn_init(hLogConn, 0, &LogConnParms);
	if(!rc)
		return;

	do
	{
		rc = loggerConn_waitForConn(hLogConn);
		if(rc) {

			IpcEvent_EnableEvent(IpcEvent, IPC_EVENT_LOGGER);
			loggerConn_sendMsg(hLogConn, logger_welcome_message, sizeof(logger_welcome_message));

			do {
				sres = recv((SOCKET)((LOG_CONN_DATA*)hLogConn)->data_sock, gdata, sizeof(gdata), 0);

                if(sres && (sres != SOCKET_ERROR)) {
				    ProcessPacket(CuCmd, gdata+2, sres-2);
				}

			} while(sres && (sres != SOCKET_ERROR));

		}
		else {
			os_error_printf(CU_MSG_ERROR, "%s: socket error Error %d. Disconnecting...\n", __FUNCTION__, WSAGetLastError());
		}

    }while(1);
#endif

}


void ProcessLoggerMessage(PU8 data, U32 len)
{
	if(!loggerConn_sendMsg(hLogConn, data, len)) {
		IpcEvent_DisableEvent(IpcEvent, IPC_EVENT_LOGGER);
		os_error_printf(CU_MSG_ERROR, "%s: logger disconnected. disable IPC_EVENT_LOGGER\n", __FUNCTION__);
	}
}

void g_tester_send_event(U8 event_index)
{
}

void Console_Stop(THandle hConsole)
{
}
