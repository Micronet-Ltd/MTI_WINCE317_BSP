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
//     CPipe.cpp
// Abstract:  
//     Implements the Pipe class for managing open pipes for UHCI
//
//                             CPipe (ADT)
//                           /             \
//                  CQueuedPipe (ADT)       CIsochronousPipe
//                /         |       \ 
//              /           |         \
//   CControlPipe    CInterruptPipe    CBulkPipe
// 
// 
// Notes: 
// 
//

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include <windows.h>
#include "trans.h"
#include "Cpipe.h"
#include "chw.h"
#include "CMhcd.h"
#include <omap_musbcore.h>
#include <omap3530_musbcore.h>
#include <omap3530_musbotg.h>
#pragma warning(pop)

#undef ZONE_DEBUG
#define ZONE_DEBUG 0

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// disable PREFAST warning for empty _except block
#pragma warning (disable: 6322)

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

// ******************************************************************
// Scope: public 
CPipe::CPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
              IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
              IN const UCHAR bDeviceAddress,
              IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
              IN CMhcd *const pCMhcd)
//
// Purpose: constructor for CPipe
//
// Parameters: lpEndpointDescriptor - pointer to endpoint descriptor for
//                                    this pipe (assumed non-NULL)
//
//             fIsLowSpeed - indicates if this pipe is low speed
//
// Returns: Nothing.
//
// Notes: Most of the work associated with setting up the pipe
//        should be done via OpenPipe. The constructor actually
//        does very minimal work.
//
//        Do not modify static variables here!!!!!!!!!!!
// ******************************************************************
: CPipeAbs(lpEndpointDescriptor->bEndpointAddress )
, m_usbEndpointDescriptor( *lpEndpointDescriptor )
, m_bDeviceAddress(bDeviceAddress)
, m_pCMhcd(pCMhcd)
, m_fIsLowSpeed( !!fIsLowSpeed ) // want to ensure m_fIsLowSpeed is 0 or 1
, m_fIsHighSpeed( !!fIsHighSpeed)
, m_fIsHalted( FALSE )
, m_bHubAddress (bHubAddress)
, m_bHubPort (bHubPort)
, m_TTContext(ttContext)
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CPipe::CPipe\n"),GetControllerName()) );
    // CPipe::Initialize should already have been called by now
    // to set up the schedule and init static variables
    //DEBUGCHK( pUHCIFrame->m_debug_fInitializeAlreadyCalled );

    InitializeCriticalSection( &m_csPipeLock );
    m_fIsHalted = FALSE;
    // Assume it is Async. If it is not It should be ovewrited.
    m_bFrameSMask =  0;
    m_bFrameCMask =  0;

    m_tx_fifo_size = 0;
    m_tx_fifo_addr = 0;
    m_rx_fifo_size = 0;
    m_rx_fifo_addr = 0;

    m_ucDataToggle = 0;
    m_bProcessDataToggle = TRUE;

    // Set Transfer Mode by default
    SetTransferMode(TRANSFER_FIFO);

    // Control EndPoint
    if (USB_ENDPOINT(lpEndpointDescriptor->bEndpointAddress) == 0)
    {
        m_hEPTransferEvent = NULL;
        m_hDMATransferEvent = NULL;
    }
    else
    {
        m_hEPTransferEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_pCMhcd->IsDMASupport())
            m_hDMATransferEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        else
            m_hDMATransferEvent = NULL;
    }
    // 
    
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CPipe::CPipe\n")) );
}

//*****************************************************************
UCHAR CPipe::GetDataToggle(void)
{
    return m_ucDataToggle;
}

//******************************************************************
void CPipe::SetDataToggle(UCHAR dataToggle)
{
    RETAILMSG(0, (TEXT("SetDataToggle to %d for EP 0x%x (mapped %d) DevAddr %d OUT %d\r\n"),
        dataToggle, m_usbEndpointDescriptor.bEndpointAddress, GetMappedEndPoint(), 
        GetReservedDeviceAddr(), USB_ENDPOINT_DIRECTION_OUT(m_usbEndpointDescriptor.bEndpointAddress)));
    m_ucDataToggle = dataToggle;
    m_bProcessDataToggle = TRUE;
}

BOOL CPipe::IsDataToggle(void)
{
    RETAILMSG(0, (TEXT("IsDataToggle for EP 0x%x (mapped %d) DevAddr %d OUT %d return %d\r\n"),
        m_usbEndpointDescriptor.bEndpointAddress, GetMappedEndPoint(), 
        GetReservedDeviceAddr(), USB_ENDPOINT_DIRECTION_OUT(m_usbEndpointDescriptor.bEndpointAddress), m_bProcessDataToggle));

    if (m_bProcessDataToggle)
    {
        m_bProcessDataToggle = FALSE;
        return TRUE;
    }
    return FALSE;
}
   
