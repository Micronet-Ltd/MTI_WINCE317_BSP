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

//
// This module contains routines for keeping track of devices and 
// managing device power.
//

#include <pmimpl.h>
#include <CSync.h>
#include <CMthread.h>

// This routine enumerates device power restrictions in the registry
// and adds them to the list of existing restrictions.  It returns a pointer
// to the new list.

class RegKeyOrValue {
public:
    RegKeyOrValue(HKEY hKey,LPCTSTR lpRegName,RegKeyOrValue *pNextRegKeyOrValue) : m_hParentKey (hKey),m_pNextRegKeyOrValue(pNextRegKeyOrValue) {
        m_lpRegName = NULL;
        if (lpRegName) {
            m_lpRegName = new TCHAR [ _tcslen(lpRegName) +1 ];
            if (m_lpRegName)
                _tcscpy(m_lpRegName,lpRegName);
        }
        else 
            m_lpRegName = NULL;
        
    };
    virtual BOOL Init() { return m_lpRegName!=NULL; };
    virtual ~RegKeyOrValue() {
        if (m_lpRegName!=NULL)
            delete m_lpRegName;
    }
    LPTSTR  GetName() { return m_lpRegName; };
    RegKeyOrValue * SetNextRegKeyOrValuePtr (RegKeyOrValue *pNextRegKeyOrValue) { 
        RegKeyOrValue *  pReturnPtr = m_pNextRegKeyOrValue;
        m_pNextRegKeyOrValue = pNextRegKeyOrValue;
        return  pReturnPtr;
    }
    RegKeyOrValue * GetNextRegKeyOrValuePtr() { return m_pNextRegKeyOrValue; };
protected:
    LPTSTR m_lpRegName;
    HKEY   m_hParentKey;
    RegKeyOrValue * m_pNextRegKeyOrValue;
};

class RegValue : public RegKeyOrValue{
public:
    RegValue(HKEY hKey,LPCTSTR lpRegName,RegValue * pNextRegValue ) :  RegKeyOrValue(hKey,lpRegName,pNextRegValue) { 
        m_dwValueSize=0;
        m_lpByteValue = NULL;
        UpdateRegValue();
    };
    virtual ~RegValue () {
        if ( m_lpByteValue)
            delete  m_lpByteValue;
    }
    virtual BOOL Init() { return (m_dwValueSize!=NULL && m_lpByteValue!=NULL); };
    virtual BOOL GetRegValue(PVOID pvData, LPDWORD pdwSize, LPDWORD pdwType)  {
        if (m_lpByteValue &&  pdwSize ) {
            if (pvData)
                memcpy(pvData,m_lpByteValue, min (*pdwSize, m_dwValueSize));
            *pdwSize = m_dwValueSize;
            if (pdwType)
                *pdwType = m_dwValueType;
            return TRUE;
        }
        else
            return FALSE;
    }
protected:
    virtual BOOL UpdateRegValue()  {
        if (m_lpRegName) {
            if (m_lpByteValue == NULL) {
                m_dwValueSize = 0;
                LONG status =  RegQueryValueEx(m_hParentKey,m_lpRegName,NULL, &m_dwValueType,NULL,&m_dwValueSize);
                if ( status == ERROR_SUCCESS || status == ERROR_MORE_DATA )
                    m_lpByteValue = new BYTE [ m_dwValueSize ];
            }
            if (m_lpByteValue != NULL) {
                DWORD dwLen = m_dwValueSize;
                if (RegQueryValueEx(m_hParentKey,m_lpRegName,NULL, &m_dwValueType,m_lpByteValue,&dwLen) == ERROR_SUCCESS) {
                    return TRUE;
                }
                
            }
        }
        return FALSE;
    };
    DWORD  m_dwValueType;
    DWORD  m_dwValueSize;
    LPBYTE m_lpByteValue;
    
};

