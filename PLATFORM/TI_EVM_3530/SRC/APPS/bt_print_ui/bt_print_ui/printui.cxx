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
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.


Abstract:
    Windows CE Bluetooth application sample

**/
#include <windows.h>
//#include <windev.h>
#include <stdio.h>

#include <winsock2.h>

#include <bt_api.h>
#include <bthapi.h>
#include <initguid.h>
#include <bt_sdp.h>
//#include <pkfuncs.h>


#include <svsutil.hxx>
#include "..\sdpcommon\sdpcommon.h"

#include "resource.h"

#define APPNAME     L"BluetoothPrintUI"

#define MAX_NAME    248
#define MAX_BA      20

#define DO_NOTHING  0
#define DO_INQUIRY  1
#define DO_NAMERES  2
#define DO_SDP      3
#define DO_STOP     4

#define DEFAULT_PRINTER_PORT L"COM7:"

extern "C" DWORD WaitForAPIReady (DWORD dwAPISet, DWORD dwTimeout);
void Print(TCHAR *szPortName, TCHAR *szTextToPrint);

struct InquiryResult {
    InquiryResult   *pNext;

    BT_ADDR         b;                  // Address of the item

    unsigned int    fHaveSDP  : 1;      // Do we have SDP record for it?

    unsigned char   channel;

    InquiryResult (void) {
        memset (this, 0, sizeof(*this));
    }
};

struct Global : public SVSSynch {
    int                 fState;
    HINSTANCE           hInst;
    HWND                hWnd;
    HWND                hWndHidden;

    HANDLE              hDevice;

    InquiryResult       *pDev;

    Global (void) {
        fState = DO_NOTHING;
        hInst  = NULL;
        hWnd   = NULL;
        hWndHidden = NULL;
        pDev   = NULL;
        hDevice = NULL;
    }
};

static Global *g_pState = NULL;

#define CHECK_STOP  \
    {                                                   \
        if ((g_pState->fState == DO_STOP) ||            \
                    (! g_pState->hWnd)) {               \
            g_pState->fState = DO_NOTHING;              \
            g_pState->Unlock ();                        \
                                                        \
            SetWindowText (hWndButton, L"Inquiry");     \
            SetWindowText (hWnd, L"Print UI");          \
                                                        \
            return 0;                                   \
        }                                               \
    }