//*******************************************************************
#if 0
void CPipe::CalculateFIFO()
{
    UCHAR endpoint;
    UCHAR protocol;
    UCHAR outdir;

    endpoint = USB_ENDPOINT(m_usbEndpointDescriptor.bEndpointAddress);
    protocol = m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK;
    outdir = USB_ENDPOINT_DIRECTION_OUT(m_usbEndpointDescriptor.bEndpointAddress);

    USHORT size = max(m_usbEndpointDescriptor.wMaxPacketSize, 8);
    DWORD pwr_of_two = size/8;
    DWORD fifo_size_index;
    DWORD fifo_addr_index = (m_pCMhcd->m_fifo_avail_addr)/8;
    int i = 0;

    while (i < 10 ) // max = 4096 i.e. i = 9
    {
        if (pwr_of_two & (1<<i)) 
        {
            fifo_size_index = i;
            break;
        }        
        i++;
    }
    
    if (endpoint != 0)
    {
        if (outdir) 
        {
            m_tx_fifo_size = fifo_size_index;
            m_tx_fifo_addr = fifo_addr_index;
        }
        else
        {
            m_rx_fifo_size = fifo_size_index;
            m_rx_fifo_addr = fifo_addr_index;
        }
        m_pCMhcd->m_fifo_avail_addr += size;
    }
    else
    {
        m_tx_fifo_size = fifo_size_index;
        m_tx_fifo_addr = fifo_addr_index;
        m_pCMhcd->m_fifo_avail_addr += size;

        fifo_addr_index = (m_pCMhcd->m_fifo_avail_addr)/8;
        m_rx_fifo_size = fifo_size_index;
        m_rx_fifo_addr = fifo_addr_index;
        m_pCMhcd->m_fifo_avail_addr += size;
    }
    return;
}
#endif
// ******************************************************************
// Scope: public virtual 
CPipe::~CPipe( )
//
// Purpose: Destructor for CPipe
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes:   Most of the work associated with destroying the Pipe
//          should be done via ClosePipe
//
//          Do not delete static variables here!!!!!!!!!!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CPipe::~CPipe\n"),GetControllerName()) );
    // transfers should be aborted or closed before deleting object
    DeleteCriticalSection( &m_csPipeLock );    
    if (m_hEPTransferEvent)
        CloseHandle(m_hEPTransferEvent);
    if (m_hDMATransferEvent)
        CloseHandle(m_hDMATransferEvent);
    m_hEPTransferEvent = NULL;
    m_hDMATransferEvent = NULL;
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CPipe::~CPipe\n")) );
}
CPhysMem *CPipe::GetCPhysMem()
{
     return m_pCMhcd->GetPhysMem();
}
// ******************************************************************
// Scope: public
HCD_REQUEST_STATUS CPipe::IsPipeHalted( OUT LPBOOL const lpbHalted )
//
// Purpose: Return whether or not this pipe is halted (stalled)
//
// Parameters: lpbHalted - pointer to BOOL which receives
//                         TRUE if pipe halted, else FALSE
//
// Returns: requestOK 
//
// Notes:  Caller should check for lpbHalted to be non-NULL
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CPipe(%s)::IsPipeHalted\n"),GetControllerName(), GetPipeType()) );

    DEBUGCHK( lpbHalted ); // should be checked by CUhcd

    EnterCriticalSection( &m_csPipeLock );
    if (lpbHalted)
        *lpbHalted = m_fIsHalted;
    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CPipe(%s)::IsPipeHalted, *lpbHalted = %d, returning HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetPipeType(), *lpbHalted, requestOK) );
    return requestOK;
}
// ******************************************************************
// Scope: public
void CPipe::ClearHaltedFlag( void )
//
// Purpose: Clears the pipe is halted flag
//
// Parameters: None
//
// Returns: Nothing 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CPipe(%s)::ClearHaltedFlag\n"),GetControllerName(), GetPipeType() ) );

    EnterCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_WARNING && !m_fIsHalted, (TEXT("%s: CPipe(%s)::ClearHaltedFlag - warning! Called on non-stalled pipe\n"),GetControllerName(), GetPipeType()) );
    m_fIsHalted = FALSE;
    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CPipe(%s)::ClearHaltedFlag\n"),GetControllerName(), GetPipeType()) );
}

void CPipe::ResetEndPoint(void)
{
    EnterCriticalSection(&m_csPipeLock);
    RETAILMSG(1, (TEXT("ResetEndPoint at endpoint 0x%x\r\n"), m_bEndpointAddress));
    m_pCMhcd->ResetEndPoint(m_bEndpointAddress);
    LeaveCriticalSection(&m_csPipeLock);
}
// ******************************************************************               
// Scope: public
CQueuedPipe::CQueuedPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
                 IN CMhcd *const pCMhcd)
//
// Purpose: Constructor for CQueuedPipe
//
// Parameters: See CPipe::CPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CPipe( lpEndpointDescriptor, fIsLowSpeed,fIsHighSpeed, bDeviceAddress,bHubAddress,bHubPort,ttContext, pCMhcd )   // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CQueuedPipe::CQueuedPipe\n"),GetControllerName()) );
    m_pPipeQHead = NULL;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CQueuedPipe::CQueuedPipe\n"),GetControllerName()) );
}

