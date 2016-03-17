// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Module Name:
//     hcd.cpp
// Abstract:
//     This file contains the Chcd object, which is the main entry
//     point for all HCDI calls by USBD
//
// Notes:
//
#include <globals.hpp>
#include <hcd.hpp>
CHcd::CHcd( )
{
    m_pCRootHub=NULL;
}
CHcd::~CHcd()
{
    Lock();
    CRootHub *pRoot = m_pCRootHub;
    m_pCRootHub = NULL;
    Unlock();
     // signal root hub to close
    if ( pRoot ) {
        pRoot->HandleDetach();
        delete pRoot;
    }
   
}
CRootHub* CHcd::SetRootHub(CRootHub* pRootHub)
{
    Lock();
    CRootHub* pReturn=m_pCRootHub;
    m_pCRootHub=pRootHub;
    Unlock();
    return pReturn;
}

// ******************************************************************
BOOL CHcd::OpenPipe( IN UINT address,
                      IN LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                      OUT LPUINT lpPipeIndex )
//
// Purpose: Create a logical communication pipe to the endpoint described
//          by lpEndpointDescriptor for device address.
//
// Parameters:  address - address of device to open pipe for
//
//              lpEndpointDescriptor - describes endpoint to open
//
//              lpPipeIndex - out param, indicating index of opened pipe
//
// Returns: TRUE if pipe opened ok, FALSE otherwise
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{
    DEBUGMSG(ZONE_UHCD, (TEXT("+CHcd::OpenPipe for device on addr %d\n"), address));
    BOOL fSuccess = FALSE;

    Lock();
    CRootHub *pRoot = m_pCRootHub;

    if ( pRoot != NULL &&
         lpEndpointDescriptor != NULL &&
         lpEndpointDescriptor->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
         lpEndpointDescriptor->bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
         lpPipeIndex != NULL ) {

        // root hub will send the request to the appropriate device
        fSuccess = ( requestOK == pRoot->OpenPipe( address,
                                                   lpEndpointDescriptor,
                                                   lpPipeIndex ) );
    }
    Unlock();
    DEBUGMSG(ZONE_UHCD, (TEXT("-CHcd::OpenPipe for address %d, returning BOOL %d\n"), address, fSuccess ));
    return fSuccess;
}

// ******************************************************************
BOOL CHcd::ClosePipe( IN UINT address,
                       IN UINT pipeIndex )
