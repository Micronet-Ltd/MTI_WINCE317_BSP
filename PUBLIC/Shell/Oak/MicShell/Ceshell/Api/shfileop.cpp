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

#include "shfileop.h"
#include "cundo.h"
#include "idlist.h"
#include "shelldialogs.h"
#include <shellsdk.h>
#include "usersettings.h"
#include "iniutil.h"

const UINT CFileOperation::LOCKED_FOLDERS[] = { CSIDL_DESKTOPDIRECTORY, CSIDL_FAVORITES,
                                                CSIDL_FONTS, CSIDL_PERSONAL, CSIDL_PROGRAMS,
                                                CSIDL_STARTUP, CSIDL_WINDOWS, 0 };
const UINT CFileOperation::PROTECTED_FOLDERS[] = { CSIDL_PROGRAM_FILES, 0 };
const DWORD CFileOperation::LONGEST_PATH = MAX_PATH;
const ULONGLONG CCalculateTotalSize::SHOW_PROGRESS_THRESHOLD = (ULONGLONG) 50*1024;

WINSHELLAPI int WINAPI SHFileOperation(LPSHFILEOPSTRUCT lpFileOp)
{
   CFileOperation * pFileOperation = NULL;
   HRESULT hr = S_OK;

   if (!lpFileOp)
   {
      return 1;
   }

   // Currently supported flags
   FILEOP_FLAGS fSupportedFlags = (
      FOF_ALLOWUNDO |         // Preserve Undo information, if possible. If pFrom does not contain fully qualified path and file names, this flag is ignored.
      FOF_NOCONFIRMATION |    // Respond with "Yes to All" for any dialog box that is displayed.
      FOF_NOCONFIRMMKDIR |    // Do not confirm the creation of a new directory if the operation requires one to be created.
      FOF_NOERRORUI |         // Do not display a user interface if an error occurs.
      FOF_RENAMEONCOLLISION | // Give the file being operated on a new name in a move, copy, or rename operation if a file with the target name already exists.
      FOF_SILENT |            // Do not display a progress dialog box.
      FOF_SIMPLEPROGRESS);    // Display a progress dialog box but do not show the file names.

   // Unsupported flags
   // FOF_CONFIRMMOUSE
   // FOF_FILESONLY           // Perform the operation on files only if a wildcard file name (*.*) is specified.
   // FOF_MULTIDESTFILES
   // FOF_NOCOPYSECURITYATTRIBS
   // FOF_NORECURSION         // Only operate in the local directory. Don't operate recursively into subdirectories.

   // Unported flags
   // FOF_NO_CONNECTED_ELEMENTS
   // FOF_RECURSEREPARSE
   // FOF_NORECURSEREPARSE
   // FOF_WANTMAPPINGHANDLE
   // FOF_WANTNUKEWARNING


   if (lpFileOp->fFlags & (~fSupportedFlags))
   {
      return 1;
   }

   switch (lpFileOp->wFunc)
   {
      case FO_COPY:
         pFileOperation = new CCopy();
      break;

      case FO_MOVE:
         pFileOperation = new CMove();
      break;

      case FO_DELETE:
         pFileOperation = new CDelete();
      break;

      case FO_RENAME:
         pFileOperation = new CRename();
      break;

      default:
         return 1;
   }

   if (!pFileOperation)
   {
      return 1;
   }

   // Execute the file operation
   hr = pFileOperation->execute(lpFileOp);
   delete pFileOperation;

   return (SUCCEEDED(hr) ? 0 : 1);
}

HRESULT CDirectoryWalker::Walk(LPTSTR pszWalkRoot, const size_t cchWalkRoot, WalkAction * pWalkAction)
{
   LPTSTR pszFile = NULL; // Points to the filename in pszWalkRoot
   DWORD dwWalkRootAttrib = 0;
   WIN32_FIND_DATA fd = {0};
   HANDLE hFind = NULL;
   HRESULT hr = S_OK;
   BOOL fDoPop = TRUE;

   if (!pszWalkRoot || !pWalkAction)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   if (!cchWalkRoot)
   {
      return SHFO_ERR_INSUFFICIENT_BUFFER;
   }

   // Figure out what kind of path was passed to us and set pszFile
   dwWalkRootAttrib = GetFileAttributes(pszWalkRoot);
   if ((-1 != dwWalkRootAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwWalkRootAttrib))
   {
      // Pass the directory on to the action callback
      hr = pWalkAction->doDirectoryAction(WalkAction::PUSH_DIRECTORY,
                                          pszWalkRoot);

      // If the user wants us to ignore this directory, return now
      if (!SHFO_CONTINUE(hr))
      {
         return hr;
      }

      // Tack on the * for a full search and adjust pszCurrentFile
      hr = StringCchCatEx(pszWalkRoot, cchWalkRoot, TEXT("\\*"),
                          &pszFile, NULL, STRSAFE_NO_TRUNCATION);
      if (SUCCEEDED(hr))
      {
         pszFile--;
      }
      else
      {
         pszFile = NULL;
      }
   }
   else
   {
      // We are most likely dealing with a file or wild card string
      pszFile = (LPTSTR) TraverseBack(pszWalkRoot, 1);
      if (pszFile)
      {
         pszFile++;
      }
      else
      {
         hr = SHFO_ERR_MALFORMED_PATH;
      }
   }

   // Check for errors
   if (FAILED(hr))
   {
      goto walk_exit;
   }

   // We should have filename or wildcard here
   ASSERT(pszFile && *pszFile);

   // Bail if there is nothing in this directory to walk
   hFind = FindFirstFile(pszWalkRoot, &fd);
   if (INVALID_HANDLE_VALUE == hFind)
   {
      goto walk_exit;
   }

   // Walk the directory and callback the files
   do
   {
#ifndef UNDER_CE
      // Don't worry about these
      if (!_tcsicmp(TEXT("."), fd.cFileName) ||
          !_tcsicmp(TEXT(".."), fd.cFileName))
      {
         continue;
      }
#endif

      // Verify that the path will not get too long
      if (cchWalkRoot < ((pszFile-pszWalkRoot)+_tcslen(fd.cFileName)+1))
      {
         hr = SHFO_ERR_INSUFFICIENT_BUFFER;
         break; // We could continue here and hope the next file has a shorter name
      }

      // Build the new path and recurse down on it
      _tcscpy(pszFile, fd.cFileName);
      ASSERT(-1 != fd.dwFileAttributes);
      if (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)
      {
         hr = Walk(pszWalkRoot, cchWalkRoot, pWalkAction);
      }
      else
      {
         hr = pWalkAction->doFileAction(pszWalkRoot, fd);
      }

      // The user asked us to skip this operation, don't bother to pop
      if (!SHFO_CONTINUE(hr))
      {
         fDoPop = FALSE;
      }

   } while (SUCCEEDED(hr) && FindNextFile(hFind, &fd));
   FindClose(hFind);

walk_exit:
   // Pop out of the current directory
   if (fDoPop && (-1 != dwWalkRootAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwWalkRootAttrib))
   {
      if (pszFile)
      {
         *(--pszFile) = TEXT('\0');
      }
      
      hr = pWalkAction->doDirectoryAction(WalkAction::POP_DIRECTORY, pszWalkRoot);
   }

   return hr;
}

LPCTSTR CDirectoryWalker::TraverseBack(LPCTSTR pszPath, DWORD dwHowMany)
{
   if (!pszPath || !*pszPath)
   {
      return NULL;
   }

   LPTSTR pszVolumeRoot = NULL;
   LPTSTR pszTail = NULL;
   size_t strLen = 0;

   // Find The length of the path, this will also validate pszPath
   if (FAILED(StringCchLength(pszPath, CFileOperation::LONGEST_PATH, &strLen)))
   {
      return NULL;
   }
   ASSERT(strLen);

   // Find the volume root
   pszVolumeRoot = (LPTSTR) pszPath;
   // Skip anything like 'c:'
   while (TEXT('\\') != *pszVolumeRoot)
   {
      if (!*pszVolumeRoot)
      {
         return NULL;
      }
      pszVolumeRoot++;
   }
   // Check for network path '\\'
   if (TEXT('\\') == *(pszVolumeRoot+1))
   {
      ASSERT(pszVolumeRoot == pszPath);
      // Walk past the machine name and point pszVolumeRoot to the share name
      pszVolumeRoot += 2;
      while (TEXT('\\') != *pszVolumeRoot)
      {
         if (!*pszVolumeRoot)
         {
            return NULL;
         }
         pszVolumeRoot++;
      }
   }

   // Set the pointer to the end of the string
   pszTail = (LPTSTR) pszPath+strLen-1;

   // Do the loop for each directory we want to back up or return the volume root if we back up too far
   for (DWORD i = 0; i < dwHowMany; i++)
   {
      // Remove trailing '\'
      if (TEXT('\\') == *pszTail)
      {
         if (pszTail <= pszVolumeRoot)
         {
            return pszVolumeRoot;
         }
         pszTail--;
      }

      // Walk backwards looking for another '\'
      while (TEXT('\\') != *pszTail)
      {
         if (pszTail <= pszVolumeRoot)
         {
            return pszVolumeRoot;
         }
         pszTail--;
      }
   }

   return pszTail;   
}

BOOL CDirectoryWalker::FindParentDirectory(LPCTSTR pszPath, LPTSTR pszParent, const size_t cchParent)
{
   if (!pszPath || !pszParent)
   {
      return FALSE;
   }

   if (!cchParent)
   {
      return FALSE;
   }

   LPTSTR psz2Back = (LPTSTR) CDirectoryWalker::TraverseBack(pszPath, 2);
   LPTSTR psz1Back = (LPTSTR) CDirectoryWalker::TraverseBack(pszPath, 1);
   if (!psz2Back || !psz1Back)
   {
      pszParent = TEXT('\0');
      return FALSE;
   }

   if (psz2Back == psz1Back)
   {
      if (FAILED(StringCchCopy(pszParent, cchParent, pszPath)))
      {
         pszParent = TEXT('\0');
         return FALSE;
      }
      pszParent[psz1Back-pszPath+1] = TEXT('\0'); // Chop off any trailing characters
   }
   else
   {
      psz2Back++; // Don't pick up the \ 
      if (cchParent < (size_t)(psz1Back-psz2Back+1))
      {
         pszParent = TEXT('\0');
         return FALSE;
      }

      memcpy(pszParent, psz2Back, (psz1Back-psz2Back)*sizeof(TCHAR));
      pszParent[psz1Back-psz2Back] = TEXT('\0');
   }

   return TRUE;
}

