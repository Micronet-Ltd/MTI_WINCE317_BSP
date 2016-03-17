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
#include "recbin.h"
#include "idlist.h"
#include "resource.h"
#include "shelldialogs.h"
#include "shfileop.h"
#include "usersettings.h"

#undef  PROPID_FILE
#define CLSID_RECYLCEDB (0x10211201)
#define PROPID_FILE     (0x10210000 | CEVT_LPWSTR)
#define PROPID_ORGPATH  (0x10220000 | CEVT_LPWSTR)
#define PROPID_DELDATE  (0x10230000 | CEVT_FILETIME)

const TCHAR CRecycleBin::BITBUCKET[] = TEXT("\\Recycled");        // Folder name
const TCHAR CRecycleBin::DBVOLUMENAME[] = TEXT("\\recycle.bin");  // DB Volume name
const TCHAR CRecycleBin::EXPLORER[] = TEXT("Explorer");
const TCHAR CRecycleBin::RECYCLEINFO[]   = TEXT("RecycleData");   // Database name
const TCHAR CRecycleBin::RECYCLE_BIN_ENABLEDBFILE[] = TEXT("RecycleBinEnableDBFile");
const TCHAR CRecycleBin::RECYCLE_BIN_FLUSH[] = TEXT("RecycleBinFlush");

BOOL CRecycleBin::m_fcsInit;
BOOL CRecycleBin::m_fFlushVolume = TRUE;
BOOL CRecycleBin::m_fMounted;
CEGUID CRecycleBin::m_guid;
CEOID CRecycleBin::m_oid;
CRITICAL_SECTION	CRecycleBin::m_cs;
HWND CRecycleBin::m_hwndDesktop = NULL;
HANDLE CRecycleBin::m_hDB;
TCHAR CRecycleBin::m_szBitBucketPath[] = TEXT("");
TCHAR CRecycleBin::m_szCSIDLProfile[] = TEXT("");
TCHAR CRecycleBin::m_szFindFile[] = TEXT("");
UINT CRecycleBin::m_dwPercentage;
UINT CRecycleBin::m_iNotObjectStore = 0;

CRecycleBin::CRecycleBin(SHCONTF grfFlags) :
   m_dwInitMountTimeout(20000),
   m_ObjRefCount(0),
   m_hfind(INVALID_HANDLE_VALUE),
   m_grfFlags(grfFlags)
{
   m_dwPercentage = UserSettings::GetRecycleBinSize();
   ZeroMemory(&m_FindData, sizeof(m_FindData));  
   m_ObjRefCount = 1;
}

CRecycleBin::~CRecycleBin() {}

void CRecycleBin::GlobalInit()
{
   InitializeCriticalSection(&m_cs);
   CRecycleBin::m_fcsInit = TRUE;

   CREATE_INVALIDGUID(&m_guid);
   m_fMounted = FALSE;
   m_oid = 0;
   m_hDB = NULL;
}

void CRecycleBin::GlobalCleanup()
{

   if (m_hDB)
   {
      CloseHandle(m_hDB);
   }

   if (1 == m_iNotObjectStore)
   {
      if (!CHECK_INVALIDGUID(&m_guid))
      {
         CeUnmountDBVol(&m_guid);
         m_fMounted = FALSE;
      }
   }

   if (m_fcsInit)
   {
      DeleteCriticalSection(&m_cs);
   }
}

void CRecycleBin::BeginRecycle(SHCONTF grfFlags)
{
   GetTypeAndInit();

   if (grfFlags)
   {
      m_grfFlags = grfFlags;
   }
}

void CRecycleBin::EndRecycle()
{
   if ((INVALID_HANDLE_VALUE != m_hfind) && (NULL != m_hfind))
   {
      FindClose(m_hfind);
   }

   EnterCriticalSection(&m_cs);

   if (m_hDB)
   {
      CloseHandle(m_hDB);
      m_hDB = NULL;
   }

   if (1 != m_iNotObjectStore)
   {
      // For the object store DB
      CREATE_INVALIDGUID(&m_guid);
   }

   LeaveCriticalSection(&m_cs);
}

// AddRecord adds a record to the DB if the specified record does not exist. 
UINT CRecycleBin::AddRecord(LPCTSTR lpszRecycleName, LPCTSTR lpszPath)
{
   CEOID oidRet;
   CEPROPVAL rgPropVal[3];
   FILETIME ftLocal;
   SYSTEMTIME st;
   UINT uReturn = ERROR_SUCCESS;

   if (!lpszRecycleName || !lpszPath)
   {
      return ERROR_INVALID_PARAMETER;
   }

   uReturn = OpenDatabase();

   if(ERROR_SUCCESS == uReturn)
   {
      EnterCriticalSection(&m_cs);

      if (!CHECK_INVALIDGUID(&m_guid) && (INVALID_HANDLE_VALUE != m_hDB) && m_oid)
      {
         rgPropVal[0].propid = PROPID_FILE;
         rgPropVal[0].wFlags = 0;
         rgPropVal[0].val.lpwstr = (LPTSTR)PathFindFileName(lpszRecycleName);

         rgPropVal[1].propid = PROPID_ORGPATH;
         rgPropVal[1].val.lpwstr = (LPTSTR)lpszPath;
         rgPropVal[1].wFlags = 0;
      
         rgPropVal[2].propid = PROPID_DELDATE;
         rgPropVal[2].wFlags = 0;

         GetLocalTime(&st);
         SystemTimeToFileTime(&st, &ftLocal);
         LocalFileTimeToFileTime(&ftLocal, &rgPropVal[2].val.filetime);

         oidRet = CeSeekDatabaseEx(m_hDB, CEDB_SEEK_VALUEFIRSTEQUAL, 
                     (DWORD)&rgPropVal[0],   //dwValue to use for the seek operation
                     1,                      //the # of CEPROPVAL structures supplied in the dwValue
                     NULL);
         if (!oidRet)
         {
            //No record, add a new one
            if (CeWriteRecordProps(m_hDB, oidRet, 3, rgPropVal))
            {
               // In a mounted database volume, all write operations are cached. To explicitly force 
               // the data to be written out to permanent storage, the CeFlushDBVol function must be called.
               if (m_fFlushVolume)
               {
                  CeFlushDBVol(&m_guid);
               }
            }
            else
            {
               uReturn = GetLastError();
            }
         }      
      }
      LeaveCriticalSection(&m_cs);
   }

   return uReturn;
}

BOOL CRecycleBin::CanAddFile(HANDLE hFile, ULONGLONG * ullFileSize)
{
   BOOL fRes = FALSE;
   ULARGE_INTEGER liFileSize = {0};
    
   *ullFileSize = 0;

   ASSERT(hFile);
   if (!hFile || INVALID_HANDLE_VALUE == hFile)
   {
      return FALSE;
   }
 
   liFileSize.LowPart = ::GetFileSize(hFile, &(liFileSize.HighPart));
   if (-1 != liFileSize.LowPart)
   {
      ULONGLONG ullSize = StoreSize();
    
      if (ullSize)
      {
         //Compare the file size with the Recycle Bin size
         *ullFileSize = (ULONGLONG) liFileSize.QuadPart;
         if ((ULONGLONG) liFileSize.QuadPart <= (ULONGLONG)m_dwPercentage * (ullSize / (ULONGLONG)100))
         {
            fRes = TRUE;
         }
      }
   }

   return fRes;
}

