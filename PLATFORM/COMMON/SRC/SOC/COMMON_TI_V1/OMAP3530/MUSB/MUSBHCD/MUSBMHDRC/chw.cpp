// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Module Name:
//     CHW.cpp
// Abstract:
//     This file implements the MUSBMHDRC specific register routines
//
// Notes:
//
//

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <pm.h>
#include <omap3530.h>
#include <oal_io.h>
#include <initguid.h>
#define HCD_SUSPEND_RESUME 1 // For test only
#undef ZONE_INIT
#undef ZONE_WARNING
#undef ZONE_ERROR
#include <Uhcdddsi.h>
#include <globals.hpp>
#include <ctd.h>
#include <chw.h>
#include <cmhcd.h>
#include <omap3530_musbotg.h>
#include <omap3530_musbhcd.h>
#include <omap3530_musbcore.h>
#include <omap_musbcore.h>
#pragma warning(pop)

#undef ZONE_DEBUG
#define ZONE_DEBUG 0

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

extern "C" DWORD Host_ProcessEPxRx(PVOID pHSMUSBContext, DWORD endpoint);
extern "C" BOOL HcdPdd_PreTransferActivation(SMHCDPdd *pPdd);
extern "C" void HcdPdd_PostTransferDeactivation(SMHCDPdd *pPdd);


//BOOL    gbToggle;
// ******************************** CDummyPipe **********************************               
const USB_ENDPOINT_DESCRIPTOR dummpDesc = {
    sizeof(USB_ENDPOINT_DESCRIPTOR),USB_ENDPOINT_DESCRIPTOR_TYPE, 0xff,  USB_ENDPOINT_TYPE_INTERRUPT,8,1
};
//*******************************************************************************
CDummyPipe::CDummyPipe(IN CPhysMem * const pCPhysMem)
: CPipe( &dummpDesc,FALSE,TRUE,0xff,0xff,0xff,NULL,NULL)
, m_pCPhysMem(pCPhysMem)
//
//  Purpose:    Constructor for CDummyPipe class. This class is created at the 
//              begining of loading the driver.
//
//  Parameter:  pChysMem - Pointer to CPhysMem object
//              
//
//  Return:     Nil
//********************************************************************************
{
    ASSERT( m_pCPhysMem!=NULL);
    m_bFrameSMask = 0xff;
    m_bFrameCMask = 0;

};


// ******************************BusyPipeList***************************************
//  CBusyPipeList class is one which store the list of pipe that are currently
//  opened and being used.
//**********************************************************************************
//**********************************************************************************
BOOL  CBusyPipeList::Init()
//
//  Purpose: To initialize the CBusyPipeList class. It generates a thread which keep
//           wait for the signal of any pipe being processed.
//
//  Parameter:
//
//  Return: TRUE - success, FALSE - failure
//**********************************************************************************
{
    int i;
    DEBUGMSG(ZONE_PIPE, (TEXT("CBusyPipeList::Init+\r\n")));
    m_fCheckTransferThreadClosing=FALSE;     
    m_pBusyPipeList = NULL;
#ifdef DEBUG
    m_debug_numItemsOnBusyPipeList = 0;
#endif
    // In order to identify which endpoint , we need to find out which endpoint is receiving
    // intr.

    // Create endpoint update event
    m_hUpdateEPEvent = CreateEvent( NULL, FALSE, FALSE, NULL );    
    if ( m_hUpdateEPEvent == NULL ) {    
        DEBUGMSG(ZONE_ERROR, (TEXT("-CPipe::Initialize. Error creating process done transfers event\n")));            
        return FALSE;
    }
  
    // set up our thread to check for done transfers
    // currently, the context passed to CheckForDoneTransfersThread is ignored
    m_hCheckForDoneTransfersThread = CreateThread( 0, 0, CheckForDoneTransfersThreadStub, (PVOID)this, 0, NULL );
    if ( m_hCheckForDoneTransfersThread == NULL ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CPipe::Initialize. Error creating process done transfers thread\n")));
        return FALSE;
    }
    CeSetThreadPriority( m_hCheckForDoneTransfersThread, g_IstThreadPriority + RELATIVE_PRIO_CHECKDONE );

    m_hEP0CheckForDoneTransfersEvent = NULL;            
    m_PipeListInfoEP0CheckForDoneTransfersCount = NULL;
    
    for (i = 0; i < HOST_MAX_EPNUM; i++)
    {
        m_hEP2Handles[i][DIR_IN] = NULL;
        m_hEP2Handles[i][DIR_OUT] = NULL;
    }

    for (i = 0; i < DMA_MAX_CHANNEL; i++)
    {
        m_hDMA2Handles[i] = NULL;
    }

    m_SignalDisconnectACK = TRUE;
    return TRUE;

}
void CBusyPipeList::DeInit()
{
    m_fCheckTransferThreadClosing=TRUE;  
    // We just check Update EP
    if ( m_hUpdateEPEvent != NULL ) {
       SetEvent(m_hUpdateEPEvent);
       if ( m_hCheckForDoneTransfersThread ) {
           DWORD dwWaitReturn = WaitForSingleObject( m_hCheckForDoneTransfersThread, 5000 );
           if ( dwWaitReturn != WAIT_OBJECT_0 ) {
                DEBUGCHK( 0 ); // check why thread is blocked
#pragma prefast(suppress: 258, "Try to recover gracefully from a pathological failure")
                TerminateThread( m_hCheckForDoneTransfersThread, DWORD(-1) );
           }
           CloseHandle( m_hCheckForDoneTransfersThread );
           m_hCheckForDoneTransfersThread = NULL;
        }
        CloseHandle( m_hUpdateEPEvent);
        m_hUpdateEPEvent = NULL;
    }

    if (m_hEP0CheckForDoneTransfersEvent)
    {
        CloseHandle(m_hEP0CheckForDoneTransfersEvent);
        m_hEP0CheckForDoneTransfersEvent = NULL;            
        m_PipeListInfoEP0CheckForDoneTransfersCount = NULL;
    }        
}

// ******************************************************************
// Scope: public static
BOOL CBusyPipeList::SignalCheckForDoneDMA(IN const UCHAR channel)
//
// Purpose: This function is called when an DMA interrupt is received by
//          the CHW class. We then signal the CheckForDoneTransfersThread
//          to check for any transfers which have completed
//
// Parameters:  channel - channel no.
//
// Returns: TRUE - valid DMA interrupt,  FALSE - invalid DMA interrupt
//
// Notes: DO MINIMAL WORK HERE!! Most processing should be handled by
//        The CheckForDoneTransfersThread. If this procedure blocks, it
//        will adversely affect the interrupt processing thread.
// ******************************************************************
{    
    if (m_hDMA2Handles[channel])
    {        
        DEBUGMSG(ZONE_PIPE, (TEXT("Release DMA for channel %d\r\n"), channel));
        DEBUGCHK( m_hDMA2Handles[channel] && m_hCheckForDoneTransfersThread );
        DEBUGMSG(ZONE_PIPE, (TEXT("CBusyPipeList::SignalCheckForDoneDMA at channel %d\r\n"), channel));
        SetEvent( m_hDMA2Handles[channel]);
        return TRUE;
    }
    return FALSE;
}


// ******************************************************************
// Scope: public static
void CBusyPipeList::SignalCheckForDoneTransfers(IN const UCHAR endpoint, IN const UCHAR ucIsOut)
//
// Purpose: This function is called when an interrupt is received by
//          the CHW class. We then signal the CheckForDoneTransfersThread
//          to check for any transfers which have completed
//
// Parameters:  endpoint - endpoint to signal
//              ucIsOut - OUT endpoint or not
//
// Returns: Nothing
//
// Notes: DO MINIMAL WORK HERE!! Most processing should be handled by
//        The CheckForDoneTransfersThread. If this procedure blocks, it
//        will adversely affect the interrupt processing thread.
// ******************************************************************
{    
    if (m_hEP2Handles[USB_ENDPOINT(endpoint)][ucIsOut])
    {       
        DEBUGMSG(ZONE_PIPE, (TEXT("CBusyPipeList::SignalCheckForDoneTransfer at ep %d OUT %d\r\n"), endpoint, ucIsOut));
        SetEvent( m_hEP2Handles[USB_ENDPOINT(endpoint)][ucIsOut]);
    }
}
// ******************************************************************
ULONG CALLBACK CBusyPipeList::CheckForDoneTransfersThreadStub( IN PVOID pContext)
{
    return ((CBusyPipeList *)pContext)->CheckForDoneTransfersThread( );
}
// Scope: private static
ULONG CBusyPipeList::CheckForDoneTransfersThread( )
//
// Purpose: Thread for checking whether busy pipes are done their
//          transfers. This thread should be activated whenever we
//          get a USB transfer complete interrupt (this can be
//          requested by the InterruptOnComplete field of the TD)
//
// Parameters: 32 bit pointer passed when instantiating thread (ignored)
//                       
// Returns: 0 on thread exit
//
// Notes: 
// ******************************************************************
{
    DWORD timeout = INFINITE;
    DWORD dwErr = 0;
    EVENT_INFO EventInfoObj[HOST_MAX_EP+1+DMA_MAX_CHANNEL];
    HANDLE hWaitHandle[HOST_MAX_EP+2+DMA_MAX_CHANNEL];
    DWORD hCount;    

    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("+CPipe::CheckForDoneTransfersThread\n")) );
    
    PPIPE_LIST_ELEMENT pCurrent = NULL;
       
    int i = 0;
    for (i = 0; i < HOST_MAX_EP+1+DMA_MAX_CHANNEL; i++)
    {
        EventInfoObj[i].pPipe = NULL;
        EventInfoObj[i].IsDMA = FALSE;
        EventInfoObj[i].ucReservedDeviceAddr = 0xff;
        EventInfoObj[i].ucDeviceEndPoint = 0xff;
    }    
    
    // First add the EP Update Event into the hWaitHandle first.
    hWaitHandle[0] = m_hUpdateEPEvent;
    
    hCount = 1;

    while ( !m_fCheckTransferThreadClosing ) {
        UCHAR signal_evt;
        CPipe * pSignalPipe;
        BOOL  signal_isDMA;        
        UCHAR signal_ep;

        // Wait for any of the endpoint generate interrupt.        
        dwErr = WaitForMultipleObjects(hCount, hWaitHandle, FALSE, timeout);        

        // Receive either timeout or event signal
        if (( m_fCheckTransferThreadClosing ) || (dwErr == WAIT_FAILED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("WaitForMultipleObjects in BusyPipe failed with err %d\r\n"),
                GetLastError()));
            break;
        }

        if (dwErr == WAIT_OBJECT_0)
        {                          
            DWORD dwCurEPSetList = 0;
            pCurrent = m_pBusyPipeList;                        
            hCount = 1;
            while (pCurrent != NULL)
            {
                USB_ENDPOINT_DESCRIPTOR endptDesc = pCurrent->pPipe->GetEndptDescriptor();            
                UCHAR endpoint = USB_ENDPOINT(endptDesc.bEndpointAddress);
                // Re-assign the handle onto the hWaitHandle
                // hWaitHandle[0] is always there, so, we don't need to 
                // worry about that.
                if (endpoint == 0)
                {                   
                    DWORD EPSet = dwCurEPSetList & 1;                    
                    if ((m_hEP0CheckForDoneTransfersEvent) && (EPSet == 0))
                    {
                        hWaitHandle[hCount] = m_hEP0CheckForDoneTransfersEvent;
                        EventInfoObj[hCount].pPipe = NULL;
                        EventInfoObj[hCount].IsDMA = FALSE;
                        EventInfoObj[hCount].ucReservedDeviceAddr = pCurrent->pPipe->GetReservedDeviceAddr();
                        EventInfoObj[hCount].ucDeviceEndPoint = endptDesc.bEndpointAddress;
                        m_hEP2Handles[0][DIR_IN] = m_hEP2Handles[0][DIR_OUT] = m_hEP0CheckForDoneTransfersEvent;
                        dwCurEPSetList |= 1;
                        hCount++;
                    }
                    else if (EPSet != 0)
                        DEBUGMSG(1, (TEXT("EP 0 has been setup\r\n")));
                    else
                        DEBUGMSG(ZONE_WARNING, (TEXT("Warning!!! EndPoint 0 => no handle\r\n")));
                                                
                }
                else // Non-endpoint 0
                {
                    // For the rest of the non-control pipe, just
                    // register the handle associated with the pipe.
                    if (!pCurrent->pPipe->GetEPTransferEvent())
                        DEBUGMSG(1, (TEXT("ERROR!!! No Transfer Event for EP 0x%x\r\n"), endpoint));
                    else
                    {
                        hWaitHandle[hCount] = pCurrent->pPipe->GetEPTransferEvent();
                        EventInfoObj[hCount].pPipe = pCurrent->pPipe;
                        EventInfoObj[hCount].IsDMA = FALSE;
                        EventInfoObj[hCount].ucReservedDeviceAddr = pCurrent->pPipe->GetReservedDeviceAddr();
                        EventInfoObj[hCount].ucDeviceEndPoint = endptDesc.bEndpointAddress;
                        hCount++;
                    }

                    // Finally do the DMA event
                    if (pCurrent->pPipe->GetDMATransferEvent())
                    {
                        hWaitHandle[hCount] = pCurrent->pPipe->GetDMATransferEvent();
                        EventInfoObj[hCount].pPipe = pCurrent->pPipe;
                        EventInfoObj[hCount].IsDMA = TRUE;
                        EventInfoObj[hCount].ucReservedDeviceAddr = pCurrent->pPipe->GetReservedDeviceAddr();
                        EventInfoObj[hCount].ucDeviceEndPoint = endptDesc.bEndpointAddress;
                        hCount++;
                    }
                }              

                pCurrent = pCurrent->pNext;
            }            
            DEBUGMSG(ZONE_PIPE, (TEXT("CBusyPipeL ist::Total no of event = %d\r\n"), hCount));
            continue;
        }

        signal_evt = (UCHAR)(dwErr - WAIT_OBJECT_0);
        // Find out which pipe corresponding to
        pSignalPipe = EventInfoObj[signal_evt].pPipe;
        // Is this DMA signal?
        signal_isDMA = EventInfoObj[signal_evt].IsDMA;

        signal_ep = USB_ENDPOINT(EventInfoObj[signal_evt].ucDeviceEndPoint);


        DEBUGMSG(ZONE_TRANSFER, (TEXT("CBusyPipeList get signal evt %d EP 0x%x signal, DMA (%d)\r\n"), 
            signal_evt, ((signal_ep ==0)? 0:(pSignalPipe->GetEndptDescriptor()).bEndpointAddress), 
            signal_isDMA));
       
        Lock();
        pCurrent = m_pBusyPipeList;
             
        while ( pCurrent != NULL ) {            
#ifdef DEBUG
            USB_ENDPOINT_DESCRIPTOR endptDesc = pCurrent->pPipe->GetEndptDescriptor();
#endif
            BOOL match = FALSE;
            // Find out which actual data is being transferred.
            // We need to check the endpoint and also the status.

            // Need to find out which actual pipe is sending 
            // as there are now more than 1 pipes in same endpoint
            if (!signal_isDMA)
            {
                if (signal_ep != 0)
                    match = (pCurrent->pPipe == pSignalPipe);
                else
                    match = pCurrent->pPipe->m_pCMhcd->IsDeviceLockEP0(pCurrent->pPipe->GetReservedDeviceAddr());
            }
            else
            {
                if (signal_ep == 0)
                    DEBUGMSG(1, (TEXT("Wrong!! EP0 cannot have DMA interrupt!!\r\n")));
                else
                    match = (pCurrent->pPipe == pSignalPipe);
            }

            if (match)
            {
                // We don't need to loop through, just check                
                DEBUGMSG(ZONE_TRANSFER, (TEXT("CBusyPipeThread=>ProcessEP %d dir %d\r\n"), 
                    USB_ENDPOINT(endptDesc.bEndpointAddress), USB_ENDPOINT_DIRECTION_OUT(endptDesc.bEndpointAddress)));
                pCurrent->pPipe->ProcessEP(STATUS_PROCESS_INTR, signal_isDMA);
                pCurrent->pPipe->CheckForDoneTransfers();
                break;
            }         
            pCurrent = pCurrent->pNext;
        }        
        Unlock();
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("-CPipe::CheckForDoneTransfersThread\n")) );
    return 0;
}
// ******************************************************************
// Scope: protected static 
BOOL CBusyPipeList::AddToBusyPipeList( IN CPipe * const pPipe,
                               IN const BOOL fHighPriority )
//
// Purpose: Add the pipe indicated by pPipe to our list of busy pipes.
//          This allows us to check for completed transfers after 
//          getting an interrupt, and being signaled via 
//          SignalCheckForDoneTransfers
//
// Parameters: pPipe - pipe to add to busy list
//
//             fHighPriority - if TRUE, add pipe to start of busy list,
//                             else add pipe to end of list.
//
// Returns: TRUE if pPipe successfully added to list, else FALSE
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CPipe::AddToBusyPipeList - new pipe(%s) 0x%x, pri %d\n"), pPipe->GetPipeType(), pPipe, fHighPriority ));    
    PREFAST_DEBUGCHK( pPipe != NULL );
    BOOL fSuccess = FALSE;

    // make sure there nothing on the pipe already (it only gets officially added after this function succeeds).
    Lock();
