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
// Registry handling code for dialing entries.
//
// Substantial mods to registry storage scheme 
// Note that I refer to the registry data as SzSz, even though
// I may have zero length strings.  So when parsing, always pay
// attention to all data returned by registry, don't stop at the
// first double NULL you come across.

#include <windows.h>
#include <memory.h>
//#include <malloc.h>
#include <tchar.h>
#include "resource.h"
//#include "globals.h"
#include "loc.h"
#include "dial.h"																			


// Globals declared in transdlg.c
extern int nNumLocations, nCurrLocation;
extern HINSTANCE v_hInst;

#ifdef TARGET_NT
static const TCHAR  szRegistryKey1[]=TEXT("PegControlPanel\\Dial");
static const TCHAR  szRegistryKey2[]=TEXT("PegControlPanel\\Dial\\Locations");
#else
static const TCHAR  szRegistryKey1[]=TEXT("ControlPanel\\Dial");
static const TCHAR  szRegistryKey2[]=TEXT("ControlPanel\\Dial\\Locations");
#endif
// static const TCHAR  szLocations[] = TEXT("Locs");
static const TCHAR  szCurrentLoc[] = TEXT("CurrentLoc");
static const TCHAR  szHighLoc[] = TEXT("HighLocID");
static const TCHAR  szNoCW[] = TEXT("noCW");
static const TCHAR  szDialDigits[] = TEXT("Digits");

// Default location in case one isn't found in registry
#define	LEN_LOCATION	50
static TCHAR  szLOCATION[LEN_LOCATION] =  L"\0";


DWORD dwCurrLocId, dwHighLocId;

// --------------------------------------------
// parse location from string.
// --------------------------------------------
void ParseLocation(LPTSTR lpLocIDPtr, LPTSTR lpLocPtr, LOCEDIT *Location)
{
    LPTSTR  lpstr;
	int	i;
    
//    DPF2("Parsing key x%X, Location x%X\r\n", lpLocPtr, Location);

	if ((! Location) || (!lpLocPtr))
		return;

    // First, the key name is the location ID.  Convert from string.
    Location->dwPermLocationID = (DWORD)_wtoi(lpLocIDPtr);
//    DPF2("dwPermLocationID %s, %d\r\n", lpLocIDPtr, Location->dwPermLocationID);
    
    // And then lpLocPtr is SzSz string containing all other fields
    lpstr = lpLocPtr;
//    DPF1("szName %s\r\n", lpstr);
    lstrcpy(Location->szName, lpstr);

    lpstr = wcschr(lpstr, 0) + 1;
    lstrcpy(Location->szLocPrefix, lpstr);
//    DPF1("szLocPrefix %s\r\n", lpstr);
	
    lpstr = wcschr(lpstr, 0) + 1;
    lstrcpy(Location->szLDPrefix, lpstr);
//    DPF1("szLDPrefix %s\r\n", lpstr);
	
    lpstr = wcschr(lpstr, 0) + 1;
    lstrcpy(Location->szIntlPrefix, lpstr);
//    DPF1("szIntlPrefix %s\r\n", lpstr);
	
    lpstr = wcschr(lpstr, 0) + 1;
    lstrcpy(Location->szAreaCode, lpstr);
//    DPF1("szAreaCode %s\r\n", lpstr);

    lpstr = wcschr(lpstr, 0) + 1;
    lstrcpy(Location->szCancelCW, lpstr);
//    DPF1("szCancelCW %s\r\n", lpstr);

    // LoadString doesn't handle it if I have double NULL anywhere in a string, so
    // we make noCW a magic string which gets replaced by NULL.  The registry and
    // all the rest of the code is OK with the NULLs.
    if (!lstrcmp(szNoCW, Location->szCancelCW)) {
    	*Location->szCancelCW = 0;
	}

	for (i=0; Location->szCancelCW[i] != TEXT('\0'); i++) {
		if (Location->szCancelCW[i] != TEXT(' ')) {
			break;
		}
	}
	if (Location->szCancelCW[i] == TEXT('\0')) {
		Location->szCancelCW[0] = TEXT('\0');
	}

    lpstr = wcschr(lpstr, 0) + 1;
    lstrcpy(Location->szCntryRgnCode, lpstr);
//    DPF1("szCntryRgnCode %s\r\n", lpstr);

    lpstr = wcschr(lpstr, '\0') + 1;		 // skip '\0'
    Location->dwOptions = (DWORD)_wtoi(lpstr);
//    DPF2("szOptions %s, %d\r\n", lpstr, Location->dwOptions);
    
}


