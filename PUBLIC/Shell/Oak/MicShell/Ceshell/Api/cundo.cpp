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

#include "cundo.h"
#include "idlist.h"
#include "recbin.h"
#include "shfileop.h"

CUndo::CUndo()
{
    InitializeCriticalSection(&m_csUndoData);

    m_fDataAccess = FALSE;
    m_dwThreadID = 0;
    m_iOperation = UNDO_NOTSET;
    m_bufFromLen = 0;
    m_bufToLen = 0;
}

CUndo::~CUndo()
{
    m_fDataAccess = FALSE;
    m_dwThreadID = 0;
    m_iOperation = UNDO_NOTSET;
    m_bufFromLen = 0;
    m_bufToLen = 0;

    DeleteCriticalSection(&m_csUndoData);
}

//AddItem preserves Undo information
/*
    File operations: copy | move | rename | delete

    Open();
    ...
    if(SUCCEEDED(operation))
    {  ...
        AddItem();
    }

    Close();
*/
//From MSDN for SHFileOperation (FO_DELETE):
//If pFrom is set to a file name, deleting the file with FO_DELETE will not move it to the Recycle Bin,
//even if the FOF_ALLOWUNDO flag is set. You must use a full path.
UINT CUndo::AddItem(LPCTSTR lpszFrom, LPCTSTR lpszTo)
{
    LPUNDODATA lpData;
    LPTSTR lpszTmp = NULL;
    size_t strFromLen = 0, strToLen = 0;
    HRESULT hr = S_OK;
    UINT uReturn = ERROR_SUCCESS;

    EnterCriticalSection(&m_csUndoData);

    // Add item to the list ONLY for the last started thread. Do not add item and return success for threads
    // which started before the last one.
    if (m_dwThreadID != GetCurrentThreadId())
    {
        LeaveCriticalSection(&m_csUndoData);
        return uReturn;
    }

    ASSERT(m_fDataAccess && m_dwThreadID);
    if (!m_fDataAccess || !m_dwThreadID)
    {
        LeaveCriticalSection(&m_csUndoData);
        return ERROR_INVALID_DATA;
    }

    ASSERT(UNDO_NOTSET != m_iOperation);

    if ((UNDO_NOTSET == m_iOperation) ||
        !lpszFrom || !lpszTo)
    {
        LeaveCriticalSection(&m_csUndoData);
        return ERROR_INVALID_PARAMETER;
    }

    strFromLen = _tcslen(lpszFrom);
    strToLen = _tcslen(lpszTo);

   if ((strFromLen >= MAX_PATH) || (strToLen >= MAX_PATH))
   {
      LeaveCriticalSection(&m_csUndoData);
      return ERROR_INVALID_DATA;
   }

    lpData = (LPUNDODATA)LocalAlloc(LMEM_FIXED, sizeof(UNDODATA));
    if (!lpData)
    {
        LeaveCriticalSection(&m_csUndoData);
        return ERROR_OUTOFMEMORY;
    }

    lpData->lpszTo = NULL;        //Do not need to preserve "To" for delete operation
    lpData->lpszFrom = NULL;    //Do not need to preserve "From" for copy operation

    //"To" should be preserved for copy, move, rename and delete
    if (lpszTo && (TEXT('\0') != *lpszTo))
    {
        lpszTmp = new TCHAR[strToLen + 1];
        if(!lpszTmp)
        {
            LeaveCriticalSection(&m_csUndoData);

            LocalFree((HLOCAL)lpData);
            lpData = NULL;
            return ERROR_OUTOFMEMORY;
        }

        hr = StringCchCopy(lpszTmp, strToLen + 1, lpszTo);
        if(FAILED(hr))
        {
            LeaveCriticalSection(&m_csUndoData);

            delete [] lpszTmp;
            lpszTmp = NULL;
            LocalFree((HLOCAL)lpData);
            lpData = NULL;

            return HRESULT_CODE(hr);

        }
        else
        {
            lpData->lpszTo = lpszTmp;
            m_bufToLen += (strToLen + 1);
        }
    }

    //"From" should be preserved for move, rename, delete
    if ((UNDO_COPY != m_iOperation) && lpszFrom && (TEXT('\0') != *lpszFrom))
    {
        lpszTmp = new TCHAR[strFromLen + 1];
        if(!lpszTmp)
        {
            LeaveCriticalSection(&m_csUndoData);

            delete [] lpData->lpszTo;
            lpData->lpszTo = NULL;
            LocalFree((HLOCAL)lpData);
            lpData = NULL;

            return ERROR_OUTOFMEMORY;
        }

        hr = StringCchCopy(lpszTmp, strFromLen + 1, lpszFrom);
        if(FAILED(hr))
        {
            LeaveCriticalSection(&m_csUndoData);

            delete [] lpszTmp;
            lpszTmp = NULL;

            delete [] lpData->lpszTo;
            lpData->lpszTo = NULL;
            LocalFree((HLOCAL)lpData);
            lpData = NULL;

            return HRESULT_CODE(hr);

        }
        else
        {
            lpData->lpszFrom = lpszTmp;
            m_bufFromLen += (strFromLen + 1);
        }
    }

   m_list.AddHead(&lpData->lpObj);

    LeaveCriticalSection(&m_csUndoData);

    return uReturn;
}

