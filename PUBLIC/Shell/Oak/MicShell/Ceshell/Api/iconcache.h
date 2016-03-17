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
#include "list.hpp"
#include "resource.h"

#pragma once

enum { DOCUMENT_ICON = 0,
       FOLDER_ICON = 1,
       APPLICATION_ICON = 2,
       STORAGECARD_ICON = 3,
       NETWORKFOLDER_ICON = 4,
       SHORTCUT_ICON = 5
     };

class CIconCache
{
protected:
   struct CacheInfo : LISTOBJ
   {
      LPTSTR m_pszItem;
      int m_iImageIndex;
      int m_iImageAltIndex;
      BOOL m_fFreeType;
      LPTSTR m_pszType;

      CacheInfo();
      ~CacheInfo();
   };

   static const DWORD HIBERNATE_FREEZE_CACHE;
   static const DWORD LCID_ENGLISH;

   HIMAGELIST m_himlSmall;
   HIMAGELIST m_himlLarge;
   CList m_list;

   CacheInfo * CacheItem(LPCTSTR pszFilePath);
   int GetDefaultImageIndex(LPCTSTR pszFilePath, DWORD dwAttrib) const;
   CacheInfo * GetItem(LPCTSTR pszFilePath);
   BOOL IExtractIcon(LPTSTR pszIcon, HICON * phiconSM, HICON * phiconLG) const;

public:
   CIconCache();
   ~CIconCache();

   static CIconCache * pIconCache;

   int GetImageIndex(LPCTSTR pszFilePath, DWORD dwAttrib, BOOL fGetAltImageIndex);
   HIMAGELIST GetSystemImageList(UINT uFlags) const;
   BOOL GetType(LPCTSTR pszFilePath, DWORD dwAttrib, LPTSTR pszType, size_t cchType);
   BOOL RebuildSystemImageList();
};

