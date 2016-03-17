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

#pragma once

#include "ceshhpc.h"
#include "list.hpp"

//Undo Types correspond to FO_ flags (SHFileOperation)
#define UNDO_NOTSET	0
#define UNDO_MOVE		FO_MOVE
#define UNDO_COPY		FO_COPY
#define UNDO_DELETE	FO_DELETE
#define UNDO_RENAME	FO_RENAME

typedef struct tagUNDODATA {
	LISTOBJ	lpObj;				// list.hpp
	LPTSTR	lpszFrom;
	LPTSTR	lpszTo;	
} UNDODATA, *LPUNDODATA;

class CUndo
{
	BOOL		m_fDataAccess;
	DWORD		m_dwThreadID;
	int		m_iOperation;
	size_t	m_bufFromLen;
	size_t	m_bufToLen;

	CRITICAL_SECTION m_csUndoData;
	CList		m_list;

public:
	
	CUndo();
	~CUndo();

	UINT AddItem(LPCTSTR lpszFrom, LPCTSTR lpszTo);
	UINT Close();
	BOOL HasData();
	UINT Open(int iOperation);	
	UINT Undo(HWND hwndOwner = NULL);
};

