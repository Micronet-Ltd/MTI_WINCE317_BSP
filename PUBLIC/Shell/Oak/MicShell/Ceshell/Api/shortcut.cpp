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

#include "shortcut.h"
#include "resource.h"

//NOTE: This is being done since we have a very useless file format
//      and we are going to force all shortcuts we create to have quotes
//      around the target executable, so that means we need to use 2 of
//      the characters in the path for quotes.

#define MAX_SHORTCUT_PATH       (MAX_PATH-2)

// UTF8 Signature (byte order mark)
#define UTF8_BOM1   0xEF
#define UTF8_BOM2   0xBB
#define UTF8_BOM3   0xBF

extern IMalloc * g_pShellMalloc;

STDAPI_(BOOL) PathMakeUniqueNameEx(
    LPCTSTR lpszPath,
    LPCTSTR lpszT1,
    LPCTSTR lpszT2,
    BOOL bLink,
    BOOL bSourceIsDirectory,
    LPTSTR lpszUniqueName,
    UINT cchUniqueName
    );

BOOL Shortcut_CreateEx(LPCWSTR pwszTarget, LPCWSTR pwszDir, LPCWSTR pwszFile,
                  LPWSTR  pwszShortcut, LPDWORD lpcbShortcut,
                  DWORD dwCreation, BOOL fUniqueName)
{
   HANDLE hFile;
   WCHAR wszTemp1[MAX_PATH];
   WCHAR wszTemp2[MAX_PATH];
   LPWSTR pwszTag = NULL;
   DWORD dwErrCode = ERROR_FILENAME_EXCED_RANGE;
   BOOL bRet = FALSE;

   PREFAST_ASSERT(pwszTarget && pwszDir && pwszFile);

   if ((::wcslen(pwszTarget) + 1) >= MAX_SHORTCUT_PATH)
   {
      goto leave;
   }

   if ((::wcslen(pwszDir) + ::wcslen(pwszFile) + 2) >= MAX_PATH)
   {
      goto leave;
   }

   // Prepare dir path in wszTemp2
   if (FAILED(::StringCchCopy(wszTemp2, lengthof(wszTemp2), pwszDir)))
   {
      goto leave;       
   }
   
   if (0 != ::wcscmp(wszTemp2, L"\\"))
   {
      if (FAILED(::StringCchCat(wszTemp2, lengthof(wszTemp2), L"\\")))
      {
         goto leave;       
      }
   }

   // If this is a guid use the DisplayName from the registry
   if (PathIsGUID(pwszFile) == NOERROR)
   {
      LONG lResult = ERROR_SUCCESS;
      HKEY hKey = NULL;
      WCHAR wszDisplayName[MAX_PATH] = L"CLSID\\";
      DWORD cb = sizeof(wszDisplayName);

      if (FAILED(::StringCchCat(wszDisplayName, lengthof(wszDisplayName), pwszFile)))
      {
         goto leave;
      }

      lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, wszDisplayName, 0, 0, &hKey);
      if (ERROR_SUCCESS == lResult)
      {
         lResult = RegQueryValueEx(hKey,
                           L"DisplayName",
                           NULL,
                           NULL,
                           (LPBYTE)wszDisplayName,
                           &cb);
         RegCloseKey(hKey);

         // NULL terminate the string
        wszDisplayName[lengthof(wszDisplayName)-1] = 0;

         if ((ERROR_SUCCESS == lResult) &&
            FAILED(StringCchCat(wszTemp2, lengthof(wszTemp2), wszDisplayName)))
         {
            // error appending the display name
            goto leave;
         }
      }
   }
   else
   {
      if (FAILED(::StringCchCat(wszTemp2, lengthof(wszTemp2), pwszFile)))
      {
         goto leave;
      }
   }

   if (fUniqueName) 
   {
      BOOL bIsDirectory;

      // NOTE: We enter this to make a unique name
      //       "Foo.bar"             --> "Shortcut To Foo.lnk"
      //       "Foo.lnk"             --> "Foo (2).lnk"
      //       "Shortcut To Foo.lnk" --> "Shortcut To Foo (2).lnk"
      if (FAILED(::StringCchCopy(wszTemp1, lengthof(wszTemp1), pwszTarget)))
      {
         goto leave;
      }

      PathRemoveQuotesAndArgs(wszTemp1);
      bIsDirectory = PathIsDirectory(wszTemp1);

      if (!PathIsLink(wszTemp1))
      {
         pwszTag = LOAD_STRING(IDS_SHORTCUT_TO);
      }

      if (FAILED(::StringCchCopy(wszTemp1, lengthof(wszTemp1), wszTemp2)))
      {
         goto leave;
      }

      // PathMakeUniqueNameEx has an extra parameter to
      // identify the source as a directory (do not remove extension)
      if (!PathMakeUniqueNameEx(wszTemp1, pwszTag, NULL, TRUE, bIsDirectory, wszTemp2, lengthof(wszTemp2)))
      {
         dwErrCode = ERROR_FILE_EXISTS;
         goto leave;
      }

      // see if user wants the unique name returned
      if (pwszShortcut && lpcbShortcut)
      {
         DWORD dwLen = ::wcslen(wszTemp2)+1;
         // check if buffer is big enough
         if (*lpcbShortcut < dwLen)
         {
            *lpcbShortcut = dwLen;
            dwErrCode = ERROR_INSUFFICIENT_BUFFER;
            goto leave;
         }
         ::wcscpy(pwszShortcut, wszTemp2);
      }
   }

   // NOTE: After we have gotten the file handle, we are then allowed
   //       to use the buffer wszTemp2 again for temporary string manipultaion.
   hFile = CreateFile(wszTemp2,
                     GENERIC_WRITE,
                     0,
                     NULL,
                     dwCreation,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
   if (hFile == INVALID_HANDLE_VALUE)
   {
      // LastError already set by CreateFile
      dwErrCode = 0;
      goto leave;
   }

   bRet = Shortcut_Write(hFile, pwszTarget);
   CloseHandle(hFile);
   dwErrCode = 0;

leave:
   if (dwErrCode)
   {
      SetLastError(dwErrCode);
   }

   return bRet;
}