#ifdef DEBUG
{
    // make sure this pipe isn't already in the list. That should never happen.
    // also check that our m_debug_numItemsOnBusyPipeList is correct
    PPIPE_LIST_ELEMENT pBusy = m_pBusyPipeList;
    int count = 0;
    while ( pBusy != NULL ) {
        DEBUGCHK( pBusy->pPipe != NULL &&
                  pBusy->pPipe != pPipe );
        pBusy = pBusy->pNext;
        count++;
    }
    DEBUGCHK( m_debug_numItemsOnBusyPipeList == count );
}
#endif // DEBUG
    
    PPIPE_LIST_ELEMENT pNewBusyElement = new PIPE_LIST_ELEMENT;
    if ( pNewBusyElement != NULL ) {
        pNewBusyElement->pPipe = pPipe;
        if ( fHighPriority || m_pBusyPipeList == NULL ) {
            // add pipe to start of list
            pNewBusyElement->pNext = m_pBusyPipeList;
            m_pBusyPipeList = pNewBusyElement;
        } else {
            // add pipe to end of list
            PPIPE_LIST_ELEMENT pLastElement = m_pBusyPipeList;
            while ( pLastElement->pNext != NULL ) {
                pLastElement = pLastElement->pNext;
            }
            pNewBusyElement->pNext = NULL;
            pLastElement->pNext = pNewBusyElement;
        }
        fSuccess = TRUE;
    #ifdef DEBUG
        m_debug_numItemsOnBusyPipeList++;
    #endif // DEBUG
    }

    // Finally check if endpoint 0, register it.
    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    UCHAR endpoint = USB_ENDPOINT(endptDesc.bEndpointAddress);
        
    if (endpoint == 0)
    {

        pPipe->SetMappedEndPoint(0);

        DEBUGMSG(ZONE_PIPE|ZONE_DEBUG, (TEXT("AddToBusyPipeList with EP %d, devaddr %d\r\n"), endpoint, pPipe->GetReservedDeviceAddr()));
            
        if (m_hEP0CheckForDoneTransfersEvent)
        {
            if (m_PipeListInfoEP0CheckForDoneTransfersCount == NULL)
                DEBUGMSG(ZONE_ERROR, (TEXT("CBusyPipeList::ERROR - EP Handle has been created but count is not\r\n")));
            else
            {
                PPIPE_LIST_INFO pLast = NULL;
                PPIPE_LIST_INFO pTemp = m_PipeListInfoEP0CheckForDoneTransfersCount;                
                while (pTemp != NULL)
                {
                    if (pTemp->deviceAddr == pPipe->GetReservedDeviceAddr())
                    {
                        DEBUGMSG(1, (TEXT("Device EP %d has been added\r\n"), endpoint));
                        break;
                    }
                    pLast = pTemp;
                    pTemp = pTemp->pNext;
                }

                if (pTemp == NULL)
                {
                    pTemp = new PIPE_LIST_INFO;
                    pTemp->deviceAddr = pPipe->GetReservedDeviceAddr();
                    pTemp->pNext = NULL;
                    if (pLast == NULL) // Head should not happen here, anyway.
                        m_PipeListInfoEP0CheckForDoneTransfersCount = pTemp;
                    else
                        pLast->pNext = pTemp;
                }
            }
        }
        else // if m_hEP0CheckForDoneTransfersEvent is not created yet
        {
            m_hEP0CheckForDoneTransfersEvent = CreateEvent( NULL, FALSE, FALSE, NULL );  
            if (m_hEP0CheckForDoneTransfersEvent == NULL)
                DEBUGMSG(ZONE_ERROR, (TEXT("CBusyPipeList- CreateEvent failure!!!\r\n")));
            else
            {
                if (m_PipeListInfoEP0CheckForDoneTransfersCount != NULL)
                    DEBUGMSG(ZONE_ERROR, (TEXT("CBusyPipeList EP %d , index has orphan count\r\n"), 
                        endpoint));
                else
                {
                    m_PipeListInfoEP0CheckForDoneTransfersCount = new PIPE_LIST_INFO;
                    m_PipeListInfoEP0CheckForDoneTransfersCount->deviceAddr = pPipe->GetReservedDeviceAddr();
                    m_PipeListInfoEP0CheckForDoneTransfersCount->pNext = NULL;
                }
            }
        } // end the else case.
        

#ifdef DEBUG
        int count = 0;
        PPIPE_LIST_INFO pList = m_PipeListInfoEP0CheckForDoneTransfersCount;
        while (pList != NULL)
        {
            count++;
            pList = pList->pNext;
        }

        DEBUGMSG(ZONE_PIPE|ZONE_DEBUG, (TEXT("AddBusyPipeList leaving EP %d, count %d\r\n"),
            endpoint, count));
#endif
    }  // if (endpoint == 0)
    SetEvent(m_hUpdateEPEvent); 
    
    Unlock();
    DEBUGMSG( ZONE_PIPE, (TEXT("-CPipe::AddToBusyPipeList - new pipe(%s) 0x%x, pri %d, returning BOOL %d\n"), pPipe->GetPipeType(), pPipe, fHighPriority, fSuccess) );    
    return fSuccess;
}
// *****************************************************************
// Scope: protected static
BOOL CBusyPipeList::SignalDisconnectComplete(PVOID pContext)
//
// Purpose: Signal back to OTG that the disconnect is totally complete now.
//
// Parameters: pContext - Pointer to PHSMUSB_T
//
// Return: TRUE - it is completed, FALSE - not yet complete
{
    PHSMUSB_T pOTG = (PHSMUSB_T)(pContext);
    
    if ((m_pBusyPipeList == NULL) && (m_PipeListInfoEP0CheckForDoneTransfersCount == NULL) && (!m_SignalDisconnectACK))
    {
        {
            m_SignalDisconnectACK = TRUE;        
            pOTG->connect_status |= CONN_DC;
            DEBUGMSG(ZONE_DEBUG, (TEXT("Signal Disconnect Complete\r\n")));
            SetEvent(pOTG->hSysIntrEvent);
            return TRUE;
        }
    }

    return FALSE;
}

// ******************************************************************
// Scope: protected static
void CBusyPipeList::RemoveFromBusyPipeList( IN CPipe * const pPipe )
//
// Purpose: Remove this pipe from our busy pipe list. This happens if
//          the pipe is suddenly aborted or closed while a transfer
//          is in progress
//
// Parameters: pPipe - pipe to remove from busy list
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CPipe::RemoveFromBusyPipeList - pipe(%s) 0x%x\n"), pPipe->GetPipeType(), pPipe ) );    
    Lock();
#ifdef DEBUG
    BOOL debug_fRemovedPipe = FALSE;
    {
        // check m_debug_numItemsOnBusyPipeList
        PPIPE_LIST_ELEMENT pBusy = m_pBusyPipeList;
        int count = 0;
        while ( pBusy != NULL ) {
            DEBUGCHK( pBusy->pPipe != NULL );
            pBusy = pBusy->pNext;
            count++;
        }
        DEBUGCHK( m_debug_numItemsOnBusyPipeList == count );
    }
#endif // DEBUG
    PPIPE_LIST_ELEMENT pPrev = NULL;
    PPIPE_LIST_ELEMENT pCurrent = m_pBusyPipeList;
    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    UCHAR endpoint = USB_ENDPOINT(endptDesc.bEndpointAddress);

    while ( pCurrent != NULL ) {
        if ( pCurrent->pPipe == pPipe ) {
            // Remove item from the linked list
            if ( pCurrent == m_pBusyPipeList ) {
                DEBUGCHK( pPrev == NULL );
                m_pBusyPipeList = m_pBusyPipeList->pNext;
            } else {
                DEBUGCHK( pPrev != NULL &&
                          pPrev->pNext == pCurrent );
                pPrev->pNext = pCurrent->pNext;
            }
            delete pCurrent;
            pCurrent = NULL;
        #ifdef DEBUG
            debug_fRemovedPipe = TRUE;
            DEBUGCHK( --m_debug_numItemsOnBusyPipeList >= 0 );
        #endif // DEBUG
            break;
        } else {
            // Check next item
            pPrev = pCurrent;
            pCurrent = pPrev->pNext;
        }
    }

    // Finally, close the handle and notify the removal 
    if (endpoint == 0)
    {
        BOOL fFound = FALSE;
        PPIPE_LIST_INFO pListTemp = m_PipeListInfoEP0CheckForDoneTransfersCount;
        PPIPE_LIST_INFO pListPrev;

        pListPrev = pListTemp;
        while (pListTemp != NULL)
        {
            if (pListTemp->deviceAddr == pPipe->GetReservedDeviceAddr())
            {
                fFound = TRUE;
                DEBUGMSG(ZONE_PIPE|ZONE_DEBUG, (TEXT("RemoveFromBusyPipeList found the device\r\n")));
                if (pListPrev == pListTemp) // head
                {
                    delete pListTemp;
                    m_PipeListInfoEP0CheckForDoneTransfersCount = NULL;
                }
                else
                {
                    pListPrev->pNext = pListTemp->pNext;
                    delete pListTemp;
                }
                break;
            }
            pListPrev = pListTemp;
            pListTemp = pListTemp->pNext;   
        } // end while (pListTemp != NULL)

        if (fFound == FALSE)
            DEBUGMSG(ZONE_PIPE, (TEXT("Warning! Not able to find EP %d,DevAddr %d\r\n"),
                endpoint, pPipe->GetReservedDeviceAddr()));


        if (m_PipeListInfoEP0CheckForDoneTransfersCount == NULL)
        {
            DEBUGMSG(ZONE_PIPE|ZONE_DEBUG, (TEXT("RemoveBusyPipeList for EP %d  -clear\r\n"),
                endpoint));
            if (USB_ENDPOINT(endpoint) == 0)
                DEBUGMSG(ZONE_PIPE, (TEXT("RemoveBusyPipeList clean EP 0\r\n")));
            CloseHandle(m_hEP0CheckForDoneTransfersEvent);
            m_hEP0CheckForDoneTransfersEvent = NULL;
        }

#ifdef DEBUG
        int count = 0;
        PPIPE_LIST_INFO pList = m_PipeListInfoEP0CheckForDoneTransfersCount;
        while (pList != NULL)
        {
            count++;
            pList = pList->pNext;
        }

        DEBUGMSG(ZONE_PIPE|ZONE_DEBUG, (TEXT("RemoveBusyPipeList leaving EP %d, count %d\r\n"),
            endpoint, count));
#endif
    } // if (endpoint == 0)
    else
    {        
        UCHAR channel = pPipe->m_pCMhcd->DeviceInfo2Channel(pPipe);                
        pPipe->m_pCMhcd->ReleaseDMAChannel(pPipe, channel);
        pPipe->m_pCMhcd->ReleasePhysicalEndPoint(pPipe, TRUE, TRUE);
        DEBUGMSG(ZONE_PIPE, (TEXT("RemoveFromBusyPipeList: Release all\r\n")));
    }

    SetEvent(m_hUpdateEPEvent);
        
    Unlock();    
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE && debug_fRemovedPipe, (TEXT("-CPipe::RemoveFromBusyPipeList, removed pipe(%s) 0x%x\n"), pPipe->GetPipeType(), pPipe));
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE && !debug_fRemovedPipe, (TEXT("-CPipe::RemoveFromBusyPipeList, pipe(%s) 0x%x was not on busy list\n"), pPipe->GetPipeType(), pPipe ));
}

#define ASYNC_PARK_MODE 1
#define FRAME_LIST_SIZE 0x400

// ************************************CHW ******************************  
CHW::CHW( IN CPhysMem * const pCPhysMem, IN LPVOID pvUhcdPddObject,  IN DWORD dwSysIntr)
: m_cBusyPipeList(FRAME_LIST_SIZE)
{    
    int i; 
// definitions for static variables    
    g_fPowerUpFlag = FALSE;
    g_fPowerResuming = FALSE;
    dwSysIntr = 1;//Verify
    //m_pHcd = pHcd;
    m_pMem = pCPhysMem;
    m_pPddContext = pvUhcdPddObject;
    //m_FrameListMask = FRAME_LIST_SIZE-1;  
    m_hUsbHubConnectEvent = NULL;
    m_hUsbHubDisconnectEvent = NULL;

    m_intr_rx_avail = 0;
    m_bDoResume=FALSE;
    m_NumOfPort = 1; // Hardcoded as this point
    InitializeCriticalSection( &m_csFrameCounter );
    InitializeCriticalSection(&m_csDMAChannel);
    InitializeCriticalSection(&m_csEndPoint);
    m_hLockDMAAccess = CreateEvent(NULL, FALSE, TRUE, NULL);    
    for (i = 0; i < MAX_DMA_CHANNEL; i++)
        m_DMAChannel[i] = 0xff;     
    for (i = 0; i < HOST_MAX_EPNUM; i++)
    {
        m_EndPoint[i][DIR_IN].usDeviceInfo = 0xffff;
        m_EndPoint[i][DIR_OUT].usDeviceInfo = 0xffff;
        m_EndPoint[i][DIR_IN].usPrevDevInfo = 0xffff;
        m_EndPoint[i][DIR_OUT].usPrevDevInfo = 0xffff;
    }

    m_EndPointInUseCount = 0;
}
CHW::~CHW()
{
    DeInitialize();
    DeleteCriticalSection( &m_csFrameCounter );
}

// ******************************************************************
BOOL CHW::Initialize( )
// Purpose: Reset and Configure the Host Controller with the schedule.
//
// Parameters: portBase - base address for host controller registers
//
//             dwSysIntr - system interrupt number to use for USB
//                         interrupts from host controller
//
//             frameListPhysAddr - physical address of frame list index
//                                 maintained by CPipe class
//
//             pvUhcdPddObject - PDD specific structure used during suspend/resume
//
// Returns: TRUE if initialization succeeded, else FALSE
//
// Notes: This function is only called from the CUhcd::Initialize routine.
//
//        This function is static
// ******************************************************************
{   
    DEBUGMSG( ZONE_INIT, (TEXT("+CHW::Initialize\n")));

    // set up the frame list area.
    if (m_cBusyPipeList.Init()==FALSE) {
        DEBUGMSG( ZONE_ERROR, (TEXT("-CHW::Initialize - zero Register Base or CeriodicMgr or CAsyncMgr fails\n")));
        ASSERT(FALSE);
        return FALSE;
    }
    m_hUsbHubConnectEvent = CreateEvent( NULL, FALSE, FALSE, NULL );    
    m_hUsbHubDisconnectEvent = CreateEvent( NULL, FALSE, FALSE, NULL );    
    InitializeCriticalSection(&m_LockEP0DeviceAddress.hLockCS);    
    m_LockEP0DeviceAddress.ucLockEP = 0xff;

    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS, (TEXT("CHW::Initialize - signalling global reset\n")));    
    m_fifo_avail_addr = 0;    

    return TRUE;
}
// ******************************************************************
void CHW::DeInitialize( void )
//
// Purpose: Delete any resources associated with static members
//
// Parameters: none
//
// Returns: nothing
//
// Notes: This function is only called from the ~CUhcd() routine.
//
//        This function is static
// ******************************************************************
{
   // Stop The Controller.
    {
        // Stop USB Controller
    }
    m_cBusyPipeList.DeInit();
}

// ******************************************************************
void CHW::EnterOperationalState( void )
//
// Purpose: Signal the host controller to start processing the schedule
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: This function is only called from the CUhcd::Initialize routine.
//        It assumes that CPipe::Initialize and CHW::Initialize
//        have already been called.
//
//        This function is static
// ******************************************************************
{
    
    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS && ZONE_VERBOSE, (TEXT("CHW::EnterOperationalState - clearing status reg\n")));

}

// ******************************************************************
BOOL CHW::GetFrameNumber(OUT LPDWORD lpdwFrameNumber )
//
// Purpose: Return the current frame number
//
// Parameters: None
//
// Returns: 32 bit current frame number
//
// Notes: See also comment in UpdateFrameCounter
// ******************************************************************
{
    PHSMUSB_T pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
        return FALSE;

    *lpdwFrameNumber = INREG16(&pOTG->pUsbGenRegs->Frame);
    return TRUE;
}

// ******************************************************************
BOOL CHW::GetFrameLength(OUT LPUSHORT lpuFrameLength)
//
// Purpose: Return the current frame length in 12 MHz clocks
//          (i.e. 12000 = 1ms)
//
// Parameters: None
//
// Returns: frame length
//
// Notes:
// ******************************************************************
{
    *lpuFrameLength=60000; // same as EHCI
    return TRUE;
}

