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
#include <windows.h>
#include <btagpub.h>

/*

This file is here for compatibility with Windows Mobile.  On Windows Mobile, this lib can be implemented
to allow the profile to be included on the Windows Mobile platform.  On WinCE, we will define some types
(e.g. IBTDevice) since it is specific to Windows Mobile.

*/


class IBTDevice {
};

typedef BOOL (*PFN_PlatformPrompt) (HWND hwndParent, const TCHAR* pszDeviceName);


extern "C" void OnBthDeviceAdded(const BT_ADDR* pBA, const GUID* pServiceGUID, LPVOID pContext)
{
	//
	// This routine should be implemented to run this profile on
	// Windows Mobile devices for hooking into the BT control
	// panel.
	//

	RETAILMSG(1, (L"BTAGSVC:%S\r\n",__FUNCTION__));
}

extern "C" void OnBthDeviceRemoved(const BT_ADDR* pBA)
{
	//
	// This routine should be implemented to run this profile on
	// Windows Mobile devices for hooking into the BT control
	// panel.
	//
}


extern "C" BOOL BthAGSetAsDefaultHandsfree(BT_ADDR* pBA, const GUID* pServiceGUID)
{            
	//
	// This routine should be implemented to run this profile on
	// Windows Mobile devices for hooking into the BT control
	// panel.
	//
	
	return TRUE;
}

extern "C" HRESULT BthAGPromptToSetAsHandsfreeDevice(HWND hwndParent, IBTDevice* pDevice, PFN_PlatformPrompt pfnPrompt)
{
	//
	// This routine should be implemented to run this profile on
	// Windows Mobile devices for hooking into the BT control
	// panel.
	//

	return S_OK;
}


extern "C" HRESULT BthAGFindDeviceGUID(IBTDevice* pDevice, GUID* pGuid, BT_ADDR* pAddr)
{
	//
	// This routine should be implemented to run this profile on
	// Windows Mobile devices for hooking into the BT control
	// panel.
	//

	return S_OK;
}



