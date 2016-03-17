//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.


Abstract:
	Windows CE Bluetooth application sample

**/
#include <windows.h>
#include <windev.h>
#include <stdio.h>

#include <winsock2.h>

#include <ras.h>
#include <notify.h>

#include <bt_api.h>
#include <bthapi.h>
#include <initguid.h>
#include <bt_sdp.h>


#include <svsutil.hxx>
#include <sdpcommon.h>
//#include "..\..\..\..\..\..\common\oak\drivers\bluetooth\sample\sdpcommon\sdpcommon.h"

#include "resource.h"

#ifdef __cplusplus
extern "C"  {
#endif

BOOL CeEventHasOccurred (LONG lWhichEvent, TCHAR *pwszEndOfCommandLine);

#ifdef __cplusplus
}

#endif /*__cplusplus*/
#define MAX_NAME	248
#define MAX_BA		20

#define DO_NOTHING	0
#define DO_INQUIRY	1
#define DO_NAMERES	2
#define DO_SDP		3
#define DO_STOP		4
#define DO_OPTIONS	5
#define DO_SYNC		6

#define DEFAULT_AS_PORT L"COM6:"
#define RAS_NAME_BLUETOOTH	L"`Bluetooth"

struct InquiryResult {
	InquiryResult	*pNext;

	BT_ADDR			b;					// Address of the item

	unsigned int	fHaveSDP  : 1;		// Do we have SDP record for it?

	unsigned char	channel;

	InquiryResult (void) {
		memset (this, 0, sizeof(*this));
	}
};

struct Global : public SVSSynch
{
	int					fState;
	HINSTANCE			hInst;

	HANDLE				hDevice;

	InquiryResult		*pDev;

	Global(void)
	{
		fState = DO_NOTHING;
		hInst  = NULL;
		pDev   = NULL;
		hDevice = NULL;
	}
};

static Global *g_pState = NULL;

int		g_fHaveSav = FALSE;

#define CHECK_STOP									\
{													\
		if(g_pState->fState == DO_STOP)				\
		{											\
			g_pState->fState = DO_NOTHING;			\
			g_pState->Unlock ();					\
													\
			SetWindowText(hWndButton, L"&Inquiry");	\
			return 0;								\
		}											\
}

//
//	SDP parsing is better illustrated in sdp sample
//
static HRESULT ServiceAndAttributeSearch(
    UCHAR *szResponse,             // in - response returned from SDP ServiceAttribute query
    DWORD cbResponse,            // in - length of response
    ISdpRecord ***pppSdpRecords, // out - array of pSdpRecords
    ULONG *pNumRecords           // out - number of elements in pSdpRecords
    )
{
    *pppSdpRecords = NULL;
    *pNumRecords = 0;

    ISdpStream *pIStream = NULL;

    HRESULT hres = CoCreateInstance(__uuidof(SdpStream),NULL,CLSCTX_INPROC_SERVER,
                            __uuidof(ISdpStream),(LPVOID *) &pIStream);

    if (FAILED(hres))
        return hres;  

	ULONG ulError;

    hres = pIStream->Validate (szResponse,cbResponse,&ulError);

    if (SUCCEEDED(hres)) {
        hres = pIStream->VerifySequenceOf(szResponse,cbResponse,
                                          SDP_TYPE_SEQUENCE,NULL,pNumRecords);

        if (SUCCEEDED(hres) && *pNumRecords > 0) {
            *pppSdpRecords = (ISdpRecord **) CoTaskMemAlloc(sizeof(ISdpRecord*) * (*pNumRecords));

            if (pppSdpRecords != NULL) {
                hres = pIStream->RetrieveRecords(szResponse,cbResponse,*pppSdpRecords,pNumRecords);

                if (!SUCCEEDED(hres)) {
                    CoTaskMemFree(*pppSdpRecords);
                    *pppSdpRecords = NULL;
                    *pNumRecords = 0;
                }
            }
            else {
                hres = E_OUTOFMEMORY;
            }
        }
    }

    if (pIStream != NULL) {
        pIStream->Release();
        pIStream = NULL;
    }

	return hres;
}

static int IsRfcommUuid(NodeData *pNode)  {
	if (pNode->type != SDP_TYPE_UUID)
		return FALSE;

	if (pNode->specificType == SDP_ST_UUID16)
		return (pNode->u.uuid16 == RFCOMM_PROTOCOL_UUID16);
	else if (pNode->specificType == SDP_ST_UUID32)
		return (pNode->u.uuid32 == RFCOMM_PROTOCOL_UUID16);
	else if (pNode->specificType == SDP_ST_UUID128)
		return (0 == memcmp(&RFCOMM_PROTOCOL_UUID,&pNode->u.uuid128,sizeof(GUID)));

	return FALSE;
}

