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

/** \file  CmdBldItr.c 
 *  \brief Command builder. Interrogate commands
 *
 *  \see   CmdBld.h 
 */

#define __FILE_ID__  FILE_ID_95
#include "tidef.h"
#include "CmdBld.h"
#include "CmdBldItrIE.h"


TI_STATUS cmdBld_ItrMemoryMap (TI_HANDLE hCmdBld, MemoryMap_t *apMap, void *fCb, TI_HANDLE hCb)
{
    return cmdBld_ItrIeMemoryMap (hCmdBld, apMap, fCb, hCb);
}


TI_STATUS cmdBld_ItrRoamimgStatisitics (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void *pCb)
{
    return cmdBld_ItrIeRoamimgStatisitics (hCmdBld, fCb, hCb, pCb);
}


TI_STATUS cmdBld_ItrErrorCnt (TI_HANDLE hCmdBld, void * fCb, TI_HANDLE hCb, void * pCb)
{
    return cmdBld_ItrIeErrorCnt (hCmdBld, fCb, hCb, pCb);
}


/*
 * ----------------------------------------------------------------------------
 * Function : TWD_GetAverageRSSI
 *
 * Input    :   averageRSSI - pointer for return verage RSSI result
 *
 * Output   :   averageRSSI
 * Process  :
 * -----------------------------------------------------------------------------
 */
TI_STATUS cmdBld_ItrRSSI (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void *pCb)
{
    return cmdBld_ItrIeRSSI (hCmdBld, fCb, hCb, pCb);
}


/****************************************************************************
 *                      cmdBld_GetSoftGeminiParams()
 ****************************************************************************
 * DESCRIPTION: Get Soft Gemini config parameter
 *
 * INPUTS:
 *
 * OUTPUT:
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrSg (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void* pCb)
{                  
    return cmdBld_ItrIeSg (hCmdBld, fCb, hCb, pCb);
}

TI_STATUS cmdBld_ItrRateParams(TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void* pCb)
{
    TTwd *pTWD = (TTwd *)hCmdBld;

    return cmdBld_ItrIeRateParams (pTWD->hCmdBld, fCb, hCb, pCb);

}


TI_STATUS cmdBld_ItrStatistics (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void *pCb)
{
    return cmdBld_ItrIeStatistics (hCmdBld, fCb, hCb, pCb);
}


TI_STATUS cmdBld_ItrPowerConsumptionstat (TI_HANDLE hTWD, void *fCb, TI_HANDLE hCb, void* pCb)
{
    TTwd *pTWD = (TTwd *)hTWD;

    return  cmdBld_ItrIePowerConsumptionstat (pTWD->hCmdBld, fCb, hCb, pCb);
}

 /****************************************************************************
 *                      cmdBld_ItrDataFilterStatistics()
 ****************************************************************************
 * DESCRIPTION: Get the ACX GWSI counters 
 *
 * INPUTS:
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrDataFilterStatistics (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void *pCb)
{
    return cmdBld_ItrIeDataFilterStatistics (hCmdBld, fCb, hCb, pCb);
}