//
//  SDP parsing is better illustrated in sdp sample
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
	int i,j;

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
        for (j = 0; (! *pChann) && (j < cProtocols); j++) {
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

    WSAQUERYSET     wsaq;
    memset (&wsaq, 0, sizeof(wsaq));
    wsaq.dwSize      = sizeof(wsaq);
    wsaq.dwNameSpace = NS_BTH;
    wsaq.lpcsaBuffer = NULL;

//    if (!fDoNames) {
        // perform initial device inquiry
        iRet = BthNsLookupServiceBegin (&wsaq, LUP_CONTAINERS, phLookup);
//    } else {
        // reset iterator to front of list to find names
//        DWORD dwUnused;
//        iRet = BthNsLookupServiceNext (*phLookup, BTHNS_LUP_RESET_ITERATOR, &dwUnused, NULL);
//    }
    
    if (iRet != ERROR_SUCCESS)
        return iRet;

    HANDLE hLookup = *phLookup;


    while ((iRet == ERROR_SUCCESS) && (g_pState->fState == DO_INQUIRY) && g_pState->hWnd) {
        union {
            CHAR buf[5000];
            SOCKADDR_BTH    __unused;   // properly align buffer to BT_ADDR requirements
        };

        LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
        DWORD dwSize  = sizeof(buf);

        memset(pwsaResults,0,sizeof(WSAQUERYSET));
        pwsaResults->dwSize      = sizeof(WSAQUERYSET);
        pwsaResults->dwNameSpace = NS_BTH;
        pwsaResults->lpBlob      = NULL;

        SVSUTIL_ASSERT(hLookup);
//        iRet = BthNsLookupServiceNext (hLookup, (fDoNames ? LUP_RETURN_NAME : 0) | LUP_RETURN_ADDR, &dwSize, pwsaResults);
        iRet = BthNsLookupServiceNext (hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults);

        if (iRet == ERROR_SUCCESS) {
            SVSUTIL_ASSERT (pwsaResults->dwNumberOfCsAddrs == 1);

            InquiryResult *pRes = NULL;
            g_pState->Lock();
    
            //if (fDoNames) {
            //    pRes = g_pState->pDev;
            //    while (pRes && (pRes->b != ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr))
            //        pRes = pRes->pNext;
            //} else {
                pRes = new InquiryResult;
                if (pRes) {
                    pRes->b = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
                    pRes->pNext = g_pState->pDev;
                    g_pState->pDev = pRes;
                }
            //}
            g_pState->Unlock ();

            if (! pRes)
                continue;


            int fHaveName = fDoNames && (pwsaResults->lpszServiceInstanceName && *(pwsaResults->lpszServiceInstanceName));

            WCHAR szDevice[MAX_NAME+25];
            wsprintf (szDevice, L"%s%s%04x%08x%s", fHaveName ? pwsaResults->lpszServiceInstanceName : L"", 
                fHaveName ? L"(" : L"", GET_NAP(pRes->b), GET_SAP(pRes->b), fHaveName ? L")" : L"");

/*            if (fDoNames) {
                int iNdx = SendMessage (hWndDevList, LB_ADDSTRING, 0, (LPARAM)szDevice);
                if (iNdx >= LB_OKAY)
                    SendMessage (hWndDevList, LB_SETITEMDATA, (WPARAM)iNdx, (LPARAM)pRes);
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
            } else */{
                int iNdx = SendMessage (hWndDevList, LB_ADDSTRING, 0, (LPARAM)szDevice);
                if (iNdx >= LB_OKAY)
                    SendMessage (hWndDevList, LB_SETITEMDATA, (WPARAM)iNdx, (LPARAM)pRes);
            }
        } else  //BthNsLookupServiceNext returns SOCKET_ERROR and sets last error
            iRet = GetLastError();
    }

    return ((iRet == WSA_E_NO_MORE) ? ERROR_SUCCESS : iRet);
}

