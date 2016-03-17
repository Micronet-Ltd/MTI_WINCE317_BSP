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
//     CDevice.cpp
// Abstract:
//     This file manages the USB devices
//
//                  CDevice (ADT)
//                /               \
//            CFunction        CHub (ADT)
//                            /          \
//                        CRootHub   CExternalHub
//
// Notes:
//

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include "cdevice.hpp"
#include "hcd.hpp"
#include <usb200.h>
#include <ceotgbus.h>
#pragma warning(pop)

#if 0
#undef ZONE_ATTACH
#define ZONE_ATTACH 1

#undef ZONE_VERBOSE
#define ZONE_VERBOSE 1

#undef ZONE_HUB
#define ZONE_HUB 1

#undef ZONE_DEVICE
#define ZONE_DEVICE 1

#undef ZONE_DESCRIPTORS
#define ZONE_DESCRIPTORS 1
#endif

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// disable PREFAST warning for empty _except block
#pragma warning (disable: 6322)

#define STANDARD_REQUEST_TIMEOUT 500 // 500 Mili Second.
// address array variables - initially set to have address 0 used
// because it is reserved for the root hub (besides, no real device
// can permanently use addr0, since it is used at set address time)
//-----------------------------------------------------------------------
//  Function:   CDeviceGlobal::CDeviceGlobal
//
//  Routine Description:
//
//      Constructor for CDeviceGlobal class
//
//  Arguments:
//
//      Nil
//
//  Return:
//
//      Nil
//
CDeviceGlobal::CDeviceGlobal( ):m_objCountdown(0)
{

    m_dwFreeAddressArray[0]=0x00000001;
    m_dwFreeAddressArray[1] = m_dwFreeAddressArray[2] = m_dwFreeAddressArray[3] = 0x00000000;
    
    m_hUSBDInstance = NULL;
    m_pUSBDAttachProc = NULL;
    m_pUSBDDetachProc = NULL;
    m_pvHcdContext = NULL;
    m_pUSBDSelectConfigurationProc = NULL;
#ifdef DEBUG
    g_fAlreadyCalled = FALSE;
#endif // DEBUG
    //InitCritSec_Ex( &m_csAddress0Lock );
    InitializeCriticalSection( &m_csFreeAddressArrayLock );
};

//-----------------------------------------------------------------------
//  Function:   CDeviceGlobal::~CDeviceGlobal
//
//  Routine Description:
//
//      Destructor for CDeviceGlobal class
//
//  Arguments:
//
//      Nil
//
//  Return:
//
//      Nil
//
CDeviceGlobal::~CDeviceGlobal()
{
    DeInitialize();
    // all devices, and hence all addresses, should have been freed by now
    DeleteCriticalSection( &m_csFreeAddressArrayLock );
    //DeleteCritSec_Ex( &m_csAddress0Lock );
}
// ******************************************************************
BOOL CDeviceGlobal::Initialize(IN PVOID pHcd)
//
// Purpose: Initialize any static variables associated with
//          CDevice, and establish link to USBD
//
// Parameters: pHcd - pointer to the Host Controller Driver object which
//                    we pass to USBD
//
// Returns: TRUE
//
// Notes: This function should be called only once from CHcd::Initialize
// ******************************************************************
{
    DEBUGMSG( ZONE_INIT, (TEXT("%s: +CDeviceGlobal::Initialize\n"),GetControllerName()));
    m_pHcd=pHcd;
#ifdef DEBUG
    DEBUGCHK( !g_fAlreadyCalled );
    g_fAlreadyCalled = TRUE;
#endif // DEBUG

    DEBUGCHK( DWORD(8 * sizeof( m_dwFreeAddressArray )) == DWORD(USB_MAX_ADDRESS + 1) &&
              8 * sizeof( m_dwFreeAddressArray[0] ) == 32 &&
              m_dwFreeAddressArray[ 0 ] == 0x00000001 &&
              m_dwFreeAddressArray[ 1 ] == 0x00000000 &&
              m_dwFreeAddressArray[ 2 ] == 0x00000000 &&
              m_dwFreeAddressArray[ 3 ] == 0x00000000 );

    m_csAddress0Lock.Initialize();
    m_objCountdown.UnlockCountdown ();
    // establish links to USBD.dll
    {
        // this procedure is called to establish a link to USBD
        LPUSBD_HCD_ATTACH_PROC  lpHcdAttachProc = NULL;
        // this is defined in uhcddrv.cpp
        extern HCD_FUNCS gc_HcdFuncs;

        DEBUGCHK( m_pHcd != NULL &&
                  m_hUSBDInstance == NULL &&
                  m_pUSBDDetachProc == NULL &&
                  m_pUSBDAttachProc == NULL &&
                  m_pvHcdContext == NULL );

        m_hUSBDInstance = LoadDriver(TEXT("USBD.DLL"));
        if ( m_hUSBDInstance == NULL ) {
            DEBUGMSG(ZONE_ERROR,(TEXT("%s: -CDevice::Initialize - Could not load USBD.DLL\r\n"),GetControllerName()));
            return FALSE;
        }
        lpHcdAttachProc = (LPUSBD_HCD_ATTACH_PROC) GetProcAddress(m_hUSBDInstance, TEXT("HcdAttach"));
        m_pUSBDAttachProc = (LPUSBD_ATTACH_PROC) GetProcAddress(m_hUSBDInstance, TEXT("HcdDeviceAttached"));
        m_pUSBDDetachProc = (LPUSBD_DETACH_PROC) GetProcAddress(m_hUSBDInstance, TEXT("HcdDeviceDetached"));
        m_pUSBDSelectConfigurationProc = (LPUSBD_SELECT_CONFIGURATION_PROC)GetProcAddress(m_hUSBDInstance, TEXT("HcdSelectConfiguration"));// Optional
        m_pUSBDSuspendResumed = (LPUSBD_SUSPEND_RESUME_PROC) GetProcAddress(m_hUSBDInstance, TEXT("HcdDeviceSuspendeResumed"));
        if ( m_pUSBDAttachProc == NULL ||
             m_pUSBDDetachProc == NULL ||
             lpHcdAttachProc == NULL ||
             (*lpHcdAttachProc)(m_pHcd, &gc_HcdFuncs,  &m_pvHcdContext) == FALSE ) {

            DEBUGMSG(ZONE_ERROR, (TEXT("%s: -CDevice::Initialize - Could not establish USBD links\n"),GetControllerName()));
            return FALSE;
        }
        DEBUGCHK( m_pvHcdContext != NULL );
    }


    DEBUGMSG( ZONE_INIT, (TEXT("%s: -CDevice::Initialize, success!\n"),GetControllerName()));
    return TRUE;
}

// ******************************************************************
void CDeviceGlobal::DeInitialize( )
//
// Purpose: Delete any static variables associated with CDevice
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: This function should be called only once from CHcd::~CHcd
// ******************************************************************
{
    DEBUGMSG( ZONE_INIT, (TEXT("%s: +CDevice::DeInitialize\n"),GetControllerName()));

#ifdef DEBUG
    DEBUGCHK(g_fAlreadyCalled == TRUE);
    g_fAlreadyCalled = FALSE;
#endif // DEBUG

    // wait for any stray detach threads
    // This can block waiting for a callback into a client driver to return.
    // Since callbacks aren't supposed to block this oughtn't cause deadlock,
    // but a misbehaving client driver can cause us serious grief.
    // Nonetheless, not waiting means we might free USBD.DLL while it's still in use.
    //DeleteCountdown(&m_objCountdown);
    m_objCountdown.WaitForCountdown( TRUE);

    // unload USBD.dll
    if ( m_hUSBDInstance ) {
        LPUSBD_HCD_DETACH_PROC lpHcdDetachProc;
        lpHcdDetachProc = (LPUSBD_HCD_DETACH_PROC) GetProcAddress(m_hUSBDInstance, TEXT("HcdDetach"));
        if ( lpHcdDetachProc != NULL ) {
            (*lpHcdDetachProc)(m_pvHcdContext);
        }
        FreeLibrary( m_hUSBDInstance );
        m_hUSBDInstance = NULL;
    }
    m_pUSBDAttachProc = NULL;
    m_pUSBDDetachProc = NULL;
    m_pvHcdContext = NULL;

    m_dwFreeAddressArray[0] = 0x00000001;
    m_dwFreeAddressArray[1] = m_dwFreeAddressArray[2] = m_dwFreeAddressArray[3] = 0x00000000;


    DEBUGMSG( ZONE_INIT, (TEXT("%s: -CDevice::DeInitialize\n"),GetControllerName()));
}

// ******************************************************************
BOOL CDeviceGlobal::ReserveAddress( OUT UCHAR& rAddress )
//
// Purpose: Finds an unused USB address (1-127), marks it as used, and
//          returns the address
//
// Parameters: rAddress - OUT parameter, which is set to a free address
//
// Returns: TRUE if rAddress set to a valid free address, else FALSE
//
// Notes: Address 0 is permanently marked as used. This is reserved for
//        the root hub.
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CDeviceGlobal::ReserveAddress\n"),GetControllerName()) );

    BOOL fSuccess = FALSE;

    EnterCriticalSection( &m_csFreeAddressArrayLock );

    // the address has 7 bits:
    //
    // xxyyyyyb
    //
    // xxb is the index into the m_dwFreeAddressArray (0-3)
    // yyyyyb is the bit of the m_dwFreeAddressArray[xxb] DWORD
    // that the address corresponds to

    // address 0 should always be marked used
    DEBUGCHK(m_dwFreeAddressArray[0] & 1 );

    for ( UCHAR address = 1; address <= USB_MAX_ADDRESS; address++ ) {
        const UCHAR index = (address >> 5); // 5 == log base 2 of 32
        const UCHAR bit = address & (32 - 1); // 32 == # of bits in DWORD
        if ( (m_dwFreeAddressArray[ index ] & (1 << bit)) == 0 ) {
            // this address is free
            fSuccess = TRUE;
            rAddress = address;
            // mark address as used
            m_dwFreeAddressArray[ index ] |= (1 << bit);
            break;
        }
    }
    LeaveCriticalSection( &m_csFreeAddressArrayLock );

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CDevice::ReserveAddress, returning rAddress %d, success = %d\n"),GetControllerName(), rAddress, fSuccess ) );
    return fSuccess;
}
// ******************************************************************
void CDeviceGlobal::FreeAddress( IN const UCHAR address )
//
// Purpose: Return address to the list of unused USB device addresses
//
// Parameters: address - address to free
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CDeviceGlobal::FreeAddress - address = %d\n"),GetControllerName(), address) );

    EnterCriticalSection( &m_csFreeAddressArrayLock );

    // the address has 7 bits:
    //
    // xxyyyyyb
    //
    // xxb is the index into the m_dwFreeAddressArray (0-3)
    // yyyyyb is the bit of the m_dwFreeAddressArray[xxb] DWORD
    // that the address corresponds to

    const UCHAR index = (address >> 5); // 5 == log base 2 of 32
    const UCHAR bit = address & (32 - 1); // 32 == # of bits in DWORD

    // make sure this address is marked as used
    DEBUGCHK( index < 4 &&
              (m_dwFreeAddressArray[ index ] & ( 1 << bit )) );

    // free this address
    m_dwFreeAddressArray[ index ] &= ~(1 << bit);

    LeaveCriticalSection( &m_csFreeAddressArrayLock );

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CDevice::FreeAddress - address = %d\n"),GetControllerName(), address) );
}

// ******************************************************************
DWORD CALLBACK CDevice::TransferDoneCallbackSetEvent( PVOID context )
//
// Purpose: This function is a callback for the CPipe class. When a
//          transfer completes, and this function was set in the
//          lpStartAddress field of IssueTransfer, we will be called
//
// Parameters: context - HANDLE to an event we should signal
//
// Returns: 0
//
// Notes: Calling this function directly is rather useless (it will
//        just have the same effect as SetEvent( context ) ), so
//        it should only be used as a callback
// ******************************************************************
{
    DEBUGCHK( context );
    SetEvent( (HANDLE) context );
    return 0;
}

// ******************************************************************
CDevice::CDevice( IN const UCHAR address,
                  IN const USB_DEVICE_INFO& rDeviceInfo,
                  IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                  IN const UCHAR tierNumber,
                  IN CDeviceGlobal * const pDeviceGlobal ,
                  IN CHub * const pAttachedHub,const UCHAR sAttachedPort)
//
// Purpose: Constructor for CDevice
//
// Parameters: address - USB address of this device. This will also be
//                       used as the device's index number when
//                       communicating with USBD
//
//             rDeviceInfo- object containing device's USB descriptors
//
//             fIsLowSpeed - indicates whether this device is low speed
//
//             tierNumber - indicates how far away this device is from
//                          the root hub
//
// Returns: Nothing.
//
// Notes: Do not initialize static variables here. Do that in
//        the Initialize() routine
// ******************************************************************
: m_address( address ) // USB address of this device
, m_deviceInfo( rDeviceInfo ) // USB descriptors/information about this device
, m_fIsLowSpeed( fIsLowSpeed ) // indicates TRUE for low speed devices
, m_fIsHighSpeed(fIsHighSpeed)
, m_tierNumber( tierNumber ) // tier number of device (0 for root hub, 1 for first tier, etc)
, m_pDeviceGlobal (pDeviceGlobal )
, m_pAttachedHub (pAttachedHub)
, m_sAttachedPort(sAttachedPort)
, m_maxNumPipes( 0 ) // current size of m_ppCPipe array
, m_ppCPipe( NULL ) // dynamically allocated array of pointers to open pipes
{
    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: +CDevice::CDevice\n"),GetControllerName()) );
    m_fIsSuspend = FALSE;
    DEBUGCHK( m_deviceInfo.dwCount == sizeof( USB_DEVICE ) &&
              m_deviceInfo.Descriptor.bDescriptorType == USB_DEVICE_DESCRIPTOR_TYPE &&
              m_deviceInfo.Descriptor.bLength == sizeof( USB_DEVICE_DESCRIPTOR ) &&
              address <= USB_MAX_ADDRESS &&
              tierNumber <= USB_MAXIMUM_HUB_TIER + 1 );

    InitializeCriticalSection( &m_csDeviceLock );
    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: -CDevice::CDevice\n"),GetControllerName()) );
}

// ******************************************************************
CDevice::~CDevice( )
//
// Purpose: Destructor for CDevice
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: Do not delete static variables here. Do that in
//        DeInitialize();
// ******************************************************************
{
    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: +CDevice::~CDevice\n"),GetControllerName()) );

    // delete m_deviceInfo structure
    if ( m_deviceInfo.lpConfigs != NULL ) {
        DEBUGCHK( m_deviceInfo.Descriptor.bDescriptorType == USB_DEVICE_DESCRIPTOR_TYPE &&
                  m_deviceInfo.Descriptor.bLength == sizeof( USB_DEVICE_DESCRIPTOR ) &&
                  m_deviceInfo.dwCount == sizeof( USB_DEVICE_INFO ) &&
                  m_deviceInfo.Descriptor.bNumConfigurations > 0 );
        for ( UINT config = 0; config < m_deviceInfo.Descriptor.bNumConfigurations; config++ ) {
            DeleteUsbConfigurationStructure( m_deviceInfo.lpConfigs[ config ] );
        }
        delete [] m_deviceInfo.lpConfigs;
        m_deviceInfo.lpConfigs = NULL;
    }
    m_deviceInfo.lpActiveConfig = NULL;

#ifdef DEBUG
{
    DEBUGCHK( (m_ppCPipe == NULL && m_maxNumPipes == 0) ||
              (m_ppCPipe != NULL && m_maxNumPipes > 0) );
    // all pipes should have been closed/deleted by HandleDetach
    for ( UCHAR pipe = 0; m_ppCPipe && pipe < m_maxNumPipes; pipe++ ) {
        DEBUGCHK( m_ppCPipe[ pipe ] == NULL );
    }
}
#endif // DEBUG
    delete [] m_ppCPipe;
    m_ppCPipe = NULL;
    m_maxNumPipes = 0;

    // free address
    FreeAddress( m_address );

    // nothing to be done with any of these:
    // m_deviceInfo;   // holds device's USB descriptors
    // m_fIsLowSpeed;  // indicates if device is low speed
    // m_tierNumber;   // indicates tier # of device

    DeleteCriticalSection( &m_csDeviceLock );
    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: -CDevice::~CDevice\n"),GetControllerName()) );
}
// ******************************************************************
CHub * CDevice::GetUSB2TT(UCHAR& pTTAddr, UCHAR& pTTPort,BOOL& ttContext)
//
// Purpose: Found Transaction Translate for Full Speed Device.
// Returns: The Hub object where this TT located
//
// ******************************************************************
{
    if (!m_fIsHighSpeed) {
        CHub *  pHub = m_pAttachedHub;
        UCHAR  sAttachedPort = m_sAttachedPort;
        while (pHub!=NULL && pHub->m_fIsHighSpeed!=TRUE) {
            sAttachedPort = pHub->m_sAttachedPort;
            pHub =  pHub->m_pAttachedHub;
        }
        if (pHub) {
            pTTAddr = pHub->m_address;
            pTTPort = sAttachedPort;
            ASSERT(sAttachedPort!=0 && sAttachedPort<= pHub->m_usbHubDescriptor.bNumberOfPorts);
            ttContext = pHub->m_pAddedTT[sAttachedPort-1];
        }
        return pHub;
    }
    return NULL;
}
// ******************************************************************
BOOL CDevice::CreateUsbConfigurationStructure( IN NON_CONST_USB_CONFIGURATION& rConfig, IN const PUCHAR pDataBuffer, IN const UINT dataBufferLen ) const
//
// Purpose: Fill in rConfig using data from the given pDataBuffer
//
// Parameters: rConfig - reference to NON_CONST_USB_CONFIGURATION structure to fill in
//
//             pDataBuffer - data buffer from which to create NON_CONST_USB_CONFIGURATION.
//                           This should have been the data retrieved from the USB
//                           device's GET_CONFIGURATION_DESCRIPTOR request
//
//             dataBufferLen - length of pDataBuffer
//
// Returns: TRUE if configuration set properly, else FALSE
//
// Notes: This should be called after the Descriptor field of the configuration
//        has already been filled in
//
//        This function is protected
// ******************************************************************
{
    DEBUGMSG( ZONE_DESCRIPTORS && ZONE_VERBOSE, (TEXT("%s: +CDevice::CreateUsbConfigurationStructure\n"),GetControllerName()));

    DEBUGCHK( pDataBuffer != NULL &&
              dataBufferLen == rConfig.Descriptor.wTotalLength );

    PUSB_CONFIGURATION_DESCRIPTOR pusbConfigDesc = (PUSB_CONFIGURATION_DESCRIPTOR) pDataBuffer;
    DEBUGCHK( memcmp( &rConfig.Descriptor, pusbConfigDesc, sizeof( USB_CONFIGURATION_DESCRIPTOR ) ) == 0 );

#ifdef DEBUG
    DumpConfigDescriptor( &rConfig.Descriptor );
#endif // DEBUG

    rConfig.dwNumInterfaces = 0;
    rConfig.lpbExtended = NULL;
    rConfig.lpInterfaces = NULL;
    rConfig.dwExtendedSize = 0;

    BOOL retval = FALSE;

    if ( pusbConfigDesc != NULL &&
         pusbConfigDesc->wTotalLength == rConfig.Descriptor.wTotalLength &&
         dataBufferLen == pusbConfigDesc->wTotalLength &&
         pusbConfigDesc->bLength >= sizeof( USB_CONFIGURATION_DESCRIPTOR ) &&
         pusbConfigDesc->bDescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE ) {

        UINT offset = pusbConfigDesc->bLength;
        PUSB_COMMON_DESCRIPTOR pusbCommon = NULL;
        rConfig.dwCount = sizeof ( NON_CONST_USB_CONFIGURATION );

        // first step - count number of extended bytes for this config descriptor,
        // and copy data if needed
        {
            UINT configDescExtendedBytes = 0;
            while ( offset + configDescExtendedBytes < dataBufferLen ) {
                pusbCommon = (PUSB_COMMON_DESCRIPTOR)(pDataBuffer + offset + configDescExtendedBytes );
                if ( pusbCommon->bDescriptorType != USB_INTERFACE_DESCRIPTOR_TYPE ) {
                    configDescExtendedBytes += pusbCommon->bLength;
                } else {
                    break;
                }
            }
            // next, copy Config Descriptor's extended bytes
            if ( configDescExtendedBytes > 0 ) {
                rConfig.lpbExtended = new BYTE[ configDescExtendedBytes ];
                if ( rConfig.lpbExtended == NULL ) {
                    goto configDescMemoryError;
                }
                rConfig.dwExtendedSize = configDescExtendedBytes ;
                memcpy(  rConfig.lpbExtended, pDataBuffer + offset, configDescExtendedBytes );
                offset += configDescExtendedBytes;
            #ifdef DEBUG
                DumpExtendedBytes( rConfig.lpbExtended, configDescExtendedBytes );
            #endif // DEBUG
            }
        }
        // second step - get the number of interfaces for this configuration
        // note - this isn't always the same as the bNumInterfaces field of
        // rConfig.Descriptor, due to Alternate settings for Interfaces
        {
            UINT x = 0; // temporary counter
            DEBUGCHK( rConfig.dwNumInterfaces == 0 );
            while ( offset + x + sizeof( USB_INTERFACE_DESCRIPTOR ) < dataBufferLen ) {
                pusbCommon = (PUSB_COMMON_DESCRIPTOR)(pDataBuffer + offset + x);
                if ( pusbCommon->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE ) {
                    rConfig.dwNumInterfaces++;
                }
                x += pusbCommon->bLength;
            }
        }

        // next - create array for INTERFACE objects
        DEBUGCHK( rConfig.lpInterfaces == NULL );
        if ( rConfig.dwNumInterfaces > 0 ) {
            // allocate this many interface objects
            rConfig.lpInterfaces = new NON_CONST_USB_INTERFACE[ rConfig.dwNumInterfaces ];
            if ( rConfig.lpInterfaces == NULL ) {
                goto configDescMemoryError;
            }
            memset( rConfig.lpInterfaces, 0, rConfig.dwNumInterfaces * sizeof( NON_CONST_USB_INTERFACE ) );
            for ( UCHAR interfaceNumber = 0; interfaceNumber < rConfig.dwNumInterfaces; interfaceNumber++ ) {
                NON_CONST_USB_INTERFACE & rInterface = rConfig.lpInterfaces[ interfaceNumber ];
                rInterface.dwCount = sizeof( NON_CONST_USB_INTERFACE );
                // for each interface,
                // 1) Copy the interface descriptor
                // 2) Allocate and copy any extended bytes
                // 3) Allocate room for endpoints, if any
                // 4) Copy over endpoints -
                //      for each endpoint:
                //      a) copy the endpoint descriptor
                //      b) copy any extended bytes


                // we should now be pointing to a complete USB_INTERFACE_DESCRIPTOR
                DEBUGCHK( offset + sizeof( USB_CONFIGURATION_DESCRIPTOR ) <= dataBufferLen );
                PUSB_INTERFACE_DESCRIPTOR pusbInterfaceDesc = (PUSB_INTERFACE_DESCRIPTOR)(pDataBuffer + offset);
                DEBUGCHK( pusbInterfaceDesc->bLength >= sizeof( USB_INTERFACE_DESCRIPTOR ) &&
                          pusbInterfaceDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE );

                // 1) copy interface descriptor, and skip over it
                memcpy( &rInterface.Descriptor, pusbInterfaceDesc, sizeof( USB_INTERFACE_DESCRIPTOR ) );
                offset += pusbInterfaceDesc->bLength;
            #ifdef DEBUG
                DumpInterfaceDescriptor( &rInterface.Descriptor );
            #endif // DEBUG

                // 2) copy any extended info, if it exists
                {
                    UINT interfaceDescExtendedBytes = 0;
                    while ( offset + interfaceDescExtendedBytes < dataBufferLen ) {
                        pusbCommon = (PUSB_COMMON_DESCRIPTOR)(pDataBuffer + offset + interfaceDescExtendedBytes);
                        if ( pusbCommon->bDescriptorType != USB_INTERFACE_DESCRIPTOR_TYPE &&
                             pusbCommon->bDescriptorType != USB_ENDPOINT_DESCRIPTOR_TYPE &&
                             offset + interfaceDescExtendedBytes + pusbCommon->bLength <= dataBufferLen ) {

                            interfaceDescExtendedBytes += pusbCommon->bLength;
                        } else {
                            break;
                        }
                    }
                    DEBUGCHK( rInterface.lpbExtended == NULL );
                    if ( interfaceDescExtendedBytes > 0 ) {
                        rInterface.lpbExtended = new BYTE[ interfaceDescExtendedBytes ];
                        if ( rInterface.lpbExtended == NULL ) {
                            goto configDescMemoryError;
                        }
                        rInterface.dwExtendedSize = interfaceDescExtendedBytes ;
                        memcpy( rInterface.lpbExtended, pDataBuffer + offset, interfaceDescExtendedBytes );
                        offset += interfaceDescExtendedBytes;
                    #ifdef DEBUG
                        DumpExtendedBytes( rInterface.lpbExtended, interfaceDescExtendedBytes );
                    #endif // DEBUG
                    }
                }

                // 3) allocate any endpoints
                DEBUGCHK( rInterface.lpEndpoints == NULL );
                if ( rInterface.Descriptor.bNumEndpoints == 0 ) {
                    continue; // continue interface loop
                }
                rInterface.lpEndpoints = new NON_CONST_USB_ENDPOINT[ rInterface.Descriptor.bNumEndpoints ];
                if ( rInterface.lpEndpoints == NULL ) {
                    goto configDescMemoryError;
                }

                memset( rInterface.lpEndpoints, 0, rInterface.Descriptor.bNumEndpoints * sizeof( NON_CONST_USB_ENDPOINT ) );
                for ( UINT endpoint = 0; endpoint < rInterface.Descriptor.bNumEndpoints; endpoint++ ) {
                    NON_CONST_USB_ENDPOINT & rEndpoint = rInterface.lpEndpoints[ endpoint ];
                    rEndpoint.dwCount = sizeof( NON_CONST_USB_ENDPOINT );

                    // should now be pointing at an endpoint descriptor
                    PUSB_ENDPOINT_DESCRIPTOR pusbEndpointDesc = (PUSB_ENDPOINT_DESCRIPTOR)(pDataBuffer + offset);
                    DEBUGCHK( pusbEndpointDesc->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
                              pusbEndpointDesc->bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) );

                    // 4a) copy the endpoint descriptor
                    memcpy( &rEndpoint.Descriptor, pDataBuffer + offset, sizeof( USB_ENDPOINT_DESCRIPTOR ) );
                    offset += pusbEndpointDesc->bLength;
                #ifdef DEBUG
                    DumpEndpointDescriptor( &rEndpoint.Descriptor );
                #endif // DEBUG

                    // 4b) copy any extended info, if it exists
                    UINT endpointDescExtendedBytes = 0;
                    while ( offset + endpointDescExtendedBytes < dataBufferLen ) {
                        pusbCommon = (PUSB_COMMON_DESCRIPTOR)(pDataBuffer + offset + endpointDescExtendedBytes);
                        if ( pusbCommon->bDescriptorType != USB_ENDPOINT_DESCRIPTOR_TYPE &&
                             pusbCommon->bDescriptorType != USB_INTERFACE_DESCRIPTOR_TYPE &&
                             offset + endpointDescExtendedBytes + pusbCommon->bLength <= dataBufferLen ) {

                            endpointDescExtendedBytes += pusbCommon->bLength;
                        } else {
                            break;
                        }
                    }
                    DEBUGCHK( rEndpoint.lpbExtended == NULL );
                    if ( endpointDescExtendedBytes > 0 ) {
                        rEndpoint.lpbExtended = new BYTE[ endpointDescExtendedBytes ];
                        if ( rEndpoint.lpbExtended == NULL ) {
                            goto configDescMemoryError;
                        }
                        rEndpoint.dwExtendedSize = endpointDescExtendedBytes ;
                        memcpy( rEndpoint.lpbExtended, pDataBuffer + offset, endpointDescExtendedBytes );
                        offset += endpointDescExtendedBytes;
                    #ifdef DEBUG
                        DumpExtendedBytes( rEndpoint.lpbExtended, endpointDescExtendedBytes );
                    #endif // DEBUG
                    }
                } // end endpoint for loop
            } // end interface for loop
        } // end check for interfaces
        retval = TRUE;
    }
    DEBUGMSG( ZONE_DESCRIPTORS && ZONE_VERBOSE, (TEXT("%s: -CDevice::CreateUsbConfigurationStructure - returning %d\n"),GetControllerName(), retval));
    return retval;