/*--------------------------------------------------------------------------*\

    Function:   RegReadLocations
    
    Purpose:    
      Reads in a entry from the reg file.  
\*--------------------------------------------------------------------------*/
UINT RegReadLocations(LPHANDLE   lphMemBlock)
{ 
DWORD	dwSize, dwType, dwDisp;
long	hRes =  -1;
LPTSTR  lpszValue=NULL;
int 	i, j;
HKEY	hKey1=NULL, hKey2=NULL; 
HANDLE	lpMem;
LOCATION *ptr;
#ifdef DEBUG_TIMING
int		t1,t2;		//  for performance
#endif
DWORD   dwLocLen, dwDataLen;

#define SZ_DUMMY	10
TCHAR    achLocName[SZ_DUMMY] = L""; /* buffer for location key name   */ 
TCHAR    achClass[SZ_DUMMY] = L""; /* buffer for class name   */ 
DWORD    cchClassName = SZ_DUMMY;  /* length of class string  */ 
DWORD    cSubKeys=0;               /* number of subkeys       */ 
DWORD    cbMaxSubKey=0;            /* longest subkey size     */ 
DWORD    cchMaxClass=0;            /* longest class string    */ 
DWORD    cValues=0;                /* number of values for key    */ 
DWORD    cchMaxValue=0;            /* longest value name          */ 
DWORD    cbMaxValueData=0;         /* longest value data          */ 


   //-------------------------------
//	DPF2("RegReadLocations:  key='%s' key='%s'\r\n", szRegistryKey1, szRegistryKey2);

    // First key contains current location and max permProvider ID
//	DBG_TimeStart();
#ifdef TARGET_NT
	hRes = RegCreateKeyExW(HKEY_CURRENT_USER, szRegistryKey1, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey1, &dwDisp);
#else
	hRes = RegCreateKeyEx(HKEY_CURRENT_USER, szRegistryKey1, 0, NULL, 0, 0, NULL, &hKey1, &dwDisp);
#endif					 
//	DBG_TimeEnd((TEXT("RegCreateKeyEx  '%s'. time=%d \r\n"),
//                  szRegistryKey1, t2-t1) );

	if ( (hRes != ERROR_SUCCESS) || (REG_CREATED_NEW_KEY == dwDisp) ) {
        dwCurrLocId = 0;
        dwHighLocId = 1;
//		DPF3("Error open '%s' registry key. Error code=%d, Disp x%X\r\n",szRegistryKey1, hRes, dwDisp);
        if (hKey1)
            RegCloseKey(hKey1);
        hKey1 = NULL;
    } else {
        // Read the Curr and High Location IDs stored under this key
        dwSize = sizeof(dwCurrLocId);
        hRes = RegQueryValueEx(hKey1,
                               szCurrentLoc,
                               NULL,
                               &dwType,
                               (PUCHAR)&dwCurrLocId,
                               &dwSize);
        if (ERROR_SUCCESS != hRes)
            dwCurrLocId = 0;

        dwSize = sizeof(dwHighLocId);
        hRes = RegQueryValueEx(hKey1,
                               szHighLoc,
                               NULL,
                               &dwType,
                               (PUCHAR)&dwHighLocId,
                               &dwSize);
        if (ERROR_SUCCESS != hRes)
            dwHighLocId = 0;  // Gets incremented below if needed.
    }
    
    // Second key contains list of SZSZ strings, each representing a locale
//	DBG_TimeStart();
#ifdef TARGET_NT
	hRes = RegCreateKeyExW(HKEY_CURRENT_USER, szRegistryKey2, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey2, &dwDisp);
#else
	hRes = RegCreateKeyEx(HKEY_CURRENT_USER, szRegistryKey2, 0, NULL, 0, 0, NULL, &hKey2, &dwDisp);
#endif					 
//	DBG_TimeEnd((TEXT("RegCreateKeyEx  '%s'. time=%d \r\n"),
//                  szRegistryKey2, t2-t1) );

	if ((hRes != ERROR_SUCCESS) || (REG_CREATED_NEW_KEY == dwDisp)) {
//		DPF3("Error open '%s' registry key. Error code=%d, disp 0x%X\r\n",szRegistryKey2, hRes, dwDisp);
		}

    // See how many keys exist and how large they are
    hRes = RegQueryInfoKey( hKey2, NULL, NULL, NULL,
                                 &cSubKeys,
                                 &cbMaxSubKey,
                                 &cchMaxClass,
                                 &cValues,
                                 &cchMaxValue,
                                 &cbMaxValueData,
                                 NULL, NULL );

	if (!hKey2 || !cchMaxValue)	{
        // No key exists, load a default
		i = LoadString(v_hInst, INTLMSG_RES_LOCATION, szLOCATION, LEN_LOCATION);
        cbMaxValueData = lstrlen(szLOCATION);
        cValues = 1;
        dwCurrLocId = 0;
        dwHighLocId = 1;
    }

    // allocate space to store the locations
    MyHeapChk();
    if (*lphMemBlock == NULL)  {
        *lphMemBlock = lpMem = LocalAlloc(LPTR, cValues * sizeof(LOCATION));
    }
    else  {
        lpMem = LocalReAlloc(*lphMemBlock, cValues * sizeof(LOCATION),  LMEM_MOVEABLE | LMEM_ZEROINIT);
        *lphMemBlock = lpMem;
    }
    MyHeapChk();
    ptr = (LOCATION *) *lphMemBlock;
    if (ptr == NULL) 
        goto exit;

//    DPF2("Allocated space for %d locations at %X\r\n", cValues, ptr);

    if( hKey2 && cchMaxValue )
    {
        // Allocate a temp string to store a single location from registry
        cbMaxValueData += sizeof(TCHAR);  // Leave room for final NULL
//        DPF1("Allocating %d bytes for single location\r\n", cbMaxValueData);
        lpszValue = VirtualAlloc(NULL, cbMaxValueData * sizeof(TCHAR), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
//        DPF2("Allocating space for %d locations at %X\r\n", cValues, *lphMemBlock);

        // Enumerate through the available locations.
        for( i=0; i<(int)cValues; i++ )
        {
            dwLocLen = sizeof(achLocName)/sizeof(TCHAR);
            dwDataLen = cbMaxValueData;
//            DPF1("Enumerating key index %d\r\n", i);
            hRes = RegEnumValue(hKey2,
                                i,
                                achLocName,
                                &dwLocLen,
                                NULL,
                                &dwType,
                                (LPBYTE)lpszValue,
                                &dwDataLen);

            if( ERROR_SUCCESS != hRes )
            {
//                DPF1("Error x%X enumerating key\r\n", hRes);
                break;
            }

            // Copy data from SZSZ string to struct
//            RETAILMSG(1, (TEXT("Parsing key index %d (x%X, %d)\r\n"),
//                          i, lpszValue, dwDataLen) );
            ParseLocation(achLocName, lpszValue, &ptr[i].LocEdit);

            for( j=0; j<(int)(dwDataLen/sizeof(TCHAR)); j++ )
//            RETAILMSG(1, (TEXT("%4.4X "),
//                          lpszValue[j] ) );
//            RETAILMSG(1, (TEXT("\r\n")));
                
            // Check to see if this location is current location
            if( ptr[i].LocEdit.dwPermLocationID == dwCurrLocId )
            {
                nCurrLocation = i;
//                DPF1("Found current location at index %d\r\n", nCurrLocation);
            }

            // Safety check to make sure high location id is always OK
            if( dwHighLocId <= ptr[i].LocEdit.dwPermLocationID )
                dwHighLocId = ptr[i].LocEdit.dwPermLocationID + 1;
        }
        nNumLocations = i;
        hRes = ERROR_SUCCESS;
//        DPF2("Done parsing Curr = %d, High = %d\r\n", dwCurrLocId, dwHighLocId );
    }
    else
    {
        // No strings found in registry, use default.
        // Copy data from SZSZ string to struct
//        RETAILMSG(1, (TEXT("Parsing default key\r\n")) );
        ParseLocation( TEXT("0"), szLOCATION, &ptr[0].LocEdit );
        ptr[0].LocEdit.dwStatus = CPL_ENTRY_ADDED;
        nNumLocations = 1;
        nCurrLocation = 0;
        hRes = ERROR_SUCCESS;
//        RETAILMSG(1, (TEXT("Done Parsing default key\r\n")) );
    }
    
exit:
	if (lpszValue)				  
		VirtualFree(lpszValue,0, MEM_RELEASE);
    
    if (hKey1)
	    RegCloseKey(hKey1);
    if (hKey2)
	    RegCloseKey(hKey2);
    return hRes;
}  // end RegReadLocations

// create a location string from the current location
int GetLocationString(int i, LPTSTR lpszName, LPTSTR lpszText)
{
LOCEDIT *lpLocation;
int     cTextLen = 0;

	GetLocationPtr(i, &lpLocation);
    if (lpLocation == NULL) 
        return 0;
	wsprintf(lpszName, L"%d", lpLocation->dwPermLocationID & 0x0000FFFF);   // Value name is location ID

    if( lpszText )
    {
//        DPF("GetLocationString - Building location\r\n");
        cTextLen = wsprintf(lpszText, L"%s", lpLocation->szName) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->szName, cTextLen);
        cTextLen += wsprintf(lpszText+cTextLen, L"%s", lpLocation->szLocPrefix) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->szLocPrefix, cTextLen);
        cTextLen += wsprintf(lpszText+cTextLen, L"%s", lpLocation->szLDPrefix) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->szLDPrefix, cTextLen);
        cTextLen += wsprintf(lpszText+cTextLen, L"%s", lpLocation->szIntlPrefix) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->szIntlPrefix, cTextLen);
        cTextLen += wsprintf(lpszText+cTextLen, L"%s", lpLocation->szAreaCode) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->szAreaCode, cTextLen);
        cTextLen += wsprintf(lpszText+cTextLen, L"%s", lpLocation->szCancelCW) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->szCancelCW, cTextLen);
        cTextLen += wsprintf(lpszText+cTextLen, L"%s", lpLocation->szCntryRgnCode) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->szCntryRgnCode, cTextLen);
        cTextLen += wsprintf(lpszText+cTextLen, L"%d", lpLocation->dwOptions) + 1;