//For the last in thread sets a flag that data has been accumulated
UINT CUndo::Close()
{
    UINT uReturn = ERROR_SUCCESS;

    EnterCriticalSection(&m_csUndoData);

    // Close the list for the last SHFileOperation call ONLY (more then one thread can run)
    if (m_dwThreadID == GetCurrentThreadId())
    {
        ASSERT(m_fDataAccess);    //Something Wrong: finishing to preserve without starting

        if(m_fDataAccess)
        {
            m_fDataAccess = FALSE;
        }
        else
        {
            uReturn = ERROR_INVALID_DATA;
        }
    }

    LeaveCriticalSection(&m_csUndoData);

    return uReturn;
}

BOOL CUndo::HasData()
{
    BOOL fResult = TRUE;

    EnterCriticalSection(&m_csUndoData);

    //Still getting data (m_fDataAccess = TRUE)
    if (m_fDataAccess)
    {
        LeaveCriticalSection(&m_csUndoData);
        return FALSE;
    }

    //Operation flag is not set properly
    if (UNDO_NOTSET == m_iOperation)
    {
        fResult = FALSE;
    }

    //No data: something wrong with a list
    if (! ((LPUNDODATA)m_list.Head()))
    {
        fResult = FALSE;
    }

    LeaveCriticalSection(&m_csUndoData);

    return fResult;
}

//Open sets operation type and raises flag to allows new data accumulation
UINT CUndo::Open(int iOperation)
{
    ASSERT(UNDO_NOTSET != iOperation);

    if (UNDO_NOTSET == iOperation || 1 > iOperation || 4 < iOperation)
    {
        return ERROR_INVALID_DATA;
    }

    EnterCriticalSection(&m_csUndoData);

    // Open() is called per thread.
    // Clean up the existing list any time Open() is called, which means remove date preserved from
    // a previous SHFileOperation call.
    LPUNDODATA lpUndoData = (LPUNDODATA)m_list.Head();

    while(lpUndoData)
    {
        delete [] lpUndoData->lpszTo;
        delete [] lpUndoData->lpszFrom;

        //Move the head and release the old one
        m_list.Disconnect((LPLISTOBJ)lpUndoData);
        LocalFree(lpUndoData);

        lpUndoData = (LPUNDODATA)m_list.Head();
    }

    m_fDataAccess = TRUE;
    m_iOperation = iOperation;
    m_dwThreadID = GetCurrentThreadId();    // Keep last started threadId
    m_bufFromLen = 0;
    m_bufToLen = 0;

    LeaveCriticalSection(&m_csUndoData);

    return ERROR_SUCCESS;
}