// ******************************************************************
BOOL CHW::SetFrameLength( IN HANDLE /*hEvent*/,
                          IN const USHORT /*uFrameLength*/ )
//
// Purpose: Set the Frame Length in 12 Mhz clocks. i.e. 12000 = 1ms
//
// Parameters:  hEvent - event to set when frame has reached required
//                       length
//
//              uFrameLength - new frame length
//
// Returns: TRUE if frame length changed, else FALSE
//
// Notes:
// ******************************************************************
{
    BOOL fSuccess = FALSE;

    // to prevent multiple threads from simultaneously adjusting the
    // frame length, InterlockedTestExchange is used. This is
    // cheaper than using a critical section.
    return fSuccess;
}

// ******************************************************************
BOOL CHW::StopAdjustingFrame( void )
//
// Purpose: Stop modifying the host controller frame length
//
// Parameters: None
//
// Returns: TRUE
//
// Notes:
// ******************************************************************
{
    return TRUE;
}
DWORD CALLBACK CHW::CeResumeThreadStub ( IN PVOID context )
{
    return ((CHW *)context)->CeResumeThread ( );
}

// ******************************************************************
DWORD CHW::CeResumeThread ( )
//
// Purpose: Force the HCD to reset and regenerate itself after power loss.
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: Because the PDD is probably maintaining pointers to the Hcd and Memory
//   objects, we cannot free/delete them and then reallocate. Instead, we destruct
//   them explicitly and use the placement form of the new operator to reconstruct
//   them in situ. The two flags synchronize access to the objects so that they
//   cannot be accessed before being reconstructed while also guaranteeing that
//   we don't miss power-on events that occur during the reconstruction.
//
//        This function is static
// ******************************************************************
{
    // reconstruct the objects at the same addresses where they were before;
    // this allows us not to have to alert the PDD that the addresses have changed.

    DEBUGCHK( g_fPowerResuming == FALSE );

    // order is important! resuming indicates that the hcd object is temporarily invalid
    // while powerup simply signals that a powerup event has occurred. once the powerup
    // flag is cleared, we will repeat this whole sequence should it get resignalled.
    g_fPowerUpFlag = FALSE;
    g_fPowerResuming = TRUE;

    DeviceDeInitialize();
    for(;;) {  // breaks out upon successful reinit of the object

        if (DeviceInitialize())
            break;
        // getting here means we couldn't reinit the HCD object!
        ASSERT(FALSE);
        DEBUGMSG(ZONE_ERROR, (TEXT("USB cannot reinit the HCD at CE resume; retrying...\n")));
        DeviceDeInitialize();
        Sleep(15000);
    }

    // the hcd object is valid again. if a power event occurred between the two flag
    // assignments above then the IST will reinitiate this sequence.
    g_fPowerResuming = FALSE;
    if (g_fPowerUpFlag)
        PowerMgmtCallback(TRUE);
    
    return 0;
}

// ******************************************************************
BOOL CHW::DidPortStatusChange( IN const UCHAR port )
//
// Purpose: Determine whether the status of root hub port # "port" changed
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
// Returns: TRUE if status changed, else FALSE
//
// Notes:
// ******************************************************************
{
    PHSMUSB_T pOTG;
    DWORD PortSpeed = 0x00; // 0x01 - LS, 0x10 - FS, 0x11 - HS
    // port == specifies root hub itself, whose status never changes
    if ( port > 0 ) {
        DEBUGMSG(ZONE_VERBOSE, (TEXT("CHW::DidPortStatusChange for port %d\r\n"), port));
        // Reset the port now and make sure it is okay
        pOTG = (PHSMUSB_T)GetOTGContext();
        if (pOTG == NULL)
            return FALSE;

        DEBUGMSG(ZONE_HCD, (TEXT("CHW::B4 Power = 0x%x\r\n"), pOTG->pUsbGenRegs->Power));
        ResetAndEnablePort(port);
        DEBUGMSG(ZONE_HCD, (TEXT("CHW::DidPortStatusChange: After reset\r\n")));
        DEBUGMSG(ZONE_HCD, (TEXT("CHW::Power = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->Power)));
        DEBUGMSG(ZONE_HCD, (TEXT("CHW::DevCtl = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->DevCtl)));
        if (INREG8(&pOTG->pUsbGenRegs->DevCtl) & DEVCTL_LSDEV)
            PortSpeed = 0x01;
        else if (INREG8(&pOTG->pUsbGenRegs->DevCtl) & DEVCTL_FSDEV) 
            PortSpeed = ((INREG8(&pOTG->pUsbGenRegs->Power) & POWER_HSMODE)? 0x11: 0x10);               

        DEBUGMSG(ZONE_HCD, (TEXT("CHW::PortSpeed = %s\r\n"), 
            ((PortSpeed == 0x01)? TEXT("LS"): ((PortSpeed == 0x10)? TEXT("FS"): TEXT("HS")))));

        // Now testing and see the connect status
        DEBUGMSG(ZONE_HCD, (TEXT("Connect Status = 0x%x\r\n"), pOTG->connect_status));
        if (pOTG->connect_status & CONN_CSC)
            return TRUE;
    }
    return FALSE;
}
// ******************************************************************
BOOL CHW::GetPortStatus( IN const UCHAR port,
                         OUT USB_HUB_AND_PORT_STATUS& rStatus )
//
// Purpose: This function will return the current root hub port
//          status in a non-hardware specific format
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
//             rStatus - reference to USB_HUB_AND_PORT_STATUS to get the
//                       status
//
// Returns: TRUE
//
// Notes:
// ******************************************************************
{
    PHSMUSB_T pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("GetPortStatus with pOTG failed\r\n")));
        return FALSE;
    }
    memset( &rStatus, 0, sizeof( USB_HUB_AND_PORT_STATUS ) );
    if ( port > 0 ) {
        // request refers to a root hub port
       
        // read the port status register
        rStatus.change.port.ConnectStatusChange = ((pOTG->connect_status & CONN_CSC)? 1:0);
        rStatus.change.port.PortEnableChange = 0;
        rStatus.change.port.OverCurrentChange = 0;
        rStatus.status.port.DeviceIsLowSpeed = (INREG8(&pOTG->pUsbGenRegs->DevCtl) & DEVCTL_LSDEV);
        rStatus.status.port.DeviceIsHighSpeed = ((INREG8(&pOTG->pUsbGenRegs->Power) & POWER_HSMODE)? 1:0) ;
        rStatus.status.port.PortConnected = ((pOTG->connect_status & CONN_CCS)?1:0);
        rStatus.status.port.PortEnabled = 1;
        rStatus.status.port.PortOverCurrent = 0;
        rStatus.status.port.PortPower = 1;
        rStatus.status.port.PortReset = INREG8(&pOTG->pUsbGenRegs->Power) & POWER_RESET;
        rStatus.status.port.PortSuspended =  INREG8(&pOTG->pUsbGenRegs->Power) & POWER_SUSPENDM;
    }
#ifdef DEBUG
    else {
        // request is to Hub. rStatus was already memset to 0 above.
        DEBUGCHK( port == 0 );
        // local power supply good
        DEBUGCHK( rStatus.status.hub.LocalPowerStatus == 0 );
        // no over current condition
        DEBUGCHK( rStatus.status.hub.OverCurrentIndicator == 0 );
        // no change in power supply status
        DEBUGCHK( rStatus.change.hub.LocalPowerChange == 0 );
        // no change in over current status
        DEBUGCHK( rStatus.change.hub.OverCurrentIndicatorChange == 0 );
    }
#endif // DEBUG

    return TRUE;
}

// ******************************************************************
BOOL CHW::RootHubFeature( IN const UCHAR port,
                          IN const UCHAR setOrClearFeature,
                          IN const USHORT feature )
//
// Purpose: This function clears all the status change bits associated with
//          the specified root hub port.
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
// Returns: TRUE iff the requested operation is valid, FALSE otherwise.
//
// Notes: Assume that caller has already verified the parameters from a USB
//        perspective. The HC hardware may only support a subset of that
//        (which is indeed the case for UHCI).
// ******************************************************************
{
    PHSMUSB_T pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_HCD, (TEXT("GetPortStatus with pOTG failed\r\n")));
        return FALSE;
    }

    if (port == 0) {
        // request is to Hub but...
        // uhci has no way to tweak features for the root hub.
        return FALSE;
    }

    DEBUGMSG(ZONE_HCD, (TEXT("SetOrClearFeature(0x%x), feature(0x%x), port(%d)\r\n"), setOrClearFeature, feature, port));
    // mask the change bits because we write 1 to them to clear them //      
    if (setOrClearFeature == USB_REQUEST_SET_FEATURE)
    {
        switch (feature) {
            case USB_HUB_FEATURE_PORT_RESET:            
                DEBUGMSG(ZONE_HCD, (TEXT("USB_HUB_FEATURE_PORT_RESET\r\n")));
                ResetAndEnablePort(1);
                break;
            case USB_HUB_FEATURE_PORT_SUSPEND:            
                DEBUGMSG(ZONE_HCD, (TEXT("USB_HUB_FEATURE_PORT_SUSPEND\r\n")));
                break;
            case USB_HUB_FEATURE_PORT_POWER:            
                DEBUGMSG(ZONE_HCD, (TEXT("USB_HUB_FEATURE_PORT_POWER\r\n")));
                break;
            default: return FALSE;
        }
    }
    else
    {
        switch (feature) {
            case USB_HUB_FEATURE_PORT_ENABLE:
                DEBUGMSG(ZONE_HCD, (TEXT("CLEAR: USB_HUB_FEATURE_PORT_ENABLE\r\n")));
                break;
            case USB_HUB_FEATURE_PORT_SUSPEND:    
                DEBUGMSG(ZONE_HCD, (TEXT("CLEAR: USB_HUB_FEATURE_PORT_SUSPEND\r\n")));
                break;
            case USB_HUB_FEATURE_C_PORT_CONNECTION:       
                    // I think it should be done by critical section or function pointer in OTG side
                DEBUGMSG(ZONE_HCD, (TEXT("USB_HUB_FEATURE_C_PORT_CONNECTION\r\n")));
                pOTG->connect_status &= ~CONN_CSC; // clear the connect status change
                break;
            case USB_HUB_FEATURE_C_PORT_ENABLE:           
            case USB_HUB_FEATURE_C_PORT_RESET:            
            case USB_HUB_FEATURE_C_PORT_SUSPEND:
            case USB_HUB_FEATURE_C_PORT_OVER_CURRENT:
            case USB_HUB_FEATURE_PORT_POWER:
            default: return FALSE;
        }
    }
    return TRUE;
}


// ******************************************************************
BOOL CHW::ResetAndEnablePort( IN const UCHAR port )
//
// Purpose: reset/enable device on the given port so that when this
//          function completes, the device is listening on address 0
//
// Parameters: port - root hub port # to reset/enable
//
// Returns: TRUE if port reset and enabled, else FALSE
//
// Notes: This function takes approx 60 ms to complete, and assumes
//        that the caller is handling any critical section issues
//        so that two different ports (i.e. root hub or otherwise)
//        are not reset at the same time. please refer 4.2.2 for detail
// ******************************************************************
{
    BOOL fSuccess = TRUE;
    PHSMUSB_T pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
        return FALSE;

    DEBUGMSG(ZONE_HCD, (TEXT("ResetAndEnablePort(%d)\r\n"), port));        
        // USB 1.1 spec, 7.1.7.3 - device may take up to 10 ms
        // to recover after reset is removed
        //Do a reset now no matter what
    if (port > 0)
    {
        // We do the Power |= POWER_RESET
        SETREG8(&pOTG->pUsbGenRegs->Power, POWER_RESET);
        Sleep(50); // Sleep for 50ms after reset as request
        // We do the Power &= ~POWER_RESET
        CLRREG8(&pOTG->pUsbGenRegs->Power, POWER_RESET);
    }

    Sleep( 10 );
  

    return fSuccess;
}
// ******************************************************************
void CHW::DisablePort( IN const UCHAR port )
//
// Purpose: disable the given root hub port
//
// Parameters: port - port # to disable
//
// Returns: nothing
//
// Notes: This function will take about 10ms to complete
// ******************************************************************
{
#if 0
    PORTSC portSC=Read_PORTSC(port);;
    // no point doing any work unless the port is enabled
    if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.Enabled ) {
        // clear port enabled bit and enabled change bit,
        // but don't alter the connect status change bit,
        // which is write-clear.
        portSC.bit.Enabled=0;
        portSC.bit.ConnectStatusChange=0;
        portSC.bit.EnableChange=1;
        portSC.bit.OverCurrentChange=0;        
        Write_PORTSC( port, portSC );

        // disable port can take some time to act, because
        // a USB request may have been in progress on the port.
        Sleep( 10 );
    }
#else
#ifdef DEBUG
    DEBUGMSG(ZONE_HCD, (TEXT("CHW::DisablePort %d\r\n"), port));
#else
    UNREFERENCED_PARAMETER(port);
#endif
#endif
}
BOOL CHW::WaitForPortStatusChange (HANDLE m_hHubChanged)
{
    DWORD dwErr;
    DWORD timeout = 500;
    static BOOL bDisconnect = FALSE;
    if ((m_hUsbHubDisconnectEvent) && (m_hUsbHubConnectEvent)) {
       for(;;)
       {           
            if (m_hHubChanged!=NULL) {
                HANDLE hArray[3];
                hArray[0]=m_hUsbHubConnectEvent;
                hArray[1]=m_hUsbHubDisconnectEvent;
                hArray[2]=m_hHubChanged;
                dwErr = WaitForMultipleObjects(3,hArray,FALSE,timeout);
            }
            else
            {
                HANDLE hArray[2];
                hArray[0] = m_hUsbHubConnectEvent;
                hArray[1] = m_hUsbHubDisconnectEvent;
                dwErr = WaitForMultipleObjects(2, hArray, FALSE,timeout);
            }
            if (dwErr == WAIT_OBJECT_0+1)
                bDisconnect = TRUE;
            else if (dwErr == WAIT_TIMEOUT) 
            {
                BOOL bSignalDetach = FALSE;
                CRootHub *pRootHub = GetRootHub();
                if (pRootHub == NULL)
                {                    
                    DEBUGMSG(1, (TEXT("RootHUB::NULL\r\n")));
                    bSignalDetach = TRUE;
                }
                else 
                {
                    CDevice ** ppDevicePort;
                    int port = pRootHub->GetNumberOfPort();             
                    DEBUGMSG(ZONE_HCD, (TEXT("RootHUB:NoOfPort = %d\r\n"), port));
                    ppDevicePort = pRootHub->GetDeviceOnPort();
                    if (ppDevicePort == NULL)
                    {         
                        DEBUGMSG(1, (TEXT("RootHUB::ppDevicePort = NULL\r\n")));
                        bSignalDetach = TRUE;
                    }
                    else
                    {
                        int i = 0;
                        int no_device = 0;
                        for (i = 0; i < port; i++)
                        {
                            DEBUGMSG(ZONE_HCD, (TEXT("RootHUB:Port[%d] = 0x%x\r\n"), i, ppDevicePort[i]));                                    
                            if (ppDevicePort[i] == NULL)
                                no_device++;
                        }
                        if (no_device == port)
                            bSignalDetach = TRUE;
                    }

                    if ((bSignalDetach) && (bDisconnect))
                    {
                        bDisconnect = FALSE;
                        SignalDisconnectComplete();
                    }
                    timeout = INFINITE;
                }
            }

            if (dwErr != WAIT_TIMEOUT)
                return TRUE;
       }
    }    
    return FALSE;
}

//**************************************************************************************
UCHAR CHW::AcquireDMAChannel(CPipe *pPipe)
// 
//  Description: Get the DMA channel
//
//  Paramter: pPipe - Associated pipe which to acquire the DMA channel
//
//  Return: the DMA channel
{
    int i;
    PHSMUSB_T pOTG = (PHSMUSB_T)pPipe->m_pCMhcd->GetOTGContext();
    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    USHORT deviceInfo = ((pPipe->GetReservedDeviceAddr() << 8)| (endptDesc.bEndpointAddress & 0xff));
    UCHAR channel = 0xff;
    UCHAR avail_channel = 0xff;
    BOOL  fNewChannel = FALSE;
    static int next_availDMA = 0;    

#ifdef DEBUG
	UCHAR endpoint = USB_ENDPOINT(endptDesc.bEndpointAddress);
#endif

    int start = next_availDMA;
    i = start;
    EnterCriticalSection(&m_csDMAChannel);
    for (;;)
    {
        if (m_DMAChannel[i] == deviceInfo)
        {
            channel = (UCHAR)i;
            break;
        }
        else if ((m_DMAChannel[i] == 0xff) && (avail_channel == 0xff))
            avail_channel = (UCHAR)i;

        i = (i+1)%MAX_DMA_CHANNEL;
        if (i == start)
            break;
    }

    if (channel == 0xff)
    {
        channel = avail_channel;
        if (avail_channel != 0xff)
        {
            m_DMAChannel[avail_channel] = deviceInfo;
            next_availDMA = (avail_channel+1)%MAX_DMA_CHANNEL;
            fNewChannel = TRUE;
        }
    }

    if (avail_channel != 0xff)
    {
        // Finally set the m_hEP2Handles 
        m_cBusyPipeList.m_hDMA2Handles[channel] = pPipe->GetDMATransferEvent();        
    }

    if (fNewChannel)
        HcdPdd_PreTransferActivation((SMHCDPdd *)pOTG->pContext[HOST_MODE-1]);

    LeaveCriticalSection(&m_csDMAChannel);
           
    DEBUGMSG(ZONE_HCD|ZONE_DEBUG, (TEXT("AcquireDMAChannel:ep %d, device %d channel %d value = 0x%x\r\n"),
        endpoint, pPipe->GetReservedDeviceAddr(), channel, ((channel == 0xff)?0x00:m_DMAChannel[channel])));


    return channel;
}