//
// Purpose: Close the logical communication pipe to this device's endpoint
//
// Parameters:  address - address of device to close pipe for
//
//              pipeIndex - indicating index of pipe to close
//
// Returns: TRUE if call to m_pCRootHub succeeds, else FALSE
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{
    DEBUGMSG( ZONE_UHCD, (TEXT("+CHcd::ClosePipe - address = %d, pipeIndex = %d\n"), address, pipeIndex) );
    BOOL fSuccess = FALSE;

    Lock();
    CRootHub *pRoot = m_pCRootHub;

    if (pRoot != NULL)
        fSuccess = (requestOK == pRoot->ClosePipe( address, pipeIndex ) );

    Unlock();
    DEBUGMSG( ZONE_UHCD, (TEXT("-CHcd::ClosePipe - address = %d, pipeIndex = %d, returning BOOL %d\n"), address, pipeIndex, fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL CHcd::IssueTransfer( IN UINT address,
                           IN UINT pipeIndex,
                           IN LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                           IN LPVOID lpvNotifyParameter,
                           IN DWORD dwFlags,
                           IN LPCVOID lpvControlHeader,
                           IN DWORD dwStartingFrame,
                           IN DWORD dwFrames,
                           IN LPCDWORD aLengths,
                           IN DWORD dwBufferSize,
                           IN_OUT LPVOID lpvBuffer,
                           IN ULONG paBuffer,
                           IN LPCVOID lpvCancelId,
                           OUT LPDWORD adwIsochErrors,
                           OUT LPDWORD adwIsochLengths,
                           OUT LPBOOL lpfComplete,
                           OUT LPDWORD lpdwBytesTransfered,
                           OUT LPDWORD lpdwError )
// Purpose: Issue a USB transfer
//
// Parameters:  address - address of device
//
//              pipeIndex - index of pipe to issue transfer on (NOT the endpoint address)
//
//              lpStartAddress - ptr to function to callback when transfer completes
//                               (this can be NULL)
//
//              lpvNotifyParameter - parameter to pass to lpStartAddress in callback
//
//              dwFlags - combination of
//                              USB_IN_TRANSFER
//                              USB_OUT_TRANSFER
//                              USB_NO_WAIT
//                              USB_SHORT_TRANSFER_OK
//                              USB_START_ISOCH_ASAP
//                              USB_COMPRESS_ISOCH
//                              USB_SEND_TO_DEVICE
//                              USB_SEND_TO_INTERFACE
//                              USB_SEND_TO_ENDPOINT
//                              USB_DONT_BLOCK_FOR_MEM
//                         defined in usbtypes.h
//
//              lpvControlHeader - for control transfers, a pointer to the
//                                 USB_DEVICE_REQUEST structure
//
//
//              dwStartingFrame - for Isoch transfers, this indicates the
//                                first frame of the transfer. If the
//                                USB_START_ISOCH_ASAP flag is set in
//                                dwFlags, the dwStartingFrame is ignored
//                                and the transfer is scheduled As Soon
//                                As Possible
//
//              dwFrames - indicates over how many frames to conduct this
//                         Isochronous transfer. Also should be the # of
//                         entries in aLengths, adwIsochErrors, adwIsochLengths
//
//
//              aLengths - array of dwFrames long. aLengths[i] is how much
//                         isoch data to transfer in the i'th frame. The
//                         sum of all entries should be dwBufferSize
//
//              dwBufferSize - size of data buffer passed in lpvBuffer
//
//              lpvBuffer - data buffer (may be NULL)
//
//              paBuffer - physical address of data buffer (may be 0)
//
//              lpvCancelId - identifier used to refer to this transfer
//                            in case it needs to be canceled later
//
//              adwIsochErrors - array of dwFrames long. adwIsochErrors[ i ]
//                               is set on transfer complete to be the status
//                               of the i'th frame of the isoch transfer
//
//              adwIsochLengths - array of dwFrames long. adwIsochLengths[ i ]
//                                is set on transfer complete to be the amount
//                                of data transferred in the i'th frame of
//                                the isoch transfer
//
//              lpfComplete - pointer to BOOL indicating TRUE/FALSE on
//                            whether this transfer is complete
//
//              lpdwBytesTransfered - pointer to DWORD indicating total # of
//                                    bytes successfully transferred
//
//              lpdwError - pointer to DWORD set to error status on
//                          transfer complete
//
// Returns: TRUE if transfer scheduled ok, otherwise FALSE
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{
   DEBUGMSG( ZONE_UHCD | ZONE_TRANSFER, (TEXT("+CHcd::IssueTransfer - address = %d, pipe = %d, dwFlags = 0x%x, lpvControlHeader = 0x%x, lpvBuffer = 0x%x, dwBufferSize = %d\n"), address, pipeIndex, dwFlags, lpvControlHeader, lpvBuffer, dwBufferSize));
   BOOL fSuccess = FALSE;
   Lock();
   CRootHub *pRoot = m_pCRootHub;
   HCD_REQUEST_STATUS status = requestIgnored;
   
   if (pRoot != NULL) {
       status = pRoot->IssueTransfer( address,
                                       pipeIndex,
                                       lpStartAddress,
                                       lpvNotifyParameter,
                                       dwFlags,
                                       lpvControlHeader,
                                       dwStartingFrame,
                                       dwFrames,
                                       aLengths,
                                       dwBufferSize,
                                       lpvBuffer,
                                       paBuffer,
                                       lpvCancelId,
                                       adwIsochErrors,
                                       adwIsochLengths,
                                       lpfComplete,
                                       lpdwBytesTransfered,
                                       lpdwError );

       if(status == requestOK) {
           fSuccess = TRUE;
       }
       else if (status == requestIgnored) {
           SetLastError(ERROR_DEV_NOT_EXIST);
       }       
   }
   Unlock();
   DEBUGMSG( ZONE_UHCD | ZONE_TRANSFER, (TEXT("-CHcd::IssueTransfer - returing BOOL %d\n"), fSuccess ) );
   return fSuccess;
}


// ******************************************************************
BOOL CHcd::AbortTransfer( IN UINT address,
                           IN UINT pipeIndex,
                           IN LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                           IN LPVOID lpvNotifyParameter,
                           IN LPCVOID lpvCancelId )
//
// Purpose: Abort a previously issued transfer
//
// Parameters:  address - address of device on which transfer was issued
//
//              pipeIndex - index of pipe on which transfer was issued
//
//              lpCancelAddress - function to callback when this transfer has aborted
//
//              lpvNotifyParameter - parameter to callback lpCancelAddress
//
//              lpvCancelId - used to identify the transfer to abort. This was passed
//                            in when IssueTransfer was called
//
// Returns: TRUE if call to m_pCRootHub succeeds, else FALSE
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{
    DEBUGMSG( ZONE_UHCD | ZONE_TRANSFER, (TEXT("+CHcd::AbortTransfer - address = %d, pipeIndex = %d, lpvCancelId = 0x%x\n"), address, pipeIndex, lpvCancelId) );
    BOOL fSuccess = FALSE;

    Lock();
    CRootHub *pRoot = m_pCRootHub;

    if (pRoot != NULL)
        fSuccess = (requestOK == pRoot->AbortTransfer( address,
                                                       pipeIndex,
                                                       lpCancelAddress,
                                                       lpvNotifyParameter,
                                                       lpvCancelId ) );

    Unlock();
    DEBUGMSG( ZONE_UHCD | ZONE_TRANSFER, (TEXT("-CHcd::AbortTransfer - address = %d, pipeIndex = %d, returning BOOL %d\n"), address, pipeIndex, fSuccess ) );
    return fSuccess;
}

// ******************************************************************
BOOL CHcd::IsPipeHalted( IN UINT address,
                          IN UINT pipeIndex,
                          OUT LPBOOL lpbHalted )
//
// Purpose: Check whether the pipe indicated by address/pipeIndex is
//          halted (stalled) and return result in *lpbHalted
//
// Parameters:  address - address of device to check pipe for
//
//              pipeIndex - indicating index of pipe to check
//
//              lpbHalted - out param, indicating TRUE if pipe is halted,
//                          else FALSE
//
// Returns: TRUE if correct status placed in *lpbHalted, else FALSE
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{
    DEBUGMSG(ZONE_UHCD,(TEXT("+CHcd::IsPipeHalted - address = %d, pipeIndex = %d\n"), address, pipeIndex ));
    BOOL retval = FALSE;

    Lock();
    CRootHub *pRoot = m_pCRootHub;

    if ( pRoot != NULL && lpbHalted != NULL ) {
        retval = ( requestOK == pRoot->IsPipeHalted( address,
                                                     pipeIndex,
                                                     lpbHalted ) );
    }
    Unlock();
    DEBUGMSG(ZONE_UHCD,(TEXT("-CHcd::IsPipeHalted - address = %d, pipeIndex = %d, *lpbHalted = %d, retval = %d\n"), address, pipeIndex, ((lpbHalted) ? *lpbHalted : -1), retval ));
    return retval;
}

// ******************************************************************
BOOL CHcd::ResetPipe( IN UINT address,
                       IN UINT pipeIndex )
//
// Purpose: Reset a stalled pipe on device at address "address"
//
// Parameters:  address - address of device to reset pipe for
//
//              pipeIndex - indicating index of pipe to check
//
//              lpbHalted - out param, indicating TRUE if pipe is halted,
//                          else FALSE
//
// Returns: TRUE if correct status placed in *lpbHalted, else FALSE
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{
    DEBUGMSG( ZONE_UHCD, (TEXT("+CHcd::ResetPipe - address = %d, pipeIndex = %d\n"), address, pipeIndex) );
    BOOL fSuccess = FALSE;

    Lock();
    CRootHub *pRoot = m_pCRootHub;

    if (pRoot != NULL)
        fSuccess = (requestOK == pRoot->ResetPipe( address, pipeIndex ) );

    Unlock();
    DEBUGMSG( ZONE_UHCD, (TEXT("-CHcd::ResetPipe - address = %d, pipeIndex = %d, returning BOOL %d\n"), address, pipeIndex, fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL CHcd::DisableDevice( IN const UINT address,  IN const BOOL fReset )
//
// Purpose: Disable on device at address "address"
//
// Parameters:  address - address of device to disable for
//
//              fReset - Reset it after disable.
//
// Returns: TRUE if Susess.
//
// Notes: This needs to be implemented for HCDI
{
    DEBUGMSG( ZONE_HCD, (TEXT("+CHcd::DisableDevice - address = %d,fReset = %d\n"), address, fReset) );
    BOOL fSuccess = FALSE;

    Lock();
    CRootHub *pRoot = m_pCRootHub;

    if (pRoot != NULL)
        fSuccess = (requestOK == pRoot->DisableDevice( address, fReset ) );
    Unlock();
    DEBUGMSG( ZONE_HCD, (TEXT("-CHcd::DisableDevice - address = %d, returning BOOL %d\n"), address, fSuccess) );
    return fSuccess;
    
};

// ******************************************************************
BOOL CHcd::SuspendResume( IN const UINT address,IN const BOOL fSuspend )
//
// Purpose: Suspend or Resume on device at address "address"
//
// Parameters:  address - address of device to disable for
//
//               fSuspend  - Suspend, otherwise resume..
//
//
// Returns: TRUE if Susess.
//
// Notes: This needs to be implemented for HCDI
{
    DEBUGMSG( ZONE_HCD, (TEXT("+CHcd::SuspendResume - address = %d,  fSuspend = %d\n"), address,  fSuspend) );
    BOOL fSuccess = FALSE;

    Lock();
    CRootHub *pRoot = m_pCRootHub;

    if (pRoot != NULL)
        fSuccess = (requestOK == pRoot->SuspendResume( address,  fSuspend ) );

    Unlock();
    DEBUGMSG( ZONE_HCD, (TEXT("-CHcd::SuspendResume - address = %d,fSuspend = %d, returning BOOL %d\n"), address,  fSuspend, fSuccess) );
    return fSuccess;

};




