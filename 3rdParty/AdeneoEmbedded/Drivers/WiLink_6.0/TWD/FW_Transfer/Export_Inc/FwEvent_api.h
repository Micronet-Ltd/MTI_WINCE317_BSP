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

/** \file FwEvent_api.h
 *  \brief FwEvent api
 *
 *  \see FwEvent.c
 */


#ifndef _FW_EVENT_API_H
#define _FW_EVENT_API_H

/* Public Function Definitions */

/*
 * \brief	Create the FwEvent module object
 * 
 * \param  hOs  - OS module object handle
 * \return Handle to the created object
 * 
 * \par Description
 * Calling this function creates a FwEvent object
 * 
 * \sa fwEvent_Destroy
 */
TI_HANDLE       fwEvent_Create              (TI_HANDLE hOs);


/*
 * \brief	Destroys the FwEvent object
 * 
 * \param  hFwEvent  - The object to free
 * \return TI_OK
 * 
 * \par Description
 * Calling this function destroys a FwEvent object
 * 
 * \sa fwEvent_Create
 */
TI_STATUS       fwEvent_Destroy             (TI_HANDLE hFwEvent);


/*
 * \brief	Requests the context engine to schedule the driver task
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \return void
 * 
 * \par Description
 * Called by the FW-Interrupt ISR.
 * Requests the context engine to schedule the driver task 
 * for handling the FW-Events (FwEvent callback).
 * 
 * \sa
 */
void            fwEvent_InterruptRequest    (TI_HANDLE hFwEvent);


/*
 * \brief	Config the FwEvent module object
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \param  hTWD  - Handle to TWD module
 * \return TI_OK
 * 
 * \par Description
 * From hTWD we extract : hOs, hReport, hTwIf, hContext,
 *      hHealthMonitor, hEventMbox, hCmdMbox, hRxXfer, 
 *      hTxHwQueue, hTxResult
 * In this function we also register the FwEvent to the context engine
 * 
 * \sa
 */
TI_STATUS       fwEvent_Init                (TI_HANDLE hFwEvent, TI_HANDLE hTWD);


/*
 * \brief	Called by any handler that completed after pending
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 *
 * \par Description
 *
 * Decrement pending handlers counter and if 0 call the SM to complete its process.
 * 
 * \sa
 */
void            fwEvent_HandlerCompleted    (TI_HANDLE hFwEvent);


/*
 * \brief	Stop & reset FwEvent (called by the driver stop process)
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \return TI_OK
 * 
 * \par Description
 *
 * \sa
 */
TI_STATUS       fwEvent_Stop                (TI_HANDLE hFwEvent);


/*
 * \brief	Translate host to FW time (Usec)
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \param  uHostTime - The host time in MS to translate
 *
 * \return FW Time in Usec
 * 
 * \par Description
 * 
 * \sa
 */
TI_UINT32       fwEvent_TranslateToFwTime (TI_HANDLE hFwEvent, TI_UINT32 uHostTime);



/*
 * \brief	Mask all interrupts
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \return void
 * 
 * \par Description
 *
 * Masks all interrupts from FW.
 * 
 * \sa
 */
void fwEvent_MaskAllFwInterrupts(TI_HANDLE hFwEvent);


/*
 * \brief	Disable the FwEvent client of the context thread handler
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \return void
 * 
 * \par Description
 *
 * \sa
 */
void            fwEvent_DisableInterrupts   (TI_HANDLE hFwEvent);


/*
 * \brief	Enable the FwEvent client of the context thread handler
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \return void
 * 
 * \par Description
 *
 * \sa
 */
void            fwEvent_EnableInterrupts    (TI_HANDLE hFwEvent);


/*
 * \brief	Unmask all interrupts, set Rx interrupt bit and call FwEvent_Handle
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \return void
 * 
 * \par Description
 * Called when driver Start or recovery process is completed.
 * Unmask all interrupts, set Rx interrupt bit and call FwEvent_Handle 
 * (in case we missed an Rx interrupt in a recovery process).
 *
 * \sa
 */
void            fwEvent_EnableExternalEvents(TI_HANDLE hFwEvent);


/*
 * \brief	Unmask only cmd-cmplt and events interrupts (needed for init phase)
 * 
 * \param  hFwEvent  - FwEvent Driver handle
 * \return Event mask
 * 
 * \par Description
 * Unmask only cmd-cmplt and events interrupts (needed for init phase).

 *
 * \sa
 */
void            fwEvent_SetInitMask         (TI_HANDLE hFwEvent);



#ifdef TI_DBG

void            fwEvent_PrintStat           (TI_HANDLE hFwEvent);

#endif  /* TI_DBG */



#endif /* _FW_EVENT_API_H */







