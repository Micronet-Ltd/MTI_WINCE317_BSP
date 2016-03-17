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

/** \file  ipc_k.c 
 *  \brief 
 *
 *  \see   
 */

#include <ndis.h>
#include <ntddndis.h>
#include <TI_IPC_Api.h>
#include "WlanDrvIf.h"

/** 
 * \fn     IPCKernelInit
 * \brief  
 * 
 * \param  hAdapter
 * \param  hIPCEv
 * \return TI_UINT32
 */
TI_UINT32 IPCKernelInit(HANDLE hAdapter, void* hIPCEv)
{
	TWlanDrvIfObjPtr pAdapter;

	pAdapter = (TWlanDrvIfObjPtr)hAdapter;

    PRINT(DBG_INIT_LOUD, "IPC: IPCKernelInit \n");

    pAdapter->IPC.pRxIPCEvent=*(HANDLE*)hIPCEv;
	pAdapter->IPC.IPCEventCounter++;

    PRINTF(DBG_INIT_LOUD,("JSR: TICCXWLN:IPC_K.C: Init Event Handle: %d",
            pAdapter->IPC.pRxIPCEvent ));
    PRINT(DBG_INIT_LOUD, "IPC: IPCKernelInit Out \n");

    return TI_OK;
}


/** 
 * \fn     IPCKernelDeInit
 * \brief  
 * 
 * \param  hAdapter
 * \return TI_UINT32
 */
TI_UINT32 IPCKernelDeInit(HANDLE hAdapter)
{
	TWlanDrvIfObjPtr pAdapter;

    pAdapter = (TWlanDrvIfObjPtr)hAdapter;

    PRINTF(DBG_INIT_LOUD,("JSR: TICCXWLN:IPC_K.C: De Init Event Handle: %d",
            pAdapter->IPC.pRxIPCEvent ));

    if (pAdapter->IPC.pRxIPCEvent == NULL) 
	{
		PRINT(DBG_INIT_LOUD, "IPC: IPCKernel Already deinitialized! \n");
		return TI_OK;
	}

	pAdapter->IPC.IPCEventCounter --;

	if(pAdapter->IPC.IPCEventCounter == 0)
	{
		pAdapter->IPC.pRxIPCEvent = NULL;
	}

    PRINT(DBG_INIT_LOUD, "IPC: IPCKernelDeInit Out \n");

	return TI_OK;
}


/** 
 * \fn     IPC_EventSend
 * \brief  Generic Event Send
 * 
 * \param  hAdapter - handle to the adpter
 * \param  pEvData - event to send
 * \param  EvDataSize - event buffer size
 * \return TI_UINT32
 */
TI_INT32 IPC_EventSend (HANDLE hAdapter, TI_UINT8 *pEvData, TI_UINT32 EvDataSize)
{
    TWlanDrvIfObjPtr        pAdapter;
    HANDLE temp_Handle;

    pAdapter = (TWlanDrvIfObjPtr)hAdapter;

    if(pEvData == NULL)
    {
        if(	pAdapter->IPC.IPCEventCounter == 0)
        {
            PRINT(DBG_INIT_LOUD,"JSR:EVENT:: PC.pRxIPCEvent =NULL");
            return TI_NOK;
        }

         PRINTF(DBG_INIT_LOUD,("JSR:EVENT: TICCXWLN:IPC_K.C:Set Event, Event size: %d, Event Handle: %d",
                EvDataSize,pAdapter->IPC.pRxIPCEvent ));

        temp_Handle= CreateEvent(NULL, FALSE, FALSE, IPC_RX_EVENT);

        if (SetEvent(temp_Handle)==0)
        {
             RETAILMSG(1,(TEXT("JSR:EVENT:: Set Event Failed......Helpppppppp!!!!! = %d, %u \n "),
                 GetLastError(),pAdapter->IPC.pRxIPCEvent ));
        }

    }
    else
    {
        PRINTF(DBG_INIT_LOUD,("JSR: TICCXWLN:Else calling callback Event size: %d Event Handle: %d",
            EvDataSize, pAdapter->IPC.pRxIPCEvent));

        ((IPC_EV_DATA*)pEvData)->EvParams.pfEventCallback((IPC_EV_DATA*)pEvData);
    }

    return TI_OK;
}