configDescMemoryError:
    DEBUGMSG( ZONE_ERROR, (TEXT("%s: -CDevice::CreateUsbConfigurationStructure - error allocating memory\n"),GetControllerName()));
    DeleteUsbConfigurationStructure( rConfig );
    return FALSE;
}

// ******************************************************************
void CDevice::DeleteUsbConfigurationStructure( IN NON_CONST_USB_CONFIGURATION& rConfig ) const
//
// Purpose: Free the memory associated with the rConfig structure
//
// Parameters: rConfig - reference to NON_CONST_USB_CONFIGURATION struct to free
//
// Returns: Nothing
//
// Notes: This function is protected
// ******************************************************************
{
    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: +CDevice::DeleteUsbConfigurationStructure\n"),GetControllerName()));

    // this code is right out of the destructor of OHCD.cpp
    if ( rConfig.lpInterfaces ) {
        DEBUGCHK( rConfig.Descriptor.bDescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE &&
                  rConfig.Descriptor.bLength == sizeof( USB_CONFIGURATION_DESCRIPTOR ) &&
                  rConfig.dwNumInterfaces >= rConfig.Descriptor.bNumInterfaces );
        LPNON_CONST_USB_INTERFACE lpInterface = rConfig.lpInterfaces;
        for ( UINT iInterface = 0; iInterface < rConfig.dwNumInterfaces; ++iInterface, ++lpInterface ) {
            if ( lpInterface->lpEndpoints ) {
                DEBUGCHK( lpInterface->Descriptor.bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE );
                LPNON_CONST_USB_ENDPOINT lpEndpoint = lpInterface->lpEndpoints;
                for ( UINT iEndpoint = 0; iEndpoint < lpInterface->Descriptor.bNumEndpoints; ++iEndpoint, ++lpEndpoint ) {
                    delete [] lpEndpoint->lpbExtended;
                    lpEndpoint->lpbExtended = NULL;
                }
                delete [] lpInterface->lpEndpoints;
                lpInterface->lpEndpoints = NULL;
            }
            delete [] lpInterface->lpbExtended;
            lpInterface->lpbExtended = NULL;
        }
        delete [] rConfig.lpInterfaces;
        rConfig.lpInterfaces = NULL;
    }
    delete [] rConfig.lpbExtended;
    rConfig.lpbExtended = NULL;

    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: -CDevice::DeleteUsbConfigurationStructure\n"),GetControllerName()));
}

// ******************************************************************
BOOL CDevice::AllocatePipeArray( void )
//
// Purpose: Allocate memory for the pipe array of this device based on
//          the number of endpoints given in the m_deviceInfo structure
//
// Parameters: None
//
// Returns: TRUE if array allocated, else FALSE
//
// Notes: The entries of the array will be set to NULL by this function
// ******************************************************************
{
    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: +CDevice(%s tier %d)::AllocatePipeArray\n"),GetControllerName(), GetDeviceType(), m_tierNumber ) );

    BOOL fSuccess = FALSE;

    EnterCriticalSection( &m_csDeviceLock );

    DEBUGCHK( m_ppCPipe == NULL && // shouldn't be allocated yet
              m_maxNumPipes == 0 && // shouldn't be allocated yet
              m_deviceInfo.lpActiveConfig != NULL &&
              m_deviceInfo.lpActiveConfig->lpInterfaces != NULL &&
              m_deviceInfo.lpActiveConfig->lpInterfaces[0].Descriptor.bNumEndpoints <= 15 );

    // number of endpoints does not include the endpoint 0
    int numPipes = 1;

    int bNumberEndpoints = 0;
    int bInterfaceNumber = m_deviceInfo.lpActiveConfig->lpInterfaces[0].Descriptor.bInterfaceNumber;
    int bCurNumEndpoints;
    for(DWORD i = 0; i < m_deviceInfo.lpActiveConfig->dwNumInterfaces; ++i){
        if(bInterfaceNumber == m_deviceInfo.lpActiveConfig->lpInterfaces[i].Descriptor.bInterfaceNumber){
            bCurNumEndpoints = m_deviceInfo.lpActiveConfig->lpInterfaces[i].Descriptor.bNumEndpoints;
            if(bNumberEndpoints < bCurNumEndpoints)
                bNumberEndpoints = bCurNumEndpoints;
        } else {
            bInterfaceNumber = m_deviceInfo.lpActiveConfig->lpInterfaces[i].Descriptor.bInterfaceNumber;
            numPipes += bNumberEndpoints;
            bNumberEndpoints = m_deviceInfo.lpActiveConfig->lpInterfaces[i].Descriptor.bNumEndpoints;
        }
    }
    numPipes += bNumberEndpoints;

    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: CDevice(%s tier %d)::AllocatePipeArray - attempting to allocate %d pipes\n"),GetControllerName(), GetDeviceType(), m_tierNumber, numPipes) );
    m_ppCPipe = new CPipeAbs* [ numPipes ];
    if ( m_ppCPipe != NULL ) {
        memset( m_ppCPipe, 0, numPipes * sizeof( CPipeAbs * ) );
        m_maxNumPipes = (UCHAR)numPipes;
        fSuccess = TRUE;
    }
    DEBUGMSG( ZONE_ERROR && !m_ppCPipe, (TEXT("%s: CDevice(%s tier %d)::AllocatePipeArray - no memory!\n"),GetControllerName(), GetDeviceType(), m_tierNumber ) );

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_DEVICE && ZONE_VERBOSE, (TEXT("%s: -CDevice(%s tier %d)::AllocatePipeArray, returning BOOL %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, fSuccess) );
    return fSuccess;
}
#ifdef DEBUG
// ******************************************************************
void CDevice::DumpDeviceDescriptor( IN const PUSB_DEVICE_DESCRIPTOR pDescriptor ) const
//
// Purpose: print out the contents of the descriptor via DEBUGMSG
//
// Parameters: pDescriptor - pointer to descriptor
//
// Returns: Nothing.
//
// Notes: Used in debug mode only
//
//        See USB spec section 9.6.1
// ******************************************************************
{
    DEBUGCHK( pDescriptor != NULL );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: +Dump USB_DEVICE_DESCRIPTOR\n"),GetControllerName()) );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbLength = 0x%02x\n"), pDescriptor->bLength ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDescriptorType = 0x%02x\n"), pDescriptor->bDescriptorType ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbcdUSB = 0x%04x\n"), pDescriptor->bcdUSB ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDeviceClass = 0x%02x\n"), pDescriptor->bDeviceClass ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDeviceSubClass = 0x%02x\n"), pDescriptor->bDeviceSubClass ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDeviceProtocol = 0x%02x\n"), pDescriptor->bDeviceProtocol ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbMaxPacketSize0 = 0x%02x\n"), pDescriptor->bMaxPacketSize0 ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tidVendor = 0x%04x\n"), pDescriptor->idVendor ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tidProduct = 0x%04x\n"), pDescriptor->idProduct ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbcdDevice = 0x%04x\n"), pDescriptor->bcdDevice ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tiManufacturer = 0x%02x\n"), pDescriptor->iManufacturer ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tiProduct = 0x%02x\n"), pDescriptor->iProduct ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tiSerialNumber = 0x%02x\n"), pDescriptor->iSerialNumber ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbNumConfigurations = 0x%02x\n"), pDescriptor->bNumConfigurations ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: -Dump USB_DEVICE_DESCRIPTOR\n"),GetControllerName()) );
}
#endif // DEBUG
#ifdef DEBUG
// ******************************************************************
void CDevice::DumpConfigDescriptor( IN const PUSB_CONFIGURATION_DESCRIPTOR pDescriptor ) const
//
// Purpose: print out the contents of the descriptor via DEBUGMSG
//
// Parameters: pDescriptor - pointer to descriptor
//
// Returns: Nothing.
//
// Notes: Used in debug mode only
//
//        See USB spec section 9.6.2
// ******************************************************************
{
    DEBUGCHK( pDescriptor != NULL );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: +Dump USB_CONFIGURATION_DESCRIPTOR\n"),GetControllerName()) );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbLength = 0x%02x\n"), pDescriptor->bLength ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDescriptorType = 0x%02x\n"), pDescriptor->bDescriptorType ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\twTotalLength = 0x%04x\n"), pDescriptor->wTotalLength ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbNumInterfaces = 0x%02x\n"), pDescriptor->bNumInterfaces ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbConfigurationValue = 0x%02x\n"), pDescriptor->bConfigurationValue ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tiConfiguration = 0x%02x\n"), pDescriptor->iConfiguration ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbmAttributes = 0x%02x\n"), pDescriptor->bmAttributes ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\tbmAttributes, Bus Powered = %d\n"), !!(pDescriptor->bmAttributes & BUS_POWERED) ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\tbmAttributes, Self Powered = %d\n"), !!(pDescriptor->bmAttributes & SELF_POWERED) ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\tbmAttributes, Remote Wakeup = %d\n"), !!(pDescriptor->bmAttributes & REMOTE_WAKEUP) ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tMaxPower = 0x%02x\n"), pDescriptor->MaxPower ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: -Dump USB_CONFIGURATION_DESCRIPTOR\n"),GetControllerName()) );
}
#endif // DEBUG
#ifdef DEBUG
// ******************************************************************
void CDevice::DumpInterfaceDescriptor( IN const PUSB_INTERFACE_DESCRIPTOR pDescriptor ) const
//
// Purpose: print out the contents of the descriptor via DEBUGMSG
//
// Parameters: pDescriptor - pointer to descriptor
//
// Returns: Nothing.
//
// Notes: Used in debug mode only
//
//        See USB spec section 9.6.3
// ******************************************************************
{
    DEBUGCHK( pDescriptor != NULL );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: +Dump USB_INTERFACE_DESCRIPTOR\n"),GetControllerName()) );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbLength = 0x%02x\n"), pDescriptor->bLength ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDescriptorType = 0x%02x\n"), pDescriptor->bDescriptorType ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbInterfaceNumber = 0x%02x\n"), pDescriptor->bInterfaceNumber ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbAlternateSetting = 0x%02x\n"), pDescriptor->bAlternateSetting ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbNumEndpoints = 0x%02x\n"), pDescriptor->bNumEndpoints ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbInterfaceClass = 0x%02x\n"), pDescriptor->bInterfaceClass ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbInterfaceSubClass = 0x%02x\n"), pDescriptor->bInterfaceSubClass ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbInterfaceProtocol = 0x%02x\n"), pDescriptor->bInterfaceProtocol ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tiInterface = 0x%02x\n"), pDescriptor->iInterface ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: -Dump USB_INTERFACE_DESCRIPTOR\n"),GetControllerName()) );
}
#endif // DEBUG
#ifdef DEBUG
// ******************************************************************
void CDevice::DumpEndpointDescriptor( IN const PUSB_ENDPOINT_DESCRIPTOR pDescriptor ) const
//
// Purpose: print out the contents of the descriptor via DEBUGMSG
//
// Parameters: pDescriptor - pointer to descriptor
//
// Returns: Nothing.
//
// Notes: Used in debug mode only
//
//        See USB spec section 9.6.4
// ******************************************************************
{
    static const TCHAR* cszEndpointTypes[4] = {
        TEXT("CONTROL"),
        TEXT("ISOCHRONOUS"),
        TEXT("BULK"),
        TEXT("INTERRUPT")
    };
    DEBUGCHK( pDescriptor != NULL );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: +Dump USB_ENDPOINT_DESCRIPTOR\n"),GetControllerName()) );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbLength = 0x%02x\n"), pDescriptor->bLength ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDescriptorType = 0x%02x\n"), pDescriptor->bDescriptorType ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbEndpointAddress = 0x%02x\n"), pDescriptor->bEndpointAddress ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\tbEndpointAddress, endpoint # = %d\n"), pDescriptor->bEndpointAddress & TD_ENDPOINT_MASK ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\tbEndpointAddress, direction = %s\n"), (USB_ENDPOINT_DIRECTION_IN(pDescriptor->bEndpointAddress) ? TEXT("IN") : TEXT("OUT")) ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbmAttributes = 0x%02x\n"), pDescriptor->bmAttributes ));
    DEBUGCHK( (pDescriptor->bmAttributes & USB_ENDPOINT_TYPE_MASK) < 4 );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\tbmAttributes, endpoint type = %s\n"), cszEndpointTypes[ pDescriptor->bmAttributes & USB_ENDPOINT_TYPE_MASK ] ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\twMaxPacketSize = 0x%04x\n"), pDescriptor->wMaxPacketSize ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbInterval = 0x%02x\n"), pDescriptor->bInterval ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: -Dump USB_ENDPOINT_DESCRIPTOR\n"),GetControllerName()) );
}
#endif // DEBUG

#ifdef DEBUG
// ******************************************************************
void CDevice::DumpExtendedBytes( IN const PBYTE pByteArray, IN const DWORD dwSize ) const
//
// Purpose: print out the bytes of pByteArray
//
// Parameters: pByteArray - array of extended bytes for a descriptor
//
//             dwSize - number of entries in pByteArray
//
// Returns: Nothing.
//
// Notes: Used in debug mode only
// ******************************************************************
{
    DEBUGCHK( pByteArray != NULL && dwSize > 0 );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: +Dump extended bytes, size = %d\n"),GetControllerName(), dwSize) );
    for ( DWORD dwPrinted = 0; dwPrinted < dwSize; dwPrinted += 4 ) {
        DWORD dwFourBytes = 0;
        for ( UCHAR index = 0; index < 4; index++  ) {
            dwFourBytes <<= 8;
            if ( dwPrinted + index < dwSize ) {
                dwFourBytes |= pByteArray[ dwPrinted + index ];
            }
        }
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tBytes %d to %d = 0x%08x\n"), dwPrinted + 1, dwPrinted + 4, dwFourBytes ) );
    }
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("%s: -Dump extended bytes, size = %d\n"),GetControllerName(), dwSize) );
}
#endif // DEBUG

// ******************************************************************
CHub::CHub( IN const UCHAR address,
            IN const USB_DEVICE_INFO& rDeviceInfo,
            IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
            IN const UCHAR tierNumber,
            IN const USB_HUB_DESCRIPTOR& rUsbHubDescriptor,
            IN CHcd * const pCHcd ,
            IN CHub * const pAttachedHub,const UCHAR uAttachedPort)
//
// Purpose: Constructor for CHub
//
// Parameters: address, rDeviceInfo, fIsLowSpeed, tierNumber - see CDevice::CDevice
//
//             rUsbHubDescriptor - USB descriptor for a hub
//
// Returns: Nothing.
//
// Notes: Do not initialize static variables here. Do that in
//        the Initialize() routine
// ******************************************************************
: CDevice( address, rDeviceInfo, fIsLowSpeed,fIsHighSpeed, tierNumber,pCHcd , pAttachedHub,uAttachedPort ) // call base class constructor
, m_pCHcd(pCHcd)
, m_usbHubDescriptor( rUsbHubDescriptor ) // USB descriptor of this hub
, m_ppCDeviceOnPort( NULL ) // dynamic array of pointers to the devices on this hub's ports
, m_fHubThreadClosing( FALSE )       // indicator to thread that it should close
, m_hHubStatusChangeEvent( NULL ) // event for hub status change thread
, m_hHubStatusChangeThread( NULL ) // checks for connect changes on the hub's ports
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub::CHub\n"),GetControllerName()) );
    m_pDetachedDevice=NULL;
    m_pDetachedDeviceHandled = CreateEvent(NULL,TRUE,FALSE,NULL); // Manual Reset Event;
    m_hHubSuspendBlockEvent = CreateEvent(NULL,TRUE,TRUE,NULL); // Manual Reset Event;
    DEBUGCHK( rDeviceInfo.Descriptor.bDeviceClass == USB_DEVICE_CLASS_HUB &&
              rUsbHubDescriptor.bDescriptorType == USB_HUB_DESCRIPTOR_TYPE &&
              rUsbHubDescriptor.bDescriptorLength >= USB_HUB_DESCRIPTOR_MINIMUM_SIZE &&
              rUsbHubDescriptor.bNumberOfPorts > 0 &&
              tierNumber <= USB_MAXIMUM_HUB_TIER );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub::CHub\n"),GetControllerName()) );
}

// ******************************************************************
CHub::~CHub( )
//
// Purpose: Destructor for CHub
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: Do not delete static variables here. Do that in
//        DeInitialize();
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub::~CHub\n"),GetControllerName()) );

    // this should have been taken care of in HandleDetach,
    // or if EnterOperationalState failed.
    DEBUGCHK( m_hHubStatusChangeEvent == NULL );
    DEBUGCHK( m_hHubStatusChangeThread == NULL );
    DEBUGCHK( m_pDetachedDevice == NULL);

#ifdef DEBUG
    if ( m_ppCDeviceOnPort != NULL ) {
        for ( UCHAR port = 1; port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
            // devices should have been freed by HandleDetach
            DEBUGCHK( m_ppCDeviceOnPort[ port - 1 ] == NULL );
        }
    }
#endif // DEBUG
    if (m_ppCDeviceOnPort){
        delete [] m_ppCDeviceOnPort;
        m_ppCDeviceOnPort = NULL;
    }
    if (m_pAddedTT){
        delete m_pAddedTT;
        m_pAddedTT = NULL;
    }
    if (m_pDetachedDeviceHandled)
        CloseHandle(m_pDetachedDeviceHandled );    
    if (m_hHubSuspendBlockEvent)
        CloseHandle(m_hHubSuspendBlockEvent);
    // nothing to do with m_usbHubDescriptor
    // nothing to do with m_fHubThreadClosing

    // rest of work done in ~CDevice

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub::~CHub\n"),GetControllerName()) );
}


