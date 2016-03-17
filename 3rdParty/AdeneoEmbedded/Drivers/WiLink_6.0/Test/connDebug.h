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
#ifndef __CONN_DEBUG_H__
#define __CONN_DEBUG_H__

#include "tidef.h"
#include "paramOut.h"

#define CONN_PRINT_TEST_FUNCTIONS	0
#define	CONN_PRINT_TEST				1

void connDebugSetParam(TI_HANDLE hConn, TI_UINT32 paramType, TI_UINT32 *value);
void connDebugGetParam(TI_HANDLE hConn, TI_UINT32 paramType, TI_UINT32 *value);
void connDebugFunction(TI_HANDLE hConn, 
					   TI_UINT32	funcType, 
					   void		*pParam);

#endif /* __CONN_DEBUG_H__ */