//-----------------------------------------------------------------------------------------------
USHORT CHW::Channel2DeviceInfo(UCHAR channel)
// 
//  Purpose: To acquire the device info from the channel
//
//  Paramter: Channel
//
//  Return: Device Info
{
    return m_DMAChannel[channel];
}

//------------------------------------------------------------------------------------------------
UCHAR CHW::DeviceInfo2Channel(CPipe *pPipe)
//
//  Purpose: To get the channel no based on the device info 
//
//  Paramter : pPipe - Pipe to be assocated with
//
{
    int i; 
    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    USHORT deviceInfo = ((pPipe->GetReservedDeviceAddr() << 8)| (endptDesc.bEndpointAddress & 0xff));

    EnterCriticalSection(&m_csDMAChannel);
    for (i = 0; i < DMA_MAX_CHANNEL; i++)
    {
        if (m_DMAChannel[i] == deviceInfo)
        {
            LeaveCriticalSection(&m_csDMAChannel);
            return (UCHAR)i;
        }
    }
    
    LeaveCriticalSection(&m_csDMAChannel);
    return 0xff;
}

//****************************************************************************
BOOL CHW::ReleaseDMAChannel(CPipe *pPipe, UCHAR channel)
//
//  Purpose: To release the channel after being used.
//
//  Parameter : channel - the DMA channel to be released
//              pPipe - the pipe associated
{
    PHSMUSB_T pOTG;
    if ((channel < 0)|| (channel >= DMA_MAX_CHANNEL))
        return TRUE;
    
    pOTG = (PHSMUSB_T)pPipe->m_pCMhcd->GetOTGContext();    
    EnterCriticalSection(&m_csDMAChannel);
    ClearDMAChannel(channel);
    m_DMAChannel[channel] = 0xff;     
    m_cBusyPipeList.m_hDMA2Handles[channel] = NULL;
    LeaveCriticalSection(&m_csDMAChannel);
  
    HcdPdd_PostTransferDeactivation((SMHCDPdd *)pOTG->pContext[HOST_MODE-1]);

    return TRUE;
}


void CHW::DumpRxCSR(UCHAR endpoint)
{
    PHSMUSB_T pOTG;        
    UCHAR csrIndex = INDEX(endpoint);    

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::ConfigEP failed\r\n")));
        return;
    }  
    DEBUGMSG(ZONE_DEBUG, (TEXT("DumpCSR: EP %d\r\n"), endpoint));
    EnterCriticalSection(&pOTG->regCS); 
    OUTREG8(&pOTG->pUsbGenRegs->Index, csrIndex);
    
    DEBUGMSG(ZONE_DEBUG, (TEXT("RxCSR = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));
    DEBUGMSG(ZONE_DEBUG, (TEXT("RxCount = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount)));

    LeaveCriticalSection(&pOTG->regCS);    
}

//-----------------------------------------------------------------------------
UCHAR CHW::AcquirePhysicalEndPoint(CPipe *pPipe)
//  
//  Description: This function is to get the mapped endpoint to do the transfer
//
//  Parameter:  pPipe - the pipe associated with it.
//
//  Return : Return the mapped ep
{
    int i;    
    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    UCHAR endpoint = USB_ENDPOINT(endptDesc.bEndpointAddress);
    UCHAR isOut = USB_ENDPOINT_DIRECTION_OUT(endptDesc.bEndpointAddress)?1:0;    
    UCHAR mappedEP = 0xff;
    UCHAR avail_EP = 0xff;
    UCHAR reuse_EP = 0xff;
    USHORT deviceInfo = ((pPipe->GetReservedDeviceAddr() << 8)| (endptDesc.bEndpointAddress & 0xff));

    // Try to implement a circular queue and see if it can get improved.
    static int next_assigned_EP[2] = {1,1}; // {IN, OUT}

    if (USB_ENDPOINT(endpoint) == 0)
    {
        for (i = 0; i < MAX_DIR; i++)
        {            
            m_EndPoint[0][i].usDeviceInfo = deviceInfo;
            // No need to update the m_hEP2Handles
        }
        return 0;
    }

    EnterCriticalSection(&m_csEndPoint);
    i = next_assigned_EP[isOut];
    int start = i;

    for (;;)
    {
        if (m_EndPoint[i][isOut].usDeviceInfo == deviceInfo)
        {
            mappedEP = (UCHAR)i;
            break;
        }        
        else if ((m_EndPoint[i][isOut].usDeviceInfo == 0xffff) && (reuse_EP == 0xff) &&
            (m_EndPoint[i][isOut].usPrevDevInfo == deviceInfo))
        {
            avail_EP = (UCHAR)i;
            reuse_EP = (UCHAR)i;
        }
        else if ((m_EndPoint[i][isOut].usDeviceInfo == 0xffff) && (avail_EP == 0xff))
        {         
            avail_EP = (UCHAR)i;
        }  
        
        i++;
        if (i >= HOST_MAX_EPNUM)
            i = 1;

        if (i == start)
            break;
    }
    
    if (mappedEP == 0xff)
    {
        mappedEP = avail_EP;        
        if (avail_EP != 0xff)
        {
            next_assigned_EP[isOut] = mappedEP;
            next_assigned_EP[isOut]++;
            if (next_assigned_EP[isOut] >= HOST_MAX_EPNUM)
                next_assigned_EP[isOut] = 1;
            m_EndPoint[avail_EP][isOut].usDeviceInfo = deviceInfo;
            m_EndPoint[avail_EP][isOut].usPrevDevInfo = deviceInfo;
            m_EndPointInUseCount++;
            DEBUGMSG(ZONE_HCD, (TEXT("AcquirePhysicalEndPoint for EP 0x%x Device 0x%x -> Mapped %d [%s]\r\n"),
                endptDesc.bEndpointAddress, pPipe->GetReservedDeviceAddr(), mappedEP, isOut?TEXT("OUT"):TEXT("IN")));
        }
        else
        {
            RETAILMSG(1, (TEXT("FAILED TO GET THE EP!!!\r\n")));
            return 0xff;
        }

    }

    // Finally set the m_hEP2Handles 
    m_cBusyPipeList.m_hEP2Handles[mappedEP][isOut] = pPipe->GetEPTransferEvent();
    pPipe->SetMappedEndPoint(mappedEP);

    LeaveCriticalSection(&m_csEndPoint);
    DEBUGMSG(ZONE_HCD, (TEXT("AcquirePhysicalEP(ep[%d], device[%d]) (%s) value 0x%x, MEP %d\r\n"),
        endpoint, pPipe->GetReservedDeviceAddr(), isOut? TEXT("OUT"):TEXT("IN"), deviceInfo, mappedEP));
    return mappedEP;
}

//--------------------------------------------------------------------------------
BOOL CHW::ReleasePhysicalEndPoint(CPipe *pPipe, BOOL fForce, BOOL fClearAll)
//
//  Description: Release the Physical endpoint after used. If there are available EPs,
//               we should keep it. Otherwise, release it and let other EP use it.
//  
//  Parameter:  pPipe - the transfer pipe associated
//              fForce - force to release it, usually when close pipe.
{
    UCHAR mappedEP = pPipe->GetMappedEndPoint();
    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    UCHAR isOut = USB_ENDPOINT_DIRECTION_OUT(endptDesc.bEndpointAddress)?1:0;

    // Avoid Crash situation
    if ((mappedEP <= 0) || (mappedEP >= HOST_MAX_EPNUM))
        return TRUE;
        
    EnterCriticalSection(&m_csEndPoint);

    // Currently release it only if it is very busy.    
    if ((m_EndPointInUseCount >= 1) || (fForce))
    {
        PHSMUSB_T pOTG;               
        pOTG = (PHSMUSB_T)GetOTGContext();

        m_EndPoint[mappedEP][isOut].usDeviceInfo = 0xffff;
        if (fClearAll)
            m_EndPoint[mappedEP][isOut].usPrevDevInfo = 0xffff;
        m_EndPointInUseCount--;
        m_cBusyPipeList.m_hEP2Handles[mappedEP][isOut] = NULL;
        pPipe->SetMappedEndPoint(0xff);
        if (pOTG != NULL)
        {
            EnterCriticalSection(&pOTG->regCS); 
            OUTREG8(&pOTG->pUsbGenRegs->Index, USB_ENDPOINT(mappedEP));
            // Flush the endpoint
            if (isOut)
            {
                SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR, TXCSR_H_FlushFIFO);
                SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR, TXCSR_H_FlushFIFO);

                DWORD txcsr = (TXCSR_H_AutoSet|TXCSR_H_DMAReqEn|TXCSR_H_FrcDataTog|TXCSR_H_Mode);
                CLRREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR, txcsr);

            }
            else
            {
                SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR, RXCSR_H_FlushFIFO);            
                SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR, RXCSR_H_FlushFIFO);            

                DWORD rxcsr = (RXCSR_H_AutoClr|RXCSR_H_AutoReq|RXCSR_H_DMAReqEn);
                CLRREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR, rxcsr);
            }
            LeaveCriticalSection(&pOTG->regCS); 
        }

            DEBUGMSG(ZONE_DEBUG, (TEXT("ReleasePhysicalEndPoint for EP 0x%x Device 0x%x -> Mapped %d [%s]\r\n"),
            endptDesc.bEndpointAddress, pPipe->GetReservedDeviceAddr(), mappedEP, isOut?TEXT("OUT"):TEXT("IN")));
    }

    LeaveCriticalSection(&m_csEndPoint);
    return TRUE;
}

//-----------------------------------------------------------------------------
UCHAR    CHW::GetCurrentToggleBit(UCHAR mappedEP, UCHAR isIn)
//
//  Description: Get the upcoming transfer toggle bit value
//
//  Parameter:  mappedEP - endpoint 
//              isIn - if it is IN transcation or not
{
    DWORD DataToggle;
    PHSMUSB_T pOTG;        

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::GetCurrentToggleBit failed\r\n")));
        return 0xff;
    }  

    if (isIn)
    {
        DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR) & RXCSR_H_DataToggle);
    }
    else
    {
        DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR) & TXCSR_H_DataToggle);

    }

    DEBUGMSG(ZONE_DEBUG, (TEXT("GetCurrentToggle bit for mapped EP %d (%s) = DataToggle (0x%x)\r\n"),
        USB_ENDPOINT(mappedEP), isIn? TEXT("IN"):TEXT("OUT"), DataToggle));

    return ((DataToggle != 0)? 1: 0);
}

//------------------------------------------------------------------
BOOL CHW::SignalHubChangeEvent(BOOL fConnect) 
// 
//  Purpose: This is to signal the Hub Change Event
//
{
    if (fConnect)
        SetEvent(m_hUsbHubConnectEvent); 
    else
        SetEvent(m_hUsbHubDisconnectEvent);
    return TRUE;
};
// ******************************************************************
VOID CHW::PowerMgmtCallback( IN BOOL fOff )
//
// Purpose: System power handler - called when device goes into/out of
//          suspend.
//
// Parameters:  fOff - if TRUE indicates that we're entering suspend,
//                     else signifies resume
//
// Returns: Nothing
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{

    PHSMUSB_T pOTG;            

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::PowerMgmt failed\r\n")));
        return;
    }  

    if (pOTG->operateMode != HOST_MODE)
        return;

    SMHCDPdd *pPdd = (SMHCDPdd *)pOTG->pContext[HOST_MODE-1];
    if ( fOff )
    {
        SuspendHostController();
        if (pPdd->m_lpUSBClockProc)
            pPdd->m_lpUSBClockProc(FALSE);        
    }
    else
    {   // resuming...
        if (pPdd->m_lpUSBClockProc)
            pPdd->m_lpUSBClockProc(TRUE);
        ResumeHostController();
        DEBUGMSG(1, (TEXT("SetInterruptEvent for SysIntr 0x%x\r\n"), pOTG->dwSysIntr));
        SetInterruptEvent(pOTG->dwSysIntr);
    }
    
    return;
}
VOID CHW::SuspendHostController()
{
    DEBUGMSG(ZONE_DEBUG, (TEXT("+CHW::SuspendHostController\r\n")));
    // Clock must have started already.
    // Procedure to do:
    // (1) Set the Suspend Bit on the Power D0
    // (2) Stop the USB Clock 
#if 0
    PHSMUSB_T pOTG;        

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::SuspendHostController failed\r\n")));
        return;
    }  

    //pOTG->dwPwrMgmt = MODE_SYSTEM_SUSPEND;
    // Clear the DEVCTL_SESSION at this point
    // This is to avoid the activation of HNP during suspend which does not suppose to do.    
    //CLRREG8(&pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);
    //SETREG8(&pOTG->pUsbGenRegs->Power, POWER_SUSPENDM);

    //Keep the transceiver on during system suspend and when a USB device is attached
    //SETREG8(&pOTG->pUsbGenRegs->Power, POWER_EN_SUSPENDM);
#endif
    DEBUGMSG(ZONE_DEBUG, (TEXT("-CHW::SuspendHostController\r\n")));
}

VOID CHW::ResumeHostController()
{
#if 0
    PHSMUSB_T pOTG;        

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::ResumeHostController failed\r\n")));
        return;
    }  

    DEBUGMSG(ZONE_DEBUG, (TEXT("CHW::ResumeHostController\r\n")));
    pOTG->dwPwrMgmt = MODE_SYSTEM_RESUME;
    SETREG8(&pOTG->pUsbGenRegs->Power, POWER_RESUME);
#endif
}

//-------------------------------------------------------------------------
void CHW::SetDeviceAddress(UCHAR mappedEP, UCHAR ucDevAddress, UCHAR ucHubAddress, UCHAR ucHubPort, UCHAR outdir)
//
//  Purpose: Set the device address
//
//  Parameter: usAddress - device address
//
//  Return: nil
{
    PHSMUSB_T pOTG;
    pOTG = (PHSMUSB_T)GetOTGContext();    

    OUTREG8(&pOTG->pUsbGenRegs->Index, USB_ENDPOINT(mappedEP));
    //OUTREG8(&pOTG->pUsbGenRegs->FAddr, ucAddress);

    DEBUGMSG(ZONE_HCD, (TEXT("SetDeviceAddress DevAddr 0x%x, HubAddress 0x%x, HubPort 0x%x\r\n"),
        ucDevAddress, ucHubAddress, ucHubPort));

    if (USB_ENDPOINT(mappedEP) == 0)
    {
        OUTREG8(&pOTG->pUsbGenRegs->ep[0].TxFuncAddr, ucDevAddress);
        OUTREG8(&pOTG->pUsbGenRegs->ep[0].RxFuncAddr, ucDevAddress);

        OUTREG8(&pOTG->pUsbGenRegs->ep[0].TxHubAddr, 0x80|ucHubAddress);
        OUTREG8(&pOTG->pUsbGenRegs->ep[0].RxHubAddr, 0x80|ucHubAddress);

        OUTREG8(&pOTG->pUsbGenRegs->ep[0].TxHubPort, ucHubPort);
        OUTREG8(&pOTG->pUsbGenRegs->ep[0].RxHubPort, ucHubPort);


    }
    else 
    {
        if (outdir) // OUT
        {
            OUTREG8(&pOTG->pUsbGenRegs->ep[INDEX(mappedEP)].TxFuncAddr, ucDevAddress);
            OUTREG8(&pOTG->pUsbGenRegs->ep[INDEX(mappedEP)].TxHubAddr, 0x80|ucHubAddress);
            OUTREG8(&pOTG->pUsbGenRegs->ep[INDEX(mappedEP)].TxHubPort, ucHubPort);
        }
        else
        {
            OUTREG8(&pOTG->pUsbGenRegs->ep[INDEX(mappedEP)].RxFuncAddr, ucDevAddress);
            OUTREG8(&pOTG->pUsbGenRegs->ep[INDEX(mappedEP)].RxHubAddr, 0x80|ucHubAddress);
            OUTREG8(&pOTG->pUsbGenRegs->ep[INDEX(mappedEP)].RxHubPort, ucHubPort);

        }
    }
    DEBUGMSG(ZONE_HCD, (TEXT("CHW::SetDeviceAddress(ep(0x%x), dev addr(0x%x), hub addr(0x%x), port (0x%x)), index(0x%x)\r\n"), mappedEP, ucDevAddress, ucHubAddress, ucHubPort, INDEX(mappedEP)));
}
BOOL CHW::IsDMASupport(void)
{
    PHSMUSB_T pOTG;        
    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::IsDMASupport failed\r\n")));
        return FALSE;
    }

    SMHCDPdd * pPdd = (SMHCDPdd *)pOTG->pContext[HOST_MODE-1];

    return (pPdd->bDMASupport);
}