// ******************************************************************
DWORD CALLBACK CHub::HubStatusChangeThreadStub( IN PVOID context )
//
// Purpose: Stub function for starting HubStatusChangeThread
//
// Parameters: context - pointer to descendant of CHub which contains
//                       the actual HubStatusChangeThread function
//
// Returns: Return value of HubStatusChangeThread
//
// Notes:
// ******************************************************************
{
    return ((CHub*)context)->HubStatusChangeThread();
}
// ******************************************************************
DWORD CHub::HubStatusChangeThread( void )
//
// Purpose: Main hub thread for handling changes to the hub's ports
//
// Parameters: None
//
// Returns: 0 on thread exit
//
// Notes: This routine needs to work for both root/external hubs
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::HubStatusChangeThread\n"),GetControllerName(), GetDeviceType(), m_tierNumber ) );
    DEBUGCHK( m_hHubStatusChangeEvent != NULL && m_hHubStatusChangeThread != NULL );

    UCHAR                       port;
    USB_HUB_AND_PORT_STATUS     hubStatus;
    BOOL                        fSuccess = FALSE;

    // before we can process port changes, we need
    // to power all ports
    while ( !m_fHubThreadClosing && !fSuccess) {
        fSuccess = PowerAllHubPorts();
    }
    if ( !m_fHubThreadClosing ) {
#if 0
        Sleep( 2 * m_usbHubDescriptor.bPowerOnToPowerGood );
#else
        // According to the USB spec 1.1, section 7.1.7.1, there
        // is supposed to be a delay of up to 100ms (t2) before the device
        // can signal attach. I don't know if the software is
        // supposed to implement this delay. No harm in implementing
        // it though.
        Sleep( 100 + 2 * m_usbHubDescriptor.bPowerOnToPowerGood );
#endif
    }
    SetOrClearRemoteWakup(TRUE);
    while ( !m_fHubThreadClosing ) {
        
        fSuccess = (WaitForSingleObject(m_hHubSuspendBlockEvent,INFINITE) == WAIT_OBJECT_0);
        if (m_fHubThreadClosing || !fSuccess)
            break;
        
        fSuccess = WaitForPortStatusChange( port, hubStatus );
        if ( m_fHubThreadClosing || !fSuccess ) {
                DEBUGMSG( ZONE_ERROR && !m_fHubThreadClosing, (TEXT("%s: CHub(%s tier %d)::HubStatusChangeThread - error reading port status change\n"),GetControllerName(), GetDeviceType(), m_tierNumber ));
            continue; // loop will exit if m_fHubThreadClosing is set
        }
        if ( port == 0 ) { // Port 0 indicate this is hub status.
            if (hubStatus.change.hub.OverCurrentIndicatorChange ) {
                if ( hubStatus.status.hub.OverCurrentIndicator ) {
                    RETAILMSG(1, (TEXT("CHub(tier %d)::HubStatusChangeThread - addr %d port %d over current!\n"),
                                  m_tierNumber, m_address, port));
                } else {
                    // hub is no longer over current - re-enumerate all ports
                    // Could re-enumerate all hub ports during hub over-current recovery...
                }
            }
            continue;
        }

        // we will get here if the status of port # "port" has changed.
        // the status information will be in "hubStatus"
        DEBUGCHK( port <= m_usbHubDescriptor.bNumberOfPorts );
        DEBUGMSG( ZONE_ATTACH, (TEXT("%s: CHub(%s tier %d)::HubStatusChangeThread - port %d, change = 0x%04x, status = 0x%04x\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port, hubStatus.change.word, hubStatus.status.word ) );

        if (hubStatus.change.port.OverCurrentChange ) {
            if ( hubStatus.status.port.PortOverCurrent ) {
                RETAILMSG(1, (TEXT("CHub(tier %d)::HubStatusChangeThread - addr %d port %d over current!\n"),
                              m_tierNumber, m_address, port));
                DetachDevice( port );
#if 1   // the "correct" thing to do, according to my reading of the USB spec
                SetOrClearFeature( port, USB_REQUEST_CLEAR_FEATURE, USB_HUB_FEATURE_PORT_POWER );
#else   // another approach
                do {
                    Sleep( 500 );
                    GetStatus( port, hubStatus );
                } while (hubStatus.status.port.PortOverCurrent && !m_fHubThreadClosing);
                hubStatus.change.port.ConnectStatusChange = 1;
#endif
            } else {
                // port is no longer over current - pretend this is a normal attach
                // simulate a connect status change. this has the undesirable but basically harmless
                // side effect of wasting 100 ms to needlessly debounce the power rail.
                hubStatus.change.port.ConnectStatusChange = 1;
            }
        }
        // Resume Notification.
        EnterCriticalSection( &m_csDeviceLock );
        if ( hubStatus.change.port.SuspendChange && !hubStatus.status.port.PortSuspended  && 
                m_ppCDeviceOnPort[port-1]!=NULL ) {
            m_ppCDeviceOnPort[port-1]->ResumeNotification();
        }
        LeaveCriticalSection( &m_csDeviceLock );
        if ( hubStatus.change.port.PortEnableChange &&
             !hubStatus.status.port.PortEnabled &&
             hubStatus.status.port.PortConnected ) {
            // Connected device has become disabled. If the device was
            // already successfully attached, let's try detach/reattach.
            // It is important to check that the device was successfully
            // attached - otherwise, we can get into an infinite loop
            // of try attach, fail, disable port, retry attach.
            BOOL fDeviceIsPresent;

            EnterCriticalSection( &m_csDeviceLock );
            DEBUGCHK( m_ppCDeviceOnPort != NULL );
            fDeviceIsPresent = ( m_ppCDeviceOnPort[ port - 1 ] != NULL );
            LeaveCriticalSection( &m_csDeviceLock );

            if ( fDeviceIsPresent ) {
                DEBUGMSG( ZONE_WARNING, (TEXT("%s: CHub(%s tier %d)::HubStatusChangeThread - device on port %d is connected but has been disabled. Trying to detach & re-attach\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port) );
                DetachDevice( port );
                // this will cause device attach below, since
                // hubStatus.status.port.PortConnected is already set
                hubStatus.change.port.ConnectStatusChange = 1;
                DEBUGCHK( hubStatus.status.port.PortConnected );
            }
        } // we can ignore all other enabled changes

        // now check for connect changes
        if ( hubStatus.change.port.ConnectStatusChange ) {
            EnterCriticalSection( &m_csDeviceLock );
            BOOL fDeviceAlreadyExists = (m_ppCDeviceOnPort[ port - 1 ] != NULL);
            LeaveCriticalSection( &m_csDeviceLock );

            // we got a connect status change notification on this port, so...
            if (fDeviceAlreadyExists) {
                // ... a change when the device is already here must be a detach;
                //     if there's still something connected then it must be new.
                DEBUGMSG( ZONE_ATTACH, (TEXT("%s: CHub(%s tier %d)::HubStatusChangeThread - device detached on port %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port ) );
                DetachDevice(port);
#ifdef DEBUG
                if ( hubStatus.status.port.PortConnected ) {
                    DEBUGMSG( (ZONE_WARNING && ZONE_VERBOSE) || ZONE_ATTACH,
                              (TEXT("CHub(%s tier %d)::HubStatusChangeThread -")
                               TEXT(" quick detach and re-attach on port %d\n"),
                               GetDeviceType(), m_tierNumber, port) );
                }
#endif // DEBUG
            }
            // ... a change with no device present must be an attach
            //     but section 7.1.7.1 of the USB 1.1 spec says we're
            //     responsible for de-bouncing the attach signalling.
            //
            // we do the de-bouncing by waiting until a 100 ms interval
            // (t3 on figure 7-19 in the spec) elapses with no connection
            // status change on the port. Then we can examine the current
            // connect status reliably.
            BOOL fPoll = TRUE;

            while (fPoll) {
                USB_HUB_AND_PORT_STATUS hubStatus2;
                Sleep( 100 );
                if ( GetStatus( port, hubStatus2 ) == FALSE ) {
                    // failed to get status; probably power-cycle or upper-level detach
                    hubStatus.status.port.PortConnected = FALSE;
                    fPoll = FALSE;
                }
                else if ( hubStatus2.change.port.ConnectStatusChange ) {
                    // ack the status change and wait again
                    SetOrClearFeature( port, USB_REQUEST_CLEAR_FEATURE,
                                       USB_HUB_FEATURE_C_PORT_CONNECTION );
                }
                else {
                    // ah, stability.
                    hubStatus.status.word = hubStatus2.status.word;
                    fPoll = FALSE;
                }
            }
            // We have to reset port and get speed infomation.
            DEBUGMSG(ZONE_ATTACH, (TEXT("CHub::ResetAndEnablePort\r\n")));
            if(hubStatus.status.port.PortConnected)
            {
                ResetAndEnablePort( port );
                Sleep(20);
                GetStatus(port , hubStatus);
                if ( hubStatus.status.port.PortConnected ) {
                    // device attach
                    RETAILMSG( 1, (TEXT("CHub::HubStatusChangeThread - device attached on port %d\n"), port ) );
                    AttachDevice( port,
                                  hubStatus.status.port.DeviceIsLowSpeed,
                                  m_fIsHighSpeed?hubStatus.status.port.DeviceIsHighSpeed:FALSE );
                }
            }
        } // end of ConnectStatusChange processing
    }
    DEBUGMSG( ZONE_HUB, (TEXT("%s: -CHub(%s tier %d)::HubStatusChangeThread, THREAD EXITING, returning 0\n"),GetControllerName(), GetDeviceType(), m_tierNumber ) );
    return 0;
}
// ******************************************************************
HCD_REQUEST_STATUS CDevice::DisableDevice( IN const UINT address,
                                     IN const BOOL fReset )
//
// Purpose: Disable Downstream Device with address "address"
//
// Parameters: See description in CHcd::DisableDevice.
//
// Returns: requestOK - if Device Reset
//
//          requestFailed - if device exists, but unable to disable it.
//
//          requestIgnored - if no device found with matching address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: CHub(%s tier %d)::DisableDevice - address = %d, pipeIndex = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, fReset) );
    HCD_REQUEST_STATUS status = requestIgnored;
    if (address == m_address ) { // If it is this device
        m_pAttachedHub->DisableOffStreamDevice( address, fReset);
        status = requestOK;
    }
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CDevice::SuspendResume( IN const UINT address,
                                     IN const BOOL fSuspend )
//
// Purpose: Suspend or Resume on device with address "address"
//
// Parameters: See description in CHcd::SuspendResume
//
// Returns: requestOK - if device suspend or resumed
//
//          requestFailed - if device exists, but unable to reset it
//
//          requestIgnored - if no device found with matching address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::SuspendResume - address = %d, fSuspend = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, fSuspend) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );
    if ( address == m_address ) {
        if (m_pAttachedHub->SuspendResumeOffStreamDevice(address, fSuspend)) {
            m_fIsSuspend=fSuspend;
            status = requestOK;
        }
        else
            status = requestFailed;
    }
    LeaveCriticalSection( &m_csDeviceLock );
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::SuspendResume - address = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, status) );
    return status;
}

#ifdef DEBUG
// ******************************************************************
void CHub::DumpHubDescriptor( IN const PUSB_HUB_DESCRIPTOR pDescriptor )
//
// Purpose: print out the contents of the descriptor via DEBUGMSG
//
// Parameters: pDescriptor - pointer to descriptor
//
// Returns: Nothing.
//
// Notes: Used in debug mode only
//
//        Refer to USB spec 1.1, Section 11.15.2.1
// ******************************************************************
{
    PREFAST_DEBUGCHK( pDescriptor != NULL );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("+Dump USB_HUB_DESCRIPTOR\n")) );
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDescriptorLength = 0x%02x\n"), pDescriptor->bDescriptorLength ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbDescriptorType = 0x%02x\n"), pDescriptor->bDescriptorType ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbNumberOfPorts = 0x%02x\n"), pDescriptor->bNumberOfPorts ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\twHubCharacteristics = 0x%04x\n"), pDescriptor->wHubCharacteristics ));
    if ( pDescriptor->wHubCharacteristics & USB_HUB_CHARACTERISTIC_NO_POWER_SWITCHING ) {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\twHubCharacteristics, No Port Power Switching\n")));
    } else if ( pDescriptor->wHubCharacteristics & USB_HUB_CHARACTERISTIC_INDIVIDUAL_POWER_SWITCHING ) {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\twHubCharacteristics, Individual Port Power Switching\n")));
    } else {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\twHubCharacteristics, Ganged Port Power Switching\n")));
    }
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\twHubCharacteristics, Hub %s part of compound device\n"), ((pDescriptor->wHubCharacteristics & USB_HUB_CHARACTERISTIC_PART_OF_COMPOUND_DEVICE) ? TEXT("IS") : TEXT("IS NOT")) ));
    if ( pDescriptor->wHubCharacteristics & USB_HUB_CHARACTERISTIC_NO_OVER_CURRENT_PROTECTION ) {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\twHubCharacteristics, No Over Current Protection\n")));
    } else if ( pDescriptor->wHubCharacteristics & USB_HUB_CHARACTERISTIC_INDIVIDUAL_OVER_CURRENT_PROTECTION ) {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\twHubCharacteristics, Individual Over Current Protection\n")));
    } else {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\t\twHubCharacteristics, Global Over Current Protection\n")));
    }
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbPowerOnToPowerGood = 0x%02x\n"), pDescriptor->bPowerOnToPowerGood ));
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tbHubControlCurrent = 0x%02x\n"), pDescriptor->bHubControlCurrent ));
    const UCHAR numBytes = 1 + (pDescriptor->bNumberOfPorts >> 3);
    for ( UCHAR offset = 0; offset < numBytes; offset++  ) {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tDeviceRemovable bitmask byte #%d = 0x%02x\n"), offset + 1, pDescriptor->bRemoveAndPowerMask[ offset ] ) );
    }
    for ( offset = numBytes; offset < 2 * numBytes; offset++ ) {
        DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("\tPortPwrCtrlMask bitmask byte #%d = 0x%02x\n"), offset - numBytes + 1, pDescriptor->bRemoveAndPowerMask[ offset ] ) );
    }
    DEBUGMSG( ZONE_DESCRIPTORS, (TEXT("-Dump USB_HUB_DESCRIPTOR\n")) );
}
#endif // DEBUG

// ******************************************************************
void CHub::AttachDevice( IN const UCHAR port,
                         IN const BOOL fIsLowSpeed ,
                         IN const BOOL fIsHighSpeed )