// ******************************************************************               
// Scope: public virtual
CQueuedPipe::~CQueuedPipe( )
//
// Purpose: Destructor for CQueuedPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CQueuedPipe::~CQueuedPipe\n"),GetControllerName()) );
    // queue should be freed via ClosePipe before calling destructor
    EnterCriticalSection( &m_csPipeLock );
    ASSERT(m_pPipeQHead == NULL);
    ASSERT(m_pUnQueuedTransfer==NULL);      // ptr to last transfer in queue
    ASSERT(m_pQueuedTransfer==NULL);
    LeaveCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CQueuedPipe::~CQueuedPipe\n"),GetControllerName()) );
}
// ******************************************************************               
// Scope: public (implements CPipe::AbortTransfer = 0)
HCD_REQUEST_STATUS CQueuedPipe::AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId )
//
// Purpose: Abort any transfer on this pipe if its cancel ID matches
//          that which is passed in.
//
// Parameters: lpCancelAddress - routine to callback after aborting transfer
//
//             lpvNotifyParameter - parameter for lpCancelAddress callback
//
//             lpvCancelId - identifier for transfer to abort
//
// Returns: requestOK if transfer aborted
//          requestFailed if lpvCancelId doesn't match currently executing
//                 transfer, or if there is no transfer in progress
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("%s: +CQueuedPipe(%s)::AbortTransfer - lpvCancelId = 0x%x\n"),GetControllerName(), GetPipeType(), lpvCancelId) );

    HCD_REQUEST_STATUS status = requestFailed;
    BOOL fCheckSchedule = FALSE;

    EnterCriticalSection( &m_csPipeLock );
    CQTransfer * pCurTransfer = m_pUnQueuedTransfer;
    CQTransfer * pPrevTransfer= NULL;
    while (pCurTransfer && pCurTransfer->GetSTransfer().lpvCancelId !=  lpvCancelId ) {
        pPrevTransfer = pCurTransfer;
        pCurTransfer= ( CQTransfer *)pCurTransfer->GetNextTransfer();
        
    };
    if (pCurTransfer) { // Found Transfer that not queue yet.
        pCurTransfer->AbortTransfer();
        if (pPrevTransfer)
            pPrevTransfer->SetNextTransfer(pCurTransfer->GetNextTransfer());
        else
            m_pUnQueuedTransfer = ( CQTransfer *)pCurTransfer->GetNextTransfer();
    }
    else {
        if (m_pQueuedTransfer!=NULL &&  m_pQueuedTransfer->GetSTransfer().lpvCancelId ==  lpvCancelId ) {
            // This is one in the schdeule
            // Not sure if it needs to do something here.
            m_pQueuedTransfer->AbortTransfer();           
            GetQHead()->InvalidNextTD(); 
            Sleep(2);// this sleep is for Interrupt Pipe;
            pCurTransfer = m_pQueuedTransfer;
            m_pQueuedTransfer = NULL;            
            GetQHead()->ClearTDList();
        }
        else 
            ASSERT(FALSE);        
    };
    if (pCurTransfer) {
        pCurTransfer->DoneTransfer();
        if ( lpCancelAddress ) {
            __try { // calling the Cancel function
                ( *lpCancelAddress )( lpvNotifyParameter );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  DEBUGMSG( ZONE_ERROR, (TEXT("%s: CQueuedPipe::AbortTransfer - exception executing cancellation callback function\n"),GetControllerName()) );
            }
        }
        status = requestOK;
        delete pCurTransfer;
        fCheckSchedule = TRUE;
    }
    LeaveCriticalSection( &m_csPipeLock );
    if (fCheckSchedule && (m_pQueuedTransfer == NULL) ) { // This queue is no longer active. re-activate it.
            ScheduleTransfer();
    }

    DEBUGMSG( ZONE_TRANSFER, (TEXT("-CQueuedPipe(%s)::AbortTransfer - lpvCancelId = 0x%x, returning HCD_REQUEST_STATUS %d\n"), GetPipeType(), lpvCancelId, status) );
    return status;
}
// ******************************************************************
// Scope: public
void CQueuedPipe::ClearHaltedFlag( void )
//
// Purpose: Clears the pipe is halted flag
//
// Parameters: None
//
// Returns: Nothing 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CQueuedPipe(%s)::ClearHaltedFlag\n"),GetControllerName(), GetPipeType() ) );

    EnterCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_WARNING && !m_fIsHalted, (TEXT("%s: CQueuedPipe(%s)::ClearHaltedFlag - warning! Called on non-stalled pipe\n"),GetControllerName(), GetPipeType()) );
    m_fIsHalted = FALSE;
    
    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CQueuedPipe(%s)::ClearHaltedFlag\n"),GetControllerName(), GetPipeType()) );
}
// ******************************************************************               
// Scope: private
void  CQueuedPipe::AbortQueue( void ) 
//
// Purpose: Abort the current transfer (i.e., queue of TDs).
//
// Parameters: pQH - pointer to Queue Head for transfer to abort
//
// Returns: Nothing
//
// Notes: not used for OHCI
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("%s: +CQueuedPipe(%s)::AbortQueue \n"),GetControllerName(), GetPipeType()) );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pUnQueuedTransfer) {      // ptr to last transfer in queue
        while ( m_pUnQueuedTransfer) {
            m_pUnQueuedTransfer ->AbortTransfer();
            m_pUnQueuedTransfer ->DoneTransfer();
            CQTransfer * pCurTransfer= (CQTransfer *) m_pUnQueuedTransfer->GetNextTransfer();
            delete m_pUnQueuedTransfer;
            m_pUnQueuedTransfer = pCurTransfer;
        }       
    }
    ASSERT( m_pUnQueuedTransfer == NULL);
    if (m_pQueuedTransfer) {
        RETAILMSG(1, (TEXT("AbortQueue\r\n")));
        m_pQueuedTransfer;
        m_pQueuedTransfer ->AbortTransfer();
        GetQHead()->InvalidNextTD();  
        Sleep(2);// this sleep is for Interrupt Pipe;
        m_pQueuedTransfer->DoneTransfer();
        m_pQueuedTransfer =  NULL;
        delete m_pQueuedTransfer;
        m_pQueuedTransfer = NULL;        
        GetQHead()->ClearTDList();
    }
    ASSERT(m_pQueuedTransfer == NULL);

    LeaveCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_TRANSFER, (TEXT("%s: -CQueuedPipe(%s)::AbortQueue - %d\n"),GetControllerName(), GetPipeType()) );
}

// ******************************************************************               
// Scope: public 
HCD_REQUEST_STATUS  CQueuedPipe::IssueTransfer( 
                                    IN const UCHAR address,
                                    IN LPTRANSFER_NOTIFY_ROUTINE const lpStartAddress,
                                    IN LPVOID const lpvNotifyParameter,
                                    IN const DWORD dwFlags,
                                    IN LPCVOID const lpvControlHeader,
                                    IN const DWORD dwStartingFrame,
                                    IN const DWORD dwFrames,
                                    IN LPCDWORD const aLengths,
                                    IN const DWORD dwBufferSize,     
                                    IN_OUT LPVOID const lpvClientBuffer,
                                    IN const ULONG paBuffer,
                                    IN LPCVOID const lpvCancelId,
                                    OUT LPDWORD const adwIsochErrors,
                                    OUT LPDWORD const adwIsochLengths,
                                    OUT LPBOOL const lpfComplete,
                                    OUT LPDWORD const lpdwBytesTransferred,
                                    OUT LPDWORD const lpdwError )
