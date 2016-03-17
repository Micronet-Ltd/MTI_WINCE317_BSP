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

#include "iniutil.h"


BOOL ReadIni(LPCWSTR pwszSection, LPCWSTR pwszKey, LPWSTR pwszValue, size_t cchValue, LPCWSTR pwszIniFile)
{
   // GetPrivateProfileString is unavailable on CE

   BOOL bSuccess = FALSE;
   FILE * fin;
   WCHAR wszLine[256];
   LPWSTR pwsz = NULL;

   if (!pwszIniFile ||
      !pwszSection ||
      !pwszKey ||
      !pwszValue ||
      !cchValue)
   {
      goto leave;
   }

   fin = _wfopen(pwszIniFile, L"r");
   if (fin)
   {
      while (fgetws(wszLine, lengthof(wszLine), fin))
      {
         if (!wcsstr(wszLine, pwszSection)) // Section
         {
            continue;
         }

         while (fgetws(wszLine, lengthof(wszLine), fin)) // Key & Value
         {
            pwsz = wcschr(wszLine, L'=');
            if (!pwsz)
            {
               continue;
            }

            *pwsz = L'\0'; // Erase the '='

            if (0 != _wcsicmp(wszLine, pwszKey)) // Key
            {
               continue;
            }

            pwsz++; // Point to the value

            bSuccess = SUCCEEDED(StringCchCopyEx(
                                    pwszValue,
                                    cchValue,
                                    pwsz,
                                    &pwsz,
                                    NULL,
                                    0
                                    ));

            if (bSuccess)
            {
               if (*--pwsz == L'\n')
               {
                  *pwsz = L'\0';
               }
            }
         }
      }

      fclose(fin);
   }

leave:
   return bSuccess;
}


BOOL WriteIni(LPCWSTR pwszSection, LPCWSTR pwszKey, LPWSTR pwszValue, size_t cchValue, LPCWSTR pwszIniFile)
{
   // WritePrivateProfileString is unavailable on CE

   BOOL bSuccess = FALSE;
   BOOL bReplace;
   FILE * fin;
   fpos_t posSrc;
   fpos_t posDest;
   size_t cchLine;
   size_t cchNewLine;
   WCHAR wszLine[256];
   WCHAR wszNewLine[256];
   LPWSTR pwsz = NULL;
   DWORD dwAttrib;

   if (!pwszIniFile ||
      !pwszSection ||
      !pwszKey ||
      (!pwszValue && cchValue) ||
      (cchValue >= lengthof(wszLine)))
   {
      goto leave;
   }

   dwAttrib = GetFileAttributes(pwszIniFile);
   if (dwAttrib == -1)
   {
      goto leave;
   }

   SetFileAttributes(pwszIniFile, dwAttrib & ~FILE_ATTRIBUTE_READONLY);

   fin = _wfopen(pwszIniFile, L"r+");
   if (fin)
   {
      bReplace = FALSE;
      cchNewLine = 0;

      while (fgetws(wszLine, lengthof(wszLine), fin))
      {
         if (!wcsstr(wszLine, pwszSection)) // Section
            continue;

         if (fgetpos(fin, &posSrc) || fgetpos(fin, &posDest))
         {
            goto closeFile;
         }

         while (fgetws(wszLine, lengthof(wszLine), fin)) // Key & Value
         {
            cchLine = wcslen(wszLine);

            pwsz = wcschr(wszLine, L'=');
            if (!pwsz)
            {
               goto nextLine;
            }

            *pwsz = L'\0'; // Erase the '='

            if (0 != _wcsicmp(wszLine, pwszKey)) // Key
            {
               *pwsz = L'='; // Add the '=' back
               goto nextLine;
            }

            *pwsz = L'='; // Add the '=' back
            pwsz++; // Point to the value

            // Clear Key & Value on NULL or empty string 
            if (!pwszValue || !cchValue)
            {
               bReplace = TRUE;
               cchLine = 0;
               goto nextLine;
            }

            // Do not overwritte more than the number of read characters.
            size_t cchKey = (pwsz - wszLine);
            if (SUCCEEDED(StringCchCopyNEx(
                                    wszNewLine,
                                    lengthof(wszNewLine),
                                    wszLine,
                                    cchKey,
                                    &pwsz,
                                    &cchNewLine,
                                    0
                                    )) &&
               SUCCEEDED(StringCchCopyNEx(
                                    pwsz,
                                    cchNewLine,
                                    pwszValue,
                                    min((cchLine - cchKey), cchValue),
                                    &pwsz,
                                    &cchNewLine,
                                    STRSAFE_IGNORE_NULLS
                                    )))
            {
               bReplace = TRUE;

               BOOL bAppendNewLine = FALSE;
               if (wszLine[cchLine-1] == L'\n')
               {
                  cchLine--;
                  bAppendNewLine = TRUE;
               }

               // See if there are remaining characters to copy
               if (cchValue > (cchLine - cchKey))
               {
                  if (bAppendNewLine)
                  {
                     cchLine++;
                  }

                  pwszValue += (cchLine - cchKey);
                  if (FAILED(StringCchCopyNEx(
                                       wszLine,
                                       lengthof(wszLine),
                                       pwszValue,
                                       (cchValue - (cchLine - cchKey)),
                                       &pwsz,
                                       &cchLine,
                                       0
                                       )) ||
                     (bAppendNewLine &&
                     FAILED(StringCchCopy(pwsz, cchLine, L"\n"))) ||
                     FAILED(StringCchLength(wszLine, lengthof(wszLine), &cchLine)))
                  {
                     bReplace = FALSE;
                  }
               }
               else
               {
                  if (bAppendNewLine &&
                     FAILED(StringCchCopy(pwsz, cchNewLine, L"\n")))
                  {
                    bReplace = FALSE;
                  }
                  cchLine = 0;
               }

               if (FAILED(StringCchLength(wszNewLine, lengthof(wszNewLine), &cchNewLine)))
               {
                  bReplace = FALSE;
               }
            }

nextLine:
            if (bReplace)
            {
               BOOL bEOF = (BOOL)feof(fin);

               // Save read offset, then restore write offset
               if (fgetpos(fin, &posSrc) || fsetpos(fin, &posDest))
               {
                  goto closeFile;
               }

               if (cchNewLine)
               {
                  bSuccess = (WEOF != fputws(wszNewLine, fin));
                  if (cchLine &&
                     SUCCEEDED(StringCchCopyN(
                                          wszNewLine,
                                          lengthof(wszLine),
                                          wszLine,
                                          cchLine
                                          )))
                  {
                     cchNewLine = cchLine;
                  }
                  else
                  {
                     cchNewLine = 0;
                  }
               }
               else if (cchLine)
               {
                  bSuccess = (WEOF != fputws(wszLine, fin));
                  cchLine = 0;
               }

               // Save write offset, then restore read offset
               if (fgetpos(fin, &posDest) ||
                  bEOF ||
                  fsetpos(fin, &posSrc))
               {
                  goto closeFile;
               }
            }
            else
            {
               // Just update read and write offsets
               if (fgetpos(fin, &posSrc) || fgetpos(fin, &posDest))
               {
                  goto closeFile;
               }
            }
         }
      }

closeFile:
      if (bReplace && !fsetpos(fin, &posDest))
      {
         if (cchNewLine)
         {
            bSuccess = (WEOF != fputws(wszNewLine, fin));
         }
         SetEndOfFile(_fileno(fin));
      }

      fclose(fin);
   }

   SetFileAttributes(pwszIniFile, dwAttrib);

leave:
   return bSuccess;
}


