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

/****************************************************************************
 *
 *   MODULE:  fwDebug_api.h
 *
 *   PURPOSE: FW-Debug module API.
 * 
 ****************************************************************************/

#ifndef _FW_DEBUG_API_H
#define _FW_DEBUG_API_H



#define FW_DEBUG_MAX_BUF		256
#define FW_DBG_CMD_MAX_PARAMS	2


/* FW Debug commands. */
typedef enum
{
	FW_DBG_CMD_READ_MEM,
	FW_DBG_CMD_WRITE_MEM
}fwDbg_dbgCmd_e;

typedef struct
{
	TI_UINT32	addr;
	TI_UINT32 	length;
	union
	{
		TI_UINT8	buf8[FW_DEBUG_MAX_BUF];
		TI_UINT32	buf32[FW_DEBUG_MAX_BUF/4];
	} UBuf;
}TFwDebugParams;

/* for TWD Debug */
typedef struct
{
	TI_UINT32		func_id;
	union
	{
		TI_UINT32		opt_param;
		TFwDebugParams	mem_debug;
	}debug_data;
} TTwdDebug;

typedef void(*TFwDubCallback)(TI_HANDLE hCb);


/* Public Function Definitions */


TI_HANDLE fwDbg_Create      (TI_HANDLE hOs);

void      fwDbg_Destroy     (TI_HANDLE hFwDebug);

void      fwDbg_Init	    (TI_HANDLE hFwDebug,
                             TI_HANDLE hReport,
                             TI_HANDLE hTwif,
                             TI_HANDLE hFwEvent);

TI_STATUS fwDbg_WriteAddr   (TI_HANDLE hFwDebug,
                             TI_UINT32 Address,
                             TI_UINT32 Length,
                             TI_UINT8* Buffer,
                             TFwDubCallback fCb,
                             TI_HANDLE hCb);

TI_STATUS fwDbg_ReadAddr    (TI_HANDLE hFwDebug,
                             TI_UINT32 Address,
                             TI_UINT32 Length,
                             TI_UINT8* Buffer,
                             TFwDubCallback fCb,
						     TI_HANDLE hCb);

TI_BOOL fwDbg_isValidMemoryAddr    (TI_HANDLE hFwDebug,
                             TI_UINT32 Address,
                             TI_UINT32 Length);

TI_BOOL fwDbg_isValidRegAddr    (TI_HANDLE hFwDebug,
                             TI_UINT32 Address,
                             TI_UINT32 Length);


TI_STATUS fwDbg_ValidateSdio(TI_HANDLE hFwDebug, TI_UINT32 uNumOfLoops, TI_UINT32 uTxnSize);


#endif /* _FW_DEBUG_API_H */