//
// Purpose: Issue a Transfer on this pipe
//
// Parameters: address - USB address to send transfer to
//
//             OTHER PARAMS - see comment in CUhcd::IssueTransfer
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes:   
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("%s: +CPipe(%s)::IssueTransfer, address = %d\n"),GetControllerName(), GetPipeType(), address) );
 
    //m_pPipeQHead ->SetDeviceAddress(m_bDeviceAddress, USB_ENDPOINT_DIRECTION_OUT(m_usbEndpointDescriptor.bEndpointAddress));
    // Check if it is connected first
    if(m_pCMhcd->IsHostConnect() == FALSE)
    {
        DEBUGMSG(ZONE_WARNING, (TEXT("CQueuedPipe::IssueTransfer:: Disconnect\r\n")));
        return requestFailed;
    }


    

    STransfer sTransfer = {
    // These are the IssueTransfer parameters
        lpStartAddress,lpvNotifyParameter, dwFlags,lpvControlHeader, dwStartingFrame,dwFrames,
        aLengths,dwBufferSize,lpvClientBuffer,paBuffer,lpvCancelId,adwIsochErrors, adwIsochLengths,
        lpfComplete,lpdwBytesTransferred,lpdwError};
    HCD_REQUEST_STATUS  status = requestFailed;
    if (AreTransferParametersValid(&sTransfer) && GetQHead() && m_bDeviceAddress == address ) {
        EnterCriticalSection( &m_csPipeLock );
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
        __try { // initializing transfer status parameters
            *sTransfer.lpfComplete = FALSE;
            *sTransfer.lpdwBytesTransferred = 0;
            *sTransfer.lpdwError = USB_NOT_COMPLETE_ERROR;
            CQTransfer * pTransfer = new CQTransfer(this,m_pCMhcd->GetPhysMem(),sTransfer);
            if (pTransfer && pTransfer->Init()) {
                CQTransfer * pCur = m_pUnQueuedTransfer;
                if (pCur) {
                    while (pCur->GetNextTransfer()!=NULL)
                         pCur = (CQTransfer * )pCur->GetNextTransfer();
                    pCur->SetNextTransfer( pTransfer);
                }
                else
                    m_pUnQueuedTransfer=pTransfer;
                status=requestOK ;
            }
            else
                if (pTransfer) { // We return fails here so do not need callback;
                    pTransfer->DoNotCallBack() ;
                    delete pTransfer;    
                }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
        }
#pragma prefast(pop)
        LeaveCriticalSection( &m_csPipeLock );
        ScheduleTransfer( );
    }
    else
        ASSERT(FALSE);
    DEBUGMSG( ZONE_TRANSFER, (TEXT("%s: -CPipe(%s)::IssueTransfer - address = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetPipeType(), address, status) );
    return status;
}
// ******************************************************************               
// Scope: public 
HCD_REQUEST_STATUS   CQueuedPipe::ScheduleTransfer( void )
//
// Purpose: Schedule a Transfer on this pipe
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes:   
// ******************************************************************
{
    HCD_REQUEST_STATUS  status = requestFailed;
    BOOL bGetLock = TRUE;
    DEBUGMSG(ZONE_TRANSFER, (TEXT("ST on EP 0x%x, QT(0x%x), UQT(0x%x), QH(0x%x), Active(0x%x)\r\n"), 
        m_usbEndpointDescriptor.bEndpointAddress, m_pQueuedTransfer, m_pUnQueuedTransfer, m_pPipeQHead,
        m_pPipeQHead->IsActive()));
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pQueuedTransfer == NULL && m_pUnQueuedTransfer!=NULL  && 
            m_pPipeQHead && m_pPipeQHead->IsActive()==FALSE)
    { // We need cqueue new Transfer.        
        CQTransfer * pCurTransfer = m_pUnQueuedTransfer;        
        ASSERT(pCurTransfer!=NULL);
        m_pUnQueuedTransfer = (CQTransfer * )pCurTransfer->GetNextTransfer();
        pCurTransfer->SetNextTransfer(NULL);
        m_pQueuedTransfer = pCurTransfer;            

        // Acquire the Phyical EndPoint now    
        if(USB_ENDPOINT(m_usbEndpointDescriptor.bEndpointAddress) == 0)
            bGetLock = m_pCMhcd->LockEP0(GetReservedDeviceAddr());
        else
        {
            UCHAR ep = m_pCMhcd->AcquirePhysicalEndPoint(this);        
            bGetLock = (ep == 0xff)? FALSE: TRUE;
        }

        if (bGetLock == FALSE)
        {
            status = requestFailed;
            m_pQueuedTransfer = NULL;
            RETAILMSG(1, (TEXT("ScheduleTransfer failed to acquire ep %d\r\n"), 
                m_usbEndpointDescriptor.bEndpointAddress));
            goto SCHEDULE_TRANSFER_EXIT;
        }
        
        if (GetQHead()->IssueTransfer(pCurTransfer->GetCQTDList())==ERROR_SUCCESS) {
            ASSERT(pCurTransfer->GetCQTDList()->GetTransfer()== pCurTransfer);
            status=requestOK ;
        }
        else {
            ASSERT(FALSE);
            // Can not Queue. 
            RETAILMSG(1, (TEXT("ScheduleTransfer failed\r\n")));
            m_fIsHalted = TRUE;            
            m_pQueuedTransfer ->AbortTransfer();
            //ProcessEP(STATUS_PROCESS_ABORT);
            // we should not use ProcessEP, but global abort instead
            CheckForDoneTransfers();
        }
    }
    else 
        DEBUGMSG( ZONE_TRANSFER, (TEXT("-CPipe(%s)::ScheduleTransfer - Schedule called during QHead Busy or nothing to schedule! \n"), GetPipeType()) );

SCHEDULE_TRANSFER_EXIT:
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************
BOOL CQueuedPipe::ProcessEP(DWORD dwStatus, BOOL isDMA)
//
//  Purpose: Process the EP depending on the type of the TID. If data IN, we need
//           to read the data into the buffer.
//
//  Parameter:  dwStatus - cause of processing
//              isDMA - is it DMA
//
//  Return value: TRUE - success, FALSE - failure
//
{
    BOOL bReturn = FALSE;
    EnterCriticalSection( &m_csPipeLock );
    
    if (m_pQueuedTransfer!=NULL && m_pPipeQHead!=NULL) {
        // Check All the transfer done or not.
        CQTransfer * pCurTransfer = m_pQueuedTransfer;                        
        bReturn = pCurTransfer->ProcessEP(dwStatus, isDMA);
    }
    else
        DEBUGMSG(ZONE_TRANSFER, (TEXT("ProcessEP empty m_pQueuedTransfer(0x%x), m_pPipeQHead(0x%x)\r\n"),
            m_pQueuedTransfer, m_pPipeQHead));
    LeaveCriticalSection( &m_csPipeLock );
    return bReturn;   


}
// ******************************************************************               
// Scope: protected (Implements CPipe::CheckForDoneTransfers = 0)
BOOL CQueuedPipe::CheckForDoneTransfers(void )
//
// Purpose: Check if the transfer on this pipe is finished, and 
//          take the appropriate actions - i.e. remove the transfer
//          data structures and call any notify routines
//
// Parameters: None
//
// Returns: TRUE if transfer is done, else FALSE
//
// Notes:
// ******************************************************************
{
    BOOL bReturn = FALSE;
    EnterCriticalSection( &m_csPipeLock );
    
    if (m_pQueuedTransfer!=NULL && m_pPipeQHead!=NULL) {
        // Check All the transfer done or not.
        CQTransfer * pCurTransfer = m_pQueuedTransfer;                        
        if (pCurTransfer->IsTransferDone() == TRUE) {           
            ASSERT(m_pPipeQHead->IsActive() == FALSE) ;// Pipe Stopped.            
            m_fIsHalted = (pCurTransfer->DoneTransfer()!=TRUE);
            // Put it into done Queue.
            delete pCurTransfer;
            if (m_fIsHalted)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("Halt and need to do some data toggle\r\n")));
            }
            // Excute Next one if there is any.
            m_pQueuedTransfer =NULL;
            m_pPipeQHead->InvalidNextTD();
            m_pPipeQHead->ClearTDList();
            bReturn = TRUE;
        }
        else
        {  // Process the next one                       
           m_pPipeQHead->IssueTransfer(pCurTransfer->GetCQTDList());                        
        }

    }
    LeaveCriticalSection( &m_csPipeLock );
    if (m_pQueuedTransfer==NULL)
    {
        ScheduleTransfer();        
    }

    return bReturn;   
}

// ******************************************************************               
// Scope: public
CBulkPipe::CBulkPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
                 IN CMhcd *const pCMhcd)