//        DPF2("Added %s (total len %d)\r\n", lpLocation->dwOptions, cTextLen);

        cTextLen += sizeof(TCHAR);  // Add room for final null
    }
    
    return cTextLen;
}


/*--------------------------------------------------------------------------*\

    Function:   RegSaveDialSettings
    
    Purpose:    
      Save dialing settings to registry.  
\*--------------------------------------------------------------------------*/
HRESULT RegSaveDialSettings(void)
{
#define SZ_DUMMY	10
TCHAR   achLocName[SZ_DUMMY] = L""; /* buffer for location key name   */ 
HKEY	hKey1=NULL, hKey2=NULL;
HRESULT hRes;
LOCEDIT *lpLocation;
int		i;
LPTSTR  lpszText = NULL;
int  size;
DWORD   dwNewHighLocId = dwHighLocId;
#ifdef DEBUG_TIMING
int  t1,t2;		//  for performance
#endif

	//loop to see if anything to save	
//	DPF("In RegSaveDialSettings \r\n");

	// Keys were created in RegReadLocations if they didn't already exist.  Open them.
//	DBG_TimeStart();
	hRes = RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey1, 0, KEY_ALL_ACCESS, &hKey1);
//	DBG_TimeEnd((TEXT("Opened key '%s'. time=%d \r\n"),
//                  szRegistryKey1, t2-t1) );

	if (hRes != ERROR_SUCCESS) {
//		RETAILMSG(1, (L"Error opening %s registry key.\r\n", szRegistryKey1));
		goto  exit;
		}

