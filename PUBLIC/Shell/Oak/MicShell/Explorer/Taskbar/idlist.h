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
#include <shlobj.h>

#pragma once

#define IL_ALL               ((UINT)-1)

void ILFree(LPCITEMIDLIST pidl);
BOOL ILIsEmpty(LPCITEMIDLIST pidl);
LPITEMIDLIST ILNext(LPCITEMIDLIST pidl);
size_t ILSize(LPCITEMIDLIST pidl, UINT cbItems);
LPITEMIDLIST ILConcatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
LPITEMIDLIST ILCopy(LPCITEMIDLIST pidl, UINT cbItems);
