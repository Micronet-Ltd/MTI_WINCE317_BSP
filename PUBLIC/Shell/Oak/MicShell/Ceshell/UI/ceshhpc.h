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
#include "aygutils.h"

// Important externs
// TODO: comment below line
extern IMalloc * g_pShellMalloc;
class FileChangeManager;
extern FileChangeManager * g_pFileChangeManager;
// TODO: comment below part
class CUndo;
extern CUndo * g_pUndo;
class CRecycleBin;
extern CRecycleBin * g_pRecBin;

