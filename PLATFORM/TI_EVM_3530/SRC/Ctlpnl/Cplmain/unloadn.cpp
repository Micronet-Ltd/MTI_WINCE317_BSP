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
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Abstract: Code for processing uninstall-scripts
**/

#include "cplpch.h"
#include "ce_setup.h"
#include "unload.h"
#include "devmgmt.h"

#define ZONE_UNLOAD 1

HWND g_hwndList;
int  g_dwDelErrors;

HRESULT ProcessUnloadFile(FILE* fp, LPCTSTR pszInstDir, BOOL fCheck);
void DeleteInstallDirPath(LPCTSTR lpInstDir, int iCount);
BOOL RemoveSharedFile(LPTSTR pszPath);
BOOL UnloadCloseApps();
BOOL AddToSharedFileList(LPTSTR pszPath);
BOOL APIENTRY UnloadErrorDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL AddToErrorList(LPTSTR szBuf);

#define myleave(h)	{ hRes=h; goto myfinally; }

// DevMgmt PID support
typedef HRESULT (*pfnSetDMPkLst)(PDMSINVRECORD, DWORD);

extern "C" int APIENTRY UninstallApplication(HINSTANCE hinst, HWND hwndParent, LPCTSTR pszApp)
{
	HWND hwndDlg = NULL;
	HRESULT hRes=0, hRes1=0, hRes2=0;
	LPCTSTR pszInstDir;
	LPCTSTR pszISVDll;
	HINSTANCE hinstISVDll = 0;
    pfnUNINSTALL_INIT pfnUninstall_Init;
	TCHAR szUnloadFile[MAX_PATH];
	FILE* fpScript = NULL;
	
	DEBUGMSG(ZONE_UNLOAD, (L"Starting Unload(%x, %x, %s)\r\n", hinst, hwndParent, pszApp));
	
	// try to open registry keys
	CReg reg1(HKEY_LOCAL_MACHINE, RK_SOFTWAREAPPS);
	CReg reg2; reg2.Open(reg1, pszApp, KEY_ALL_ACCESS); // need to delete values too
	
	// find installed dir in registry
	if(!(pszInstDir = reg2.ValueSZ(RV_INSTALLDIR))) {
		myleave(ERR_REGISTRY_APP_NAME)
	}

	/*** This is fraught with problems
	CReg reg0(HKEY_LOCAL_MACHINE, RK_CONTROLPANEL);
	if(reg0.ValueDW(RV_UNLOADCLOSEAPPS)) {
	    // Close all apps if so configured. (On PsPC type devices, this should 
	    // be transparent to the user since apps are shut down on low memory too.
	    // We shut down apps so that we know the app being uninstalled isn't
    	// running. If a WM_CLOSE isn't good enough for an ISV, they can use 
    	// the Uninstall_Init() entry point. If this function fails, no big loss...
	    UnloadCloseApps();
	}
	****/

	// create a hidden dialog so we can report errors
	g_dwDelErrors = 0;
	if(!(hwndDlg = CreateDialog(hinst, MAKEINTRESOURCE(IDD_UNLOADERRORDLG), hwndParent, UnloadErrorDlgProc)))
	    myleave(ERR_INTERNAL);

    // If we find ISV defined Setup DLL name, load it and call its Uninstall_Init().
	if( (pszISVDll = reg2.ValueSZ(RV_ISVFILE)) && 
		(hinstISVDll = LoadLibrary(pszISVDll)) &&
        (pfnUninstall_Init = (pfnUNINSTALL_INIT) GetProcAddress(hinstISVDll, szUNINSTALL_INIT)) ) {
            if (codeUNINSTALL_INIT_CANCEL == pfnUninstall_Init(hwndParent, pszInstDir))
                myleave(0);
    }

	// Get path ('\Windows\AppName.Unload') to script file
	StringCchPrintf(szUnloadFile, MAX_PATH, UNLOAD_SCRIPT, pszApp);

	// check if script file is in use (if not in ROM)
    if(!IsROMFile(szUnloadFile) && IsFileInUse(szUnloadFile))
	{
		DEBUGMSG(ZONE_UNLOAD, (L"Unload: File '%s' in use\r\n", szUnloadFile));
		hRes1 = ERR_FILE_IN_USE;
		AddToErrorList(szUnloadFile);
		// dont exit right away--wait until after the first pass
	}

	// Try to open script file
	if(!(fpScript = _wfopen(szUnloadFile, L"rb")))
		myleave(ERR_UNLOAD_OPEN);

	// FIRST PASS: iterate through script file checking for syntax & 
	// that all the files we want to delete are not in use
	hRes2 = ProcessUnloadFile(fpScript, pszInstDir, TRUE);
	if(FAILED(hRes2))
		myleave(hRes2);
	if(FAILED(hRes1))
		myleave(hRes1);

	// rewind the file
	if (0 != fseek(fpScript, 0, SEEK_SET))
		myleave(E_FAIL);
	
	// SECOND PASS: iterate through script file excecuting the commands.
	// At this point we will go through to the end, regardless of errors
    // Show the error text for the second pass and hide the error text for the first pass.    
    ShowWindow(GetDlgItem(hwndDlg, IDC_ABORTTEXT), SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg, IDC_FAILTEXT), SW_SHOW);
	hRes = ProcessUnloadFile(fpScript, pszInstDir, FALSE);

	// done with the script file
	fclose(fpScript);

	// remove the script file
	if (!IsROMFile(szUnloadFile) && !DeleteFile(szUnloadFile)) 
	{
        DEBUGMSG(ZONE_UNLOAD, (L"Can't delete file '%s' \r\n", szUnloadFile));
		AddToErrorList(szUnloadFile);
		hRes = ERR_FILE_IN_USE;
	}

	// find InstlCreateDirCount dir in registry, remove dir tree & remove the reg value
	DeleteInstallDirPath(pszInstDir, reg2.ValueDW(RV_INSTALLDIRCOUNT));
	reg2.DeleteValue(RV_INSTALLDIRCOUNT);
	
	// remove InstlDir, InstlCreateDirCount & Installed flags from registry
	reg2.DeleteValue(RV_INSTALLDIRCOUNT);
	reg2.DeleteValue(RV_INSTALLDIR);
	reg2.DeleteValue(RV_INSTALLED);