PREFAST_SUPPRESS(262, "This sample uses a lot of memory on the stack");
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

    SOCKADDR_BTH    sa;

    memset (&sa, 0, sizeof(sa));

    *(BT_ADDR *)(&sa.btAddr) = *pb;
    sa.addressFamily = AF_BT;

    CSADDR_INFO     csai;

    memset (&csai, 0, sizeof(csai));
    csai.RemoteAddr.lpSockaddr = (sockaddr *)&sa;
    csai.RemoteAddr.iSockaddrLength = sizeof(sa);

    WSAQUERYSET     wsaq;

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
        if (iRet == ERROR_SUCCESS) {    // Success - got the stream
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

static DWORD WINAPI DoInquiry (LPVOID lpUnused) {
    g_pState->Lock ();

    HWND hWnd        = g_pState->hWnd;

    if ((! g_pState->hWnd) || (g_pState->fState == DO_STOP)) {
        g_pState->Unlock ();

        return 0;
    }
    
    if (g_pState->fState != DO_NOTHING) {
        g_pState->fState = DO_STOP;
        g_pState->Unlock ();

        SetWindowText (hWnd, L"Stopping...");

        return 0;
    }

    CleanInquiryData ();

    g_pState->fState = DO_INQUIRY;

    g_pState->Unlock ();

    HWND hWndButton  = GetDlgItem (hWnd, IDC_INQUIRY);
    HWND hWndDevList = GetDlgItem (hWnd, IDC_DEVICELIST);

    SetWindowText (hWndButton, L"STOP");
    SetWindowText (hWnd, L"Inquiry Running...");
    SendMessage (hWndDevList, LB_RESETCONTENT, 0, 0);

    HANDLE hLookup;
    int iErr;
//	iErr = PerformInquiry(hWndDevList, FALSE, &hLookup);
//    BthNsLookupServiceEnd(hLookup);

//    if (iErr == ERROR_SUCCESS)
        iErr = PerformInquiry(hWndDevList, TRUE, &hLookup);

    BthNsLookupServiceEnd(hLookup);

    if (iErr != ERROR_SUCCESS) {
        SetWindowText (hWndButton, L"Inquiry");
        SetWindowText (hWnd, L"Print UI");

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

    while (g_pState->hWnd && (g_pState->fState != DO_STOP)) {
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

    SetWindowText (hWndButton, L"Inquiry");
    SetWindowText (hWnd, L"Print UI");
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
        DeregisterBluetoothCOMPort (g_pState->hDevice);
        g_pState->hDevice = NULL;
    }

    HKEY hk;

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\printui\\device", 0, KEY_READ, &hk))
        return;

    WCHAR szName[50];
    DWORD dwSize = sizeof(szName);
    DWORD dwType;

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"port", NULL, &dwType, (BYTE *)szName, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize > sizeof(szName))) {
        RegCloseKey (hk);

        return;
    }

    RegCloseKey (hk);

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"Printers\\Ports", 0, KEY_ALL_ACCESS, &hk))
        return;


    WCHAR szPortName[10];
    for (int i = 1; i < 10 ; ++i) {
        wsprintf (szPortName, L"port%d", i);

        WCHAR szString[256];
        dwSize = sizeof(szString);
        if (ERROR_SUCCESS == RegQueryValueEx (hk, szPortName, NULL, &dwType, (BYTE *)szString, &dwSize)) {
            if ((dwType == REG_SZ) && (wcsicmp (szString, szName) == 0)) {
                RegDeleteValue (hk, szPortName);
                break;
            }
        }
    }

    RegCloseKey (hk);
}

static int IsRegistered (void) {
    HKEY hk;

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\printui\\device", 0, KEY_ALL_ACCESS, &hk))
        return FALSE;

    RegCloseKey (hk);

    return TRUE;
}

static void Deregister (void) {
    StopDevice ();

    RegDeleteKey (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\printui\\device");
}

static int GetRegistration (WCHAR *szOut, int cSize) {
    if (cSize < 64)
        return FALSE;

    wcscpy (szOut, L"No bonding");

    HKEY hk;

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\printui\\device", 0, KEY_ALL_ACCESS, &hk))
        return TRUE;

    WCHAR szString[256];
    DWORD dwSize = sizeof(szString);
    DWORD dwType;
    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"port", NULL, &dwType, (BYTE *)szString, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize > sizeof(szString))) {
        wcscpy (szOut, L"No printer port!");
        RegCloseKey (hk);

        return TRUE;
    }

    if ((toupper (szString[0]) != 'C') || (toupper (szString[1]) != 'O') ||
        (toupper (szString[2]) != 'M') || (szString[3] < '0') || (szString[3] > '9') ||
        (szString[4] != ':') || (szString[5] != '\0')) {
        wcscpy (szOut, L"Bad printer port!");

        RegCloseKey (hk);
        return TRUE;
    }

    DWORD ndx = szString[3] - '0';

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

    wsprintf (szOut, L"%04x%08x %d %s on COM%d:", GET_NAP(b), GET_SAP(b), c, g_pState->hDevice ? L"Running" : L"Stopped", ndx);

    return TRUE;
}