static int GetChannel (NodeData *pChannelNode) {
	if (pChannelNode->specificType == SDP_ST_UINT8)
		return pChannelNode->u.uint8;
	else if (pChannelNode->specificType == SDP_ST_INT8)
		return pChannelNode->u.int8;
	else if (pChannelNode->specificType == SDP_ST_UINT16)
		return pChannelNode->u.uint16;
	else if (pChannelNode->specificType == SDP_ST_INT16)
		return pChannelNode->u.int16;
	else if (pChannelNode->specificType == SDP_ST_UINT32)
		return pChannelNode->u.uint32;
	else if (pChannelNode->specificType == SDP_ST_INT32)
		return pChannelNode->u.int32;

	return 0;
}

static HRESULT FindRFCOMMChannel (unsigned char *pStream, int cStream, unsigned char *pChann) {
	ISdpRecord **pRecordArg;
	int cRecordArg = 0;
	int i;

	*pChann = 0;

	HRESULT hr = ServiceAndAttributeSearch (pStream, cStream, &pRecordArg, (ULONG *)&cRecordArg);

	if (FAILED(hr))
		return hr;

	for (i = 0; (! *pChann) && (i < cRecordArg); i++) {
	    ISdpRecord *pRecord = pRecordArg[i];    // particular record to examine in this loop
	    CNodeDataFreeString protocolList;     // contains SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST data, if available

		if (ERROR_SUCCESS != pRecord->GetAttribute(SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST,&protocolList) ||
		    (protocolList.type != SDP_TYPE_CONTAINER))
			continue;

		ISdpNodeContainer *pRecordContainer = protocolList.u.container;
		int cProtocols = 0;
		NodeData protocolDescriptor; // information about a specific protocol (i.e. L2CAP, RFCOMM, ...)

		pRecordContainer->GetNodeCount((DWORD *)&cProtocols);
		for (int j = 0; (! *pChann) && (j < cProtocols); j++) {
			pRecordContainer->GetNode(j,&protocolDescriptor);

			if (protocolDescriptor.type != SDP_TYPE_CONTAINER)
				continue;

			ISdpNodeContainer *pProtocolContainer = protocolDescriptor.u.container;
			int cProtocolAtoms = 0;
			pProtocolContainer->GetNodeCount((DWORD *)&cProtocolAtoms);

			for (int k = 0; (! *pChann) && (k < cProtocolAtoms); k++) {
				NodeData nodeAtom;  // individual data element, such as what protocol this is or RFCOMM channel id.

				pProtocolContainer->GetNode(k,&nodeAtom);

				if (IsRfcommUuid(&nodeAtom))  {
					if (k+1 == cProtocolAtoms) {
						// misformatted response.  Channel ID should follow RFCOMM uuid
						break;
					}

					NodeData channelID;
					pProtocolContainer->GetNode(k+1,&channelID);

					*pChann = (unsigned char)GetChannel(&channelID);
					break; // formatting error
				}
			}
		}
	}

    for (i = 0; i < cRecordArg; i++) 
		pRecordArg[i]->Release();

    CoTaskMemFree(pRecordArg);

	return (*pChann != 0) ? NO_ERROR : E_FAIL;
}

static void CleanInquiryData (void) {
	while (g_pState->pDev) {
		InquiryResult *pNext = g_pState->pDev->pNext;
		delete g_pState->pDev;
		g_pState->pDev = pNext;
	}
}