BOOL CDirectoryWalker::PathIsVolumeRoot(LPCTSTR pszPath)
{
   if (!pszPath || !*pszPath)
   {
      return FALSE;
   }

   // Find The length of the path, this will also validate pszPath
   size_t strLen = 0;
   if (FAILED(StringCchLength(pszPath, CFileOperation::LONGEST_PATH, &strLen)))
   {
      return FALSE;
   }

   LPTSTR pszCurrent = (LPTSTR) pszPath;

   // Walk past the end of the root
   if (TEXT('\\') == *pszCurrent)
   {
      pszCurrent++;
      if (!*pszCurrent)
      {
         return TRUE; // The path is a regular ce root '\'
      }

      // Check for network path '\\'
      if (TEXT('\\') == *pszCurrent)
      {
         pszCurrent++;
         // Walk past the machine name
         while (TEXT('\\') != *pszCurrent)
         {
            if (!*pszCurrent)
            {
               return FALSE; // Incomplete path \\foo
            }
            pszCurrent++;
         }

         pszCurrent++;
         // Walk past share name
         while (TEXT('\\') != *pszCurrent)
         {
            if (!*pszCurrent)
            {
               return TRUE; // A good network volume root '\\foo\bar'
            }
            pszCurrent++;
         }

         pszCurrent++;
         // This handles the case where the path is '\\foo\bar\'
         if (!*pszCurrent)
         {
            return TRUE; // A good network volume root '\\foo\bar\'
         }
      }
   }
   else
   {
      // Skip anything like 'c:'
      while (TEXT('\\') != *pszCurrent)
      {
         if (!*pszCurrent)
         {
            return FALSE; // This is probably just a text string 'xyz'
         }
         pszCurrent++;
      }

      pszCurrent++;
      if (!*pszCurrent)
      {
         return TRUE; // Must be a desktop style root 'c:\'
      }
   }

   return FALSE; // Some valid path that is not a volume root
}

HRESULT CCalculateTotalSize::doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom)
{
   if (m_pProgressUI)
   {
      // Check to see if the user canceled the operation
      if (PROGRESS_CANCEL == m_pProgressUI->getStatus())
      {
         return SHFO_ERR_CANCELLED;
      }
   }

   return S_OK;
}

HRESULT CCalculateTotalSize::doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd)
{
   m_ullTotalSize += (((ULONGLONG)fd.nFileSizeHigh*(ULONGLONG)MAXDWORD) +
                      (ULONGLONG)fd.nFileSizeLow);

   if (m_pProgressUI)
   {
      // Show the dialog for operations over the threshold
      if (SHOW_PROGRESS_THRESHOLD < m_ullTotalSize)
      {
         if (!m_pProgressUI->isVisible())
         {
            m_pProgressUI->showDialog();
         }
      }

      // Check to see if the user canceled the operation
      if (PROGRESS_CANCEL == m_pProgressUI->getStatus())
      {
         return SHFO_ERR_CANCELLED;
      }
   }

   return S_OK;
}

CProgressUI::CProgressUI() :
   m_hParent(NULL),
   m_hDialog(NULL),
   m_hSyncEvent(NULL),
   m_status(PROGRESS_CONTINUE),
   m_dwStartTime(0)
{
}

CProgressUI::~CProgressUI()
{
   if (m_hDialog)
   {
      destroyDialog();
   }
}

DWORD WINAPI CProgressUI::dialogThread(LPVOID lpVoid)
{
   CProgressUI * pThis = reinterpret_cast<CProgressUI *>(lpVoid);
   if (pThis && !pThis->m_hDialog)
   {
      INITCOMMONCONTROLSEX iccex = {0};
      HWND hDialog = NULL;
      MSG msg = {0};

      iccex.dwSize = sizeof(iccex);
      iccex.dwICC = ICC_PROGRESS_CLASS;
      InitCommonControlsEx(&iccex);

      hDialog = CreateDialogParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_FILEOP_PROGRESS),
                                  pThis->m_hParent, dialogProc, (LPARAM) pThis);
      ASSERT(hDialog && pThis->m_hDialog && (hDialog == pThis->m_hDialog));

      while (::GetMessage(&msg, NULL, 0, 0))
      {
         if (!::IsDialogMessage(hDialog, &msg))
         {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
         }

         if (!::IsWindow(hDialog) || (GetWindowLong(hDialog, GWL_USERDATA) != reinterpret_cast<LONG>(pThis)))
         {
            // The dialog has been destroyed or USERDATA no longer points to pThis
            // which means the dialog must have been destroyed and reused. This
            // should only happen on a WM_CLOSE or a WM_DESTROY.
            ASSERT((WM_CLOSE == msg.message) || (WM_DESTROY == msg.message));
            break;
         }
      }
   }
   return (DWORD) -1;
}

BOOL CALLBACK CProgressUI::dialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CProgressUI * pThis = reinterpret_cast<CProgressUI *>(GetWindowLong(hDlg, GWL_USERDATA));

   if (!pThis && (uMsg != WM_INITDIALOG))
   {
      goto DoDefault;
   }

   switch (uMsg)
   {
      case WM_INITDIALOG:
         ASSERT(!pThis);
         pThis = reinterpret_cast<CProgressUI *>(lParam);
         pThis->m_hDialog = hDlg;
         ::SetWindowLong(hDlg, GWL_USERDATA, (LONG) pThis);
         ::SendMessage(GetDlgItem(hDlg, IDC_FILEOP_PROGRESS), PBM_SETRANGE32, 0, (LPARAM) 100);
         ::SendMessage(GetDlgItem(hDlg, IDC_FILEOP_PROGRESS), PBM_SETPOS, 0, 0);
         
         AygInitDialog( hDlg, SHIDIF_SIPDOWN );
         
         if (pThis->m_hSyncEvent)
         {
            SetEvent(pThis->m_hSyncEvent);
         }
      break;

      case WM_COMMAND:
         if (IDCANCEL == LOWORD(wParam))
         {
            pThis->m_status = PROGRESS_CANCEL;
            pThis->hideDialog();
            ::DestroyWindow(hDlg);
         }
      break;

      case WM_CLOSE:
         ::DestroyWindow(hDlg);
      break;

      case WM_DESTROY:
         pThis->m_dwStartTime = 0;
         pThis->m_hDialog = NULL;
         ::SetWindowLong(hDlg, GWL_USERDATA, 0);
         if (pThis->m_hSyncEvent)
         {
            ::SetEvent(pThis->m_hSyncEvent);
         }
      break;

DoDefault:
      default:
         return FALSE;
   }

   return TRUE;
}

void CProgressUI::createDialog(HWND hParent)
{
   if (m_hDialog)
   {
      return;
   }

   m_hParent = hParent;
   m_hSyncEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
   if (m_hSyncEvent)
   {
      HANDLE hThread = ::CreateThread(NULL, 0, dialogThread, this, 0, NULL);
      if (hThread)
      {
         ::WaitForSingleObject(m_hSyncEvent, INFINITE);
         ::CloseHandle(hThread);
      }
      ::CloseHandle(m_hSyncEvent);
      m_hSyncEvent = NULL;
   }
}

void CProgressUI::destroyDialog()
{
   if (m_hDialog)
   {
      m_hSyncEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
      if (m_hSyncEvent)
      {
         ::PostMessage(m_hDialog, WM_CLOSE, 0, 0);
         ::WaitForSingleObject(m_hSyncEvent, INFINITE);
      }
      ::CloseHandle(m_hSyncEvent);
      m_hSyncEvent = NULL;
   }
}

void CProgressUI::showDialog()
{
   if (!m_hDialog || isVisible())
   {
      return;
   }

   ::ShowWindow(m_hDialog, SW_SHOW);
}

void CProgressUI::hideDialog()
{
   if (!m_hDialog || !isVisible())
   {
      return;
   }

   ::ShowWindow(m_hDialog, SW_HIDE);
}

BOOL CProgressUI::isVisible()
{
   if (!m_hDialog)
   {
      return FALSE;
   }

   return ::IsWindowVisible(m_hDialog);
}

void CProgressUI::setTitleString(LPCTSTR pszTitle) const
{
   if (!m_hDialog || !pszTitle)
   {
      return;
   }

   HWND hDlgItem = ::GetDlgItem(m_hDialog, IDC_TITLE);
   ASSERT(hDlgItem);
   ::SendMessage(hDlgItem, WM_SETTEXT, 0, (LPARAM) pszTitle);
}

void CProgressUI::setInfoString(LPCTSTR pszInfo) const
{
   if (!m_hDialog || !pszInfo)
   {
      return;
   }

   HWND hDlgItem = ::GetDlgItem(m_hDialog, IDC_INFO);
   ASSERT(hDlgItem);
   ::SendMessage(hDlgItem, WM_SETTEXT, 0, (LPARAM) pszInfo);
}