#if 0   
        // remove PID for device management or anyone use /PID setting to install app
        if(reg2.ValueSZ(RV_PID, dmsRecord.wzPackageID, DEVICE_ID_LEN))
        {
            HINSTANCE hcoredll = NULL;
            pfnSetDMPkLst pfnSetPL = NULL;
            DWORD dwFlags;

            if (hcoredll = LoadLibrary(L"dmsdk.dll"))
            {
               if (pfnSetPL = (pfnSetDMPkLst)(GetProcAddress(hcoredll, L"CeSetDevMgmtPackageList")))
               {
                  dwFlags = PACKAGE_DELETE;
                  if (FAILED(pfnSetPL(&dmsRecord, dwFlags)))
                  {
                      DEBUGMSG(ZONE_UNLOAD, (L"Can't delete DevMgmt Package ID '%s' \r\n", dmsRecord.wzPackageID));
                  }
               }
            }

        }

        // remove PID from registry
        reg2.DeleteValue(RV_PID);
#endif
	
myfinally:
    DEBUGMSG(ZONE_UNLOAD, (L"Unload DONE: Exit code=%d\r\n", hRes));

    // Call Uninstall_Exit and unload the ISV's setup DLL.
    if (hinstISVDll)
    {
		pfnUNINSTALL_EXIT pfnUninstall_Exit;
        if(pfnUninstall_Exit = (pfnUNINSTALL_EXIT) GetProcAddress(hinstISVDll, szUNINSTALL_EXIT))
            pfnUninstall_Exit(hwndParent);
        FreeLibrary(hinstISVDll);
    }
	// if errors, display dialog box
	if(g_dwDelErrors)
	{
		DEBUGCHK(hRes == (HRESULT)ERR_FILE_IN_USE);	
		MSG msg;

		// Disable close button in the dialog.
        // EnableMenuItem(GetSystemMenu(hwndDlg, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);

		ShowWindow(hwndDlg, SW_SHOW);
	    while (IsWindow(hwndDlg) && GetMessage(&msg, NULL, 0, 0))
	    {
			if (!IsDialogMessage(hwndDlg, &msg))
			{
	            TranslateMessage( &msg );
    	        DispatchMessage( &msg );
			}		
		}
		hwndDlg = 0;
		hRes = 0;
	}
	
	// cleanup
   	if(fpScript) 
		fclose(fpScript);

	if(hwndDlg)
		DestroyWindow(hwndDlg);
        
	// Notify parent
	DEBUGCHK(hwndParent);
	if (hwndParent) 
	{
		if(PostMessage(hwndParent, WM_COMMAND, IDC_REMOVE_FLAG, hRes))
			DEBUGMSG(ZONE_UNLOAD, (L"Unload: PostMsg(%x) failed GLE=%d\r\n", hwndParent, GetLastError()));
	}
    return hRes;
}