static DWORD PerformInquiry(HWND hWndDevList, int fDoNames, HANDLE *phLookup) {
	int iRet = ERROR_SUCCESS;

	WSAQUERYSET		wsaq;
	memset (&wsaq, 0, sizeof(wsaq));
	wsaq.dwSize      = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpcsaBuffer = NULL;

	if (!fDoNames) {
		// perform initial device inquiry
		iRet = BthNsLookupServiceBegin (&wsaq, LUP_CONTAINERS, phLookup);
	} else {
		// reset iterator to front of list to find names
		DWORD dwUnused;
		iRet = BthNsLookupServiceNext (*phLookup, LUP_CONTAINERS | BTHNS_LUP_RESET_ITERATOR, &dwUnused, NULL);
	}
	
	if (iRet != ERROR_SUCCESS)
		return iRet;

	HANDLE hLookup = *phLookup;


	while((iRet == ERROR_SUCCESS) && (g_pState->fState == DO_INQUIRY))
	{
		union {
			CHAR buf[5000];
			SOCKADDR_BTH	__unused;	// properly align buffer to BT_ADDR requirements
		};

		LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
		DWORD dwSize  = sizeof(buf);

		memset(pwsaResults,0,sizeof(WSAQUERYSET));
		pwsaResults->dwSize      = sizeof(WSAQUERYSET);
		pwsaResults->dwNameSpace = NS_BTH;
		pwsaResults->lpBlob      = NULL;

		iRet = BthNsLookupServiceNext (hLookup, (fDoNames ? LUP_RETURN_NAME : 0) | LUP_RETURN_ADDR, &dwSize, pwsaResults);

		if (iRet == ERROR_SUCCESS) {

			InquiryResult *pRes = NULL;
			g_pState->Lock();
	
			if (fDoNames) {
				pRes = g_pState->pDev;
				while (pRes && (pRes->b != ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr))
					pRes = pRes->pNext;
			} else {
				pRes = new InquiryResult;
				if (pRes) {
					pRes->b = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
					pRes->pNext = g_pState->pDev;
					g_pState->pDev = pRes;
				}
			}
			g_pState->Unlock ();

			if (! pRes)
				continue;


			int fHaveName = fDoNames && (pwsaResults->lpszServiceInstanceName && *(pwsaResults->lpszServiceInstanceName));

			WCHAR szDevice[MAX_NAME+25];
			wsprintf (szDevice, L"%s%s%04x%08x%s", fHaveName ? pwsaResults->lpszServiceInstanceName : L"", 
				fHaveName ? L"(" : L"", GET_NAP(pRes->b), GET_SAP(pRes->b), fHaveName ? L")" : L"");

			if (fDoNames) {
				if (fHaveName) {
					for (int i = 0 ; ; ++i) {
						int iData = SendMessage (hWndDevList, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
						if (iData == LB_ERR)
							break;

						if (iData == (int)pRes) {
							if (SendMessage(hWndDevList, LB_GETTEXTLEN, (WPARAM)i, (LPARAM)0) <= (MAX_NAME + 64)) {
								SendMessage (hWndDevList, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
								SendMessage (hWndDevList, LB_INSERTSTRING, (WPARAM)i, (LPARAM)szDevice);
								SendMessage (hWndDevList, LB_SETITEMDATA,  (WPARAM)i, (LPARAM)pRes);
							}

							break;
						}
					}
				}
			} else {
				int iNdx = SendMessage (hWndDevList, LB_ADDSTRING, 0, (LPARAM)szDevice);
				if (iNdx >= LB_OKAY)
					SendMessage (hWndDevList, LB_SETITEMDATA, (WPARAM)iNdx, (LPARAM)pRes);
			}
		} else	//BthNsLookupServiceNext returns SOCKET_ERROR and sets last error
			iRet = GetLastError();
	}

	return ((iRet == WSA_E_NO_MORE) ? ERROR_SUCCESS : iRet);
}

static int DoSDP (BT_ADDR *pb) {
	CoInitializeEx (0, COINIT_MULTITHREADED);

	int iResult = 0;

	BTHNS_RESTRICTIONBLOB RBlob;

	memset (&RBlob, 0, sizeof(RBlob));

	RBlob.type = SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST;
	RBlob.numRange = 1;
	RBlob.pRange[0].minAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
	RBlob.pRange[0].maxAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
	RBlob.uuids[0].uuidType = SDP_ST_UUID16;
	RBlob.uuids[0].u.uuid16 = SerialPortServiceClassID_UUID16;

	BLOB blob;
	blob.cbSize = sizeof(RBlob);
	blob.pBlobData = (BYTE *)&RBlob;

	SOCKADDR_BTH	sa;

	memset (&sa, 0, sizeof(sa));

	*(BT_ADDR *)(&sa.btAddr) = *pb;
	sa.addressFamily = AF_BT;

	CSADDR_INFO		csai;

	memset (&csai, 0, sizeof(csai));
	csai.RemoteAddr.lpSockaddr = (sockaddr *)&sa;
	csai.RemoteAddr.iSockaddrLength = sizeof(sa);

	WSAQUERYSET		wsaq;

	memset (&wsaq, 0, sizeof(wsaq));
	wsaq.dwSize      = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpBlob      = &blob;
	wsaq.lpcsaBuffer = &csai;

	HANDLE hLookup;
	int iRet = BthNsLookupServiceBegin (&wsaq, 0, &hLookup);

	if (ERROR_SUCCESS == iRet) {
		CHAR buf[5000];
		LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
		DWORD dwSize  = sizeof(buf);

		memset(pwsaResults,0,sizeof(WSAQUERYSET));
		pwsaResults->dwSize      = sizeof(WSAQUERYSET);
		pwsaResults->dwNameSpace = NS_BTH;
		pwsaResults->lpBlob      = NULL;

		iRet = BthNsLookupServiceNext (hLookup, 0, &dwSize, pwsaResults);
		if (iRet == ERROR_SUCCESS) {	// Success - got the stream
			unsigned char cChannel = 0;
			if (ERROR_SUCCESS == FindRFCOMMChannel (pwsaResults->lpBlob->pBlobData,
					pwsaResults->lpBlob->cbSize, &cChannel))
				iResult = cChannel;
		}

		BthNsLookupServiceEnd(hLookup);
	}

	CoUninitialize ();
	return iResult;
}

static DWORD WINAPI DoInquiry(LPVOID lpUnused)
{
	HWND hWnd = (HWND)lpUnused;

	if((g_pState->fState == DO_STOP))
		return 0;
	
	g_pState->Lock ();
	if (g_pState->fState != DO_NOTHING) {
		g_pState->fState = DO_STOP;
		g_pState->Unlock ();

		return 0;
	}

	CleanInquiryData ();

	g_pState->fState = DO_INQUIRY;

	g_pState->Unlock ();

	HWND hWndButton  = GetDlgItem (hWnd, IDC_INQUIRY);
	HWND hWndDevList = GetDlgItem (hWnd, IDC_DEVICELIST);

	SetWindowText (hWndButton, L"STO&P");
	SetWindowText (hWnd, L"Inquiry Running...");
	SendMessage (hWndDevList, LB_RESETCONTENT, 0, 0);

	HANDLE hLookup;
	int iErr = PerformInquiry(hWndDevList, FALSE, &hLookup);

	if (iErr == ERROR_SUCCESS)
		iErr = PerformInquiry(hWndDevList, TRUE, &hLookup);

	BthNsLookupServiceEnd(hLookup);

	if (iErr != ERROR_SUCCESS) {
		SetWindowText (hWndButton, L"&Inquiry");
		SetWindowText (hWnd, L"Bluetooth ActiveSync");

		g_pState->fState = DO_NOTHING;

		WCHAR szString[64];
		wsprintf (szString, L"Bluetooth hardware error %d\n", iErr);
		MessageBox(hWnd, szString, L"Error", MB_OK | MB_TOPMOST);
		return 0;
	}

	SetWindowText (hWnd, L"SDP Queries");

	g_pState->Lock ();
	CHECK_STOP;

	g_pState->fState = DO_SDP;

	while(g_pState->fState != DO_STOP)
	{
		InquiryResult *pRes = g_pState->pDev;
		while (pRes && pRes->fHaveSDP)
			pRes = pRes->pNext;

		if (! pRes)
			break;

		pRes->fHaveSDP = TRUE;
		BT_ADDR b = pRes->b;

		g_pState->Unlock ();

		WCHAR szTitle[64];
		wsprintf (szTitle, L"SDP %04x%08x", GET_NAP(b), GET_SAP(b));
		SetWindowText (hWnd, szTitle);

		int channel = DoSDP (&b);

		if (channel) {
			for (int i = 0 ; ; ++i) {
				int iData = SendMessage (hWndDevList, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
				if (iData == LB_ERR)
					break;

				if (iData == (int)pRes) {
					WCHAR szOldName[MAX_NAME + 64];
					WCHAR szNewName[MAX_NAME + 128];
					if (SendMessage(hWndDevList, LB_GETTEXTLEN, (WPARAM)i, (LPARAM)0) <= (MAX_NAME + 64)) {
						SendMessage (hWndDevList, LB_GETTEXT, (WPARAM)i, (LPARAM)szOldName);
						wsprintf (szNewName, L"%s (c = %d)", szOldName, channel);

						SendMessage (hWndDevList, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
						SendMessage (hWndDevList, LB_INSERTSTRING, (WPARAM)i, (LPARAM)szNewName);
						SendMessage (hWndDevList, LB_SETITEMDATA,  (WPARAM)i, (LPARAM)pRes);
					}

					break;
				}
			}

		}

		g_pState->Lock ();
		if (channel) {
			InquiryResult *pX = g_pState->pDev;
			while (pX && (pX != pRes))
				pX = pX->pNext;

			if (pX)
				pX->channel = channel;
		}
	}

	g_pState->fState = DO_NOTHING;
	g_pState->Unlock ();

	SetWindowText (hWndButton, L"&Inquiry");
	SetWindowText (hWnd, L"Bluetooth ActiveSync");
	return 0;
}

static BOOL CALLBACK DlgProc2 (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowText (GetDlgItem (hWnd, IDC_CHANNEL), L"");
		SetFocus (GetDlgItem (hWnd, IDC_CHANNEL));

		return 0;

	case WM_COMMAND:
		{
			int wID = LOWORD(wParam);
			switch (wID)
			{
			case IDOK:
				{
					WCHAR szChannel[64];
					GetWindowText (GetDlgItem (hWnd, IDC_CHANNEL), szChannel, 64);
					int c = _wtoi (szChannel);
					EndDialog (hWnd, c);
				}
				return 0;

			case IDCANCEL:
				EndDialog (hWnd, 0);
				return 0;
			}
		}
		break;
	}

	return 0;
}

static int GetBA (WCHAR *pp, BT_ADDR *pba) {
	int i;
	while (*pp == ' ')
		++pp;

	for (i = 0 ; i < 4 ; ++i, ++pp) {
		if (! iswxdigit (*pp))
			return FALSE;

		int c = *pp;
		if (c >= 'a')
			c = c - 'a' + 0xa;
		else if (c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';

		if ((c < 0) || (c > 16))
			return FALSE;

		*pba = *pba * 16 + c;
	}

	for (i = 0 ; i < 8 ; ++i, ++pp) {
		if (! iswxdigit (*pp))
			return FALSE;

		int c = *pp;
		if (c >= 'a')
			c = c - 'a' + 0xa;
		else if (c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';

		if ((c < 0) || (c > 16))
			return FALSE;

		*pba = *pba * 16 + c;
	}

	if ((*pp != ' ') && (*pp != '\0'))
		return FALSE;

	return TRUE;
}

static void StopDevice (void) {
	if (g_pState->hDevice) {
		DeregisterDevice (g_pState->hDevice);
		g_pState->hDevice = NULL;
	}

	RasDeleteEntry (NULL, RAS_NAME_BLUETOOTH);
}

static int IsRegistered (void) {
	HKEY hk;

	if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\asui\\device", 0, KEY_ALL_ACCESS, &hk))
		return FALSE;

	RegCloseKey (hk);

	return TRUE;
}

static void Deregister (void) {
	StopDevice ();

	RegDeleteKey (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\asui\\device");
}

static int GetRegistration (WCHAR *szOut, int cSize)
{
	if(cSize < 64)
		return 0;

	wcscpy(szOut, L"No bonding");

	HKEY hk;

	if(ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"ExtModems\\bluetooth_syn", 0, KEY_READ, &hk)) {
		wcscpy (szOut, L"No sync key!");

		return 1;
	}

	WCHAR szString[256];
	DWORD dwSize = sizeof(szString);
	DWORD dwType;

	if((ERROR_SUCCESS != RegQueryValueEx (hk, L"port", NULL, &dwType, (BYTE *)szString, &dwSize)) ||
		(dwType != REG_SZ) || (dwSize > sizeof(szString))) {
		wcscpy (szOut, L"No sync port!");
		RegCloseKey (hk);

		return TRUE;
	}

	RegCloseKey (hk);

	if ((toupper (szString[0]) != 'C') || (toupper (szString[1]) != 'O') ||
		(toupper (szString[2]) != 'M') || (szString[3] < '0') || (szString[3] > '9') ||
		(szString[4] != ':') || (szString[5] != '\0')) {
		wcscpy (szOut, L"Bad AS port!");

		return TRUE;
	}

	DWORD ndx = szString[3] - '0';

	if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\asui\\device", 0, KEY_ALL_ACCESS, &hk))
		return TRUE;

	BT_ADDR b;
	unsigned char c;

	DWORD dwC;
	dwSize = sizeof(dwC);
	if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"channel", NULL, &dwType, (BYTE *)&dwC, &dwSize)) ||
		(dwType != REG_DWORD) || (dwSize != sizeof(dwC))) {
		RegCloseKey (hk);

		return TRUE;
	}

	c = (unsigned char)dwC;

	dwSize = sizeof(szString);

	if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"device", NULL, &dwType, (BYTE *)szString, &dwSize)) ||
		(dwType != REG_SZ) || (dwSize > sizeof(szString))) {
		RegCloseKey (hk);

		return TRUE;
	}

	RegCloseKey (hk);

	if ((c <= 0) || (c > 31)) {
		wcscpy (szOut, L"Bad channel!");
		return TRUE;
	}

	if (! GetBA (szString, &b)) {
		wcscpy (szOut, L"Bad address!");
		return TRUE;
	}

	wsprintf(szOut, L"%04x%08x %d %s on COM%d:", GET_NAP(b), GET_SAP(b), c, g_pState->hDevice ? L"Running" : L"Stopped", ndx);

	return TRUE;
}