#define MAX_REG_VALUE_ENTRY 0x10
class RegKey : public RegKeyOrValue {
public:
    RegKey(HKEY hCurrentOpenKey, LPCTSTR lpKeyPath,DWORD dwFlag, RegKey * pNextRegKey) : RegKeyOrValue( hCurrentOpenKey,lpKeyPath,pNextRegKey) {
        m_pRegValueList=NULL;
        m_pRegKeyList=NULL;
        m_RegKey = NULL;
    }
    BOOL Init() {  
        RefreshAll();
        return TRUE; 
    };
    virtual ~RegKey() {
        DeleteAll();
        if (m_RegKey) {
            RegCloseKey( m_RegKey );
        }
    }
    //  Function 
    BOOL    RefreshAll (BOOL bDoNotCloseKey = FALSE) {
        BOOL fReturn = FALSE;
        if(m_RegKey == NULL && RegOpenKeyEx(m_hParentKey, m_lpRegName, 0, 0, &m_RegKey ) != ERROR_SUCCESS )  {
            m_RegKey = NULL;
        }
        if (m_RegKey) {
            // Backup old key first
            RegValue * m_pBackupRegValueList = m_pRegValueList;
            RegKey *   m_pBackupRegKeyList = m_pRegKeyList;
            m_pRegValueList = NULL;
            m_pRegKeyList = NULL;
            if ( EnumerateAllKey() && EnumerateAllValue()) { // Succeed to enumerate new key. Delete old key.
                while (m_pBackupRegValueList ) {
                    RegValue * pNextRegValue = (RegValue * )m_pBackupRegValueList->GetNextRegKeyOrValuePtr() ;
                    delete m_pBackupRegValueList;
                    m_pBackupRegValueList = pNextRegValue;
                }
                while (m_pBackupRegKeyList) {
                    RegKey * pNextKey = (RegKey *) m_pBackupRegKeyList->GetNextRegKeyOrValuePtr();
                    delete m_pBackupRegKeyList;
                    m_pBackupRegKeyList =  pNextKey ;
                }
                fReturn = TRUE;
            }
            else { // Fails, We need recover old key
                DeleteAll(); // Delete partial succeeded key and value.
                m_pRegValueList = m_pBackupRegValueList;
                m_pRegKeyList = m_pBackupRegKeyList ;
                fReturn = FALSE;
            }
            if (!bDoNotCloseKey) {
                RegCloseKey( m_RegKey );
                m_RegKey = NULL;
            }
        }
        return fReturn;
    }
    RegKey * RegFindKey(LPCTSTR lpKeyPath) {
        RegKey * pReturnKey =  m_pRegKeyList;
        while (pReturnKey) {
            if (_tcsicmp( pReturnKey->GetName(),lpKeyPath)==0)
                return pReturnKey;
            else
                pReturnKey=(RegKey * )pReturnKey->GetNextRegKeyOrValuePtr();
        }
        return NULL;
    }
    LONG RegFindValue(LPCTSTR lpValueName, PVOID pvData, LPDWORD pdwSize, LPDWORD pdwType) {
        RegValue * pCurValue =  m_pRegValueList;
        while (pCurValue) {
            if (_tcsicmp( pCurValue->GetName(),lpValueName)==0) {
                return (pCurValue->GetRegValue(pvData,pdwSize,pdwType)?ERROR_SUCCESS :ERROR_INVALID_PARAMETER);
            }
            else
                pCurValue=(RegValue *)pCurValue->GetNextRegKeyOrValuePtr();
        }
        return ERROR_NO_MORE_ITEMS;
        
    }
    LONG RegEnum(RegKeyOrValue * pList, DWORD dwReqIndex, PWSTR lpName, PDWORD lpcbName) {
        RegKeyOrValue * pReturnKey = pList;
        for (DWORD dwIndex = 0; pReturnKey!=NULL &&  dwIndex<dwReqIndex; dwIndex++) {
            pReturnKey =(RegKey * )pReturnKey->GetNextRegKeyOrValuePtr();
        }
        if (pReturnKey && pReturnKey->GetName()) {
            LONG lReturn = ERROR_SUCCESS;
            if ( lpcbName) {
                DWORD dwSize = _tcslen(pReturnKey->GetName()) + 1 ;
                if ( lpName) {
                    _tcsncpy(lpName,pReturnKey->GetName(),min(dwSize,*lpcbName));
                    if (dwSize>*lpcbName)
                        lReturn = ERROR_MORE_DATA;
                }
                *lpcbName =  dwSize;
            }
            return lReturn;
        }
        else
            return ERROR_NO_MORE_ITEMS;
    }
    LONG RegEnumKeyEx( DWORD dwReqIndex, PWSTR lpName, PDWORD lpcbName) {
        return RegEnum(m_pRegKeyList, dwReqIndex, lpName, lpcbName) ;
    }
    LONG RegEnumValue( DWORD dwReqIndex,LPWSTR lpValueName, LPDWORD lpcbValueName) {
        return RegEnum(m_pRegValueList, dwReqIndex, lpValueName, lpcbValueName) ;
    }

protected:
    RegKeyOrValue * SearchByName(RegKeyOrValue * pRegKeyOrValueList,LPCTSTR pszName) {
        while (pRegKeyOrValueList) {
            if (_tcsicmp(pRegKeyOrValueList->GetName(),pszName)== 0) // found it 
                break;
            else
                pRegKeyOrValueList = pRegKeyOrValueList->GetNextRegKeyOrValuePtr();
        }
        return pRegKeyOrValueList;
    }
    void DeleteAll() {
        while (m_pRegValueList) {
            RegValue * pNextRegValue = (RegValue * )m_pRegValueList->GetNextRegKeyOrValuePtr() ;
            delete m_pRegValueList;
            m_pRegValueList = pNextRegValue;
        }
        while (m_pRegKeyList) {
            RegKey * pNextKey = (RegKey *) m_pRegKeyList->GetNextRegKeyOrValuePtr();
            delete m_pRegKeyList;
            m_pRegKeyList =  pNextKey ;
        }
    }
    BOOL EnumerateAllKey() {
        BOOL fReturn = FALSE;
        if (m_RegKey) {
            TCHAR regName[MAX_PATH];
            fReturn = TRUE;
            for (DWORD RegEnum = 0;;RegEnum++) {
                DWORD ValLen = MAX_PATH;
                LONG status = ::RegEnumKeyEx(m_RegKey, RegEnum, regName, &ValLen, NULL, NULL, NULL, NULL);
                if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {
                    regName[MAX_PATH -1] =0;
                    RegKey * newKey = new RegKey (m_RegKey,regName,0,m_pRegKeyList);
                    if (newKey && newKey->Init())
                        m_pRegKeyList = newKey;
                    else{
                        if (newKey)
                            delete newKey;
                        fReturn = FALSE; // Not all the key has been enumerated.
                    }
                }
                else
                    break;
            }
        }
        return fReturn;
    }
    BOOL EnumerateAllValue() {
        BOOL fReturn = FALSE;
        if (m_RegKey) {
            fReturn = TRUE;
            TCHAR regName[MAX_PATH];
            for (DWORD RegEnum = 0;;RegEnum++) {
                DWORD ValLen = MAX_PATH;
                LONG status = ::RegEnumValue (m_RegKey, RegEnum, regName, &ValLen, NULL, NULL, NULL, NULL);
                if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {
                    regName[MAX_PATH -1] =0;
                    RegValue* newValue = new RegValue(m_RegKey,regName,m_pRegValueList);
                    if (newValue && newValue->Init())
                        m_pRegValueList = newValue;
                    else {
                        if (newValue) 
                            delete newValue;
                        fReturn = FALSE;
                    }
                }
                else
                    break;
            }
        }
        return fReturn;
    }
    HKEY            m_RegKey;
    RegValue *      m_pRegValueList;
    RegKey *        m_pRegKeyList;
    
};
class SystemNotifyRegKey :public RegKey,public CLockObject, public CMiniThread {
public:
    SystemNotifyRegKey(HKEY hCurrentOpenKey, LPCTSTR lpKeyPath) 
        :RegKey ( hCurrentOpenKey,lpKeyPath, KEY_NOTIFY,NULL )
        ,CMiniThread(0,TRUE ){
        m_hNotifyEvent = INVALID_HANDLE_VALUE;
        m_hTerminated = CreateEvent(NULL,TRUE,FALSE,NULL);
        if(m_RegKey == NULL && RegOpenKeyEx(m_hParentKey, m_lpRegName, 0, 0, &m_RegKey ) != ERROR_SUCCESS )  {
            m_RegKey = NULL;
        }
        if (m_RegKey!=NULL) {
            m_hNotifyEvent=CeFindFirstRegChange(m_RegKey,TRUE,REG_NOTIFY_CHANGE_LAST_SET);
        }
     }
    BOOL Init() { 
        if (m_RegKey!=NULL && m_hTerminated!=NULL && UpdateAllValue()) {
            m_fUpdated = TRUE;
            ThreadStart();
            return TRUE;
        } else
            return FALSE;
    };
    virtual ~SystemNotifyRegKey() {
        m_bTerminated = TRUE;
        ThreadStart();
        if (m_hTerminated)
            SetEvent(m_hTerminated);
        WaitThreadComplete( 1000);
        if (m_hTerminated)
            CloseHandle(m_hTerminated);
        if (m_hNotifyEvent != INVALID_HANDLE_VALUE) {
            CeFindCloseRegChange(m_hNotifyEvent) ;
            m_hNotifyEvent = INVALID_HANDLE_VALUE;
        }
    }
    void EnterLock() { 
        Lock(); 
        if ( m_fUpdated == FALSE)
            UpdateAllValue();
    };
    void LeaveLock() { Unlock(); };
    BOOL UpdateAllValue() {
        if (m_hNotifyEvent!=INVALID_HANDLE_VALUE) {
            Lock();
            CeFindNextRegChange( m_hNotifyEvent);
            m_fUpdated = FALSE;
            if (RefreshAll(TRUE))
                 m_fUpdated = TRUE;
            Unlock();
            return TRUE;
        } else
            return FALSE;
    }
    BOOL UpdateRegistryChange() {
        if (m_hNotifyEvent!=INVALID_HANDLE_VALUE && 
                WaitForSingleObject( m_hNotifyEvent, 0) == WAIT_OBJECT_0 ) { // Change has happen.
            return UpdateAllValue();
        }
        else
            return FALSE;
    }
private:
    HANDLE          m_hNotifyEvent;
    HANDLE          m_hTerminated;
    BOOL            m_fUpdated; // Indicate Last update suceeded or not.
    virtual DWORD       ThreadRun() {
        while ( !IsTerminated() && m_hNotifyEvent!=INVALID_HANDLE_VALUE && m_hTerminated!=NULL ) {
            HANDLE h[2] = {m_hNotifyEvent,m_hTerminated};
            if (WaitForMultipleObjects( 2, h, FALSE, INFINITE) == WAIT_OBJECT_0 && !IsTerminated()) { // Signaled
                UpdateAllValue();
            }
        }
        return 0;
    }

};

extern SystemNotifyRegKey * g_pSysRegistryAccess;