void AddPath(LPTSTR pszOut, LPCTSTR pszFile, LPCTSTR pszDir)
{
	// we now have a filename. Make it a path (if reqd)
	if(*pszFile != L'\\') {
		lstrcpy(pszOut, pszDir);
		lstrcat(pszOut, pszFile);
	} else {
		lstrcpy(pszOut, pszFile);
	}
}

HKEY ScriptGetKey(LPTSTR lpszPtr)
{
	switch (*lpszPtr) {
		case 'l' :
			return HKEY_LOCAL_MACHINE;
		case 'c' :
			return HKEY_CURRENT_USER;
		case 'u' :
			return HKEY_USERS;          
		case 'r' :
			return HKEY_CLASSES_ROOT;  
		}
	return NULL;
}

HRESULT ProcessUnloadFile(FILE* fp, LPCTSTR pszInstDir, BOOL fCheckOnly)
{
	HRESULT hRes = 0;
	TCHAR szBuf[MAX_PATH];
	TCHAR szFileDir[MAX_PATH];
	TCHAR szPath[MAX_PATH];
	HKEY hKey;
	
	// Process the script file
	// In pass-1 (fCheckOnly==TRUE) we check for syntax errors and for each file
	// 		we want to delete we check if can write to this file (means it is not
	// 		being used currently) if cannot write to it--add to error list
	//
	// In pass-2 process each command
	// for each of the files needed to remove
	//   if it is REMOVE_SHARED command
	//		go to the registry 'SharedFile' section
	// 		if the value for it is >0  just decreament ref count
	//      if equal 0 . delete file and remove key                 
	//   if it is REMOVE_FILE commmand
	// 		delete file
	//   if it is REMOVE_DIR commmand
	// 		delete directory
	//   if it is REMOVE_REG_VALUE commmand
	// 		delete value
	//   if it is REMOVE_REG_KEY commmand
	// 		delete key

	int idCommand = -1;
	int iFileLine = -1;
	int iSectionLine = -1;
	
	while(fgetws(szBuf, MAX_PATH, fp))
	{
		// strip trailing \r\n\t and space
		int iLen;
		for(iLen=lstrlen(szBuf); iLen>0; iLen--)
		{
			if(szBuf[iLen-1]==L'\t' || szBuf[iLen-1]==L'\r' || szBuf[iLen-1]==L'\n' || szBuf[iLen-1]==L' ')
				szBuf[iLen-1] = 0;
			else 
				break;
		}
			
		iFileLine++;
        DEBUGMSG(ZONE_UNLOAD, (L"Unload: ReadLine(%d): '%s'\r\n", iFileLine, szBuf));		

        // skip blank lines & comments
        if(iLen==0 || TEXT('\0') == szBuf[0] || szBuf[0] == '/')
            continue;					   
		iSectionLine++;
		
		if((szBuf[0] != L' ') && (szBuf[0] != L'\t'))
		{
			// We have a new section
			idCommand = szBuf[0]  - '0';           
			if (idCommand < 0 || idCommand >= REMOVE_LAST_CMD)
				myleave(ERR_UNLOAD_COMMAND);

			// otherwise start a new section
			DEBUGMSG(ZONE_UNLOAD, (L"Unload: Got section: Command id: %d....\r\n", idCommand));
			iSectionLine = 0;
		}	
		else 
		{
			// We have a file line within a section
			if (idCommand < 0)	 {
		        DEBUGMSG(ZONE_UNLOAD, (L"Illegal command '%s' line=%d \r\n", szBuf, iFileLine));
				myleave(ERR_UNLOAD_COMMAND);		// no command found in unload script file 
			}
			// skip leading spaces
			LPTSTR pszNext;
			for(pszNext=szBuf; (*pszNext==L' ') || (*pszNext==L'\t'); pszNext++)
				;

			switch 	(idCommand) 
			{
			default:
				break;
				
			case EXECUTE_FILE:
			{
				LPTSTR pszArgs;
				// remove exe argument if any
				if(pszArgs = wcschr(pszNext+1, ' ')) {
					*pszArgs++ = 0;
				}
				// we now have a EXE filename. Make it a path (if reqd) based on the InstallDir
				AddPath(szPath, pszNext, pszInstDir);

				// if file does not exist, just ignore
				if(!FileExists(szPath)) {
			        DEBUGMSG(ZONE_UNLOAD, (L"Unload: EXE File %s does not exist\r\n", szPath));
			        break;
				}
				if(fCheckOnly)
					break;
					
			//// End checks ////

				if(!MyCreateProcess(szPath, pszArgs))
					DEBUGMSG(ZONE_UNLOAD, (L"Unload: CreateProcess failed GLE=%d\r\n", GetLastError()));
				break;
			}
				
			case EXECUTE_DLL_FUNCTION:
			{
				HINSTANCE hinst;
               	DLLFUNC pfn = 0;
				LPTSTR pszFunc = wcschr(pszNext, TEXT(';'));
				if(!pszFunc)
					myleave(ERR_UNLOAD_COMMAND);
				if(fCheckOnly)
					break;
					
			//// End checks ////
			
				*pszFunc++ = 0;
				hinst = LoadLibrary(pszNext);
               	if(hinst) 
               		pfn = (DLLFUNC)GetProcAddress(hinst, pszFunc);
				if(pfn)
					pfn();
				else
			        DEBUGMSG(ZONE_UNLOAD, (L"Cant load/call unload DLL(%s) Func(%s)\r\n", pszNext, pszFunc));
			    if(hinst)
			    	FreeLibrary(hinst);
                break;
			}
			
			case REMOVE_DIR:
			{
				if(fCheckOnly)
					break;
					
			//// End checks ////
			
				AddPath(szPath, pszNext, pszInstDir);
				// if it doesnt exist or is in ROM ignore
				if(!FileExists(szPath) || IsROMFile(szPath))
					break;
				if(!RemoveDirectory(szPath)) {
			        DEBUGMSG(ZONE_UNLOAD, (L"Can't delete directory '%s' GetLastError=%d\r\n", szPath, GetLastError()));
			        hRes = ERR_FILE_IN_USE;
					AddToErrorList(szPath);
				}
				break;
			}
			
			case REMOVE_REG_KEY:
			{
				if(!(hKey = ScriptGetKey(pszNext)))
					myleave(ERR_REG_COMMAND);
				if(fCheckOnly)
					break;
					
			//// End checks ////
			
				pszNext += 2;		// skip over the the key name and one space
				if(!RegDeleteKey(hKey, pszNext)) {
					DEBUGMSG(ZONE_UNLOAD, (L"Unload: Error removing Reg Key %s\r\n", pszNext));
				}
				break;
			}
				
			case REMOVE_REG_VALUE:
            case REMOVE_REG_VALUE_EX:
            {
				if(!(hKey = ScriptGetKey(pszNext)))
					myleave(ERR_REG_COMMAND);
					
				LPTSTR pszValName;
				LPTSTR pszKey = pszNext+2; // skip over the the key name and one space
				
				// REMOVE_REG_VALUE_EX uses ';' as delimiter to allow names with spaces
				if (idCommand == REMOVE_REG_VALUE) {
					pszValName = wcschr(pszKey, TEXT(' '));
				} else {
					pszValName = wcschr(pszKey, TEXT(';'));
				}
				if(!pszValName)
					myleave(ERR_REG_COMMAND);
				if(fCheckOnly)
					break;
					
			//// End checks ////
			
				*pszValName++ = 0; // terminate keyname & skip to valname
				CReg reg; reg.Open(hKey, pszKey, KEY_ALL_ACCESS); // need write access
				if(!reg.DeleteValue(pszValName)) {
					DEBUGMSG(ZONE_UNLOAD, (L"Unload: Error removing Reg Value %s\\%s \r\n", pszKey, pszValName));
				}
				break;
			}
				
			case REMOVE_FILE_SHARED:
			case REMOVE_FILE:
				if (iSectionLine == 1) 
				{
					// make copy of path, appending \ if reqd
					lstrcpy(szFileDir, pszNext);
					AppendSlash(szFileDir);
					DEBUGMSG(ZONE_UNLOAD, (L"Unload: Files directory is:'%s'\r\n", szFileDir));
					break;
				}
				// we now have a filename. Make it a path (if reqd)
				AddPath(szPath, pszNext, szFileDir);

				// if file does not exist, just ignore
				if(!FileExists(szPath) || IsROMFile(szPath))
				{
			        DEBUGMSG(ZONE_UNLOAD, (L"Unload: File %s does not exist or is ROM\r\n", szPath));
			        break;
				}

				if(fCheckOnly)
				{
					if(IsFileInUse(szPath))
					{
						DEBUGMSG(ZONE_UNLOAD, (L"Unload: File '%s' in use\r\n", szPath));
				        hRes = ERR_FILE_IN_USE;
						AddToErrorList(pszNext);
					}
				}
				else if(idCommand == REMOVE_FILE_SHARED)
				{
					// it is safe to decr shared-file counters because when we're in pass-2 
					// we're past the point of no return. The unload will complete & the script
					// will be deleted, regardless.
					if(!RemoveSharedFile(szPath))
					{
						DEBUGMSG(ZONE_UNLOAD, (L"Unload: Can't delete SHARED file '%s' GetLastError=%d\r\n", szPath, GetLastError()));
						hRes = ERR_FILE_IN_USE;
						AddToErrorList(szPath);
					}
				}
				else
				{
					if(!DeleteFile(szPath)) 
					{
						DEBUGMSG(ZONE_UNLOAD, (L"Unload: Can't delete file '%s' GetLastError=%d\r\n", szPath, GetLastError()));
						hRes = ERR_FILE_IN_USE;
						AddToErrorList(szPath);
					}
				}
				break;
		  	}	
		}	
	}
	if((iFileLine < 0) || ferror(fp))
		myleave(ERR_UNLOAD_OPEN);
	return hRes;
	
myfinally:
	ASSERT(fCheckOnly==TRUE);
	DEBUGMSG(ZONE_UNLOAD, (L"Unload: ProcessLoadFile ERROR %d at line=%d: %s \r\n", hRes, iFileLine, szBuf));
	return hRes;
}

