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


/** \file   WlanDrvCommon.h 
 *  \brief  Defines WlanDrvIf objects common to all OS types.                                  
 *
 *  \see    WlanDrvIf.h
 */

#ifndef __WLAN_DRV_COMMON_H__
#define __WLAN_DRV_COMMON_H__


#include "tidef.h"
#include "TWDriver.h"

#define DRV_ADDRESS_SIZE					(sizeof(TI_INT32))
#define MAX_CHUNKS_IN_FILE					(1000)
#define OS_SPECIFIC_RAM_ALLOC_LIMIT			(0xFFFFFFFF)	/* assume OS never reach that limit */

/* Driver steady states - for driver external users */
typedef enum 
{
    DRV_STATE_IDLE,
    DRV_STATE_RUNNING,
    DRV_STATE_STOPING,
    DRV_STATE_STOPPED,
    DRV_STATE_FAILED,
    DRV_STATE_SUSPENDING,
    DRV_STATE_SUSPENDED	
} EDriverSteadyState;


/* The driver Start/Stop actions */
typedef enum
{
    ACTION_TYPE_NONE, 
    ACTION_TYPE_START, 
    ACTION_TYPE_STOP,
    ACTION_TYPE_SET_ACTIVE_MODE_RX_FILTER,
    ACTION_TYPE_SET_SUSPEND_MODE_RX_FILTER,
} EActionType;

/* Initialization file info */
typedef struct 
{
    void            *pImage;
    unsigned long    uSize;
} TInitImageInfo;

/* WlanDrvIf object common part (included by TWlanDrvIfObj from each OS abstraction layer) */
typedef struct 
{
    /* Other modules handles */
    void               *hDrvMain;
    void               *hCmdHndlr;
    void               *hContext;
    void               *hTxDataQ;
    void               *hTxMgmtQ;
    void               *hTxCtrl;
    void               *hTWD;
    void               *hEvHandler;
    void               *hReport;
    void               *hCmdDispatch;

    /* Initialization files info */
    TInitImageInfo      tIniFile;
    TInitImageInfo      tNvsImage;
    TInitImageInfo      tFwImage;

    EDriverSteadyState  eDriverState;   /* The driver state as presented to the OS */
    TI_UINT32           uLinkSpeed;

} TWlanDrvIfCommon;


/* The loader files interface */
typedef struct
{
  TI_UINT32 uNvsFileLength;
  TI_UINT32 uFwFileLength;
  TI_UINT32 uIniFileLength;
  char data[1];
  /* eeprom image follows   */
  /* firmware image follows */
  /* init file follows      */
} TLoaderFilesData;



#endif /* __WLAN_DRV_COMMON_H__ */
