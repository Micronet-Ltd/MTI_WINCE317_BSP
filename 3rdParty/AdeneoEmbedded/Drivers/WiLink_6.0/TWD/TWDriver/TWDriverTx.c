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
 *   MODULE:    TNETW_Driver_Tx.c
 *
 *   PURPOSE:   TNETW_Driver Tx API functions needed externally to the driver.
 *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_120
#include "report.h"
#include "TWDriver.h"
#include "txCtrlBlk_api.h"
#include "txHwQueue_api.h"
#include "txXfer_api.h"
#include "txResult_api.h"
#include "CmdBld.h"

/** \file  TWDriverTx.c 
 *  \brief TI WLAN HW TX Access Driver
 *
 *  \see   TWDriver.h 
 */


/****************************************************************************
 *                  Tx Control Block API functions                          *
 ****************************************************************************/

TTxCtrlBlk *TWD_txCtrlBlk_Alloc (TI_HANDLE hTWD)
{
    TTwd *pTWD = (TTwd *)hTWD;

    return txCtrlBlk_Alloc (pTWD->hTxCtrlBlk);
}

void TWD_txCtrlBlk_Free (TI_HANDLE hTWD, TTxCtrlBlk *pCurrentEntry)
{
    TTwd *pTWD = (TTwd *)hTWD;

    txCtrlBlk_Free (pTWD->hTxCtrlBlk, pCurrentEntry);
}

TTxCtrlBlk *TWD_txCtrlBlk_GetPointer (TI_HANDLE hTWD, TI_UINT8 descId)
{
    TTwd *pTWD = (TTwd *)hTWD;

    return txCtrlBlk_GetPointer (pTWD->hTxCtrlBlk, descId);
}



/****************************************************************************
 *                      Tx HW Queue API functions                           *
 ****************************************************************************/
ETxHwQueStatus TWD_txHwQueue_AllocResources (TI_HANDLE hTWD, TTxCtrlBlk *pTxCtrlBlk)
{
    TTwd *pTWD = (TTwd *)hTWD;

    return txHwQueue_AllocResources (pTWD->hTxHwQueue, pTxCtrlBlk);
}

/****************************************************************************
 *                          Tx Xfer API functions                           *
 ****************************************************************************/

ETxnStatus TWD_txXfer_SendPacket (TI_HANDLE hTWD, TTxCtrlBlk *pPktCtrlBlk)
{
    TTwd *pTWD = (TTwd *)hTWD;

    return txXfer_SendPacket (pTWD->hTxXfer, pPktCtrlBlk);
}

void TWD_txXfer_EndOfBurst (TI_HANDLE hTWD)
{
    TTwd *pTWD = (TTwd *)hTWD;

    txXfer_EndOfBurst (pTWD->hTxXfer);
}




