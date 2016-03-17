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

/* \file FW_debug.h
 *  \brief This file include private definitions for the FW debug module.
 */

#ifndef __FW_DEBUG_H__
#define __FW_DEBUG_H__


#include "tidef.h"
#include "public_infoele.h"
#include "TWDriver.h"
#include "TWDriverInternal.h"

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_FW_PRINT_HELP					 	0
#define DBG_FW_SEND_GENERAL_TEST_CMD	    	1
#define DBG_FW_IBSS_CONNECTION	        		2
#define DBG_FW_SEND_MGMT_PACKET                	3
#define DBG_FW_SEND_DATA_PACKET	                4
#define DBG_FW_START_LOOPBACK      				5
#define DBG_FW_STOP_LOOPBACK                    6
#define DBG_FW_INFINIT_SEND	                    7
#define DBG_FW_GENERAL		                    10

/*
 ***********************************************************************
 *	Enums.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	Typedefs.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	Structure definitions.
 ***********************************************************************
 */
 typedef struct {
 INFO_ELE_HDR  
 unsigned char len;
 unsigned long buf[20];
 }FWDebugTestCmdParamter_t;

 typedef struct {
 INFO_ELE_HDR  
 TTestCmd     Plt; 
 }FWDebugPLT_t;

/*
 ***********************************************************************
 *	External data definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	External functions definitions
 ***********************************************************************
 */

void FWDebugFunction(TI_HANDLE hDrvMain, 
					 TI_HANDLE hOs, 
					 TI_HANDLE hTWD, 
					 TI_HANDLE hMlme, 
					 TI_HANDLE hTxMgmtQ,
					 TI_HANDLE hTxCtrl,
					 unsigned long funcType, 
					 void *pParam 
					 /*yael unsigned long packetNum*/);
void FW_debugSendGeneralTestCmd(TI_HANDLE hTWD, void *pParam);
void FW_DebugSendJoinCommand(TI_HANDLE hTWD, TI_HANDLE hTxMgmtQ);
void FW_DebugSendPacket(TI_HANDLE hDrvMain ,TI_HANDLE hOs, TI_HANDLE hTxMgmtQ, void* pParam);
void FW_DebugInfinitSendPacket(TI_HANDLE hDrvMain ,TI_HANDLE hOs);
void FW_DebugStartLoopBack (TI_HANDLE hDrvMain, TI_HANDLE hTWD);
void FW_DebugStopLoopBack (TI_HANDLE hDrvMain, TI_HANDLE hTWD);
void FW_ComparePacket (char *buf);
void printFWDbgFunctions(void);
void FW_DebugGeneral(TI_HANDLE hTWD, void *pParam);



#endif 