BOOL Shortcut_Write (HANDLE hFile, LPCWSTR pwszTarget)
{
   BOOL bRet = FALSE;
   DWORD dwErrCode = 0;
   DWORD cbLength, cbWritten;
   WCHAR wszTemp1[MAX_PATH];
   WCHAR wszTemp2[MAX_PATH];
   WCHAR wszTemp3[MAX_PATH];
   char* pszUTF8 = NULL;
   DWORD cbBuffer;
   int nCount;

   if (!pwszTarget)
   {
      dwErrCode = ERROR_INVALID_PARAMETER;
      goto leave;
   }

   // NOTE: need to remember what the arguments were.
   if (FAILED(StringCchCopy(wszTemp1, lengthof(wszTemp1), pwszTarget)) ||
      FAILED(StringCchCopy(wszTemp2, lengthof(wszTemp2), PathGetArgs(wszTemp1))))
   {
      dwErrCode = ERROR_FILENAME_EXCED_RANGE;
      goto leave;
   }

   PathRemoveQuotesAndArgs(wszTemp1);

   // NOTE: We need to see if we are making a shortcut to a shortcut,
   //       and as such would need to get the actual target.
   if (PathIsLink(wszTemp1))
   {
      if (! Shortcut_GetTarget(wszTemp1, wszTemp1, lengthof(wszTemp1)))
      {
         // LastError already set by Shortcut_GetTarget
         goto errorReturn;
      }
      ::wcscpy(wszTemp2, PathGetArgs(wszTemp1));
      PathRemoveQuotesAndArgs(wszTemp1);
   }

   // NOTE: Get the character count of the shortcut string.
   nCount = ::wcslen(wszTemp2);
   if (0 != nCount)
   {
      nCount ++; // For the space between the target and args.
   }
   nCount += ::wcslen(wszTemp1) + 2;

   if (FAILED(::StringCchPrintf(wszTemp3,
                     lengthof(wszTemp3),
                     L"%d#\"%s\"",
                     nCount,
                     wszTemp1)))
   {
      dwErrCode = ERROR_FILENAME_EXCED_RANGE;
      goto leave;
   }

   // Get required buffer size for encoding
   cbBuffer = WideCharToMultiByte(CP_UTF8, 0, wszTemp3, ::wcslen(wszTemp3), NULL, 0, NULL, NULL);
   pszUTF8 = (char*)g_pShellMalloc->Alloc(cbBuffer+3);
   if (!pszUTF8)
   {
      SetLastError(ERROR_OUTOFMEMORY);
      goto errorReturn;
   }

   // UTF8 signature
   pszUTF8[0] = (BYTE)UTF8_BOM1;
   pszUTF8[1] = (BYTE)UTF8_BOM2;
   pszUTF8[2] = (BYTE)UTF8_BOM3;

   cbLength = WideCharToMultiByte(CP_UTF8, 0, wszTemp3, ::wcslen(wszTemp3), pszUTF8+3, cbBuffer, NULL, NULL);
   if (!cbLength || !WriteFile(hFile, pszUTF8, cbLength+3, &cbWritten, NULL))
   {
      // LastError already set by WriteFile or WideCharToMultiByte
      goto errorReturn;
   }

   if (::wcslen(wszTemp2))
   {
      if (FAILED(::StringCchPrintf(wszTemp3,
                        lengthof(wszTemp3),
                        L" %s",
                        wszTemp2)))
      {
         dwErrCode = ERROR_FILENAME_EXCED_RANGE;
         goto leave;
      }

      // Reallocate buffer for encoding; use previous buffer on failure
      cbLength = WideCharToMultiByte(CP_UTF8, 0, wszTemp3, ::wcslen(wszTemp3), NULL, 0, NULL, NULL);
      char* pszTemp = (char*)g_pShellMalloc->Realloc(pszUTF8, cbLength);
      if (pszTemp)
      {
         pszUTF8 = pszTemp;
         cbBuffer = cbLength;
      }

      cbLength = WideCharToMultiByte(CP_UTF8, 0, wszTemp3, ::wcslen(wszTemp3), pszUTF8, cbBuffer, NULL, NULL);
      if (!cbLength || !WriteFile(hFile, pszUTF8, cbLength, &cbWritten, NULL))
      {
         // LastError already set by WriteFile or WideCharToMultiByte
         goto errorReturn;
      }
   }

   bRet = TRUE;

leave:
   if (pszUTF8)
   {
      g_pShellMalloc->Free(pszUTF8);
   }

   SetEndOfFile(hFile);
   SetLastError(dwErrCode);
   return bRet;

errorReturn:
   // Need to save & restore LastError
   dwErrCode = GetLastError();
   SetFilePointer(hFile, 0, 0, FILE_BEGIN);
   goto leave;
}