//
// Purpose: This function is called when a new device is attached
//          on port "port". After this procedure finishes, a configured
//          device will be added to the hub's port array
//
// Parameters: port - indicates hub port on which device was attached
//
//             fIsLowSpeed - indicates if new device is low speed
//
// Returns: Nothing
//
// Notes: If this function fails, the port will be disabled
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH, (TEXT("%s: +CHub(%s tier %d)::AttachDevice - port = %d, fIsLowSpeed = %d, fIsHighSpeed =%d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port, fIsLowSpeed, fIsHighSpeed ) );

    DEBUGCHK( port > 0 && port <= m_usbHubDescriptor.bNumberOfPorts );

    // device related variables
    CDevice*                pNewDevice = NULL;
    UCHAR                   address = 0xff; // illegal address
	USB_DEVICE_INFO         deviceInfo = {0};
    USB_HUB_DESCRIPTOR      usbHubDescriptor = {0};
    CPipeAbs*               pControlPipe = NULL; // pipe to device's endpoint 0

    // setup process related varibles
    BOOL                    fUsingAddr0 = FALSE;
    DEVICE_CONFIG_STATUS    configStatus = DEVICE_CONFIG_STATUS_OPENING_ENDPOINT0_PIPE;
    UCHAR                   configFailures = 0;
    UINT                    currentConfigDescriptorIndex = 0;
    BOOL                    fPipeHalted = FALSE;

    deviceInfo.dwCount = sizeof( USB_DEVICE_INFO );
    deviceInfo.lpActiveConfig = NULL;
    deviceInfo.lpConfigs = NULL;

    if ( !ReserveAddress( address ) ) {
        DEBUGMSG( ZONE_WARNING, (TEXT("%s: CHub(%s tier %d)::AttachDevice - all USB addresses are in use!\n"),GetControllerName(), GetDeviceType(), m_tierNumber) );
        // don't do any attach processing
        configStatus = DEVICE_CONFIG_STATUS_DONE;
    }
    while ( configStatus != DEVICE_CONFIG_STATUS_DONE ) {
        if ( m_fHubThreadClosing || fPipeHalted || configFailures > 2 ) {
            configStatus = DEVICE_CONFIG_STATUS_FAILED;
        }
               
        DEBUGMSG( ZONE_ATTACH, (TEXT("CHub(tier %d)::AttachDevice - status = %s, failures = %d\n"),  m_tierNumber, STATUS_TO_STRING( configStatus ), configFailures ) );        
        switch ( configStatus ) {
        case DEVICE_CONFIG_STATUS_OPENING_ENDPOINT0_PIPE:
        {
            // Create TT if it is neccessary.
            if (m_fIsHighSpeed && !fIsHighSpeed &&  !m_pAddedTT[port-1]) { // Hi-Speed Hub and Low Speed device.
                m_pAddedTT[port-1] = m_pDeviceGlobal->AddedTt(m_address,port);
            }
            DEBUGCHK( pControlPipe == NULL );
            USB_ENDPOINT_DESCRIPTOR usbEndpointZeroDescriptor;
            usbEndpointZeroDescriptor.bDescriptorType = USB_ENDPOINT_DESCRIPTOR_TYPE;
            usbEndpointZeroDescriptor.bEndpointAddress = 0;
            // usbEndpointZeroDescriptor.bInterval = ; <- ignored for control pipes
            usbEndpointZeroDescriptor.bLength = sizeof( USB_ENDPOINT_DESCRIPTOR );
            usbEndpointZeroDescriptor.bmAttributes = USB_ENDPOINT_TYPE_CONTROL;
            usbEndpointZeroDescriptor.wMaxPacketSize = ENDPOINT_ZERO_MIN_MAXPACKET_SIZE;
            UCHAR uTTHubAddr= m_address;
            UCHAR uTTHubPort= port;
            BOOL ttContext = NULL;
            GetUSB2TT(uTTHubAddr,uTTHubPort,ttContext);
            pControlPipe = CreateControlPipe( &usbEndpointZeroDescriptor,
                                             fIsLowSpeed, fIsHighSpeed ,0,
                                             uTTHubAddr,uTTHubPort,ttContext,
                                             m_pCHcd);
            if ( pControlPipe && pControlPipe->SetReservedDeviceAddr(address) && pControlPipe->OpenPipe() == requestOK ) {
                    // success
                    configStatus = DEVICE_CONFIG_STATUS_USING_ADDRESS0;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, unable to open control pipe\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                configFailures++;
                delete pControlPipe;
                pControlPipe = NULL;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_USING_ADDRESS0:
        {
            DEBUGCHK( !fUsingAddr0 );
            fUsingAddr0 = TRUE;
            //if (EnterCritSec_Ex( &m_csAddress0Lock, INFINITE ) != CSS_SUCCESS) {
            if (m_pDeviceGlobal->Addr0LockEntry(INFINITE )!= CSS_SUCCESS) {
                DEBUGMSG( ZONE_ATTACH, (TEXT("CHub(tier %d)::AttachDevice - shutdown signalled during enumeration\n"),  m_tierNumber) );
                fUsingAddr0 = FALSE;
                configStatus = DEVICE_CONFIG_STATUS_FAILED;
            } else
                // successfully acquired the Address0Lock
                configStatus = DEVICE_CONFIG_STATUS_RESET_AND_ENABLE_PORT;
            break;
        }
        case DEVICE_CONFIG_STATUS_RESET_AND_ENABLE_PORT:
        {
            DEBUGCHK( fUsingAddr0 );
            if ( ResetAndEnablePort( port ) ) {
                configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR_TEST;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, unable to reset/enable port\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR_TEST: 
        {
            if ( GetDescriptor( pControlPipe,
                                0,
                                USB_DEVICE_DESCRIPTOR_TYPE,
                                0, // descriptor index
                                ENDPOINT_ZERO_MIN_MAXPACKET_SIZE,
                                &deviceInfo.Descriptor ) ) {
                // success
                configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_SET_ADDRESS;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                pControlPipe->IsPipeHalted( &fPipeHalted );

                configFailures++;
                configStatus = DEVICE_CONFIG_STATUS_RESET_AND_ENABLE_PORT;                
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_SET_ADDRESS:
        {
            DEBUGCHK( fUsingAddr0 &&
                      pControlPipe != NULL &&
                      address > 0 );
            BOOL                fTransferDone = FALSE;
            DWORD               dwBytesTransferred = 0;
            DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
            HCD_REQUEST_STATUS  status = requestFailed;
            USB_DEVICE_REQUEST  usbRequest;

            usbRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE;
            usbRequest.bRequest = USB_REQUEST_SET_ADDRESS;
            usbRequest.wValue = address; // new address
            usbRequest.wIndex = 0;
            usbRequest.wLength = 0;
            // Note - at this point, the device is listening on address 0,
            // so we pass 0, not "address", to the IssueTransfer function
            if ( !m_fHubThreadClosing ) {
                status = pControlPipe->IssueTransfer(
                                        0, // device address (addr0)
                                        TransferDoneCallbackSetEvent, // callback func
                                        m_hHubStatusChangeEvent, // callback param
                                        USB_OUT_TRANSFER | USB_SEND_TO_DEVICE, // transfer params
                                        &usbRequest, // control request
                                        0, // dwStartingFrame (not used)
                                        0, // dwFrames (not used)
                                        NULL, // aLengths (not used)
                                        0, // buffer size
                                        NULL, // buffer
                                        0, // phys addr of buffer (not used)
                                        this, // cancel id
                                        NULL, // adwIsochErrors (not used)
                                        NULL, // adwIsochLengths (not used)
                                        &fTransferDone, // OUT status param
                                        &dwBytesTransferred, // OUT status param
                                        &dwErrorFlags ); // OUT status param
                if ( status == requestOK ) {
                    WaitForSingleObject( m_hHubStatusChangeEvent, INFINITE );
                    if ( m_fHubThreadClosing ) {
                        pControlPipe->AbortTransfer( NULL, // callback function
                                                     NULL, // callback parameter
                                                     this ); // cancel ID
                        ResetEvent(m_hHubStatusChangeEvent);
                    }
                }
                DEBUGCHK( fTransferDone );
            }
            if ( status == requestOK &&
                 fTransferDone &&
                 dwBytesTransferred == 0 &&
                 dwErrorFlags == USB_NO_ERROR ) {
                configStatus = DEVICE_CONFIG_STATUS_LEAVE_ADDRESS0;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, fTransferDone = %d, dwBytesTrans = 0x%x, dwErrorFlags = 0x%x\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ), fTransferDone, dwBytesTransferred, dwErrorFlags ) );
                configFailures++;
                pControlPipe->IsPipeHalted( &fPipeHalted );
                configStatus = DEVICE_CONFIG_STATUS_RESET_AND_ENABLE_PORT;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_LEAVE_ADDRESS0:
        {
            DEBUGCHK( fUsingAddr0 );
            // it can take some time for SetAddress to
            // take effect. In the OHCD code, there is a comment
            // that some devices (i.e. Andromeda hub) need a
            // delay here
            Sleep( 10 );
            //LeaveCritSec_Ex( &m_csAddress0Lock );
            m_pDeviceGlobal->Addr0LockLeave();
            fUsingAddr0 = FALSE;
            configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_DEVICE_DESCRIPTOR;
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_DEVICE_DESCRIPTOR:
        {
            if ( GetDescriptor( pControlPipe,
                                address,
                                USB_DEVICE_DESCRIPTOR_TYPE,
                                0, // descriptor index
                                ENDPOINT_ZERO_MIN_MAXPACKET_SIZE,
                                &deviceInfo.Descriptor ) ) {

                DEBUGCHK( m_tierNumber <= USB_MAXIMUM_HUB_TIER );
                if ( m_tierNumber == USB_MAXIMUM_HUB_TIER &&
                     deviceInfo.Descriptor.bDeviceClass == USB_DEVICE_CLASS_HUB ) {
                    RETAILMSG( 1, (TEXT("USB specification does not allow more than %d hubs in a row\n"), USB_MAXIMUM_HUB_TIER ));
                    configStatus = DEVICE_CONFIG_STATUS_FAILED;
                } else {
                    // success
                    if ( deviceInfo.Descriptor.bMaxPacketSize0 > ENDPOINT_ZERO_MIN_MAXPACKET_SIZE ) {
                        pControlPipe->ChangeMaxPacketSize( deviceInfo.Descriptor.bMaxPacketSize0 );
                    }
                    configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR;
                }
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                pControlPipe->IsPipeHalted( &fPipeHalted );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR:
        {
            if ( GetDescriptor( pControlPipe,
                                address,
                                USB_DEVICE_DESCRIPTOR_TYPE,
                                0, // descriptor index
                                sizeof( deviceInfo.Descriptor ),
                                &deviceInfo.Descriptor ) ) {
                // success
                configStatus = DEVICE_CONFIG_STATUS_SETUP_CONFIGURATION_DESCRIPTOR_ARRAY;
            #ifdef DEBUG
                DumpDeviceDescriptor( &deviceInfo.Descriptor );
            #endif // DEBUG
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                pControlPipe->IsPipeHalted( &fPipeHalted );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SETUP_CONFIGURATION_DESCRIPTOR_ARRAY:
        {
            DEBUGCHK( deviceInfo.lpActiveConfig == NULL &&
                      deviceInfo.lpConfigs == NULL );

            const UINT numConfigurations = deviceInfo.Descriptor.bNumConfigurations;
            deviceInfo.lpConfigs = new NON_CONST_USB_CONFIGURATION[ numConfigurations ];
            if ( deviceInfo.lpConfigs != NULL ) {
                memset( deviceInfo.lpConfigs, 0, numConfigurations * sizeof( NON_CONST_USB_CONFIGURATION ) );
                for ( UINT config = 0; config < numConfigurations ; config++ ) {
                    deviceInfo.lpConfigs[ config ].dwCount = sizeof( NON_CONST_USB_CONFIGURATION );
                }
                currentConfigDescriptorIndex = 0;
                configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_CONFIG_DESCRIPTOR;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, no memory\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_CONFIG_DESCRIPTOR:
        {
            DEBUGCHK( currentConfigDescriptorIndex < deviceInfo.Descriptor.bNumConfigurations );
            DEBUGCHK( deviceInfo.lpActiveConfig == NULL &&
                      deviceInfo.lpConfigs != NULL );

            if ( GetDescriptor( pControlPipe,
                                address,
                                USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                (UCHAR)currentConfigDescriptorIndex,
                                sizeof( deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].Descriptor ),
                                &deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].Descriptor ) ) {
                // success
                configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_CONFIG_DESCRIPTOR;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                pControlPipe->IsPipeHalted( &fPipeHalted );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_GET_CONFIG_DESCRIPTOR:
        {
            DEBUGCHK( currentConfigDescriptorIndex < deviceInfo.Descriptor.bNumConfigurations );
            DEBUGCHK( deviceInfo.lpActiveConfig == NULL &&
                      deviceInfo.lpConfigs != NULL &&
                      deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].Descriptor.bDescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE &&
                      deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].Descriptor.bLength == sizeof( USB_CONFIGURATION_DESCRIPTOR ) &&
                      deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].lpbExtended == NULL &&
                      deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].lpInterfaces == NULL );

            const USHORT wTotalLength = deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].Descriptor.wTotalLength;
            PUCHAR pDataBuffer = new UCHAR[ wTotalLength ];            
            if ( pDataBuffer != NULL &&
                 GetDescriptor( pControlPipe,
                                address,
                                USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                (UCHAR)currentConfigDescriptorIndex,
                                wTotalLength,
                                pDataBuffer ) &&
                 memcmp( &deviceInfo.lpConfigs[ currentConfigDescriptorIndex ].Descriptor, pDataBuffer, sizeof( USB_CONFIGURATION_DESCRIPTOR ) ) == 0 &&
                 CreateUsbConfigurationStructure( deviceInfo.lpConfigs[ currentConfigDescriptorIndex ], pDataBuffer, wTotalLength ) ) {
                // success
                currentConfigDescriptorIndex++;
                if ( currentConfigDescriptorIndex < deviceInfo.Descriptor.bNumConfigurations ) {
                    // need to get more descriptors
                    configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_CONFIG_DESCRIPTOR;
                } else {
                    // done getting config descriptors
                    configStatus = DEVICE_CONFIG_STATUS_DETERMINE_CONFIG_TO_CHOOSE;
                }
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                pControlPipe->IsPipeHalted( &fPipeHalted );
                configFailures++;
            }
            delete [] pDataBuffer;
            pDataBuffer = NULL;
            break;
        }
        case DEVICE_CONFIG_STATUS_DETERMINE_CONFIG_TO_CHOOSE:
        {
            // We're not terribly smart about picking a config when the first one won't work. C'est la vie.
            // Also, we cannot check the device's actual power status until after it's configured.
            
            BYTE bConfig = 0;
            if ( m_pCHcd->GetpUSBDSelectConfigurationProc()!=NULL  // This function is supported by USBD.
                    && deviceInfo.Descriptor.bNumConfigurations > 1 ) {
                if (!(*m_pCHcd->GetpUSBDSelectConfigurationProc())(LPCUSB_DEVICE(&deviceInfo),&bConfig) || 
                        bConfig >= deviceInfo.Descriptor.bNumConfigurations ) { // if can not find anything return to 0.
                    bConfig = 0;
                }
            }
            DEBUGMSG( ZONE_ATTACH, (TEXT("%s: CHub(%s tier %d)::AttachDevice - Select Configuration %d on %s step\n"),GetControllerName(), GetDeviceType(), m_tierNumber,bConfig,STATUS_TO_STRING( configStatus ) ) );
            if (deviceInfo.lpConfigs[bConfig].Descriptor.bmAttributes & SELF_POWERED) {
                // the device we're attaching is self-powered so power is of little concern
            } else {
                DWORD dwCfgPower = deviceInfo.lpConfigs[bConfig].Descriptor.MaxPower * 2; // MaxPower is in units of 2mA
                if (deviceInfo.lpConfigs[bConfig].Descriptor.MaxPower == 0) {
                    RETAILMSG(1, (TEXT("!CHub::AttachDevice warning: invalid power configuration\n")));
                    dwCfgPower = 500; // If MaxPower is illigal, assume the maximum power is required.
                }
                {
                    BOOL isOK;
                    if (m_deviceInfo.lpConfigs == NULL) {
                        // we must be a root hub; check with the PDD
                        isOK = HcdPdd_CheckConfigPower(port, dwCfgPower, 0);
                    } else if (m_deviceInfo.lpActiveConfig->Descriptor.bmAttributes & SELF_POWERED) {
                        // we're self-powered so we can attach a high-powered device
                        isOK = TRUE;
                    } else if (dwCfgPower <= 100) {
                        // we're bus-powered so we can only attach low-powered devices
                        isOK = TRUE;
                    } else {
                        isOK = FALSE;
                    }
                    if ( ! isOK ) {
                        RETAILMSG(1, (TEXT("!USB warning: cannot attach high-power device to a bus-powered hub\n")));
                        configStatus = DEVICE_CONFIG_STATUS_FAILED;
                    }
                }
            }
            if (configStatus != DEVICE_CONFIG_STATUS_FAILED) {
                DEBUGCHK( deviceInfo.lpActiveConfig == NULL && deviceInfo.lpConfigs != NULL );

                deviceInfo.lpActiveConfig = &deviceInfo.lpConfigs[bConfig];
                DEBUGCHK( deviceInfo.lpActiveConfig->lpInterfaces != NULL );

                configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_SET_CONFIG;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_SET_CONFIG:
        {
            DEBUGCHK( deviceInfo.lpActiveConfig != NULL );

            BOOL                fTransferDone = FALSE;
            DWORD               dwBytesTransferred = 0;
            DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
            HCD_REQUEST_STATUS  status = requestFailed;
            USB_DEVICE_REQUEST  usbRequest;

            usbRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE;
            usbRequest.bRequest = USB_REQUEST_SET_CONFIGURATION;
            usbRequest.wValue = deviceInfo.lpActiveConfig->Descriptor.bConfigurationValue;
            usbRequest.wIndex = 0;
            usbRequest.wLength = 0;
            if ( !m_fHubThreadClosing ) {
                status = pControlPipe->IssueTransfer(
                                            address, // device rAddress
                                            TransferDoneCallbackSetEvent, // callback
                                            m_hHubStatusChangeEvent, // param for callback
                                            USB_OUT_TRANSFER | USB_SEND_TO_DEVICE, // transfer flags
                                            &usbRequest, // control request
                                            0, // dwStartingFrame (not used)
                                            0, // dwFrames (not used)
                                            NULL, // aLengths (not used)
                                            0, // buffer size
                                            NULL, // data buffer
                                            0, // phys addr of buffer (not used)
                                            this, // cancel ID
                                            NULL, // adwIsochErrors (not used)
                                            NULL, // adwIsochLengths (not used)
                                            &fTransferDone, // OUT status param
                                            &dwBytesTransferred, // OUT status param
                                            &dwErrorFlags ); // OUT status param
                if ( status == requestOK ) {
                    DWORD dwResult = WaitForSingleObject( m_hHubStatusChangeEvent, STANDARD_REQUEST_TIMEOUT );
                    if ( m_fHubThreadClosing || dwResult!= WAIT_OBJECT_0 ) {
                        pControlPipe->AbortTransfer( NULL, // callback function
                                                     NULL, // callback parameter
                                                     this ); // cancel ID
                        ResetEvent(m_hHubStatusChangeEvent);                        
                    }
                }
                DEBUGCHK( fTransferDone );
            }
            if ( status == requestOK &&
                 fTransferDone &&
                 dwBytesTransferred == 0 &&
                 dwErrorFlags == USB_NO_ERROR ) {
                // move to next step
                if ( deviceInfo.Descriptor.bDeviceClass == USB_DEVICE_CLASS_HUB ) {
                    // more steps need to happen for hubs
                    usbHubDescriptor.bDescriptorLength = USB_HUB_DESCRIPTOR_MINIMUM_SIZE;
                    configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_HUB_DESCRIPTOR;
                } else {
                    configStatus = DEVICE_CONFIG_STATUS_CREATE_NEW_FUNCTION;
                }
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, fTransferDone = %d, dwBytesTrans = 0x%x, dwErrorFlags = 0x%x\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ), fTransferDone, dwBytesTransferred, dwErrorFlags ) );
                pControlPipe->IsPipeHalted( &fPipeHalted );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_HUB_DESCRIPTOR:
        case DEVICE_CONFIG_STATUS_SCHEDULING_GET_HUB_DESCRIPTOR:
        {
            DEBUGCHK( deviceInfo.Descriptor.bDeviceClass == USB_DEVICE_CLASS_HUB );
            DEBUGCHK( (configStatus == DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_HUB_DESCRIPTOR &&
                       usbHubDescriptor.bDescriptorLength == USB_HUB_DESCRIPTOR_MINIMUM_SIZE) ||
                      (configStatus == DEVICE_CONFIG_STATUS_SCHEDULING_GET_HUB_DESCRIPTOR &&
                       usbHubDescriptor.bDescriptorLength > USB_HUB_DESCRIPTOR_MINIMUM_SIZE &&
                       usbHubDescriptor.bDescriptorLength <= sizeof( usbHubDescriptor ) ) );
            const UCHAR descriptorLengthToGet = usbHubDescriptor.bDescriptorLength;
            if ( GetDescriptor( pControlPipe,
                                address,
                                USB_HUB_DESCRIPTOR_TYPE,
                                0, // hub descriptor index is 0
                                descriptorLengthToGet,
                                &usbHubDescriptor ) ) {
                // success
                if ( usbHubDescriptor.bDescriptorLength > descriptorLengthToGet ) {
                    DEBUGCHK( configStatus == DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_HUB_DESCRIPTOR );
                    configStatus = DEVICE_CONFIG_STATUS_SCHEDULING_GET_HUB_DESCRIPTOR;
                } else {
                    DEBUGCHK( usbHubDescriptor.bDescriptorLength == descriptorLengthToGet );
                #ifdef DEBUG
                    DumpHubDescriptor( &usbHubDescriptor );
                #endif // DEBUG
                    configStatus = DEVICE_CONFIG_STATUS_CREATE_NEW_HUB;
                }
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                pControlPipe->IsPipeHalted( &fPipeHalted );
                configFailures++;
                // Restore bDescriptorLength for the retry.
                usbHubDescriptor.bDescriptorLength = descriptorLengthToGet;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_CREATE_NEW_HUB:
        {
            DEBUGCHK( pNewDevice == NULL );
            DEBUGCHK( deviceInfo.Descriptor.bDeviceClass == USB_DEVICE_CLASS_HUB &&
                      usbHubDescriptor.bDescriptorType == USB_HUB_DESCRIPTOR_TYPE &&
                      usbHubDescriptor.bDescriptorLength >= USB_HUB_DESCRIPTOR_MINIMUM_SIZE &&
                      deviceInfo.lpActiveConfig->lpInterfaces[0].Descriptor.bNumEndpoints == 1);

            DEBUGCHK( m_tierNumber < USB_MAXIMUM_HUB_TIER );
            pNewDevice = new CExternalHub( address,
                                           deviceInfo,
                                           fIsLowSpeed,fIsHighSpeed,
                                           m_tierNumber + 1,
                                           usbHubDescriptor,
                                           m_pCHcd,
                                           this,port);
            if ( pNewDevice != NULL ) {
                configStatus = DEVICE_CONFIG_STATUS_INSERT_NEW_DEVICE_INTO_UPSTREAM_HUB_PORT_ARRAY;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, no memory\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_CREATE_NEW_FUNCTION:
        {
            DEBUGCHK( pNewDevice == NULL );
            DEBUGCHK( deviceInfo.Descriptor.bDeviceClass != USB_DEVICE_CLASS_HUB );
            DEBUGCHK( m_tierNumber <= USB_MAXIMUM_HUB_TIER );
            pNewDevice = new CFunction( address,
                                        deviceInfo,
                                        fIsLowSpeed,fIsHighSpeed,
                                        m_tierNumber + 1,
                                        m_pCHcd,
                                        this,port);
            if ( pNewDevice != NULL ) {
                configStatus = DEVICE_CONFIG_STATUS_INSERT_NEW_DEVICE_INTO_UPSTREAM_HUB_PORT_ARRAY;
            } else {
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, no memory\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                configFailures++;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_INSERT_NEW_DEVICE_INTO_UPSTREAM_HUB_PORT_ARRAY:
        {
            DEBUGCHK( pNewDevice != NULL );
            EnterCriticalSection( &m_csDeviceLock );
            DEBUGCHK( m_ppCDeviceOnPort != NULL &&
                      m_ppCDeviceOnPort[ port - 1 ] == NULL );
            m_ppCDeviceOnPort[ port - 1 ] = pNewDevice;
            LeaveCriticalSection( &m_csDeviceLock );
            configStatus = DEVICE_CONFIG_STATUS_SIGNAL_NEW_DEVICE_ENTER_OPERATIONAL_STATE;
            break;
        }
        case DEVICE_CONFIG_STATUS_SIGNAL_NEW_DEVICE_ENTER_OPERATIONAL_STATE:
        {
            DEBUGCHK( pNewDevice != NULL );
            if ( pNewDevice->EnterOperationalState( pControlPipe ) ) {
                configStatus = DEVICE_CONFIG_STATUS_DONE;
            } else {
                // don't do any retries here! EnterOperationalState
                // should only be tried once
                DEBUGMSG( ZONE_ATTACH && ZONE_ERROR, (TEXT("%s: CHub(%s tier %d)::AttachDevice - failure on %s step, aborting attach process\n"),GetControllerName(), GetDeviceType(), m_tierNumber, STATUS_TO_STRING( configStatus ) ) );
                configStatus = DEVICE_CONFIG_STATUS_FAILED;
                configFailures = 0xff;
            }
            break;
        }
        case DEVICE_CONFIG_STATUS_FAILED:
        {
            if ( pNewDevice != NULL ) {
                // this means we have placed the device into our array
                EnterCriticalSection( &m_csDeviceLock );
                DEBUGCHK( (m_fHubThreadClosing && m_ppCDeviceOnPort[ port - 1 ] == NULL) ||
                          m_ppCDeviceOnPort[ port - 1 ] == pNewDevice );
                m_ppCDeviceOnPort[ port - 1 ] = NULL;
                LeaveCriticalSection( &m_csDeviceLock );
                // address will be freed by destructor
                // deviceInfo will be freed by destructor
                delete pNewDevice;
                pNewDevice = NULL;
            } else {
                if ( deviceInfo.lpConfigs != NULL ) {
                    for ( UINT config = 0; config < deviceInfo.Descriptor.bNumConfigurations; config++ ) {
                        DeleteUsbConfigurationStructure( deviceInfo.lpConfigs[ config ] );
                    }
                    delete [] deviceInfo.lpConfigs;
                }
                DEBUGCHK( address > 0 && address <= USB_MAX_ADDRESS );
                FreeAddress( address );
            }
            if ( pControlPipe ) {
                pControlPipe->ClosePipe();
                delete pControlPipe;
                pControlPipe = NULL;
            }
            fPipeHalted = TRUE;
            DisablePort( port );
            if ( fUsingAddr0 ) {
                //LeaveCritSec_Ex( &m_csAddress0Lock );
                m_pDeviceGlobal->Addr0LockLeave();
                fUsingAddr0 = FALSE;
            }
            configStatus = DEVICE_CONFIG_STATUS_DONE;
            break;
        }
    #ifdef DEBUG
        default:
        {
            DebugBreak(); // should never get here!
            break;
        }
    #endif // DEBUG
        } // end of switch ( configStatus )
    } // end of while ( configStatus != DEVICE_CONFIG_STATUS_DONE )

    DEBUGMSG( ZONE_ATTACH, (TEXT("%s: -CHub(%s tier %d)::AttachDevice - port = %d, fIsLowSpeed = %d, address = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port, fIsLowSpeed, address ) );
}

// ******************************************************************
BOOL CHub::GetDescriptor( IN CPipeAbs * const pControlPipe,
                          IN const UCHAR address,
                          IN const UCHAR descriptorType,
                          IN const UCHAR descriptorIndex,
                          IN const USHORT wDescriptorSize,
                          OUT PVOID pBuffer)
//
// Purpose: query a USB device for one of its descriptors
//
// Parameters: pControlPipe - pointer to a control pipe on which
//                            to issue request
//
//             address - address of USB device to query
//
//             fIsLowSpeed - indicates if device is low speed
//
//             descriptorType - type of descriptor to get. i.e
//                              USB_DEVICE_DESCRIPTOR_TYPE
//
//             descriptorIndex - index of descriptor. For instance
//                               we may want config descriptor 3
//
//             wDescriptorSize - size of descriptor to get
//
//             pBuffer - data buffer to receive descriptor
//
// Returns: TRUE if descriptor read, else FALSE
//
// Notes: caller is responsible for buffer alloc/delete
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::GetDescriptor - address = %d, Type = %d, Index = %d, Size = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, descriptorType, descriptorIndex, wDescriptorSize) );

    PREFAST_DEBUGCHK( pControlPipe != NULL );
    PREFAST_DEBUGCHK( pBuffer != NULL );
    DEBUGCHK( m_hHubStatusChangeEvent != NULL );

    BOOL                fTransferDone = FALSE;
    DWORD               dwBytesTransferred = 0;
    DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
    HCD_REQUEST_STATUS  status = requestFailed;
    USB_DEVICE_REQUEST  usbRequest;

    if ( descriptorType == USB_HUB_DESCRIPTOR_TYPE ) {
        DEBUGCHK( descriptorIndex == 0 &&
                  wDescriptorSize >= USB_HUB_DESCRIPTOR_MINIMUM_SIZE );
        usbRequest.bmRequestType = USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_CLASS | USB_REQUEST_FOR_DEVICE;
    } else {
        DEBUGCHK( (descriptorType == USB_DEVICE_DESCRIPTOR_TYPE && descriptorIndex == 0 && (wDescriptorSize == ENDPOINT_ZERO_MIN_MAXPACKET_SIZE || wDescriptorSize == sizeof( USB_DEVICE_DESCRIPTOR ) ) ) ||
                  (descriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE && wDescriptorSize >= sizeof( USB_CONFIGURATION_DESCRIPTOR ) ) );
        usbRequest.bmRequestType = USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE;
    }
    usbRequest.bRequest = USB_REQUEST_GET_DESCRIPTOR;
    usbRequest.wValue = USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(descriptorType, descriptorIndex);
    usbRequest.wIndex = 0;
    usbRequest.wLength = wDescriptorSize;

    if ( !m_fHubThreadClosing ) {
        status = pControlPipe->IssueTransfer(
                                 address, // address of device
                                 TransferDoneCallbackSetEvent, // callback routine
                                 m_hHubStatusChangeEvent, // callback param
                                 USB_IN_TRANSFER | USB_SEND_TO_DEVICE, // transfer flags
                                 &usbRequest, // control request
                                 0, // dwStartingFrame (not used)
                                 0, // dwFrames (not used)
                                 NULL, // aLengths (not used)
                                 wDescriptorSize, // buffer size
                                 pBuffer, // buffer
                                 0, // phys addr of buffer (not used)
                                 this, // cancel ID
                                 NULL, // adwIsochErrors (not used)
                                 NULL, // adwIsochLengths (not used)
                                 &fTransferDone, // OUT status param
                                 &dwBytesTransferred, // OUT status param
                                 &dwErrorFlags ); // OUT status param

        if ( status == requestOK ) {
            DWORD dwResult = WaitForSingleObject( m_hHubStatusChangeEvent, STANDARD_REQUEST_TIMEOUT );
            if (!fTransferDone || dwResult!= WAIT_OBJECT_0) {
                pControlPipe->AbortTransfer( NULL, // callback function
                                             NULL, // callback parameter
                                             this ); // cancel ID
                if ( !m_fHubThreadClosing && m_hHubStatusChangeEvent!=NULL ) {
                    ResetEvent(m_hHubStatusChangeEvent);
                }
            }
        }
        DEBUGCHK( fTransferDone );
    }

#ifndef USB_STRICT_ENFORCEMENT
    // Some IHVs have lazy firmware writers who didn't bother to set the
    // descriptor type field in all of their descriptors. Sigh.
    if ( PUSB_COMMON_DESCRIPTOR(pBuffer)->bDescriptorType == 0 ) {
        DEBUGMSG(ZONE_WARNING, (TEXT("CHub::GetDescriptor - forcing descr type 0x%x to 0x%x\n"), PUSB_COMMON_DESCRIPTOR(pBuffer)->bDescriptorType, descriptorType));
        PUSB_COMMON_DESCRIPTOR(pBuffer)->bDescriptorType = descriptorType;
    }
#endif

    BOOL fSuccess = (status == requestOK &&
                    fTransferDone &&
                    dwBytesTransferred == wDescriptorSize &&
                    dwErrorFlags == USB_NO_ERROR &&
                    PUSB_COMMON_DESCRIPTOR(pBuffer)->bDescriptorType == descriptorType);
    // note, don't check length since some descriptors can be variable
    // length (i.e. configuration descriptor bLength field will read
    // sizeof( USB_CONFIGURATION_DESCRIPTOR ), but we may have requested
    // more than this. Or, we may only have requested USB_HUB_DESCRIPTOR_MINIMUM_SIZE
    // but the hub descriptor can be longer.

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::GetDescriptor - address = %d, Type = %d, Index = %d, Size = %d, returning %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, descriptorType, descriptorIndex, wDescriptorSize, fSuccess) );
    return fSuccess;
}
DWORD CHub::DetachDownstreamDeviceThreadStub( IN PVOID context )
{
    return ((CHub *)context )->DetachDownstreamDeviceThread();
}

// ******************************************************************
DWORD CHub::DetachDownstreamDeviceThread()
//
// Purpose: This is the worker thread for handling downstream device
//          detach when the current hub is still active (i.e. the
//          hub itself has not been detached). This thread allows
//          the hub to continue merrily on its way processing
//          attach/remove, without having to wait for the detach
//          to completely finish. That's a good thing, because we
//          can be detaching a downstream hub, which has more hubs/functions
//          attached to it, etc, and we don't want to block the active hub.
//
// Parameters: context - a CDevice* device to detach
//
// Returns: 0
//
// Notes: The memory associated with CDevice passed in is our
//        responsibility to free
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH, (TEXT("%s: +CHub::DetachDownstreamDeviceThread\n"),GetControllerName()) );

    CDevice* pDeviceToDetach = m_pDetachedDevice;//(CDevice*) context;
    m_pDetachedDevice=NULL;
    SetEvent(m_pDetachedDeviceHandled);
    PREFAST_DEBUGCHK( pDeviceToDetach != NULL );

    pDeviceToDetach->HandleDetach();
    delete pDeviceToDetach;

    //DecrCountdown(&m_objCountdown);
    m_pDeviceGlobal->ObjCountdownDec();

    DEBUGMSG( ZONE_ATTACH, (TEXT("%s: -CHub::DetachDownstreamDeviceThread\n"),GetControllerName()) );

    return 0;
}

// ******************************************************************
void CHub::DetachDevice( IN const UCHAR port )
//
// Purpose: This function is called when a device on port "port" needs
//          to be detached, and we don't want to block the hub. So,
//          we spin off a thread to do the work
//
// Parameters: port - indicates hub port on which device was detached
//
// Returns: Nothing
//
// Notes: The port will be disabled
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH, (TEXT("%s: +CHub(%s tier %d)::DetachDevice - port = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port) );

    CDevice*    pDevToDetach = NULL;
    // remove the device from our array. This will prevent
    // USBD from issuing any more requests to the device
    // or its pipes.
    EnterCriticalSection( &m_csDeviceLock );
    PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
    DEBUGCHK( port >= 1 &&
              port <= m_usbHubDescriptor.bNumberOfPorts );
    pDevToDetach = m_ppCDeviceOnPort[ port - 1 ];
    m_ppCDeviceOnPort[ port - 1 ] = NULL;
    if (pDevToDetach) {
        BOOL fSuccess;
        //fSuccess = IncrCountdown(&m_objCountdown);
        fSuccess = m_pDeviceGlobal->ObjCountdownInc();
        // if the countdown couldn't be incremented then it must have
        // been deleted already, which would mean that all devices
        // have already been detached. Which would mean that the current
        // thread has already exited which is clearly not the case.
        DEBUGCHK( fSuccess );
    }
    LeaveCriticalSection( &m_csDeviceLock );
    // it is possible that we get a device detach
    // on a NULL device. That is, if we failed to
    // configure the device and it was left plugged
    // into the USB port. When it is unplugged, a
    // detach message will be read from the port.
    DEBUGMSG( ZONE_ATTACH && ZONE_WARNING && !pDevToDetach, (TEXT("%s: CHub(%s tier %d)::DetachDevice - reading NULL device detached on port %d, doing nothing.\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port) );
    if ( pDevToDetach != NULL ) {
        ASSERT(m_pDetachedDevice==NULL);
        m_pDetachedDevice=pDevToDetach;
        HANDLE hWorkerThread = NULL;
        if (m_pDetachedDeviceHandled) {
            ResetEvent(m_pDetachedDeviceHandled);
            hWorkerThread=CreateThread( 0, 0, DetachDownstreamDeviceThreadStub, this, 0, NULL );
        }
        if ( hWorkerThread != NULL ) {
            DEBUGMSG( ZONE_ATTACH, (TEXT("%s: CHub(%s tier %d)::DetachDevice - created worker thread to handle device detach\n"),GetControllerName(), GetDeviceType(), m_tierNumber ));
            CeSetThreadPriority( hWorkerThread, g_IstThreadPriority + RELATIVE_PRIO_DOWNSTREAM);
            // Wait for Thread DetachDownstreamDeviceThreadStub take m_pDetachedDevice before ending.
            WaitForSingleObject(m_pDetachedDeviceHandled,INFINITE);
            ASSERT(m_pDetachedDevice==NULL);
            CloseHandle( hWorkerThread );
            hWorkerThread = NULL;
        } else {
            DWORD OriginalPrio = CeGetThreadPriority(GetCurrentThread());
            // no threads available - need to handle detach ourself, which
            // can potentially block this hub for a while
            DEBUGMSG( ZONE_ATTACH, (TEXT("%s: CHub(%s tier %d)::DetachDevice - no threads available, doing detach work now.\n"),GetControllerName(), GetDeviceType(), m_tierNumber ));
            DetachDownstreamDeviceThread();
            CeSetThreadPriority(GetCurrentThread(), OriginalPrio);
        }
        DEBUGCHK( hWorkerThread == NULL );
        DisablePort( port );
    }
    if (m_pAddedTT[port-1]) {  // This is TT, we need delete this.
        m_pDeviceGlobal->DeleteTt(m_address,port,m_pAddedTT[port-1]);
        m_pAddedTT[port-1]=FALSE;
    }
    DEBUGMSG( ZONE_ATTACH, (TEXT("%s: -CHub(%s tier %d)::DetachDevice - port = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, port) );
}

// ******************************************************************
BOOL CHub::AllocateDeviceArray( void )
//
// Purpose: Allocate memory for the device array of this hub based on
//          the number of ports given in the m_usbHubDescriptor structure
//
// Parameters: None
//
// Returns: TRUE if array allocated, else FALSE
//
// Notes: The entries of the array will be set to NULL by this function
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::AllocateDeviceArray\n"),GetControllerName(), GetDeviceType(), m_tierNumber) );

    BOOL fSuccess = FALSE;

    EnterCriticalSection( &m_csDeviceLock  );

    DEBUGCHK( m_ppCDeviceOnPort == NULL &&
              m_usbHubDescriptor.bNumberOfPorts > 0 );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: CHub(%s tier %d)::AllocateDeviceArray - attempting to allocate %d devices\n"),GetControllerName(), GetDeviceType(), m_tierNumber, m_usbHubDescriptor.bNumberOfPorts ) );
    m_ppCDeviceOnPort = new CDevice* [ m_usbHubDescriptor.bNumberOfPorts ];
    m_pAddedTT = new BOOL [m_usbHubDescriptor.bNumberOfPorts ];
    if ( m_ppCDeviceOnPort != NULL && m_pAddedTT!=NULL ) {
        memset( m_ppCDeviceOnPort, 0, m_usbHubDescriptor.bNumberOfPorts * sizeof( CDevice* ) );
        for (DWORD dwIndex=0;dwIndex<m_usbHubDescriptor.bNumberOfPorts; dwIndex++)
            m_pAddedTT[dwIndex] = NULL;
        fSuccess = TRUE;
    }
    DEBUGMSG( ZONE_HUB && ZONE_ERROR && !m_ppCDeviceOnPort, (TEXT("%s: CHub(%s tier %d)::AllocateDeviceArray - no memory!\n"),GetControllerName(), GetDeviceType(), m_tierNumber ) );

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::AllocateDeviceArray, returning BOOL %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, fSuccess) );
    return fSuccess;
}

// ******************************************************************
void CHub::HandleDetach( void )
//
// Purpose: This function is called when the hub is to be detached.
//
// Parameters: Nothing
//
// Returns: Nothing
//
// Notes: This function has to work for both root and external hubs
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB || ZONE_ATTACH, (TEXT("%s: +CHub(%s tier %d)::HandleDetach\n"),GetControllerName(), GetDeviceType(), m_tierNumber) );

    // close our hub thread
    DEBUGCHK( m_hHubStatusChangeEvent && m_hHubStatusChangeThread );
    // tell the thread to abort port status change processing.
    // ***DO NOT*** enter m_csDeviceLock here, otherwise we will
    // block the thread trying to close
    m_fHubThreadClosing = TRUE;
    SetEvent( m_hHubStatusChangeEvent );
    SetEvent( m_hHubSuspendBlockEvent );

    // If we are the root hub then this is a total shutdown rather than a detach,
    // so we force anybody in the middle of address assignment to give up.
    if (m_tierNumber == 0)
        //PrepareDeleteCritSec_Ex( &m_csAddress0Lock );
        m_pDeviceGlobal->Addr0LockPrepareDelete();

    // In the case where the hub was detached right when a new function was
    // being attached on one of its ports, the hub thread can be at
    // AttachDevice - DEVICE_CONFIG_STATUS_SIGNAL_NEW_DEVICE_ENTER_OPERATIONAL_STATE.
    // At this point, the thread is somewhere in USBD.DLL, and not in UHCI.DLL, and we
    // need to wait until USBD is finished. USBD can have a dialog up asking the
    // user for the driver's DLL name. So, we don't want to just blindly call
    // TerminateThread if the wait fails.
    DWORD dwWaitReturn = WAIT_FAILED;
#ifdef DEBUG
    DWORD dwTickCountStart = GetTickCount();
#endif // DEBUG
    do {
        dwWaitReturn = WaitForSingleObject( m_hHubStatusChangeThread, 1000 );
        DEBUGMSG( ZONE_WARNING && dwWaitReturn != WAIT_OBJECT_0, (TEXT("%s: CHub(%s tier %d)::HandleDetach - hub thread blocked - could be waiting for user input\n"),GetControllerName(), GetDeviceType(), m_tierNumber) );
    } while ( dwWaitReturn != WAIT_OBJECT_0 );
    DEBUGMSG( ZONE_ATTACH, (TEXT("%s: CHub(%s tier %d)::HandleDetach - status change thread closed in %d ms\n"),GetControllerName(), GetDeviceType(), m_tierNumber, GetTickCount() - dwTickCountStart ) );

    EnterCriticalSection( &m_csDeviceLock );

#ifdef DEBUG
    if ( m_address == 0 ) {
        // root hub - no pipes
        DEBUGCHK( m_ppCPipe == NULL &&
                  m_maxNumPipes == 0 );
    } else {
        // external hub - should have two pipes
        DEBUGCHK( m_ppCPipe != NULL &&
                  m_maxNumPipes == 2 &&
                  m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] != NULL &&
                  m_ppCPipe[ STATUS_CHANGE_INTERRUPT_PIPE ] != NULL );
    }
#endif // DEBUG
    for ( UCHAR pipe = 0; m_ppCPipe!=NULL && pipe < m_maxNumPipes; pipe++ ) {
        m_ppCPipe[ pipe ]->ClosePipe();
        delete m_ppCPipe[ pipe ];
        m_ppCPipe[ pipe ] = NULL;
    }
    // m_ppCPipe[] will be freed in ~CDevice

    // if m_ppCDeviceOnPort was not allocated ok, EnterOperationalState
    // would have failed, and we should never be at HandleDetach stage.
    PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
    // we need to detach all the devices on our ports
    for ( UCHAR port = 1; port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        // Don't call DetachDevice, because that function is intended for
        // when this hub is active. It has the extra overhead of spinning off
        // a thread to do the detach work. Instead, just call the HandleDetach
        // procedure directly
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
            m_ppCDeviceOnPort[ port - 1 ]->HandleDetach();
            delete m_ppCDeviceOnPort[ port - 1 ];
            m_ppCDeviceOnPort[ port - 1 ] = NULL;
        }
    }
    // m_ppCDeviceOnPort[] will be freed in ~CHub

    // Now that all pipes are closed, we can close our thread/event handles.
    // If we did this earlier, we would risk having a callback from an
    // active pipe, and having TransferDoneCallbackSetEvent accidentally
    // set a dead m_hHubStatusChangeEvent
    CloseHandle( m_hHubStatusChangeThread );
    m_hHubStatusChangeThread = NULL;

    CloseHandle( m_hHubStatusChangeEvent );
    m_hHubStatusChangeEvent = NULL;

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB || ZONE_ATTACH, (TEXT("%s: -CHub(%s tier %d)::HandleDetach\n"),GetControllerName(), GetDeviceType(), m_tierNumber ) );
}

// ******************************************************************
HCD_REQUEST_STATUS CHub::OpenPipe( IN const UINT address,
                                   IN LPCUSB_ENDPOINT_DESCRIPTOR const lpEndpointDescriptor,
                                   OUT LPUINT const lpiEndpointIndex )
//
// Purpose: Open a new pipe to the device with the given address. The
//          pipe information is passed in by lpEndpointDescriptor, and
//          the index of the new pipe is returned in *lpiEndpointIndex
//
// Parameters: see CHcd::OpenPipe
//
// Returns: requestOK - if pipe opened successfully, and index stored
//                      in lpEndpointDescriptor.
//
//          requestFailed - if unable to open the pipe on the device
//                          with the given address
//
//          requestIgnored - if we were unable to find a device with
//                           the given address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::OpenPipe - address = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );

    // no one should be calling OpenPipe on hubs, since we handle
    // hubs internally.
    DEBUGCHK( address != m_address );

    PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
    for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
            status = m_ppCDeviceOnPort[ port - 1 ]->OpenPipe( address,
                                                              lpEndpointDescriptor,
                                                              lpiEndpointIndex );
        }
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("-CHub(%s tier %d)::OpenPipe - address = %d, returing HCD_REQUEST_STATUS %d\n"), GetDeviceType(), m_tierNumber, address, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CHub::ClosePipe( IN const UINT address,
                                    IN const UINT pipeIndex )
//
// Purpose: Close pipe "pipeIndex" on device with address "address"
//
// Parameters: See description in CHcd::ClosePipe
//
// Returns: requestOK - if pipe closed
//
//          requestFailed - if pipe exists, but unable to close it
//
//          requestIgnored - if no device found with matching address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CHub(%s tier %d)::ClosePipe - address = %d, pipeIndex = %d\n"), GetDeviceType(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );

    // no one should be calling ClosePipe on hubs, since we handle
    // hubs internally.
    DEBUGCHK( address != m_address );

    PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
    for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
            status = m_ppCDeviceOnPort[ port - 1 ]->ClosePipe( address,
                                                               pipeIndex );
        }
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::ClosePipe - address = %d, pipeIndex = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, pipeIndex, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CHub::IssueTransfer(
                            IN const UINT address,
                            IN const UINT pipeIndex,
                            IN LPTRANSFER_NOTIFY_ROUTINE const lpStartAddress,
                            IN LPVOID const lpvNotifyParameter,
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
                            OUT LPDWORD const lpdwBytesTransfered,
                            OUT LPDWORD const lpdwError )
//
// Purpose: Issue a USB transfer to the pipe "pipe" on the device whose
//          address is "address"
//
// Parameters: see CHcd::OpenPipe
//
// Returns: requestOK - if transfer issued
//
//          requestFailed - if unable to issue transfer
//
//          requestIgnored - if we were unable to find a device with
//                           the given address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CHub(%s tier %d)::IssueTransfer, address = %d, pipe = %d\n"), GetDeviceType(), m_tierNumber, address, pipeIndex));

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );

    // no one should be calling IssueTransfer on hubs, since we handle
    // hubs internally.
    DEBUGCHK( address != m_address );

    PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
    for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {            
            status = m_ppCDeviceOnPort[ port - 1 ]->IssueTransfer(
                                                        address,
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
        }
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("-CHub(%s tier %d)::IssueTransfer address = %d, pipe = %d, returing HCD_REQUEST_STATUS %d\n"), GetDeviceType(), m_tierNumber, address, pipeIndex, status ) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CHub::AbortTransfer(
                            IN const UINT address,
                            IN const UINT pipeIndex,
                            IN LPTRANSFER_NOTIFY_ROUTINE const lpCancelAddress,
                            IN LPVOID const lpvNotifyParameter,
                            IN LPCVOID const lpvCancelId )
//
// Purpose: Abort transfer currently occuring on device with USB address
//          "address", pipe "pipeIndex".
//
// Parameters: See description in CHcd::AbortTransfer
//
// Returns: requestOK - if transfer aborted
//
//          requestFailed - if transfer exists, but unable to abort it
//
//          requestIgnored - if address doesn't match this device's address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::AbortTransfer - address = %d, pipeIndex = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );

    // no one should be calling AbortTransfer on hubs, since we handle
    // hubs internally.
    DEBUGCHK( address != m_address );

    PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
    for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
            status = m_ppCDeviceOnPort[ port - 1 ]->AbortTransfer( address,
                                                                   pipeIndex,
                                                                   lpCancelAddress,
                                                                   lpvNotifyParameter,
                                                                   lpvCancelId );
        }
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::AbortTransfer - address = %d, pipeIndex = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, pipeIndex, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CHub::IsPipeHalted( IN const UINT address,
                                       IN const UINT pipeIndex,
                                       OUT LPBOOL const lpbHalted )
//
// Purpose: Determine if the pipe "pipeIndex" on device at
//          address "address" is halted, and return result in
//          lpbHalted
//
// Parameters: See description in CHcd::IsPipeHalted
//
// Returns: requestOK - if lpbHalted set to pipe's correct status
//
//          requestFailed - if pipe exists, but unable to set lpbHalted
//
//          requestIgnored - if pipe does not exist
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CHub(%s tier %d)::IsPipeHalted - address = %d, pipeIndex = %d\n"), GetDeviceType(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );

    // no one should be calling IsPipeHalted on hubs, since we handle
    // hubs internally.
    DEBUGCHK( address != m_address );

    PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
    for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
            status = m_ppCDeviceOnPort[ port - 1 ]->IsPipeHalted( address,
                                                                  pipeIndex,
                                                                  lpbHalted );
        }
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::IsPipeHalted - address = %d, pipe = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, pipeIndex, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CHub::ResetPipe( IN const UINT address,
                                    IN const UINT pipeIndex )
//
// Purpose: Reset pipe "pipeIndex" on device with address "address"
//
// Parameters: See description in CHcd::ResetPipe
//
// Returns: requestOK - if pipe reset
//
//          requestFailed - if pipe exists, but unable to reset it
//
//          requestIgnored - if no device found with matching address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CHub(%s tier %d)::ResetPipe - address = %d, pipeIndex = %d\n"), GetDeviceType(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );

    // no one should be calling ResetPipe on hubs, since we handle
    // hubs internally.
    DEBUGCHK( address != m_address );
    PREFAST_DEBUGCHK( m_ppCDeviceOnPort!=NULL );
    
    DEBUGCHK( m_ppCDeviceOnPort != NULL );
    for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
            status = m_ppCDeviceOnPort[ port - 1 ]->ResetPipe( address,
                                                               pipeIndex );
        }
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::ResetPipe - address = %d, pipeIndex = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, pipeIndex, status) );
    return status;
}
// ******************************************************************
HCD_REQUEST_STATUS CHub::DisableDevice( IN const UINT address,
                                     IN const BOOL fReset )
//
// Purpose: Disable Downstream Device with address "address"
//
// Parameters: See description in CHcd::DisableDevice.
//
// Returns: requestOK - if Device Reset
//
//          requestFailed - if device exists, but unable to disable it.
//
//          requestIgnored - if no device found with matching address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CHub(%s tier %d)::DisableDevice - address = %d, pipeIndex = %d\n"), GetDeviceType(), m_tierNumber, address, fReset) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );
    if ( address != m_address ) {
        PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
        for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
            if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
                status =  m_ppCDeviceOnPort[ port - 1 ]-> DisableDevice(address, fReset);
            }
        }
    }
    else { // It should not happens
        ASSERT(FALSE);
        status = requestFailed;
    }
    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::DisableDevice - address = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, status) );
    return status;
}
BOOL CHub::NotifyOnSuspendedResumed(BOOL fResumed)
{
    for ( UCHAR port = 1; m_ppCDeviceOnPort != NULL && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) { // Notify all down stream device.
            m_ppCDeviceOnPort[ port - 1 ]-> NotifyOnSuspendedResumed(fResumed);
        }
    }
    return TRUE;
}

