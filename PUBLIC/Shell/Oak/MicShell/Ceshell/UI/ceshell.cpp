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
#include "ceshhpc.h"
#include "filechangemgr.h"
#include "idlist.h"
#include "recbin.h"
#include "usersettings.h"
#include "..\api\iconcache.h"
#include "..\api\drag.h"
#include "..\ui\drivesfolder.h"

HINSTANCE HINST_CESHELL = NULL;
HMODULE HMODULE_CESHELL_DATAFILE = NULL;
IMalloc * g_pShellMalloc = NULL;
FileChangeManager * g_pFileChangeManager = NULL;
CUndo * g_pUndo = NULL;
CRecycleBin * g_pRecBin = NULL;
extern CDropTargetList* g_pDropTargetList;
NameSpacePidlCache * g_pNameSpacePidlCache = NULL;

DBGPARAM dpCurSettings = { L"ceshell",
   { L"Fatal",   L"Warning",  L"Verbose", L"Info",
     L"Trace4",  L"Trace5",   L"Trace6",  L"Trace7",
     L"Trace8",  L"Trace9",   L"TraceA",  L"TraceB",
     L"TraceC",  L"TraceD",   L"TraceE",  L"TraceF" },
   (1 << ZONE_FATAL) };

STDAPI_(VOID) DestroyShellObjects()
{
   if (g_pFileChangeManager)
   {
      delete g_pFileChangeManager;
      g_pFileChangeManager = NULL;
   }

   if (g_pRecBin)
   {
      g_pRecBin->GlobalCleanup();
      g_pRecBin->Release();
      g_pRecBin = NULL;
   }

   if(CIconCache::pIconCache)
   {
      delete CIconCache::pIconCache;
      CIconCache::pIconCache = NULL;
   }

   if(g_pDropTargetList)
   {
      delete g_pDropTargetList;
      g_pDropTargetList = NULL;
   }

   if (NULL != g_pNameSpacePidlCache)
   {
      delete g_pNameSpacePidlCache;
      g_pNameSpacePidlCache = NULL;
   }

   if (NULL != CDrivesFolder::m_psfDrivesDirectory)
   {
      // This should have only been created once by the CDrivesFolder constructor
      CDrivesFolder::m_psfDrivesDirectory->Release();
      CDrivesFolder::m_psfDrivesDirectory = NULL;
   }

   if (g_pShellMalloc)
   {
      g_pShellMalloc->Release();
      g_pShellMalloc = NULL;
   }

   if (NULL != HMODULE_CESHELL_DATAFILE)
   {
      FreeLibrary(HMODULE_CESHELL_DATAFILE);
      HMODULE_CESHELL_DATAFILE = NULL;
   }

   UnInitializeAygUtils();
}

STDAPI_(BOOL) InitializeShellObjects()
{
   BOOL fReturn = TRUE;

   if (SUCCEEDED(SHGetMalloc(&g_pShellMalloc)))
   {
      g_pFileChangeManager = new FileChangeManager();
      if (!g_pFileChangeManager)
         fReturn = FALSE;

      if (fReturn)
      {
         if (TEXT('\0') == CRecycleBin::m_szBitBucketPath[0])
         {
            // Only once to init the Recycle Bin
            g_pRecBin = new CRecycleBin();
            if (g_pRecBin)
            {
               g_pRecBin->GlobalInit();
               UserSettings::Query();
            }
            else
            {
               DestroyShellObjects();
               fReturn = FALSE;
            }
         }
      }
      else
      {
         DestroyShellObjects();
      }
   }
   else
   {
      fReturn = FALSE;
   }

   if (fReturn)
   {
      // If this fails we just won't use the cache
      g_pNameSpacePidlCache = new NameSpacePidlCache();

      // Cache ceshell as a datafile so that the mui_sz loads will be faster
      HMODULE_CESHELL_DATAFILE = LoadLibraryEx(TEXT("ceshell.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
   }

   InitializeAygUtils();

   return fReturn;
}

BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved)
{
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
         HINST_CESHELL = (HINSTANCE) hInst;
         return InitializeShellObjects();
      break;

      case DLL_PROCESS_DETACH:
         DestroyShellObjects();
      break;
   }

   return TRUE;
}

