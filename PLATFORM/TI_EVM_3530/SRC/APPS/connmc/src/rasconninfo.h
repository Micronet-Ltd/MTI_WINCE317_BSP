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
#ifndef RAS_CONN_INFO_H__
#define RAS_CONN_INFO_H__

#include "conninfo.h"
#include "resource.h"

#include <ras.h>
#include <raserror.h>
#include <autoras.h>
#include <eap.h>

#define OPTION_DIRECTION_NORMAL  0
#define OPTION_DIRECTION_REVERSE 1

typedef struct _RASOPTIONMAP {
    DWORD dwfOptionBit;
    DWORD dwControlId;
    DWORD dwDirection;
} RASOPTIONMAP, *PRASOPTIONMAP;

class RasConnInfo : public ConnInfo
{
public:
   RASENTRY RasEntry;

private:
   // Array containing information about EAP extensions in the system
   PEAP_EXTENSION_INFO m_pEapExtInfo;
   DWORD               m_cEapExtInfo; // Count of number of elements in the array

   // EAP connection data for the currently selected EAP authentication type
   // (NULL/0 if no EAP type is currently selected)
   PBYTE               m_pEapConnData;
   DWORD               m_cbEapConnData;

public:
   DWORD    EapEnumExtensions();
   void     ClearEapConnData();
   DWORD    GetEapConnData();
   DWORD    SetEapConnData();
   DWORD    ChangeEapConnData(PVOID pNewConnData, DWORD cbNewConnData);

protected:
   PBYTE pDevConfig;
   DWORD cbDevConfig;

   // Used to enum ras devices
   static LPRASENTRYNAME pREN;
   static DWORD dwRENCount;
   static DWORD dwRENCurrent;
   static const TCHAR szBadRasCharList[];

   // Internal helper methods
   HRASCONN getRasConnHandle() const;
   void writeChanges();
   void handleDevConfig(HWND hDlg);
   static DWORD WINAPI DevConfigThread(LPVOID pVoid);

   // Ras helper methods
   static DWORD GetDevices(LPRASDEVINFO * pRasDevInfo);
   static LPLINETRANSLATECAPS GetTranslateCaps(HINSTANCE hInstance);
   static void CALLBACK NullLineCallbackFunc(DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
                                             DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
   static DWORD GetDefaultCntryRgnCode(LPLINETRANSLATECAPS pCaps);
   static void GetDefaultAreaCode(LPLINETRANSLATECAPS pCaps, TCHAR * pszAreaCode, DWORD dwMaxLen);
   static BOOL isValidRasInfoString(TCHAR * pszValidate);

   // TCP/IP Configuration prop sheet
   BOOL ConfigureTcpIp(HWND hParent, HINSTANCE hInstance);
   static BOOL CALLBACK TCPDlgProc0(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
   static BOOL CALLBACK TCPDlgProc1(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

   // Used to enum ras devices
   static RasConnInfo * RasFactory(TCHAR * pszEntryName);

   // Security Settings prop sheet
   BOOL ConfigureSecurity(HWND hParent, HINSTANCE hInstance);
   static BOOL CALLBACK SecurityDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
   static void SetCheckRasOptions(HWND hDlg, RasConnInfo * pRasConn, PRASOPTIONMAP pMap);
   static void UpdateRasOptions(HWND hDlg, RasConnInfo * pRasConn, PRASOPTIONMAP pMap);
   BOOL InitEAPControls(HWND hDlg, RasConnInfo * pRasConn);
   void SaveEAPConnectionData(HWND hDlg, RasConnInfo * pRasConn);

public:
   RasConnInfo(TCHAR * pszName = NULL);
   ~RasConnInfo();

   // Used to enum ras devices
   static RasConnInfo * FindFirst();
   static RasConnInfo * FindNext();

   virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("RasConnInfo")); }
   virtual BOOL isValid(UINT * pError) const;
   static BOOL HasDevices(TCHAR * pszDeviceType, TCHAR *pszDeviceNamePrefix = NULL);

   static BOOL GetUnusedRasName(HINSTANCE hInstance, TCHAR * pszName);

   virtual void getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType, DWORD cchType) const;

   // ConnInfo.h
   virtual BOOL getDefaultActionString(HINSTANCE hInstance, TCHAR * pszDefaultAction, UINT * pError);
   virtual BOOL defaultAction(HINSTANCE hInstance, HWND hParent, UINT * pError);

   virtual BOOL allowRemove(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual BOOL remove(UINT * pError);

   virtual BOOL allowRename(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual BOOL rename(TCHAR * pszNewName, UINT * pError);

   virtual BOOL allowCopy(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual BOOL copy(HINSTANCE hInstance, UINT * pError);

   virtual BOOL allowShortcut(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual BOOL shortcut(HINSTANCE hInstance, UINT * pError);

   virtual BOOL allowSetAsDefault(BOOL fInGroup, UINT * pReason) const { return !fInGroup; }
   virtual BOOL setAsDefault(BOOL fSet, UINT * pError);
   virtual BOOL isSetAsDefault();

   virtual BOOL getDisplayInfo(UINT col, HINSTANCE hInstance, TCHAR * pszInfo,
                               DWORD cchInfo, UINT * pError) const;

private:
   static BOOL CALLBACK FindRNAAppWindow(HWND hWnd, LPARAM lParam);
};

#endif // RAS_CONN_INFO_H__