UINT CUndo::Undo(HWND hwndOwner)
{
    LPUNDODATA lpUndoData;
    LPTSTR lpszToBuf = NULL;
    LPTSTR lpszToTmp;
    size_t bufToTmpLen = 0, strLen = 0;

    SHFILEOPSTRUCT shfo    = {0};
    shfo.hwnd = hwndOwner;

    //TODO: What should be under the Title
    TCHAR * lpszProgressTitle[] = {TEXT("Restore")};

    HRESULT hr = S_OK;
    UINT uReturn = ERROR_SUCCESS;

    EnterCriticalSection(&m_csUndoData);

    ASSERT(!m_fDataAccess);
    if (m_fDataAccess)
    {
        LeaveCriticalSection(&m_csUndoData);
        return ERROR_INVALID_DATA;
    }

    ASSERT(UNDO_NOTSET != m_iOperation);

    if ((UNDO_NOTSET == m_iOperation) ||
        ((UNDO_COPY != m_iOperation) && !m_bufFromLen) ) //(move| rename | delete) & no "From" buffer size
    {
        LeaveCriticalSection(&m_csUndoData);
        return ERROR_INVALID_DATA;
    }

    lpUndoData = (LPUNDODATA)m_list.Head();

    if (!lpUndoData )
    {
        LeaveCriticalSection(&m_csUndoData);
        return ERROR_INVALID_DATA;
    }

    //Create "To" buffer for UNDO_COPY
   if(UNDO_COPY == m_iOperation)
   {
        bufToTmpLen = m_bufToLen + 1;    //+1 for the 2nd \0
        lpszToBuf = new TCHAR[bufToTmpLen];
        if (!lpszToBuf)
        {
            LeaveCriticalSection(&m_csUndoData);
            return ERROR_OUTOFMEMORY;
        }

        bufToTmpLen--;    //-1 for the 2nd \0
    }

    lpszToTmp = lpszToBuf;

    shfo.fFlags = FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
    shfo.lpszProgressTitle = lpszProgressTitle[0];

    //Fill in "To" buffer for UNDO_COPY
    //OR Undo delete (Restore files from the RecycleBin)
    //OR Undo move, rename
    while (lpUndoData)
    {
        if(UNDO_COPY == m_iOperation)
        {
            strLen = 0;
            if (lpUndoData->lpszTo && (TEXT('\0') != *(lpUndoData->lpszTo)))
            {
                strLen = _tcslen(lpUndoData->lpszTo);
                hr = StringCchCopy(lpszToTmp, bufToTmpLen, lpUndoData->lpszTo);
                if(SUCCEEDED(hr))
                {
                    lpszToTmp += (strLen + 1);
                    bufToTmpLen -= (strLen + 1);
                }
                else
                {
                    uReturn = HRESULT_CODE(hr);
                }
            }
        }

        //Restore file from the Recycle Bin
        if (UNDO_DELETE == m_iOperation)
        {
            if (lpUndoData->lpszFrom && (TEXT('\0') != *(lpUndoData->lpszFrom)) &&
                lpUndoData->lpszTo && (TEXT('\0') != *(lpUndoData->lpszTo)))
            {
                LPITEMIDLIST pidl = NULL;
                if (SUCCEEDED(CreateFileSystemPidl(lpUndoData->lpszTo, &pidl)))
                {
                    g_pRecBin->BeginRecycle();
                    if (FAILED(g_pRecBin->RestoreFile(pidl)))
                    {
                        uReturn = ERROR_INVALID_DATA;
                    }

                    g_pRecBin->EndRecycle();
                    ILFree(pidl);
                }
                else
                {
                    uReturn = ERROR_INVALID_DATA;
                }
            }
        }

        if ((UNDO_MOVE == m_iOperation) || (UNDO_RENAME == m_iOperation))
        {
            LPTSTR lpszTo, lpszFrom;
            size_t strTo, strFrom;

            if (lpUndoData->lpszFrom && (TEXT('\0') != *(lpUndoData->lpszFrom)) &&
                lpUndoData->lpszTo && (TEXT('\0') != *(lpUndoData->lpszTo)))
            {
                strTo = _tcslen(lpUndoData->lpszTo);
                strFrom = _tcslen(lpUndoData->lpszFrom);

                lpszTo = new TCHAR[strTo + 2];
                lpszFrom = new TCHAR[strFrom + 2];

                if (!lpszTo || !lpszFrom)
                {
                    uReturn = ERROR_OUTOFMEMORY;
                }
                else
                {
                    hr = StringCchCopy(lpszTo, strTo + 1, lpUndoData->lpszTo);
                    if (SUCCEEDED(hr))
                    {
                        hr = StringCchCopy(lpszFrom, strFrom + 1, lpUndoData->lpszFrom);
                        if (SUCCEEDED(hr))
                        {
                            //Check if lpszFrom (folder case) is already restored from the
                            //previous step when file was restored.
                            DWORD dwAttrib;
                            dwAttrib = GetFileAttributes(lpszTo);
                            BOOL bMove = TRUE;
                            BOOL bIgnoreError = FALSE;

                            if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
                            {
                                dwAttrib = GetFileAttributes(lpszFrom);
                                if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
                                {
                                    if (UNDO_RENAME == m_iOperation)
                                    {
                                        //Do not assume lpszFrom is already restored.
                                        //Show error to the user instead (continue operation) and
                                        //remove data from the list
                                        bIgnoreError = TRUE;
                                    }
                                    else
                                    {
                                        //lpszFrom is already restored.
                                        //lpszTo should be empty at this point: Delete lpszTo
                                        uReturn = (RemoveDirectory(lpszTo) ? ERROR_SUCCESS : GetLastError());
                                        bMove = FALSE;
                                    }
                                }
                            }

                            if (bMove)
                            {
                                lpszTo[strTo + 1] = TEXT('\0');
                                lpszFrom[strFrom + 1] = TEXT('\0');

                                if (UNDO_MOVE == m_iOperation)
                                {
                                    shfo.wFunc  = FO_MOVE;
                                }
                                else
                                {
                                    shfo.wFunc  = FO_RENAME;
                                }

                                shfo.pFrom  = lpszTo;
                                shfo.pTo    = lpszFrom;
                                if (SHFileOperation(&shfo))
                                {
                                    uReturn = (bIgnoreError ? ERROR_SUCCESS : GetLastError());
                                }
                            }
                        }
                        else
                        {
                            uReturn = HRESULT_CODE(hr);
                        }
                    }
                    else
                    {
                        uReturn = HRESULT_CODE(hr);
                    }
                }

                if (lpszTo)
                {
                    delete [] lpszTo;
                    lpszTo = NULL;
                }

                if (lpszFrom)
                {
                    delete [] lpszFrom;
                    lpszFrom = NULL;
                }
            }
        }

        //Something happened: Stop handling data and keep all data under the list
        if (ERROR_SUCCESS != uReturn)
        {
            break;
        }

        delete [] lpUndoData->lpszTo;
        delete [] lpUndoData->lpszFrom;

        //Move the head and release the old one
        m_list.Disconnect((LPLISTOBJ)lpUndoData);
        LocalFree(lpUndoData);
        lpUndoData = NULL;

        if ((UNDO_COPY == m_iOperation) && !bufToTmpLen)
        {
            break;
        }

        lpUndoData = (LPUNDODATA)m_list.Head();
    }

    int iOperation = m_iOperation;

    LeaveCriticalSection(&m_csUndoData);

    //Undo copy: delete files located under "To" buffer
    if((ERROR_SUCCESS == uReturn) && (UNDO_COPY == iOperation))
    {
        //Second \0 for the buffers
        lpszToBuf[m_bufToLen] = TEXT('\0');

        shfo.wFunc  = FO_DELETE;
        shfo.pFrom  = lpszToBuf;
        shfo.pTo    = NULL;

        if (SHFileOperation(&shfo))
        {
            uReturn = GetLastError();
        }
    }

    delete [] lpszToBuf;
    lpszToBuf = NULL;

    return uReturn;
}