DWORD CHW::GetDMAMode(void)
{
    PHSMUSB_T pOTG;        
    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::GetDMAMode failed\r\n")));
        return FALSE;
    }

    SMHCDPdd * pPdd = (SMHCDPdd *)pOTG->pContext[HOST_MODE-1];

    return (pPdd->dwDMAMode);
}
//-------------------------------------------------------------------------
BOOL CHW::IsHostConnect(void)
//
//  Purpose: Check if the connection is still valid
//
//  Parameters: nil
//
//  Return: TRUE - connect, FALSE - not valid
{
    PHSMUSB_T pOTG;    
    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_HCD, (TEXT("CHW::IsHostConnect failed\r\n")));
        return FALSE;
    }

    UCHAR devctl = INREG8(&pOTG->pUsbGenRegs->DevCtl);
    if ((devctl & DEVCTL_SESSION) && (devctl & DEVCTL_HOSTMODE))
    {
        DEBUGMSG(ZONE_HCD, (TEXT("IsHostConnect return TRUE\r\n")));
        return TRUE;
    }

    DEBUGMSG(ZONE_DEBUG, (TEXT("IsHostConnect return devctl = 0x%x\r\n"), devctl));
    DEBUGMSG(ZONE_HCD, (TEXT("IsHostConnect return FALSE\r\n")));
    return FALSE;
}

//------------------------------------------------------------------------
DWORD CHW::GetRxCount(void *pContext, IN const UCHAR endpoint)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)(pContext);
    UCHAR csrIndex = INDEX(endpoint);
    DWORD dwCount;
    DEBUGMSG(ZONE_HCD, (TEXT("GetRxCount for endpoint %d\r\n"), endpoint));
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failure to restore configure\r\n")));
        return 0;
    }

    // Set the Index Register
    EnterCriticalSection(&pOTG->regCS); 
    OUTREG8(&pOTG->pUsbGenRegs->Index, csrIndex);
    if (USB_ENDPOINT(endpoint) != 0)
        dwCount = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount);
    else
        dwCount = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.Count0);
    
    LeaveCriticalSection(&pOTG->regCS);    
    return dwCount;
}
//-------------------------------------------------------------------------
DWORD CHW::CheckDMAResult(void *pContext, IN const UCHAR channel)
// 
//  Purpose: Check if there is any DMA problem.
//
{
    PHSMUSB_T pOTG = (PHSMUSB_T)(pContext);
    DWORD dmacntl;
    DWORD dwError = USB_NO_ERROR;
    DEBUGMSG(ZONE_HCD, (TEXT("+CheckDMAResult for channel %d\r\n"), channel));
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failure to restore configure\r\n")));
        return FALSE;
    }
    dmacntl = INREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl);

    if (dmacntl & DMA_CNTL_BusError)
    {
        RETAILMSG(1, (TEXT("Device Not Responding\r\n")));
        dwError = USB_DEVICE_NOT_RESPONDING_ERROR;
    }

    return dwError;
}

//-------------------------------------------------------------------------
BOOL CHW::RestoreRxConfig(void *pContext, IN const UCHAR endpoint)
// 
//  Purpose: Restore back to Normal FIFO state
//
{
    PHSMUSB_T pOTG = (PHSMUSB_T)(pContext);
    USHORT rxcsr = 0;
    UCHAR csrIndex = INDEX(endpoint);    
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failure to restore configure\r\n")));
        return FALSE;
    }

    // Set the Index Register
    EnterCriticalSection(&pOTG->regCS); 
    OUTREG8(&pOTG->pUsbGenRegs->Index, csrIndex);

    rxcsr = (RXCSR_H_AutoClr|RXCSR_H_AutoReq|RXCSR_H_DMAReqEn);
    CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, rxcsr);
    LeaveCriticalSection(&pOTG->regCS);    
    return TRUE;
}

//
BOOL CHW::UpdateDataToggle(IN CPipe * const pPipe, BOOL fResetDataToggle, BOOL fUpdateDataToggle)
// 
//  Purpose: Update the Endpoint FIFO toggle
//
//  Parameter: pPipe - associated transfer pipe
//             fResetDataToggle - indicate if ClrDataToggle (Reset to 0) is required.  
//             fUpdateDataToggle - update the toggle according to the pPipe->GetDataToggle bit

//  Return: TRUE - success, FALSE - failure
//  
{
    PHSMUSB_T pOTG;    

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::ConfigEP failed\r\n")));
        return FALSE;
    }  

    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    UCHAR endpoint = USB_ENDPOINT(endptDesc.bEndpointAddress);
    UCHAR mappedEP = pPipe->GetMappedEndPoint();
    UCHAR outdir   = USB_ENDPOINT_DIRECTION_OUT(endptDesc.bEndpointAddress);


    EnterCriticalSection(&pOTG->regCS);
    // Set the Index Register
    OUTREG8(&pOTG->pUsbGenRegs->Index, USB_ENDPOINT(mappedEP));

    if (!pPipe->IsDataToggle())
    {
        LeaveCriticalSection(&pOTG->regCS);
        return TRUE;
    }

    if (endpoint != 0)
    {
        if (outdir)
        {
            if (fResetDataToggle)
            {
                OUTREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR, 0);
                SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR,
                    TXCSR_H_ClrDataTog|TXCSR_H_FlushFIFO);
            }
            else if (fUpdateDataToggle)
            {
                DWORD dwDataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR) & TXCSR_H_DataToggle);
                UCHAR ucNextDataToggle = ((dwDataToggle != 0)? 1: 0);
                if (pPipe->GetDataToggle() != ucNextDataToggle)
                {
                    if (pPipe->GetDataToggle() == 0)                    
                        SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR, TXCSR_H_ClrDataTog);
                    else
                        SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR, 
                            TXCSR_H_DataTogWrEn|TXCSR_H_DataToggle);
                }
            }

        }
        else
        {
            if (fResetDataToggle)
            {
                OUTREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR, 0);
                SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR, 
                    RXCSR_H_ClrDataTog|RXCSR_H_FlushFIFO);
            }
            else if (fUpdateDataToggle)
            {
                DWORD dwDataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR) & RXCSR_H_DataToggle);
                UCHAR ucNextDataToggle = ((dwDataToggle != 0)? 1: 0);
                if (pPipe->GetDataToggle() != ucNextDataToggle)
                {
                    if (pPipe->GetDataToggle() == 0)
                        SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR, RXCSR_H_ClrDataTog);
                    else
                        SETREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR, 
                            RXCSR_H_DataTogWrEn|RXCSR_H_DataToggle);
                        DEBUGMSG(ZONE_DEBUG, (TEXT("UpdateDataToggle for EP 0x%x (mappedEP 0x%x) DeviceAddr(0x%x)=> RxCSR(0x%x)\r\n"),
                        endpoint, mappedEP, pPipe->GetReservedDeviceAddr(), INREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR)));

                }
            }

        }
    }
    
    LeaveCriticalSection(&pOTG->regCS);
    return TRUE;
}

//-------------------------------------------------------------------------
BOOL CHW::InitFIFO(IN CPipe * const pPipe)
// 
//  Purpose: Initialize the FIFO
//
//  Parameter: pPipe - associated transfer pipe

//  Return: TRUE - success, FALSE - failure
//  
{
    PHSMUSB_T pOTG;    
    int i;

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::ConfigEP failed\r\n")));
        return FALSE;
    }  

    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    UCHAR endpoint = USB_ENDPOINT(endptDesc.bEndpointAddress);
    UCHAR mappedEP = pPipe->GetMappedEndPoint();
    UCHAR outdir   = USB_ENDPOINT_DIRECTION_OUT(endptDesc.bEndpointAddress);


    EnterCriticalSection(&pOTG->regCS);
    // Set the Index Register
    OUTREG8(&pOTG->pUsbGenRegs->Index, USB_ENDPOINT(mappedEP));

    USHORT size			  = max(endptDesc.wMaxPacketSize, 8);    
    DWORD pwr_of_two	  = size>>3;
    DWORD fifo_size_index = 0;
    // To avoid the change dynamically, each endpoint has been reserved for
    // max packet size *2 
    DWORD fifo_addr_index = (1024*USB_ENDPOINT(mappedEP))>>3;         
    
    i = 0;
    while (i < 10 ) // max = 4096 i.e. i = 9
    {
        if (pwr_of_two & (1<<i)) 
        {
            fifo_size_index = i;
            break;
        }        
        i++;
    }

    DEBUGMSG(ZONE_HCD|ZONE_DEBUG, (TEXT("Fifo size = %d, index = %d\r\n"), size, fifo_size_index));
    if (endpoint != 0)
    {
        if (outdir)
        {
            OUTREG8(&pOTG->pUsbGenRegs->TxFIFOsz, fifo_size_index);
            // Add index 512 = 4096 bytes for seperate the addr from Rx
            OUTREG16(&pOTG->pUsbGenRegs->TxFIFOadd, fifo_addr_index+0x40);                        
        }
        else
        {
            OUTREG8(&pOTG->pUsbGenRegs->RxFIFOsz, fifo_size_index);
            OUTREG16(&pOTG->pUsbGenRegs->RxFIFOadd, fifo_addr_index);            

        }
    }
    else
    {
        // endpoint 0, set the FIFO size to be 64 bytes
        OUTREG8(&pOTG->pUsbGenRegs->TxFIFOsz, fifo_size_index);
        OUTREG16(&pOTG->pUsbGenRegs->TxFIFOadd, fifo_addr_index);
        fifo_addr_index = fifo_addr_index + (size>>3);

        OUTREG8(&pOTG->pUsbGenRegs->RxFIFOsz, fifo_size_index);        
        OUTREG16(&pOTG->pUsbGenRegs->RxFIFOadd, fifo_addr_index);            
    }

    DEBUGMSG(ZONE_DEBUG, (TEXT("FIFO Address setup for device ep %d, mapped %d\r\n"), endpoint, USB_ENDPOINT(mappedEP)));
    DEBUGMSG(ZONE_DEBUG, (TEXT("RxFIFOAddr = 0x%x\r\n"), INREG16(&pOTG->pUsbGenRegs->RxFIFOadd)));
    DEBUGMSG(ZONE_DEBUG, (TEXT("TxFIFOAddr = 0x%x\r\n"), INREG16(&pOTG->pUsbGenRegs->TxFIFOadd)));
    DEBUGMSG(ZONE_DEBUG, (TEXT("RxFIFOsz = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->RxFIFOsz)));
    DEBUGMSG(ZONE_DEBUG, (TEXT("TxFIFOsz = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->TxFIFOsz)));
    DEBUGMSG(ZONE_DEBUG, (TEXT("RxCSR = 0x%x\r\n"),INREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].RxCSR)));
    DEBUGMSG(ZONE_DEBUG, (TEXT("TxCSR = 0x%x\r\n"),INREG16(&pOTG->pUsbCsrRegs->ep[USB_ENDPOINT(mappedEP)].CSR.TxCSR)));
    
    LeaveCriticalSection(&pOTG->regCS);
    return TRUE;
}
//-------------------------------------------------------------------------
//
BOOL CHW::ConfigEP(IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor, UCHAR mappedEP,
                   UCHAR bDeviceAddress, UCHAR bHubAddress, UCHAR bHubPort, int speed, UCHAR transferMode, BOOL bClrTog)
//
//  Purpose: Configure the endpoint
//
//  Parameters: lpEndpointDescriptor - Pointer to USB_ENDPOINT_DESCRIPTOR
//              mappedEP - mapped EP
//              bDeviceAddress - address of the device
//              speed - high speed/full speed or low speed
//              transferMode - current transfer mode of the pipe
//              bClrTog - TRUE - clear toggle data, FALSE - no need
//  
//  Return: TRUE - success
{
    PHSMUSB_T pOTG;
    UCHAR endpoint;
    UCHAR protocol;
    UCHAR outdir;
    UCHAR csrIndex;    
    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CHW::ConfigEP failed\r\n")));
        return FALSE;
    }

    endpoint = USB_ENDPOINT(lpEndpointDescriptor->bEndpointAddress);
    protocol = lpEndpointDescriptor->bmAttributes & USB_ENDPOINT_TYPE_MASK;
    outdir = USB_ENDPOINT_DIRECTION_OUT(lpEndpointDescriptor->bEndpointAddress);
  
    DEBUGMSG(ZONE_HCD, (TEXT("ConfigEP TransferMode(0x%x), EP(0x%x), Dev(0x%x), DIR:%s\r\n"),
        transferMode, endpoint, bDeviceAddress, outdir?(TEXT("OUT")):(TEXT("IN"))));

    // Set the Index Register
    EnterCriticalSection(&pOTG->regCS); 
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(mappedEP));    

    if (endpoint == 0) // Endpoint 0, there is no need to do any configuration
    {
        DEBUGMSG(ZONE_HCD, (TEXT("Configure the speed of the device\r\n")));
        DEBUGMSG(ZONE_HCD, (TEXT("CSR0 = 0x%x\r\n"), pOTG->pUsbCsrRegs->ep[0].CSR.CSR0));
        DEBUGMSG(ZONE_HCD, (TEXT("ConfigData=0x%x\r\n"), pOTG->pUsbCsrRegs->ep[0].Config.ConfigData));
        DEBUGMSG(ZONE_HCD, (TEXT("Speed of Device = 0x%x\r\n"), speed));
        DEBUGMSG(ZONE_HCD, (TEXT("ConfigEP: EP0 with bDeviceAddress = %d\r\n"), bDeviceAddress));
        // Set the speed 
        OUTREG8(&pOTG->pUsbCsrRegs->ep[0].Type.Type0, (speed << 6));
        SetDeviceAddress(mappedEP, bDeviceAddress, bHubAddress, bHubPort, outdir);
    }
    else
    {
        csrIndex = INDEX(mappedEP);
        // 1. Setup RxType or TxType
        // 2. Setup RxMaxP or TxMaxP
        // 3. Setup Interval
        // 4. Setup RxCSR or TxCSR - data toggle bit
        // 5. Check and see if flush is required
        // 6. Set the TxFuncAddr/RxFuncAddr
        switch (protocol)
        {
        case (USB_ENDPOINT_TYPE_BULK):
        case (USB_ENDPOINT_TYPE_INTERRUPT):
            {
                if (outdir)
                {
                    USHORT txcsr;
                    // step 1
                    UCHAR txtype = (UCHAR)RxTxTYPE(speed, protocol, endpoint);
                    OUTREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].Type.TxType, txtype);

                    // step 2                                        
                    OUTREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].TxMaxP, lpEndpointDescriptor->wMaxPacketSize);

                    // step 3 - set to 0 at this point, should change later
                    if (protocol == USB_ENDPOINT_TYPE_BULK)
                        OUTREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].timeout.TxInterval, 0);
                    else
                        OUTREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].timeout.TxInterval, lpEndpointDescriptor->bInterval);

                    // step 4                     
                    // clear AutoSet, DMAReqEn, FrcDataTog
                    txcsr = (TXCSR_H_AutoSet|TXCSR_H_DMAReqEn|TXCSR_H_FrcDataTog|TXCSR_H_Mode);
                    CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, txcsr);

                    // if DMA enable, set the AutoSet = 1, DMAReqEnab = 1
                    if (transferMode != TRANSFER_FIFO)
                    {
                        txcsr = (TXCSR_H_AutoSet|TXCSR_H_DMAReqEn|TXCSR_H_DMAReqMode);
                        SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, txcsr);
                    }

                    // set the mode, clr data tog
                    if (bClrTog)
                        SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_ClrDataTog);

                    // step 5
                    if (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR) & TXCSR_H_FIFONotEmpty)
                    {
                        // Need to check if double buffering, if so, do twice.
                        SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_FlushFIFO);
                    }

                    // Step 6
                    SetDeviceAddress(mappedEP, bDeviceAddress , bHubAddress, bHubPort, outdir);
                    
                }
                else
                {
                    USHORT  rxcsr;
                    
                    // step 0, clear the mode if set in order to enable Rx mode
                    //CLRREG16(&pOTG->pUsbGenRegs->INDEX_REG.hIndex.CSR.TxCSR, TXCSR_H_Mode);
                    //OUTREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, 0);
                    
                    // step 1
                    UCHAR rxtype = (UCHAR)RxTxTYPE(speed, protocol, endpoint);
                    OUTREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].RxType, rxtype);

                    // step 2                    
                    OUTREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxMaxP, lpEndpointDescriptor->wMaxPacketSize);
                    DEBUGMSG(ZONE_HCD, (TEXT("ConfigEP: RxMaxP = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxMaxP)));

                    // step 3 - set to 0 at this point, should change later
                    if (protocol == USB_ENDPOINT_TYPE_BULK)
                        OUTREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].RxInterval, 0);
                    else
                        OUTREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].RxInterval, lpEndpointDescriptor->bInterval);

                    // step 4                     
                    // clear AutoClr, AutoReq, DMAReqMode,DMAReqEn
                    rxcsr = (RXCSR_H_AutoClr|RXCSR_H_AutoReq|RXCSR_H_DMAReqEn);

                    CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, rxcsr);
                    // set data toggle bit
                    if (bClrTog)
                        SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_ClrDataTog);

                                        
                    // step 5
                    if (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR) & RXCSR_H_RxPktRdy)
                    {
                        // Need to check if double buffering, if so, do twice.
                        SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_FlushFIFO);
                    }                    

                    // Step 6
                    SetDeviceAddress(mappedEP, bDeviceAddress, bHubAddress, bHubPort, outdir);
                }
            }
            break;
        case (USB_ENDPOINT_TYPE_CONTROL):
            DEBUGMSG(ZONE_ERROR, (TEXT("CHW::ConfigEP - Control EP (%d) can only happen in EP0\r\n"), endpoint));
            break;
        case (USB_ENDPOINT_TYPE_ISOCHRONOUS):
        default:
            RETAILMSG(1, (TEXT("CHW::ConfigEP - not support of this type\r\n")));
            break;
        }
    }

    LeaveCriticalSection(&pOTG->regCS);    
    return TRUE;

}
void CHW::PrintRxTxCSR(UCHAR MappedEP)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)GetOTGContext();
    UCHAR csrIndex = INDEX(MappedEP);
    USHORT reg1 = 0;
    USHORT reg2 = 0;
    EnterCriticalSection(&pOTG->regCS);
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(MappedEP));
    reg2 = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR);
    reg1 = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR);
    RETAILMSG(1, (TEXT("Print RxCSR:%x TxCSR:%x \r\n"),reg1,reg2));
    LeaveCriticalSection(&pOTG->regCS);
}