/*
   @func BOOL | SHCreateShortcut | Creates a shortcut
   @parm LPWSTR | pwszShortcut | Pointer to a buffer that contains the path & 
                        filename of   the shortcut to create.
   @parm LPWSTR | pwszTarget | Pointer to a buffer that contains the target 
                        path of the shortcut.
*/
extern "C" DWORD WINAPI SHCreateShortcut(LPWSTR pwszShortcut, LPWSTR pwszTarget)
{
   BOOL bRet = FALSE;
   WCHAR wszPath[MAX_PATH];
   DWORD dwErrCode = ERROR_INVALID_PARAMETER;

   // Validate params
   if (!pwszShortcut || !pwszTarget || !pwszShortcut[0] || !pwszTarget[0])
   {
      goto leave;
   }

   if ((::wcslen(pwszShortcut)) + 1 >= MAX_PATH)
   {
      dwErrCode = ERROR_FILENAME_EXCED_RANGE;
      goto leave;
   }

   if (!PathIsValidPath(pwszShortcut))
   {
      goto leave;
   }

   // seperate shortcut directory & filename 
   if (FAILED(StringCchCopy(wszPath, lengthof(wszPath), pwszShortcut)))
   {
      dwErrCode = ERROR_FILENAME_EXCED_RANGE;
      goto leave;
   }
   PathRemoveFileSpec(wszPath);

   bRet = Shortcut_CreateEx(pwszTarget, wszPath, PathFindFileName(pwszShortcut), NULL, NULL, CREATE_NEW, FALSE);
   // LastError already set by Shortcut_CreateEx
   dwErrCode = 0;

leave:
   if (dwErrCode)
   {
      SetLastError(dwErrCode);
   }

   return bRet;
}

