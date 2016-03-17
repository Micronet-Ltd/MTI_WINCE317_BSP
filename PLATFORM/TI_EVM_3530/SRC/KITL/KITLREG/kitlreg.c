/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//
//  File:  kitlreg.c
//

//-------------------------------------------------------------------------------

#include <bsp.h>
#include <devload.h>

//------------------------------------------------------------------------------
//  Local definition

#ifndef HKEY_LOCAL_MACHINE
#define HKEY_LOCAL_MACHINE          ((HKEY)(ULONG_PTR)0x80000002)
#endif

//------------------------------------------------------------------------------
static int SetDeviceDriverFlags(LPCWSTR szKeyPath, DWORD flags)
{
    LONG code;
    HKEY hKey;
    UINT32 value;

    // Open/create key
    code = NKRegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL, 0, 0, NULL, &hKey, &value);          

    if(code != ERROR_SUCCESS)
		return 0;

    // Set value
    code = NKRegSetValueEx(hKey, L"Flags", 0, REG_DWORD, (UCHAR*)&flags, sizeof(DWORD));

    // Close key
    NKRegCloseKey(hKey);

    return (code == ERROR_SUCCESS);
}

//------------------------------------------------------------------------------
//
void OEMEthernetDriverEnable(BOOL bEnable)
{
}

//------------------------------------------------------------------------------
//
void OEMUsbDriverEnable(BOOL bEnable)
{
    if(bEnable)
    {
        SetDeviceDriverFlags(L"Drivers\\BuiltIn\\MUsbOtg", DEVFLAGS_NONE);
        SetDeviceDriverFlags(L"Drivers\\BuiltIn\\MUsbOtg\\Hcd", DEVFLAGS_NONE);
        SetDeviceDriverFlags(L"Drivers\\BuiltIn\\MUsbOtg\\UsbFn", DEVFLAGS_NONE);
    }
    else
    {
        SetDeviceDriverFlags(L"Drivers\\BuiltIn\\MUsbOtg", DEVFLAGS_NOLOAD);
        SetDeviceDriverFlags(L"Drivers\\BuiltIn\\MUsbOtg\\Hcd", DEVFLAGS_NOLOAD);
        SetDeviceDriverFlags(L"Drivers\\BuiltIn\\MUsbOtg\\UsbFn", DEVFLAGS_NOLOAD);
    }
}
