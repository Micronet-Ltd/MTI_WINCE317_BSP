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

#include "idlist.h"

LPITEMIDLIST ILConcatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
   LPITEMIDLIST pidlCat = NULL;

   if (pidl1 && pidl2)
   {
      size_t cb1 = ILSize(pidl1, IL_ALL)-sizeof(pidl1->mkid.cb);
      size_t cb2 = ILSize(pidl2, IL_ALL)-sizeof(pidl1->mkid.cb);

      pidlCat = (LPITEMIDLIST) LocalAlloc(LPTR, cb1+cb2+sizeof(pidl1->mkid.cb));
      if (pidlCat)
      {
         memcpy(pidlCat, pidl1, cb1);
         memcpy(((LPBYTE) pidlCat)+cb1, pidl2, cb2);
      }
   }

   return pidlCat;
}

LPITEMIDLIST ILCopy(LPCITEMIDLIST pidl, UINT cbItems)
{
   LPITEMIDLIST pidlCopy = NULL;

   if (pidl)
   {
      size_t cb = ILSize(pidl, cbItems);
      pidlCopy = (LPITEMIDLIST) LocalAlloc(LPTR, cb);
      if (pidlCopy)
         memcpy(pidlCopy, pidl, cb-sizeof(pidl->mkid.cb));
   }

   return pidlCopy;
}

DWORD ILCount(LPCITEMIDLIST pidl)
{
   DWORD dw = 0;

   while (!ILIsEmpty(pidl))
   {
      dw++;
      pidl = ILNext(pidl);
   }

   return dw;
}

LPCWSTR ILDisplayName(LPCITEMIDLIST pidl)
{
   if (pidl)
      return (LPCWSTR) IL_DISPLAYNAME(pidl);
   else
      return NULL;
}

LPITEMIDLIST ILFindChild(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
   if (ILIsParent(pidlParent, pidlChild, FALSE))
   {
      while (!ILIsEmpty(pidlParent))
      {
         pidlChild = ILNext(pidlChild);
         pidlParent = ILNext(pidlParent);
      }

      return (LPITEMIDLIST) pidlChild;
   }

   return NULL;
}

LPITEMIDLIST ILFindLast(LPCITEMIDLIST pidl)
{
   LPITEMIDLIST pidlLast = (LPITEMIDLIST) pidl;
   LPITEMIDLIST pidlNext = (LPITEMIDLIST) pidl;

   while (!ILIsEmpty(pidlNext))
   {
      pidlLast = pidlNext;
      pidlNext = ILNext(pidlLast);
   }

   return pidlLast;
}

void ILFree(LPCITEMIDLIST pidl)
{
   if (pidl)
      LocalFree((void*)pidl);
}

BOOL ILGetDateCached(LPCITEMIDLIST pidl, FILETIME* ftDateOut)
{
   if (pidl && (CEIDLIST_FS_DATECACHED & IL_GET_FLAGS(pidl)))
   {
      memcpy(ftDateOut, IL_DATECACHED(pidl), sizeof(FILETIME));
      return TRUE;
   }
   return FALSE;
}
BOOL ILGetSizeCached(LPCITEMIDLIST pidl, ULARGE_INTEGER* uliSizeOut)
{
   if (pidl && (CEIDLIST_FS_SIZECACHED & IL_GET_FLAGS(pidl)))
   {
      memcpy(uliSizeOut, IL_SIZECACHED(pidl), sizeof(ULARGE_INTEGER));
      return TRUE;
   }
   return FALSE;
}

LPCWSTR ILGetTypeCached(LPCITEMIDLIST pidl)
{
   if (pidl && (CEIDLIST_FS_TYPECACHED & IL_GET_FLAGS(pidl)))
      return (LPCWSTR) IL_TYPECACHED(pidl);
   else
      return NULL;
}

BOOL ILHasDateCached(LPCITEMIDLIST pidl)
{
   if (ILIsEmpty(pidl))
      return FALSE;

   return (CEIDLIST_FS_DATECACHED & IL_GET_FLAGS(pidl));
}

BOOL ILHasSizeCached(LPCITEMIDLIST pidl)
{
   if (ILIsEmpty(pidl))
      return FALSE;

   return (CEIDLIST_FS_SIZECACHED & IL_GET_FLAGS(pidl));
}

BOOL ILHasTypeCached(LPCITEMIDLIST pidl)
{
   if (ILIsEmpty(pidl))
      return FALSE;

   return (CEIDLIST_FS_TYPECACHED & IL_GET_FLAGS(pidl));
}

BOOL ILIsEmpty(LPCITEMIDLIST pidl)
{
   if(!pidl)
      return TRUE;

   return (0 == (pidl)->mkid.cb);
}

BOOL ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
   BOOL fEqual = FALSE;
   IShellFolder * psfDesktop = NULL;

   if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
   {
      fEqual = (0 == psfDesktop->CompareIDs(0, pidl1, pidl2));
      psfDesktop->Release();
   }

   return fEqual;
}

