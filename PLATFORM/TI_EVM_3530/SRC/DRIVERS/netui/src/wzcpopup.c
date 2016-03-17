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
#include "QuickConfigUI.h"


#define HKEY_ETHMAN_POPUP			_T("Drivers\\Builtin\\Ethman\\Popup")
#define REGVAL_ETM_POPUP_TIMEOUT	_T("Timeout")
#define REGVAL_ETM_POPUP			_T("Popup")

#define DEFAULT_POPUP_TIMEOUT       3600    // One hour


typedef struct _WZCCache
{
	CHAR szNetwork[MAX_SSID_LEN+1];
	DWORD dwInfraMode;
	struct _WZCCache* pNext;
	DWORD dwTimeStamp;
}
WZCCache;

WZCCache* g_pWZCCacheHead = NULL;


// Is the popup check box on ethman suppose to be enabled
DWORD GetDialogPopup(BOOL* pfPopup, DWORD* pdwTimeoutSecs)
{
	HKEY 	hKeyEtm = NULL;
	DWORD	dwDisp;
	DWORD	dwErr = ERROR_SUCCESS;
	
	// Set default values in case of failure
	if (pfPopup) *pfPopup = TRUE;
	if (pdwTimeoutSecs) *pdwTimeoutSecs = DEFAULT_POPUP_TIMEOUT;
		
	dwErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					HKEY_ETHMAN_POPUP,
					0,
					NULL,
					0,
					0,
					NULL,
					&hKeyEtm,
					&dwDisp);
	if(dwErr != ERROR_SUCCESS)
	{
		goto exit;
	}

	// If this was a new key then let's set up the values and 
	// return the default value
	if(dwDisp == REG_CREATED_NEW_KEY)
	{	
		// Set timeout value
		DWORD dwValue = DEFAULT_POPUP_TIMEOUT;
		dwErr = RegSetValueEx(hKeyEtm,
						REGVAL_ETM_POPUP_TIMEOUT,
						0,
						REG_DWORD,
						(PBYTE)&dwValue,
						sizeof(DWORD));
		if(dwErr != ERROR_SUCCESS)
		{
			goto exit;
		}		

		if(pdwTimeoutSecs)
		{
			*pdwTimeoutSecs = dwValue;
		}
		
		// Set popup value
		dwValue = TRUE;		
		dwErr = RegSetValueEx(hKeyEtm,
						REGVAL_ETM_POPUP,
						0,
						REG_DWORD,
						(PBYTE)&dwValue,
						sizeof(DWORD));
		if(dwErr != ERROR_SUCCESS)
		{
			goto exit;
		}
		
		if(pfPopup)
		{
			*pfPopup = (BOOL) dwValue;
		}
	}
	else
	{
		DWORD dwValue = 0;
		DWORD cdwValue = sizeof(DWORD);

		if(pdwTimeoutSecs)
		{
			// Get popup value
			dwErr = RegQueryValueEx(hKeyEtm,
							REGVAL_ETM_POPUP_TIMEOUT,
							0,
							NULL,
							(PBYTE)&dwValue,
							&cdwValue);
			if(dwErr != ERROR_SUCCESS)
			{
				goto exit;
			}

			*pdwTimeoutSecs = dwValue;
		}
		
		if(pfPopup)
		{
			cdwValue = sizeof(DWORD);

			// Get popup value
			dwErr = RegQueryValueEx(hKeyEtm,
							REGVAL_ETM_POPUP,
							0,
							NULL,
							(PBYTE)&dwValue,
							&cdwValue);
			if(dwErr != ERROR_SUCCESS)
			{
				goto exit;
			}

			*pfPopup = dwValue;
		}		
	}
			
exit:
	if (hKeyEtm)
	{
		RegCloseKey(hKeyEtm);
	}
	
	return dwErr;
}


// Set the popup status and timeout of ethman dialog
DWORD SetDialogPopup(BOOL fPopup, DWORD dwTimeoutSecs)
{
	DWORD			dwErr = ERROR_SUCCESS;
	HKEY 			hKeyEtm = NULL;
	DWORD			dwDisp;
	WZC_CONTEXT 	context = {0};
	DWORD			dwOutFlags = 0;
	
	dwErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					HKEY_ETHMAN_POPUP,
					0,
					NULL,
					0,
					0,
					NULL,
					&hKeyEtm,
					&dwDisp);
	if(dwErr != ERROR_SUCCESS)
	{
		goto exit;
	}

	if(dwTimeoutSecs > 0)
	{
		// Set timeout value
		dwErr = RegSetValueEx(hKeyEtm,
						REGVAL_ETM_POPUP_TIMEOUT,
						0,
						REG_DWORD,
						(PBYTE)&dwTimeoutSecs,
						sizeof(DWORD));
		if(dwErr != ERROR_SUCCESS)
		{
			goto exit;
		}
	}
	
	// Set popup value
	dwErr = RegSetValueEx(hKeyEtm,
					REGVAL_ETM_POPUP,
					0,
					REG_DWORD,
					(PBYTE)&fPopup,
					sizeof(DWORD));
	
