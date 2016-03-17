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
#include "recbin.h"
#include "resource.h"
#pragma warning (disable:4512) // 'class' : assignment operator could not be generated

#define SHFO_ERR_ACCESS_DENIED        HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED)
#define SHFO_ERR_ALREADY_EXISTS       HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)
#define SHFO_ERR_CANCELLED            HRESULT_FROM_WIN32(ERROR_CANCELLED)
#define SHFO_ERR_DEST_IS_SUBFOLDER    HRESULT_FROM_WIN32(ERROR_BAD_COMMAND)
#define SHFO_ERR_DIRECTORY_LOCKED     HRESULT_FROM_WIN32(ERROR_CURRENT_DIRECTORY)
#define SHFO_ERR_DISK_FULL            HRESULT_FROM_WIN32(ERROR_DISK_FULL)
#define SHFO_ERR_FILE_NOT_FOUND       HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)
#define SHFO_ERR_INSUFFICIENT_BUFFER  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)
#define SHFO_ERR_INVALID_DATA         HRESULT_FROM_WIN32(ERROR_INVALID_DATA)
#define SHFO_ERR_INVALID_FILENAME     HRESULT_FROM_WIN32(ERROR_INVALID_NAME)
#define SHFO_ERR_INVALID_FLAGS        HRESULT_FROM_WIN32(ERROR_INVALID_FLAGS)
#define SHFO_ERR_INVALID_PARAMETER    HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER)
#define SHFO_ERR_INVALID_REQUEST      HRESULT_FROM_WIN32(ERROR_FILE_EXISTS)
#define SHFO_ERR_MALFORMED_PATH       HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME)
#define SHFO_ERR_PATH_NOT_FOUND       HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)
#define SHFO_ERR_SHARING_VIOLATION    HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION)
#define SHFO_ERR_FILENAME_EXCED_RANGE  HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE)


#define SHFO_CONTINUE(hr) ((!FAILED((hr))) && ((hr) != S_FALSE))

class CDirectoryWalker
{
protected:
   CDirectoryWalker() {}

public:
   struct WalkAction
   {
      enum WalkReason { PUSH_DIRECTORY, POP_DIRECTORY };
      virtual HRESULT doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom) = 0;
      virtual HRESULT doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd) = 0;
   };

   static HRESULT Walk(LPTSTR pszWalkRoot, const size_t cchWalkRoot, WalkAction * pWalkAction);
   static LPCTSTR TraverseBack(LPCTSTR pszPath, DWORD dwHowMany);
   static BOOL FindParentDirectory(LPCTSTR pszPath, LPTSTR pszParent, const size_t cchParent);
   static BOOL PathIsVolumeRoot(LPCTSTR pszPath);
};

class CProgressUI
{
protected:
   HWND m_hParent;
   HWND m_hDialog;
   HANDLE m_hSyncEvent;
   UINT m_status;
   DWORD m_dwStartTime;

   static DWORD WINAPI dialogThread(LPVOID lpVoid);
   static BOOL CALLBACK dialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
   CProgressUI();
   virtual ~CProgressUI();

   inline UINT getStatus() const { return m_status; }
   void createDialog(HWND hParent);
   void destroyDialog();
   void showDialog();
   void hideDialog();
   BOOL isVisible();
   void setTitleString(LPCTSTR pszTitle) const;
   void setInfoString(LPCTSTR pszInfo) const;
   void updateProgress(DWORD dwPercent);
};

class CCalculateTotalSize : public CDirectoryWalker::WalkAction
{
protected:
   static const ULONGLONG SHOW_PROGRESS_THRESHOLD;

   ULONGLONG m_ullTotalSize;
   CProgressUI * m_pProgressUI;

public:
   CCalculateTotalSize(CProgressUI * pProgressUI = NULL) : m_ullTotalSize(0), m_pProgressUI(pProgressUI) {}
   virtual ~CCalculateTotalSize() {}

   virtual HRESULT doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom);
   virtual HRESULT doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd);

   const ULONGLONG getTotalSize() const { return m_ullTotalSize; }
};

class CFileOperation
{
protected:
   static const UINT LOCKED_FOLDERS[]; // Folders that can not be changed
   static const UINT PROTECTED_FOLDERS[]; // Folders that require warning before changing

	FILEOP_FLAGS m_fFlags;
   LPSHFILEOPSTRUCT m_pSHFileOpStruct;

   virtual HRESULT validate();
   HRESULT doSpecialFolderCheck(LPCTSTR pszPath);
   virtual UINT SHFO_ERR_INROM() = 0;
   BOOL buildErrorString(HRESULT hrError, LPTSTR pszHeader, LPTSTR pszPath, LPTSTR pszError, size_t cchError);
   virtual void showError(HRESULT hr);
   virtual HRESULT doOperation() = 0;

public:
   static const DWORD LONGEST_PATH;