static void CreateDevice (void) {
    int imtu = 0;

    HKEY hk;
    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\printui", 0, KEY_READ, &hk))
        return;

    DWORD dw;
    DWORD dwSize = sizeof(dw);
    DWORD dwType;
    if ((ERROR_SUCCESS == RegQueryValueEx (hk, L"mtu", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
        (dwType == REG_DWORD) && (dwSize == sizeof(dw)))
        imtu = dw;

    RegCloseKey (hk);

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\printui\\device", 0, KEY_READ, &hk))
        return;

    BT_ADDR b;
    unsigned char c;
    WCHAR szName[50];
    dwSize = sizeof(szName);

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"port", NULL, &dwType, (BYTE *)szName, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize > sizeof(szName))) {
        MessageBox (NULL, L"Bluetooth printer is not defined (wrong or nonexistent port)!", L"PrintUI Error", MB_OK | MB_TOPMOST);
        RegCloseKey (hk);

        return;
    }

    if ((toupper (szName[0]) != 'C') || (toupper (szName[1]) != 'O') ||
        (toupper (szName[2]) != 'M') || (szName[3] < '0') || (szName[3] > '9') ||
        (szName[4] != ':') || (szName[5] != '\0')) {
        MessageBox (NULL, L"Bluetooth printer is not defined (bad port)!", L"PrintUI Error", MB_OK | MB_TOPMOST);
        RegCloseKey (hk);

        return;
    }

    DWORD ndx = szName[3] - '0';

    DWORD dwC;
    dwSize = sizeof(dwC);

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"channel", NULL, &dwType, (BYTE *)&dwC, &dwSize)) ||
        (dwType != REG_DWORD) || (dwSize != sizeof(dwC))) {
        RegCloseKey (hk);

        return;
    }

    c = (unsigned char)dwC;

    WCHAR szString[256];
    dwSize = sizeof(szString);

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"device", NULL, &dwType, (BYTE *)szString, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize > sizeof(szString))) {
        RegCloseKey (hk);

        return;
    }

    RegCloseKey (hk);

    if ((c <= 0) || (c > 31)) {
        MessageBox (NULL, L"Bad channel in print UI registry!", L"PrintUI Error", MB_OK | MB_TOPMOST);
        return;
    }

    if (! GetBA (szString, &b)) {
        MessageBox (NULL, L"Bad address in print UI registry!", L"PrintUI Error", MB_OK | MB_TOPMOST);
        return;
    }

    StopDevice ();

    PORTEMUPortParams pp;
    memset (&pp, 0, sizeof(pp));

    pp.device = b;
    pp.channel = c;
    pp.uiportflags = 0;
    pp.imtu = imtu;

    g_pState->hDevice = RegisterBluetoothCOMPort (L"COM", ndx, &pp);

    if (! g_pState->hDevice) {
        wsprintf (szString, L"Failed to register COM port, Error = %d", GetLastError ());

        MessageBox (NULL, szString, L"PrintUI Error", MB_OK | MB_TOPMOST);
    }

    if (! g_pState->hDevice)
        return;
/*
    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"Printers\\Ports", 0, KEY_ALL_ACCESS, &hk)) {
        MessageBox (NULL, L"Bluetooth printer is not defined (no registry entry)!", L"PrintUI Error", MB_OK | MB_TOPMOST);

        return;
    }

    int iMinPort = 10;

    WCHAR szPortName[10];
    for (int i = 1; i < 10 ; ++i) {
        wsprintf (szPortName, L"port%d", i);

        dwSize = sizeof(szString);
        if (ERROR_SUCCESS == RegQueryValueEx (hk, szPortName, NULL, &dwType, (BYTE *)szString, &dwSize)) {
            if ((dwType == REG_SZ) && (wcsicmp (szString, szName) == 0)) {
                RegCloseKey (hk);
                return;
            }
        } else if (iMinPort > i)
            iMinPort = i;
    }

    if (iMinPort > 10) {
        RegCloseKey (hk);
        MessageBox (NULL, L"Too many printers already defined!", L"PrintUI Error", MB_OK | MB_TOPMOST);
        return;
    }

    wsprintf (szPortName, L"port%d", iMinPort);
*/
	Print(szName, L"Hellow PCL\r\n");
    RegSetValueEx (hk, szName, 0, REG_SZ, (BYTE *)szName, (wcslen (szName) + 1) * sizeof(WCHAR));

    RegCloseKey (hk);
}