static void CreateDevice (void) {
	HKEY hk;
	WCHAR szName[50];
	DWORD dwSize = sizeof(szName);
	DWORD dwType, dwDisp;

	if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"ExtModems\\bluetooth_syn", 0, 0, 0, KEY_ALL_ACCESS, 0, &hk, &dwDisp))
		return;

	if((ERROR_SUCCESS != RegQueryValueEx(hk, L"port", NULL, &dwType, (BYTE *)szName, &dwSize)) || (dwType != REG_SZ) || (dwSize > sizeof(szName)))
	{
		lstrcpy(szName, DEFAULT_AS_PORT);
		RegSetValueEx(hk, L"port", 0, REG_SZ, (BYTE *)szName, (_tcslen(szName) + 1) * sizeof(TCHAR));
	}

	RegCloseKey (hk);

	if( (toupper(szName[0]) != 'C') || (toupper (szName[1]) != 'O') || (toupper (szName[2]) != 'M') ||
		(szName[3] < '0') || (szName[3] > '9') || (szName[4] != ':') || (szName[5] != '\0'))
	{
		MessageBox (NULL, L"ActiveSync partner is not defined (bad port)!", L"Bluetooth AS Error", MB_OK | MB_TOPMOST);
		return;
	}

	DWORD ndx = szName[3] - '0';

	int imtu = 0;

	if(ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\asui", 0, KEY_READ, &hk))
		return;

	DWORD dw;
	dwSize = sizeof(dw);
	if((ERROR_SUCCESS == RegQueryValueEx(hk, L"mtu", NULL, &dwType, (BYTE *)&dw, &dwSize)) && (dwType == REG_DWORD) && (dwSize == sizeof(dw)))
		imtu = dw;

	RegCloseKey (hk);

	BT_ADDR b;
	unsigned char c;

	DWORD dwC;
	dwSize = sizeof(dwC);

	if(ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\asui\\device", 0, KEY_READ, &hk))
		return;

	if((ERROR_SUCCESS != RegQueryValueEx(hk, L"channel", 0, &dwType, (BYTE *)&dwC, &dwSize)) || (dwType != REG_DWORD) || (dwSize != sizeof(dwC)))
	{
		RegCloseKey (hk);

		return;
	}

	c = (unsigned char)dwC;

	WCHAR szString[256];
	dwSize = sizeof(szString);

	if((ERROR_SUCCESS != RegQueryValueEx(hk, L"device", 0, &dwType, (BYTE *)szString, &dwSize)) || (dwType != REG_SZ) || (dwSize > sizeof(szString)))
	{
		RegCloseKey (hk);

		return;
	}

	RegCloseKey (hk);

	if((c <= 0) || (c > 31))
	{
		MessageBox (NULL, L"Bad channel in AS registry!", L"Bluetooth AS Error", MB_OK | MB_TOPMOST);
		return;
	}

	if(!GetBA (szString, &b))
	{
		MessageBox(NULL, L"Bad address in AS registry!", L"Bluetooth AS Error", MB_OK | MB_TOPMOST);
		return;
	}

	StopDevice ();

	PORTEMUPortParams pp;
	memset (&pp, 0, sizeof(pp));

	pp.device = b;
	pp.channel = c;
	pp.uiportflags = 0;
	pp.imtu = imtu;

	g_pState->hDevice = RegisterDevice (L"COM", ndx, L"btd.dll", (DWORD)&pp);

	if(!g_pState->hDevice)
	{
		wsprintf(szString, L"Failed to register COM port, Error = %d", GetLastError ());

		MessageBox(NULL, szString, L"Bluetooth AS Error", MB_OK | MB_TOPMOST);
	}

	if(!g_pState->hDevice)
		return;

//	Name=`Bluetooth
//  +UseCountryAndAreaCodes=N
//  +SpecificIpAddr=N
//  +SpecificNameServers=N
//  +DeviceType=direct
//  +DeviceName=BluetoothSYN
//  +IpHeaderCompression=N
//  +SwCompression=N
//  UserName=guest
//  Password=guest

	RASENTRY RasEntry;

	RasEntry.dwSize = sizeof(RASENTRY);
	DWORD cb = sizeof(RASENTRY);
	RasGetEntryProperties (NULL, L"", &RasEntry, &cb, NULL, NULL);

	RasEntry.dwfOptions &= ~(RASEO_SpecificNameServers|RASEO_SpecificIpAddr | RASEO_IpHeaderCompression|RASEO_SwCompression|RASEO_UseCountryAndAreaCodes);

	wcscpy(RasEntry.szDeviceType, RASDT_Direct);
	wcscpy(RasEntry.szDeviceName, szName);

	RasSetEntryProperties(NULL, RAS_NAME_BLUETOOTH, &RasEntry, sizeof(RasEntry), NULL, 0);

//	RASDIALPARAMS	RasDialParams;
//	memset((char *)&RasDialParams, 0, sizeof(RasDialParams));

//	RasDialParams.dwSize = sizeof(RASDIALPARAMS);
//	wcscpy (RasDialParams.szEntryName, RAS_NAME_BLUETOOTH);

//	wcscpy (RasDialParams.szUserName, L"guest");
//	wcscpy (RasDialParams.szPassword, L"guest");

//	RasSetEntryDialParams(NULL, &RasDialParams, FALSE);
}

static void StopSearch (void) {
	int fWaitForStop = FALSE;

	g_pState->Lock ();

	if (g_pState->fState != DO_NOTHING) {
		g_pState->fState = DO_STOP;
		fWaitForStop = TRUE;
	}

	g_pState->Unlock ();

	if (fWaitForStop) {

		for (int i = 0 ; i < 20 ; ++i) {
			if (g_pState->fState == DO_NOTHING)
				break;

			Sleep (1000);
		}

	}
}

static int OK (HWND hWnd) {
	g_pState->Lock ();

	HINSTANCE hInst = g_pState->hInst;

	g_pState->Unlock ();

	StopSearch ();

	HWND hWndDevList   = GetDlgItem (hWnd, IDC_DEVICELIST);

	int iNdx = SendMessage (hWndDevList, LB_GETCURSEL, 0, 0);
	if (iNdx < 0) {
		MessageBox (hWnd, L"Nothing selected", L"Error", MB_OK | MB_TOPMOST);
		return FALSE;
	}

	int iItemData = SendMessage (hWndDevList, LB_GETITEMDATA, (WPARAM)iNdx, (LPARAM)0);

	g_pState->Lock ();
	InquiryResult *pRes = g_pState->pDev;
	while (pRes && (pRes != (InquiryResult *)iItemData))
		pRes = pRes->pNext;

	if (! pRes) {
		g_pState->Unlock ();
		MessageBox (hWnd, L"Please requery. This record is obsolete.", L"Error", MB_OK | MB_TOPMOST);
		return FALSE;
	}


	BT_ADDR b = pRes->b;
	unsigned char c = pRes->channel;
	int fHaveSDP = pRes->fHaveSDP;

	g_pState->Unlock ();

	if (! fHaveSDP) {
		c = DoSDP (&b);
	}

	if (! c)
		c = DialogBox (hInst, MAKEINTRESOURCE (IDD_ENTERCHANNEL), NULL, DlgProc2);

	if ((c <= 0) || (c >= 32))
		return FALSE;

	int iRes = BthRevokePIN(&b);

	if(ERROR_SUCCESS == iRes)
		iRes = BthRevokeLinkKey(&b);

	unsigned char pin[16] = "0000";
	iRes = BthSetPIN(&b, strlen((char *)pin), pin);
	if(ERROR_SUCCESS == iRes)
	{
		unsigned short h = 0;
		iRes = BthCreateACLConnection(&b, &h);
		if(ERROR_SUCCESS == iRes)
		{
			iRes = BthAuthenticate(&b);
			iRes = BthCloseConnection(h);
		}
	}

	HKEY hk;
	DWORD dwDisp;
	if (ERROR_SUCCESS == RegCreateKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\asui\\device", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, &dwDisp)) {
		WCHAR szAddress[13];
		DWORD dw = c;
		wsprintf (szAddress, L"%04x%08x", GET_NAP(b), GET_SAP(b));
		RegSetValueEx (hk, L"device", 0, REG_SZ, (BYTE *)szAddress, sizeof(szAddress));
		RegSetValueEx (hk, L"channel", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
		RegCloseKey (hk);
	}

	CreateDevice ();

	return TRUE;
}

static void SetButtonStates (HWND hWnd) {
	EnableWindow (GetDlgItem (hWnd, IDC_DEREGISTER), IsRegistered ());
	SetWindowText (GetDlgItem (hWnd, IDC_STARTSTOP), g_pState->hDevice ? L"STO&P" : L"S&TART");
	WCHAR szText[128];
	SetWindowText (GetDlgItem (hWnd, IDC_CURRENT), GetRegistration (szText, 128) ? szText : L"ERROR");
}

static int CopyValue(HKEY hkDest, HKEY hkSrc, WCHAR *pszValueName) { // Returns ERROR_SUCCESS
	WCHAR szValue[256];
	DWORD dwType = 0;
	DWORD dwSize = sizeof(szValue);
	
	if ((! hkDest) || (! hkSrc) || (! pszValueName))
		return ERROR_FILE_NOT_FOUND;

	int fRes = RegQueryValueEx(hkSrc, pszValueName, NULL, &dwType, (LPBYTE )szValue, &dwSize);
	if (fRes != ERROR_SUCCESS)
		return fRes;

	fRes = RegSetValueEx(hkDest, pszValueName, 0, dwType, (LPBYTE )szValue, dwSize);

	return fRes;
}

static int CopyValues(HKEY hkDest, HKEY hkSrc) { // Returns ERROR_SUCCESS
	DWORD dwIndex = 0;
	WCHAR szValueName[256];
	DWORD cbValueName = sizeof(szValueName) / sizeof(WCHAR);
	DWORD dwType = 0;

	while(ERROR_SUCCESS==RegEnumValue(hkSrc, dwIndex++, szValueName, &cbValueName, NULL, &dwType, NULL, NULL)) {
		int fRet = CopyValue (hkDest, hkSrc, szValueName);

		if (fRet != ERROR_SUCCESS)
			return fRet;

		cbValueName = sizeof(szValueName) / sizeof(WCHAR);
	}

	return ERROR_SUCCESS;
}

static int MoveRegistryKey (LPWSTR szTo, LPWSTR szFrom) { // Returns ERROR_SUCCESS
	HKEY hKeyDest=NULL;
	DWORD dwDisp = 0;

	int fRet = RegCreateKeyEx(HKEY_CURRENT_USER, szTo, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKeyDest, &dwDisp);
	if (fRet != ERROR_SUCCESS)
		return fRet;

	HKEY hKeySrc=NULL;
	fRet = RegOpenKeyEx(HKEY_CURRENT_USER, szFrom, 0, KEY_READ, &hKeySrc);

	if (fRet != ERROR_SUCCESS) {
		RegCloseKey (hKeyDest);
		RegDeleteKey (HKEY_CURRENT_USER, szTo);
		return fRet;
	}

	fRet = CopyValues (hKeyDest, hKeySrc);

	RegCloseKey(hKeyDest);
	RegCloseKey(hKeySrc);

	if (fRet != ERROR_SUCCESS)
		RegDeleteKey (HKEY_CURRENT_USER, szTo);
	else
		RegDeleteKey (HKEY_CURRENT_USER, szFrom);

	return fRet;
}

static void LaunchActiveSync () {
	HKEY hk;

	if (g_fHaveSav)
		return;

	MoveRegistryKey (L"ControlPanel\\CommSav", L"ControlPanel\\Comm");

	DWORD dwDisp;
	if (ERROR_SUCCESS == RegCreateKeyEx (HKEY_CURRENT_USER, L"ControlPanel\\Comm", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, &dwDisp)) {
		g_fHaveSav = TRUE;
		DWORD one = 1;

		RegSetValueEx (hk, L"Cnct", 0, REG_SZ, (LPBYTE)RAS_NAME_BLUETOOTH, sizeof(RAS_NAME_BLUETOOTH));
		RegSetValueEx (hk, L"AutoCnct", 0, REG_DWORD, (LPBYTE)&one, sizeof(one));

		RegCloseKey (hk);

		CeEventHasOccurred(NOTIFICATION_EVENT_RS232_DETECTED, NULL);
//		SetTimer (hTimer, 1, 5000, NULL);

/*		PROCESS_INFORMATION pi;

		WCHAR szProcName[128];
		wcscpy (szProcName, L"repllog.exe");

		WCHAR szParameters[128];
		wcscpy (szParameters, L"/" APP_RUN_AT_RS232_DETECT);

		int fRes = CreateProcess (szProcName, szParameters, NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);

		if (fRes) {
			CloseHandle (pi.hProcess);
			CloseHandle (pi.hThread);
		}
*/
	}
}

static void RestoreActiveSync (void) {
	if (! g_fHaveSav)
		return;

	MoveRegistryKey (L"ControlPanel\\Comm", L"ControlPanel\\CommSav");

	g_fHaveSav = FALSE;
}

static BOOL CALLBACK DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		SetButtonStates (hWnd);
		SetForegroundWindow (hWnd);
		SetFocus (GetDlgItem (hWnd, IDCANCEL));
		SendMessage(GetDlgItem(hWnd, IDC_DEVICELIST), LB_SETHORIZONTALEXTENT, 250, 0); 

		return 0;

	case WM_COMMAND:
		{
			int wID = LOWORD(wParam);
			switch (wID)
			{
			case IDOK:
				if (OK (hWnd))
				{
					RegFlushKey(HKEY_LOCAL_MACHINE);
					RegFlushKey(HKEY_CURRENT_USER);
				}
					EndDialog (hWnd, TRUE);

				return 0;

			case IDCANCEL:
				StopSearch ();

				EndDialog (hWnd, TRUE);
				return 0;

			case IDC_EXIT:
				StopSearch ();

				EndDialog (hWnd, FALSE);
				return 0;

			case IDC_STARTSTOP:
				if (g_pState->hDevice)
					StopDevice();
				else
					CreateDevice();
				SetButtonStates(hWnd);
				break;

			case IDC_DEREGISTER:
				Deregister ();
				SetButtonStates(hWnd);
				break;

			case IDC_INQUIRY:	// Inquiry
				CreateThread(NULL, 0, DoInquiry, (LPVOID)hWnd, 0, NULL);
				break;
			}
		}
		break;
	}

	return 0;
}