// ******************************************************************
HCD_REQUEST_STATUS CHub::SuspendResume( IN const UINT address,
                                     IN const BOOL fSuspend )
//
// Purpose: Suspend or Resume on device with address "address"
//
// Parameters: See description in CHcd::SuspendResume
//
// Returns: requestOK - if device suspend or resumed
//
//          requestFailed - if device exists, but unable to reset it
//
//          requestIgnored - if no device found with matching address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CHub(%s tier %d)::SuspendResume - address = %d, fSuspend = %d\n"), GetDeviceType(), m_tierNumber, address, fSuspend) );

    HCD_REQUEST_STATUS status = requestIgnored;

    EnterCriticalSection( &m_csDeviceLock );
    if ( address != m_address ) {
        PREFAST_DEBUGCHK( m_ppCDeviceOnPort != NULL );
        for ( UCHAR port = 1; status == requestIgnored && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
            if ( m_ppCDeviceOnPort[ port - 1 ] != NULL ) {
                status = m_ppCDeviceOnPort[ port - 1 ] ->SuspendResume(address, fSuspend) ;
            }
        }
    }
    else { // It should not happens
        ASSERT(FALSE);
        status = requestFailed;
    }
    LeaveCriticalSection( &m_csDeviceLock );
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::SuspendResume - address = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, status) );
    return status;
}
BOOL CHub::DisableOffStreamDevice( IN const UINT address, IN const BOOL fReset )
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::DisableOffStreamDevice - address = %d, pipeIndex = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, fReset) );
    EnterCriticalSection( &m_csDeviceLock );
    BOOL fReturn = FALSE;
    for ( UCHAR port = 1; port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if ( m_ppCDeviceOnPort[ port - 1 ] != NULL &&  m_ppCDeviceOnPort[ port - 1 ]->GetDeviceAddress()==address) {
            USB_HUB_AND_PORT_STATUS     hubStatus;
            if (GetStatus(port, hubStatus) && hubStatus.status.port.PortConnected) {
                // Hub Status change thread will NOT wake up by Disable Port becuase
                // DisablePort will NOT generate  hubStatus.change.port.PortEnableChange 
                // So after DisablePort(port), we have to call AttachDevice Manually
                DetachDevice(port );
                if (fReset) 
                    AttachDevice( port, hubStatus.status.port.DeviceIsLowSpeed,hubStatus.status.port.DeviceIsHighSpeed );
            }
            fReturn = TRUE;
            break;
        }
    }
    LeaveCriticalSection( &m_csDeviceLock );    
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CHub(%s tier %d)::DisableOffStreamDevice - address = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, fReturn) );
    ASSERT(fReturn == TRUE);
    return fReturn;

}
BOOL CHub::SuspendResumeOffStreamDevice( IN const UINT address, IN const BOOL fSuspend )
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CHub(%s tier %d)::SuspendResume - address = %d, fSuspend = %d\n"),GetControllerName(), GetDeviceType(), m_tierNumber, address, fSuspend) );
    BOOL fReturn = FALSE;
    for ( UCHAR port = 1; port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        if (m_ppCDeviceOnPort[ port - 1 ]!= NULL && m_ppCDeviceOnPort[ port - 1 ]->GetDeviceAddress()== address) { // Found.
            if (SetOrClearFeature( port,
                   fSuspend? USB_REQUEST_SET_FEATURE: USB_REQUEST_CLEAR_FEATURE,
                   USB_HUB_FEATURE_PORT_SUSPEND)) {
                fReturn = TRUE;
                if ( !fSuspend) { // If it is resume.
                    Sleep(20);
                    m_ppCDeviceOnPort[ port-1 ]-> ResumeNotification();
                }
                m_ppCDeviceOnPort[ port-1 ]-> NotifyOnSuspendedResumed(!fSuspend);
            }
            break;
        }
    }
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("-CHub(%s tier %d)::SuspendResume - address = %d, returing HCD_REQUEST_STATUS %d\n"), GetDeviceType(), m_tierNumber, address, fReturn) );
    return fReturn;
}

// ******************************************************************
CRootHub::CRootHub( IN const USB_DEVICE_INFO& rDeviceInfo,
                    IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                    IN const USB_HUB_DESCRIPTOR& rUsbHubDescriptor ,
                    IN CHcd * const pCHcd  )
//
// Purpose: Constructor for CRootHub
//
// Parameters: See description in CHub
//
// Returns: Nothing.
//
// Notes: Do not initialize static variables here. Do that in
//        the Initialize() routine
//
//        The root hub gets address 0, since real devices are not
//        allowed to communicate on address 0 except at config time,
//        and since the RootHub never needs to send transfers for itself.
//        Also, root hub is by definition at tier 0
// ******************************************************************
: CHub( 0, rDeviceInfo, fIsLowSpeed, fIsHighSpeed, 0, rUsbHubDescriptor,pCHcd,NULL,0 ) // call base constructor
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CRootHub::CRootHub\n"),GetControllerName()) );
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CRootHub::CRootHub\n"),GetControllerName()) );
}

// ******************************************************************
CRootHub::~CRootHub( )
//
// Purpose: Destructor for CRootHub
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: Do not delete static variables here. Do that in
//        DeInitialize();
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CRootHub::~CRootHub\n")) );
    // Nothing to do here yet...
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CRootHub::~CRootHub\n"),GetControllerName()) );
}