UINT CRecycleBin::DeleteDatabase()
{
   UINT uReturn = ERROR_SUCCESS;

   EnterCriticalSection(&m_cs);

   if (!m_hDB || INVALID_HANDLE_VALUE == m_hDB)
   {
      LeaveCriticalSection(&m_cs);
      return ERROR_INVALID_DATA;
   }

   if (CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      return ERROR_INVALID_DATA;
   }

   if ((1 == m_iNotObjectStore) && CHECK_SYSTEMGUID(&m_guid))
   {
      // Not an object store database volume but guid is for the object store db
      LeaveCriticalSection(&m_cs);
      return ERROR_INVALID_DATA;
   }

   if ((1 != m_iNotObjectStore) && !CHECK_SYSTEMGUID(&m_guid))
   {
      // Object store database but guid is not an object store guid
      LeaveCriticalSection(&m_cs);
      return ERROR_INVALID_DATA;
   }

   BOOL fRes = CloseHandle(m_hDB);
   if(m_oid && fRes)
   {      
      if (CeDeleteDatabaseEx(&m_guid, m_oid))
      {
         if (1 == m_iNotObjectStore)
         {
            fRes = CeUnmountDBVol(&m_guid);   //A database volume is also flushed when it is unmounted.
            if (fRes)
            {
               m_fMounted = FALSE;
            }
         }

         CREATE_INVALIDGUID(&m_guid);         
         m_hDB = NULL;
         m_oid = 0;

         if ((1 == m_iNotObjectStore) && fRes)
         {
            // Delete db file
            TCHAR szVolName[MAX_PATH];
            if (SUCCEEDED(StringCchCopy(szVolName, MAX_PATH, m_szCSIDLProfile)))
            {
               if (SUCCEEDED(StringCchCat(szVolName, MAX_PATH, DBVOLUMENAME)))
               {
                  if (!::DeleteFile(szVolName))
                  {
                     uReturn = GetLastError();
                  }
               }
            }
         }
      }
      else
      {
         uReturn = GetLastError();
         ASSERT(FALSE);
      }
   }
   else
   {
      uReturn = ERROR_INVALID_DATA;
   }

   LeaveCriticalSection(&m_cs);

   if ((ERROR_SUCCESS == uReturn) && !fRes)
   {
      uReturn = ERROR_INVALID_DATA;
   }

   return uReturn;
}

// Get Database type (located in the object store or not) and constract m_szBitBucketPath and m_szFindFile
HRESULT CRecycleBin::GetDBType()
{
   DWORD defVal = 0;
   HRESULT hr = NOERROR;

   EnterCriticalSection(&m_cs);

   if ((TEXT('\0') == m_szBitBucketPath[0]))
   {
      // RecycleBin DB location: object store | not the object store
   
      m_iNotObjectStore = GetDWORD(HKEY_LOCAL_MACHINE, EXPLORER, RECYCLE_BIN_ENABLEDBFILE, defVal);  
      if (1 == m_iNotObjectStore)
      {
         defVal = 0;
         m_fFlushVolume = (GetDWORD(HKEY_LOCAL_MACHINE, EXPLORER, RECYCLE_BIN_FLUSH, defVal) ? TRUE : FALSE);
            
         // Get a path where user specific data can be stored
         if (SHGetSpecialFolderPath(NULL, m_szCSIDLProfile, CSIDL_PROFILE, FALSE))
         {
            hr = StringCchCopy(m_szBitBucketPath, MAX_PATH, m_szCSIDLProfile);
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(m_szBitBucketPath, MAX_PATH, BITBUCKET);
               if (SUCCEEDED(hr))
               {
                  hr = StringCchCopy(m_szFindFile, MAX_PATH, m_szBitBucketPath);
                  if (SUCCEEDED(hr))
                  {
                     hr = StringCchCat(m_szFindFile, MAX_PATH, TEXT("\\*.*"));
                  }
               }
            }
         }
         else
         {  
            // For CSIDL_PROFILE: SHGetSpecialFolderPath is a wrapper for  GetUserDirectory.
               hr = E_FAIL;
         }
      }
      else
      {
         hr = StringCchCopy(m_szBitBucketPath, MAX_PATH, BITBUCKET);
         if (SUCCEEDED(hr))
         {
            hr =StringCchCopy(m_szFindFile, MAX_PATH, BITBUCKET);
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(m_szFindFile, MAX_PATH, TEXT("\\*.*"));
            }
         }
      }
   }

   LeaveCriticalSection(&m_cs);
   return hr;
}

BOOL CRecycleBin::GetDBVolume(CEGUID *pguid)
{
   TCHAR szVolName[MAX_PATH];
   BOOL fRes = FALSE;

   CREATE_INVALIDGUID(pguid);

   // Constract name of the volume to mount (name of the file that will contain one or more databases).
   // The format is UserData\BITBUCKET\DBVOLUMENAME (or \profiles\Default\Recycled\recycle.bin).

   if (SUCCEEDED(StringCchCopy(szVolName, MAX_PATH, m_szCSIDLProfile)))
   {
      if (SUCCEEDED(StringCchCat(szVolName, MAX_PATH, DBVOLUMENAME)))
      {
         int iTimeout = m_dwInitMountTimeout / 5000;
         if (1 > iTimeout)
         {
            iTimeout = 1;
         }

         for (int i = 0; i < iTimeout; i++)
         {
            // CeMountDBVol fills a buffer with a CEGUID that is the GUID 
            // of a mounted database, which identifies the location of the database file.
            // It can create a new volume or open an existing volume.
            if (CeMountDBVol (pguid, szVolName, OPEN_ALWAYS))
            {
               fRes = TRUE;
               m_fMounted = TRUE;

               DWORD dw = GetFileAttributes(szVolName);
               if (-1 != dw)
               {
                  if (!(FILE_ATTRIBUTE_SYSTEM & dw))
                  {
                     dw |= FILE_ATTRIBUTE_SYSTEM;
                  }

                  if (!(FILE_ATTRIBUTE_HIDDEN & dw))
                  {
                     dw |= FILE_ATTRIBUTE_SYSTEM;
                  }

                  if (!SetFileAttributes(szVolName, dw))
                  {
                     fRes = FALSE;
                  }
               }                  
               break;
            }

            // Could not mount - wait
            CREATE_INVALIDGUID (pguid);
            Sleep (5000);
         }
      }
   }

   return fRes;
}

DWORD CRecycleBin::GetDWORD(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, DWORD defVal)
{
   HKEY hKey = NULL;
   DWORD retVal = defVal;

   if (ERROR_SUCCESS == ::RegOpenKeyEx(hkeyRoot, pszPath, 0, 0, &hKey))
   {
      LPBYTE p = (LPBYTE) &retVal;
      DWORD dw = sizeof(retVal);
      if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, pszKey, 0, NULL, p, &dw))
      {
         retVal = defVal;
      }

      ::RegCloseKey(hKey);
   }
   else
   {
      retVal = defVal;
   }

   return retVal;
}