// TODO: Keep status to avoid unnecessary flicker
void CProgressUI::updateProgress(DWORD dwPercent)
{
   if (!m_hDialog)
   {
      return;
   }

   if (!m_dwStartTime)
   {
      m_dwStartTime = GetTickCount();
   }

   // Use one-digit precision. 
   // This improves TimeRemaining estimates for larger files
   ASSERT(1000 >= dwPercent);
   if (1000 < dwPercent)
   {
      dwPercent = 1000;
   }

   // TOTAL LEFT = TIME SO FAR * (1000 - PROGRESS) / PROGRESS
   // Assumption: Nobody copies for 49.7 days. 
   // Handle tickcount rollover anyways (operation might start just before it happens)
   DWORD dwTick = GetTickCount();
   ULONGLONG ulGuess;

   if(dwTick > m_dwStartTime)
   {
      ulGuess = (dwTick-m_dwStartTime);
   }
   else
   {
      ulGuess = (dwTick + (~m_dwStartTime)+1);
   }

   if ((0 < ulGuess) && (0 < dwPercent))
   {
      ulGuess *= (1000 - dwPercent);
      ulGuess /= dwPercent;
      ulGuess /= (1000*60); // Convert to minutes

      HRESULT hr;
      TCHAR szTimeRemaining[128] = TEXT("");
      LPTSTR pszTimeRemaining = NULL;
      if (0 < (UINT)ulGuess)
      {
         pszTimeRemaining = LOAD_STRING(IDS_SHFO_PROGRESS_X_MINUTES);
         if (pszTimeRemaining)
         {
            hr = ::StringCchPrintf(szTimeRemaining,
                                            lengthof(szTimeRemaining),
                                            pszTimeRemaining,
                                            (UINT)ulGuess);
         }
      }
      else
      {
         pszTimeRemaining = LOAD_STRING(IDS_SHFO_PROGRESS_LT_MINUTE);
         if (pszTimeRemaining)
         {
            hr = ::StringCchCopy(szTimeRemaining,
                                            lengthof(szTimeRemaining),
                                            pszTimeRemaining);
         }
      }
      ::SendMessage(GetDlgItem(m_hDialog, IDC_TIME), WM_SETTEXT,
                    0, (LPARAM) szTimeRemaining);
   }
   else
   {
      ::SendMessage(GetDlgItem(m_hDialog, IDC_TIME), WM_SETTEXT, 0, (LPARAM) TEXT(""));
   }

   // It is OK to use percentile number in ProgressBar
   dwPercent /= 10;
   ::SendMessage(GetDlgItem(m_hDialog, IDC_FILEOP_PROGRESS), PBM_SETPOS, (WPARAM) dwPercent, 0);

   if (100 <= dwPercent)
   {
      destroyDialog();
   }
}

HRESULT CFileOperation::validate()
{
   if (!m_pSHFileOpStruct || !m_pSHFileOpStruct->pFrom)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   return S_OK;
}

HRESULT CFileOperation::doSpecialFolderCheck(LPCTSTR pszPath)
{
   HRESULT hr = S_OK;
   TCHAR szProtected[MAX_PATH];

   for (int i = 0; LOCKED_FOLDERS[i]; i++)
   {
      if (SHGetSpecialFolderPath(m_pSHFileOpStruct->hwnd, szProtected,
                                 LOCKED_FOLDERS[i], FALSE))
      {
         if (0 == _tcsicmp(pszPath, szProtected))
         {
            if (!(FOF_NOERRORUI & m_fFlags))
            {
               WCHAR szError[256];
               if (SUCCEEDED(::StringCchPrintfEx(szError,
                                               lengthof(szError),
                                               NULL,
                                               NULL,
                                               STRSAFE_IGNORE_NULLS,
                                               LOAD_STRING(IDS_SHFO_ERR_FOLDERLOCKED),
                                               PathFindFileName(pszPath))))
               {
                  ::MessageBox(NULL, szError,
                               LOAD_STRING(IDS_TITLE_PROTECTEDFOLDER),
                               MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
               }
            }

            hr = S_FALSE;
            return hr;
         }
      }
   }

   for (int j = 0; PROTECTED_FOLDERS[j]; j++)
   {
      if (SHGetSpecialFolderPath(m_pSHFileOpStruct->hwnd, szProtected,
                                 PROTECTED_FOLDERS[j], FALSE))
      {
         if (0 == _tcsicmp(pszPath, szProtected))
         {
            int result = IDOK;

            if (!(FOF_NOCONFIRMATION & m_fFlags))
            {
               result = ::MessageBox(NULL,
                                     LOAD_STRING(IDS_CONFIRM_GEN_PROTECTED),
                                     LOAD_STRING(IDS_TITLE_PROTECTEDFOLDER),
                                     MB_ICONEXCLAMATION | MB_YESNO | MB_SETFOREGROUND);
            }

            if (IDNO == result)
            {
               hr = S_FALSE;
            }

            return hr;
         }
      }
   }

   return hr;
}

BOOL CFileOperation::buildErrorString(HRESULT hrError, LPTSTR pszHeader, LPTSTR pszPath, LPTSTR pszError, size_t cchError)
{
   HRESULT hr;
   TCHAR szError[256] = TEXT("");

   ASSERT(pszHeader);
   ASSERT(pszPath);
   ASSERT(pszError);
   ASSERT(0 < cchError);

   if (!pszHeader || !pszPath || !pszError || (0 == cchError))
   {
      return FALSE;
   }

   if (SUCCEEDED(hrError) || (SHFO_ERR_CANCELLED == hrError))
   {
      return FALSE;
   }

   // Select the error to use, if the error string does not requre pszExtended
   // delete and NULL it here
   switch (hrError)
   {
      case SHFO_ERR_ACCESS_DENIED:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_ACCESS));
      break;

      case SHFO_ERR_ALREADY_EXISTS:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_ALREADYEXISTS));
      break;

      case SHFO_ERR_DEST_IS_SUBFOLDER:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_DSTISSUBFOLDER));
      break;

      case SHFO_ERR_DISK_FULL:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_DISKFULL));
      break;

      case SHFO_ERR_FILE_NOT_FOUND:
      case SHFO_ERR_PATH_NOT_FOUND:
      {
         DWORD dwAttrib = ::GetFileAttributes(pszPath);
         if ((-1 != dwAttrib) && (dwAttrib & FILE_ATTRIBUTE_ROMMODULE))
         {
            hr = ::StringCchCopy(szError, lengthof(szError),
                                 LOAD_STRING(SHFO_ERR_INROM()));
         }
         else
         {
            hr = ::StringCchCopy(szError, lengthof(szError),
                                 LOAD_STRING(IDS_SHFO_ERR_FILENOTFOUND));
         }
      }
      break;

      case SHFO_ERR_MALFORMED_PATH:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_FILENOTFOUND));
      break;

      case SHFO_ERR_INVALID_FILENAME:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_BADNAME));
      break;

      case SHFO_ERR_FILENAME_EXCED_RANGE:
      case SHFO_ERR_INSUFFICIENT_BUFFER:
      case SHFO_ERR_INVALID_REQUEST:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_INVALID));
      break;

      case SHFO_ERR_SHARING_VIOLATION:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_SHARING));
      break;

      case E_OUTOFMEMORY:
         hr = ::StringCchCopy(szError, lengthof(szError),
                              LOAD_STRING(IDS_SHFO_ERR_OOM));
      break;

      case SHFO_ERR_INVALID_DATA:
      case SHFO_ERR_INVALID_FLAGS:
      case SHFO_ERR_INVALID_PARAMETER:
      default:
      {
         TCHAR szFormatMessage[128] = TEXT("");
         ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL, HRESULT_CODE(hrError), 0, szFormatMessage,
                         lengthof(szFormatMessage), NULL);

         hr = ::StringCchPrintfEx(szError,
                                lengthof(szError),
                                NULL,
                                NULL,
                                STRSAFE_IGNORE_NULLS,
                                LOAD_STRING(IDS_SHFO_ERR_GENERIC),
                                HRESULT_CODE(hrError),
                                szFormatMessage);
      }
   }

   if (SUCCEEDED(hr))
   {
      hr = ::StringCchPrintfEx(pszError,
                             cchError,
                             NULL,
                             NULL,
                             STRSAFE_IGNORE_NULLS,
                             TEXT("%s%s"),
                             pszHeader,
                             szError);
   }

   return SUCCEEDED(hr);
}