// ******************************************************************
BOOL CRootHub::EnterOperationalState( IN CPipeAbs * const DEBUG_ONLY( pEndpoint0Pipe ) )
//
// Purpose: Do processing needed to get this hub into a working state.
//          For now, -create device on ports array
//                   -start status change thread
//
// Parameters: pEndpoint0Pipe - pointer to open pipe for this device's
//                              endpoint0. (ignored for root hub)
//
// Returns: TRUE if hub is working ok, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB, (TEXT("%s: +CRootHub::EnterOperationalState\n"),GetControllerName()) );

    BOOL fSuccess = FALSE;

    EnterCriticalSection( &m_csDeviceLock );

    // don't need to allocate a Pipe array for Root Hubs
    DEBUGCHK( m_address == 0 &&
              m_maxNumPipes == 0 &&
              m_ppCPipe == NULL &&
              pEndpoint0Pipe == NULL &&
              m_deviceInfo.Descriptor.bDeviceClass == USB_DEVICE_CLASS_HUB &&
              m_deviceInfo.Descriptor.bNumConfigurations == 0 &&
              m_deviceInfo.lpActiveConfig == NULL &&
              m_deviceInfo.lpConfigs == NULL );

    DEBUGCHK( m_hHubStatusChangeEvent == NULL && m_hHubStatusChangeThread == NULL );

    // m_hHubStatusChangeEvent - Auto Reset, and Initial State = non-signaled
    m_hHubStatusChangeEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( m_hHubStatusChangeEvent != NULL &&
         AllocateDeviceArray() ) {

        m_hHubStatusChangeThread = CreateThread( 0, 0, HubStatusChangeThreadStub, this, 0, NULL );
        if ( m_hHubStatusChangeThread != NULL ) {
            CeSetThreadPriority( m_hHubStatusChangeThread, g_IstThreadPriority + RELATIVE_PRIO_STSCHG);
            fSuccess = TRUE;
        }
    }
    if ( !fSuccess ) {
        // m_ppCDeviceOnPort will be freed in ~CHub if needed

        if ( m_hHubStatusChangeEvent ) {
            CloseHandle( m_hHubStatusChangeEvent );
            m_hHubStatusChangeEvent = NULL;
        }
        DEBUGCHK( m_hHubStatusChangeThread == NULL );
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB, (TEXT("%s: -CRootHub::EnterOperationalState, returning BOOL %d\n"),GetControllerName(), fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL CRootHub::PowerAllHubPorts( void )
//
// Purpose: Power all of this hub's ports so that they can send
//          status change notifications
//
// Parameters: none
//
// Returns: TRUE - the root hub's ports are already powered
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CRootHub::PowerAllHubPorts\n"),GetControllerName()) );
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("-CRootHub::PowerAllHubPorts\n")) );
    return TRUE;
}

// ******************************************************************
BOOL CRootHub::WaitForPortStatusChange( OUT UCHAR& rPort,
                                        OUT USB_HUB_AND_PORT_STATUS& rStatus )
//
// Purpose: Wait until a status change occurs on one of this hub's ports,
//          then return the status change information in rPort and rStatus
//
// Parameters: rPort - out param to get port # of port whose status changed
//
//             rStatus - out structure describing port's status change
//
// Returns: TRUE if rPort and rStatus set properly, else FALSE
//
// Notes: This routine is also responsible for clearing the status
//        change notifications from the port itself
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("+CRootHub::WaitForPortStatusChange\n")) );

    BOOL fSuccess = FALSE;
    // root hub - we need to poll for status changes.
    while ( !m_fHubThreadClosing && !fSuccess ) {
        if (!m_pCHcd->WaitForPortStatusChange(m_hHubStatusChangeEvent)) {
            // If HCD does not support Root Hub Status Change. We do follows
            WaitForSingleObject( m_hHubStatusChangeEvent, 100 );
        }
        if ( m_fHubThreadClosing ) {
            break;
        }

        DEBUGMSG(ZONE_ATTACH && ZONE_VERBOSE, (TEXT("CRootHub::WaitForPortStatusChange::NoOfPort(0x%x)\r\n"), m_usbHubDescriptor.bNumberOfPorts));
        for ( UCHAR port = 1; !fSuccess && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
            if ( m_pCHcd->DidPortStatusChange( port ) ) {
                // port status changed on this port
                rPort = port;
                // we could just call CHW::GetPortStatus, but it is
                // better to call CRootHub::GetStatus which is
                // designed to implement the virtual CHub::GetStatus function.
                fSuccess = GetStatus( port, rStatus );
                DEBUGMSG(ZONE_ATTACH && ZONE_VERBOSE, (TEXT("GetStatus return %d\r\n"), fSuccess));
            }
        }
    }
    if (fSuccess) {
        // acknowledge the change bits
        for (USHORT bit = 0; bit < 16; ++bit) {
            if (rStatus.change.word & (1 << bit))
                SetOrClearFeature( rPort, USB_REQUEST_CLEAR_FEATURE, bit | 0x10 );
        }
    }

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CRootHub::WaitForPortStatusChange, rPort = %d, fSuccess = %d\n"),GetControllerName(), rPort, fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL  CRootHub::GetStatus( IN const UCHAR port,
                           OUT USB_HUB_AND_PORT_STATUS& rStatus )
//
// Purpose: This function will get the status of the port and return it
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
//             rStatus - reference to USB_HUB_AND_PORT_STATUS to get the
//                       status
//
// Returns: Return value of CHW::GetPortStatus function
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("+CRootHub::GetStatus - port = %d\n"), port ));

    DEBUGCHK( port <= m_usbHubDescriptor.bNumberOfPorts );

    // CHW::GetPortStatus will not clear the change bits
    BOOL fSuccess = m_pCHcd->GetPortStatus( port, rStatus );

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CRootHub::GetStatus - port = %d, returing BOOL %d\n"),GetControllerName(), port, fSuccess ) );
    return fSuccess;
}

// ******************************************************************
BOOL CRootHub::ResetAndEnablePort( IN const UCHAR port )
//
// Purpose: reset/enable device on the given port so that when this
//          function completes, the device is listening on address 0
//
// Parameters: port - port # to reset/enable
//
// Returns: Return value of CHW::ResetAndEnablePort function
//
// Notes: Assumes address0 lock is held
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CRootHub::ResetAndEnablePort - port = %d\n"),GetControllerName(), port) );

    DEBUGCHK( port >= 1 && port <= m_usbHubDescriptor.bNumberOfPorts );

    BOOL fSuccess = m_pCHcd->ResetAndEnablePort( port );

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CRootHub::ResetAndEnablePort - port = %d, returning %d\n"),GetControllerName(), port, fSuccess ) );
    return fSuccess;
}

// ******************************************************************
void CRootHub::DisablePort( IN const UCHAR port )
//
// Purpose: disable the given port
//
// Parameters: port - port # to disable
//
// Returns: nothing
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CRootHub::DisablePort - port = %d\n"),GetControllerName(), port) );

    DEBUGCHK( port >= 1 && port <= m_usbHubDescriptor.bNumberOfPorts );

    m_pCHcd->DisablePort( port );

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CRootHub::DisablePort - port = %d\n"),GetControllerName(), port) );
}

// ******************************************************************
BOOL CRootHub::SetOrClearFeature( IN const WORD port,
                                  IN const UCHAR setOrClearFeature,
                                  IN const USHORT feature )
//
// Purpose: This function will set or clear a feature on the given port
//          if that feature exists.
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
//             setOrClearFeature - this is USB_REQUEST_SET_FEATURE or
//                                 USB_REQUEST_CLEAR_FEATURE
//
//             feature - one of the features to set/clear - this should
//                       be USB_FEATURE_ENDPOINT_STALL or one of the
//                       USB_HUB_FEATURE_* features
//
// Returns: TRUE if the request succeeded, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CRootHub::SetOrClearFeature - port = %d, set/clear = 0x%x, feature = 0x%x\n"),GetControllerName(), port, setOrClearFeature, feature) );

#ifdef DEBUG
{
    if ( setOrClearFeature == USB_REQUEST_CLEAR_FEATURE ) {
        if ( port == 0 ) {
            // USB spec 1.1, 11.16.2.2 - these are the only
            // features which should be cleared for ports
            DEBUGCHK( feature == USB_HUB_FEATURE_C_HUB_LOCAL_POWER ||
                      feature == USB_HUB_FEATURE_C_HUB_OVER_CURRENT ||
                      feature == USB_FEATURE_ENDPOINT_STALL );
        } else {
            // USB spec 1.1, 11.16.2.2 - these are the only
            // features which should be cleared for ports
            DEBUGCHK( feature == USB_HUB_FEATURE_PORT_ENABLE ||
                      feature == USB_HUB_FEATURE_PORT_SUSPEND ||
                      feature == USB_HUB_FEATURE_PORT_POWER ||
                      feature == USB_HUB_FEATURE_C_PORT_CONNECTION ||
                      feature == USB_HUB_FEATURE_C_PORT_RESET ||
                      feature == USB_HUB_FEATURE_C_PORT_ENABLE ||
                      feature == USB_HUB_FEATURE_C_PORT_SUSPEND ||
                      feature == USB_HUB_FEATURE_C_PORT_OVER_CURRENT );
        }
    } else if ( setOrClearFeature == USB_REQUEST_SET_FEATURE ) {
        // should only be setting port features
        DEBUGCHK( port > 0 &&
                  (feature == USB_HUB_FEATURE_PORT_RESET ||
                   feature == USB_HUB_FEATURE_PORT_SUSPEND ||
                   feature == USB_HUB_FEATURE_PORT_INDICATOR ||
                   feature == USB_HUB_FEATURE_PORT_POWER) );
    } else {
        // shouldn't be here
        DebugBreak();
    }
    DEBUGCHK( port <= m_usbHubDescriptor.bNumberOfPorts );
}
#endif // DEBUG

    BOOL fSuccess = m_pCHcd->RootHubFeature((UCHAR)port, setOrClearFeature, feature);

    DEBUGMSG( ZONE_ERROR && !fSuccess, (TEXT("%s: CRootHub::SetOrClearFeature - port = %d, set/clear = 0x%x, feature = 0x%x, FAILED\n"),GetControllerName(), port, setOrClearFeature, feature ) );
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("-CRootHub::SetOrClearFeature - port = %d, set/clear = 0x%x, feature = 0x%x, returing BOOL %d\n"), port, setOrClearFeature, feature, fSuccess) );
    return fSuccess;
}
// ******************************************************************
BOOL  CRootHub::SetOrClearRemoteWakup(BOOL /*bSet*/)
//
// Purpose: This function will set or clear a remove wakeup feature 
//          External HUB.
//
//
// Returns: TRUE if the request succeeded, else FALSE
//
// Notes:
// ******************************************************************
{
    return TRUE;
}

// ******************************************************************
CExternalHub::CExternalHub( IN const UCHAR address,
                            IN const USB_DEVICE_INFO& rDeviceInfo,
                            IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                            IN const UCHAR tierNumber,
                            IN const USB_HUB_DESCRIPTOR& rUsbHubDescriptor,
                            IN CHcd * const pCHcd  ,
                            IN CHub * const pAttachedHub,const UCHAR uAttachedPort)
                            
//
// Purpose: Constructor for CExternalHub
//
// Parameters: See description in CHub
//
// Returns: Nothing.
//
// Notes: Do not initialize static variables here. Do that in
//        the Initialize() routine
// ******************************************************************
: CHub( address, rDeviceInfo, fIsLowSpeed,fIsHighSpeed, tierNumber, rUsbHubDescriptor,pCHcd , pAttachedHub,uAttachedPort)
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::CExternalHub\n"),GetControllerName()) );
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CExternalHub::CExternalHub\n"),GetControllerName()) );
}

// ******************************************************************
CExternalHub::~CExternalHub( )
//
// Purpose: Destructor for CExternalHub
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: Do not delete static variables here. Do that in
//        DeInitialize();
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::~CExternalHub\n"),GetControllerName()) );
    // Nothing to do here yet...
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CExternalHub::~CExternalHub\n"),GetControllerName()) );
}

// ******************************************************************
BOOL CExternalHub::EnterOperationalState( IN CPipeAbs* const pEndpoint0Pipe )
//
// Purpose: Do processing needed to get this hub into a working state.
//          For now, -create device on ports array
//                   -create status change pipe
//                   -start status change thread
//
// Parameters: pEndpoint0Pipe - pointer to open pipe for this device's
//                              endpoint0
//
// Returns: TRUE if hub is working ok, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB, (TEXT("%s: +CExternalHub::EnterOperationalState\n"),GetControllerName()) );

    BOOL fSuccess = FALSE;

    EnterCriticalSection( &m_csDeviceLock );

    DEBUGCHK( m_address > 0 &&
              m_address <= USB_MAX_ADDRESS &&
              m_maxNumPipes == 0 && // not yet allocated
              m_ppCPipe == NULL && // not yet allocated
              pEndpoint0Pipe != NULL && // should be passed in non NULL by caller
              m_deviceInfo.Descriptor.bDeviceClass == USB_DEVICE_CLASS_HUB );

    DEBUGCHK( m_hHubStatusChangeEvent == NULL );
    // m_hHubStatusChangeEvent - Auto Reset, and Initial State = non-signaled
    m_hHubStatusChangeEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    #ifdef DEBUG
    {

#ifndef USB_STRICT_ENFORCEMENT
        // The correct settings are listed in the 1.1 USB spec at section 11.15.1.
        // I have found at least one hub (builtin on the Clarion CDC) that does this wrong.
        if (!m_fIsHighSpeed)
            m_deviceInfo.lpActiveConfig->lpInterfaces[0].lpEndpoints[0].Descriptor.bInterval = 0xff;
#endif
        // check for status change endpoint descriptor
        const USB_ENDPOINT_DESCRIPTOR& rED = m_deviceInfo.lpActiveConfig->lpInterfaces[0].lpEndpoints[0].Descriptor;
        DEBUGCHK( rED.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
                  (rED.bEndpointAddress & 0xf) > 0 &&
                  (rED.bInterval == 0xff || m_fIsHighSpeed) &&
                  (rED.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_INTERRUPT );
    }
    #endif // DEBUG
    UCHAR uTTAddress=0;
    UCHAR uTTPort=0;
    BOOL ttContext = NULL;
    GetUSB2TT(uTTAddress, uTTPort, ttContext);
    CPipeAbs* pStatusChangePipe = CreateInterruptPipe ( &m_deviceInfo.lpActiveConfig->lpInterfaces[0].lpEndpoints[0].Descriptor,
                                                            m_fIsLowSpeed,m_fIsHighSpeed,m_address,
                                                            uTTAddress,uTTPort,ttContext,
                                                            m_pCHcd);

    // be sure to call OpenPipe last
    if ( pStatusChangePipe != NULL &&
         m_hHubStatusChangeEvent != NULL &&
         AllocatePipeArray() &&
         AllocateDeviceArray() &&
         pStatusChangePipe->SetReservedDeviceAddr(m_address) && 
         requestOK == pStatusChangePipe->OpenPipe() ) {

        // Thread will use the pipes, so put them into our array
        DEBUGCHK( m_maxNumPipes == 2 &&
                  m_ppCPipe != NULL );
        m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] = pEndpoint0Pipe;
        m_ppCPipe[ STATUS_CHANGE_INTERRUPT_PIPE ] = pStatusChangePipe;
        m_hHubStatusChangeThread = CreateThread( 0, 0, HubStatusChangeThreadStub, this, 0, NULL );
        if ( m_hHubStatusChangeThread != NULL ) {
            CeSetThreadPriority( m_hHubStatusChangeThread, g_IstThreadPriority + RELATIVE_PRIO_STSCHG);
            fSuccess = TRUE;
        } else {
            pStatusChangePipe->ClosePipe();
        }
    }
    if ( !fSuccess ) {
        // pStatusChangePipe would have been closed above in case of failure
        delete pStatusChangePipe;

        // caller will take care of closing the endpoint 0 pipe. We NULL out
        // the entries in the m_ppCPipe array because otherwise both our
        // ~CDevice destructor and the caller would try to free the pipe.
        m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] = NULL;
        m_ppCPipe[ STATUS_CHANGE_INTERRUPT_PIPE ] = NULL;
        // m_ppCPipe[] will be freed in ~CDevice

        // m_ppCDeviceOnPort [] will be freed in ~CHub

        if ( m_hHubStatusChangeEvent ) {
            CloseHandle( m_hHubStatusChangeEvent );
            m_hHubStatusChangeEvent = NULL;
        }

        DEBUGCHK( m_hHubStatusChangeThread == NULL );
    }

    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_HUB, (TEXT("%s: -CExternalHub::EnterOperationalState, returning BOOL %d\n"),GetControllerName(), fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL CExternalHub::PowerAllHubPorts( void )
//
// Purpose: Power all of this hub's ports so that they can send
//          status change notifications
//
// Parameters: none
//
// Returns: TRUE if hub ports powered, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::PowerAllHubPorts\n"),GetControllerName()) );

    BOOL fFailed = FALSE;

    for ( UCHAR port = 1; !fFailed && port <= m_usbHubDescriptor.bNumberOfPorts; port++ ) {
        fFailed = !SetOrClearFeature( port,
                                      USB_REQUEST_SET_FEATURE,
                                      USB_HUB_FEATURE_PORT_POWER );
        if (!fFailed)
            SetOrClearFeature(port,
                                      USB_REQUEST_SET_FEATURE,
                                      USB_HUB_FEATURE_PORT_INDICATOR); // Set to the Auto Indicator.
    }

    DEBUGMSG( ZONE_ERROR && fFailed, (TEXT("%s: CExternalHub::PowerAllHubPorts - failed!\n"),GetControllerName()) );
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CExternalHub::PowerAllHubPorts, returing %d\n"),GetControllerName(), !fFailed) );
    return !fFailed;
}

// ******************************************************************
BOOL CExternalHub::GetStatusChangeBitmap( OUT DWORD& rdwHubBitmap )
//
// Purpose: Wait until a status change occurs on one of this hub's ports,
//          then return the hubBitmap showing which ports have changed
//
// Parameters: rdwHubBitmap - OUT param to get hub bitmap
//
// Returns: TRUE if rdwHubBitmap set, else FALSE
//
// Notes: For a description of the hub bitmap, see USB spec 1.1,
//        section 11.13.4
// ******************************************************************
{
#define HUB_INTERRUPT_TRANFER_TIMEOUT (10*1000)
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::GetStatusChangeBitmap\n"),GetControllerName()) );

    // for 1-7 ports,  the bitmap will be 1 byte,
    // for 8-15 ports, the bitmap will be 2 bytes,
    // etc. We only support up to 31 ports since
    // DWORD is 4 bytes.
    DEBUGCHK( m_usbHubDescriptor.bNumberOfPorts <= 31 );
    const DWORD         dwBitmapSize = 1 + (m_usbHubDescriptor.bNumberOfPorts >> 3);
    BOOL                fSuccess = FALSE;
    BOOL                fTransferDone = FALSE;
    DWORD               dwBytesTransferred = 0;
    DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
    HCD_REQUEST_STATUS  status = requestFailed;

    rdwHubBitmap = 0;

    // pipe 1 = status change endpoint pipe
    if ( !m_fHubThreadClosing) {
        DEBUGCHK( m_maxNumPipes == 2 );
        PREFAST_DEBUGCHK( m_ppCPipe != NULL );
        PREFAST_DEBUGCHK( m_ppCPipe[ STATUS_CHANGE_INTERRUPT_PIPE ] != NULL );

        status = m_ppCPipe[ STATUS_CHANGE_INTERRUPT_PIPE ]->IssueTransfer(
                                    m_address, // hub address,
                                    TransferDoneCallbackSetEvent, // callback func
                                    m_hHubStatusChangeEvent, // callback param
                                    USB_IN_TRANSFER | USB_SEND_TO_DEVICE, // transfer flags
                                    NULL, // lpvControlHeader (not used)
                                    0, // dwStartingFrame (not used)
                                    0, // dwFrames (not used)
                                    NULL, // aLengths (not used)
                                    dwBitmapSize, // bitmap size to read
                                    &rdwHubBitmap, // data buffer
                                    0, // physical address of buffer (not used)
                                    this, // cancel id
                                    NULL, // adwIsochErrors (not used)
                                    NULL, // adwIsochLengths (not used)
                                    &fTransferDone, // OUT status param
                                    &dwBytesTransferred, // OUT status param
                                    &dwErrorFlags ); // OUT status param
        DEBUGMSG( ZONE_ERROR && status != requestOK, (TEXT("%s: CExternalHub::GetStatusChangeBitmap - error issuing transfer!\n"),GetControllerName()));
        if ( status == requestOK ) {
            WaitForSingleObject( m_hHubStatusChangeEvent, INFINITE);                    
            DEBUGMSG(ZONE_ATTACH, (TEXT("CExternalHub::: Get Data value 0x%x, fTransferDone %d\r\n"), rdwHubBitmap, fTransferDone));
            if (!fTransferDone ) {
                m_ppCPipe[ STATUS_CHANGE_INTERRUPT_PIPE ]->AbortTransfer(
                                               NULL, // callback function
                                               NULL, // callback parameter
                                               this ); // cancel ID
                // After Abort the has been called. So reset the event.
                ResetEvent( m_hHubStatusChangeEvent );
            }
            else
                DEBUGCHK( fTransferDone );
        }
    }
    fSuccess = ( status == requestOK &&
                 fTransferDone &&
                 dwBytesTransferred == dwBitmapSize &&
                 dwErrorFlags == USB_NO_ERROR &&
                 rdwHubBitmap != 0 );

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("-CExternal::GetStatusChangeBitmap, bitmap = 0x%08x, returing fSuccess = %d\n"), rdwHubBitmap, fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL CExternalHub::WaitForPortStatusChange( OUT UCHAR& rPort,
                                            OUT USB_HUB_AND_PORT_STATUS& rStatus )
//
// Purpose: Wait until a status change occurs on one of this hub's ports,
//          then return the status change information in rPort and rStatus
//
// Parameters: rPort - out param to get port # of port whose status changed
//
//             rStatus - out structure describing port's status change
//
// Returns: TRUE if rPort and rStatus set properly, else FALSE
//
// Notes: This routine is also responsible for clearing the status
//        change notifications from the port itself
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::WaitForPortStatusChange\n"),GetControllerName()) );

    DWORD dwStatusChangeBitmap = 0;
    BOOL  fSuccess = FALSE;

    if ( GetStatusChangeBitmap( dwStatusChangeBitmap ) ) {
        DEBUGCHK( dwStatusChangeBitmap != 0 );
        // see which port changed
        for ( rPort = 0; rPort <= m_usbHubDescriptor.bNumberOfPorts; rPort++ ) {
            if ( dwStatusChangeBitmap & (1 << rPort) ) {
                // deal with this port change, and report others next time around
                break;
            }
        }
        DEBUGCHK( rPort <= m_usbHubDescriptor.bNumberOfPorts );
//        if (m_fIsHighSpeed) { //USB2.0 11.24.2.7 This will cause Reset squence and get speed correctly.
//            BOOL bResult=SetOrClearFeature(rPort,USB_REQUEST_SET_FEATURE,USB_HUB_FEATURE_PORT_RESET);
//            ASSERT(bResult==TRUE);
//        }
        // now we need to get the actual status change
        if ( GetStatus( rPort, rStatus ) ) {
            // we need to clear each of the changed feature flags
            // otherwise we will get the change notification
            // forever.
            //
            // For ports, each changeBit corresponds to the change
            // feature "changeBit | 0x10". This happens due to the
            // way features are numbered.

            UCHAR maxChangeBit = USB_HUB_FEATURE_PORT_RESET;
            UCHAR changeBitToClearFeature = 0x10;
            if ( rPort == 0 ) { // change was on the hub itself
                // For hubs, each changeBit corresponds directly to
                // the feature "changeBit", so we | with 0.
                maxChangeBit = USB_HUB_FEATURE_C_HUB_OVER_CURRENT;
                changeBitToClearFeature = 0;
            }
            for ( UCHAR changeBit = 0; !m_fHubThreadClosing && changeBit <= maxChangeBit; changeBit++ ) {
                if ( rStatus.change.word & (1 << changeBit) ) {
                    SetOrClearFeature( rPort,
                                       USB_REQUEST_CLEAR_FEATURE,
                                       changeBit | changeBitToClearFeature );
                    // Could add recover if this fails...
                }
            }

            if(rPort > m_usbHubDescriptor.bNumberOfPorts)
                fSuccess = FALSE; //Otherwise an access violation will occur.
            else
                fSuccess = TRUE;
        }
    }

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CExternal::WaitForPortStatusChange, returing rPort = %d, fSuccess = %d\n"),GetControllerName(), rPort, fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL  CExternalHub::GetStatus( IN const UCHAR port,
                               OUT USB_HUB_AND_PORT_STATUS& rStatus )
