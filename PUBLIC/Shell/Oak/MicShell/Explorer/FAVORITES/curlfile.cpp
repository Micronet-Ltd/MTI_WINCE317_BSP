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
#include "curlfile.h"
#include "utils.h"

#ifndef MAX_URL
#define MAX_URL 2048
#endif

// UTF8 Signature (byte order mark)
const BYTE CUrlFile::UTF8_BOM1 = 0xEF;
const BYTE CUrlFile::UTF8_BOM2 = 0xBB;
const BYTE CUrlFile::UTF8_BOM3 = 0xBF;

const WCHAR CUrlFile::s_wszInternetShortCut[] = L"[InternetShortcut]";
const WCHAR CUrlFile::s_wszURLMark[] = L"URL=";


int CUrlFile::GetLine(char *pszBuffer, int cchBuffer)
{
#define CHUNK_SIZE 100

    DWORD dwCharsRead = 0;
    DWORD dwBytesRead = 0;
    DWORD dwBytesToRead = cchBuffer-1; // room for a NULL
    BOOL bRet = FALSE;
    BOOL bCR = FALSE;
    UINT i;

    if (!pszBuffer || !cchBuffer)
    {
        goto leave;
    }

    while (dwBytesToRead)
    {
        bRet = ReadFile(m_hFile,
                        pszBuffer + dwCharsRead,
                        (dwBytesToRead > CHUNK_SIZE) ? CHUNK_SIZE : dwBytesToRead,
                        &dwBytesRead,
                        0);
        if (!bRet || dwBytesRead == 0)
        {
            pszBuffer[dwCharsRead] = '\0';
            // could just be EOF
            goto leave;
        }
    
        // scan the buffer for cr-lf
        bCR = FALSE;
        for (i = 0; i<dwBytesRead; i++)
        {
            if (pszBuffer[dwCharsRead+i] == '\r')
            {
                bCR = TRUE;
            }
            else if (pszBuffer[dwCharsRead+i] == '\n' && bCR)
            {
                // End of line
                pszBuffer[dwCharsRead+i-1] = '\0';
                // backup the chars we didn't use
                SetFilePointer(m_hFile, -((long)(dwBytesRead-(i+1))), 0, FILE_CURRENT);
                dwCharsRead += dwBytesRead;
                goto leave;        
            }
            else
            {
                bCR = FALSE;
            }
        }
        // decrement by what we got
        dwCharsRead += dwBytesRead;
        dwBytesToRead -= dwBytesRead;    
    }     

    // NULL terminate at the end of the buffer    
    pszBuffer[cchBuffer-1] = '\0';

leave:
    return dwCharsRead;
}

HRESULT CUrlFile::ReadURL(WCHAR *pwszURL, int cchUrl)
{
    HRESULT hr = E_FAIL;
    DWORD cbLength;
    DWORD dwCodePage;
    DWORD dwFlags;
    WCHAR *pwszLine = NULL;
    char *pBuffer = NULL;
    char *pszTemp;
    
    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        goto leave;
    }

    if (!pwszURL || !cchUrl)
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    pBuffer= (char*) LocalAlloc(LMEM_FIXED, sizeof(char) * (MAX_URL+5));
    if (!pBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto leave;
    }

    pwszLine = (WCHAR*) LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (MAX_URL+5));
    if (!pwszLine)
    {
        hr = E_OUTOFMEMORY;
        goto leave;
    }

    if (!GetLine(pBuffer, (MAX_URL+5)))
    {
        goto leave;
    }

    // Check for UTF8 Signature
    if ((UTF8_BOM1 == (BYTE)pBuffer[0]) &&
        (UTF8_BOM2 == (BYTE)pBuffer[1]) &&
        (UTF8_BOM3 == (BYTE)pBuffer[2]))
    {
        dwCodePage = CP_UTF8;
        dwFlags = 0;
        pszTemp = pBuffer + 3;
    }
    else
    {
        dwCodePage = CP_ACP;
        dwFlags = MB_PRECOMPOSED;
        pszTemp = pBuffer;
    }

    do
    {
        cbLength = MultiByteToWideChar(dwCodePage,
                        dwFlags,
                        pszTemp,
                        (strlen(pszTemp)+1),
                        pwszLine,
                        (MAX_URL+5));
        if (!cbLength)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto leave;
        }
        pwszLine[cbLength] = L'\0';

        if (!wcsncmp(pwszLine, s_wszURLMark, (ARRAYSIZE(s_wszURLMark)-1)))
        {
            hr = StringCchCopy(pwszURL,
                            cchUrl,
                            (pwszLine + ARRAYSIZE(s_wszURLMark) - 1));
            break;
        }

        pszTemp = pBuffer;
    }while (GetLine(pBuffer, (MAX_URL+5)));
  