BOOL RemoveSharedFile(LPTSTR pszPath)
{
	CReg reg; reg.Open(HKEY_LOCAL_MACHINE, RK_APPSSHARED, KEY_ALL_ACCESS); // need write access

	DWORD dwRefCount = reg.ValueDW(pszPath);

	// if we got a valid count from the registry of more than 1
	// then decrement and update registry & dont delete
	if(dwRefCount > 1) {
		dwRefCount--;
		reg.SetDW(pszPath, dwRefCount);
		return TRUE;
	}
	reg.DeleteValue(pszPath);
			
	// if file does not exist, or is in ROM just ignore
	if(!FileExists(pszPath) || IsROMFile(pszPath))
		return TRUE;
		
	return DeleteFile(pszPath);
}

void DeleteInstallDirPath(LPCTSTR pszInstDir, int iCount)
{
	TCHAR szPath[MAX_PATH];
	LPTSTR pszTemp;
	
	if(!iCount || !pszInstDir) return;
	
	// remove installed directory and parent directories, if empty
	if (SUCCEEDED(StringCbCopy(szPath, sizeof(szPath), pszInstDir)))
	{
		for(int j = 0; j < iCount; j++)
		{
			DEBUGMSG(ZONE_UNLOAD, (L"Deleting directory '%s' \r\n", szPath));
			if(!RemoveDirectory(szPath))
				DEBUGMSG(ZONE_UNLOAD, (L"Can't delete directory '%s' GetLastError=%d\r\n", szPath, GetLastError()));

			if(!(pszTemp = wcsrchr(szPath, L'\\')))
				break;
			else
				*pszTemp = 0;
		}
	}
}

BOOL APIENTRY UnloadErrorDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
    case WM_INITDIALOG:
    	g_hwndList = DI(IDC_PROG_LIST);
		break;
    case WM_COMMAND:      
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hDlg);
			break;
		}
	case WM_CLOSE:      
		DestroyWindow(hDlg);
		break;
	}
    return (FALSE);
}

BOOL AddToErrorList(LPTSTR lszBuf)
{
	ListBox_AddString(g_hwndList, lszBuf);
	g_dwDelErrors++;         // how may files delete errors
	return TRUE;
}