//-------------------------------------------------------------------------------------
DWORD CHW::CheckTxCSR(UCHAR MappedEP)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)GetOTGContext();
    UCHAR csrIndex = INDEX(MappedEP);

    EnterCriticalSection(&pOTG->regCS);
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(MappedEP));
    // First check if RxPktRdy bit is set
    // Read the total no of bytes available
    if (USB_ENDPOINT(MappedEP) == 0) // Endpoint 0, read the COUNT0
    {
        USHORT reg = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0);
        if (reg & CSR0_H_NAKTimeout)
        {
            RETAILMSG(1, (TEXT("CSR0 NAK Timeout\r\n")));
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_NAKTimeout);
            // Set the ReqPkt again and see            
            LeaveCriticalSection(&pOTG->regCS);
            return USB_DEVICE_NOT_RESPONDING_ERROR;
        }
    }
    else
    {
        USHORT reg = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR);
        if (reg & TXCSR_H_NAKTimeout)
        {
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_NAKTimeout);
            LeaveCriticalSection(&pOTG->regCS);
            return USB_DEVICE_NOT_RESPONDING_ERROR;
        }
        else if (reg & TXCSR_H_RxStall)
        {
            RETAILMSG(1, (TEXT("TxCSR TxStall Error\r\n")));
            // Not sure if it should clear in here            
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_RxStall);
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_ClrDataTog);
            LeaveCriticalSection(&pOTG->regCS);
            return USB_STALL_ERROR;
        }
        else if (reg & TXCSR_H_Error)
        {
            RETAILMSG(1, (TEXT("TxCSR_H_Error at EP %d, clear it\r\n"), csrIndex));
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_Error);
            LeaveCriticalSection(&pOTG->regCS);
            return USB_DEVICE_NOT_RESPONDING_ERROR;
        }

    }
    LeaveCriticalSection(&pOTG->regCS);
    return USB_NO_ERROR;
}
//-------------------------------------------------------------------------------------
DWORD CHW::CheckRxCSR(UCHAR MappedEP)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)GetOTGContext();
    UCHAR csrIndex = INDEX(MappedEP);

    EnterCriticalSection(&pOTG->regCS);
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(MappedEP));
    // First check if RxPktRdy bit is set
    // Read the total no of bytes available
    if (USB_ENDPOINT(MappedEP) == 0) // Endpoint 0, read the COUNT0
    {
        USHORT reg = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0);
        if (reg & CSR0_H_Error)
        {
            RETAILMSG(1, (TEXT("CSR0 Halt Error\r\n")));
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_Error);
            // Set the ReqPkt again and see            
            LeaveCriticalSection(&pOTG->regCS);
            return USB_DEVICE_NOT_RESPONDING_ERROR;
        }
        else if (reg & CSR0_H_RxStall)
        {
            RETAILMSG(1, (TEXT("CSR0 RxStall Error\r\n")));
            // Not sure if it should clear in here            
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_RxStall);
            LeaveCriticalSection(&pOTG->regCS);
            return USB_STALL_ERROR;
        }        
    }
    else
    {
        USHORT reg = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR);        
        DEBUGMSG(ZONE_HCD, (TEXT("CheckRxCSR = 0x%x\r\n"), reg));
        if (reg & RXCSR_H_Error)
        {
            RETAILMSG(1, (TEXT("RxCSR_H_Error at EP %d, clear it\r\n"), csrIndex));
            RETAILMSG(1, (TEXT("Debug purpose, read the FIFO and see what it is => hack\r\n")));
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_Error);
            RETAILMSG(1, (TEXT("RxType = 0x%x\r\n"), INREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].RxType)));
            RETAILMSG(1, (TEXT("RxInterval = 0x%x\r\n"), INREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].RxInterval)));
            RETAILMSG(1, (TEXT("RxMaxP = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxMaxP)));
            RETAILMSG(1, (TEXT("RxCount = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount)));
            RETAILMSG(1, (TEXT("FIFOSize = 0x%x\r\n"), INREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].Config.FIFOSize)));            
            RETAILMSG(1, (TEXT("TxFIFOSz = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->TxFIFOsz)));
            RETAILMSG(1, (TEXT("RxFIFOSz = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->RxFIFOsz)));
            RETAILMSG(1, (TEXT("TxFIFOadd = 0x%x\r\n"), INREG16(&pOTG->pUsbGenRegs->TxFIFOadd)));
            RETAILMSG(1, (TEXT("RxFIFOadd = 0x%x\r\n"), INREG16(&pOTG->pUsbGenRegs->RxFIFOadd)));
            RETAILMSG(1, (TEXT("EPINFO = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->EPInfo)));

            LeaveCriticalSection(&pOTG->regCS);
            return USB_DEVICE_NOT_RESPONDING_ERROR;
        }
        else if (reg & RXCSR_H_NAKTimeout)
        {           
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_NAKTimeout|RXCSR_H_ReqPkt);
            RETAILMSG(1, (TEXT("NAKTimeout\r\n")));
            LeaveCriticalSection(&pOTG->regCS);
            return USB_DEVICE_NOT_RESPONDING_ERROR;
        }
        else if (reg & RXCSR_H_RxStall)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("RxCSR RxStall Error at ep %d\r\n"), csrIndex));
            // Not sure if it should clear in here            
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_RxStall);
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_ClrDataTog);
            LeaveCriticalSection(&pOTG->regCS);
            return USB_STALL_ERROR;
        }
    }
    LeaveCriticalSection(&pOTG->regCS);
    return USB_NO_ERROR;
}
//-------------------------------------------------------------------------------------
DWORD CHW::ReadDMA(void *pContext, IN const UCHAR endpoint, IN const UCHAR channel, 
                   void *pBuff, DWORD size, DWORD dwMaxPacket, CQTD *pCurTD)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)(pContext);
    USHORT rxcsr;    
    UCHAR  mode = 0;
    DWORD  dwCount = 0;
    DWORD  dwReadSize = 0;    
    UCHAR csrIndex = INDEX(endpoint);    
    EnterCriticalSection(&pOTG->regCS);
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));

    

    dwCount = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount);

#ifndef SHIP_BUILD
    if (dwCount == 0) 
    {
        DWORD cbStart = pCurTD->GetTotTfrSize();
        RETAILMSG(1, (TEXT("ReadDMA failed with TotTfrSize = 0x%x, Buffer = 0x%x\r\n"), cbStart, pCurTD->GetDataSize()));
        memdump(pvData, (USHORT)cbStart, 0);
        RETAILMSG(1, (TEXT("ReadDMA with count = 0, with RxCSR = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));
        // Dump the data
        RETAILMSG(1, (TEXT("Debug here\r\n")));
        // Force dwCount = dwMaxPacket
        //dwCount = dwMaxPacket;
    }
#endif

    if (size < dwCount)
    {
        DWORD dwErr = USB_CLIENT_BUFFER_ERROR;

        // We need to ack back to MDD the failure
        DEBUGMSG(ZONE_WARNING, (TEXT("Warning!!! Not enough buffer size!!!\r\n")));
        dwCount = size;                
        if (size == 0)
        {
            RETAILMSG(1, (TEXT("Assume the next data arrive\r\n")));
            SetRxDataAvail(USB_ENDPOINT(endpoint));
            dwErr = USB_NO_ERROR;
        }
        else
            dwErr = USB_CLIENT_BUFFER_ERROR;
        LeaveCriticalSection(&pOTG->regCS);

        // Using ReadFIFO to clear up the FIFO
        return dwErr;
    }
        
    if ((dwCount >= dwMaxPacket) && (size > dwCount))   
        mode = 1;
    
    DEBUGMSG(ZONE_TRANSFER|ZONE_HCD, (TEXT("ReadDMA(%d): count=0x%x, maxpacket=0x%x, size=0x%x\r\n"), 
        mode, dwCount, dwMaxPacket, size));
    // First clear the REQPKT
    rxcsr = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR); 
    rxcsr &= ~(RXCSR_H_ReqPkt);
    
    if (mode == 0)
    {
        dwReadSize = dwCount;
        rxcsr &= ~(RXCSR_H_AutoReq);
        rxcsr |= (RXCSR_H_AutoClr| RXCSR_H_DMAReqEn);
        pCurTD->SetStatus(STATUS_WAIT_DMA_0_RD_FIFO_COMPLETE);
    }
    else
    {
        dwReadSize = size;
        rxcsr &= ~(RXCSR_H_DMAReqMode);
        rxcsr |= (RXCSR_H_AutoReq| RXCSR_H_AutoClr|RXCSR_H_DMAReqEn);
        pCurTD->SetStatus(STATUS_WAIT_DMA_1_RD_FIFO_COMPLETE);
    }
    OUTREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, rxcsr);
    LeaveCriticalSection(&pOTG->regCS);
        
    pCurTD->SetCurTfrSize(dwReadSize);    
    ProcessDMAChannel((void *)pOTG, endpoint, channel, FALSE, pBuff, dwReadSize, dwMaxPacket);
    
    return dwReadSize;
}

//-------------------------------------------------------------------------------------
DWORD CHW::ReadFIFO(void *pContext, IN const UCHAR endpoint, void *pBuff, DWORD size, int *pRet)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)(pContext);
    DWORD total = 0;
    DWORD remain = 0;
    DWORD dwCount = 0;    
    DWORD dwRead = 0;    
    UCHAR csrIndex;
    // To be implemented
    // First set the index register 
    *pRet = -1;
    EnterCriticalSection(&pOTG->regCS);
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
    csrIndex = INDEX(endpoint);
    if (USB_ENDPOINT(endpoint) == 0) 
    {
        USHORT reg = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0);
        if(reg & CSR0_H_RxPktRdy) 
            dwCount = INREG8(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.Count0);            
    }
    else
    {
        USHORT reg = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR);
        if (reg & RXCSR_H_RxPktRdy)
            dwCount = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount);
    }

    DEBUGMSG(ZONE_TRANSFER, (TEXT("ReadFIFO dwCount=%d, size=%d\r\n"), dwCount, size));
    if (dwCount > size) 
    { 
        DWORD dwErr = USB_CLIENT_BUFFER_ERROR;
        RETAILMSG(1, (TEXT("Warning!!! Not enough buffer size. Need to fix\r\n")));
        RETAILMSG(1, (TEXT("ReadFIFO CriticalSection: OUT\r\n")));
        if (size == 0)
        {
            RETAILMSG(1, (TEXT("Assume the next data arrive\r\n")));
            SetRxDataAvail(USB_ENDPOINT(endpoint));
            *pRet = 0;
            dwErr = USB_NO_ERROR;
        }
        else
            dwErr = USB_CLIENT_BUFFER_ERROR;
        LeaveCriticalSection(&pOTG->regCS);
        return dwErr;
    }
    // Read the data with dwCount
    if (dwCount != 0)  // In case of IN data arrive
    {
        DWORD i = 0;
        DWORD *pData = (DWORD *)pBuff;        
        total = dwCount/4;
        remain = dwCount%4;

		// this is 32-bit align
		for (i = 0; i < total; i++)
		{
			*pData++ = INREG32(&pOTG->pUsbGenRegs->fifo[csrIndex]);
			dwRead = dwRead + 4;
		}
	        
		// Set the pByte equal to the last bytes of data being transferred
		if (remain != 0)
		{
			UCHAR* pUCHAR = (UCHAR*) pData;
			DWORD dwTemp = INREG32(&pOTG->pUsbGenRegs->fifo[csrIndex]);
	        
			while (remain--)
			{
				*pUCHAR++ = (UCHAR) (dwTemp & 0xFF);
				dwTemp>>=8;
				dwRead++;
			}
		}
    }
    else
    {
        RETAILMSG(1, (TEXT("ReadFIFO USB Not Responding\r\n")));
//        return USB_DEVICE_NOT_RESPONDING_ERROR;
    }

#if 0
    {
        DWORD i = 0;
        UCHAR *pByte = (UCHAR *)pBuff;
        RETAILMSG(1, (TEXT("Receive total of %d byte\r\n"), dwRead));
        for (i = 0; i < dwRead; i++)
            RETAILMSG(1, (TEXT("0x%x "), *(pByte+i)));
        RETAILMSG(1, (TEXT("\r\n")));
    }
#endif    
    *pRet = dwRead;
    LeaveCriticalSection(&pOTG->regCS);
    return USB_NO_ERROR;
}

BOOL CHW::ClearDMAChannel(UCHAR channel)
{
    PHSMUSB_T pOTG;
    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        RETAILMSG(1, (TEXT("Failed to ClearDMAChannel(%d)\r\n"), channel));
        return FALSE;
    }

    EnterCriticalSection(&m_csDMAChannel);
    // Write Control
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl, (DWORD)0x00);

    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Addr, (DWORD)0x00);
    // Write count
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Count, (DWORD)0x00);
    LeaveCriticalSection(&m_csDMAChannel);

    return TRUE;
}


BOOL CHW::ProcessDMAChannel(void *pContext, UCHAR endpoint, UCHAR channel, BOOL IsTx, void *ppData, DWORD size, DWORD dwMaxPacket)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)(pContext);
    DWORD dmacntl = 0;

    EnterCriticalSection(&m_csDMAChannel);
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl, (DWORD)0x00);

    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Addr, (DWORD)0x00);
    // Write count
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Count, (DWORD)0x00);

    //  Now actual configure the DMA channel
    //  DMA Mode (D2)
    if (size > dwMaxPacket)
        dmacntl |= DMA_CNTL_DMAMode;

    //  Burst Mode (D10-9)
    if (dwMaxPacket >= 64)
        dmacntl |= BURSTMODE_INCR16;
    else if (dwMaxPacket >= 32)
        dmacntl |= BURSTMODE_INCR8;
    else if (dwMaxPacket >= 16)
        dmacntl |= BURSTMODE_INCR4;
    else
        dmacntl |= BURSTMODE_UNSPEC;

    //  Direction (D1) - for sure it is Tx endpoint
    //  Interrupt Enable (D3)
    //  Enable DMA (D0)
    dmacntl |= (DMA_CNTL_Enable|DMA_CNTL_InterruptEnable);
    if (IsTx)
        dmacntl |= DMA_CNTL_Direction;

    //  Set endpoint number
    dmacntl |= (INDEX(endpoint) << 4);

    DEBUGMSG(ZONE_HCD, (TEXT("DMA Channel %d\r\n"), channel));
    DEBUGMSG(ZONE_HCD, (TEXT("DMA Channel(%d), Addr (0x%x), Count (0x%x), Cntl(0x%x), ep %d\r\n"),
        channel, (DWORD)ppData, size, dmacntl, INDEX(endpoint)));
    DEBUGMSG(ZONE_HCD, (TEXT("DMA EP %d from 0x%x to 0x%x\r\n"), INDEX(endpoint), (DWORD)ppData, (DWORD)ppData+size));

    // Write address
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Addr, (DWORD)ppData);

    // Write count
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Count, (DWORD)size);

    // Write Control
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl, (DWORD)dmacntl);
    LeaveCriticalSection(&m_csDMAChannel);

    return TRUE;
}