void CFileOperation::showError(HRESULT hr)
{
   // Don't show an error UI
   if (FOF_NOERRORUI & m_fFlags)
   {
      return;
   }

   // No error or the user canceled
   if (SUCCEEDED(hr) || (SHFO_ERR_CANCELLED == hr))
   {
      return;
   }

   TCHAR szErrorString[38]; // The exact buffer length is known
   if (SUCCEEDED(::StringCchPrintf(szErrorString,
                                   lengthof(szErrorString),
                                   TEXT("An unknown error occured (0x%8.8x)"),
                                   HRESULT_CODE(hr))))
   {
      ::MessageBox(NULL, szErrorString, NULL,
                   MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
   }
}

HRESULT CFileOperation::execute(LPSHFILEOPSTRUCT pSHFileOpStruct)
{
   HRESULT hr;

   if (!pSHFileOpStruct)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   m_pSHFileOpStruct = pSHFileOpStruct;
   m_fFlags = m_pSHFileOpStruct->fFlags;

   // Set up the CUndo object
   if (FOF_ALLOWUNDO & m_fFlags)
   {
      if ( (FO_DELETE == m_pSHFileOpStruct->wFunc) && !UserSettings::GetUseRecycleBin())
      {
         return SHFO_ERR_INVALID_DATA;
      }

      if (!g_pUndo)
      {
         g_pUndo = new CUndo();
         if (!g_pUndo)
         {
            return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
         }
      }

      // CUndo is shared by threads which call SHFileOperation.
      // Clear the list created before the last started thread and mark a new operation.
      if (ERROR_SUCCESS != g_pUndo->Open(m_pSHFileOpStruct->wFunc))
      {
         m_fFlags &= ~FOF_ALLOWUNDO;
      }
   }

   hr = validate();

   if (SHFO_CONTINUE(hr))
   {
      hr = doOperation();
   }

   showError(hr);

   // Let the user know if the operation was canceled
   pSHFileOpStruct->fAnyOperationsAborted = (SHFO_ERR_CANCELLED == hr);

   // Close the undo object
   if (FOF_ALLOWUNDO & m_fFlags)
   {
      g_pUndo->Close();
   }

   return hr;
}

CWalkingFileOperation::CWalkingFileOperation() :
   m_pszCurrentFrom(NULL),
   m_dwCount(0),
   m_ullTotalSize(0),
   m_ullCurrentSize(0)
{
}

HRESULT CWalkingFileOperation::validate()
{
   HRESULT hr = CFileOperation::validate();
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   // Validate the set of from paths here
   size_t strLen = 0;
   LPTSTR pszFrom = (LPTSTR) m_pSHFileOpStruct->pFrom;
   while (TEXT('\0') != *pszFrom)
   {
      hr = StringCchLength(pszFrom, LONGEST_PATH, &strLen);
      if (FAILED(hr))
      {
         break;
      }

      // Verify that the path exists or is a wild card
      if (!PathFileExists(pszFrom) &&
          !_tcschr(PathFindFileName(pszFrom), TEXT('*')))
      {
         hr = SHFO_ERR_PATH_NOT_FOUND;
         break;
      }

      // Verify that the path is not a volume root
      if (CDirectoryWalker::PathIsVolumeRoot(pszFrom))
      {
         hr = SHFO_ERR_MALFORMED_PATH;
         break;
      }

      m_dwCount++;
      pszFrom += (strLen+1);
   }
   ASSERT(0 < m_dwCount);

   // Set m_pszCurrentFrom since showError needs it
   if (FAILED(hr))
   {
      ASSERT(!m_pszCurrentFrom);
      setCurrentFrom(pszFrom);
   }

   return hr;
}

HRESULT CWalkingFileOperation::initialize()
{
   HRESULT hr = S_OK;

   // Bring up the dialog so the user sees that work is being done
   if (!(FOF_SILENT & m_fFlags))
   {
      m_progressUI.createDialog(m_pSHFileOpStruct->hwnd);

      if (FOF_SIMPLEPROGRESS & m_fFlags)
      {
         // If you are going to use FOF_SIMPLEPROGRESS you need to set a title in SHFILEOPSTRUCT
         ASSERT(m_pSHFileOpStruct->lpszProgressTitle);
         m_progressUI.setTitleString(m_pSHFileOpStruct->lpszProgressTitle);
      }
      else
      {
         m_progressUI.setInfoString(LOAD_STRING(SHFO_PROGRESS_PREPARING()));
      }
   }

   // Walk the whole structure once to determine the total size of all files in the directory structure
   CCalculateTotalSize totalSize(&m_progressUI);
   TCHAR szCurrent[LONGEST_PATH];
   size_t strLen = 0;
   LPTSTR pszCurrent = (LPTSTR) m_pSHFileOpStruct->pFrom;
   while (pszCurrent && (TEXT('\0') != *pszCurrent))
   {  
      hr = StringCchCopy(szCurrent, lengthof(szCurrent), pszCurrent);
      if (FAILED(hr))
      {
         return hr;
      }

      hr = CDirectoryWalker::Walk(szCurrent, lengthof(szCurrent), &totalSize);
      if (FAILED(hr))
      {
         return hr;
      }

      hr = StringCchLength(pszCurrent, LONGEST_PATH, &strLen);
      if (FAILED(hr))
      {
         return hr;
      }

      pszCurrent += (strLen+1);
   }

   m_ullTotalSize = totalSize.getTotalSize();

   return hr;
}

HRESULT CWalkingFileOperation::setCurrentFrom(LPCTSTR pszCurrentFrom)
{
   if (!pszCurrentFrom)
   {
      if (m_pszCurrentFrom)
      {
         delete [] m_pszCurrentFrom;
         m_pszCurrentFrom = NULL;
      }
      return S_OK;
   }

   if (!m_pszCurrentFrom)
   {
      m_pszCurrentFrom = new TCHAR[LONGEST_PATH];
      ASSERT(m_pszCurrentFrom);
      if (!m_pszCurrentFrom)
      {
         return E_OUTOFMEMORY;
      }
   }

   return StringCchCopy(m_pszCurrentFrom, LONGEST_PATH, pszCurrentFrom);
}

HRESULT CWalkingFileOperation::doOperation()
{
   HRESULT hr = initialize();
   if (SHFO_CONTINUE(hr))
   {
      // Walk the from list
      size_t strLen = 0;
      LPTSTR pszFrom = (LPTSTR) m_pSHFileOpStruct->pFrom;
      while (SUCCEEDED(hr) && (TEXT('\0') != *pszFrom))
      {
         hr = setCurrentFrom(pszFrom);
         if (SHFO_CONTINUE(hr))
         {
           hr = CDirectoryWalker::Walk(m_pszCurrentFrom, LONGEST_PATH, this);
         }

         if (FAILED(hr))
         {
            break;
         }

         hr = StringCchLength(pszFrom, LONGEST_PATH, &strLen);
         if (FAILED(hr))
         {
            break;
         }

         pszFrom += (strLen+1);
      }
   }

   if (!SHFO_CONTINUE(hr))
   {
      m_progressUI.destroyDialog();
   }

   return hr;
}


DWORD CALLBACK CCopyMove::ProgressCallback(LARGE_INTEGER TotalFileSize,
                                           LARGE_INTEGER TotalBytesTransferred,
                                           LARGE_INTEGER StreamSize,
                                           LARGE_INTEGER StreamBytesTransferred,
                                           DWORD dwStreamNumber,
                                           DWORD dwCallbackReason,
                                           HANDLE hSourceFile,
                                           HANDLE hDestinationFile,
                                           LPVOID lpVoid)
{
   CCopyMove * pThis = reinterpret_cast<CCopyMove *>(lpVoid);
   PREFAST_ASSERT(pThis);

   if (CALLBACK_STREAM_SWITCH == dwCallbackReason)
   {
      pThis->m_ullCurrentSize += (ULONGLONG) TotalFileSize.QuadPart;
   }

   // PROGRESS = CURRENT * 1000 / TOTAL
   ULONGLONG ullProgress = 0;
   ullProgress = (pThis->m_ullCurrentSize - (ULONGLONG) TotalFileSize.QuadPart);
   ullProgress += (ULONGLONG) TotalBytesTransferred.QuadPart;
   if ((ULONGLONG) 0 < ullProgress)
   {
      ullProgress *= (ULONGLONG) 1000;
      ullProgress /= pThis->m_ullTotalSize;
   }

   pThis->m_progressUI.updateProgress((DWORD) ullProgress);
   return pThis->m_progressUI.getStatus();
}

HRESULT CCopyMove::createDirectory(LPCTSTR pszPath, BOOL fDoConfirmMkDirCheck)
{
   TCHAR szPathCopy[LONGEST_PATH];
   size_t strLen = 0;
   DWORD dwAttrib = (DWORD) -1;
   HRESULT hr = S_OK;

   // Validate pszPath
   if (!pszPath)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   hr = StringCchLength(pszPath, LONGEST_PATH, &strLen);
   if (FAILED(hr))
   {
      return hr;
   }

   if (0 == strLen)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   // Make sure the user is ok with this
   if (fDoConfirmMkDirCheck && !(FOF_NOCONFIRMMKDIR & m_fFlags))
   {
      LPTSTR pszFormat = LOAD_STRING(IDS_SHFO_PROMPT_CREATEFOLDER);
      LPTSTR pszMessage = NULL;

      if (!pszFormat)
      {
         return E_OUTOFMEMORY;
      }

      ASSERT(_tcschr(pszFormat, TEXT('%')));
      size_t bufferLen = _tcslen(pszFormat) - 2 + strLen + 1; // -2 for the %s +1 for the \0
      pszMessage = new TCHAR[bufferLen];
      if (!pszMessage)
      {
         return E_OUTOFMEMORY;
      }

      hr = ::StringCchPrintf(pszMessage,
                                bufferLen,
                                pszFormat,
                                pszPath);
      if (FAILED(hr))
      {
         delete [] pszMessage;
         return hr;
      }

      int result = ::MessageBox(m_pSHFileOpStruct->hwnd, pszMessage,
                                LOAD_STRING(IDS_NEWFOLDER),
                                MB_ICONEXCLAMATION | MB_YESNO | MB_SETFOREGROUND);

      delete [] pszMessage;

      if (IDNO == result)
      {
         return SHFO_ERR_CANCELLED;
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      return SHFO_ERR_CANCELLED;
   }

   // Create a copy of 'pszPath' since it came in as const
   hr = StringCchCopy(szPathCopy, lengthof(szPathCopy), pszPath);
   if (FAILED(hr))
   {
      return hr;
   }

   // The folder already exists figure out what the user wants us to do
   dwAttrib = GetFileAttributes(szPathCopy);
   if (-1 != dwAttrib)
   {
      // Make sure the requested path is not an existing file
      if (!(FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
      {
         return SHFO_ERR_INVALID_REQUEST;
      }

      if (FOF_RENAMEONCOLLISION & m_fFlags)
      {
         // Rename
         TCHAR szToUnique[MAX_PATH]; // Per PathMakeUniqueName spec
         if (!PathMakeUniqueName(szPathCopy,
                                 LOAD_STRING(IDS_FILECOPY_COPY),
                                 LOAD_STRING(IDS_FILECOPY_OF),
                                 FALSE, szToUnique, lengthof(szToUnique)))
         {
            return HRESULT_FROM_WIN32_I(GetLastError());
         }

         hr = StringCchCopy(szPathCopy, lengthof(szPathCopy), szToUnique);
         if (FAILED(hr))
         {
            return hr;
         }

         dwAttrib = (DWORD) -1; // Reset because szPathCopy has a new name
      }
      else if (!(FOF_NOCONFIRMATION & m_fFlags))
      {
         int result = IDYES;
         hr = ShellDialogs::Confirm::Merge(szPathCopy,
                                           m_pSHFileOpStruct->hwnd,
                                           (1 == m_dwCount),
                                           &result);
         if (FAILED(hr))
         {
            return hr;
         }

         switch (result)
         {
            case IDNO:
               return S_FALSE;
            break;

            case IDCANCEL:
               return SHFO_ERR_CANCELLED;
            break;

            case IDYESTOALL:
               m_fFlags |= FOF_NOCONFIRMATION;
            break;
         }

         // The user wants us to combine the directories, no need to create
         return S_OK;
      }
      else
      {
         // The user wants us to combine the directories, no need to create
         return S_OK;
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      return SHFO_ERR_CANCELLED;
   }

   // Find the first root that exists
   LPTSTR psz = NULL;
   while (-1 == dwAttrib)
   {
      psz = (LPTSTR) CDirectoryWalker::TraverseBack(szPathCopy, 1);
      ASSERT(psz);
      if (!psz)
      {
         return SHFO_ERR_MALFORMED_PATH;
      }

      *psz = TEXT('\0');

      dwAttrib = GetFileAttributes(szPathCopy);
   }

   // Now create the directory path
   while (psz < (szPathCopy + strLen))
   {
      *psz = TEXT('\\');

      if (!CreateDirectory(szPathCopy, NULL))
      {
         return HRESULT_FROM_WIN32_I(GetLastError());
      }

      psz += _tcslen(psz);     
   }

   return S_OK;
}

HRESULT CCopyMove::confirmTo(LPCTSTR pszFrom, LPTSTR pszTo, size_t cchTo)
{
   HRESULT hr = S_OK;
   DWORD dwToAttrib = GetFileAttributes(pszTo);

   // Verify that to is a valid filename
   if (!PathIsValidFileName(PathFindFileName(pszTo)))
   {
      return SHFO_ERR_INVALID_FILENAME;
   }

   // The to file already exists figure out what the user wants us to do
   if ((-1 != dwToAttrib) && !(FILE_ATTRIBUTE_DIRECTORY & dwToAttrib))
   {
      if (FOF_RENAMEONCOLLISION & m_fFlags)
      {
         // Rename
         TCHAR szToUnique[MAX_PATH]; // Per PathMakeUniqueName spec
         if (PathMakeUniqueName(pszTo,
                                LOAD_STRING(IDS_FILECOPY_COPY),
                                LOAD_STRING(IDS_FILECOPY_OF),
                                FALSE, szToUnique, lengthof(szToUnique)))
         {
            hr = StringCchCopy(pszTo, cchTo, szToUnique);
         }
         else
         {
            hr = HRESULT_FROM_WIN32_I(GetLastError());
         }

         if (FAILED(hr))
         {
            return hr;
         }
      }
      else if (!_tcsicmp(pszFrom, pszTo))
      {
         // If the to and the from are the same, bail out here
         return SHFO_ERR_ALREADY_EXISTS;
      }
      else if (FOF_NOCONFIRMATION & m_fFlags)
      {
         // Blindly overwrite
         dwToAttrib &= (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));
         SetFileAttributes(pszTo, dwToAttrib);
         DeleteFile(pszTo);
      }
      else // !(FOF_NOCONFIRMATION & m_fFlags)
      {
         int result = IDYES;

         ShellDialogs::Confirm::Replace(pszFrom, pszTo,
                                        m_pSHFileOpStruct->hwnd,
                                        &result);
         switch (result)
         {
            case IDNO:
               return S_FALSE;
            break;

            case IDCANCEL:
               return SHFO_ERR_CANCELLED;
            break;

            case IDYESTOALL:
               m_fFlags |= FOF_NOCONFIRMATION;
            // fall through to YES case

            default: // IDYES
               dwToAttrib &= (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));
               SetFileAttributes(pszTo, dwToAttrib);
               DeleteFile(pszTo);
         }
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      return SHFO_ERR_CANCELLED;
   }

   ASSERT(S_OK == hr); // we return on all of the error cases
   return hr;
}

HRESULT CCopyMove::validate()
{
   HRESULT hr = CWalkingFileOperation::validate();
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   if (!m_pSHFileOpStruct->pTo)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   // Validate the set of from paths here
   size_t strLen = 0;
   LPTSTR pszFrom = (LPTSTR) m_pSHFileOpStruct->pFrom; // validated in CFileOperation::validate
   while (TEXT('\0') != *pszFrom)
   {
      hr = StringCchLength(pszFrom, LONGEST_PATH, &strLen);
      if (FAILED(hr))
      {
         break;
      }

      if  (!_tcsnicmp(pszFrom, m_pSHFileOpStruct->pTo, strLen))
      {
         // Verify that the 'to' is is not the same as the 'from'
         if (TEXT('\0') == m_pSHFileOpStruct->pTo[strLen])
         {
            hr = SHFO_ERR_ALREADY_EXISTS;
            break;
         }

         // Verify that the 'to' is not a subfolder of 'from'
         if (TEXT('\\') == m_pSHFileOpStruct->pTo[strLen])
         {
            hr = SHFO_ERR_DEST_IS_SUBFOLDER;
            break;
         }
      }

      pszFrom += (strLen+1);
   }

   // Set m_pszCurrentFrom since showError needs it
   if (FAILED(hr))
   {
      ASSERT(!m_pszCurrentFrom);
      CWalkingFileOperation::setCurrentFrom(pszFrom);
   }

   return hr;
}

HRESULT CCopyMove::initialize()
{
   HRESULT hr = CWalkingFileOperation::initialize();
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   if (!(FOF_NOCONFIRMATION & m_fFlags))
   {
      // Verify that the total size isn't larger that the size of the store
      ULARGE_INTEGER uliFreeBytesAvailableToCaller = {0};
      ULARGE_INTEGER uliTotalNumberOfBytes = {0};
      if (::GetDiskFreeSpaceEx(m_pSHFileOpStruct->pTo, &uliFreeBytesAvailableToCaller,
                               &uliTotalNumberOfBytes, NULL))
      {
         if (uliFreeBytesAvailableToCaller.QuadPart < m_ullTotalSize)
         {
            int result = ::MessageBox(m_pSHFileOpStruct->hwnd,
                                      LOAD_STRING(IDS_SHFO_PROMPT_TOOMUCHDATA),
                                      LOAD_STRING(IDS_TITLE_FILEOPERATION),
                                      MB_ICONEXCLAMATION | MB_YESNO | MB_SETFOREGROUND);

            if (IDNO == result)
            {
               hr = SHFO_ERR_CANCELLED;
            }
         }
      }
   }

   return hr;
}

HRESULT CCopyMove::setCurrentFrom(LPCTSTR pszCurrentFrom)
{
   HRESULT hr = CWalkingFileOperation::setCurrentFrom(pszCurrentFrom);
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   if (!m_pszCurrentFrom || !m_pSHFileOpStruct->pTo)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   // Clear the replaced to string if it exists
   if (m_pszTo)
   {
      delete [] m_pszTo;
      m_pszTo = NULL;
   }

   DWORD dwFromAttrib = GetFileAttributes(m_pszCurrentFrom);
   DWORD dwToAttrib = GetFileAttributes(m_pSHFileOpStruct->pTo);

   // Based on the current status of the from and to, fixup our path pointers
   if (-1 == dwFromAttrib) // from is a wild card
   {
      if ((-1 == dwToAttrib) || (FILE_ATTRIBUTE_DIRECTORY & dwToAttrib))
      {
         if (-1 == dwToAttrib) // make sure the to directory exists
         {
            hr = createDirectory(m_pSHFileOpStruct->pTo, TRUE);
         }

         if (SHFO_CONTINUE(hr))
         {
            // Pick up the replaced wild card from from
            m_pszFromFixup = CDirectoryWalker::TraverseBack(m_pszCurrentFrom, 1);
            ASSERT(m_pszFromFixup);
            if (!m_pszFromFixup)
            {
               hr = SHFO_ERR_MALFORMED_PATH;
            }
            else
            {
               ASSERT(TEXT('\\') == *m_pszFromFixup);
            }
         }
      }
      else
      {
         // This means we are trying to copy wild card set into a file!?!
         hr = SHFO_ERR_INVALID_REQUEST;
      }
   }
   else if (FILE_ATTRIBUTE_DIRECTORY & dwFromAttrib) // from is a directory
   {
      if (-1 == dwToAttrib)
      {
         LPCTSTR pszRoot = CDirectoryWalker::TraverseBack(m_pSHFileOpStruct->pTo, 2);
         if (pszRoot)
         {
            // This means the we need to pick up anything that gets tacked onto from
            size_t strLen = 0;
            hr = StringCchLength(m_pszCurrentFrom, LONGEST_PATH, &strLen);
            if (FAILED(hr))
            {
               return HRESULT_FROM_WIN32_I(GetLastError());
            }

            m_pszFromFixup = (m_pszCurrentFrom+strLen);
         }
         else
         {
            // This means we are trying to copy a folder into a root that does not exist
            hr = SHFO_ERR_MALFORMED_PATH;
         }
      }
      else if (FILE_ATTRIBUTE_DIRECTORY & dwToAttrib)
      {
         // This means the directory that we will copy into already exists, set the
         // root back one so that we will copy the from directory into the to
         m_pszFromFixup = CDirectoryWalker::TraverseBack(m_pszCurrentFrom, 1);
         ASSERT(m_pszFromFixup);
         if (!m_pszFromFixup)
         {
            hr = SHFO_ERR_MALFORMED_PATH;
         }
         else
         {
            ASSERT(TEXT('\\') == *m_pszFromFixup);
         }

         // It is possible that we are trying to move/copy into the exact same
         // directory, only allow if the user wants to rename on collision
         size_t cchEnd = m_pszFromFixup-m_pszCurrentFrom;
         if (0 == cchEnd)
         {
            cchEnd++; // Since root will give m_pszFromFixup == m_pszCurrentFrom
         }

         if ((TEXT('\0') == m_pSHFileOpStruct->pTo[cchEnd]) &&
             (!_tcsnicmp(m_pszCurrentFrom, m_pSHFileOpStruct->pTo, cchEnd)))
         {
            if (FOF_RENAMEONCOLLISION & m_fFlags)
            {
               // Rename to, this will only happen once and only on the directory push
               TCHAR szToUnique[MAX_PATH]; // Per PathMakeUniqueName spec
               if (!PathMakeUniqueName(m_pszCurrentFrom,
                                       LOAD_STRING(IDS_FILECOPY_COPY),
                                       LOAD_STRING(IDS_FILECOPY_OF),
                                       FALSE, szToUnique, lengthof(szToUnique)))
               {
                  return HRESULT_FROM_WIN32_I(GetLastError());
               }

               // Replace the current to with the new one
               m_pszTo = new TCHAR[_tcslen(szToUnique)+1];
               if (!m_pszTo)
               {
                  return E_OUTOFMEMORY;
               }
               _tcscpy(m_pszTo, szToUnique);

               // Reset the fixup since we don't need the directory name anymore
               size_t strLen = 0;
               hr = StringCchLength(m_pszCurrentFrom, LONGEST_PATH, &strLen);
               if (FAILED(hr))
               {
                  return HRESULT_FROM_WIN32_I(GetLastError());
               }

               m_pszFromFixup = (m_pszCurrentFrom+strLen);
            }
            else
            {
               return SHFO_ERR_ALREADY_EXISTS;
            }
         }
      }
      else
      {
         // This means we are trying to copy a folder into a file!?!
         hr = SHFO_ERR_INVALID_REQUEST;
      }
   }
   else // from is a file
   {
      if (-1 == dwToAttrib)
      {
         // If the root of the to file does not exist create it now
         LPCTSTR pszToTail = CDirectoryWalker::TraverseBack(m_pSHFileOpStruct->pTo, 1);
         ASSERT(pszToTail);
         if (pszToTail)
         {
            ASSERT(TEXT('\\') == *pszToTail);
         }
         size_t strLen = pszToTail-m_pSHFileOpStruct->pTo;

         LPTSTR psz = new TCHAR[strLen+1];
         if (!psz)
         {
            return E_OUTOFMEMORY;
         }

         memcpy(psz, m_pSHFileOpStruct->pTo, strLen*sizeof(TCHAR));
         psz[strLen] = TEXT('\0');
         if (-1 == GetFileAttributes(psz))
         {
            hr = createDirectory(psz, TRUE);
         }
         delete [] psz;
      }
      else if (FILE_ATTRIBUTE_DIRECTORY & dwToAttrib)
      {
         // This means to does not contain the filename, walk the tail back so
         // we pick it up from the from
         m_pszFromFixup = CDirectoryWalker::TraverseBack(m_pszCurrentFrom, 1);
         ASSERT(m_pszFromFixup);
         if (!m_pszFromFixup)
         {
            hr = SHFO_ERR_MALFORMED_PATH;
         }
         else
         {
            ASSERT(TEXT('\\') == *m_pszFromFixup);
         }
      }
   }

   return hr;
}

HRESULT CCopyMove::buildTo(LPTSTR pszTo, DWORD cchTo, LPCTSTR pszFullFrom) const
{
   if (!pszTo || !pszFullFrom)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   if (!cchTo || !m_pSHFileOpStruct->pTo || !m_pszCurrentFrom)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   HRESULT hr = S_OK;
   LPTSTR pszEnd = NULL;
   size_t fromRootLength = 0;
   LPCTSTR pszCurrentTo = (m_pszTo ? m_pszTo : m_pSHFileOpStruct->pTo);

   // Figure out how much of the from we need to copy
   if (m_pszFromFixup)
   {
      fromRootLength = (m_pszFromFixup - m_pszCurrentFrom);
   }
   else
   {
      hr = StringCchLength(m_pszCurrentFrom, cchTo, &fromRootLength);
      if (FAILED(hr))
      {
         return hr;
      }
   }

   // Copy the root of the to directory
   hr = StringCchCopyEx(pszTo, cchTo, pszCurrentTo, &pszEnd, NULL, 0);
   if (FAILED(hr))
   {
      return hr;
   }

   // Tack on a \ if necessary
   if (TEXT('\\') != *(pszEnd-1))
   {
      hr = StringCchCat(pszTo, cchTo, TEXT("\\"));
      if (FAILED(hr))
      {
         return hr;
      }
   }

   // Cat the from minus the root to the to
   if (TEXT('\\') == *(pszFullFrom+fromRootLength))
   {
      hr = StringCchCatEx(pszTo, cchTo, pszFullFrom+fromRootLength+1, &pszEnd, NULL, 0);
   }
   else
   {
      hr = StringCchCatEx(pszTo, cchTo, pszFullFrom+fromRootLength, &pszEnd, NULL, 0);
   }

   // Make sure we didn't end the path in a \ 
   if (TEXT('\\') == *(pszEnd-1))
   {
      *(pszEnd-1) = TEXT('\0');
   }

   return hr;
}

HRESULT CCopy::doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom)
{
   TCHAR szTo[LONGEST_PATH];
   HRESULT hr = S_OK;

   if (!pszFullFrom)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   if (!m_pSHFileOpStruct)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   ASSERT((PUSH_DIRECTORY == reason) || (POP_DIRECTORY == reason));

   // It is possible that we created a directory during setCurrentFrom then a
   // wild card search picked it up, don't push into it in that case
   if  (!_tcsicmp(pszFullFrom, m_pSHFileOpStruct->pTo))
   {
      return S_FALSE;
   }

   if (PUSH_DIRECTORY == reason)
   {
      hr = buildTo(szTo, lengthof(szTo), pszFullFrom);
      if (SHFO_CONTINUE(hr))
      {
         hr = createDirectory(szTo, FALSE);
      }

      if (SHFO_CONTINUE(hr) && (FOF_ALLOWUNDO & m_fFlags))
      {
         g_pUndo->AddItem(pszFullFrom, szTo);
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      hr = SHFO_ERR_CANCELLED;
   }

   return hr;
}

HRESULT CCopy::doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd)
{
   HRESULT hr = S_OK;
   TCHAR szTo[LONGEST_PATH];

   if (!pszFullFrom)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   if (!m_pSHFileOpStruct)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   // Make sure from exists
   if (-1 == GetFileAttributes(pszFullFrom))
   {
      return SHFO_ERR_FILE_NOT_FOUND;
   }

   // Build to
   hr = buildTo(szTo, lengthof(szTo), pszFullFrom);
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   // Confirm that the to is ok with the user
   hr = confirmTo(pszFullFrom, szTo, lengthof(szTo));

   // S_FALSE means go on without doing this action
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   // Set the from to and filename fields of the progress dialog
   if (!(FOF_SILENT & m_fFlags) && !(FOF_SIMPLEPROGRESS & m_fFlags))
   {
      LPTSTR pszTitle = (LPTSTR) CDirectoryWalker::TraverseBack(szTo, 1);
      if (pszTitle)
      {
         m_progressUI.setTitleString(++pszTitle);
      }

      TCHAR szF[MAX_PATH] = TEXT("");
      TCHAR szT[MAX_PATH] = TEXT("");
      TCHAR szFromTo[MAX_PATH] = TEXT("");
      if (CDirectoryWalker::FindParentDirectory(pszFullFrom, szF, lengthof(szF)) &&
          CDirectoryWalker::FindParentDirectory(szTo, szT, lengthof(szT)))
      {         
         ::StringCchPrintfEx(szFromTo,
                           lengthof(szFromTo),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                           LOAD_STRING(IDS_SHFO_PROGRESS_FROM_TO),
                           szF,
                           szT);
      }

      m_progressUI.setInfoString(szFromTo);
   }

   // Do the copy
   BOOL fCancel = FALSE;
   if (!CopyFileEx(pszFullFrom, szTo, ProgressCallback, (LPVOID) this, &fCancel, 0))
   {
      hr = HRESULT_FROM_WIN32_I(GetLastError());
   }

   if (!_tcsicmp(PathFindFileName(szTo), L"desktop.ini"))
   {
      // clean out stuff from the desktop.ini
      WriteIni(L"[.ShellClassInfo]", L"LocalizedResourceName", NULL, 0, szTo);
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      hr = SHFO_ERR_CANCELLED;
   }

   // Add to the undo list
   if (SHFO_CONTINUE(hr) && (FOF_ALLOWUNDO & m_fFlags))
   {
      g_pUndo->AddItem(pszFullFrom, szTo);
   }

   return hr;
}

void CCopy::showError(HRESULT hr)
{
   TCHAR szError[256];

   if (FOF_NOERRORUI & m_fFlags)
   {
      return;
   }

   // No error or the user canceled
   if (SUCCEEDED(hr) || (SHFO_ERR_CANCELLED == hr))
   {
      return;
   }

   HWND hwndError = NULL;
   if (m_pSHFileOpStruct)
   {
      hwndError = m_pSHFileOpStruct->hwnd;
   }

   if (buildErrorString(hr, LOAD_STRING(IDS_SHFO_ERR_HDR_COPY),
                        m_pszCurrentFrom ? m_pszCurrentFrom : TEXT(""),
                        szError, lengthof(szError)))
   {
      ShellDialogs::ShowFileError(hwndError,
                                  MAKEINTRESOURCE(IDS_TITLE_COPYERROR),
                                  szError,
                                  m_pszCurrentFrom ? m_pszCurrentFrom : TEXT(""),
                                  MB_ICONERROR | MB_OK);
   }
   else
   {
      // As a last resort use the CFileOperation error handler
      CFileOperation::showError(hr);
   }
}

HRESULT CMove::setCurrentFrom(LPCTSTR pszCurrentFrom)
{
   HRESULT hr = doSpecialFolderCheck(pszCurrentFrom);

   if (SHFO_CONTINUE(hr))
   {
      hr = CCopyMove::setCurrentFrom(pszCurrentFrom);
   }
   else
   {
      CWalkingFileOperation::setCurrentFrom(pszCurrentFrom);
   }

   return hr;
}

HRESULT CMove::doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom)
{
   TCHAR szTo[LONGEST_PATH];
   HRESULT hr = S_OK;

   if (!pszFullFrom)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   if (!m_pSHFileOpStruct)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   ASSERT((PUSH_DIRECTORY == reason) || (POP_DIRECTORY == reason));

   // It is possible that we created a directory during setCurrentFrom then a
   // wild card search picked it up, don't push into it in that case
   if  (!_tcsicmp(pszFullFrom, m_pSHFileOpStruct->pTo))
   {
      return S_FALSE;
   }

   if (PUSH_DIRECTORY == reason)
   {
      hr = buildTo(szTo, lengthof(szTo), pszFullFrom);
      if (SHFO_CONTINUE(hr))
      {
         hr = createDirectory(szTo, FALSE);
      }

      if (SHFO_CONTINUE(hr) && (FOF_ALLOWUNDO & m_fFlags))
      {
         g_pUndo->AddItem(pszFullFrom, szTo);
      }
   }
   else
   {
      if (!::RemoveDirectory(pszFullFrom))
      {
         hr = HRESULT_FROM_WIN32_I(GetLastError());
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      hr = SHFO_ERR_CANCELLED;
   }

   return hr;
}

HRESULT CMove::doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd)
{
   HRESULT hr = S_OK;
   TCHAR szTo[LONGEST_PATH];
   DWORD dwFromAttrib = GetFileAttributes(pszFullFrom);

   if (!pszFullFrom)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   if (!m_pSHFileOpStruct)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   // Make sure from exists
   if (-1 == dwFromAttrib)
   {
      return SHFO_ERR_FILE_NOT_FOUND;
   }

   // Build to
   hr = buildTo(szTo, lengthof(szTo), pszFullFrom);
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   // Verify that the user actually wants to move the from file
   if (!(FOF_NOCONFIRMATION & m_fFlags))
   {
      int result = IDYES;

      hr = ShellDialogs::Confirm::Move(pszFullFrom, m_pSHFileOpStruct->hwnd,
                                       (1 == m_dwCount), &result);
      if (FAILED(hr))
      {
         return hr;
      }

      switch (result)
      {
         case IDNO:
            return S_FALSE;
         break;

         case IDCANCEL:
            return SHFO_ERR_CANCELLED;
         break;

         case IDYESTOALL:
            m_fFlags |= FOF_NOCONFIRMATION;
         break;
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      return SHFO_ERR_CANCELLED;
   }

   // Confirm that the to is ok with the user
   hr = confirmTo(pszFullFrom, szTo, lengthof(szTo));

   // S_FALSE means go on without doing this action
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   // Set the from to and filename fields of the progress dialog
   if (!(FOF_SILENT & m_fFlags) && !(FOF_SIMPLEPROGRESS & m_fFlags))
   {
      LPTSTR pszTitle = (LPTSTR) CDirectoryWalker::TraverseBack(szTo, 1);
      if (pszTitle)
      {
         m_progressUI.setTitleString(++pszTitle);
      }

      TCHAR szF[MAX_PATH] = TEXT("");
      TCHAR szT[MAX_PATH] = TEXT("");
      TCHAR szFromTo[MAX_PATH] = TEXT("");
      if (CDirectoryWalker::FindParentDirectory(pszFullFrom, szF, lengthof(szF)) &&
          CDirectoryWalker::FindParentDirectory(szTo, szT, lengthof(szT)))
      {
         ::StringCchPrintfEx(szFromTo,
                           lengthof(szFromTo),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                           LOAD_STRING(IDS_SHFO_PROGRESS_FROM_TO),
                           szF,
                           szT);
      }

      m_progressUI.setInfoString(szFromTo);
   }

   // If these are different devices do a copy then delete otherwise just MoveFile
   if (PathIsSameDevice(pszFullFrom, szTo))
   {
      // Do the move
      if (!MoveFile(pszFullFrom, szTo))
      {
         hr = HRESULT_FROM_WIN32_I(GetLastError());
      }

      // Check to see if the user canceled the operation
      if (PROGRESS_CANCEL == m_progressUI.getStatus())
      {
         hr = SHFO_ERR_CANCELLED;
      }

      // Update the progress bar
      if (SUCCEEDED(hr))
      {
         HANDLE hFile = CreateFile(szTo, GENERIC_READ, 0, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

         if (INVALID_HANDLE_VALUE != hFile)
         {
            ULARGE_INTEGER liFileSize = {0};
            liFileSize.LowPart = GetFileSize(hFile, &(liFileSize.HighPart));
            if (-1 != liFileSize.LowPart)
            {
               m_ullCurrentSize += (ULONGLONG) liFileSize.QuadPart;
               if (0 < m_ullTotalSize)
               {
                  // PROGRESS = CURRENT * 1000 / TOTAL
                  ULONGLONG ullCurrentSize = ((m_ullCurrentSize * 1000) / m_ullTotalSize);
                  m_progressUI.updateProgress((DWORD) ullCurrentSize);
               }
            }
            CloseHandle(hFile);
         }
      }
   }
   else
   {
      // Do the move
      BOOL fCancel = FALSE;
      if (!CopyFileEx(pszFullFrom, szTo, ProgressCallback, (LPVOID) this, &fCancel, 0))
      {
         hr = HRESULT_FROM_WIN32_I(GetLastError());
      }

      // Check to see if the user canceled the operation
      if (PROGRESS_CANCEL == m_progressUI.getStatus())
      {
         hr = SHFO_ERR_CANCELLED;
      }

      // Remove the from file
      if (SUCCEEDED(hr))
      {
         dwFromAttrib &= (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));
         SetFileAttributes(pszFullFrom, dwFromAttrib);

         if (!DeleteFile(pszFullFrom))
         {
            hr = HRESULT_FROM_WIN32_I(GetLastError());
         }
      }
   }

   // Add to the undo list
   if (SHFO_CONTINUE(hr) && (FOF_ALLOWUNDO & m_fFlags))
   {
      g_pUndo->AddItem(pszFullFrom, szTo);
   }

   return hr;
}

void CMove::showError(HRESULT hr)
{
   TCHAR szError[256];

   if (FOF_NOERRORUI & m_fFlags)
   {
      return;
   }

   // No error or the user canceled
   if (SUCCEEDED(hr) || (SHFO_ERR_CANCELLED == hr))
   {
      return;
   }

   HWND hwndError = NULL;
   if (m_pSHFileOpStruct)
   {
      hwndError = m_pSHFileOpStruct->hwnd;
   }

   if (buildErrorString(hr, LOAD_STRING(IDS_SHFO_ERR_HDR_MOVE),
                        m_pszCurrentFrom ? m_pszCurrentFrom : TEXT(""),
                        szError, lengthof(szError)))
   {
      ShellDialogs::ShowFileError(hwndError,
                                  MAKEINTRESOURCE(IDS_TITLE_MOVEERROR),
                                  szError,
                                  m_pszCurrentFrom ? m_pszCurrentFrom : TEXT(""),
                                  MB_ICONERROR | MB_OK);
   }
   else
   {
      // As a last resort use the CFileOperation error handler
      CFileOperation::showError(hr);
   }
}

HRESULT CDelete::validate()
{
   HRESULT hr = CWalkingFileOperation::validate();
   DWORD dwCount = 0;
   BOOL fSameDevice = TRUE;

   if (FOF_ALLOWUNDO & m_fFlags) 
   {
      m_pRecycleBin = g_pRecBin;
      if (!m_pRecycleBin)
      {
         hr = E_OUTOFMEMORY;
         goto leave;
      }

      g_pRecBin->BeginRecycle();
   }

   if (!SHFO_CONTINUE(hr))
   {
      goto leave;
   }

   size_t strLen = 0;
   LPTSTR pszFrom = (LPTSTR) m_pSHFileOpStruct->pFrom; // validated in CFileOperation::validate
   while (TEXT('\0') != *pszFrom)
   {
      if (!PathIsSameDevice(pszFrom, CRecycleBin::m_szBitBucketPath))
      {
         fSameDevice = FALSE;
      }
      
      hr = StringCchLength(pszFrom, LONGEST_PATH, &strLen);
      if (FAILED(hr))
      {
         break;
      }

      dwCount++;

      pszFrom += (strLen+1);
   }

   // Set m_pszCurrentFrom since showError needs it
   if (FAILED(hr))
   {
      ASSERT(!m_pszCurrentFrom);
      CWalkingFileOperation::setCurrentFrom(pszFrom);
      goto leave;
   }

   // Verify multiple file delete here
   if (1 < dwCount)
   {
      if (!(FOF_NOCONFIRMATION & m_fFlags))
      {
         int result = IDYES;
         BOOL fForcePermanent = FALSE;
         
         if (!m_pRecycleBin || !fSameDevice)
         {
            fForcePermanent = TRUE;
         }

         hr = ShellDialogs::Confirm::DeleteMultiple(dwCount, m_pSHFileOpStruct->hwnd,
                                                   fForcePermanent, &result);                                                    
         if (SUCCEEDED(hr))
         {
            switch (result)
            {
               case IDNO:
                  hr = S_FALSE;
               break;

               case IDCANCEL:
                  hr = SHFO_ERR_CANCELLED;
               break;
            }
         }
      }

      m_fAlwaysConfirm = FALSE;
   }
   else
   {
      DWORD dwAttrib = ::GetFileAttributes(m_pSHFileOpStruct->pFrom);
      if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
      {
         // Verify that the user actually wants to delete the folder
         if (!(FOF_NOCONFIRMATION & m_fFlags))
         {
            int result = IDYES;
            hr = ShellDialogs::Confirm::Delete(m_pSHFileOpStruct->pFrom,
                                               m_pSHFileOpStruct->hwnd, TRUE,
                                               !fSameDevice, TRUE, &result);
            if (SUCCEEDED(hr))
            {
               switch (result)
               {
                  case IDNO:
                    hr = S_FALSE;
                  break;

                  case IDCANCEL:
                     hr = SHFO_ERR_CANCELLED;
                  break;
               }
            }
         }

         m_fAlwaysConfirm = FALSE;
      }
      else
      {
         m_fAlwaysConfirm = TRUE;
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      hr = SHFO_ERR_CANCELLED;
   }

leave:
   return hr;
}

HRESULT CDelete::setCurrentFrom(LPCTSTR pszCurrentFrom)
{
   HRESULT hr = doSpecialFolderCheck(pszCurrentFrom);

   if (SHFO_CONTINUE(hr))
   {
      hr = CWalkingFileOperation::setCurrentFrom(pszCurrentFrom);
   }
   else
   {
      CWalkingFileOperation::setCurrentFrom(pszCurrentFrom);
   }

   return hr;
}

HRESULT CDelete::doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom)
{
   HRESULT hr = S_OK;

   ASSERT((PUSH_DIRECTORY == reason) || (POP_DIRECTORY == reason));

   if (POP_DIRECTORY == reason)
   {
      // If the folder was empty then it has been moved to RecycleBin under POP_DIRECTORY branch
      if (-1 != GetFileAttributes(pszFullFrom))
      {
         if (!::RemoveDirectory(pszFullFrom))
         {
            hr = HRESULT_FROM_WIN32_I(GetLastError());
         }
      }
   }
   else
   {
      WIN32_FIND_DATA fd = {0};
      TCHAR szFrom[LONGEST_PATH];

      // Tack on the * for a full search and adjust pszCurrentFile
      hr = StringCchCopy(szFrom, LONGEST_PATH, pszFullFrom);
      if (SUCCEEDED(hr))
      {
         hr = StringCchCat(szFrom, LONGEST_PATH, TEXT("\\*"));
                          
         // If there is NOTHING in this directory then ADD the folder item to the RecBin
         HANDLE hFind = FindFirstFile(szFrom, &fd);
         if (SUCCEEDED(hr) && (INVALID_HANDLE_VALUE == hFind))
         {
            // Move to recycle bin
            if ((FOF_ALLOWUNDO & m_fFlags) && m_pRecycleBin &&
                  PathIsSameDevice(pszFullFrom, CRecycleBin::m_szBitBucketPath))
            {
               LPITEMIDLIST pidl = NULL;               
               hr = m_pRecycleBin->AddFileFolder(pszFullFrom, &pidl);
               
               if (SHFO_CONTINUE(hr) && pidl)
               {
                  // Preserve Recyclebin filename (not the pidl)
                  hr = g_pUndo->AddItem(pszFullFrom, ILDisplayName(pidl));
                  ILFree(pidl);
               }
            }
         }

         FindClose(hFind);
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      hr = SHFO_ERR_CANCELLED;
   }

   return hr;
}

HRESULT CDelete::doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd)
{
   HRESULT hr = S_OK;
   DWORD dwFromAttrib = GetFileAttributes(pszFullFrom);

   if (!pszFullFrom)
   {
      return SHFO_ERR_INVALID_PARAMETER;
   }

   if (!m_pSHFileOpStruct)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   // Make sure from exists
   if (-1 == dwFromAttrib)
   {
      return SHFO_ERR_FILE_NOT_FOUND;
   }

   // Verify that the user actually wants to delete the file
   if (!(FOF_NOCONFIRMATION & m_fFlags))
   {
      int result = IDYES;
      BOOL fSameDevice = PathIsSameDevice(pszFullFrom, CRecycleBin::m_szBitBucketPath);

      hr = ShellDialogs::Confirm::Delete(pszFullFrom, m_pSHFileOpStruct->hwnd,
                                         m_fAlwaysConfirm, !fSameDevice,
                                         (1 == m_dwCount), &result);
      if (SUCCEEDED(hr))
      {
         switch (result)
         {
            case IDNO:
              hr = S_FALSE;
            break;

            case IDCANCEL:
               hr = SHFO_ERR_CANCELLED;
            break;

            case IDYESTOALL:
               m_fFlags |= FOF_NOCONFIRMATION;
            break;
         }
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      return SHFO_ERR_CANCELLED;
   }

   // No need to do anymore work on failure
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   // Set the from and filename fields of the progress dialog
   if (!(FOF_SILENT & m_fFlags) && !(FOF_SIMPLEPROGRESS & m_fFlags))
   {
      LPTSTR pszTitle = (LPTSTR) CDirectoryWalker::TraverseBack(pszFullFrom, 1);
      if (pszTitle)
      {
         m_progressUI.setTitleString(++pszTitle);
      }

      TCHAR szF[MAX_PATH] = TEXT("");
      TCHAR szFrom[MAX_PATH] = TEXT("");
      if (CDirectoryWalker::FindParentDirectory(pszFullFrom, szF, lengthof(szF)))
      {
         ::StringCchPrintfEx(szFrom,
                           lengthof(szFrom),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                           LOAD_STRING(IDS_SHFO_PROGRESS_FROM),
                           szF);
      }

      m_progressUI.setInfoString(szFrom);
   }

   // Figure out how much the delete will take care of
   HANDLE hFile = CreateFile(pszFullFrom, GENERIC_READ, 0, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (INVALID_HANDLE_VALUE != hFile)
   {
      ULARGE_INTEGER liFileSize = {0};
      liFileSize.LowPart = GetFileSize(hFile, &(liFileSize.HighPart));
      if (-1 != liFileSize.LowPart)
      {
         m_ullCurrentSize += (ULONGLONG) liFileSize.QuadPart;
      }
      CloseHandle(hFile);
   }

   // Move to recycle bin or delete
   if ((FOF_ALLOWUNDO & m_fFlags) && m_pRecycleBin &&
         PathIsSameDevice(pszFullFrom, CRecycleBin::m_szBitBucketPath))
   {
      LPITEMIDLIST pidl = NULL;
      
      // Expect following return values: 
      // S_OK, S_FALSE (skip file handling, it is permanently deleted) or
      // SHFO_ERR_CANCELLED:
      // - sizeof(file) > sizeof(Recycle Bin) and delete operation is cancelled for the rest of the files;
      // - Recycle Bin is Full and Cancel is chosen.  
      hr = m_pRecycleBin->AddFileFolder(pszFullFrom, &pidl);
      
      if (SHFO_CONTINUE(hr) && pidl)
      {
         // Preserve Recyclebin filename (not the pidl)
         hr = g_pUndo->AddItem(pszFullFrom, ILDisplayName(pidl));
         ILFree(pidl);
      }
   }
   else
   {
      dwFromAttrib &= (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));
      SetFileAttributes(pszFullFrom, dwFromAttrib);

      if (!DeleteFile(pszFullFrom))
      {
         hr = HRESULT_FROM_WIN32_I(GetLastError());
      }
   }

   // Check to see if the user canceled the operation
   if (PROGRESS_CANCEL == m_progressUI.getStatus())
   {
      hr = SHFO_ERR_CANCELLED;
   }

   // Update the progress bar
   if (SHFO_CONTINUE(hr) && (0 < m_ullTotalSize))
   {
      // PROGRESS = CURRENT * 1000 / TOTAL
      ULONGLONG ullCurrentSize = ((m_ullCurrentSize * 1000) / m_ullTotalSize);
      m_progressUI.updateProgress((DWORD) ullCurrentSize);
   }

   return hr;
}

void CDelete::showError(HRESULT hr)
{
   TCHAR szError[256];

   if (FOF_NOERRORUI & m_fFlags)
   {
      return;
   }

   // No error or the user canceled
   if (SUCCEEDED(hr) || (SHFO_ERR_CANCELLED == hr))
   {
      return;
   }

   HWND hwndError = NULL;
   if (m_pSHFileOpStruct)
   {
      hwndError = m_pSHFileOpStruct->hwnd;
   }

   if (buildErrorString(hr, LOAD_STRING(IDS_SHFO_ERR_HDR_DELETE),
                        m_pszCurrentFrom ? m_pszCurrentFrom : TEXT(""),
                        szError, lengthof(szError)))
   {
      ShellDialogs::ShowFileError(hwndError,
                                  MAKEINTRESOURCE(IDS_TITLE_DELETEERROR),
                                  szError,
                                  m_pszCurrentFrom ? m_pszCurrentFrom : TEXT(""),
                                  MB_ICONERROR | MB_OK);
   }
   else
   {
      // As a last resort use the CFileOperation error handler
      CFileOperation::showError(hr);
   }
}

HRESULT CRename::validate()
{
   HRESULT hr = CFileOperation::validate();
   if (!SHFO_CONTINUE(hr))
   {
      return hr;
   }

   if (!m_pSHFileOpStruct->pTo)
   {
      return SHFO_ERR_INVALID_DATA;
   }

   size_t strLenFrom = 0;
   hr = StringCchLength(m_pSHFileOpStruct->pFrom, LONGEST_PATH, &strLenFrom);
   if (FAILED(hr))
   {
      return hr;
   }

   // Verify that the path exists
   if (!PathFileExists(m_pSHFileOpStruct->pFrom))
   {
      return SHFO_ERR_PATH_NOT_FOUND;
   }

   // Verify that the path is not a volume root
   if (CDirectoryWalker::PathIsVolumeRoot(m_pSHFileOpStruct->pFrom))
   {
      return SHFO_ERR_MALFORMED_PATH;
   }

   // Verify that we are only renaming one item
   if (TEXT('\0') != *(m_pSHFileOpStruct->pFrom+(strLenFrom+1)))
   {
      return SHFO_ERR_INVALID_DATA;
   }

   if  (!_tcsnicmp(m_pSHFileOpStruct->pFrom, m_pSHFileOpStruct->pTo, strLenFrom))
   {
      // Verify that the 'to' is is not exactly the same as the 'from'
      if (TEXT('\0') == m_pSHFileOpStruct->pTo[strLenFrom])
      {
         LPCTSTR pszF = CDirectoryWalker::TraverseBack(m_pSHFileOpStruct->pFrom, 1);
         LPCTSTR pszT = CDirectoryWalker::TraverseBack(m_pSHFileOpStruct->pTo, 1);
         ASSERT(pszF && pszT && (0 == _tcsicmp(pszF, pszT)));
         if (pszF && pszT && (0 == _tcscmp(pszF, pszT)))
         {
            return S_FALSE;
         }
      }

      // Verify that the 'to' is not a subfolder of 'from'
      if (TEXT('\\') == m_pSHFileOpStruct->pTo[strLenFrom])
      {
         return SHFO_ERR_DEST_IS_SUBFOLDER;
      }
   }

   // Verify that to is a valid filename
   if (!PathIsValidFileName(PathFindFileName(m_pSHFileOpStruct->pTo)))
   {
      return SHFO_ERR_INVALID_FILENAME;
   }

   return doSpecialFolderCheck(m_pSHFileOpStruct->pFrom);
}

void CRename::showError(HRESULT hr)
{
   TCHAR szError[256];

   if (FOF_NOERRORUI & m_fFlags)
   {
      return;
   }

   // No error or the user canceled
   if (SUCCEEDED(hr) || (SHFO_ERR_CANCELLED == hr))
   {
      return;
   }

   HWND hwndError = NULL;
   LPTSTR pszFrom = TEXT("");

   if (m_pSHFileOpStruct)
   {
      hwndError = m_pSHFileOpStruct->hwnd;
      if (m_pSHFileOpStruct->pFrom)
      {
         pszFrom = (LPTSTR)m_pSHFileOpStruct->pFrom;
      }
   }

   if (buildErrorString(hr, LOAD_STRING(IDS_SHFO_ERR_HDR_RENAME),
                        pszFrom, szError, lengthof(szError)))
   {
      ShellDialogs::ShowFileError(hwndError,
                                  MAKEINTRESOURCE(IDS_TITLE_RENAMEERROR),
                                  szError, pszFrom, MB_ICONERROR | MB_OK);
   }
   else
   {
      // As a last resort use the CFileOperation error handler
      CFileOperation::showError(hr);
   }
}

HRESULT CRename::doOperation()
{
   HRESULT hr = S_OK;

   // Verify that the user actually wants to rename
   if (!(FOF_NOCONFIRMATION & m_fFlags))
   {
      int result = IDYES;

      hr = ShellDialogs::Confirm::Rename(m_pSHFileOpStruct->pFrom,
                                         m_pSHFileOpStruct->pTo,
                                         m_pSHFileOpStruct->hwnd,
                                         &result);
      if (FAILED(hr))
      {
         return hr;
      }

      switch (result)
      {
         case IDNO:
            return S_FALSE;
         break;

         case IDCANCEL:
            return SHFO_ERR_CANCELLED;
         break;
      }
   }

   // Do the rename
   if (!MoveFile(m_pSHFileOpStruct->pFrom, m_pSHFileOpStruct->pTo))
   {
      return HRESULT_FROM_WIN32_I(GetLastError());
   }

   // Add to the undo list
   if (SHFO_CONTINUE(hr) && (FOF_ALLOWUNDO & m_pSHFileOpStruct->fFlags))
   {
      g_pUndo->AddItem(m_pSHFileOpStruct->pFrom, m_pSHFileOpStruct->pTo);
   }

   return S_OK;
}