static void StopSearch (void) {
    int fWaitForStop = FALSE;

    BthNsLookupServiceEnd ((HANDLE)BTHNS_ABORT_CURRENT_INQUIRY);

    g_pState->Lock ();

    HWND hWnd = g_pState->hWnd;

    if (g_pState->fState != DO_NOTHING) {
        g_pState->fState = DO_STOP;
        fWaitForStop = TRUE;
    }

    g_pState->Unlock ();

    if (fWaitForStop) {
        SetWindowText (hWnd, L"Stopping search...");

        for (int i = 0 ; i < 20 ; ++i) {
            if (g_pState->fState == DO_NOTHING)
                break;

            Sleep (1000);
        }

        SetWindowText (hWnd, L"Print UI");
    }
}

static int OK (void) {
    g_pState->Lock ();

    HWND hWnd = g_pState->hWnd;
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
        SVSUTIL_ASSERT (! c);
        c = DoSDP (&b);
    }

    if (! c)
        c = DialogBox (hInst, MAKEINTRESOURCE (IDD_ENTERCHANNEL), NULL, DlgProc2);

    if ((c <= 0) || (c >= 32))
        return FALSE;

    HKEY hk;
    DWORD dwDisp;
    if (ERROR_SUCCESS == RegCreateKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\printui\\device", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, &dwDisp)) {
        WCHAR szAddress[13];
        DWORD dw = c;
        wsprintf (szAddress, L"%04x%08x", GET_NAP(b), GET_SAP(b));
        RegSetValueEx (hk, L"device", 0, REG_SZ, (BYTE *)szAddress, sizeof(szAddress));
        RegSetValueEx (hk, L"channel", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
        DWORD dwType = 0;
        WCHAR szPortName[50];
        DWORD cPortName = sizeof(szPortName);

        if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"port", NULL, &dwType, (BYTE *)szPortName, &cPortName)) ||
            (dwType != REG_SZ) || (cPortName > sizeof(szPortName)) || (wcsnicmp (szPortName, L"COM", 3) != 0) ||
            (szPortName[3] <'0') || (szPortName[3] > '9') || (szPortName[4] != '\0'))
            RegSetValueEx (hk, L"port", 0, REG_SZ, (BYTE *)DEFAULT_PRINTER_PORT, sizeof(DEFAULT_PRINTER_PORT));
        RegCloseKey (hk);
    }

    CreateDevice ();

    return TRUE;
}

static void SetButtonStates (HWND hWnd) {
    EnableWindow (GetDlgItem (hWnd, IDC_DEREGISTER), IsRegistered ());
    SetWindowText (GetDlgItem (hWnd, IDC_STARTSTOP), g_pState->hDevice ? L"STOP" : L"START");
    WCHAR szText[128];
    SetWindowText (GetDlgItem (hWnd, IDC_CURRENT), GetRegistration (szText, 128) ? szText : L"ERROR");
}

