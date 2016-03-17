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
//     cMhcd.h
// 
// Abstract:  CMhcd implements the HCDI interface. It mostly
//            just passes requests on to other objects, which
//            do the real work.
//     
// Notes: 
//
#ifndef __CMHCD_H_
#define __CMHCD_H_
class CMhcd;
#include "Chw.h"
#include <CDevice.hpp>
#include "CPipe.h"
#include "usb2lib.h"


// this class gets passed into the CUhcd Initialize routine
class CPhysMem;
// this class is our access point to all USB devices
//class CRootHub;

class CMhcd :public CHW, public  USB2lib
{
public:
    // ****************************************************
    // Public Functions for CUhcd
    // ****************************************************
    CMhcd( IN LPVOID pvUhcdPddObject,
                     IN CPhysMem * pCPhysMem,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwSysIntr);

    ~CMhcd();

    // These functions are called by the HCDI interface
    virtual BOOL DeviceInitialize();
    virtual void DeviceDeInitialize( void );
   
    virtual BOOL    AddedTt( UCHAR uHubAddress,UCHAR uPort) { return USB2lib::AddedTt( uHubAddress,uPort); };
    virtual BOOL    DeleteTt( UCHAR uHubAddress,UCHAR uPort,BOOL ttContext) { return USB2lib::DeleteTt( uHubAddress,uPort,ttContext); };
    
    virtual BOOL    SignalHubChangeEvent(BOOL fConnect);    
    virtual void    SetSignalDisconnectACK(BOOL fAck) { CHW::SetSignalDisconnectACK(fAck);};
    
    //public variables
    CPhysMem * const m_pMem;

    DWORD           m_dwSysIntr;
    REGISTER        m_portBase;
    LPCWSTR         m_lpRegPath;
};

#endif