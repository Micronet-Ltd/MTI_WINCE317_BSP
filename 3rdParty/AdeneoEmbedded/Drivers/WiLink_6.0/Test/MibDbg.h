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

#ifndef __MIB_DBG_H__
#define __MIB_DBG_H__

#include "osTIType.h"
#include "tidef.h"

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_MIB_PRINT_HELP						0
#define DBG_MIB_GET_ARP_TABLE					1
#define DBG_MIB_GET_GROUP_ADDRESS_TABLE	   		2
#define DBG_MIB_GET_COUNTER_TABLE	       		3
#define DBG_MIB_MODIFY_CTS_TO_SELF 	        	4
#define DBG_MIB_GET_CTS_TO_SELF 	        	5
#define DBG_MIB_SET_MAX_RX_LIFE_TIME      		6



/*
 ***********************************************************************
 *	External functions definitions
 ***********************************************************************
 */
void MibDebugFunction(TI_HANDLE hTWD ,TI_UINT32 funcType, void* pParam);


#endif	/* #define __MIB_DBG_H__ */
