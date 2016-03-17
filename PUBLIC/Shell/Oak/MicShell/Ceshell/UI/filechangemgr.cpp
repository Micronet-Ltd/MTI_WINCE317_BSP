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

#include "filechangemgr.h"
#include "idlist.h"

const DWORD FileChangeManager::NOTIFY_FILTER = FILE_NOTIFY_CHANGE_CEGETINFO |
                                               FILE_NOTIFY_CHANGE_FILE_NAME |
                                               FILE_NOTIFY_CHANGE_DIR_NAME |
                                               FILE_NOTIFY_CHANGE_SIZE |
                                               FILE_NOTIFY_CHANGE_LAST_WRITE;

const DWORD FileChangeManager::NOTIFY_BUFFER_MAX = 0x0FFF;

FileChangeManager::FileChangeManager() :
   m_hThread(NULL),
   m_dwWaitObjects(0),
   m_fShutdown(FALSE)
{
   ::InitializeCriticalSection(&m_cs);
   ::InitializeCriticalSection(&m_cs2);
   m_waitObjects[m_dwWaitObjects++] = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

FileChangeManager::~FileChangeManager()
{
   if (m_hThread)
      Shutdown();

   if (m_waitObjects[0])
      ::CloseHandle(m_waitObjects[0]);

   ::DeleteCriticalSection(&m_cs);
   ::DeleteCriticalSection(&m_cs2);
}

DWORD WINAPI FileChangeManager::ThreadProc(LPVOID lpParameter)
{
   FileChangeManager * pFileChangeManager = (FileChangeManager *) lpParameter;
   pFileChangeManager->ProcessEvents();
   return 0;
}

void FileChangeManager::RefreshAllViews()
{
   ::EnterCriticalSection(&m_cs2);
   ::SetEvent(m_waitObjects[0]);
   ::EnterCriticalSection(&m_cs);

   // this is a special call to force a refresh on all registered windows.
   for (DWORD i = 1; i < m_dwWaitObjects; i++)
   {
      m_ShellChangeNotify[i-1]->AddRef();
      m_ShellChangeNotify[i-1]->OnChange(SHCNE_ASSOCCHANGED, NULL, NULL);
      m_ShellChangeNotify[i-1]->Release();
   }

   ::LeaveCriticalSection(&m_cs);
   ::LeaveCriticalSection(&m_cs2);
}

void FileChangeManager::OnChange(HANDLE hNotificationInfo,
                                 IShellChangeNotify * pShellChangeNotify)
{
   DWORD cbBuffer = 0;
   LPBYTE pBuffer = NULL;

   if (!::CeGetFileNotificationInfo(hNotificationInfo, 0, NULL,
                                    0, NULL, &cbBuffer))
   {
      return;
   }

   // Limit the buffer size so we don't loop for to long processing the notifies
   ASSERT(0 < cbBuffer);
   if (NOTIFY_BUFFER_MAX < cbBuffer)
      cbBuffer = NOTIFY_BUFFER_MAX;

   pBuffer = (LPBYTE) g_pShellMalloc->Alloc(cbBuffer);
   if (!pBuffer)
      return;

   if (!::CeGetFileNotificationInfo(hNotificationInfo, 0, pBuffer,
                                    cbBuffer, NULL, NULL))
   {
      g_pShellMalloc->Free(pBuffer);
      return;
   }

   FILE_NOTIFY_INFORMATION * pNotifyInfo = NULL;
   DWORD cbOffset = 0;
   LPITEMIDLIST pidl = NULL;
   do
   {
      pNotifyInfo = (FILE_NOTIFY_INFORMATION *) (pBuffer+cbOffset);

      if (0 != ::wcscmp(L"\\", pNotifyInfo->FileName))
      {
         if (SUCCEEDED(CreateFileSystemPidl(pNotifyInfo->FileName, &pidl)))
         {
            switch (pNotifyInfo->Action)
            {
               case FILE_ACTION_ADDED:
               {
                  pShellChangeNotify->OnChange(SHCNE_CREATE, pidl, NULL);
               }
               break;

               case FILE_ACTION_REMOVED:
               {
                  pShellChangeNotify->OnChange(SHCNE_DELETE, pidl, NULL);
               }
               break;

               case FILE_ACTION_MODIFIED:
               {
                  pShellChangeNotify->OnChange(SHCNE_ATTRIBUTES, pidl, NULL);
               }
               break;

               case FILE_ACTION_CHANGE_COMPLETED:
               {
                  pShellChangeNotify->OnChange(SHCNE_ATTRIBUTES, pidl, NULL);
                  pShellChangeNotify->OnChange(SHCNE_UPDATEITEM, pidl, NULL);
               }
               break;

               case FILE_ACTION_RENAMED_OLD_NAME:
               {
                  cbOffset += pNotifyInfo->NextEntryOffset;
                  pNotifyInfo = (FILE_NOTIFY_INFORMATION *) (pBuffer+cbOffset);
                  PREFAST_ASSERT(pNotifyInfo);
                  ASSERT(FILE_ACTION_RENAMED_NEW_NAME == pNotifyInfo->Action);

                  LPITEMIDLIST pidl2 = NULL;
                  if (SUCCEEDED(CreateFileSystemPidl(pNotifyInfo->FileName, &pidl2)))
                  {
                     pShellChangeNotify->OnChange(SHCNE_RENAMEITEM, pidl, pidl2);
                     ILFree(pidl2);
                  }
               }
            }

            ILFree(pidl);
            pidl = NULL;
         }
      }
      else
      {
         // Special notification which means this directory no longer exists
         pShellChangeNotify->OnChange(SHCNE_RMDIR, NULL, NULL);
      }

      cbOffset += pNotifyInfo->NextEntryOffset;
   } while(pNotifyInfo->NextEntryOffset);

   g_pShellMalloc->Free(pBuffer);
}

void FileChangeManager::Shutdown()
{
   m_fShutdown = TRUE;
   ::EnterCriticalSection(&m_cs2);
   ::SetEvent(m_waitObjects[0]);
   ::EnterCriticalSection(&m_cs);

   for (DWORD i = 1; i < m_dwWaitObjects; i++)
   {
      ::FindCloseChangeNotification(m_waitObjects[i]);
      m_ShellChangeNotify[i-1]->Release();
   }

   ::CloseHandle(m_hThread);
   m_hThread = NULL;

   ::LeaveCriticalSection(&m_cs);
   ::LeaveCriticalSection(&m_cs2);
}

void FileChangeManager::ProcessEvents()
{
   DWORD dwWait;
   HANDLE hNotificationInfo = NULL;
   IShellChangeNotify * pShellChangeNotify = NULL;

   while (!m_fShutdown)
   {
      ::EnterCriticalSection(&m_cs);
      dwWait = ::WaitForMultipleObjects(m_dwWaitObjects, m_waitObjects,
                                        FALSE, INFINITE);
      if ((0 < dwWait-WAIT_OBJECT_0) && (m_dwWaitObjects > dwWait-WAIT_OBJECT_0))
      {
         hNotificationInfo = m_waitObjects[dwWait-WAIT_OBJECT_0];
         pShellChangeNotify = m_ShellChangeNotify[dwWait-WAIT_OBJECT_0-1];
         pShellChangeNotify->AddRef();
      }
      ::LeaveCriticalSection(&m_cs);

      if (WAIT_OBJECT_0 == dwWait)
      {
         ::ResetEvent(m_waitObjects[0]);
      }
      else if (m_dwWaitObjects > dwWait-WAIT_OBJECT_0)
      {
         OnChange(hNotificationInfo, pShellChangeNotify);
         pShellChangeNotify->Release();
      }
      else
      {
         ASSERT(WAIT_OBJECT_0+m_dwWaitObjects > dwWait);
      }

      // Wait on CriticalSection2. 
      // This allows other threads to gain control of the main CriticalSection
      // NOTE: it is important to hold the thread here (before re-entering
      // the while loop), since another thread may have shut it down.
      ::EnterCriticalSection(&m_cs2);
      ::LeaveCriticalSection(&m_cs2);
   }
}

UINT FileChangeManager::AddWatch(LPCWSTR pwszPath,
                                 IShellChangeNotify * pShellChangeNotify)
{
   ASSERT(pwszPath);
   ASSERT(pShellChangeNotify);
   if (!pwszPath || !pShellChangeNotify)
      return 0;

   if (lengthof(m_waitObjects) <= m_dwWaitObjects)
      return 0;

   UINT uReturn = 0;
   HANDLE hChangeNotification = ::FindFirstChangeNotification(pwszPath,
                                                              FALSE,
                                                              NOTIFY_FILTER);
   if (INVALID_HANDLE_VALUE == hChangeNotification)
      return 0;

   ::EnterCriticalSection(&m_cs2);
   ::SetEvent(m_waitObjects[0]);
   ::EnterCriticalSection(&m_cs);

   m_waitObjects[m_dwWaitObjects] = hChangeNotification;
   m_ShellChangeNotify[m_dwWaitObjects-1] = pShellChangeNotify;
   m_ShellChangeNotify[m_dwWaitObjects-1]->AddRef();
   uReturn = (UINT) m_waitObjects[m_dwWaitObjects];
   m_dwWaitObjects++;

   if (!m_hThread && (1 < m_dwWaitObjects))
   {
      m_fShutdown = FALSE;
      m_hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
      ASSERT(m_hThread);
   }

   ::LeaveCriticalSection(&m_cs);
   ::LeaveCriticalSection(&m_cs2);

   return uReturn;
}

BOOL FileChangeManager::RemoveWatch(UINT uWatchID)
{
   DWORD dwWatch = (DWORD) -1;
   HANDLE hThreadOld = NULL;

   ::EnterCriticalSection(&m_cs2);
   ::SetEvent(m_waitObjects[0]);
   ::EnterCriticalSection(&m_cs);

   for (DWORD i = 0; i < m_dwWaitObjects; i++)
   {
      if (uWatchID == (UINT) m_waitObjects[i])
      {
         dwWatch = i;
         break;
      }
   }

   if (-1 != dwWatch)
   {
      PREFAST_ASSERT(m_ShellChangeNotify[dwWatch-1]);

      // Free the objects
      ::FindCloseChangeNotification(m_waitObjects[dwWatch]);
      m_ShellChangeNotify[dwWatch-1]->Release();

      // Copy the exiting handles down one slot
      m_dwWaitObjects--;
      ::memcpy(m_waitObjects+dwWatch, m_waitObjects+dwWatch+1,
               (m_dwWaitObjects-dwWatch)*sizeof(*m_waitObjects));
      ::memcpy(m_ShellChangeNotify+dwWatch-1, m_ShellChangeNotify+dwWatch,
               (m_dwWaitObjects-dwWatch)*sizeof(*m_ShellChangeNotify));
   }

   if (m_hThread && (1 == m_dwWaitObjects))
   {
      // If last object is being removed, shut down the thread
      hThreadOld = m_hThread;
      m_hThread = NULL;
      m_fShutdown = TRUE;
   }

   ::LeaveCriticalSection(&m_cs);
   ::LeaveCriticalSection(&m_cs2);

   if (hThreadOld)
   {
      // On shutdown, wait until thread is finsihed.
      // NOTE: This object can get destroyed, while the thread is being
      // terminated. The following was added, to ensure thread is not 
      // longer holding resources such as CS
      WaitForSingleObject(hThreadOld, INFINITE);
      ::CloseHandle(hThreadOld);
   }

   return (-1 != dwWatch);
}

