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
/** \file smeMgr.h
 *  \brief SME interface
 *
 *  
 */

/****************************************************************************/
/*																			*/
/*		MODULE:		smeApi.h												*/
/*		PURPOSE:	SME interface to Other core modules						*/
/*																			*/
/****************************************************************************/
#ifndef __SME_API_H__
#define __SME_API_H__

#include "tidef.h"
#include "paramOut.h"
#include "mlmeApi.h"
#include "ScanCncn.h"

/* Typedefs */
typedef enum
{
	NO_MATCH = 0,
	MATCH		= 1
} match_e;

/* Prototypes */

TI_STATUS   sme_SetParam (TI_HANDLE hSme, paramInfo_t *pParam);
TI_STATUS   sme_GetParam (TI_HANDLE hSme, paramInfo_t *pParam);
void        SME_ConnectRequired (TI_HANDLE hSme);
void        SME_Disconnect (TI_HANDLE hSme);
void        sme_SetScanResultTable(TI_HANDLE hSme, TI_HANDLE hScanResultTable);
#endif /* __SME_API_H__ */