// GetFileInfo gets the original file info (name, date deleted) based on the RecycleBin file name.
CEOID CRecycleBin::GetFileInfo(LPCTSTR lpszRecycledName, LPTSTR lpszPath, const size_t cchPath, LPFILETIME lpft)
{
   CEOID oidRet = 0;
   CEPROPID rgPropId[2];
   CEPROPVAL *rgPropVal;
   DWORD dwBufSize;
   WORD wProps = 2;
   
   if (!lpszRecycledName || !m_hDB || (INVALID_HANDLE_VALUE == m_hDB))
   {
      return 0;
   }
   
   rgPropVal = (CEPROPVAL *)LocalAlloc(LMEM_FIXED, sizeof(CEPROPVAL) * 2);
   if (!rgPropVal)
   {
      return 0;
   }
   
   rgPropVal[0].propid = PROPID_FILE;
   rgPropVal[0].wFlags = 0;
   rgPropVal[0].val.lpwstr = (LPTSTR)PathFindFileName(lpszRecycledName);
   
   oidRet = CeSeekDatabaseEx(m_hDB, CEDB_SEEK_VALUEFIRSTEQUAL, 
               (DWORD)&rgPropVal[0],   //dwValue to use for the seek operation
               1,                      //the # of CEPROPVAL structures supplied in the dwValue parameter.
               NULL);
   if (oidRet)
   {
      if (lpszPath || lpft)
      {
         rgPropId[0] = PROPID_ORGPATH;
         rgPropId[1] = PROPID_DELDATE;
         dwBufSize = sizeof(CEPROPVAL) * 2;
         
         oidRet = CeReadRecordPropsEx(m_hDB, CEDB_ALLOWREALLOC, &wProps, rgPropId, 
                  (LPBYTE *)&rgPropVal, &dwBufSize, NULL);
         if (oidRet)
         {
            if (lpszPath)
            {
               // If StringCchCopyEx fails: the destination buffer will be set to the empty string.
               StringCchCopyEx(lpszPath, cchPath, rgPropVal[0].val.lpwstr, NULL, NULL, STRSAFE_NULL_ON_FAILURE);               
            }

            if (lpft)
            {
               *lpft = rgPropVal[1].val.filetime;
            }
         }
      }
   }

   LocalFree(rgPropVal);
   return oidRet;   
}

