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

#include "ceshpriv.h"
#include "list.hpp"

typedef struct tagDROPTARGETINFO {
	LISTOBJ	lpObj;
	HWND	hwnd;
	IDropTarget *pDropTarget;	
} DROPTARGETINFO, *LPDROPTARGETINFO;

#ifdef __cplusplus

class CDropTargetList
{
public:
	CDropTargetList();
	~CDropTargetList();

	LPDROPTARGETINFO FindDropTargetByHwnd(HWND hwndTarget);
	BOOL RemoveDropTarget(HWND hwndTarget);
	BOOL AddDropTarget(HWND hwndTarget, IDropTarget *pDropTarget);
	
private:
	CList m_list;
	
}; /* CDropTargetList */


//+-------------------------------------------------------------------------
//
//  Class:		CDragOperation
//
//  Purpose:	Handles breaking down drag operation into managable pieces
//
//  Interface:	UpdateTarget - update where we are trying to drop
//              HandleFeedBack - handle cursor feedback
//				DragOver - handle dragging object over target
//				HandleMessages - Handle windows messages
//				CompleteDrop - Do drop or clean up
//              CancelDrag - notify operation that drag is canceled.
//              ReleaseCapture - release capture on the mouse
//              GetDropTarget - get target for drop
//
//  History:	dd-mmm-yy Author    Comment
//				04-Apr-94 Ricksa    Created
//				05-Oct-95 ScottSh	Ported to Pegasus
//
//--------------------------------------------------------------------------
class CDragOperation
{
public:
	CDragOperation(
		LPDATAOBJECT pDataObject,
		LPDROPSOURCE pDropSource,
		DWORD dwOKEffects,
		DWORD FAR *pdwEffect,
		HRESULT& hr);

	~CDragOperation(void);

    BOOL		UpdateTarget(void);
    BOOL        HandleFeedBack(HRESULT hr);
    BOOL		DragOver(void);
    BOOL		HandleMessages(void);
    HRESULT		CompleteDrop(void);
//    void        CancelDrag(void);

private:

    LPDATAOBJECT	m_pDataObject;
    LPDROPSOURCE	m_pDropSource;
    LPDROPTARGET	m_pDropTarget;
	LPDROPTARGET	m_pdtNoTarget;
	POINTL			m_ptlLastPoint;
    DWORD			m_dwOKEffects;
    DWORD FAR * 	m_pdwEffect;
    BOOL			m_fEscapePressed;
    HWND			m_hwndLast;
	HWND			m_hwndDropTarget;
    DWORD			m_grfKeyState;
    HRESULT			m_hrDragResult;
    BOOL            m_fReleasedCapture;

	// NOTE: Window which gets created to hold the capture of the drag
	HWND			m_hwndHasDragCapture;
};


class CNullDropTarget : public IDropTarget
{
public:
	CNullDropTarget() { m_dwRefs = 1; };

// IUnknown
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef) (VOID) ;
    STDMETHOD_(ULONG,Release) (VOID);

// IDropTarget

	STDMETHOD(DragEnter)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt,
						 DWORD *pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt,
					DWORD *pdwEffect);
private:
	DWORD			m_dwRefs;
};

inline POINT POINTfromPOINTL(POINTL pl) 
{
	POINT p;
	p.x = pl.x;
	p.y = pl.y;
	return p;
}

#endif // __cplusplus

LRESULT CALLBACK DragDrop_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);


