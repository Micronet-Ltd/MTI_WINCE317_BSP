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
//     cmhcd.cpp
// Abstract:
//     This file contains the CMhcd object, which is the main entry
//     point for all HCDI calls by USBD
//
// Notes:
//

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include <windows.h>
#include "cmhcd.h"
#include "cpipe.h"
#pragma warning(pop)

// ****************************************************************
// PUBLIC FUNCTIONS
// ****************************************************************

// ******************************************************************
CMhcd::CMhcd( IN LPVOID pvUhcdPddObject,
                        IN CPhysMem * pCPhysMem,
                        IN LPCWSTR lpRegPath, // szDriverRegistryKey ignored for now
                        IN REGISTER portBase,
                        IN DWORD dwSysIntr)
:CHW( pCPhysMem, pvUhcdPddObject, dwSysIntr)
,m_pMem(pCPhysMem)
// Purpose: Initialize variables associated with this class
//
// Parameters: None
//
// Returns: Nothing (Constructor can NOT fail!)g208
//
// Notes: *All* initialization which could possibly fail should be done
//        via the Initialize() routine, which is called right after
//        the constructor
// ******************************************************************
{
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("+CEhcd::CEhcd\n")));
    m_dwSysIntr = dwSysIntr;
    m_portBase = portBase;
    m_lpRegPath = lpRegPath;
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("-CEhcd::CEhcd\n")));
}

// ******************************************************************
CMhcd::~CMhcd()
//
// Purpose: Destroy all memory and objects associated with this class
//
// Parameters: None
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("+CMhcd::~CMhcd\n")));

    // make the API set inaccessible
    CRootHub *pRoot = SetRootHub(NULL);
    // signal root hub to close
    if ( pRoot ) {
        pRoot->HandleDetach();
        delete pRoot;
    }

    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("-CMhcd::~CMhcd\n")));
}
void CMhcd::DeviceDeInitialize( void )
{
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("+CEhcd::DeInitialize\n")));

    // make the API set inaccessible
    CRootHub *pRoot = SetRootHub(NULL);
    // signal root hub to close
    if ( pRoot ) {
        pRoot->HandleDetach();
        delete pRoot;
    }
    // this is safe because by now all clients have been unloaded
    //DeleteCriticalSection ( &m_csHCLock );

    CHW::DeInitialize();
    CDeviceGlobal::DeInitialize();

    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("-CEhcd::DeInitialize\n")));

}

// ******************************************************************
BOOL CMhcd::DeviceInitialize()
//
// Purpose: Set up the Host Controller hardware, associated data structures,
//          and threads so that schedule processing can begin.
//
// Parameters: pvUhcdPddObject - pointer to the PDD object for this driver
//
//             pCPhysMem - pointer to class for managing physical memory
//
//             szDriverRegistryKey - unused ?
//
//             portBase - base address for USB registers
//
//             dwSysIntr - interrupt identifier for USB interrupts
//
// Returns: TRUE - if initializes successfully and is ready to process
//                 the schedule
//          FALSE - if setup fails
//
// Notes: This function is called by right after the constructor.
//        It is the starting point for all initialization.
//
//        This needs to be implemented for HCDI
// ******************************************************************
{
    DEBUGMSG(ZONE_INIT,(TEXT("+CEhcd::Initialize. Entry\r\n")));

    
    // All Initialize routines must be called, so we can't write
    // if ( !CDevice::Initialize() || !CPipe::Initialize() etc )
    // due to short circuit eval.
    {
        m_pMem ->ReInit();
        BOOL fCDeviceInitOK = CDeviceGlobal::Initialize(this);
        BOOL fCHWInitOK = CHW::Initialize( );

        if ( !fCDeviceInitOK || !fCHWInitOK ) {
            DEBUGMSG(ZONE_ERROR, (TEXT("-CEhcd::Initialize. Error - could not initialize device/pipe/hw classes\n")));
            ASSERT(FALSE);
            return FALSE;
        }
    }

    // set up the root hub object
    {
        USB_DEVICE_INFO deviceInfo;
        USB_HUB_DESCRIPTOR usbHubDescriptor;

        deviceInfo.dwCount = sizeof( USB_DEVICE_INFO );
        deviceInfo.lpConfigs = NULL;
        deviceInfo.lpActiveConfig = NULL;
        deviceInfo.Descriptor.bLength = sizeof( USB_DEVICE_DESCRIPTOR );
        deviceInfo.Descriptor.bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE;
        deviceInfo.Descriptor.bcdUSB = 0x200; // USB spec 200
        deviceInfo.Descriptor.bDeviceClass = USB_DEVICE_CLASS_HUB;
        deviceInfo.Descriptor.bDeviceSubClass = 0xff;
        deviceInfo.Descriptor.bDeviceProtocol = 0xff;
        deviceInfo.Descriptor.bMaxPacketSize0 = 0;
        deviceInfo.Descriptor.bNumConfigurations = 0;

        usbHubDescriptor.bDescriptorType = USB_HUB_DESCRIPTOR_TYPE;
        usbHubDescriptor.bDescriptorLength = USB_HUB_DESCRIPTOR_MINIMUM_SIZE;
        usbHubDescriptor.bNumberOfPorts = (BYTE)GetNumberOfPort();
        usbHubDescriptor.wHubCharacteristics = USB_HUB_CHARACTERISTIC_NO_POWER_SWITCHING |
                                               USB_HUB_CHARACTERISTIC_NOT_PART_OF_COMPOUND_DEVICE |
                                               USB_HUB_CHARACTERISTIC_NO_OVER_CURRENT_PROTECTION;
        usbHubDescriptor.bPowerOnToPowerGood = 0;
        usbHubDescriptor.bHubControlCurrent = 0;
        DEBUGCHK( usbHubDescriptor.bNumberOfPorts <= 8 );
        usbHubDescriptor.bRemoveAndPowerMask[0] = 0; // all devices on hub are removable
        usbHubDescriptor.bRemoveAndPowerMask[1] = 0xFF; // must be 0xFF, USB spec 1.1, table 11-8

        // FALSE indicates root hub is not low speed
        // (though, this is ignored for hubs anyway)
        SetRootHub( new CRootHub( deviceInfo, FALSE,TRUE, usbHubDescriptor,this ));
    }
    if ( !GetRootHub() ) {
        DEBUGMSG( ZONE_ERROR, (TEXT("-CMhcd::Initialize - unable to create root hub object\n")) );
        return FALSE;
    }

    // Signal root hub to start processing port changes
    // The root hub doesn't have any pipes, so we pass NULL as the
    // endpoint0 pipe
    if ( !GetRootHub()->EnterOperationalState( NULL ) ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CEhcd::Initialize. Error initializing root hub\n")));
        return FALSE;
    }
    // Start processing frames
    CHW::EnterOperationalState();

    DEBUGMSG(ZONE_INIT,(TEXT("-CMhcd::Initialize. Success!!\r\n")));    
    return TRUE;
}
BOOL CMhcd::SignalHubChangeEvent(BOOL fConnect)
{    
    return CHW::SignalHubChangeEvent(fConnect); 
}

CHcd * CreateHCDObject(IN LPVOID pvUhcdPddObject,
                     IN CPhysMem * pCPhysMem,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwSysIntr)
{    
    return new CMhcd (pvUhcdPddObject, pCPhysMem,szDriverRegistryKey,portBase,dwSysIntr);
}