static BOOL CALLBACK DlgProc3(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			RECT rect;
			RECT r;
			HWND hwnd_dt = FindWindow(TEXT("DesktopExplorerWindow"), 0);
			GetClientRect(hwnd_dt, &rect);
			GetWindowRect(hWnd, &r);
			if(rect.right > 640)
				rect.right = 636;
			int x = rect.right - r.right - 16, y = rect.top + 4, cx = r.right, cy = r.bottom;
			SetWindowPos(hWnd,0, x, y, cx, cy,SWP_SHOWWINDOW);
			EnableWindow(GetDlgItem(hWnd, IDOK), g_pState->hDevice?1: 0);

			WCHAR szText[128];
			SetWindowText(GetDlgItem (hWnd, IDC_CURRENT), GetRegistration(szText, 128)?szText:L"ERROR");

			SetForegroundWindow(hWnd);
			SetFocus(GetDlgItem(hWnd, g_pState->hDevice?IDOK:IDC_CURRENT));

			return 1;
		}
		case WM_COMMAND:
		{
			int wID = LOWORD(wParam);
			switch(wID)
			{
				case IDCANCEL:
					EndDialog(hWnd, DO_STOP);
					return 0;

				case IDOK:
					LaunchActiveSync();
					break;
				case IDC_OPTIONS:
					DialogBox(g_pState->hInst, MAKEINTRESOURCE(IDD_MAINBOX), 0, DlgProc);
					break;
			}

			g_pState->Lock();
			g_pState->fState = DO_NOTHING;
			CleanInquiryData();
			g_pState->Unlock();

			EnableWindow(GetDlgItem(hWnd, IDOK), g_pState->hDevice?1: 0);

			WCHAR szText[128];
			SetWindowText(GetDlgItem (hWnd, IDC_CURRENT), GetRegistration(szText, 128)?szText:L"ERROR");

			SetForegroundWindow(hWnd);
			SetFocus(GetDlgItem(hWnd, g_pState->hDevice?IDOK:IDC_CURRENT));
			break;
		}
	}

	return 0;
}
extern "C" DWORD WaitForAPIReady (DWORD dwAPISet, DWORD dwTimeout);

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpszCmdLine, int nCmdShow)
{
    if( WAIT_OBJECT_0 != WaitForAPIReady(SH_WMGR, 10000)) {
        return 0;
    }

	g_pState = new Global;

	if(!g_pState)
		return 0;

	g_pState->hInst = hInst;

	CreateDevice ();

	DialogBox(g_pState->hInst, MAKEINTRESOURCE(IDD_STARTBOX), NULL, DlgProc3);

	StopDevice ();

	RestoreActiveSync();

	return 0;
}