//
// Purpose: This function will get the status of the port and return it
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
//             rStatus - reference to USB_HUB_AND_PORT_STATUS to get the
//                       status
//
// Returns: TRUE if the request succeeded, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::GetStatus - port = %d\n"),GetControllerName(), port ));

    PREFAST_DEBUGCHK( m_ppCPipe != NULL);
    DEBUGCHK( m_maxNumPipes == 2 &&
              m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] != NULL &&
              port <= m_usbHubDescriptor.bNumberOfPorts );

    BOOL                fTransferDone = FALSE;
    DWORD               dwBytesTransferred = 0;
    DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
    HCD_REQUEST_STATUS  status = requestFailed;
    USB_DEVICE_REQUEST  usbRequest;

    if ( port == 0 ) {
        // requst is to the hub iteself
        usbRequest.bmRequestType = USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_CLASS | USB_REQUEST_FOR_DEVICE;
    } else {
        // request is to an actual port
        usbRequest.bmRequestType = USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_CLASS | USB_REQUEST_FOR_OTHER;
    }
    usbRequest.bRequest = USB_REQUEST_GET_STATUS;
    usbRequest.wValue = 0;
    usbRequest.wIndex = port;
    usbRequest.wLength = sizeof( USB_HUB_AND_PORT_STATUS );
    // wLength should be 4, according to USB spec 1.1 11.16.2.5/11.16.2.6
    DEBUGCHK( sizeof( USB_HUB_AND_PORT_STATUS ) == 4 );

    if ( !m_fHubThreadClosing ) {
        status = m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->IssueTransfer(
                         m_address, // address of this hub
                         TransferDoneCallbackSetEvent, // callback func
                         m_hHubStatusChangeEvent, // callback param
                         USB_IN_TRANSFER | USB_SEND_TO_DEVICE, // transfer flags
                         &usbRequest, // control request
                         0, // dwStartingFrame (not used)
                         0, // dwFrames (not used)
                         NULL, // aLengths (not used)
                         sizeof( USB_HUB_AND_PORT_STATUS ), // buffer size
                         &rStatus, // buffer
                         0, // phys addr of buffer (not used)
                         this, // cancel id
                         NULL, // adwIsochErrors (not used)
                         NULL, // adwIsochLengths (not used)
                         &fTransferDone, // OUT param for transfer
                         &dwBytesTransferred, // OUT param for transfer
                         &dwErrorFlags ); // OUT param for transfer
        if ( status == requestOK ) {
            WaitForSingleObject( m_hHubStatusChangeEvent, INFINITE );
            if ( m_fHubThreadClosing ) {
                m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->AbortTransfer(
                                             NULL, // callback function
                                             NULL, // callback parameter
                                             this ); // cancel ID
            }
        }
        DEBUGMSG(1, (TEXT("Abort Transfer for Device Address 0x%x\r\n"),
            m_address));
        DEBUGCHK( fTransferDone );
    }
    BOOL fSuccess = (status == requestOK &&
                     fTransferDone &&
                     dwBytesTransferred == sizeof( USB_HUB_AND_PORT_STATUS ) &&
                     dwErrorFlags == USB_NO_ERROR);

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CExternalHub::GetStatus - port = %d, returing BOOL %d\n"),GetControllerName(), port, fSuccess) );
    return fSuccess;
}

// ******************************************************************
BOOL CExternalHub::ResetAndEnablePort( IN const UCHAR port )
//
// Purpose: reset/enable device on the given port so that when this
//          function completes, the device is listening on address 0
//
// Parameters: port - port # to reset/enable
//
// Returns: TRUE if port reset, else FALSE
//
// Notes: Assumes address0 lock is held
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::ResetAndEnablePort - port = %d\n"),GetControllerName(), port) );

    DEBUGCHK( port >= 1 && port <= m_usbHubDescriptor.bNumberOfPorts );

    BOOL fSuccess = FALSE;

    if ( !SetOrClearFeature( port,
                            USB_REQUEST_SET_FEATURE,
                            USB_HUB_FEATURE_PORT_RESET ) ) {
        DEBUGMSG( ZONE_ERROR, (TEXT("%s: -CExternalHub::ResetAndEnablePort - could set reset feature for port %d\n"),GetControllerName(), port) );
        return FALSE;
    }
    for ( UCHAR attempt = 0; !m_fHubThreadClosing && attempt < 5; attempt++ ) {
        DWORD   dwStatusChangeBitmap = 0;
        // USB2.0 11.5.1.5 20 ms resetting period.
        Sleep(20);
        if ( GetStatusChangeBitmap( dwStatusChangeBitmap ) &&
             (dwStatusChangeBitmap & (1 << port)) ) {
            USB_HUB_AND_PORT_STATUS portStatus;
            fSuccess = ( GetStatus( port, portStatus ) &&
                         portStatus.change.port.ResetChange && // port has reset
                         !portStatus.status.port.PortReset && // reset no longer asserted
                         portStatus.status.port.PortEnabled && // port enabled
                         SetOrClearFeature( port, // clear reset change flag
                                            USB_REQUEST_CLEAR_FEATURE,
                                            USB_HUB_FEATURE_C_PORT_RESET ) );
            if (fSuccess) { // According Specification. 50 ms delay after this.
                Sleep(50);
            }
            break;
        }
    }
#if 0
    while ( fSuccess && !m_fHubThreadClosing ) {
        // make sure reset is signalled for at least 10ms
        Sleep( 10 );
        fSuccess = !m_fHubThreadClosing && GetStatus( port, portStatus );
        if ( fSuccess &&
             portStatus.change.port.ResetChange && // port has reset
             !portStatus.status.port.PortReset && // reset no longer asserted
             portStatus.status.port.PortEnabled ) { // port enabled

            DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: CExternalHub::ResetAndEnablePort - status.change = 0x%x, status.word = 0x%x\n"),GetControllerName(), portStatus.change.word, portStatus.status.word) );
            Sleep( 10 ); // give reset 10 ms to take hold
            break;
        }
    }
#endif
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CExternalHub::ResetAndEnablePort - port = %d, returning %d\n"),GetControllerName(), port, fSuccess ) );
    return fSuccess;
}

// ******************************************************************
void CExternalHub::DisablePort( IN const UCHAR port )
//
// Purpose: disable the given port
//
// Parameters: port - port # to disable
//
// Returns: nothing
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::DisablePort - port = %d\n"),GetControllerName(), port) );

    if ( !m_fHubThreadClosing ) {
        SetOrClearFeature( port,
                           USB_REQUEST_CLEAR_FEATURE,
                           USB_HUB_FEATURE_PORT_ENABLE );
        // disable port can take time to act
        Sleep( 10 );
    }

    DEBUGMSG( ZONE_ATTACH && ZONE_VERBOSE, (TEXT("%s: -CExternalHub::DisablePort - port = %d\n"),GetControllerName(), port) );
}

// ******************************************************************
BOOL CExternalHub::SetOrClearFeature( IN const WORD port,
                                      IN const UCHAR setOrClearFeature,
                                      IN const USHORT feature )
//
// Purpose: This function will set or clear a feature on the given port.
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
//             setOrClearFeature - this is USB_REQUEST_SET_FEATURE or
//                                 USB_REQUEST_CLEAR_FEATURE
//
//             feature - one of the features to set/clear - this should
//                       be USB_FEATURE_ENDPOINT_STALL or one of the
//                       USB_HUB_FEATURE_* features
//
// Returns: TRUE if the request succeeded, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::SetOrClearFeature - port = %d, set/clear = 0x%x, feature = 0x%x\n"),GetControllerName(), port, setOrClearFeature, feature) );

#ifdef DEBUG
{
    if ( setOrClearFeature == USB_REQUEST_CLEAR_FEATURE ) {
        if ( port == 0 ) {
            // USB spec 1.1, 11.16.2.2 - these are the only
            // features which should be cleared for ports
            DEBUGCHK( feature == USB_HUB_FEATURE_C_HUB_LOCAL_POWER ||
                      feature == USB_HUB_FEATURE_C_HUB_OVER_CURRENT ||
                      feature == USB_FEATURE_ENDPOINT_STALL );
        } else {
            // USB spec 1.1, 11.16.2.2 - these are the only
            // features which should be cleared for ports
            DEBUGCHK( feature == USB_HUB_FEATURE_PORT_ENABLE ||
                      feature == USB_HUB_FEATURE_PORT_SUSPEND ||
                      feature == USB_HUB_FEATURE_PORT_POWER ||
                      feature == USB_HUB_FEATURE_C_PORT_CONNECTION ||
                      feature == USB_HUB_FEATURE_C_PORT_RESET ||
                      feature == USB_HUB_FEATURE_C_PORT_ENABLE ||
                      feature == USB_HUB_FEATURE_C_PORT_SUSPEND ||
                      feature == USB_HUB_FEATURE_C_PORT_OVER_CURRENT );
        }
    } else if ( setOrClearFeature == USB_REQUEST_SET_FEATURE ) {
        // should only be setting port features
        DEBUGCHK( port > 0 &&
                  (feature == USB_HUB_FEATURE_PORT_RESET ||
                   feature == USB_HUB_FEATURE_PORT_SUSPEND ||
                   feature == USB_HUB_FEATURE_PORT_INDICATOR ||
                   feature == USB_HUB_FEATURE_PORT_POWER) );
    } else {
        // shouldn't be here
        DebugBreak();
    }
    DEBUGCHK( m_maxNumPipes == 2 && // control and status change
              m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] != NULL &&
              port <= m_usbHubDescriptor.bNumberOfPorts );
}
#endif // DEBUG

    BOOL                fTransferDone = FALSE;
    DWORD               dwBytesTransferred = 0;
    DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
    HCD_REQUEST_STATUS  status = requestFailed;
    USB_DEVICE_REQUEST  usbRequest;
    if ( port == 0 ) {
        usbRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_CLASS | USB_REQUEST_FOR_DEVICE;
    } else {
        usbRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_CLASS | USB_REQUEST_FOR_OTHER;
    }
    usbRequest.bRequest = setOrClearFeature;
    usbRequest.wValue = feature;
    usbRequest.wIndex = port;
    usbRequest.wLength = 0;
    PREFAST_DEBUGCHK(m_ppCPipe != NULL);
    
    if ( !m_fHubThreadClosing ) {
        status = m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->IssueTransfer(
                         m_address, // address of this hub
                         TransferDoneCallbackSetEvent, // callback func
                         m_hHubStatusChangeEvent, // callback param
                         USB_OUT_TRANSFER | USB_SEND_TO_DEVICE, // transfer flags
                         &usbRequest, // control request
                         0, // dwStartingFrame (not used)
                         0, // dwFrames (not used)
                         NULL, // aLengths (not used)
                         0, // buffer size
                         NULL, // buffer
                         0, // phys addr of buffer (not used)
                         this, // cancel id
                         NULL, // adwIsochErrors (not used)
                         NULL, // adwIsochLengths (not used)
                         &fTransferDone, // OUT param for transfer
                         &dwBytesTransferred, // OUT param for transfer
                         &dwErrorFlags ); // OUT param for transfer
        if ( status == requestOK ) {
            WaitForSingleObject( m_hHubStatusChangeEvent,STANDARD_REQUEST_TIMEOUT);         
            if ( m_fHubThreadClosing ) {
                m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->AbortTransfer(
                                               NULL, // callback function
                                               NULL, // callback parameter
                                               this ); // cancel ID
            }
        }
        //DEBUGCHK( fTransferDone );
    }
    BOOL fSuccess = (status == requestOK &&
                     fTransferDone &&
                     dwBytesTransferred == 0 &&
                     dwErrorFlags == USB_NO_ERROR);

    DEBUGMSG( ZONE_ERROR && !fSuccess, (TEXT("%s: CExternalHub::SetOrClearFeature - port = %d, set/clear = 0x%x, feature = 0x%x, FAILED\n"),GetControllerName(), port, setOrClearFeature, feature ) );
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CExternalHub::SetOrClearFeature - port = %d, set/clear = 0x%x, feature = 0x%x, returing BOOL %d\n"),GetControllerName(), port, setOrClearFeature, feature, fSuccess) );
    return fSuccess;
}
// ******************************************************************
BOOL  CExternalHub::SetOrClearRemoteWakup(BOOL bSet)
//
// Purpose: This function will set or clear a remove wakeup feature 
//          External HUB.
//
//
// Returns: TRUE if the request succeeded, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: +CExternalHub::SetOrClearRemoteWakup - bSet = %d\r\n"),GetControllerName(), bSet) );

    BOOL                fTransferDone = FALSE;
    DWORD               dwBytesTransferred = 0;
    DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
    HCD_REQUEST_STATUS  status = requestFailed;
    USB_DEVICE_REQUEST  usbRequest;
    
    usbRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_FOR_DEVICE;
    usbRequest.bRequest = (bSet? USB_REQUEST_SET_FEATURE : USB_REQUEST_CLEAR_FEATURE);
    usbRequest.wValue = USB_FEATURE_REMOTE_WAKEUP;
    usbRequest.wIndex = 0;
    usbRequest.wLength = 0;

    if ( !m_fHubThreadClosing ) {
        status = m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->IssueTransfer(
                         m_address, // address of this hub
                         TransferDoneCallbackSetEvent, // callback func
                         m_hHubStatusChangeEvent, // callback param
                         USB_OUT_TRANSFER | USB_SEND_TO_DEVICE, // transfer flags
                         &usbRequest, // control request
                         0, // dwStartingFrame (not used)
                         0, // dwFrames (not used)
                         NULL, // aLengths (not used)
                         0, // buffer size
                         NULL, // buffer
                         0, // phys addr of buffer (not used)
                         this, // cancel id
                         NULL, // adwIsochErrors (not used)
                         NULL, // adwIsochLengths (not used)
                         &fTransferDone, // OUT param for transfer
                         &dwBytesTransferred, // OUT param for transfer
                         &dwErrorFlags ); // OUT param for transfer
        if ( status == requestOK ) {
            WaitForSingleObject( m_hHubStatusChangeEvent, INFINITE );
            if ( m_fHubThreadClosing ) {
                m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->AbortTransfer(
                                               NULL, // callback function
                                               NULL, // callback parameter
                                               this ); // cancel ID
            }
        }
        DEBUGCHK( fTransferDone );
    }
    BOOL fSuccess = (status == requestOK &&
                     fTransferDone &&
                     dwBytesTransferred == 0 &&
                     dwErrorFlags == USB_NO_ERROR);

    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("-CExternalHub::SetOrClearRemoteWakup -  returing BOOL %d\n"),fSuccess));
    return fSuccess;
    
}

//***************************************************************************************
void CExternalHub::SignalHub(void) 
//
//  Purpose: Signal HUB Status Change Event to exit from the Waiting interrupt pipe
{ 
    if (m_hHubStatusChangeEvent) 
        SetEvent(m_hHubStatusChangeEvent); 
    // We need to call this again so that it would go to next tier ... until the end    
    SignalHubStatusChange();

}
// ******************************************************************
CFunction::CFunction( IN const UCHAR address,
                      IN const USB_DEVICE_INFO& rDeviceInfo,
                      IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                      IN const UCHAR tierNumber,
                      IN CHcd * const pCHcd,
                      IN CHub * const pAttachedHub,const UCHAR uAttachedPort)
//
// Purpose: Constructor for CFunction
//
// Parameters: address, rDeviceInfo, fIsLowSpeed, tierNumber - see CDevice::CDevice
//
// Returns: Nothing.
//
// Notes: Do not initialize static variables here. Do that in
//        the Initialize() routine
//
//        Do not call into USBD yet, that will be done in EnterOperationalState
// ******************************************************************
: CDevice( address, rDeviceInfo, fIsLowSpeed,fIsHighSpeed, tierNumber,pCHcd ,pAttachedHub,uAttachedPort ) // call base constructor
, m_pCHcd (pCHcd)
, m_lpvDetachId( NULL ) // detach id for USBD
, m_hFunctionFeatureEvent( NULL )
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction(tier %d)::CFunction\n"),GetControllerName(), tierNumber) );
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::CFunction\n"),GetControllerName(), tierNumber) );
}

// ******************************************************************
CFunction::~CFunction( )
//
// Purpose: Destructor for CFunction
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: Do not delete static variables here. Do that in
//        the DeInitialize() routine
//
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction(tier %d)::~CFunction\n"),GetControllerName(), m_tierNumber) );

    DEBUGCHK( m_lpvDetachId == NULL );

    // rest of processing done in base destructors...
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::~CFunction\n"),GetControllerName(), m_tierNumber) );
}

// ******************************************************************
BOOL CFunction::EnterOperationalState( IN CPipeAbs* const pEndpoint0Pipe )
//
// Purpose: Enable this function to start processing USB transactions
//
// Parameters: pEndpoint0Pipe - pointer to an already open control pipe
//                              to the function's endpoint 0
//
// Returns: TRUE if pipe array is allocated and call to USBD succeeds
//          else FALSE
//
// Notes:
//
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION, (TEXT("%s: +CFunction(tier %d)::EnterOperationalState\n"),GetControllerName(), m_tierNumber) );

    BOOL fSuccess = FALSE;

    EnterCriticalSection( &m_csDeviceLock );

    DEBUGCHK( m_pCHcd->GetpUSBDAttachProc() != NULL &&
              m_pCHcd->GetpHcdContext() != NULL &&
              m_lpvDetachId == NULL &&  // device not attached yet
              m_ppCPipe == NULL && // not allocated yet
              m_maxNumPipes == 0 && // refers to m_ppCPipe, which is not allocated
              m_address > 0 &&
              m_address <= USB_MAX_ADDRESS &&
              pEndpoint0Pipe != NULL ); // control pipe to endpoint 0

    DEBUGCHK(m_hFunctionFeatureEvent == NULL);
    // m_hFunctionFeatureEvent - Auto Reset, and Initial State = non-signaled
    m_hFunctionFeatureEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
  
    if ( AllocatePipeArray() ) {
        m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] = pEndpoint0Pipe;
        if (SetOrClearFeature( USB_REQUEST_FOR_DEVICE,0,USB_REQUEST_SET_FEATURE,USB_FEATURE_B_HNP_ENABLE)){
            BOOL bReturn = SetOrClearFeature( USB_REQUEST_FOR_DEVICE,0,USB_REQUEST_SET_FEATURE,USB_FEATURE_A_HNP_SUPPORT ) ;
            DEBUGCHK(bReturn);
            bReturn = m_pCHcd->BusChildIoControl(IOCTL_BUS_USBOTG_HNP_ENABLE,NULL,0);
            ASSERT(bReturn);        
        }
        else
        {
            RETAILMSG(1,(TEXT("SET FEATURE USB_FEATURE_B_HNP_ENABLE is Failed \r\n")));       
            m_pCHcd->BusChildIoControl(IOCTL_BUS_USBOTG_HNP_DISABLE, NULL, 0);
        }
        LeaveCriticalSection( &m_csDeviceLock );
#if 1
        {
            USB_DEVICE_DESCRIPTOR                   Descriptor;            
            LPNON_CONST_USB_CONFIGURATION           lpActiveConfig;

            Descriptor = m_deviceInfo.Descriptor;            
            lpActiveConfig = m_deviceInfo.lpActiveConfig;
            RETAILMSG(1, (TEXT("USBDAttachProc with address %d\r\n"), m_address));
            RETAILMSG(1, (TEXT("m_deviceInfo =>\r\n")));
            RETAILMSG(1, (TEXT("dwCount = 0x%x\r\n"), m_deviceInfo.dwCount));
            RETAILMSG(1, (TEXT("USB_DEVICE_DESCRIPTOR:\r\n")));
            RETAILMSG(1, (TEXT("bLength = 0x%x\r\n"), Descriptor.bLength));
            RETAILMSG(1, (TEXT("bDescriptorType = 0x%x\r\n"), Descriptor.bDescriptorType));
            RETAILMSG(1, (TEXT("bcdUSB = 0x%x\r\n"), Descriptor.bcdUSB));
            RETAILMSG(1, (TEXT("bDeviceClass = 0x%x\r\n"), Descriptor.bDeviceClass));
            RETAILMSG(1, (TEXT("bDeviceSubClass = 0x%x\r\n"), Descriptor.bDeviceSubClass));
            RETAILMSG(1, (TEXT("bDeviceProtocol = 0x%x\r\n"), Descriptor.bDeviceProtocol));
            RETAILMSG(1, (TEXT("bMaxPacketSize0 = 0x%x\r\n"), Descriptor.bMaxPacketSize0));
            RETAILMSG(1, (TEXT("idVendor = 0x%x\r\n"), Descriptor.idVendor));
            RETAILMSG(1, (TEXT("idProduct = 0x%x\r\n"), Descriptor.idProduct));
            RETAILMSG(1, (TEXT("bcdDevice = 0x%x\r\n"), Descriptor.bcdDevice));
            RETAILMSG(1, (TEXT("iManufacturer = 0x%x\r\n"), Descriptor.iManufacturer));
            RETAILMSG(1, (TEXT("iProduct = 0x%x\r\n"), Descriptor.iProduct));
            RETAILMSG(1, (TEXT("iSerialNumber = 0x%x\r\n"), Descriptor.iSerialNumber));
            RETAILMSG(1, (TEXT("bNumConfigurations = 0x%x\r\n"), Descriptor.bNumConfigurations));
          
        }
#endif
        fSuccess = (*m_pCHcd->GetpUSBDAttachProc())(m_pCHcd->GetpHcdContext(), // context for Host Controller Driver
                                        m_address, // used to indicate this specific device
                                        ENDPOINT0_CONTROL_PIPE, // index of endpoint 0 pipe
                                        LPCUSB_DEVICE( &m_deviceInfo ), // USB descriptors
                                        &m_lpvDetachId ); // used to tell USBD this device is being detached
        DEBUGCHK( !fSuccess || m_lpvDetachId != NULL );
        EnterCriticalSection( &m_csDeviceLock );
    }
    if ( !fSuccess ) {
        DEBUGMSG( ZONE_ERROR, (TEXT("%s: CFunction(tier %d)::EnterOperationalState - failed\n"),GetControllerName(), m_tierNumber) );
        // caller will handle pEndpoint0Pipe
        delete [] m_ppCPipe;
        m_ppCPipe = NULL;
        m_maxNumPipes = 0;

        m_lpvDetachId = NULL;
    }
    LeaveCriticalSection( &m_csDeviceLock );

    DEBUGMSG( ZONE_FUNCTION, (TEXT("%s: -CFunction(tier %d)::EnterOperationalState, returning BOOL %d\n"),GetControllerName(), m_tierNumber, fSuccess) );
    return fSuccess;
}

BOOL CFunction::NotifyOnSuspendedResumed(BOOL fResumed)
{
    EnterCriticalSection( &m_csDeviceLock );
    LPUSBD_SUSPEND_RESUME_PROC pProc= m_pCHcd->GetpUSBDSuspendedResumed();
    BOOL fSuccess = FALSE;
    if (pProc && m_lpvDetachId ) {
        fSuccess = (*pProc)(m_lpvDetachId, fResumed);
    }
    LeaveCriticalSection( &m_csDeviceLock );
    return fSuccess;
}


// ******************************************************************
HCD_REQUEST_STATUS CFunction::OpenPipe( IN const UINT address,
                                        IN LPCUSB_ENDPOINT_DESCRIPTOR const lpEndpointDescriptor,
                                        OUT LPUINT const lpiEndpointIndex )
