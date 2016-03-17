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

#include "tbdrop.h"
#include "taskbaritem.h"

extern CTaskBar * g_TaskBar; // from explorer.cpp

const DWORD CTaskbarDropTarget::TIMER_THRESHOLD = 1000;

IDropTarget *CTaskbarDropTarget_Create(HWND hwndTB, CList *plistTaskBar)
{
    return new CTaskbarDropTarget(hwndTB, plistTaskBar);
}

CTaskbarDropTarget::CTaskbarDropTarget(HWND hwndTB, CList *plistTaskBar) :
    m_dwRefs(1),
    m_hwndTB(hwndTB),
    m_plist(plistTaskBar),
    m_uTarget((UINT)-1),
    m_dwClockTimer(0)
{
}

STDMETHODIMP CTaskbarDropTarget::QueryInterface(REFIID riid, LPVOID * ppobj)
{
    *ppobj = NULL;

    if ((riid == IID_IUnknown) || (riid == IID_IDropTarget))
    {
        ++m_dwRefs;
        *ppobj = (LPVOID)this;

        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CTaskbarDropTarget::AddRef(VOID)
{
    return ++m_dwRefs;
}

STDMETHODIMP_(ULONG) CTaskbarDropTarget::Release(VOID)
{
    if (--m_dwRefs == 0)
    {
        delete this;
        return 0;
    }

    return m_dwRefs;
}

HRESULT
CTaskbarDropTarget::DragEnter(
    IDataObject * pDataObj, 
    DWORD grfKeyState, 
    POINTL pt,
    DWORD *pdwEffect
)
{
    if (!pDataObj || !pdwEffect)
    {
        return E_INVALIDARG;
    }

    ImageList_DragEnter(NULL, pt.x, pt.y);

    return S_OK;
}

HRESULT 
CTaskbarDropTarget::DragOver(
    DWORD grfKeyState, 
    POINTL pt, 
    DWORD *pdwEffect
)
{
    UINT grfFlags;
    UINT uIndex;
    LPTBOBJ ptbobj;

    // Find out if the cursor is on the item. If it is,
    // set the timer, and change the cursor accordingly.
    POINT ptTaskbar = { pt.x, pt.y };
    ScreenToClient(m_hwndTB, &ptTaskbar);
    uIndex = g_TaskBar->HitTest(m_hwndTB, ptTaskbar, &grfFlags);

    ImageList_DragMove(pt.x, pt.y);

    // Unhide the task bar if auto hide is selected
    g_TaskBar->SetUnhideTimer(ptTaskbar.x, ptTaskbar.y);

    if (uIndex != (UINT)-1)
    {
        ptbobj = g_TaskBar->GetItem(m_hwndTB, uIndex);

        if (ptbobj && (ptbobj->ptbi->m_uFlags & HHTBS_BUTTON) && 
            ((ptbobj->ptbi->m_hwndMain == HHTB_DESKTOP) || IsWindow(ptbobj->ptbi->m_hwndMain)))
        {
            if(m_dwClockTimer && uIndex == m_uTarget)
            {
                // Check timer
                if(GetTickCount() - m_dwClockTimer > TIMER_THRESHOLD)
                {
                    // Bring app forward
                    HIMAGELIST himl = ImageList_GetDragImage(NULL, NULL);;
                    ImageList_DragShowNolock(FALSE);

                    if (ptbobj->ptbi->m_hwndMain == HHTB_DESKTOP)
                    {
                        SendMessage(m_hwndTB, WM_COMMAND, (IDM_TASKBAR_CLOSE + 1),
                                    MAKELONG(0, TRUE));
                    }
                    else
                    {
                        SendMessage(m_hwndTB, WM_COMMAND, (IDM_TASKBAR_CLOSE + 1),
                                    MAKELONG(uIndex, TRUE));
                    }

                    ImageList_DragShowNolock(TRUE);
                    if (himl)
                    {
                        ImageList_SetDragCursorImage(himl, 0, 0, 0);
                    }

                    m_dwClockTimer = 0;
                }
            }
            else
            {
                m_dwClockTimer = GetTickCount();
            }
            
        }
        else
        {
            *pdwEffect = DROPEFFECT_NONE;
        }
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }
    
    m_uTarget = uIndex;
    return S_OK;
}

HRESULT 
CTaskbarDropTarget::DragLeave()
{
    ImageList_DragLeave(NULL);
    return S_OK;
}

HRESULT 
CTaskbarDropTarget::Drop(
    IDataObject *pDataObj, 
    DWORD grfKeyState, 
    POINTL pt,
    DWORD *pdwEffect
)
{
    if (!pDataObj)
    {
        return E_INVALIDARG;
    }

    ImageList_DragLeave(NULL);

    // pop up message box like win95

    return E_UNEXPECTED;
}