exit:
	if (hKeyEtm)
	{
		RegCloseKey(hKeyEtm);
	}
	
	return dwErr;
}

// Add a network to the cache
DWORD WZCCacheAdd(CHAR* szNetwork, DWORD dwInfraMode)
{
	WZCCache* 	pCurr;
	BOOL		fExists 	= FALSE;
	DWORD		dwRetVal 	= ERROR_SUCCESS;

	WZCQCFG_LOCK;

	// Check if item is already in cache, and if so only update
	// the timestamp
	pCurr = g_pWZCCacheHead;
	while(pCurr != NULL)
	{
		if(!strcmp(szNetwork, pCurr->szNetwork) &&
			dwInfraMode == pCurr->dwInfraMode)
		{
			// Found a match, update timestamp
			fExists = TRUE;
			pCurr->dwTimeStamp = GetTickCount() / 1000;
			break;
		}

		pCurr = pCurr->pNext;
	}

	// If it does not exist yet, then create a new cache item
	if(!fExists)
	{
		WZCCache* pNode = NULL;

		pNode = LocalAlloc(0, sizeof(WZCCache));
		if(!pNode)
		{
			dwRetVal = E_FAIL;
			goto exit;
		}
		
		// Fill in new data
		strcpy(pNode->szNetwork, szNetwork);
		pNode->dwInfraMode = dwInfraMode;
		pNode->dwTimeStamp = GetTickCount() / 1000;

		// Insert node at head of list
		pNode->pNext = g_pWZCCacheHead;
		g_pWZCCacheHead = pNode;

		DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Adding <%hs> to Zero Config cache"), szNetwork));
	}
	
exit:
	WZCQCFG_UNLOCK;

	return dwRetVal;
}

// Check if a network is in the cache
BOOL IsInWZCCache(CHAR* szNetwork, DWORD dwInfraMode)
{
	BOOL 		fRetVal = FALSE;
	WZCCache* 	pCurr;

	WZCQCFG_LOCK;

	RefreshWZCCache();

	// Look for item in cache
	pCurr = g_pWZCCacheHead;
	while(pCurr != NULL)
	{
		if(!strcmp(szNetwork, pCurr->szNetwork) &&
			dwInfraMode == pCurr->dwInfraMode)
		{
			// Found a match
			fRetVal = TRUE;
			break;
		}

		pCurr = pCurr->pNext;
	}

	WZCQCFG_UNLOCK;

#ifdef DEBUG
if(fRetVal == FALSE) {
	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Network: <%hs> is not in Zero Config cache"), szNetwork));
}
#endif

	return fRetVal;
}

// Refresh the cache
void RefreshWZCCache()
{
	DWORD 		dwTimeout;
	WZCCache*	pCurr;
	WZCCache* 	pPrev;
	INT			iDiff;
	DWORD		dwCurrTime;

	WZCQCFG_LOCK;
	
	// If any times have expired then delete those nodes
	GetDialogPopup(NULL, &dwTimeout);

	dwCurrTime = GetTickCount() / 1000;

	// Check if item is already in cache, and if so only update
	// the timestamp
	pPrev = NULL;
	pCurr = g_pWZCCacheHead;
	while(pCurr != NULL)
	{
		iDiff = (INT) dwCurrTime - pCurr->dwTimeStamp;
		if(iDiff > (INT)dwTimeout || iDiff < 0)
		{
			if(pPrev == NULL)
			{
				// Delete node
				g_pWZCCacheHead = g_pWZCCacheHead->pNext;
				LocalFree(pCurr);

				// Reassign pointer for next iteration
				pCurr = g_pWZCCacheHead;
			}
			else
			{
				// Delete node
				pPrev->pNext = pCurr->pNext;
				LocalFree(pCurr);

				// Reassign pointer for next iteration
				pCurr = pPrev->pNext;
			}
		}
		else
		{
			pPrev = pCurr;
			pCurr = pCurr->pNext;
		}
	}

	WZCQCFG_UNLOCK;
}

// Clear the cache
void WZCCacheClear()
{
	while(g_pWZCCacheHead != NULL)
	{
		WZCCache* pNode;

		// Point to node to delete and advance head
		pNode = g_pWZCCacheHead;
		g_pWZCCacheHead = g_pWZCCacheHead->pNext;

		// Free node
		LocalFree(pNode);
	}
}