//	DBG_TimeStart();
	hRes = RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey2, 0, KEY_ALL_ACCESS, &hKey2);
//	DBG_TimeEnd((TEXT("Opened key '%s'. time=%d \r\n"),
//                  szRegistryKey2, t2-t1) );

	if (hRes != ERROR_SUCCESS) {
//		RETAILMSG(1, (L"Error opening %s registry key.\r\n", szRegistryKey2));
		goto  exit;
		}

    // Allocate a string for building SzSz registry values
	lpszText = VirtualAlloc(NULL, sizeof(LOCATION)+(20*sizeof(TCHAR)), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	if (! lpszText)	{
//		RETAILMSG(1, (L"Error Allocating string\r\n"));
		hRes = E_FAIL;
		goto  exit;
		}

    // Now step thru all locations, saving any modified entries
    for (i = 0; ; i++) {
        GetLocationPtr(i, &lpLocation);	
        if (! lpLocation)		// end of list
            break;
        else {
			if (lpLocation->dwStatus != CPL_ENTRY_NO_CHANGE) {
                // This entry has changed.  Do something with it.
//                DPF2("Location %d has changed, dwStatus x%X\r\n", i, lpLocation->dwStatus);
                    
                if (lpLocation->dwStatus == CPL_ENTRY_DELETED) {
                    // Get the value name, don't bother building string
                    (void) GetLocationString(i, achLocName, NULL);

                    // Remove it from the registry
//                    RETAILMSG(1, (TEXT("Deleting key index %d (%s)\r\n"),
//                                  i, achLocName ));
                    hRes = RegDeleteValue(hKey2, (LPTSTR)achLocName);
                }
                else {
                    // If its a new entry, get a location ID for it.
                    if (lpLocation->dwStatus == CPL_ENTRY_ADDED)
                        lpLocation->dwPermLocationID = dwNewHighLocId++;

                    // OK, create an SzSz string and get value name
                    size = GetLocationString(i, achLocName, lpszText);

//                    RETAILMSG(1, (TEXT("Saving key index %d \"%s\" (x%X, %d)\r\n"),
//                                  i, achLocName, lpszText, size ));
                    hRes = RegSetValueEx(hKey2, (LPTSTR)achLocName, 0, REG_MULTI_SZ, (LPBYTE)lpszText, size * sizeof(TCHAR));
                }
            }
                
        }
    }

    // Save current location ID.
    GetLocationPtr(nCurrLocation, &lpLocation);
    if (!lpLocation) 
        goto exit;
    
//    DPF2("Current Location %d, previous %d\r\n", lpLocation->dwPermLocationID, dwCurrLocId );  
    hRes = RegSetValueEx(hKey1, (LPTSTR)szCurrentLoc, 0, REG_DWORD, (LPBYTE)&lpLocation->dwPermLocationID, sizeof(DWORD));

    // Save current High Location
//    DPF2("Current HighLocation %d, previous %d\r\n", dwNewHighLocId, dwHighLocId );  
    hRes = RegSetValueEx(hKey1, (LPTSTR)szHighLoc, 0, REG_DWORD, (LPBYTE)&dwNewHighLocId, sizeof(DWORD));    

exit:
	if (lpszText)
		VirtualFree(lpszText, 0, MEM_RELEASE);

    if (hKey1) 
	    RegCloseKey(hKey1);					   
    if (hKey2) 
	    RegCloseKey(hKey2);					   
	return hRes;
}