// GetTempFileName generates RecycleBin file name (format: "UserData\Recycled\tkxxx.ext") based on 
// the file extension
UINT CRecycleBin::GetTempFileName(DWORD dwAttrib, LPCTSTR lpszExt, LPTSTR *lplpsz)
{
   HANDLE hFile = NULL;
   LPTSTR lpszTmp;
   UINT uReturn = ERROR_SUCCESS;
   
   ASSERT(lplpsz);
   if (!lplpsz)
   {
      return ERROR_INVALID_PARAMETER;
   }

   *lplpsz = NULL;
   
   lpszTmp = new TCHAR[MAX_PATH + 1];
   if (lpszTmp)
   {
      for( ; ; )
      {
         HRESULT hr = ::StringCchPrintf(lpszTmp,
                                       MAX_PATH + 1,
                                       TEXT("%s\\tk%x"), 
                                       m_szBitBucketPath,
                                       Random());
         if (SUCCEEDED(hr))
         {
            if (lpszExt)
            {
               hr = StringCchCat(lpszTmp, MAX_PATH + 1, lpszExt);
               // Not a problem: no extension under the internal name
            }
            
            if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
            {
               DWORD dw = GetFileAttributes(lpszTmp);

               if ((-1 == dw) || !(FILE_ATTRIBUTE_DIRECTORY & dw))
               {
                  break;   // folder does not exist: the new name can be used
               }
            }
            else
            {
               hFile = CreateFile(lpszTmp, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
               
               if (INVALID_HANDLE_VALUE == hFile)   
               {
                  break;   // file does not exist: the new name can be used
               }
               else
               {
                  CloseHandle(hFile);
               }
            }
         }    
      }
      *lplpsz = lpszTmp;
   }
   else
   {
      uReturn = ERROR_OUTOFMEMORY;
   }

   return uReturn;   
}

DWORD CRecycleBin::GetUsedRecycleBinSize(ULONGLONG * ullUsed)
{
   DWORD dwCount = 0;
   WIN32_FIND_DATA fd;
      
   *ullUsed = 0;
   HANDLE hFile = FindFirstFile(m_szFindFile, &fd);
   if (INVALID_HANDLE_VALUE != hFile)
   {
      do 
      {         
         (*ullUsed) += ((ULONGLONG)fd.nFileSizeHigh * (ULONGLONG)MAXDWORD) + (ULONGLONG)fd.nFileSizeLow;
         dwCount++;
      } while (FindNextFile(hFile, &fd));
            
      FindClose(hFile);
   }

   return dwCount;
}

HRESULT CRecycleBin::GetTypeAndInit()
{
   HRESULT hr = GetDBType();  // Under object store or not
   
   if (SUCCEEDED(hr))
   {
      DWORD dwAttrib = GetFileAttributes(m_szBitBucketPath);
      if (-1 == dwAttrib)
      {
         if (CreateDirectory(m_szBitBucketPath, NULL))
         {
            SetFileAttributes(m_szBitBucketPath, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM);
         }
      }
      
      OpenDatabase();
   }
   else
   {
      ASSERT(FALSE);

      // Inform End-user about the failure
      ::MessageBox(NULL, LOAD_STRING(IDS_RECBININIT_FAILED), NULL,
                   MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
   }

   return hr;
}

BOOL CRecycleBin::MeetsFindCriteria()
{
   BOOL fRes = TRUE;

   ASSERT(INVALID_HANDLE_VALUE != m_hfind);
   ASSERT(NULL != m_hfind);

   if (FILE_ATTRIBUTE_DIRECTORY & m_FindData.dwFileAttributes)
   {
      // Only empty folders are under the Recycle Bin
      if ((m_FindData.nFileSizeHigh * MAXDWORD) + m_FindData.nFileSizeLow)
      {
         fRes = FALSE;
      }

      if (!(m_grfFlags & SHCONTF_FOLDERS))
      {
         fRes = FALSE;
      }
   }
   else if (!(m_grfFlags & SHCONTF_NONFOLDERS))
   {      
      fRes = FALSE;
   }
   
   return fRes;
}

BOOL CRecycleBin::Notify()
{
   if (m_hwndDesktop)
   {
      ::InvalidateRect(m_hwndDesktop, NULL, TRUE);
      ::UpdateWindow(m_hwndDesktop);
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

UINT CRecycleBin::OpenDatabase()
{
   BOOL fRes = TRUE;
   CEDBASEINFOEX dbInfo;
   SORTORDERSPECEX sos; 
   UINT uReturn = ERROR_SUCCESS;

   EnterCriticalSection(&m_cs);

   // TODO: Check the case:
   // If OpenDatabase() is called from BeginRecycle(),
   // the DB is not under the object store and was deleted or not created yet (no records)
   // then it is better to Mount and open it then AddRecord calls OpenDatabase().

   if (1 == m_iNotObjectStore)
   {
      // Do not mount if it is done (slow operation)
      if (!m_fMounted && CHECK_INVALIDGUID(&m_guid))
      {         
         fRes = GetDBVolume(&m_guid);  // guid for Not an object store DB
      }
   }
   else
   {
      // Object store DB
      if (!CHECK_SYSTEMGUID(&m_guid))
      {
         CREATE_SYSTEMGUID (&m_guid);  // guid gets ID of the database volume in the object store
      }
   }

   if (fRes)
   {   
      if (!CHECK_INVALIDGUID(&m_guid) && (!m_hDB || INVALID_HANDLE_VALUE == m_hDB))
      {
         memset(&dbInfo, 0, sizeof(CEDBASEINFOEX));

         HRESULT hr = StringCchCopy(dbInfo.szDbaseName, CEDB_MAXDBASENAMELEN, RECYCLEINFO);   
         if (SUCCEEDED(hr))
         {
            dbInfo.wVersion = CEDBASEINFOEX_VERSION;

            // MSDN:
            // CEDB_SYSTEMDB: The database is marked as a system database and cannot be deleted by an untrusted application.
            // An untrusted application cannot make the following API calls on a system database file: 
            // CeCreateDatabaseEx, CeSetDatabaseInfoEx, or CeDeleteDatabase.

            // CeCreateDatabaseEx2 cannot set system flag inside non-system file, hence
            // do not set CEDB_SYSTEMDB for Not Object store DB
            dbInfo.dwFlags = CEDB_VALIDNAME | CEDB_VALIDTYPE | CEDB_VALIDSORTSPEC;
         
            if (1 != m_iNotObjectStore)
            {
               dbInfo.dwFlags |= CEDB_SYSTEMDB;
            }

            dbInfo.dwDbaseType = CLSID_RECYLCEDB;  //the type identifier for the database
            dbInfo.wNumSortOrder = 1;              //# of sort orders active in the database. 

            sos.wVersion = 1;
            sos.wNumProps = 1;
            sos.wKeyFlags = 0;
            sos.rgPropID[0] = PROPID_FILE;
            sos.rgdwFlags[0] = CEDB_SORT_DESCENDING;

            //sort order descriptions
            dbInfo.rgSortSpecs[0] = sos;
   
            m_oid = 0;
            m_hDB = CeOpenDatabaseEx2(&m_guid, &m_oid, dbInfo.szDbaseName, &sos, 0, NULL);
            if (INVALID_HANDLE_VALUE == m_hDB)
            {
               if (CeCreateDatabaseEx2(&m_guid, &dbInfo))
               {
                  m_oid = 0;
                  m_hDB = CeOpenDatabaseEx2(&m_guid, &m_oid, dbInfo.szDbaseName, &sos, 0, NULL);
                  if (INVALID_HANDLE_VALUE == m_hDB)
                  {
                     uReturn = GetLastError();
                     CREATE_INVALIDGUID(&m_guid);
                     m_oid = 0;
                  }
               }
               else
               {
                  uReturn = GetLastError();
                  CREATE_INVALIDGUID(&m_guid);
                  m_oid = 0;
                  ASSERT(FALSE);
               }
            }
         }
         else
         {
            uReturn = HRESULT_CODE(hr);
         }
      }
   }
   else
   {
      // GetDBVolume failed
      ASSERT(FALSE);
      uReturn = ERROR_INVALID_DATA;
   }

   LeaveCriticalSection(&m_cs);

   return uReturn;   
}

// AddFileFolder adds a file OR an EMPTY folder to the RecycleBin and returns single-level PIDL
HRESULT CRecycleBin::AddFileFolder(LPCWSTR pwszFullPath, LPITEMIDLIST * ppidl)
{
   int result = IDYES;
   LPITEMIDLIST pidl = NULL;
   LPTSTR lpszRecycleName = NULL;
   ULONGLONG ullFileSize = 0;
   HRESULT hr = S_OK;
   
   ASSERT(pwszFullPath);
   if (!pwszFullPath)
   {
      return E_FAIL;
   }
   ASSERT(ppidl);
   if (!ppidl)
   {
      return E_FAIL;
   }
    
   // EMPTY folders can be added to the RecycleBin
   DWORD dwAttrib = GetFileAttributes(pwszFullPath);
   if (-1 == dwAttrib)
   {
      return E_FAIL;
   }
      
   if (!(FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
   {
      HANDLE hFile = CreateFile(pwszFullPath, GENERIC_READ, 0, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

      if (INVALID_HANDLE_VALUE != hFile)
      { 
         if (CanAddFile(hFile, &ullFileSize))
         {
            CloseHandle(hFile);

            ULONGLONG ullSize = StoreSize();

            ULONGLONG ullUsed = 0;
            GetUsedRecycleBinSize(&ullUsed);
            ullUsed += ullFileSize;

            if (ullSize && (ullUsed > (ULONGLONG)m_dwPercentage * (ullSize / (ULONGLONG)100)))
            {
               //Pop up a dialog to allow the user to empty the RecycleBin.                  
               HWND hWndWarning = FindWindow(TEXT("Dialog"), LOAD_STRING(IDS_PROPERTIES_RECBINMAX));
               if (hWndWarning)
               {
                  SetForegroundWindow(hWndWarning);
               }
               else
               {                  
                  LPTSTR pszStr = (LPTSTR)LocalAlloc(LMEM_FIXED, MAX_PATH * sizeof(TCHAR));
                  if (pszStr)
                  {
                     hr = ::StringCchPrintfEx(pszStr,
                                 MAX_PATH,
                                 NULL,
                                 NULL,
                                 STRSAFE_IGNORE_NULLS,
                                 LOAD_STRING(IDS_CONFIRM_DEL_BIG),
                                 PathFindFileName(pwszFullPath));
                     if (SUCCEEDED(hr))
                     {
                        result = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_RECYCLEBINFULL), NULL,                                    
                                          ShellDialogs::RecycleBinWarnDlgProc, (LPARAM) pszStr);
                        
                        // Cancel means cancel the whole operation
                        if (IDCANCEL == result)
                        {
                           hr = SHFO_ERR_CANCELLED;
                        }
                     }                  
                  }

                  LocalFree((HLOCAL)pszStr);
               }
            }
         }
         else
         {
            CloseHandle(hFile);

            // File size > Recycle Bin size:  delete the file permanently.
            hr = ShellDialogs::Confirm::DeleteForRecycleBin(pwszFullPath, NULL, TRUE, &result);
            if (SUCCEEDED(hr))
            {
               switch (result)
               {
                  case IDCANCEL:
                  case IDNO:
                     hr = SHFO_ERR_CANCELLED;   // Cancel the whole operation
                     break;
                  break;

                  case IDYES:
                  {
                     if (!::DeleteFile(pwszFullPath))
                     {
                        hr = HRESULT_FROM_WIN32_I(GetLastError());
                     }
                     else
                     {
                        hr = S_FALSE;
                     }
                  }
               }
            }
         }         
      }
   }
   else
   {
      // Empty folders and Empty RecBin
      if (!m_dwPercentage)
      {
         hr = ShellDialogs::Confirm::DeleteForRecycleBin(pwszFullPath, NULL, TRUE, &result);
         if (SUCCEEDED(hr))
         {
            switch (result)
            {
               case IDCANCEL:
               case IDNO:
                  hr = SHFO_ERR_CANCELLED;
                  break;

               case IDYES:
               {
                  if (!::RemoveDirectory(pwszFullPath))
                  {
                     hr = HRESULT_FROM_WIN32_I(GetLastError());
                  }
                  else
                  {
                     hr = S_FALSE;
                  }
               }
            }
         }                    
      }
   }
   
   if (S_OK == hr)
   {
      //Generate RecycleBin filename and move the file to the RecycleBin
      if (ERROR_SUCCESS == GetTempFileName(dwAttrib, PathFindExtension((LPCTSTR)pwszFullPath), &lpszRecycleName))      
      {  
         BOOL fEmpty = IsEmpty();   //Check content before adding a file

         //GetTempFileName allocated memory for Recycle Bin file|folder name.
         
         //MoveFile fails on directory moves when the destination is on a different volume.
         if (MoveFile(pwszFullPath, (LPCTSTR)lpszRecycleName))
         {            
            //Success: add the record to the DB if it does not exist.            
            if (ERROR_SUCCESS == AddRecord(lpszRecycleName, (LPCTSTR)pwszFullPath))
            {               
               if (SUCCEEDED(CreateFileSystemPidl(PathFindFileName(lpszRecycleName), &pidl)))
               {
                  if(fEmpty)
                  {
                     Notify();
                  }
               }               
            } 
            else
            { 
              //Record is not added, move the file back.
               if (!MoveFile(lpszRecycleName, (LPCTSTR)pwszFullPath))
               {
                  hr = HRESULT_FROM_WIN32_I(GetLastError());
               }
            }
         }
         else
         {
            hr = HRESULT_FROM_WIN32_I(GetLastError());
         }
         
         delete [] lpszRecycleName;
         lpszRecycleName = NULL;
      }
      *ppidl = pidl;
   }

   return hr;
} 

HRESULT CRecycleBin::DeleteAll(BOOL fConfirmDel)
{   
   CEOID oidRecord;
   TCHAR szRecPath[MAX_PATH];
   ULONGLONG ullUsed = 0;
   HRESULT hr = NOERROR;

   EnterCriticalSection(&m_cs);

   if (!m_hDB || (INVALID_HANDLE_VALUE == m_hDB) || !m_oid || CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      return E_FAIL;
   }
   
   if (fConfirmDel)
   {   
      DWORD dwCount = GetUsedRecycleBinSize(&ullUsed);
      if (dwCount)
      {
         int result = IDYES;

         //Always confirm permanent delete
         if (1 == dwCount)
         {                            
            WIN32_FIND_DATA fd = {0};   
            HANDLE hFile = FindFirstFile(m_szFindFile, &fd);

            if (INVALID_HANDLE_VALUE != hFile)
            {
               TCHAR szRecName[MAX_PATH], szOrigPath[MAX_PATH] = TEXT(""); 

               FindClose(hFile);

               hr = StringCchCopy(szRecName, MAX_PATH, m_szBitBucketPath);
               if (SUCCEEDED(hr))
               {
                  hr = StringCchCat(szRecName, MAX_PATH, TEXT("\\"));
                  if (SUCCEEDED(hr))
                  {
                     hr = StringCchCat(szRecName, MAX_PATH, fd.cFileName);
                     if (SUCCEEDED(hr))
                     {
                        if (GetFileInfo(szRecName, szOrigPath, MAX_PATH, NULL))
                        {
                           hr = ShellDialogs::Confirm::DeleteForRecycleBin(PathFindFileName(szOrigPath),
                                                                              NULL, FALSE, &result); 
                        }
                        else
                        {
                           hr = E_FAIL;
                        }
                     }
                  }
               }
            }
            else
            {
               hr = E_FAIL;
            }
         }
         else
         {
            hr = ShellDialogs::Confirm::DeleteMultiple(dwCount, NULL, TRUE, &result);
         }

         if (SUCCEEDED(hr) && IDYES == result)
         {
            hr = S_OK;
         }
         else
         {
            hr = S_FALSE;
         }
      }
   }
   
   if (S_FALSE != hr)
   {
      WIN32_FIND_DATA fd = {0};
      
      HANDLE hFile = FindFirstFile(m_szFindFile, &fd);
      if (INVALID_HANDLE_VALUE != hFile) 
      {
         do
         {
            hr = StringCchCopy(szRecPath, MAX_PATH, m_szBitBucketPath);
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(szRecPath, MAX_PATH, TEXT("\\"));
               if (SUCCEEDED(hr))
               {
                  hr = StringCchCat(szRecPath, MAX_PATH, fd.cFileName);
                  if (SUCCEEDED(hr))
                  {
                     DWORD dwAttrib = GetFileAttributes(szRecPath);
                     if (-1 != dwAttrib)
                     {
                        if (FILE_ATTRIBUTE_READONLY & dwAttrib)
                        {
                           SetFileAttributes(szRecPath, 0);
                        }
             
                        BOOL fDeleted = FALSE;
                        if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
                        {
                           fDeleted = ::RemoveDirectory(szRecPath);
                        }
                        else
                        {
                           fDeleted = ::DeleteFile(szRecPath);
                        }

                        if (fDeleted)
                        {                 
                           //Seek the record based on the recycled name
                           oidRecord = GetFileInfo(szRecPath, NULL, 0, NULL);
                           if (oidRecord) 
                           {         
                              if (!CeDeleteRecord(m_hDB, oidRecord))
                              {
                                 hr = E_FAIL;
                              }
                           }
                           //Else: No record to delete
                        } 
                        else
                        {      
                           hr = E_FAIL;
                        }
                     }
                     else
                     {
                        hr = E_FAIL;
                     }
                  }
               }
            }
                  
         } while (FindNextFile(hFile, &fd) && SUCCEEDED(hr));

         FindClose(hFile);
      }
      
      LeaveCriticalSection(&m_cs);

      if (SUCCEEDED(hr))
      {         
         if (ERROR_SUCCESS != DeleteDatabase())
         {
            hr = E_FAIL;
         }
         
         Notify();
      }
   }
   else
   {
      LeaveCriticalSection(&m_cs);
   }

   return hr;
}

// DeleteFile deletes file (if it was found) from the RecycleBin and the corresponding database record.
HRESULT CRecycleBin::DeleteFileFolder(LPCITEMIDLIST pidl)
{
   BOOL fDeleted = FALSE;
   CEOID oidRecord;
   TCHAR szRecName[MAX_PATH];   
   HRESULT hr = E_FAIL;

   ASSERT(pidl);
   if (!pidl)
   {
      return hr;
   }

   EnterCriticalSection(&m_cs);

   if (!m_hDB || (INVALID_HANDLE_VALUE == m_hDB) || !m_oid || CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      return hr;
   }

   // Get Recycled name from pidl
   hr = StringCchCopy(szRecName, lengthof(szRecName), ILDisplayName(pidl));
   if (SUCCEEDED(hr))
   {
      oidRecord = GetFileInfo(szRecName, NULL, NULL, NULL);
      if (oidRecord)
      {
         size_t strLen = _tcslen(m_szBitBucketPath) + _tcslen(szRecName) + 2;

         LPTSTR lpszRecPath = new TCHAR[strLen]; 
         if (lpszRecPath)
         {
            hr = StringCchCopy(lpszRecPath, strLen, m_szBitBucketPath);
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(lpszRecPath, strLen, TEXT("\\"));
               if (SUCCEEDED(hr))
               {
                  hr = StringCchCat(lpszRecPath, strLen, szRecName);
                  if (SUCCEEDED(hr))
                  {
                     DWORD dwAttrib = GetFileAttributes(lpszRecPath);
                     if (-1 != dwAttrib)
                     {
                        if (FILE_ATTRIBUTE_READONLY & dwAttrib)
                        {
                           SetFileAttributes(lpszRecPath, 0);
                        }

                        if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
                        {
                           fDeleted = ::RemoveDirectory(lpszRecPath);
                        }
                        else
                        {
                           fDeleted = ::DeleteFile(lpszRecPath);
                        }

                        if (fDeleted)
                        {
                           if (CeDeleteRecord(m_hDB, oidRecord))
                           {
                              hr = NOERROR;
                           }
                        }
                     }
                  }
               }
            }
         }

         delete [] lpszRecPath;
      }
   }

   LeaveCriticalSection(&m_cs);

   if (fDeleted && SUCCEEDED(hr))
   {
      if(IsEmpty())
      {
         Notify();
      }
   }

   return hr;
}

// GetFileDateDel returns "date deleted" for the original file corresponding to the RecycleBin file pidl. 
BOOL CRecycleBin::GetFileDateDel(LPCITEMIDLIST pidl, FILETIME* pftDateDel)
{
   BOOL bRet = FALSE;
   WCHAR szRecName[MAX_PATH];

   if (!pidl || !pftDateDel)
   {
      ASSERT(0);
      goto leave;
   }

   EnterCriticalSection(&m_cs);

   if (!m_hDB || (INVALID_HANDLE_VALUE == m_hDB) || !m_oid || CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      goto leave;
   }

   //Get Recycled name from pidl
   if (SUCCEEDED(StringCchCopy(szRecName, lengthof(szRecName), ILDisplayName(pidl))))
   {
      if (GetFileInfo(szRecName, NULL, 0, pftDateDel))
      {
         bRet = TRUE;
      }
   }

   LeaveCriticalSection(&m_cs);

leave:
   return bRet;
}

// GetFileOrigLoc returns original file location corresponding to the RecycleBin file pidl.
LPOLESTR CRecycleBin::GetFileOrigLoc(LPCITEMIDLIST pidl)
{
   LPWSTR pwszOrigLoc = NULL;
   TCHAR szRecName[MAX_PATH], szOrigPath[MAX_PATH];
   
   ASSERT(pidl);
   if (!pidl)
   {
      return NULL;
   }

   EnterCriticalSection(&m_cs);

   if (!m_hDB || (INVALID_HANDLE_VALUE == m_hDB) || !m_oid || CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      return NULL;
   }

   //Get Recycled name from pidl
   if (SUCCEEDED(StringCchCopy(szRecName, lengthof(szRecName), ILDisplayName(pidl))))
   {      
      if (GetFileInfo(szRecName, szOrigPath, MAX_PATH, NULL))
      {
         //Get path, no name
         PathRemoveFileSpec(szOrigPath);

         size_t strLen = wcslen((LPWSTR)szOrigPath); 
         if (g_pShellMalloc)
         {   
            if (strLen)
            {
               pwszOrigLoc = (LPWSTR) g_pShellMalloc->Alloc((strLen + 1) * sizeof(WCHAR));
               if (pwszOrigLoc)
               {
                  wcscpy(pwszOrigLoc, (LPWSTR)szOrigPath);
               }
            }
            else
            {
               //Case of "My Computer" which should be displayed as "\"
               pwszOrigLoc = (LPWSTR) g_pShellMalloc->Alloc(2 * sizeof(WCHAR));
               if (pwszOrigLoc)
               {
                  wcscpy(pwszOrigLoc, L"\\");
               }
            }
         }
      }
   }

   LeaveCriticalSection(&m_cs);
   return (LPOLESTR)pwszOrigLoc;
}

// GetFileOrigName returns original file name corresponding to the RecycleBin file pidl. 
LPOLESTR CRecycleBin::GetFileOrigName(LPCITEMIDLIST pidl)
{
   LPWSTR pwszOrigName = NULL;
   TCHAR szRecName[MAX_PATH], szOrigPath[MAX_PATH];
   HRESULT hr = S_OK;
   
   ASSERT(pidl);
   if (!pidl)
   {
      return NULL;
   }

   EnterCriticalSection(&m_cs);

   if (!m_hDB || (INVALID_HANDLE_VALUE == m_hDB) || !m_oid || CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      return NULL;
   }

   //Get Recycled name from pidl
   if (SUCCEEDED(StringCchCopy(szRecName, lengthof(szRecName), ILDisplayName(pidl))))
   {      
      if (GetFileInfo(szRecName, szOrigPath, MAX_PATH, NULL))
      {
         //Get file name only
         hr = StringCchCopy(szOrigPath, MAX_PATH, PathFindFileName(szOrigPath));

         size_t strLen = wcslen((LPWSTR)szOrigPath);

         if (SUCCEEDED(hr) && strLen && g_pShellMalloc)
         {            
            pwszOrigName = (LPWSTR) g_pShellMalloc->Alloc((strLen + 1) * sizeof(WCHAR));
            if (pwszOrigName)
            {
               wcscpy(pwszOrigName, (LPWSTR)szOrigPath);
            }
         }
      }
   }

   LeaveCriticalSection(&m_cs);
   return (LPOLESTR)pwszOrigName;
}

// GetFileSize returns size for the file corresponding to the RecycleBin pidl.
BOOL CRecycleBin::GetFileSize(LPCITEMIDLIST pidl, ULONGLONG * pullSize)
{
   LPTSTR lpszRecPath = NULL;
   WIN32_FIND_DATA fd;
   TCHAR szRecName[MAX_PATH];
   BOOL fReturn = FALSE;

   ASSERT(pidl);
   if (!pidl)
   {
      return FALSE;
   }

   *pullSize = 0;

   //Get Recycled name from pidl
   if (SUCCEEDED(StringCchCopy(szRecName, lengthof(szRecName), ILDisplayName(pidl))))
   {
      size_t strLen = _tcslen(m_szBitBucketPath) + _tcslen(szRecName) + 2;

      lpszRecPath = new TCHAR[strLen]; 
      if (lpszRecPath)
      {
         HRESULT hr = StringCchCopy(lpszRecPath, strLen, m_szBitBucketPath);
         if (SUCCEEDED(hr))
         {
            hr = StringCchCat(lpszRecPath, strLen, TEXT("\\"));
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(lpszRecPath, strLen, szRecName);
               if (SUCCEEDED(hr))
               {
                  HANDLE hFile = FindFirstFile(lpszRecPath, &fd);
                  if (INVALID_HANDLE_VALUE != hFile)
                  {            
                     //Return in bytes. The caller will convert it in a proper format
                     *pullSize = ((ULONGLONG)fd.nFileSizeHigh * (ULONGLONG)MAXDWORD) + (ULONGLONG)fd.nFileSizeLow;      
                     FindClose(hFile);
                     fReturn = TRUE;
                  }
               }
            }
         }
      }

      delete [] lpszRecPath;
   }
                   
   return fReturn;
}

UINT CRecycleBin::GetPercentage()
{
   return m_dwPercentage;
}

void CRecycleBin::Initialize()
{
   if ((INVALID_HANDLE_VALUE != m_hfind) && (NULL != m_hfind))
   {
      FindClose(m_hfind);
   }
 
   m_hfind = FindFirstFile(m_szFindFile, &m_FindData);

   // If there are no files in this directory set m_hfind as a ended search (see header info on m_hfind)
   if (INVALID_HANDLE_VALUE == m_hfind)
   {
      m_hfind = NULL;
   }
}

BOOL CRecycleBin::IsEmpty()
{
   WIN32_FIND_DATA fd;
   BOOL fEmpty = TRUE;

   HANDLE hFile = FindFirstFile(m_szFindFile, &fd);
   if (INVALID_HANDLE_VALUE != hFile)
   {
      FindClose(hFile);      
      fEmpty = FALSE;
   }                        
   
   return fEmpty;
}

BOOL CRecycleBin::IsFull()
{
   ULONGLONG ullUsed = 0;
   BOOL fFull = FALSE;

   if (!m_dwPercentage)
   {
      return TRUE;
   }

   GetUsedRecycleBinSize(&ullUsed);
   if (ullUsed)
   {
      ULONGLONG ullSize = StoreSize();

      if (ullSize && (ullUsed > (ULONGLONG)m_dwPercentage * (ullSize / (ULONGLONG)100)))
      {
         fFull = TRUE;
      }
   }
         
   return fFull;
}

// RestoreFile restores file or an empty folder (if it was found) from the RecycleBin and deletes
// the corresponding database record.
HRESULT CRecycleBin::RestoreFile(LPCITEMIDLIST pidl)
{
   BOOL fRestoreResult = FALSE;
   CEOID   oidRecord;
   SHFILEOPSTRUCT shfo = {0};
   TCHAR szRecName[MAX_PATH], szOrigPath[MAX_PATH + 1];
   HRESULT hr = E_FAIL;

   ASSERT(pidl);
   if (!pidl)
   {
      return hr;
   }

   EnterCriticalSection(&m_cs);

   if (!m_hDB || (INVALID_HANDLE_VALUE == m_hDB) || !m_oid || CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      return hr;
   }

   //Get Recycled name from pidl.
   hr = StringCchCopy(szRecName, lengthof(szRecName), ILDisplayName(pidl));
   if (SUCCEEDED(hr))
   {
      //Get a path to restore
      oidRecord = GetFileInfo(szRecName, szOrigPath, MAX_PATH, NULL);
      if (oidRecord)
      {
         size_t strLen = _tcslen(m_szBitBucketPath) + _tcslen(szRecName) + 2;

         LPTSTR lpszRecPath = new TCHAR[strLen + 1]; //+1 for the 2nd \0
         if (lpszRecPath)
         {
            hr = StringCchCopy(lpszRecPath, strLen, m_szBitBucketPath);
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(lpszRecPath, strLen, TEXT("\\"));
               if (SUCCEEDED(hr))
               {
                  hr = StringCchCat(lpszRecPath, strLen, szRecName);
                  if (SUCCEEDED(hr))
                  {
                     BOOL fChangeAttr = FALSE;
                     DWORD dwFromAttrib = GetFileAttributes(lpszRecPath);

                     if ((FILE_ATTRIBUTE_READONLY & dwFromAttrib) ||
                        FILE_ATTRIBUTE_SYSTEM & dwFromAttrib)
                     {
                        // To avoid getting internal recbin file name under 
                        // confirm msg from SHFileOperation
                        SetFileAttributes(lpszRecPath, 0);
                        fChangeAttr = TRUE;
                     }
                                          
                     BOOL fDoRestore = TRUE;                     
                     if ((FILE_ATTRIBUTE_DIRECTORY & dwFromAttrib))
                     {
                        // Empty folder restore
                        DWORD dwAttrib = GetFileAttributes(szOrigPath);
                        if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
                        {
                           // Do not need to restore (the folder already exist).
                           // Delete lpszRecPath and the the record
                           fDoRestore = FALSE;
                           if (::RemoveDirectory(lpszRecPath))
                           {
                              if (CeDeleteRecord(m_hDB, oidRecord))
                              {
                                 hr = NOERROR;
                              }
                              else
                              {
                                 hr = E_FAIL;
                              }
                           }
                           else
                           {
                              hr = HRESULT_FROM_WIN32_I(GetLastError());
                           }
                        }
                     }

                     if (fDoRestore)
                     {
                        szOrigPath[_tcslen(szOrigPath) + 1] = _T('\0');
                        lpszRecPath[_tcslen(lpszRecPath) + 1] = _T('\0');

                        shfo.wFunc  = FO_MOVE; 
                        shfo.pFrom  = lpszRecPath;
                        shfo.pTo    = szOrigPath;
                        shfo.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR;

                        //Since the user can choose 'No' for the file operation and SHFileOperation 
                        //will still succeed we need the PathFileExists check
                        if (!SHFileOperation(&shfo) && !PathFileExists(lpszRecPath))
                        {
                           if (CeDeleteRecord(m_hDB, oidRecord))
                           {
                              hr = NOERROR;
                           }
                           else
                           {
                              hr = E_FAIL;
                           }

                           if (fChangeAttr)
                           {
                              SetFileAttributes(szOrigPath, dwFromAttrib);
                           }

                           fRestoreResult = TRUE;
                        }
                        else
                        {
                           // lpszRecPath is under the recbin
                           if (fChangeAttr)
                           {
                              SetFileAttributes(lpszRecPath, dwFromAttrib);
                           }
                        }
                     }
                  }
               }
            }
         }

         delete [] lpszRecPath;
      }
   }

   LeaveCriticalSection(&m_cs);

   if (fRestoreResult && SUCCEEDED(hr))
   {
      if (IsEmpty())
      {
         Notify();
      }
   }

   return hr;
}

HRESULT CRecycleBin::RestoreAll()
{
   BOOL fRestoreResult = FALSE;
   CEOID   oidRecord;
   SHFILEOPSTRUCT shfo = {0};
   TCHAR   szRecPath[MAX_PATH + 1], szOrigPath[MAX_PATH + 1];
   WIN32_FIND_DATA fd;
   HRESULT hr = NOERROR;

   EnterCriticalSection(&m_cs);

   if (!m_hDB || (INVALID_HANDLE_VALUE == m_hDB) || !m_oid || CHECK_INVALIDGUID(&m_guid))
   {
      LeaveCriticalSection(&m_cs);
      return E_FAIL;
   }

   HANDLE hFile = FindFirstFile(m_szFindFile, &fd);
   if (INVALID_HANDLE_VALUE != hFile) 
   {
      shfo.wFunc  = FO_MOVE;
      shfo.fFlags = FOF_SILENT;
      do
      {
         hr = StringCchCopy(szRecPath, MAX_PATH, m_szBitBucketPath);
         if (SUCCEEDED(hr))
         {
            hr = StringCchCat(szRecPath, MAX_PATH, TEXT("\\"));
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(szRecPath, MAX_PATH, fd.cFileName);
               if (SUCCEEDED(hr))
               {
                  //Get a Path to restore
                  oidRecord = GetFileInfo(szRecPath, szOrigPath, MAX_PATH, NULL);
                  if (oidRecord)
                  {
                     BOOL fChangeAttr = FALSE;
                     DWORD dwFromAttrib = GetFileAttributes(szRecPath);

                     if ((FILE_ATTRIBUTE_READONLY & dwFromAttrib) ||
                        FILE_ATTRIBUTE_SYSTEM & dwFromAttrib)
                     {
                        // To avoid getting internal recbin file name under 
                        // confirm msg from SHFileOperation
                        SetFileAttributes(szRecPath, 0);
                        fChangeAttr = TRUE;
                     }
                     
                     BOOL fDoRestore = TRUE;
                     if ((FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(szRecPath)))
                     {
                        // Case for Empty folder restore
                        DWORD dwAttrib = GetFileAttributes(szOrigPath);
                        if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
                        {
                           // Do not need to restore (the folder already exist).
                           // Delete lpszRecPath and the the record
                           fDoRestore = FALSE;
                           if (::RemoveDirectory(szRecPath))
                           {
                              if (CeDeleteRecord(m_hDB, oidRecord))
                              {
                                 hr = NOERROR;
                              }
                              else
                              {
                                 hr = E_FAIL;
                              }
                           }
                           else
                           {
                              hr = HRESULT_FROM_WIN32_I(GetLastError());
                           }
                        }
                     }

                     if (fDoRestore)
                     {
                        szOrigPath[_tcslen(szOrigPath) + 1] = _T('\0');
                        szRecPath[_tcslen(szRecPath) + 1] = _T('\0');

                        shfo.pFrom  = szRecPath;
                        shfo.pTo    = szOrigPath;
                        if (!SHFileOperation(&shfo))
                        {
                           //Since the user can choose 'No' for the file operation and SHFileOperation 
                           //will still succeed we need the PathFileExists check
                           if (!PathFileExists(szRecPath))
                           {
                              if (!CeDeleteRecord(m_hDB, oidRecord))
                              {
                                 hr = E_FAIL;
                              }

                              if (fChangeAttr)
                              {
                                 SetFileAttributes(szOrigPath, dwFromAttrib);
                              }
                           }
                           else
                           {  // 'No' was choosen for SHFileOperation call
                              fRestoreResult = TRUE;

                              // lpszRecPath is under the recbin
                              if (fChangeAttr)
                              {
                                 SetFileAttributes(szRecPath, dwFromAttrib);
                              }
                           }
                        }
                        else
                        {
                           hr = E_FAIL;
                           // lpszRecPath is under the recbin
                           if (fChangeAttr)
                           {
                              SetFileAttributes(szRecPath, dwFromAttrib);
                           }
                        }
                     }
                  }
               }
            }
         }         
      } while (FindNextFile(hFile, &fd) && SUCCEEDED(hr));

      FindClose(hFile);
   }

   LeaveCriticalSection(&m_cs);
   
   if (SUCCEEDED(hr) && !fRestoreResult)
   {      
      if (ERROR_SUCCESS != DeleteDatabase())
      {
         hr = E_FAIL;
      }

      Notify();
   }

   return hr;
}