//
// Purpose: Constructor for CBulkPipe
//
// Parameters: See CQueuedPipe::CQueuedPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CQueuedPipe(lpEndpointDescriptor,fIsLowSpeed, fIsHighSpeed,bDeviceAddress, bHubAddress, bHubPort,ttContext,  pCMhcd ) // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CBulkPipe::CBulkPipe\n"),GetControllerName()) );
    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK );

    DEBUGCHK( !fIsLowSpeed ); // bulk pipe must be high speed

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CBulkPipe::CBulkPipe\n"),GetControllerName()) );
}

// ******************************************************************               
// Scope: public
CBulkPipe::~CBulkPipe( )
//
// Purpose: Destructor for CBulkPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CBulkPipe::~CBulkPipe\n"),GetControllerName()) );
    ClosePipe();
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CBulkPipe::~CBulkPipe\n"),GetControllerName()) );
}
// ******************************************************************               
// Scope: public (Implements CPipe::OpenPipe = 0)
HCD_REQUEST_STATUS CBulkPipe::OpenPipe( void )
//
// Purpose: Create the data structures necessary to conduct
//          transfers on this pipe
//
// Parameters: None
//
// Returns: requestOK - if pipe opened
//
//          requestFailed - if pipe was not opened
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("%s: +CBulkPipe::OpenPipe\n"),GetControllerName() ) );
    
    HCD_REQUEST_STATUS status = requestFailed;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
    PREFAST_DEBUGCHK( m_pCMhcd!=NULL );
    EnterCriticalSection( &m_csPipeLock );

    // if this fails, someone is trying to open
    // an already opened pipe
    DEBUGCHK( m_pPipeQHead  == NULL );
    ASSERT(m_pCMhcd !=NULL);
    DEBUGCHK( !m_fIsLowSpeed );
    // In MUSBMHDRC, we don't need to have queue head allocate by CPhysMem anymore
    // it is now turn into internal use.
    if (m_pPipeQHead == NULL ) {
        m_pPipeQHead = new CQH(this);
        if (m_pPipeQHead)             
            status = requestOK;
        else 
            ASSERT(FALSE);
    }
        
    LeaveCriticalSection( &m_csPipeLock );
    ASSERT(m_pPipeQHead  != NULL);
    if (status == requestOK) {
        if (m_pCMhcd->IsDMASupport())
        {
            if (m_pCMhcd->GetDMAMode())
                SetTransferMode(TRANSFER_DMA1);
            else
                SetTransferMode(TRANSFER_DMA0);

            // Special handling for bug in MUSBMHDRC
            if ((GetTransferMode() == TRANSFER_DMA1) && (GetSpeed() == FULL_SPEED)
                && (USB_ENDPOINT_DIRECTION_OUT(m_usbEndpointDescriptor.bEndpointAddress)))
            {
                RETAILMSG(1, (TEXT("MUSBMHDRC issue - BULK OUT pipe need to force to run at DMA 0\r\n")));
                SetTransferMode(TRANSFER_DMA0);
            }
        }
        else
            SetTransferMode(TRANSFER_FIFO);
        // Add this pipe into busy pipe list
#ifdef DEBUG
        BOOL bReturn = m_pCMhcd->AddToBusyPipeList(this, FALSE);
        ASSERT(bReturn == TRUE);
#else
		m_pCMhcd->AddToBusyPipeList(this, FALSE);
#endif
    }

        
    DEBUGMSG( ZONE_PIPE, (TEXT("-CBulkPipe::OpenPipe, returning HCD_REQUEST_STATUS %d\n"), status) );
    
    return status;
}
// ******************************************************************               
// Scope: public (Implements CPipe::ClosePipe = 0)
HCD_REQUEST_STATUS CBulkPipe::ClosePipe( void )
//
// Purpose: Abort any transfers associated with this pipe, and
//          remove its data structures from the schedule
//
// Parameters: None
//
// Returns: requestOK
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("%s: +CBulkPipe(%s)::ClosePipe\n"),GetControllerName(), GetPipeType() ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pCMhcd->RemoveFromBusyPipeList(this );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pPipeQHead) {
        
        AbortQueue();                
        m_pPipeQHead->~CQH();
        delete m_pPipeQHead;
        m_pPipeQHead = NULL;
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************               
// Scope: private (Implements CPipe::AreTransferParametersValid = 0)
BOOL CBulkPipe::AreTransferParametersValid( const STransfer *pTransfer ) const 
//
// Purpose: Check whether this class' transfer parameters are valid.
//          This includes checking m_transfer, m_pPipeQH, etc
//
// Parameters: None (all parameters are vars of class)
//
// Returns: TRUE if parameters valid, else FALSE
//
// Notes: Assumes m_csPipeLock already held
// ******************************************************************
{
    if (pTransfer == NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
        
    
    //DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("+CBulkPipe::AreTransferParametersValid\n")) );

    // these parameters aren't used by CBulkPipe, so if they are non NULL,
    // it doesn't present a serious problem. But, they shouldn't have been
    // passed in as non-NULL by the calling driver.
    DEBUGCHK( pTransfer->adwIsochErrors == NULL && // ISOCH
              pTransfer->adwIsochLengths == NULL && // ISOCH
              pTransfer->aLengths == NULL && // ISOCH
              pTransfer->lpvControlHeader == NULL ); // CONTROL
    // this is also not a serious problem, but shouldn't happen in normal
    // circumstances. It would indicate a logic error in the calling driver.
    DEBUGCHK( !(pTransfer->lpStartAddress == NULL && pTransfer->lpvNotifyParameter != NULL) );
    // DWORD                     pTransfer->dwStartingFrame (ignored - ISOCH)
    // DWORD                     pTransfer->dwFrames (ignored - ISOCH)

    BOOL fValid = ( m_pPipeQHead!=NULL &&
                    (pTransfer->lpvBuffer != NULL || pTransfer->dwBufferSize == 0) &&
                    // paClientBuffer could be 0 or !0
                    m_bDeviceAddress > 0 && m_bDeviceAddress <= USB_MAX_ADDRESS &&
                    pTransfer->lpfComplete != NULL &&
                    pTransfer->lpdwBytesTransferred != NULL &&
                    pTransfer->lpdwError != NULL );

    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE && !fValid, (TEXT("%s: !CBulkPipe::AreTransferParametersValid, returning BOOL %d\n"),GetControllerName(), fValid) );
    ASSERT(fValid);
    return fValid;
}

// ******************************************************************               
// Scope: public
CControlPipe::CControlPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
                 IN CMhcd *const pCMhcd)
//
// Purpose: Constructor for CControlPipe
//
// Parameters: See CQueuedPipe::CQueuedPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CQueuedPipe( lpEndpointDescriptor, fIsLowSpeed, fIsHighSpeed, bDeviceAddress,bHubAddress, bHubPort,ttContext, pCMhcd ) // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CControlPipe::CControlPipe\n"),GetControllerName()) );
    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_CONTROL );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CControlPipe::CControlPipe\n"),GetControllerName()) );
}