//
// Function to read the dial digits table from the registry
//
// Returns the number of bytes needed or 0 if not found.
//
// NOTE: The caller is expected to call this function first with a NULL to determine the size.
//
DWORD
RegReadDialDigitsTable(
    TCHAR * lpNewDialDigitsTable
    )
{
    HKEY hk;
    DWORD status;
    DWORD valtype;
    DWORD vallen;
    DWORD ret;

    status = RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey1, 0, 0, &hk);
    if (status) {
        return 0;
    }

    ret = 0;
    vallen = 0;
    status = RegQueryValueEx(hk, szDialDigits, NULL, &valtype, NULL, &vallen);
    if (status == ERROR_SUCCESS) {
        ret = vallen;
        if (lpNewDialDigitsTable) {
            status = RegQueryValueEx(hk, szDialDigits, NULL, &valtype, (LPVOID)lpNewDialDigitsTable, &vallen);
        }
    }
    RegCloseKey(hk);
    return ret;
}


#ifdef TARGET_NT
int MyHeapChk(void)
{
int i;
  
	i = _heapchk();
	if (! HeapValidate(GetProcessHeap(), 0, NULL))
//		DPF1("HeapValidate(): Failed!!!  i=%d \r\n", i);
#if 0	
  	switch(i) {
		case _HEAPEMPTY:
//			DPF("_heapchk(): _HEAPEMPTY \r\n");
			return 1;
		case _HEAPBADBEGIN:
//			DPF("_heapchk(): _HEAPBADBEGIN \r\n");
			return 1;
		case _HEAPBADNODE:
//			DPF("_heapchk(): _HEAPBADNODE \r\n");
			return 1;
		case _HEAPOK:
			break;
		}
#endif
	return 0;
}
#endif