VOID CRecycleBin::SetDesktopWindow(HWND hwndDesktop)
{
   m_hwndDesktop = hwndDesktop;
}

BOOL CRecycleBin::SetPercentage(UINT dwNewPercentage)
{
   DWORD   dwOldPercentage;
   BOOL fRes = TRUE;
   
   ASSERT(dwNewPercentage >= 0);
   ASSERT(dwNewPercentage <= 100);
   if (0 > dwNewPercentage || dwNewPercentage > 100)
   {
      return FALSE;
   }

   if (dwNewPercentage == m_dwPercentage)
   {
      return TRUE;
   }

   UserSettings::SetRecycleBinSize(dwNewPercentage);

   //Verify if the Recycle Bin is full with the new settings
   dwOldPercentage = m_dwPercentage;
   m_dwPercentage = dwNewPercentage;

   if (m_dwPercentage < dwOldPercentage)
   {
      Notify();
   }
   
   return fRes;
}

ULONGLONG CRecycleBin::StoreSize()
{
   ULONGLONG ullSize = 0;

   if (1 == m_iNotObjectStore)
   {
      ULARGE_INTEGER uliFreeSpace, uliTotalBytes;
      uliTotalBytes.QuadPart = 0;

      if (GetDiskFreeSpaceEx(m_szCSIDLProfile, &uliFreeSpace, &uliTotalBytes, NULL))
      {
         ullSize = uliTotalBytes.QuadPart;
      }
   }
   else
   {
      DWORD dwStorePages = 0, dwRamPages, dwPageSize = 0;
      if (GetSystemMemoryDivision(&dwStorePages, &dwRamPages, &dwPageSize))
      {
         ullSize = dwStorePages * dwPageSize;
      }
   }

   return ullSize;
}