// ******************************************************************               
// Scope: public
CControlPipe::~CControlPipe( )
//
// Purpose: Destructor for CControlPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CControlPipe::~CControlPipe\n"),GetControllerName()) );
    ClosePipe();
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CControlPipe::~CControlPipe\n"),GetControllerName()) );
}

// ******************************************************************               
// Scope: public (Implements CPipe::OpenPipe = 0)
HCD_REQUEST_STATUS CControlPipe::OpenPipe( void )
//
// Purpose: Create the data structures necessary to conduct
//          transfers on this pipe
//
// Parameters: None
//
// Returns: requestOK - if pipe opened
//
//          requestFailed - if pipe was not opened
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("%s: +CControlPipe::OpenPipe\n"),GetControllerName() ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
    PREFAST_DEBUGCHK( m_pCMhcd!=NULL );
    EnterCriticalSection( &m_csPipeLock );
    // if this fails, someone is trying to open
    // an already opened pipe
    DEBUGCHK( m_pPipeQHead  == NULL );
    ASSERT(m_pCMhcd !=NULL);
    // if this fails, we have a low speed Bulk device
    // which is not allowed by the UHCI spec (sec 1.3)
    if (m_pPipeQHead == NULL ) {
        m_pPipeQHead = new CQH (this);
        if (m_pPipeQHead ) {            
            status = requestOK;               
        }
        else {
            ASSERT(FALSE);
        }
    }    
    LeaveCriticalSection( &m_csPipeLock );

    if (status == requestOK) {
#ifdef DEBUG
        BOOL bReturn = m_pCMhcd->AddToBusyPipeList(this, FALSE);
        ASSERT(bReturn == TRUE);
#else
		m_pCMhcd->AddToBusyPipeList(this, FALSE);
#endif
    }
    DEBUGMSG( ZONE_PIPE, (TEXT("-CControlPipe::OpenPipe\n") ) );
       
    return status;
}
// ******************************************************************               
// Scope: public (Implements CPipe::ClosePipe = 0)
HCD_REQUEST_STATUS CControlPipe::ClosePipe( void )
//
// Purpose: Abort any transfers associated with this pipe, and
//          remove its data structures from the schedule
//
// Parameters: None
//
// Returns: requestOK
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("%s: +CBulkPipe(%s)::ClosePipe\n"),GetControllerName(), GetPipeType() ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pCMhcd->RemoveFromBusyPipeList(this );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pPipeQHead) {        
        AbortQueue();
        //delete m_pPipeQHead;
        m_pPipeQHead->~CQH();
        delete m_pPipeQHead;        
        m_pPipeQHead = NULL;
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************               
// Scope: public 
HCD_REQUEST_STATUS  CControlPipe::IssueTransfer( 
                                    IN const UCHAR address,
                                    IN LPTRANSFER_NOTIFY_ROUTINE const lpStartAddress,
                                    IN LPVOID const lpvNotifyParameter,
                                    IN const DWORD dwFlags,
                                    IN LPCVOID const lpvControlHeader,
                                    IN const DWORD dwStartingFrame,
                                    IN const DWORD dwFrames,
                                    IN LPCDWORD const aLengths,
                                    IN const DWORD dwBufferSize,     
                                    IN_OUT LPVOID const lpvClientBuffer,
                                    IN const ULONG paBuffer,
                                    IN LPCVOID const lpvCancelId,
                                    OUT LPDWORD const adwIsochErrors,
                                    OUT LPDWORD const adwIsochLengths,
                                    OUT LPBOOL const lpfComplete,
                                    OUT LPDWORD const lpdwBytesTransferred,
                                    OUT LPDWORD const lpdwError )
//
// Purpose: Issue a Transfer on this pipe
//
// Parameters: address - USB address to send transfer to
//
//             OTHER PARAMS - see comment in CUhcd::IssueTransfer
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes:   
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("%s: +CControlPipe::IssueTransfer, address = %d\n"),GetControllerName(), address) );

    // We can store the device address per pipe since the pipe is based on each device connection now.
    // In other words, the pipe is created per device connection if HUB is involved.
    // However, we cannot configEP here since it may be processed by other at this point.
    if (m_bDeviceAddress != address) { // Address Changed.
        if ( m_pQueuedTransfer == NULL &&  m_pPipeQHead && m_pPipeQHead->IsActive()==FALSE) { // We need cqueue new Transfer.
            m_bDeviceAddress = address;
            DEBUGMSG(ZONE_TRANSFER|0, (TEXT("CControlPipe::IssueTransfer configure m_bDeviceAddress = %d\r\n"),
                m_bDeviceAddress));
        }
        else {
            ASSERT(FALSE);
            return requestFailed;
        }
    }

    HCD_REQUEST_STATUS status = CQueuedPipe::IssueTransfer( address, lpStartAddress,lpvNotifyParameter,
            dwFlags,lpvControlHeader, dwStartingFrame, dwFrames, aLengths, dwBufferSize, lpvClientBuffer,
            paBuffer, lpvCancelId, adwIsochErrors, adwIsochLengths, lpfComplete, lpdwBytesTransferred, lpdwError );
    DEBUGMSG( ZONE_TRANSFER, (TEXT("%s: -CControlPipe::::IssueTransfer - address = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), address, status) );
    return status;
};
// ******************************************************************
// Scope: public
void CControlPipe::ChangeMaxPacketSize( IN const USHORT wMaxPacketSize )
//
// Purpose: Update the max packet size for this pipe. This should
//          ONLY be done for control endpoint 0 pipes. When the endpoint0
//          pipe is first opened, it has a max packet size of 
//          ENDPOINT_ZERO_MIN_MAXPACKET_SIZE. After reading the device's
//          descriptor, the device attach procedure can update the size.
//
// Parameters: wMaxPacketSize - new max packet size for this pipe
//
// Returns: Nothing
//
// Notes:   This function should only be called by the Hub AttachDevice
//          procedure
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CControlPipe::ChangeMaxPacketSize - new wMaxPacketSize = %d\n"),GetControllerName(), wMaxPacketSize) );
    
    EnterCriticalSection( &m_csPipeLock );

    // this pipe should be for endpoint 0, control pipe
    DEBUGCHK( (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_CONTROL &&
              (m_usbEndpointDescriptor.bEndpointAddress & TD_ENDPOINT_MASK) == 0 );
    // update should only be called if the old address was ENDPOINT_ZERO_MIN_MAXPACKET_SIZE
    DEBUGCHK( m_usbEndpointDescriptor.wMaxPacketSize == ENDPOINT_ZERO_MIN_MAXPACKET_SIZE );
    // this function should only be called if we are increasing the max packet size.
    // in addition, the USB spec 1.0 section 9.6.1 states only the following
    // wMaxPacketSize are allowed for endpoint 0
    DEBUGCHK( wMaxPacketSize > ENDPOINT_ZERO_MIN_MAXPACKET_SIZE &&
              (wMaxPacketSize == 16 ||
               wMaxPacketSize == 32 ||
               wMaxPacketSize == 64) );
    
    m_usbEndpointDescriptor.wMaxPacketSize = wMaxPacketSize;
    if ( m_pQueuedTransfer == NULL &&  m_pPipeQHead && m_pPipeQHead->IsActive()==FALSE) { // We need cqueue new Transfer.
        m_pPipeQHead ->SetMaxPacketLength(wMaxPacketSize);
    }
    else {
        ASSERT(FALSE);
    } 

    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CControlPipe::ChangeMaxPacketSize - new wMaxPacketSize = %d\n"),GetControllerName(), wMaxPacketSize) );
}
// ******************************************************************               
// Scope: private (Implements CPipe::AreTransferParametersValid = 0)
BOOL CControlPipe::AreTransferParametersValid( const STransfer *pTransfer ) const 
//
// Purpose: Check whether this class' transfer parameters are valid.
//          This includes checking m_transfer, m_pPipeQH, etc
//
// Parameters: None (all parameters are vars of class)
//
// Returns: TRUE if parameters valid, else FALSE
//
// Notes: Assumes m_csPipeLock already held
// ******************************************************************
{
    if (pTransfer == NULL)
        return FALSE;    
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("%s: +CControlPipe::AreTransferParametersValid\n"),GetControllerName()) );

    // these parameters aren't used by CControlPipe, so if they are non NULL,
    // it doesn't present a serious problem. But, they shouldn't have been
    // passed in as non-NULL by the calling driver.
    DEBUGCHK( pTransfer->adwIsochErrors == NULL && // ISOCH
              pTransfer->adwIsochLengths == NULL && // ISOCH
              pTransfer->aLengths == NULL ); // ISOCH
    // this is also not a serious problem, but shouldn't happen in normal
    // circumstances. It would indicate a logic error in the calling driver.
    DEBUGCHK( !(pTransfer->lpStartAddress == NULL && pTransfer->lpvNotifyParameter != NULL) );
    // DWORD                     pTransfer->dwStartingFrame; (ignored - ISOCH)
    // DWORD                     pTransfer->dwFrames; (ignored - ISOCH)

    BOOL fValid = ( m_pPipeQHead != NULL &&
                    m_bDeviceAddress <= USB_MAX_ADDRESS &&
                    pTransfer->lpvControlHeader != NULL &&
                    pTransfer->lpfComplete != NULL &&
                    pTransfer->lpdwBytesTransferred != NULL &&
                    pTransfer->lpdwError != NULL );
    if ( fValid ) {
        if ( pTransfer->dwFlags & USB_IN_TRANSFER ) {
            fValid = (pTransfer->lpvBuffer != NULL &&
                      // paClientBuffer could be 0 or !0
                      pTransfer->dwBufferSize > 0);
        } else {
            fValid = ( (pTransfer->lpvBuffer == NULL && 
                        pTransfer->paBuffer == 0 &&
                        pTransfer->dwBufferSize == 0) ||
                       (pTransfer->lpvBuffer != NULL &&
                        // paClientBuffer could be 0 or !0
                        pTransfer->dwBufferSize > 0) );
        }
    }

    ASSERT(fValid);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("%s: -CControlPipe::AreTransferParametersValid, returning BOOL %d\n"),GetControllerName(), fValid) );
    return fValid;
}
// ******************************************************************               
// Scope: public
CInterruptPipe::CInterruptPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
                 IN CMhcd *const pCMhcd)
