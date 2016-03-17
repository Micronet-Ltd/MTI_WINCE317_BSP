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
#ifndef LAN_CONN_INFO_H__
#define LAN_CONN_INFO_H__

#include "conninfo.h"
#include "resource.h"

#include <eapol.h>
#include <iphlpapi.h>
#include <netui.h>
#include <pm.h>
#include <winsock.h>
#include <wzcsapi.h>
#include <ndispwr.h>

class LanConnInfo : public ConnInfo
{
protected:
   static const TCHAR szNameMappingRegistryPath[];
   static const TCHAR szWirelessCacheRegistryPath[];

   static UINT enabledLanIconID;
   static UINT disabledLanIconID;
   static UINT enabledWLanIconID;
   static UINT disabledWLanIconID;

   // These are used to enum the adapters in the system
   // pAdapterInfo is refreshed by a call to FindFirst
   static TCHAR szAdapterNames[512];
   static TCHAR * pCurrentAdapter;

   TCHAR * pszAdapter;
   BOOL fEnabled;
   BOOL fSupportsPowerManagment;
   BOOL fWireless;

   DWORD dwAddress;
   DWORD dwGatewayAddress;

   static BOOL DoNdisIOControl(DWORD dwCommand, LPVOID pInBuffer,
                               DWORD cbInBuffer, LPVOID pOutBuffer,
                               DWORD * pcbOutBuffer);

   static BOOL IsValidDx(CEDEVICE_POWER_STATE Dx) { return (PwrDeviceUnspecified < Dx &&
                                                            PwrDeviceMaximum > Dx); }

   BOOL isWireless() const;
   BOOL readFriendlyName();
   BOOL writeFriendlyName() const;

   static DWORD GetIfAddressFromIfIndex(DWORD dwIndex);
   BOOL getDefaultGatewayRoute(MIB_IPFORWARDROW & ipRoute) const;

public:
   static const DWORD dwMaxFriendlyNameLength;

   LanConnInfo(TCHAR * pszName);
   virtual ~LanConnInfo();

   virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("LanConnInfo")); }
   virtual BOOL isValid(UINT * pError) const;

   static void setEnabledLanIconID(UINT iconID) { LanConnInfo::enabledLanIconID = iconID; }
   static void setDisabledLanIconID(UINT iconID) { LanConnInfo::disabledLanIconID = iconID; }
   static void setEnabledWLanIconID(UINT iconID) { LanConnInfo::enabledWLanIconID = iconID; }
   static void setDisabledWLanIconID(UINT iconID) { LanConnInfo::disabledWLanIconID = iconID; }
   static DWORD IsRasVEMAdapter(IN const WCHAR *wszAdapterName, OUT BOOL *bIsVEMAdapter);

   virtual UINT getIconID() const;

   // Allows enuming of the adapters
   static LanConnInfo * FindFirst();
   static LanConnInfo * FindNext();

   // Used by AdapterOrder to order the LAN connections
   DWORD getDefaultGatewayRouteMetric() const;
   BOOL setDefaultGatewayRouteMetric(DWORD dwRoute);

   // ConnInfo.h
   virtual BOOL getDefaultActionString(HINSTANCE hInstance, TCHAR * pszDefaultAction, UINT * pError);
   virtual BOOL defaultAction(HINSTANCE hInstance, HWND hParent, UINT * pError);

   virtual BOOL allowRename(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual BOOL rename(TCHAR * pszNewName, UINT * pError);

   virtual BOOL getDisplayInfo(UINT col, HINSTANCE hInstance, TCHAR * pszInfo,
                               DWORD cchInfo, UINT * pError) const;

   virtual DWORD getPhysicalMedium() const;

   virtual BOOL allowProperties(BOOL fInGroup, UINT * pError) const { return TRUE; }
   virtual BOOL showProperties(HINSTANCE hInstance, HWND hParent, UINT * pError);
};

#endif // LAN_CONN_INFO_H__
