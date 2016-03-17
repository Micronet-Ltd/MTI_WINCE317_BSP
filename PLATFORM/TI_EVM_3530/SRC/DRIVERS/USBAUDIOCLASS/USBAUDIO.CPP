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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// Module Name: 
//      usbaudio.cpp
//
// Abstract:
//      USB Client Driver for Audio Device Class.
//

#include "usbaudio.hpp"


extern "C" BOOL USBInstallDriver(LPCWSTR szDriverLibFile)
{
    HKEY  hKey = NULL;
    BOOL  bRc;

    const WCHAR wsUsbDeviceID[] = CLASS_NAME_SZ;
    WCHAR wsSubClassRegKey[sizeof(CLIENT_REGKEY_SZ)+16] = CLIENT_REGKEY_SZ;

    USB_DRIVER_SETTINGS usbDriverSettings = { DRIVER_SETTINGS };

    DEBUGMSG(ZONE_USB_INIT, (TEXT(">USBInstallDriver(%s)\n"), szDriverLibFile));
    TEST_TRAP();
    
    //
    // register with USBD
    //   
    bRc = RegisterClientDriverID(wsUsbDeviceID);
    if (!bRc) 
    {
      DEBUGMSG( ZONE_ERR, (TEXT("RegisterClientDriverID error:%d\n"), GetLastError()));
      return FALSE;
    }
        
    bRc = RegisterClientSettings(szDriverLibFile,
                                wsUsbDeviceID, 
                                NULL, 
                                &usbDriverSettings);
    if (!bRc) 
    {
      DEBUGMSG( ZONE_ERR, (TEXT("RegisterClientSettings error:%d\n"), GetLastError()));
      return FALSE;
    }
 
    DEBUGMSG( ZONE_USB_INIT, (TEXT("<USBInstallDriver:%d\n"), bRc ));

    return bRc;
}

extern "C" BOOL USBUnInstallDriver( VOID )
{
   BOOL bRc;
   const WCHAR wsUsbDeviceID[] = CLASS_NAME_SZ;
   USB_DRIVER_SETTINGS usbDriverSettings = { DRIVER_SETTINGS };

   DEBUGMSG( ZONE_USB_INIT, (TEXT(">USBUnInstallDriver\n")));

   bRc = UnRegisterClientSettings(wsUsbDeviceID,
                                  NULL,
                                  &usbDriverSettings);

   bRc = bRc & UnRegisterClientDriverID(wsUsbDeviceID);

   DEBUGMSG(ZONE_USB_INIT, (TEXT("<USBUnInstallDriver:%d\n"), bRc));

   return bRc;
}
