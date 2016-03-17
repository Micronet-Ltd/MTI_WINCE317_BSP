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

#ifndef __RSN_DBG_H__
#define __RSN_DBG_H__

/* definitions for ctrl dbg */

#define DBG_RSN_PRINT_HELP				0

#define DBG_RSN_SET_DEF_KEY_ID			2
#define DBG_RSN_SET_DESIRED_AUTH		3
#define DBG_RSN_SET_DESIRED_CIPHER		4

#define DBG_RSN_GEN_MIC_FAILURE_REPORT	6
#define DBG_RSN_GET_PARAM_802_11_CAPABILITY  7
#define DBG_RSN_GET_PMKID_CACHE              8
#define DBG_RSN_RESET_PMKID_CACHE            9
#define DBG_RSN_PRINT_ROGUE_AP_TABLE         10
#define DBG_RSN_SET_PORT_STATUS              11
#define DBG_RSN_PRINT_PORT_STATUS            12

void rsnDebugFunction(TI_HANDLE hRsn, TI_UINT32 funcType, void *pParam);


#endif /* __RSN_DBG_H__*/