//---------------------------------------------------------------------------------------
BOOL CHW::WriteDMA(void * pContext, UCHAR endpoint, UCHAR channel, void *ppData, DWORD size, DWORD dwMaxPacket, void *pData)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)pContext;
    CQTD *pQTD = (CQTD *)pData;
    CPipe *pPipe = pQTD->GetQH()->GetPipe();        
    UCHAR csrIndex = INDEX(endpoint);
    USHORT txcsr;    
    
    DEBUGMSG(ZONE_TRANSFER|ZONE_HCD, (TEXT("WriteDMA: maxpacket=0x%x, size=0x%x\r\n"), 
        dwMaxPacket, size));

    EnterCriticalSection(&pOTG->regCS);
    // First configure the mode
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));    
    txcsr = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR);

    txcsr &= ~(TXCSR_H_AutoSet|TXCSR_H_DMAReqMode|TXCSR_H_DMAReqEn);
    txcsr |= TXCSR_H_Mode;
    OUTREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, txcsr);

    txcsr = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR);    
    
    if(txcsr & TXCSR_H_RxStall)
    {
       CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_RxStall);
    }

    if (size <= dwMaxPacket)
    {
        // Disable autoset
        txcsr &= ~(TXCSR_H_AutoSet|TXCSR_H_DMAReqMode);
        txcsr |= TXCSR_H_DMAReqEn;        
        if (pPipe->GetTransferMode() == TRANSFER_DMA0)
            pQTD->SetStatus(STATUS_WAIT_DMA_WR_RESPONSE);
        else
            pQTD->SetStatus(STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_NOTRDY);
    }
    else
    {
        txcsr |= (TXCSR_H_AutoSet|TXCSR_H_DMAReqMode|TXCSR_H_DMAReqEn);
        // Last Packet is Maxpacket size
        if (size % dwMaxPacket == 0)
            pQTD->SetStatus(STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_RDY);
        else // you still need to set the TxPktRdy one more time
            pQTD->SetStatus(STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_NOTRDY);
            
    }

    OUTREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, txcsr);

    LeaveCriticalSection(&pOTG->regCS);
    return(ProcessDMAChannel((void *)pOTG, endpoint, channel, TRUE, ppData, size, dwMaxPacket));
    
}
//-------------------------------------------------------------------------
BOOL CHW::WriteFIFO(void *pContext, IN const UCHAR endpoint, void *pData, DWORD size)
{
    PHSMUSB_T pOTG = (PHSMUSB_T)pContext;
    int total  = size/4;
    int remain = size%4;
    int i;
    DWORD *pDword = (DWORD *)pData;
    UCHAR csrIndex;

    //memdump((uchar *)pData, (USHORT)size, 0);
    // Critical section would be handled outside
    DEBUGMSG(ZONE_HCD, (TEXT("WriteFIFO: total (0x%x), remain (0x%x), size(0x%x)\r\n"), total, remain, size));
    // Set Index register again
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
    csrIndex = INDEX(endpoint);

    // Zero-length data transferred
    if (size == 0)
        return TRUE;

    // this is 32-bit align
    for (i = 0; i < total; i++)
    {
        OUTREG32(&pOTG->pUsbGenRegs->fifo[csrIndex], *pDword++);
    }

    if (remain != 0)
    {
        // Pointer to the first byte of data
        USHORT *pWORD =(USHORT *)pDword;

        // Finally if there is remain
        if (remain & 0x2)  // either 2 or 3
        {
            // Write 2 bytes to there
            OUTREG16(&pOTG->pUsbGenRegs->fifo[csrIndex], *pWORD++);        
        }

        if (remain & 0x1)
        {
            // Write 1 byte to there
            OUTREG8(&pOTG->pUsbGenRegs->fifo[csrIndex], *((UCHAR*)pWORD));
        }
    }

	return TRUE;       
}

//-------------------------------------------------------------------------
BOOL CHW::SendOutDMA(IN const UCHAR endpoint, void *pData)
//
//  Purpose: After all the data has been put into FIFO through DMA,
//           we can flush the things by writing a TxPktRdy bit
//
{
    CQTD *pQTD = (CQTD *)pData;
    PHSMUSB_T pOTG;    
    BOOL    bResult = FALSE;
    UCHAR   csrIndex;

#ifdef DEBUG
    CPipe *pPipe = pQTD->GetQH()->GetPipe();  
#endif

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failed to read the FIFO for ep %d\r\n"), endpoint));
        return bResult;
    }
        
    // Set Index Register
    EnterCriticalSection(&pOTG->regCS);    
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
    csrIndex = INDEX(endpoint);

#ifdef DEBUG
    DWORD DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR) & TXCSR_H_DataToggle);
    DEBUGMSG(ZONE_HCD|0, (TEXT("[DataToggle]:SendOutDMA:Data OUT Trasaction on EP %d Device Addr %d DataToggle = 0x%x\r\n"), 
                csrIndex, pPipe->GetReservedDeviceAddr(), DataToggle));
#endif

    // 2. Set the SetupPkt & TxPtRdy bits
    SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_TxPktRdy);
    
    DEBUGMSG(ZONE_HCD|ZONE_DEBUG, (TEXT("SendOutDMA at ep %d (index %d) TxPktRdy\r\n"), endpoint, csrIndex));

    // 3. Wait for transfer interrupt signal
    pQTD->SetStatus(STATUS_WAIT_RESPONSE);            
    LeaveCriticalSection(&pOTG->regCS);

    return TRUE;

}
//-------------------------------------------------------------------------
//
BOOL CHW::ProcessTD(IN const UCHAR endpoint, void *pData)
//
//  Purpose: Process the new TD to be request to device.
//
//  Parameters: endpoint - Endpoint to be transferred
//              pQTD - pointer to CQTD structure
//  
//  Return: TRUE - success
{
    CQTD *pQTD = (CQTD *)pData;
    CPipe *pPipe = pQTD->GetQH()->GetPipe();        
    PHSMUSB_T pOTG;
    DWORD dwType;
    BOOL    bResult = FALSE;
    UCHAR   csrIndex;
    
    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failed to read the FIFO for ep %d\r\n"), endpoint));
        return bResult;
    }

    dwType = pQTD->GetTDType();
        
    
    switch(dwType) 
    {
    case (TD_SETUP):
        if (endpoint != 0) 
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("Control Pipe for ep %d not support\r\n"), endpoint));
            bResult = FALSE;
        }
        else
        {
                        
            // 1. Load 8 bytes of data into FIFO
            // 1.a. Get the virtual address of buffer and corresponding start address (must be 0)
            UCHAR *pvData = (UCHAR *)pQTD->GetVAData();
            DWORD cbStart = pQTD->GetTotTfrSize();
            DWORD cbTemp = cbStart;
            DWORD cbToTransfer = pQTD->GetDataSize() - cbStart;

            // 1.b. Check the size, if not equal to 8 bytes, warning!!!
            if (cbToTransfer != 8)
                DEBUGMSG(ZONE_ERROR, (TEXT("Warning!! Setup Data is not 8 bytes, it is %d bytes!!!\r\n"), pQTD->GetDataSize()));
            
            // Set Index Register
            EnterCriticalSection(&pOTG->regCS);    
            OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
            csrIndex = INDEX(endpoint);
            
            memdump((UCHAR *)(pvData+cbTemp), 8, 0);
            // 1.c Now put 8 bytes data into FIFO for ep 0            
            WriteFIFO((void *)pOTG, 0, (DWORD *)(pvData+cbTemp), 8);                      

            // Set current tfr size
            pQTD->SetCurTfrSize(8);

            // 1.d Clear anything before access - as it has some invalid IN transaction from analyzer
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_RxPktRdy|CSR0_H_ReqPkt|CSR0_H_StatusPkt);

#ifdef DEBUG
            {
                DWORD DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0) & CSR0_H_DataToggle);
                DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]:Request:Setup OUT Trasaction on EP 0 Device Addr %d DataToggle = 0x%x\r\n"), 
                        pPipe->GetReservedDeviceAddr(), DataToggle));
            }
#endif

            // 2. Set the SetupPkt & TxPtRdy bits
            DEBUGMSG(ZONE_HCD, (TEXT("Setup Packet requested with CSR0 = 0x%x\r\n"), 
                INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0)));
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, (CSR0_H_SetupPkt|CSR0_H_TxPktRdy));

            // 3. Wait for transfer interrupt signal
            pQTD->SetStatus(STATUS_WAIT_RESPONSE);            
            bResult = TRUE;
            LeaveCriticalSection(&pOTG->regCS);
        }
        break;         

    case (TD_DATA_IN):
        {
            BOOL bRequestData = TRUE;

            DEBUGMSG(ZONE_HCD, (TEXT("Request IN Transfer for EP %d, Dev %d size %d, PipeMapped %d, RxCSR(0x%x)\r\n"),
                INDEX(endpoint), pPipe->GetReservedDeviceAddr(), pQTD->GetDataSize() - pQTD->GetTotTfrSize(), USB_ENDPOINT(pPipe->GetMappedEndPoint()),
                INREG16(&pOTG->pUsbCsrRegs->ep[INDEX(endpoint)].RxCSR)));

            // Check if there is any data already available
            if (GetRxDataAvail(USB_ENDPOINT(endpoint)))
            {
                pQTD->SetStatus(STATUS_WAIT_RESPONSE);            
                // Set Index Register
                EnterCriticalSection(&pOTG->regCS);    
                OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
                csrIndex = INDEX(endpoint);

                //Check if RxCount
                DWORD dwTotal = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount);
                // We check the RxCount if it is smaller than data size since i found it sometimes,
                // it gives weird values.
                //RETAILMSG(1, (TEXT("ProcessTD:DataIN: RxDataAvail =%d, Count = %d, RxCSR = 0x%x\r\n"),
                //      GetRxDataAvail(USB_ENDPOINT(endpoint)),
                //      dwTotal,
                //      INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));

                if (((dwTotal > 0) && (dwTotal <= pQTD->GetDataSize())) &&
                    (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR) & RXCSR_H_RxPktRdy))
                {   
                    bRequestData = FALSE;
                    ClrRxDataAvail(USB_ENDPOINT(endpoint));
                    Host_ProcessEPxRx((PVOID)pOTG, USB_ENDPOINT(endpoint));                    
                }
                else if (dwTotal == 0)
                {
                    RETAILMSG(1, (TEXT("No data available with RxCSR = 0x%x\r\n"),
                        INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));
                    //Sleep(1);
                }

                LeaveCriticalSection(&pOTG->regCS);
            }
        
            if (bRequestData)
            {
                // Set Index Register
                EnterCriticalSection(&pOTG->regCS);    
                OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
                csrIndex = INDEX(endpoint);

                if (USB_ENDPOINT(endpoint) == 0)
                {               
#ifdef DEBUG
                    DWORD DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0) & CSR0_H_DataToggle);
                    DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]: Request:Data IN Trasaction on EP 0 Device Addr %d DataToggle = 0x%x\r\n"), 
                        pPipe->GetReservedDeviceAddr(), DataToggle));
#endif
                    // 0. Clear anything before access - as it has some invalid IN transaction from analyzer
                    CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_RxPktRdy|CSR0_H_StatusPkt|CSR0_H_SetupPkt);
                    // 1. Set ReqPkt (CSR0.D5) = 1
                    SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_ReqPkt);
                    
                }
                else
                {
                    // 0. Clear any thing before access - as it has some invalid IN transcation from analyzer
                    //CLRREG16(&pOTG->pUsbGenRegs->INDEX_REG.hIndex.RxCSR, RXCSR_H_RxPktRdy);
                    //RETAILMSG(1, (TEXT("Force Toggle to see\r\n")));
                    //SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_DataTogWrEn|RXCSR_H_DataToggle);
#ifdef DEBUG
					DWORD DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR) & RXCSR_H_DataToggle);
                    DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]:Request:IN Transaction on EP %d Device Addr %d DataToggle = 0x%x\r\n"),
                        csrIndex, pPipe->GetReservedDeviceAddr(), DataToggle));

                    USB_ENDPOINT_DESCRIPTOR endpt = pPipe->GetEndptDescriptor();
                    DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]: Set EP 0x%x(mapped 0x%x) DeviceAddr 0x%x Suggest DataToggle=%d, Actual DataToggle=0x%x\r\n"),
                        endpt.bEndpointAddress, pPipe->GetMappedEndPoint(), pPipe->GetReservedDeviceAddr(), pPipe->GetDataToggle(), DataToggle));
#endif
               
                    //Don't know why it doesn't work: Need to Fix that
                    UpdateDataToggle(pPipe, FALSE, TRUE);                    
                    //UpdateDataToggle(pPipe, FALSE, FALSE);                    
                    // 1. Set ReqPkt (CSR0.D5) = 1
                    DEBUGMSG(ZONE_TRANSFER, (TEXT("Request IN data for EP %d, index %d, MappedEP %d Dev %d, Toggle 0x%x\r\n"), USB_ENDPOINT(endpoint), csrIndex,
                             USB_ENDPOINT(pPipe->GetMappedEndPoint()), pPipe->GetReservedDeviceAddr(), 
                             INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR) & RXCSR_H_DataToggle));
                    CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_RxPktRdy);
                    SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_ReqPkt);
                    pQTD->SetStatus(STATUS_WAIT_RESPONSE);                                
                }                
                LeaveCriticalSection(&pOTG->regCS);
            }
        }
        break;
    case (TD_STATUS_OUT):
        DEBUGMSG(ZONE_HCD, (TEXT("Process TD_STATUS_OUT\r\n")));
        if (endpoint != 0) 
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("Control Pipe for ep %d not support\r\n"), endpoint));
            bResult = FALSE;
        }
        else
        {
            // Set Index Register
            EnterCriticalSection(&pOTG->regCS);    
            OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
            csrIndex = INDEX(endpoint);

            // 0. Clear any thing before access - as it has some invalid IN transcation from analyzer
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_RxPktRdy|CSR0_H_ReqPkt|CSR0_H_SetupPkt);

            {   
                UCHAR DataToggle = 0;
                if(INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0) & CSR0_H_DataToggle)
                    DataToggle = 1;//Verify

                //UCHAR DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0) & CSR0_H_DataToggle) >> CSR0_H_DataToggle;
                DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]:Request:Status OUT Trasaction on EP 0 Device Addr %d DataToggle = %d\r\n"), 
                        pPipe->GetReservedDeviceAddr(), DataToggle));
            }

            pQTD->SetCurTfrSize(0);            
            // 1. Set the StatusPkt, TxPktRdy
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, (CSR0_H_StatusPkt|CSR0_H_TxPktRdy));            

            // 2. Wait for transfer interrupt signal
            pQTD->SetStatus(STATUS_WAIT_RESPONSE);
            LeaveCriticalSection(&pOTG->regCS);
            bResult = TRUE;
        }
        break;
    case (TD_DATA_OUT):
        {
            // Set Index Register
            EnterCriticalSection(&pOTG->regCS);    
            OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
            csrIndex = INDEX(endpoint);

            DEBUGMSG(ZONE_TRANSFER, (TEXT("Request OUT Transfer for EP %d, size %d MappedEP %d, Device %d, TxCSR 0x%x\r\n"),
                INDEX(endpoint), pQTD->GetDataSize() - pQTD->GetTotTfrSize(),  USB_ENDPOINT(pPipe->GetMappedEndPoint()),
                pPipe->GetReservedDeviceAddr(), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR)));
            
            // 1. Load data into FIFO
            // 1.a. Get the virtual address of buffer and corresponding start address (must be 0)
            UCHAR *pvData = (UCHAR *)pQTD->GetVAData();
            UCHAR *ppData = (UCHAR *)pQTD->GetPAData();
            DWORD cbStart = pQTD->GetTotTfrSize();
            DWORD cbTemp = cbStart;
            DWORD dwPacketSize = pQTD->GetPacketSize();
            DWORD cbToTransfer;            

            if (pPipe->GetTransferMode() == TRANSFER_DMA1)          
            {
                cbToTransfer = pQTD->GetDataSize() - cbStart;                          
            }
            else
            {
                cbToTransfer = min(pQTD->GetDataSize() - cbStart, dwPacketSize);                           
            }

            // We need to get the max packet size in here            
            DEBUGMSG( ZONE_HCD, (TEXT("Packet Size = 0x%x for ep %d\r\n"), dwPacketSize, endpoint));
            DEBUGMSG( ZONE_HCD, (TEXT("Process TD_DATA_OUT at endpoint %d size %d\r\n"), endpoint, cbToTransfer));
            //memdump((UCHAR *)(pvData+cbTemp), (USHORT)cbToTransfer, 0);

            // 1.c Clear any thing before access - as it has some invalid OUT transcation from analyzer
            if (endpoint == 0)
                CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_TxPktRdy);
            else
                CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_TxPktRdy);
            //SETREG16(&pOTG->pUsbGenRegs->INDEX_REG.hIndex.CSR.TxCSR, TXCSR_H_ClrDataTog);
            
            // 1.d Set the current size to be transferred.
            pQTD->SetCurTfrSize(cbToTransfer);
            