//
// Purpose: Constructor for CInterruptPipe
//
// Parameters: See CQueuedPipe::CQueuedPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CQueuedPipe( lpEndpointDescriptor, fIsLowSpeed, fIsHighSpeed, bDeviceAddress, bHubAddress,bHubPort,ttContext, pCMhcd ) // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CInterruptPipe::CInterruptPipe\n"),GetControllerName()) );
    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_INTERRUPT );
    // Note: Need Get S-MASK and C-MASK...
    
    memset(&m_EndptBuget,0,sizeof(m_EndptBuget));
    m_EndptBuget.max_packet= lpEndpointDescriptor->wMaxPacketSize & 0x7ff;
    BYTE bInterval=lpEndpointDescriptor->bInterval;
    if (bInterval==0)
        bInterval=1;
    if (fIsHighSpeed) { // Table 9-13
        m_EndptBuget.max_packet *=(((lpEndpointDescriptor->wMaxPacketSize >>11) & 3)+1);
        m_EndptBuget.period = (1<< (bInterval-1));
    }
    else {
        m_EndptBuget.period = bInterval;
        for (UCHAR uBit=0x80;uBit!=0;uBit>>=1) {
            if ((m_EndptBuget.period & uBit)!=0) {
                m_EndptBuget.period = uBit;
                break;
            }
        }
    }
    ASSERT(m_EndptBuget.period!=0);
    m_EndptBuget.ep_type = interrupt ;
    m_EndptBuget.type= lpEndpointDescriptor->bDescriptorType;
    m_EndptBuget.direction =  (USB_ENDPOINT_DIRECTION_OUT(lpEndpointDescriptor->bEndpointAddress)?OUTDIR:INDIR);
    m_EndptBuget.speed=(fIsHighSpeed?HSSPEED:(fIsLowSpeed?LSSPEED:FSSPEED));

    m_bSuccess= pCMhcd->AllocUsb2BusTime(bHubAddress,bHubPort,m_TTContext,&m_EndptBuget);
    ASSERT(m_bSuccess);
    if (m_bSuccess ) {
        if (fIsHighSpeed) { // Update SMask and CMask for Slit Interrupt Endpoitt
            m_bFrameSMask = 0xff;
            m_bFrameCMask = 0;
        }
        else {
            m_bFrameSMask=pCMhcd->GetSMASK(&m_EndptBuget);
            m_bFrameCMask=pCMhcd->GetCMASK(&m_EndptBuget);
        }
    }
    else {
        ASSERT(FALSE);
    }

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CInterruptPipe::CInterruptPipe\n"),GetControllerName()) );
}