//
// Purpose: Open a new pipe to the device with the given address. The
//          pipe information is passed in by lpEndpointDescriptor, and
//          the index of the new pipe is returned in *lpiEndpointIndex
//
// Parameters: see CHcd::OpenPipe
//
// Returns: requestOK - if pipe opened successfully, and index stored
//                      in lpEndpointDescriptor.
//
//          requestFailed - if unable to open the pipe on the device
//                          with the given address
//
//          requestIgnored - if we were unable to find a device with
//                           the given address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction(tier %d)::OpenPipe - address = %d\n"),GetControllerName(), m_tierNumber, address) );

    HCD_REQUEST_STATUS status = requestIgnored;
    UCHAR pipe				  = 0;
	UCHAR bNumEndpoints		  = 0;
	UCHAR bEndpoint			  = 0;
	UCHAR bMaxEndpoints		  = 0;
    int cInterfaces			  = 0;
    BOOL fFound				  = FALSE;

    // don't have to enter critical section until after checking
    // m_address (since m_address is const). That ensures we
    // won't wait for the critical section unless we actually
    // have something to do
    if ( address == m_address ) {
        EnterCriticalSection( &m_csDeviceLock );
        // OpenPipe is referring to this device. Must return
        // something other than requestIgnored.
        status = requestFailed;

        PREFAST_DEBUGCHK( m_ppCPipe != NULL);
        DEBUGCHK( m_maxNumPipes > 0 &&
                  m_deviceInfo.lpActiveConfig != NULL &&
                  m_deviceInfo.lpActiveConfig->lpInterfaces != NULL);// &&
//                  m_maxNumPipes == m_deviceInfo.lpActiveConfig->lpInterfaces[0].Descriptor.bNumEndpoints + 1 );

//        DEBUGCHK( m_deviceInfo.lpActiveConfig->lpInterfaces[0].lpEndpoints != NULL );
        // There are m_maxNumPipes, but the endpoint descriptor for
        // the control pipe to endpoint 0 is not stored in our array of
        // endpoint descriptors. Thus, we only need to check
        // m_maxNumPipes - 1 pipes. The pipe needs to be indexed from 1,
        // since 0 already refers to the endpoint0 control pipe.
//        DEBUGCHK( m_maxNumPipes - 1 == m_deviceInfo.lpActiveConfig->lpInterfaces[0].Descriptor.bNumEndpoints );
        DEBUGCHK( m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] != NULL );
        pipe = 0;
        fFound = FALSE;
        bMaxEndpoints = 0;
        cInterfaces = m_deviceInfo.lpActiveConfig->dwNumInterfaces;
        if (cInterfaces < 1) {
            DEBUGCHK(0); // shouldn't be possible - test is here to make prefast happy.
            return status;
        }
        for ( UCHAR bInterface = 0; bInterface < cInterfaces && !fFound; bInterface++ ){
            bNumEndpoints = m_deviceInfo.lpActiveConfig->lpInterfaces[bInterface].Descriptor.bNumEndpoints;
            for(bEndpoint = 0; bEndpoint < bNumEndpoints && !fFound; bEndpoint++ ){
                if ( memcmp( &m_deviceInfo.lpActiveConfig->lpInterfaces[bInterface].lpEndpoints[ bEndpoint ].Descriptor, lpEndpointDescriptor, sizeof( USB_ENDPOINT_DESCRIPTOR ) ) == 0 ) {
                    fFound = TRUE;
                }
            }
            if(m_deviceInfo.lpActiveConfig->lpInterfaces[bInterface].Descriptor.bAlternateSetting != 0){
                if(bMaxEndpoints < bNumEndpoints)
                    bMaxEndpoints = bNumEndpoints;
            }
            else {
                pipe = pipe + (UCHAR)bMaxEndpoints;
                bMaxEndpoints = bNumEndpoints;
            }

        }

        DEBUGCHK(fFound);

        if(fFound){
            pipe = pipe + (UCHAR)bEndpoint;

            DEBUGMSG( ZONE_ERROR && pipe == m_maxNumPipes, (TEXT("%s: CFunction(tier %d)::OpenPipe - endpoint descriptor doesn't match any of this device's endpoints!\n"),GetControllerName(), m_tierNumber) );
            if ( pipe < m_maxNumPipes ) {
                if ( m_ppCPipe[ pipe ] != NULL ) {
                    DEBUGMSG( ZONE_WARNING, (TEXT("%s: CFunction(tier %d)::OpenPipe - address %d pipe %d appears to have been opened before\n"),GetControllerName(), m_tierNumber, address, pipe) );
                    status = requestOK;
                    *lpiEndpointIndex = pipe;
                } else {
                    UCHAR uTTAddress=0;
                    UCHAR uTTPort=0;
                    BOOL ttContext = NULL;
                    GetUSB2TT(uTTAddress, uTTPort,ttContext);
                    switch ( lpEndpointDescriptor->bmAttributes & USB_ENDPOINT_TYPE_MASK ) {
                      case USB_ENDPOINT_TYPE_BULK:
                        m_ppCPipe[ pipe ] = CreateBulkPipe( lpEndpointDescriptor,
                                                           m_fIsLowSpeed,m_fIsHighSpeed,m_address,
                                                           uTTAddress,uTTPort,ttContext,
                                                           m_pCHcd);
                        break;
                      case USB_ENDPOINT_TYPE_CONTROL:
                        m_ppCPipe[ pipe ] = CreateControlPipe( lpEndpointDescriptor,
                                                              m_fIsLowSpeed,m_fIsHighSpeed,m_address,
                                                              uTTAddress,uTTPort,ttContext,
                                                              m_pCHcd);
                        break;
                      case USB_ENDPOINT_TYPE_INTERRUPT:
                        m_ppCPipe[ pipe ] = CreateInterruptPipe( lpEndpointDescriptor,
                                                                m_fIsLowSpeed,m_fIsHighSpeed,m_address,
                                                                uTTAddress,uTTPort,ttContext,
                                                                m_pCHcd);
                        break;
                      case USB_ENDPOINT_TYPE_ISOCHRONOUS:
#if 0
                        m_ppCPipe[ pipe ] = CreateIsochronousPipe( lpEndpointDescriptor,
                                                                  m_fIsLowSpeed,m_fIsHighSpeed,m_address,
                                                                  uTTAddress,uTTPort,ttContext,
                                                                  m_pCHcd);
#else
                        RETAILMSG(1, (TEXT("Isochronous not supported\r\n")));
#endif
                        break;
#ifdef DEBUG
                      default:
                        DebugBreak(); // shouldn't be here
                            break;
#endif // DEBUG
                    }
                    if ( m_ppCPipe[ pipe ] != NULL &&
                         m_ppCPipe[ pipe ]->SetReservedDeviceAddr(m_address) && 
                         m_ppCPipe[ pipe ]->OpenPipe() == requestOK ) {
                        DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: CFunction(tier %d)::OpenPipe - opened new pipe, address %d, pipe %d\n"),GetControllerName(), m_tierNumber, address, pipe ) );
                        status = requestOK;
                        *lpiEndpointIndex = pipe;
                    } else {
                        delete m_ppCPipe[ pipe ];
                        m_ppCPipe[ pipe ] = NULL;
                    }
                }
            }
        }
        LeaveCriticalSection( &m_csDeviceLock );
    }

    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::OpenPipe - address = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), m_tierNumber, address, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CFunction::ClosePipe( IN const UINT address,
                                         IN const UINT pipeIndex )
//
// Purpose: Close the pipe "pipeIndex" if this device's address
//          is "address"
//
// Parameters: See description in CHcd::ClosePipe
//
// Returns: requestOK - if pipe closed or doesn't exist
//
//          requestFailed - if pipe exists, but unable to close it
//
//          requestIgnored - if this device's address is not "address"
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction(tier %d)::ClosePipe - address = %d, pipe = %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;


    // don't have to enter critical section until after checking
    // m_address (since m_address is const). That ensures we
    // won't wait for the critical section unless we actually
    // have something to do
    if ( address == m_address ) {
        EnterCriticalSection( &m_csDeviceLock );
        status = requestOK; // if the pipe doesn't exists, we can return
                            // that ClosePipe worked
        if ( pipeIndex < m_maxNumPipes && m_ppCPipe[ pipeIndex ] != NULL ) {
            status = m_ppCPipe[ pipeIndex ]->ClosePipe( );
            DEBUGMSG( ZONE_FUNCTION, (TEXT("%s: CFunction(tier %d)::ClosePipe - address = %d, deleting pipe %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex) );
            delete m_ppCPipe[ pipeIndex ];
            m_ppCPipe[ pipeIndex ] = NULL;
        }
    #ifdef DEBUG
        else {
            DEBUGMSG( ZONE_WARNING, (TEXT("%s: CFunction(tier %d)::ClosePipe - warning, pipe does not exist. Returning requestOK\n"),GetControllerName(), m_tierNumber) );
        }
    #endif // DEBUG
        LeaveCriticalSection( &m_csDeviceLock );
    }

    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::ClosePipe - address = %d, pipe = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CFunction::IssueTransfer(
                            IN const UINT address,
                            IN const UINT pipeIndex,
                            IN LPTRANSFER_NOTIFY_ROUTINE const lpStartAddress,
                            IN LPVOID const lpvNotifyParameter,
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
                            OUT LPDWORD const lpdwBytesTransfered,
                            OUT LPDWORD const lpdwError )
//
// Purpose: Issue a USB transfer to the pipe "pipe" on the device whose
//          address is "address"
//
// Parameters: see CHcd::OpenPipe
//
// Returns: requestOK - if transfer issued
//
//          requestFailed - if unable to issue transfer
//
//          requestIgnored - if we were unable to find a device with
//                           the given address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("+CFunction(tier %d)::IssueTransfer\n"), m_tierNumber));

    HCD_REQUEST_STATUS status = requestIgnored;

    // don't have to enter critical section until after checking
    // m_address (since m_address is const). That ensures we
    // won't wait for the critical section unless we actually
    // have something to do
    if ( address == m_address ) {
        EnterCriticalSection( &m_csDeviceLock );
        status = requestFailed;
        if (m_fIsSuspend ) {
            RETAILMSG(1,(TEXT("!!!CFunction( tier %d)::IssueTransfer on suspend state is illgle, address = %d, pipe = %d\n"), m_tierNumber, address, pipeIndex));
            ASSERT(FALSE);
        }
        else
        if ( pipeIndex < m_maxNumPipes && m_ppCPipe[ pipeIndex ] != NULL ) {
            DEBUGMSG(ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("CFunction::IssueTransfer => pipeIndex[%d], size[%d], dir[%s]\r\n"),
                pipeIndex, dwBufferSize, ((dwFlags & USB_IN_TRANSFER)? TEXT("IN"):TEXT("OUT"))));
            status = m_ppCPipe[ pipeIndex ]->IssueTransfer(
                                                        (UCHAR)address,
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
        }
        LeaveCriticalSection( &m_csDeviceLock );
    }


    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::IssueTransfer - returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), m_tierNumber, status ) );
    return status;
}
// ******************************************************************
HCD_REQUEST_STATUS CFunction::AbortTransfer(
                            IN const UINT address,
                            IN const UINT pipeIndex,
                            IN LPTRANSFER_NOTIFY_ROUTINE const lpCancelAddress,
                            IN LPVOID const lpvNotifyParameter,
                            IN LPCVOID const lpvCancelId )
//
// Purpose: Abort the transfer occuring on pipe "pipeIndex"
//
// Parameters: See description in CHcd::AbortTransfer
//
// Returns: requestOK - if transfer aborted
//
//          requestFailed - if no transfer exists, or transfer exists
//                          and unable to abort it
//
//          requestIgnored - if "address" doesn't match this device's
//                           address
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction(tier %d)::AbortTransfer - address = %d, pipe = %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;

    // don't have to enter critical section until after checking
    // m_address (since m_address is const). That ensures we
    // won't wait for the critical section unless we actually
    // have something to do
    if ( address == m_address ) {
        EnterCriticalSection( &m_csDeviceLock );
        status = requestFailed;
        if ( pipeIndex < m_maxNumPipes && m_ppCPipe[ pipeIndex ] != NULL ) {
            status = m_ppCPipe[ pipeIndex ]->AbortTransfer( lpCancelAddress,
                                                            lpvNotifyParameter,
                                                            lpvCancelId );
        }
        LeaveCriticalSection( &m_csDeviceLock );
    }

    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::AbortTransfer - address = %d, pipe = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CFunction::IsPipeHalted( IN const UINT address,
                                            IN const UINT pipeIndex,
                                            OUT LPBOOL const lpbHalted )
//
// Purpose: Determine if the pipe "pipeIndex" on device at
//          address "address" is halted, and return result in
//          lpbHalted
//
// Parameters: See description in CHcd::IsPipeHalted
//
// Returns: requestOK - if lpbHalted set to pipe's correct status
//
//          requestFailed - if pipe exists, but unable to set lpbHalted
//
//          requestIgnored - if pipe does not exist
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction(tier %d)::IsPipeHalted - address = %d, pipe = %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;

    // don't have to enter critical section until after checking
    // m_address (since m_address is const). That ensures we
    // won't wait for the critical section unless we actually
    // have something to do
    if ( address == m_address ) {
        EnterCriticalSection( &m_csDeviceLock );
        status = requestFailed;
        if (m_fIsSuspend ) {
            RETAILMSG(1,(TEXT("!!!CFunction( tier %d)::IsPipeHalted on suspend state is illgle, address = %d, pipe = %d\n"), m_tierNumber, address, pipeIndex));
            ASSERT(FALSE);
        }
        else
        if ( pipeIndex < m_maxNumPipes && m_ppCPipe[ pipeIndex ] != NULL ) {
            status = m_ppCPipe[ pipeIndex ]->IsPipeHalted( lpbHalted );
        }
        LeaveCriticalSection( &m_csDeviceLock );
    }

    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::IsPipeHalted - address = %d, pipe = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex, status) );
    return status;
}

// ******************************************************************
HCD_REQUEST_STATUS CFunction::ResetPipe( IN const UINT address,
                                         IN const UINT pipeIndex )
//
// Purpose: Reset the pipe "pipeIndex" if this device's address
//          is "address"
//
// Parameters: See description in CHcd::ResetPipe
//
// Returns: requestOK - if pipe reset or doesn't exist
//
//          requestFailed - if pipe exists, but unable to reset it
//
//          requestIgnored - if this device's address is not "address"
//
// ******************************************************************
{
    // Some [broken] devices panic if you try to clear the HALT feature on EP0.
    // Since it's an unnecessary operation in any case, we'll translate it to a NOP.
    if (pipeIndex == 0)
        return requestOK;

    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction(tier %d)::ResetPipe - address = %d, pipe = %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex) );

    HCD_REQUEST_STATUS status = requestIgnored;

    // don't have to enter critical section until after checking
    // m_address (since m_address is const). That ensures we
    // won't wait for the critical section unless we actually
    // have something to do
    if ( address == m_address ) {
        status = requestFailed;
        EnterCriticalSection( &m_csDeviceLock );
        if (m_fIsSuspend ) {
            RETAILMSG(1,(TEXT("!!!CFunction( tier %d)::ResetPipe on suspend state is illgle, address = %d, pipe = %d\n"), m_tierNumber, address, pipeIndex));
            ASSERT(FALSE);
        }
        else
        if ( pipeIndex < m_maxNumPipes && m_ppCPipe[ pipeIndex ] != NULL ) {
            m_ppCPipe[ pipeIndex ]->ResetEndPoint();
            m_ppCPipe[ pipeIndex ]->ClearHaltedFlag();

            if(SetOrClearFeature( USB_ENDPOINT_RECIPIENT,
                                  m_ppCPipe[ pipeIndex ]->m_bEndpointAddress,
                                  USB_REQUEST_CLEAR_FEATURE,
                                  USB_FEATURE_ENDPOINT_HALT ))
                status = requestOK;
        }
        LeaveCriticalSection( &m_csDeviceLock );
    }

    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: -CFunction(tier %d)::ResetPipe - address = %d, pipe = %d, returing HCD_REQUEST_STATUS %d\n"),GetControllerName(), m_tierNumber, address, pipeIndex, status) );
    return status;
}

// ******************************************************************
void CFunction::HandleDetach( void )
//
// Purpose: This function is called when the device is to be detached.
//          For CFunction devices, we need to notify USBD
//
// Parameters: Nothing
//
// Returns: Nothing
//
// Notes: This procedure needs to try and exit as fast as possible,
//        so we try to create a worker thread to do all of the
//        processing in a separate static thread. This allows the
//        hub which called HandleDetach to delete this CFunction
//        class without having to wait for detach to fully complete.
// ******************************************************************
{
    DEBUGMSG( ZONE_ATTACH || ZONE_FUNCTION, (TEXT("%s: +CFunction(tier %d)::HandleDetach\n"),GetControllerName(), m_tierNumber) );

    EnterCriticalSection( &m_csDeviceLock );

    PREFAST_DEBUGCHK( m_ppCPipe != NULL );
    DEBUGCHK( m_maxNumPipes > 0 &&
              m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] != NULL );

    for ( UCHAR pipe = 0; pipe < m_maxNumPipes; pipe++ ) {
        if ( m_ppCPipe[ pipe ] != NULL ) {
            DEBUGMSG(ZONE_ATTACH, (TEXT("HandleDetach close pipe %d\r\n"), pipe));
            m_ppCPipe[ pipe ]->ClosePipe();
            delete m_ppCPipe[ pipe ];
            m_ppCPipe[ pipe ] = NULL;
        }
    }
    // m_ppCPipe[] freed in ~CDevice

    DEBUGCHK( m_pCHcd->GetpUSBDDetachProc() != NULL );
    DEBUGCHK( m_lpvDetachId != NULL );

    if(m_hFunctionFeatureEvent) {
        SetEvent(m_hFunctionFeatureEvent);
        CloseHandle(m_hFunctionFeatureEvent);
    }
    m_hFunctionFeatureEvent = NULL;

    LPVOID lpvDetachId = m_lpvDetachId;
    m_lpvDetachId = NULL;

    LeaveCriticalSection( &m_csDeviceLock );

    // Inform USBD of device detach, so the message can be passed
    // along to the client driver.
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    __try {
        (*m_pCHcd->GetpUSBDDetachProc())(lpvDetachId);
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
    }
#pragma prefast(pop)

    DEBUGMSG( ZONE_ATTACH || ZONE_FUNCTION, (TEXT("%s: -CFunction(tier %d)::HandleDetach\n"),GetControllerName(), m_tierNumber) );
}

// ******************************************************************
BOOL CFunction::SetOrClearFeature( IN const UCHAR recipient,
                                   IN const WORD  wIndex,
                                   IN const UCHAR setOrClearFeature,
                                   IN const USHORT feature )
//
// Purpose: This function will set or clear a feature for the giver recipient
//          (device, endpoint, or interface).
//
// Parameters: recipient - 0 for device, 1 for interface, 2 for endpoint
//
//         wIndex - recepient number (0 for device)
//
//             setOrClearFeature - this is USB_REQUEST_SET_FEATURE or
//                                 USB_REQUEST_CLEAR_FEATURE
//
//             feature - one of the features to set/clear
//
// Returns: TRUE if the request succeeded, else FALSE
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_FUNCTION && ZONE_VERBOSE, (TEXT("%s: +CFunction::SetOrClearFeature - recipient = %d, wIndex = %d, set/clear = 0x%x, feature = 0x%x\n"),GetControllerName(), recipient, wIndex, setOrClearFeature, feature) );

#ifdef DEBUG
    {
    switch(recipient) {
            case USB_DEVICE_RECIPIENT:
                // USB spec 1.1, 9.4 - there is only one
                // features which should be set or cleared for ports
                DEBUGCHK( wIndex == 0 && (feature == USB_DEVICE_REMOTE_WAKEUP || 
                    feature == USB_FEATURE_B_HNP_ENABLE || feature == USB_FEATURE_A_HNP_SUPPORT || feature == USB_FEATURE_A_ALT_HNP_SUPPORT) );
                break;
            case USB_ENDPOINT_RECIPIENT:
                // USB spec 1.1, 9.4 - there is only one
                // features which should be set or cleared for endpoints
                DEBUGCHK( feature == USB_FEATURE_ENDPOINT_HALT );
                break;
            case USB_INTERFACE_RECIPIENT:
                // USB spec 1.1, 9.4 - there are not features available at this time
                DEBUGCHK( 0);
                break;
            default:
                DEBUGCHK(0);
    }
        DEBUGCHK( m_ppCPipe != NULL && m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ] != NULL);
    }
#endif // DEBUG

    BOOL                fTransferDone = FALSE;
    DWORD               dwBytesTransferred = 0;
    DWORD               dwErrorFlags = USB_NOT_COMPLETE_ERROR;
    HCD_REQUEST_STATUS  status = requestFailed;
    USB_DEVICE_REQUEST  usbRequest;

    usbRequest.bmRequestType = recipient;
    usbRequest.bRequest = setOrClearFeature;
    usbRequest.wValue = feature;
    usbRequest.wIndex = wIndex;
    usbRequest.wLength = 0;
    PREFAST_DEBUGCHK(m_ppCPipe!=NULL);
    EnterCriticalSection( &m_csDeviceLock );
    if (m_fIsSuspend ) {
        RETAILMSG(1,(TEXT("!!!CFunction( tier %d)::SetOrClearFeature on suspend state is illgle, address = %d\n"), m_tierNumber, m_address));
        ASSERT(FALSE);
    }
    else {
        status = m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->IssueTransfer(
                         m_address, // address of this function
                         TransferDoneCallbackSetEvent, // callback func
                         m_hFunctionFeatureEvent, // callback param
                         USB_OUT_TRANSFER | USB_SEND_TO_DEVICE, // transfer flags
                         &usbRequest, // control request
                         0, // dwStartingFrame (not used)
                         0, // dwFrames (not used)
                         NULL, // aLengths (not used)
                         0, // buffer size
                         NULL, // buffer
                         0, // phys addr of buffer (not used)
                         this, // cancel id
                         NULL, // adwIsochErrors (not used)
                         NULL, // adwIsochLengths (not used)
                         &fTransferDone, // OUT param for transfer
                         &dwBytesTransferred, // OUT param for transfer
                         &dwErrorFlags ); // OUT param for transfer
        if ( status == requestOK ) {
            DWORD dwReturn = WaitForSingleObject( m_hFunctionFeatureEvent, STANDARD_REQUEST_TIMEOUT );
            DEBUGCHK( fTransferDone );
            if (!fTransferDone || dwReturn!= WAIT_OBJECT_0) {
#ifdef DEBUG
                HCD_REQUEST_STATUS abortStatus = m_ppCPipe[ ENDPOINT0_CONTROL_PIPE ]->AbortTransfer(NULL, NULL, this );
                ASSERT(abortStatus == requestOK);
#endif
                if (m_hFunctionFeatureEvent)
                    ResetEvent(m_hFunctionFeatureEvent);
            }
        }
    }
    LeaveCriticalSection( &m_csDeviceLock );

    BOOL fSuccess = (status == requestOK &&
                     fTransferDone &&
                     dwBytesTransferred == 0 &&
                     dwErrorFlags == USB_NO_ERROR);

    DEBUGMSG( ZONE_ERROR && !fSuccess, (TEXT("%s: CFunction::SetOrClearFeature - recipient = %d, wIndex = %d, set/clear = 0x%x, feature = 0x%x, FAILED\n"),GetControllerName(), recipient, wIndex, setOrClearFeature, feature ) );
    DEBUGMSG( ZONE_HUB && ZONE_VERBOSE, (TEXT("%s: -CFunction::SetOrClearFeature - recipient = %d, wIndex = %d, set/clear = 0x%x, feature = 0x%x, returing BOOL %d\n"),GetControllerName(), recipient, wIndex, setOrClearFeature, feature, fSuccess) );
    return fSuccess;
}