//            UpdateDataToggle(pPipe, FALSE, TRUE);

            LeaveCriticalSection(&pOTG->regCS);

            if (endpoint == 0)
                DEBUGMSG(ZONE_HCD, (TEXT("Request OUT ProcessTD EP %d Dev %d data Toggle = 0x%x\r\n"), 
                    endpoint, pPipe->GetReservedDeviceAddr(), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0) & CSR0_H_DataToggle));
            else
                DEBUGMSG(ZONE_HCD, (TEXT("Request OUT ProcessTD EP %d Dev %d data Toggle = 0x%x\r\n"), 
                    endpoint, pPipe->GetReservedDeviceAddr(), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR)& TXCSR_H_DataToggle));

            // Dump the TxCSR before out 
            // 1.e Now put data into FIFO for ep 
            if ((pPipe->GetTransferMode() != TRANSFER_FIFO) && (USB_ENDPOINT(endpoint) != 0) && (cbToTransfer != 0))
            {    
                if (pQTD->GetStatus() == STATUS_PROCESS_DMA)
                {                                    
                    UCHAR channel = pPipe->m_pCMhcd->AcquireDMAChannel(pPipe);
                    if (channel == 0xff)
                    {
                        RETAILMSG(1, (TEXT("Failed to get the DMA channel, goto to FIFO mode\r\n")));
                        goto PROCESS_WRITE_FIFO;
                    }
                    UpdateDataToggle(pPipe, FALSE, TRUE);
					
                    DEBUGMSG(ZONE_TRANSFER, (TEXT("ProcessTD::WriteDMA at ep %d (mapped %d), size %d channel %d\r\n"), 
                        endpoint, pPipe->GetMappedEndPoint(), cbToTransfer, channel));                    
                    bResult = WriteDMA((void *)pOTG, pPipe->GetMappedEndPoint(), channel, (DWORD *)(ppData+cbTemp), 
                        cbToTransfer, dwPacketSize, (void *)pQTD);                                        
                }
                else
                    RETAILMSG(1, (TEXT("Something wrong with status %d!!!\r\n"), pQTD->GetStatus()));
            }
            else 
            {                
PROCESS_WRITE_FIFO:
                EnterCriticalSection(&pOTG->regCS);
                WriteFIFO((void *)pOTG, endpoint, (DWORD *)(pvData+cbTemp), cbToTransfer);                      

                // 2. Set the SetupPkt & TxPtRdy bits
                if (endpoint == 0)
                    DEBUGMSG(ZONE_HCD, (TEXT("Data OUT Packet requested with CSR0 = 0x%x\r\n"), 
                        INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0)));
                else
                    DEBUGMSG(ZONE_HCD, (TEXT("Data OUT Packet requested with TxCSR = 0x%x\r\n"), 
                        INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR)));

                if (endpoint == 0)
                {
#ifdef DEBUG
                    DWORD DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0) & CSR0_H_DataToggle);
                    DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]:Request:Data OUT Trasaction on EP %d Device Addr %d DataToggle = 0x%x\r\n"), 
                                csrIndex, pPipe->GetReservedDeviceAddr(), DataToggle));
#endif
                    UpdateDataToggle(pPipe, FALSE, TRUE);
                    SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_TxPktRdy);                
                }
                else
                {                                        
#ifdef DEBUG
                    DWORD DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR) & TXCSR_H_DataToggle);
                    DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]:Request:Data OUT Trasaction on EP %d Device Addr %d DataToggle = 0x%x\r\n"), 
                                    csrIndex, pPipe->GetReservedDeviceAddr(), DataToggle));
#endif
                    UpdateDataToggle(pPipe, FALSE, TRUE);
                    DEBUGMSG(ZONE_TRANSFER, (TEXT("Request OUT data for ep %d size %d\r\n"), csrIndex, cbToTransfer));
                    SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_TxPktRdy);                
#if 1
                    if (cbToTransfer > 0)                  
                        pQTD->SetStatus(STATUS_WAIT_RESPONSE);            
                    else
                    {
                        UINT32 timeout = 100000;
                        while (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR) & TXCSR_H_TxPktRdy && timeout-- > 0);

                        if (timeout == 0) {
                            RETAILMSG(1,(L"CHW::ProcessTD: TIMEOUT Sending ZERO_LENGTH PACKET 0x%04X;\r\n",
                            INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR)));
                        }
					
                        pQTD->SetStatus(STATUS_COMPLETE);
                        pPipe->CheckForDoneTransfers();
                    }
#else
                    pQTD->SetStatus(STATUS_WAIT_RESPONSE);            
#endif

                }

                // 3. Wait for transfer interrupt signal                
                LeaveCriticalSection(&pOTG->regCS);
                bResult = TRUE;
            }
        }
        break;

    case (TD_STATUS_IN):
        DEBUGMSG(ZONE_HCD, (TEXT("Process TD_STATUS_IN\r\n")));
        if (endpoint != 0) 
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("Control Pipe for ep %d not support\r\n"), endpoint));
            bResult = FALSE;
        }
        else
        {
            // Set Index Register
            EnterCriticalSection(&pOTG->regCS);    
            OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
            csrIndex = INDEX(endpoint);

            // 0. Clear any thing before access - as it has some invalid IN transcation from analyzer
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, CSR0_H_RxPktRdy|CSR0_H_ReqPkt|CSR0_H_SetupPkt);

#ifdef DEBUG
            {
                DWORD DataToggle = (INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0) & CSR0_H_DataToggle);
                DEBUGMSG(ZONE_HCD, (TEXT("[DataToggle]:Request:Status IN Trasaction on EP 0 Device Addr %d DataToggle = 0x%x\r\n"), 
                        pPipe->GetReservedDeviceAddr(), DataToggle));
            }
#endif

            // 1. Set the StatusPkt, TxPktRdy
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.CSR0, (CSR0_H_StatusPkt|CSR0_H_ReqPkt));            

            // 2. Wait for transfer interrupt signal
            pQTD->SetStatus(STATUS_WAIT_RESPONSE);
            LeaveCriticalSection(&pOTG->regCS);
            bResult = TRUE;
        }

        break;
    
    default:
        break;
    }
                
    return bResult;

}

//********************************************************************************
void CHW::ResetEndPoint(UCHAR endpointAddress)
//
//  Purpose: Flush the endpoint due to error, called by ResetPipe
//
//  Parameter: endpointAddress - endpoint address
//
{
    PHSMUSB_T pOTG;
    DWORD csrIndex;
    DWORD dir;
    UCHAR endpoint;

    pOTG = (PHSMUSB_T)GetOTGContext();
    if (pOTG == NULL)
    {
        RETAILMSG(1, (TEXT("Failed to read the FIFO for ep %d\r\n"), endpointAddress));
        return;
    }

    csrIndex = INDEX(endpointAddress);
    dir = USB_ENDPOINT_DIRECTION_IN(endpointAddress);
    endpoint = USB_ENDPOINT(endpointAddress);

    EnterCriticalSection(&pOTG->regCS); 
#if 0
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));

    DEBUGMSG(ZONE_DEBUG, (TEXT("CHW::ResetEndpoint at ep %d, dir IN %d, csrIndex %d\r\n"),
        endpoint, dir, csrIndex));
    if (USB_ENDPOINT(endpoint) == 0)
    {
        DEBUGMSG(ZONE_HCD, (TEXT("CSR0 register = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[0].CSR.CSR0)));
        SETREG16(&pOTG->pUsbCsrRegs->ep[0].CSR.CSR0, CSR0_H_FlushFIFO);
    }
    else {
        if (dir)
        {
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_FlushFIFO|RXCSR_H_ClrDataTog);
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_ReqPkt);
            DEBUGMSG(ZONE_HCD, (TEXT("FlushFIFO::RXCSR (%d) register = 0x%x\r\n"), endpoint, 
                INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));
            DEBUGMSG(ZONE_HCD, (TEXT("FlushFIFO: RxCount(0x%x)\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount)));


        }
        else
        {
            DEBUGMSG(ZONE_HCD, (TEXT("CHW::ResetEndPoint::FlushFIFO::TXCSR (%d) register = 0x%x\r\n"), endpoint, 
                INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR)));
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_FlushFIFO|TXCSR_H_ClrDataTog);
        }
    }
#endif    
    LeaveCriticalSection(&pOTG->regCS);
}

//********************************************************************************
BOOL CHW::LockEP0(UCHAR address)
//
//  Purpose: Lock the EP0 as they can be accessed by other device at the same time
//
//  Parameter : address - device address
{
    BOOL fLock = FALSE;

    DEBUGMSG(ZONE_HCD, (TEXT("LockEP0 => 0x%x\r\n"), address));

    EnterCriticalSection(&m_LockEP0DeviceAddress.hLockCS);

    if (m_LockEP0DeviceAddress.ucLockEP == 0xFF)
    {
        m_LockEP0DeviceAddress.ucLockEP = address;
        fLock = TRUE;       
    }
    else if (m_LockEP0DeviceAddress.ucLockEP == address)
    {
        DEBUGMSG(1, (TEXT("Address %d for ep 0  is not unlocked\r\n"), address));
        fLock = TRUE;
    }    
    LeaveCriticalSection(&m_LockEP0DeviceAddress.hLockCS);
    return fLock;
}
//********************************************************************************
void CHW::UnlockEP0(UCHAR address)
//
//  Purpose: UnLock the EP0 as they can be accessed by other device at the same time
//
//  Parameter : address - device address
{
    DEBUGMSG(ZONE_HCD, (TEXT("UnLockEP0 => 0x%x\r\n"), address));
    EnterCriticalSection(&m_LockEP0DeviceAddress.hLockCS);
    if (m_LockEP0DeviceAddress.ucLockEP == address)
        m_LockEP0DeviceAddress.ucLockEP = 0xFF;
    else
        DEBUGMSG(1, (TEXT("Cannot unlock as used by other device address %d\r\n"), address));
    LeaveCriticalSection(&m_LockEP0DeviceAddress.hLockCS);
}

BOOL CHW::IsDeviceLockEP0(UCHAR address)
{
    if (m_LockEP0DeviceAddress.ucLockEP == address)
        return TRUE;

    return FALSE;
}
// ======================= Following is exported function ========================
//------------------------------------------------------------------------------
//
//  Function:   Host_ResumeIRQ
//
//  Routine description:
//
//      Handling of Resume interrupt
//
//  Arguments:
//
//      pHSMUSBContext  :    Handle of HSMUSB_T
//      others          :    to be determined.
//
//  Return values:
//
//      ERROR_SUCCESS
//
extern "C" DWORD Host_ResumeIRQ (PVOID pHSMUSBContext)
{
    UNREFERENCED_PARAMETER(pHSMUSBContext);
    DEBUGMSG(ZONE_DEBUG, (TEXT("+Host_ResumeIRQ\r\n")));
    return ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
//  Function:   Host_ProcessEPxRx
//
//  Routine description:
//
//      Handle of data transfer, it would include RX.
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      endpoint        :   endpoint to be processed
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
extern "C" DWORD Host_ProcessEPxRx(PVOID pHSMUSBContext, DWORD endpoint)
{        
    PHSMUSB_T pHsUsb = (PHSMUSB_T)pHSMUSBContext;
    SMHCDPdd * pPdd = (SMHCDPdd *)pHsUsb->pContext[HOST_MODE-1];
    CMhcd *pMhcd = (CMhcd *)pPdd->lpvMHCDMddObject;

    DEBUGMSG(ZONE_TRANSFER|ZONE_HCD, (TEXT("Host_ProcessEPxRx ep %d indx %d+\r\n"), endpoint, R_RX_INDEX(endpoint)));
    
    pMhcd->SignalCheckForDoneTransfers((unsigned char)R_RX_INDEX(endpoint), 0);
    DEBUGMSG(ZONE_HCD, (TEXT("Host_ProcessEPxRx-\r\n")));

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//  Function:   Host_ProcessEPxTx
//
//  Routine description:
//
//      Handle of data transfer, it would include TX.
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      endpoint        :   endpoint to be processed
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
extern "C" DWORD Host_ProcessEPxTx(PVOID pHSMUSBContext, DWORD endpoint)
{        
    PHSMUSB_T pHsUsb = (PHSMUSB_T)pHSMUSBContext;
    SMHCDPdd * pPdd = (SMHCDPdd *)pHsUsb->pContext[HOST_MODE-1];
    CMhcd *pMhcd = (CMhcd *)pPdd->lpvMHCDMddObject;

    DEBUGMSG(ZONE_TRANSFER|ZONE_HCD, (TEXT("Host_ProcessEPxTx mapped ep %d indx %d\r\n"), endpoint, R_TX_INDEX(endpoint)));
    DEBUGMSG(ZONE_TRANSFER, (TEXT("Host_ProcessEPxTx mapped ep %d indx %d\r\n"), endpoint, R_TX_INDEX(endpoint)));
    pMhcd->SignalCheckForDoneTransfers((unsigned char)R_TX_INDEX(endpoint), 1);
    DEBUGMSG(ZONE_HCD, (TEXT("Host_ProcessEPxTx-\r\n")));

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:   Host_ProcessEP0
//
//  Routine description:
//
//      Handling of endpoint 0 transfer
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//      
//      call Host_ProcessEPx
//
extern "C" DWORD Host_ProcessEP0 (PVOID pHSMUSBContext)
{
    return Host_ProcessEPxTx(pHSMUSBContext, 0);
}

//------------------------------------------------------------------------------
//  Function:   Host_Connect
//
//  Routine description:
//
//      Handle of Connect interrupt
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
extern "C" DWORD Host_Connect(PVOID pHSMUSBContext)
{
    PHSMUSB_T pHsUsb = (PHSMUSB_T)pHSMUSBContext;
    SMHCDPdd * pPdd = (SMHCDPdd *)pHsUsb->pContext[HOST_MODE-1];
    CHcd *pHcd = (CHcd *)pPdd->lpvMHCDMddObject;

    // This one should be set only on connect or it would have
    // racing condition causing access violation.
    pHcd->SetOTGContext(pHSMUSBContext);        
    pHcd->SetSignalDisconnectACK(FALSE);
    pHcd->SignalHubChangeEvent(TRUE);    
    DEBUGMSG(ZONE_HCD, (TEXT("Host_Connect\r\n")));    
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//  Function:   Host_Disconnect
//
//  Routine description:
//
//      Handle of Disconnect interrupt
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
extern "C" DWORD Host_Disconnect(PVOID pHSMUSBContext)
{
    PHSMUSB_T pHsUsb = (PHSMUSB_T)pHSMUSBContext;
    SMHCDPdd * pPdd = (SMHCDPdd *)pHsUsb->pContext[HOST_MODE-1];
    CHcd *pHcd = (CHcd *)pPdd->lpvMHCDMddObject;

    pHcd->SignalHubChangeEvent(FALSE);

    if(pHcd->m_fHCDState == USBHCD_ACTIVE)
    {
        WaitForSingleObject(pHcd->m_hPowerDownEvent, INFINITE);
    }
    // Jeffrey: HNP
    //pHsUsb->pUsbGenRegs->DevCtl &= ~DEVCTL_SESSION;
    DEBUGMSG(ZONE_HCD, (TEXT("Host_Disconnect\r\n")));
    
    return ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
//  Function:   Host_Suspend
//
//  Routine description:
//
//      Handle of suspend interrupt. This is the result of Remote Wakeup
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
extern "C" DWORD Host_Suspend(PVOID pHSMUSBContext)
{
    UNREFERENCED_PARAMETER(pHSMUSBContext);
    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
//  Function:   Host_ProcessDMA
//
//  Routine Description:
//
//      Handle the DMA interrupt 
//
//  Arguments:
//
//      pHSMUBContext   :   Handle of HSMUSB_T
//      channel         :   DMA channel
//
//  Return values:
//
//      ERROR_SUCCESS
//
extern "C" DWORD Host_ProcessDMA(PVOID pContext, UCHAR channel)
{
    PHSMUSB_T pHsUsb = (PHSMUSB_T)pContext;
    SMHCDPdd * pPdd = (SMHCDPdd *)pHsUsb->pContext[HOST_MODE-1];
    CMhcd *pMhcd = (CMhcd *)pPdd->lpvMHCDMddObject;

    DEBUGMSG(ZONE_HCD, (TEXT("Host_ProcessDMA with channel %d\r\n"), channel));    
    // Not sure if we should use the SetEventData by accessing different bit for different endpoint
    // However, it would require some critical section handling.  Anyway, each pipe would have an associate 
    // DMA event handle and we can set the event with corresponding channel.    
    pMhcd->SignalCheckForDoneDMA(channel);
    return ERROR_SUCCESS;
}