extern "C" DWORD WINAPI SHCreateShortcutEx(LPWSTR pwszDir, LPWSTR pwszTarget, LPWSTR pwszShortcut, LPDWORD lpcbShortcut)
{
   BOOL bRet = FALSE;
   WCHAR wszPath[MAX_PATH];
   DWORD dwErrCode = ERROR_INVALID_PARAMETER;

   // Validate params
   if (!pwszDir || !pwszTarget || !pwszDir[0] || !pwszTarget[0])
   {
      goto leave;
   }

   if (!PathIsValidPath(pwszDir))
   {
      goto leave;
   }

    // seperate shortcut directory & filename 
   if (FAILED(StringCchCopy(wszPath, lengthof(wszPath), pwszDir)))
   {
      dwErrCode = ERROR_FILENAME_EXCED_RANGE;
      goto leave;
   }
   PathRemoveFileSpec(wszPath);

   bRet = Shortcut_CreateEx(pwszTarget, wszPath, PathFindFileName(pwszDir), pwszShortcut, lpcbShortcut, CREATE_NEW, TRUE);
   // LastError already set by Shortcut_CreateEx
   dwErrCode = 0;

leave:
   if (dwErrCode)
   {
      SetLastError(dwErrCode);
   }

   return bRet;
}


BOOL Shortcut_GetTarget(LPCWSTR pwszFile, LPWSTR pwszTarget, UINT cbLength)
{
#define SGT_BUFSIZE (3*MAX_PATH)
   BOOL bRet = FALSE;
   HANDLE hFile = INVALID_HANDLE_VALUE;
   DWORD cbRead;
   char *pBuffer;
   char szUTF8[SGT_BUFSIZE];
   DWORD dwCodePage;
   DWORD dwFlags;

   // Validate params
   if (!pwszFile || !pwszTarget || !pwszFile[0] || !cbLength)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      goto leave;
   }

   hFile = CreateFile(pwszFile,
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
   if (INVALID_HANDLE_VALUE == hFile)
   {
      // LastError already set by CreateFile
      goto leave;
   }

   // reset the file
   SetFilePointer (hFile, 0, NULL, FILE_BEGIN) ; 

   if (!ReadFile(hFile, szUTF8, SGT_BUFSIZE-1, &cbRead, NULL))
   {
      // LastError already set by ReadFile
      goto leave;
   }

   szUTF8[cbRead] = 0; // NULL terminate the string.

   // Check for UTF8 Signature
   if ((UTF8_BOM1 == (BYTE)szUTF8[0]) &&
      (UTF8_BOM2 == (BYTE)szUTF8[1]) &&
      (UTF8_BOM3 == (BYTE)szUTF8[2]))
   {
      dwCodePage = CP_UTF8;
      dwFlags = 0;
      pBuffer = szUTF8 + 3;
   }
   else
   {
      DWORD dwType;
      DWORD dwLen = sizeof(DWORD);

      if (ERROR_SUCCESS != RegQueryValueEx(HKEY_LOCAL_MACHINE,
                        L"ACP",
                        (LPDWORD)L"SOFTWARE\\Microsoft\\International",
                        &dwType,
                        (LPBYTE)&dwCodePage,
                        &dwLen))
      {
         dwCodePage = CP_ACP;
      }
      dwFlags = MB_PRECOMPOSED;
      pBuffer = szUTF8;
   }

   while (*pBuffer)
   {
      if (*pBuffer == '#')
      {
         pBuffer++;
         goto foundtarget;
      }

      if (*pBuffer < '0' || *pBuffer > '9')
      {
         break;
      }
      pBuffer++;
   }
   SetLastError(ERROR_BAD_FORMAT);
   goto leave;

 foundtarget:
   if (MultiByteToWideChar(dwCodePage, dwFlags, pBuffer, strlen(pBuffer)+1, pwszTarget, cbLength) == 0)
   {
      // LastError already set by MultiByteToWideChar
      goto leave;
   }   

   SetLastError(0);
   bRet = TRUE;

leave:
   if (INVALID_HANDLE_VALUE != hFile)
   {
      CloseHandle(hFile);
   }

   return bRet;
} 


/*
   @doc BOTH EXTERNAL PEGSHELL

   @func BOOL | SHGetShortcutTarget | retrieves the path which the shortcut points to.

   @parm LPCWSTR | pwszShortcut | Pointer to a buffer that contains the name of
   the shortcut to create.
   @parm LPWSTR | pwszTarget | Pointer to a buffer that gets the target path
   of the shortcut.
   @parm int | cbMax | count in characters to copy

*/
extern "C" BOOL WINAPI SHGetShortcutTarget(LPCWSTR pwszShortcut, LPWSTR pwszTarget, int cb)
{
   return Shortcut_GetTarget(pwszShortcut, pwszTarget, cb);
}