// ******************************************************************               
// Scope: public
CInterruptPipe::~CInterruptPipe( )
//
// Purpose: Destructor for CInterruptPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: +CInterruptPipe::~CInterruptPipe\n"),GetControllerName()) );
    if (m_bSuccess)
        m_pCMhcd->FreeUsb2BusTime( m_bHubAddress, m_bHubPort,m_TTContext,&m_EndptBuget);
    DEBUGMSG( ZONE_PIPE|ZONE_DEBUG, (TEXT("CInterruptPipe::~CInterruptPipe close\r\n")));
    ClosePipe();
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("%s: -CInterruptPipe::~CInterruptPipe\n"),GetControllerName()) );
}
// ******************************************************************               
// Scope: public (Implements CPipe::OpenPipe = 0)
HCD_REQUEST_STATUS CInterruptPipe::OpenPipe( void )
//
// Purpose: Create the data structures necessary to conduct
//          transfers on this pipe
//
// Parameters: None
//
// Returns: requestOK - if pipe opened
//
//          requestFailed - if pipe was not opened
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("%s: +CInterruptPipe::OpenPipe\n"),GetControllerName() ) );

    HCD_REQUEST_STATUS status = requestFailed;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
    if (!m_bSuccess)
        return status;
    EnterCriticalSection( &m_csPipeLock );

    // if this fails, someone is trying to open
    // an already opened pipe
    DEBUGCHK(m_pPipeQHead == NULL );
    if ( m_pPipeQHead == NULL )
        m_pPipeQHead =  new CQH(this);
    if (m_pPipeQHead) {        
        // Interrupt QHs are a bit complicated. See comment
        // in Initialize() routine as well.
        //        
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );
   
    if (status == requestOK) {
        if (m_pCMhcd->IsDMASupport())
        {
            // We can only set the transfer DMA mode 0 for Interrupt pipe
             if (m_pCMhcd->GetDMAMode())
                SetTransferMode(TRANSFER_DMA1);
            else
                SetTransferMode(TRANSFER_DMA0);

            // Special handling for bug in MUSBMHDRC
            if ((GetTransferMode() == TRANSFER_DMA1) && (GetSpeed() == FULL_SPEED)
                && (USB_ENDPOINT_DIRECTION_OUT(m_usbEndpointDescriptor.bEndpointAddress)))
            {
                RETAILMSG(1, (TEXT("MUSBMHDRC issue - INTERRUPT OUT need to operate at DMA 0\r\n")));
                SetTransferMode(TRANSFER_DMA0);
            }
        }
        else
            SetTransferMode(TRANSFER_FIFO);

#ifdef DEBUG
        BOOL bReturn = m_pCMhcd->AddToBusyPipeList(this, FALSE);
        ASSERT(bReturn == TRUE);
#else
		m_pCMhcd->AddToBusyPipeList(this, FALSE);
#endif
    }
    DEBUGMSG( ZONE_PIPE, (TEXT("%s: -CInterruptPipe::OpenPipe, returning HCD_REQUEST_STATUS %d\n"),GetControllerName(), status) );
    return status;
}
// ******************************************************************               
// Scope: public (Implements CPipe::ClosePipe = 0)
HCD_REQUEST_STATUS CInterruptPipe::ClosePipe( void )
//
// Purpose: Abort any transfers associated with this pipe, and
//          remove its data structures from the schedule
//
// Parameters: None
//
// Returns: requestOK
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("%s: +CBulkPipe(%s)::ClosePipe\n"),GetControllerName(), GetPipeType() ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pCMhcd->RemoveFromBusyPipeList(this );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pPipeQHead) {        
        AbortQueue( );
        //delete m_pPipeQHead;        
        m_pPipeQHead->~CQH();
        delete m_pPipeQHead;        
        m_pPipeQHead = NULL;
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************               
// Scope: private (Implements CPipe::AreTransferParametersValid = 0)
BOOL CInterruptPipe::AreTransferParametersValid( const STransfer *pTransfer ) const
//
// Purpose: Check whether this class' transfer parameters are valid.
//          This includes checking m_transfer, m_pPipeQH, etc
//
// Parameters: None (all parameters are vars of class)
//
// Returns: TRUE if parameters valid, else FALSE
//
// Notes: Assumes m_csPipeLock already held
// ******************************************************************
{
    if (pTransfer == NULL)
        return FALSE;
    
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("%s: +CInterruptPipe::AreTransferParametersValid\n"),GetControllerName()) );

    // these parameters aren't used by CInterruptPipe, so if they are non NULL,
    // it doesn't present a serious problem. But, they shouldn't have been
    // passed in as non-NULL by the calling driver.
    DEBUGCHK( pTransfer->adwIsochErrors == NULL && // ISOCH
              pTransfer->adwIsochLengths == NULL && // ISOCH
              pTransfer->aLengths == NULL && // ISOCH
              pTransfer->lpvControlHeader == NULL ); // CONTROL
    // this is also not a serious problem, but shouldn't happen in normal
    // circumstances. It would indicate a logic error in the calling driver.
    DEBUGCHK( !(pTransfer->lpStartAddress  == NULL && pTransfer->lpvNotifyParameter  != NULL) );
    // DWORD                     pTransfer->dwStartingFrame (ignored - ISOCH)
    // DWORD                     pTransfer->dwFrames (ignored - ISOCH)

    BOOL fValid = (  m_pPipeQHead!= NULL &&
                    m_bDeviceAddress > 0 && m_bDeviceAddress <= USB_MAX_ADDRESS &&
                    (pTransfer->lpvBuffer != NULL || pTransfer->dwBufferSize == 0) &&
                    // paClientBuffer could be 0 or !0
                    pTransfer->lpfComplete != NULL &&
                    pTransfer->lpdwBytesTransferred != NULL &&
                    pTransfer->lpdwError != NULL );

    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("%s: -CInterruptPipe::AreTransferParametersValid, returning BOOL %d\n"),GetControllerName(), fValid) );
    return fValid;
}


CPipeAbs * CreateBulkPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const BOOL ttContext,
               IN CHcd * const pChcd)
{    
    return new CBulkPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,ttContext,(CMhcd * const)pChcd);
}
CPipeAbs * CreateControlPipe(IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const BOOL ttContext,
               IN CHcd * const pChcd)
{     
    return new CControlPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,ttContext,(CMhcd * const)pChcd);
}

CPipeAbs * CreateInterruptPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const BOOL ttContext,
               IN CHcd * const pChcd)
{     
    return new CInterruptPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,ttContext,(CMhcd * const)pChcd);
}

// This is not supported at this point
#if 0
CPipeAbs * CreateIsochronousPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const BOOL ttContext,
               IN CHcd * const pChcd)
{ 
    return new CIsochronousPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,ttContext,(CMhcd * const)pChcd);
}
#endif



