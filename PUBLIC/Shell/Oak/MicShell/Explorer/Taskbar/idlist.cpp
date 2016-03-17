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

void ILFree(LPCITEMIDLIST pidl)
{
   if (pidl)
      LocalFree((void*)pidl);
}

BOOL ILIsEmpty(LPCITEMIDLIST pidl)
{
   return (0 == (pidl)->mkid.cb);
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


