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
#ifndef __PIPEABS_HPP_
#define __PIPEABS_HPP_

#define TD_ENDPOINT_MASK                    DWORD(0xF)

class CHcd;
class CPipeAbs { // Abstract Pipe class.
public:
    CPipeAbs(UCHAR const bEndpointAddress):m_bEndpointAddress(bEndpointAddress) {;};
    virtual ~CPipeAbs() {;};
    virtual HCD_REQUEST_STATUS  OpenPipe( void ) = 0;

    virtual HCD_REQUEST_STATUS  ClosePipe( void ) = 0;

    virtual HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR address,
                                IN LPTRANSFER_NOTIFY_ROUTINE const lpfnCallback,
                                IN LPVOID const lpvCallbackParameter,
                                IN const DWORD dwFlags,
                                IN LPCVOID const lpvControlHeader,
                                IN const DWORD dwStartingFrame,
                                IN const DWORD dwFrames,
                                IN LPCDWORD const aLengths,
                                IN const DWORD dwBufferSize,     
                                IN_OUT LPVOID const lpvBuffer,
                                IN const ULONG paBuffer,
                                IN LPCVOID const lpvCancelId,
                                OUT LPDWORD const adwIsochErrors,
                                OUT LPDWORD const adwIsochLengths,
                                OUT LPBOOL const lpfComplete,
                                OUT LPDWORD const lpdwBytesTransferred,
                                OUT LPDWORD const lpdwError )=0;

    virtual HCD_REQUEST_STATUS AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId ) = 0;

    virtual HCD_REQUEST_STATUS IsPipeHalted( OUT LPBOOL const lpbHalted )=0;

    virtual void ClearHaltedFlag( void )=0;
    virtual void ResetEndPoint(void) = 0;
    
    virtual void ChangeMaxPacketSize( IN const USHORT ) {ASSERT(FALSE);}; // only available for Controller endpoint

    virtual BOOL  SetReservedDeviceAddr(UCHAR bReservedDeviceAddr)=0;
    virtual UCHAR GetReservedDeviceAddr()=0;


    UCHAR const m_bEndpointAddress;
   
};

CPipeAbs * CreateBulkPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const BOOL ttContext,
               IN CHcd * const pChcd);

CPipeAbs * CreateControlPipe(IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const BOOL ttContext,
               IN CHcd * const pChcd);

CPipeAbs * CreateInterruptPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const BOOL ttContext,
               IN CHcd * const pChcd);

#if 0
CPipeAbs * CreateIsochronousPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,const PVOID ttContext,
               IN CHcd * const pChcd);
#endif

#endif

