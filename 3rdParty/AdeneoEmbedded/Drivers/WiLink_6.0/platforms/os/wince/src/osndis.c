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

/** \file  osndis.c 
 *  \brief implementation of ndis functions
 *
 *  \see   
 */

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>

#include "ndis.h"
#include "WlanDrvIf.h"
#include "IPCKernelApi.h"
#include "TI_IPC_API.h"
#include "osApi.h"
#include "osEntry.h"

TI_INT32 os_IndicateEvent(TI_HANDLE OsContext, IPC_EV_DATA* pData);   
VOID MiniportUnInitialize(TWlanDrvIfObjPtr pAdapter);

/** 
 * \fn     MiniportHalt
 * \brief  Deallocates resources when adapter is removed and halts the adapter.
 * 
 * \param  MiniportAdapterContext - our adapter context.
 * \return void
 */
void MiniportHalt(IN NDIS_HANDLE MiniportAdapterContext)
{
    TWlanDrvIfObjPtr        pAdapter = (TWlanDrvIfObjPtr)MiniportAdapterContext;

	PRINT(DBG_NDIS_CALLS_INFO, "TIWL: ndisMiniportHalt In\n");
    WLAN_OS_REPORT(("MiniportHalt() In\n"));
    wlanDrvIf_Stop(pAdapter);
    wlanDrvIf_Destroy(pAdapter);
    MiniportUnInitialize(pAdapter);

    WLAN_OS_REPORT(("MiniportHalt() Out\n"));
	PRINT(DBG_NDIS_CALLS_INFO, "TIWL: ndisMiniportHalt Out\n");
}

/** 
 * \fn     ndisMiniportReset
 * \brief  Resets our adapter.
 *
 * \note  This function doesn't do anything  
 * \param  pfAddressingReset - always set to TI_FALSE.
 * \param  MiniportAdapterContext - our adapter context.
 * \return  NDIS_STATUS_SUCCESS
 */
NDIS_STATUS MiniportReset(TI_BOOLEAN* pfAddressingReset, IN NDIS_HANDLE MiniportAdapterContext)
{
	PRINT(DBG_NDIS_CALLS_LOUD, "TIWL: ndisMiniportReset In\n");

    return NDIS_STATUS_SUCCESS;
}
/** 
 * \fn     MiniportShutdown
 * \brief
 *
 * \note  This function doesn't do anything  
 * \param  MiniportAdapterContext - our adapter context.
 * \return  void
 */
void MiniportShutdown(IN NDIS_HANDLE MiniportAdapterContext)
{
}

/*-----------------------------------------------------------------------------

Routine Name:

	ndisMiniportISR


	MiniportAdapterContext - our adapter context.

Return Value:

	None

-----------------------------------------------------------------------------*/

VOID MiniportISR(
	PBOOLEAN pfInterruptRecognized,
	PBOOLEAN pfQueueDPC,
	NDIS_HANDLE MiniportAdapterContext
	)
{
	TWlanDrvIfObjPtr pTWlanDrvIf = (TWlanDrvIfObjPtr) MiniportAdapterContext;

	PRINT(DBG_INTERRUPT_LOUD, "TIWL: ndisMiniportISR In\n");

    *pfInterruptRecognized = TRUE;
    *pfQueueDPC            = FALSE;
    TWD_InterruptRequest (pTWlanDrvIf->tCommon.hTWD);

	PRINT(DBG_INTERRUPT_LOUD, "TIWL: ndisMiniportISR Out\n");

} /* MiniportISR() */

