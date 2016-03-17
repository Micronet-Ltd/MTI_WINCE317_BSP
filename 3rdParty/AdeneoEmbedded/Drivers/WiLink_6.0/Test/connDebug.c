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
/** \file reportReplvl.c
 *  \brief Report level implementation
 *
 *  \see reportReplvl.h
 */

/***************************************************************************/
/*																									*/
/*		MODULE:	reportReplvl.c																*/
/*    PURPOSE:	Report level implementation	 										*/
/*																									*/
/***************************************************************************/
#include "tidef.h"
#include "osApi.h"
#include "paramOut.h"
#include "connDebug.h"
#include "conn.h"
#include "connApi.h"
#include "report.h"

void printConnDbgFunctions(void);

void connDebugSetParam(TI_HANDLE hConn, TI_UINT32 paramType, TI_UINT32 *value)
{
	conn_t	*pConn = (conn_t *)hConn;

	switch (paramType)
	{
	default:
		WLAN_OS_REPORT(("Invalid param type in Set Debug Connection command: %d, curr state %d\n\n", value, pConn->state));
		break;
	}
}

void connDebugGetParam(TI_HANDLE hConn, TI_UINT32 paramType, TI_UINT32 *value)
{
	conn_t	*pConn = (conn_t *)hConn;

	switch (paramType)
	{
	default:
		WLAN_OS_REPORT(("Invalid param type in Get Debug Connection command: %d, curr state %d\n\n", value, pConn->state));
		break;
	}
}

void connDebugFunction(TI_HANDLE hConn, 
					   TI_UINT32	funcType, 
					   void		*pParam)
{
	conn_t	*pConn = (conn_t *)hConn;

	switch (funcType)
	{
	case CONN_PRINT_TEST_FUNCTIONS:
		printConnDbgFunctions();
		break;

	case CONN_PRINT_TEST:
		WLAN_OS_REPORT(("Connection Print Test, param = %d , curr state %d\n\n", *((TI_UINT32 *)pParam), pConn->state));
		break;

	default:
		WLAN_OS_REPORT(("Invalid function type in Debug Connection Function Command: %d\n\n", funcType));
		break;
	}
} 

void printConnDbgFunctions(void)
{
	WLAN_OS_REPORT(("  Conn Dbg Functions  \n"));
	WLAN_OS_REPORT(("----------------------\n"));

	WLAN_OS_REPORT(("601 - Connection Print Test.\n"));
}