static BOOL CALLBACK DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        g_pState->hWnd = hWnd;
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
                if (OK ())
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
                    StopDevice ();
                else
                    CreateDevice ();
                SetButtonStates (hWnd);
                break;

            case IDC_DEREGISTER:
                Deregister ();
                SetButtonStates (hWnd);
                break;

            case IDC_INQUIRY:   // Inquiry
                CreateThread (NULL, 0, DoInquiry, NULL, 0, NULL);
                break;
            }
        }
        break;
    }

    return 0;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        return 0;

    case WM_DESTROY:
        PostQuitMessage (0);
        break;

    case WM_USER + 1:
        if (lParam == WM_LBUTTONDOWN) {
            if (g_pState->hWnd) {
                SetForegroundWindow (g_pState->hWnd);
                break;
            }

            if (! DialogBox (g_pState->hInst, MAKEINTRESOURCE (IDD_MAINBOX), NULL, DlgProc)) {
                NOTIFYICONDATA nid;
                memset (&nid, 0, sizeof(nid));

                nid.cbSize = sizeof(NOTIFYICONDATA);
                nid.hWnd   = hWnd;
                nid.uID    = 1;

                Shell_NotifyIcon (NIM_DELETE, &nid);

                DestroyWindow (hWnd);
            }

            g_pState->Lock ();
            g_pState->hWnd = NULL;
            g_pState->fState = DO_NOTHING;
            CleanInquiryData ();
            g_pState->Unlock ();
        }

        break;

    case WM_USER + 5:
        {
            if (g_pState->hWnd) {
                SetForegroundWindow (g_pState->hWnd);
                break;
            }

            NOTIFYICONDATA nid;
            memset (&nid, 0, sizeof(nid));

            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd   = hWnd;
            nid.uID    = 1;

            Shell_NotifyIcon (NIM_DELETE, &nid);

            DestroyWindow (hWnd);
        }

        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return FALSE;
}
#include <wingdi.h>
void Print(TCHAR *szPortName, TCHAR *szTextToPrint)

{
                DOCINFO di;

                RECT    rect;

                HDC hdc;

 

                di.cbSize      = sizeof(DOCINFO);

                di.lpszDocName = L"PRINT";

                di.lpszOutput  = NULL;

                di.fwType      = 0;

 

                hdc = CreateDC (L"PCL.dll", 0,  szPortName, 0);

                StartDoc  (hdc, &di);

 

                SetRect(&rect, 0, 0, GetDeviceCaps (hdc, HORZRES), GetDeviceCaps (hdc, VERTRES));

 

                StartPage(hdc);

 

                //Set printing fonts

                LOGFONT lf =  {0};

                lf.lfHeight = 150;

                lf.lfWeight = 100;

                lf.lfQuality = ANTIALIASED_QUALITY;

                lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

 

                _tcscpy(lf.lfFaceName, L"Tahoma");

 

                HFONT hFont = CreateFontIndirect(&lf), hOldFont;

                

                //print

                if ( hOldFont = (HFONT)SelectObject(hdc,hFont) ) 

                                ExtTextOut( hdc,

                                                                                0,

                                                                                0,

                                                                                ETO_CLIPPED,

                                                                                &rect,

                                                                                szTextToPrint,

                                                                                lstrlen(szTextToPrint),

                                                                                0);

 

 

                EndPage   (hdc);  

                EndDoc    (hdc);

                DeleteDC  (hdc);

}

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpszCmdLine, int nCmdShow) {
    if( WAIT_OBJECT_0 != WaitForAPIReady(SH_WMGR, 10000)) {
        return 0;
    }

    g_pState = new Global;

    if (! g_pState)
        return 0;

    g_pState->hInst = hInst;

    WNDCLASS    wc;
    memset (&wc, 0, sizeof(wc));

    wc.lpfnWndProc      = WndProc;
    wc.hInstance        = hInst;
    wc.lpszClassName    = APPNAME;

    if (! RegisterClass (&wc))
        return 0;

    g_pState->hWndHidden = CreateWindow (APPNAME, APPNAME, WS_DISABLED,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, hInst, NULL);

    NOTIFYICONDATA nid;
    memset (&nid, 0, sizeof(nid));

    nid.cbSize           = sizeof(NOTIFYICONDATA);
    nid.hWnd             = g_pState->hWndHidden;
    nid.uID              = 1;
    nid.uFlags           = NIF_ICON | NIF_MESSAGE;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon            = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_BTHICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    if (wcsstr (lpszCmdLine, L"/noicon") == 0)
        Shell_NotifyIcon (NIM_ADD, &nid);

    CreateDevice ();

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage (&msg) ;
        DispatchMessage(&msg);
    }

    StopDevice ();

    return 0;
}
