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
#include "ceshpriv.h"

HINSTANCE HINST_CESHELL = NULL;

DBGPARAM dpCurSettings = { L"ceshell",
   { L"Fatal",   L"Warning",  L"Verbose", L"Info",
     L"Trace4",  L"Trace5",   L"Trace6",  L"Trace7",
     L"Trace8",  L"Trace9",   L"TraceA",  L"TraceB",
     L"TraceC",  L"TraceD",   L"TraceE",  L"TraceF" },
   (1 << ZONE_FATAL) };


BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved)
{
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
         HINST_CESHELL = (HINSTANCE) hInst;
      break;

      case DLL_PROCESS_DETACH:
      break;
   }

   return TRUE;
}

STDAPI DllCanUnloadNow()
{
   return S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
   return CLASS_E_CLASSNOTAVAILABLE;
}