leave:
    if (pBuffer)
    {
        LocalFree(pBuffer);
    }

    if (pwszLine)
    {
        LocalFree(pwszLine);
    }

    return hr;
}


HRESULT CUrlFile::WriteURL(WCHAR *pwszURL)
{
    HRESULT hr = S_OK;
    size_t cch;
    DWORD cbLength;
    DWORD dwBytesWritten;
    WCHAR *pwszTemp = NULL;
    char* pszUTF8 = NULL;
    DWORD cbBuffer;

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        hr = E_FAIL;
        goto leave;
    }

    if (!pwszURL)
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    pwszTemp = (WCHAR*) LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (MAX_URL+5));
    if (!pwszTemp)
    {
        hr = E_OUTOFMEMORY;
        goto leave;
    }

    // Append End Of Line to Section Name
    hr = StringCchPrintf(pwszTemp,
                    (MAX_URL+5),
                    L"%s%s",
                    s_wszInternetShortCut,
                    L"\r\n");
    if (FAILED(hr))
    {
        goto leave;
    }

    hr = StringCchLength(pwszTemp, (MAX_URL+5), &cch);
    if (FAILED(hr))
    {
        goto leave;
    }
    
    // Get required buffer size for encoding. 
    // Account bytes for UTF8 signature
    cbBuffer = WideCharToMultiByte(CP_UTF8, 0, pwszTemp, cch, NULL, 0, NULL, NULL);
    pszUTF8 = (char*) LocalAlloc(LMEM_FIXED, (cbBuffer+3));
    if (!pszUTF8)
    {
        hr = E_OUTOFMEMORY;
        goto leave;
    }

    // UTF8 signature
    pszUTF8[0] = UTF8_BOM1;
    pszUTF8[1] = UTF8_BOM2;
    pszUTF8[2] = UTF8_BOM3;

    cbLength = WideCharToMultiByte(CP_UTF8, 0, pwszTemp, cch, pszUTF8+3, cbBuffer, NULL, NULL);
    if (!cbLength || !WriteFile(m_hFile, pszUTF8, cbLength+3,  &dwBytesWritten, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

    hr = StringCchPrintf(pwszTemp,
                    (MAX_URL+5),
                    L"%s%s",
                    s_wszURLMark,
                    pwszURL);
    if (FAILED(hr))
    {
        goto leave;
    }

    hr = StringCchLength(pwszTemp, (MAX_URL+5), &cch);
    if (FAILED(hr))
    {
        goto leave;
    }

    // Reallocate buffer for encoding; use previous buffer on failure
    cbLength = WideCharToMultiByte(CP_UTF8, 0, pwszTemp, cch, NULL, 0, NULL, NULL);
    char* pszTemp = (char*) LocalReAlloc(pszUTF8, cbLength, 0);
    if (pszTemp)
    {
        pszUTF8 = pszTemp;
        cbBuffer = cbLength;
    }

    cbLength = WideCharToMultiByte(CP_UTF8, 0, pwszTemp, cch, pszUTF8, cbBuffer, NULL, NULL);
    if (!cbLength || !WriteFile(m_hFile, pszUTF8, cbLength,  &dwBytesWritten, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

leave:
    if (pwszTemp)
    {
        LocalFree(pwszTemp);
    }

    if (pszUTF8)
    {
        LocalFree(pszUTF8);
    }

    if (FAILED(hr))
    {
        SetFilePointer(m_hFile, 0 ,0, FILE_BEGIN);
    }
    SetEndOfFile(m_hFile);

    return hr;   
}

HRESULT CUrlFile::OpenFile(WCHAR *pwszPath, BOOL bCreate)
{
    HRESULT hr = S_OK;
    DWORD dwDesiredAccess;
    DWORD dwCreationDispostion;

    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseFile();
    }

    if (bCreate)
    {
        // Write to new file
        dwDesiredAccess = GENERIC_WRITE;
        dwCreationDispostion = CREATE_NEW;
    }
    else
    {
        // Read existing file
        dwDesiredAccess = GENERIC_READ;
        dwCreationDispostion = OPEN_EXISTING;        
    }

    m_hFile = CreateFile(pwszPath, 
                    dwDesiredAccess,
                    0,
                    NULL,
                    dwCreationDispostion,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

void CUrlFile::CloseFile()
{
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
    }

    m_hFile = INVALID_HANDLE_VALUE;
}