//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CRecycleBin::QueryInterface(REFIID riid, LPVOID FAR* ppobj)
{
   HRESULT hr = E_NOINTERFACE;

   if (!ppobj)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppobj = NULL;

   if(IsEqualIID(riid, IID_IUnknown))        // IUnknown
   {
      *ppobj = this;
   }
   else if(IsEqualIID(riid, IID_IEnumIDList))// IEnumIDList
   {
      *ppobj = (IEnumIDList *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr =S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CRecycleBin::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CRecycleBin::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}

//////////////////////////////////////////////////
// IEnumIDList 

STDMETHODIMP CRecycleBin::Clone(IEnumIDList ** ppenum)
{
   return E_NOTIMPL;
}

STDMETHODIMP CRecycleBin::Next(ULONG celt,
                               LPITEMIDLIST * rgelt,
                               ULONG * pceltFetched)
{
   HRESULT hr = NOERROR;

   ASSERT(rgelt);
   if (!rgelt)
   {
      return E_INVALIDARG;
   }

  if (INVALID_HANDLE_VALUE == m_hfind)
  {
      return E_UNEXPECTED;
  }

   if (NULL == m_hfind)
   {
      return S_FALSE;
   }

   while (!MeetsFindCriteria())
   {
      if (!FindNextFile(m_hfind, &m_FindData))
      {
         FindClose(m_hfind);
         m_hfind = NULL;
         return S_FALSE;
      }
   }

   hr = CreateFileSystemPidl(m_FindData.cFileName, rgelt);
   if (pceltFetched)
   {
      *pceltFetched = (SUCCEEDED(hr) ? 1 : 0);
   }

   if (!FindNextFile(m_hfind, &m_FindData))
   {
      FindClose(m_hfind);
      m_hfind = NULL;
   }

   return hr;
}

STDMETHODIMP CRecycleBin::Reset()
{
   return E_NOTIMPL;
}

STDMETHODIMP CRecycleBin::Skip(ULONG celt)
{
   return E_NOTIMPL;
}