BOOL ILIsEqualParsingNames(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
   if (ILIsEmpty(pidl1) || ILIsEmpty(pidl2))
      return FALSE;

   return (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                    IL_PARSINGNAME(pidl1), -1,
                                    IL_PARSINGNAME(pidl2), -1));
}

BOOL ILIsFileSystemPidl(LPCITEMIDLIST pidl)
{
   if (ILIsEmpty(pidl))
      return FALSE;

   return (CEIDLIST_FILESYSTEM & IL_GET_FLAGS(pidl));
}

BOOL ILIsFileSystemDirectoryPidl(LPCITEMIDLIST pidl)
{
   if (ILIsEmpty(pidl))
      return FALSE;

   WORD flags = IL_GET_FLAGS(pidl);
   return ((CEIDLIST_FILESYSTEM & flags) && (CEIDLIST_FS_DIRECTORY & flags));
}

BOOL ILIsFileSystemLinkPidl(LPCITEMIDLIST pidl)
{
   if (ILIsEmpty(pidl))
      return FALSE;

   WORD flags = IL_GET_FLAGS(pidl);
   return ((CEIDLIST_FILESYSTEM & flags) && (CEIDLIST_FS_LINK & flags));
}


BOOL ILIsNameSpacePidl(LPCITEMIDLIST pidl)
{
   if (ILIsEmpty(pidl))
      return FALSE;

   return (CEIDLIST_NAMESPACE & IL_GET_FLAGS(pidl));
}

BOOL ILIsNameSpace(LPCITEMIDLIST pidl, REFCLSID rclsid)
{
   CLSID clsid = {0};
   LPITEMIDLIST pidlLast = ILFindLast(pidl);

   if (pidlLast && ILIsNameSpacePidl(pidlLast) &&
       SUCCEEDED(CLSIDFromString(IL_PARSINGNAME(pidlLast), &clsid)) &&
       IsEqualGUID(clsid, rclsid))
   {
      return TRUE;
   }

   return FALSE;
}

BOOL ILIsParent(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild, BOOL fImmediate)
{
   LPITEMIDLIST pidlParentTemp = (LPITEMIDLIST) pidlParent;
   LPITEMIDLIST pidlChildTemp = (LPITEMIDLIST) pidlChild;

   if (!pidlParent || !pidlChild)
      return FALSE;

   while (!ILIsEmpty(pidlParentTemp))
   {
      // If pidlChild is shorter than pidlParent, pidlParent can't be its parent.
      if (ILIsEmpty(pidlChildTemp))
         return FALSE;

      pidlParentTemp = ILNext(pidlParentTemp);
      pidlChildTemp = ILNext(pidlChildTemp);
   }

   if (fImmediate)
   {
      // If fImmediate is TRUE, pidlChildTemp should contain exactly one ID.
      if (ILIsEmpty(pidlChildTemp) || !ILNext(pidlChildTemp) || !ILIsEmpty(ILNext(pidlChildTemp)))
         return FALSE;
   }

   // Create a new IDList from a portion of pidlChild, which contains the
   // same number of IDs as pidlParent.
   size_t cb = (size_t)pidlChildTemp-(size_t)pidlChild;
   pidlChildTemp = (LPITEMIDLIST) LocalAlloc(LPTR, cb+sizeof(pidlChild->mkid.cb));
   if (pidlChildTemp)
   {
      BOOL fRet;

      memcpy(pidlChildTemp, pidlChild, cb);
      fRet = ILIsEqual(pidlParent, pidlChildTemp);

      ILFree(pidlChildTemp);
      return fRet;
   }

   return FALSE;
}

LPITEMIDLIST ILNext(LPCITEMIDLIST pidl)
{
   LPITEMIDLIST pidlNext = NULL;

   if (pidl)
   {
      pidlNext = (LPITEMIDLIST) (((LPBYTE) pidl) + pidl->mkid.cb);
      if (ILIsEmpty(pidlNext))
         pidlNext = NULL;
   }

   return pidlNext;
}

LPCWSTR ILParsingName(LPCITEMIDLIST pidl)
{
   if (pidl)
      return (LPCWSTR) IL_PARSINGNAME(pidl);
   else
      return NULL;
}

BOOL ILRemoveLast(LPITEMIDLIST pidl)
{
   BOOL fRemoved = FALSE;

   LPITEMIDLIST pidlLast = ILFindLast(pidl);
   if (pidlLast)
   {
      // Remove the last one, null-terminator note that this doesn't clean up the memory
      pidlLast->mkid.cb = 0;
      fRemoved = TRUE;
   }

   return fRemoved;
}

size_t ILSize(LPCITEMIDLIST pidl, UINT cbItems)
{
   if (!pidl)
      return 0;

   size_t cb = 0;

   while (pidl && cbItems--)
   {
      cb += pidl->mkid.cb;
      pidl = ILNext(pidl);
   }

   return (cb + sizeof(pidl->mkid.cb));
}