   CFileOperation() : m_fFlags(0), m_pSHFileOpStruct(NULL) {}
   virtual ~CFileOperation() {}

   HRESULT execute(LPSHFILEOPSTRUCT pSHFileOpStruct);
};

class CWalkingFileOperation : public CFileOperation, public CDirectoryWalker::WalkAction
{
protected:   
   LPTSTR m_pszCurrentFrom;
   CProgressUI m_progressUI;
   DWORD m_dwCount;
   ULONGLONG m_ullTotalSize;
   ULONGLONG m_ullCurrentSize;

   virtual HRESULT validate();
   virtual HRESULT doOperation();
   virtual HRESULT initialize();
   virtual HRESULT setCurrentFrom(LPCTSTR pszCurrentFrom);
   virtual UINT SHFO_PROGRESS_PREPARING() = 0;

public:
   CWalkingFileOperation();
   virtual ~CWalkingFileOperation() { setCurrentFrom(NULL); }
};

class CCopyMove : public CWalkingFileOperation
{
protected:
   LPTSTR m_pszTo; // Points the the current to string, if NULL use m_pSHFileOpStruct->pTo
   LPCTSTR m_pszFromFixup; // Points to the end of the from string

   static DWORD CALLBACK ProgressCallback(LARGE_INTEGER TotalFileSize,
                                          LARGE_INTEGER TotalBytesTransferred,
                                          LARGE_INTEGER StreamSize,
                                          LARGE_INTEGER StreamBytesTransferred,
                                          DWORD dwStreamNumber,
                                          DWORD dwCallbackReason,
                                          HANDLE hSourceFile,
                                          HANDLE hDestinationFile,
                                          LPVOID lpVoid);

   virtual HRESULT validate();
   virtual HRESULT initialize();
   HRESULT createDirectory(LPCTSTR pszPath, BOOL fDoConfirmMkDirCheck);
   HRESULT confirmTo(LPCTSTR pszFrom, LPTSTR pszTo, size_t cchTo);
   virtual HRESULT setCurrentFrom(LPCTSTR pszCurrentFrom);
   HRESULT buildTo(LPTSTR pszTo, DWORD cchTo, LPCTSTR pszFullFrom) const;

public:
   CCopyMove() :  m_pszTo(NULL), m_pszFromFixup(NULL) {}
   virtual ~CCopyMove() { if (m_pszTo) delete [] m_pszTo; }
};

class CCopy : public CCopyMove
{
protected:
   virtual UINT SHFO_ERR_INROM() { return IDS_SHFO_ERR_INROMCOPY; }
   virtual UINT SHFO_PROGRESS_PREPARING() { return IDS_SHFO_PROGRESS_PREP_COPY; }
   virtual void showError(HRESULT hr);

public:
   virtual HRESULT doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom);
   virtual HRESULT doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd);
};

class CMove : public CCopyMove
{
protected:
   virtual HRESULT setCurrentFrom(LPCTSTR pszCurrentFrom);
   virtual UINT SHFO_ERR_INROM() { return IDS_SHFO_ERR_INROMMOVE; }
   virtual UINT SHFO_PROGRESS_PREPARING() { return IDS_SHFO_PROGRESS_PREP_MOVE; }
   virtual void showError(HRESULT hr);

public:
   virtual HRESULT doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom);
   virtual HRESULT doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd);
};

class CDelete : public CWalkingFileOperation
{
protected:
   CRecycleBin * m_pRecycleBin;
   BOOL m_fAlwaysConfirm;

   virtual HRESULT validate();
   virtual HRESULT setCurrentFrom(LPCTSTR pszCurrentFrom);
   virtual UINT SHFO_ERR_INROM() { return IDS_SHFO_ERR_INROMDELETE; }
   virtual UINT SHFO_PROGRESS_PREPARING() { return IDS_SHFO_PROGRESS_PREP_DELE; }
   virtual void showError(HRESULT hr);

public:
   CDelete() : m_pRecycleBin(NULL), m_fAlwaysConfirm(TRUE) {}
   ~CDelete() { if (m_pRecycleBin) {g_pRecBin->EndRecycle(); m_pRecycleBin = NULL;} }
   virtual HRESULT doDirectoryAction(WalkReason reason, LPCTSTR pszFullFrom);
   virtual HRESULT doFileAction(LPCTSTR pszFullFrom, WIN32_FIND_DATA & fd);
};

class CRename : public CFileOperation
{
protected:
   virtual HRESULT validate();
   virtual UINT SHFO_ERR_INROM() { return IDS_SHFO_ERR_INROMMOVE; }
   virtual void showError(HRESULT hr);
   virtual HRESULT doOperation();
};

