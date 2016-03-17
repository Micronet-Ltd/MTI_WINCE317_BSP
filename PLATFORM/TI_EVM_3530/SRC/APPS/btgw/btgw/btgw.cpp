// btgw.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "btgw.h"
#include <windows.h>
#include <commctrl.h>

#define MAX_LOADSTRING 100
//
//	Constants & macros
//
//
#define IPADDR(x) ((x>>24) & 0x000000ff), ((x>>16) & 0x000000ff), ((x>>8) & 0x000000ff),  (x & 0x000000ff)
#define CRLF L"\r\n"

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND g_hList;
#ifdef SHELL_AYGSHELL
HWND				g_hWndMenuBar;		// menu bar handle
#else // SHELL_AYGSHELL
HWND				g_hWndCommandBar;	// command bar handle
#endif // SHELL_AYGSHELL

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#define CONFIG_NAME L"WinCEBlueGW.config"
#define CONFIG_NAME_SZ (sizeof(CONFIG_NAME)/sizeof(WCHAR) - 1)

#define GW_SLEEP	30

#define SDP_RECORD_SIZE 0x0000004d
#define SDP_CHANNEL_OFFSET 26

static const BYTE rgbSdpRecordLAN[] = {
    0x35, 0x4b, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19,
    0x11, 0x02, 0x09, 0x00, 0x04, 0x35, 0x0c, 0x35,
    0x03, 0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00,
    0x03, 0x08,
                0x01,								// server channel goes here (+26)
                      0x09, 0x00, 0x06, 0x35, 0x09,
    0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01,
    0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06,
    0x19, 0x11, 0x02, 0x09, 0x01, 0x00, 0x09, 0x01,
    0x00, 0x25, 0x12, 'L',  'a',  'n',  ' ',  'A',
    'c',  'c',  'e',  's',  's',  ' ',  'P',  'r',
    'o',  'f',  'i',  'l',  'e'
};

static const BYTE rgbSdpRecordDUN[] = {
    0x35, 0x4b, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19,
    0x11, 0x03, 0x09, 0x00, 0x04, 0x35, 0x0c, 0x35,
    0x03, 0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00,
    0x03, 0x08,
                0x01,								// server channel goes here (+26)
                      0x09, 0x00, 0x06, 0x35, 0x09,
    0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01,
    0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06,
    0x19, 0x11, 0x03, 0x09, 0x01, 0x00, 0x09, 0x01,
    0x00, 0x25, 0x12, 'D',  'i',  'a',  'l',  '-',
    'u',  'p',  ' ',  'N',  'e',  't',  'w',  'o',
    'r',  'k',  'i',  'n',  'g'
};

const DWORD	bmProhibitAll			=	RASEO_ProhibitPAP |
                                        RASEO_ProhibitCHAP |
                                        RASEO_ProhibitMsCHAP |
                                        RASEO_ProhibitMsCHAP2 |
                                        RASEO_ProhibitEAP;

//
//	Data types
//
//
struct Port {
    Port			*pNext;
    unsigned int	fOld : 1;
    unsigned int	fModem : 1;

    int				index;
    int				index2;
    unsigned char	channel;

    HANDLE			hDevice;	// Primary port
    HANDLE			hDevice2;	// Secondary port (when primary is a filter)
    HANDLE			hFile;		// File handle
    HANDLE			hThread;	// Monitoring thread

    WCHAR			szDevName[_MAX_PATH];

    Port (void) {
        memset (this, 0, sizeof(*this));
        hFile = INVALID_HANDLE_VALUE;
    }
};

struct Security {
    Security		*pNext;
    int				fDeletePIN;
    int             fDeleteKEY;

    BT_ADDR			bt;
    int				cPin;
    int				cKey;
    unsigned char	acPin[16];
    unsigned char	acKey[16];

    Security (void) {
        memset (this, 0, sizeof(*this));
    }
};

struct Config {
    Port			*pPortList;
    Security		*pSecurityList;

    int				fa;
    int				fe;

    unsigned int    uiRasAuth;

    Config (void) {
        memset (this, 0, sizeof(*this));
    }
};

struct PortThreadArgs {
    HANDLE hFile;
    int    fLan;
};

static inline WCHAR hxd(unsigned char c) {
    if (c >= 10)
        return c - 10 + 'a';

    return c + '0';
}

static int GetDI (WCHAR **pp, unsigned int *pi) {
    while (iswspace(**pp))
        ++*pp;

    int iDig = 0;
    *pi = 0;
    while (iswdigit (**pp)) {
        int c = **pp;

        c = c - '0';

        if ((c < 0) || (c > 9))
            return FALSE;

        *pi = *pi * 10 + c;

        ++*pp;

        ++iDig;
    }

    if ((iDig <= 0) || (iDig > 10))
        return FALSE;

    return TRUE;
}

static int GetBA (WCHAR **pp, BT_ADDR *pba) {
    memset (pba, 0, sizeof (BT_ADDR));

    while (iswspace(**pp))
        ++*pp;

    for (int i = 0 ; i < 12 ; ++i, ++*pp) {
        if (! iswxdigit (**pp))
            return FALSE;

        int c = **pp;
        if (c >= 'a')
            c = c - 'a' + 0xa;
        else if (c >= 'A')
            c = c - 'A' + 0xa;
        else c = c - '0';

        if ((c < 0) || (c > 16))
            return FALSE;

        *pba = *pba * 16 + c;
    }

    if ((! iswspace(**pp)) && (**pp != '\0'))
        return FALSE;

    return TRUE;
}

static int GetUx (WCHAR **pp, void *pRes, int nDigs, int fPrefix) {
    while (iswspace(**pp))
        ++*pp;

    if (fPrefix) {
        if (**pp != '0')
            return FALSE;

        ++*pp;

        if (**pp != 'x')
            return FALSE;

        ++*pp;
    }

    int iDig = 0;
    int iRes = 0;
    while (iswxdigit (**pp)) {
        int c = **pp;
        if (c >= 'a')
            c = c - 'a' + 0xa;
        else if (c >= 'A')
            c = c - 'A' + 0xa;
        else c = c - '0';

        if ((c < 0) || (c > 16))
            return FALSE;

        iRes = iRes * 16 + c;

        ++*pp;

        ++iDig;
    }

    if (iDig != nDigs) // ???????????? iDig > nDig
        return FALSE;

    switch (nDigs) {
    case 2:
        *(unsigned char *)pRes = (unsigned char)iRes;
        break;
    case 4:
        *(unsigned short *)pRes = (unsigned short)iRes;
        break;
    case 8:
        *(unsigned int *)pRes = (unsigned int)iRes;
        break;
    }

    return TRUE;
}

//
//	PPP server configuration and management
//
//
static DWORD GetServerParameters(RASCNTL_SERVERSTATUS *pParameters) {
    DWORD cbParameters;

    DWORD dwResult = RasIOControl(0, RASCNTL_SERVER_GET_PARAMETERS, 0, 0, (unsigned char *)pParameters, sizeof(*pParameters), &cbParameters);

    if (dwResult != 0)
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Unable to get PPP server parameters, error=%d" CRLF, dwResult));

    return dwResult;
}

static DWORD SetServerParameters(RASCNTL_SERVERSTATUS *pParameters) {
    DWORD dwResult = RasIOControl(0, RASCNTL_SERVER_SET_PARAMETERS, (unsigned char *)pParameters, sizeof(*pParameters), NULL, 0, NULL);

    if (dwResult != 0)
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Unable to set PPP server parameters, error=%d" CRLF, dwResult));

    return dwResult;
}

static DWORD SetGlobalPPPConfig (void) {
    DWORD fDHCP = TRUE;
    HKEY hk;

    if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\gateway", 0, KEY_READ, &hk)) {
        DWORD dwType = 0;
        DWORD dwSize = sizeof(fDHCP);

        RegQueryValueEx (hk, L"UseDHCP", NULL, &dwType, (BYTE *)&fDHCP, &dwSize);

        if (! fDHCP) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: NOT using DHCP for dialup lines" CRLF));
        }

        RegCloseKey (hk);
    }

    RASCNTL_SERVERSTATUS	Parameters;

    DWORD dwResult = RasIOControl(0, RASCNTL_SERVER_ENABLE, 0, 0, NULL, 0, NULL);

    if (dwResult != ERROR_SUCCESS)
        return dwResult;

    dwResult = GetServerParameters(&Parameters);

    if (dwResult == ERROR_SUCCESS) {
        Parameters.bUseDhcpAddresses = fDHCP;
        dwResult = SetServerParameters(&Parameters);
    }

    if (dwResult == ERROR_SUCCESS) {
        RASCNTL_SERVERUSERCREDENTIALS	Credentials;

        memset(&Credentials, 0, sizeof(Credentials));

        wcscpy (Credentials.tszUserName, L"guest");
        strcpy ((char *)Credentials.password, "guest");
        Credentials.cbPassword = strlen ((char *)Credentials.password) + 1;

        dwResult = RasIOControl(0, RASCNTL_SERVER_USER_SET_CREDENTIALS, (PUCHAR)&Credentials, sizeof(Credentials), NULL, 0, NULL);
    }

    return dwResult;
}

static DWORD GetLineDevices (LPRASDEVINFOW *ppRasDevInfo, PDWORD pcDevices) {
    DWORD cbNeeded = 0;
    DWORD dwResult = ERROR_SUCCESS;

    *pcDevices = 0;
    *ppRasDevInfo = NULL;

    RasIOControl(0, RASCNTL_ENUMDEV, NULL, 0, (PUCHAR)&cbNeeded, 0, pcDevices);

    if (cbNeeded) {
        *ppRasDevInfo = (LPRASDEVINFOW)malloc(cbNeeded);

        if (*ppRasDevInfo == NULL) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Unable to allocate %d bytes of memory to store device info" CRLF, cbNeeded));
            dwResult = ERROR_OUTOFMEMORY;
        } else {
            (*ppRasDevInfo)->dwSize = sizeof(RASDEVINFOW);
            dwResult = RasIOControl(0, RASCNTL_ENUMDEV, (PUCHAR)*ppRasDevInfo, 0, (PUCHAR)&cbNeeded, 0, pcDevices);
        }
    }

    return dwResult;
}

static void DisplayPPP (void) {
    RASCNTL_SERVERSTATUS *pStatus = NULL;
    DWORD				 cbStatus = 0;
	TCHAR line[1024] = {0};
    //
    //	First pass is to get the size
    //
    RasIOControl(0, RASCNTL_SERVER_GET_STATUS, 0, 0, NULL, 0, &cbStatus);

    if (cbStatus)
        pStatus = (RASCNTL_SERVERSTATUS *)malloc (cbStatus);

    if (pStatus) {
        DWORD dwResult = RasIOControl(0, RASCNTL_SERVER_GET_STATUS, 0, 0, (PUCHAR)pStatus, cbStatus, &cbStatus);

        if (dwResult == ERROR_SUCCESS) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Ras Server Status:" CRLF));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: \tEnabled               = %x" CRLF, pStatus->bEnable));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: \tFlags                 = %x" CRLF, pStatus->bmFlags));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: \tUseDhcpAddresses      = %d" CRLF,  pStatus->bUseDhcpAddresses));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: \tStaticIpAddrStart     = %u.%u.%u.%u" CRLF,  IPADDR(pStatus->dwStaticIpAddrStart)));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: \tStaticIpAddrCount     = %u" CRLF,  pStatus->dwStaticIpAddrCount));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: \tAuthenticationMethods = %x" CRLF,  pStatus->bmAuthenticationMethods));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: \tNumLines              = %u" CRLF, pStatus->dwNumLines));
			if(g_hList)
			{
				wsprintf(line, L"RAS Server status");
				SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
				wsprintf(line, L"Use DHCP Address: %s", (pStatus->bUseDhcpAddresses)?L"Yes":L"No");
				SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
				wsprintf(line, L"Start Address: %u.%u.%u.%u", IPADDR(pStatus->dwStaticIpAddrStart));
				SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
				wsprintf(line, L"Addresses: %u", pStatus->dwStaticIpAddrCount);
				SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
			}
            //
            // Per line info is in an array immediately following the global server status struct
            //
            RASCNTL_SERVERLINE   *pLine = (RASCNTL_SERVERLINE *)(&pStatus[1]);

            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Configured Line Devices:" CRLF));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Type   Flags IdleTO Name" CRLF));
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: ------ ----- ------ ----" CRLF));
            for (int iLine = 0; iLine < (int)pStatus->dwNumLines; iLine++, pLine++)
			{
                RETAILMSG(1, (L"BLUETOOTH GATEWAY:: %6s %5x %6d %s" CRLF, pLine->rasDevInfo.szDeviceType, pLine->bmFlags, pLine->DisconnectIdleSeconds, pLine->rasDevInfo.szDeviceName));
				if(g_hList)
				{
					wsprintf(line, L"%6s line: %s", pLine->rasDevInfo.szDeviceType, pLine->rasDevInfo.szDeviceName);
					SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
				}
			}
        } else
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: RasIOControl failed, error = %d" CRLF, dwResult));

        free(pStatus);
    }

    LPRASDEVINFOW	pRasDevInfo;
    DWORD		cDevices;

    DWORD dwResult = GetLineDevices(&pRasDevInfo, &cDevices);

    if (dwResult == 0) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Available RAS Line Devices:" CRLF));
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: ##  Type     Name" CRLF));
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: -- ------   ----" CRLF));
        for (int iDevice = 0; iDevice < (int)cDevices; iDevice++)
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: %2d %6s   %s" CRLF, iDevice, pRasDevInfo[iDevice].szDeviceType, pRasDevInfo[iDevice].szDeviceName));

    }

    if (pRasDevInfo)
        free (pRasDevInfo);
}

static DWORD GetDeviceInfo (WCHAR *szDeviceName, RASDEVINFO *DeviceInfo) {
    LPRASDEVINFOW	pRasDevInfo;
    DWORD		cDevices;

    DWORD dwResult = GetLineDevices(&pRasDevInfo, &cDevices);

    if (dwResult == ERROR_SUCCESS) {
        dwResult = ERROR_NOT_FOUND;
        for (int i = 0; i < (int)cDevices; i++) {
            if (_tcscmp(pRasDevInfo[i].szDeviceName, szDeviceName) == 0) {
                *DeviceInfo = pRasDevInfo[i];
                dwResult = ERROR_SUCCESS;
                break;
            }
        }
    }

    if (pRasDevInfo)
        free (pRasDevInfo);

    return dwResult;
}

static DWORD
LineGetParameters(
    IN	RASCNTL_SERVERLINE	*pLine,
    OUT RASCNTL_SERVERLINE  **ppOutBuf,
    OUT DWORD				*pdwOutBufSize)
{
    DWORD dwResult = 0;
    PRASCNTL_SERVERLINE	 pOutBuf = NULL;
    DWORD dwOutBufSize = 0,
          dwNeededSize;

    do
    {
        dwResult = RasIOControl(0, RASCNTL_SERVER_LINE_GET_PARAMETERS, (PUCHAR)pLine, sizeof(*pLine), (PUCHAR)pOutBuf, dwOutBufSize, &dwNeededSize);
        if (dwResult != ERROR_BUFFER_TOO_SMALL)
            break;

        // Free old buffer
        if (pOutBuf)
            LocalFree(pOutBuf);

        // Allocate new buffer
        pOutBuf = (PRASCNTL_SERVERLINE)LocalAlloc (LPTR, dwNeededSize);
        if (pOutBuf == NULL)
        {
            dwResult = ERROR_OUTOFMEMORY;
            break;
        }
        dwOutBufSize = dwNeededSize;

    } while (TRUE);

    *ppOutBuf = pOutBuf;
    *pdwOutBufSize = dwOutBufSize;

    return dwResult;
}

static DWORD LineSetAuthentication(RASCNTL_SERVERLINE *pLine, unsigned int uiRasAuth) {
    RASCNTL_SERVERLINE *pLineSettings;
    DWORD  cbLineSettings;
    DWORD  dwResult;

    dwResult = LineGetParameters(pLine, &pLineSettings, &cbLineSettings);
    if (dwResult == NO_ERROR)
    {
        if (uiRasAuth == 0)
            pLineSettings->bmFlags |= PPPSRV_FLAG_ALLOW_UNAUTHENTICATED_ACCESS;
        else
            pLineSettings->bmFlags &= ~PPPSRV_FLAG_ALLOW_UNAUTHENTICATED_ACCESS;

        dwResult = RasIOControl(0, RASCNTL_SERVER_LINE_SET_PARAMETERS, (PUCHAR)pLineSettings, cbLineSettings, NULL, 0, NULL);
        LocalFree(pLineSettings);
    }

    return dwResult;
}

static DWORD ServerSetAuthentication (unsigned int uiRasAuth) {
    if (! uiRasAuth)
        return ERROR_SUCCESS;

    DWORD					cbStatus	= 0;
    
    // Retrieve RAS setting
    DWORD dwResult = RasIOControl(0, RASCNTL_SERVER_GET_PARAMETERS, 0, 0, NULL, 0, &cbStatus);
    if (dwResult && (cbStatus  == 0)) {
        RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Failed to get server parameters. Error code %d" CRLF, dwResult));
        return dwResult;
    }

    RASCNTL_SERVERSTATUS *pStatus = (RASCNTL_SERVERSTATUS *) malloc ( cbStatus );
    if (pStatus == NULL) {
        RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Failed to get server parameters. Error code %d" CRLF, ERROR_OUTOFMEMORY));
        return ERROR_OUTOFMEMORY;
    }

    dwResult = RasIOControl(0, RASCNTL_SERVER_GET_PARAMETERS, 0, 0, (PUCHAR)pStatus, cbStatus, &cbStatus);
    if (dwResult) {
        free (pStatus);
        RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Failed to get server parameters. Error code %d" CRLF, dwResult));
        return dwResult;
    }
    
    pStatus->bmAuthenticationMethods &= ~bmProhibitAll;
    pStatus->bmAuthenticationMethods |= uiRasAuth;

    dwResult = RasIOControl(0, RASCNTL_SERVER_SET_PARAMETERS, (PUCHAR)pStatus, cbStatus, 0, 0, &cbStatus);
    free (pStatus);

    if (dwResult) {
        RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Failed to set server parameters. Error code %d" CRLF, dwResult));
        return dwResult;
    }

    RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Successfully set server authentication." CRLF));

    return ERROR_SUCCESS;
}

static int AddAndEnable (WCHAR *szFriendlyName, unsigned int uiRasAuth) {
    RASCNTL_SERVERLINE	Line;

    DWORD dwResult = GetDeviceInfo (szFriendlyName, &Line.rasDevInfo);
    
    if (dwResult == ERROR_SUCCESS) {
        if (ERROR_SUCCESS == (dwResult = RasIOControl(0, RASCNTL_SERVER_LINE_ADD, (PUCHAR)&Line, sizeof(Line), NULL, 0, NULL))) {
            if (ERROR_SUCCESS == (dwResult = LineSetAuthentication(&Line, uiRasAuth))) {
                if (ERROR_SUCCESS == (dwResult = RasIOControl(0, RASCNTL_SERVER_LINE_ENABLE, (PUCHAR)&Line, sizeof(Line), NULL, 0, NULL)))
                    RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Successfully enabled device %s. Ready to receive calls." CRLF, szFriendlyName));
                else
                    RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Failed to enable device %s. Error code %d" CRLF, szFriendlyName, dwResult));
            } else
                RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Failed to disable authentication for device %s. Error code %d" CRLF, szFriendlyName, dwResult));
        } else
            RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Failed to add device %s. Error code %d" CRLF, szFriendlyName, dwResult));
    } else
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Device %s is not recognized, code %d" CRLF, szFriendlyName, dwResult));

    return dwResult;
}

static int DisableAndRemove (WCHAR *szFriendlyName) {
    RASCNTL_SERVERLINE	Line;

    DWORD dwResult = GetDeviceInfo (szFriendlyName, &Line.rasDevInfo);
    if (dwResult == ERROR_SUCCESS) {
        RasIOControl(0, RASCNTL_SERVER_LINE_DISABLE, (PUCHAR)&Line, sizeof(Line), NULL, 0, NULL);
        RasIOControl(0, RASCNTL_SERVER_LINE_REMOVE, (PUCHAR)&Line, sizeof(Line), NULL, 0, NULL);
    } else
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Device %s is not recognized, code %d" CRLF, szFriendlyName, dwResult));

    return dwResult;
}

//
//	Port management
//
static ULONG RegisterSDP (HANDLE hFile, int fLan) {
    ULONG recordHandle = 0;

    DWORD port = 0;
    DWORD dwSizeOut = 0;

    if (! DeviceIoControl (hFile, IOCTL_BLUETOOTH_GET_RFCOMM_CHANNEL, NULL, 0, &port, sizeof(port), &dwSizeOut, NULL)) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to retrieve port server channel, error = %d" CRLF, GetLastError ()));

        return 0;
    }

    struct {
        BTHNS_SETBLOB	b;
        unsigned char   uca[SDP_RECORD_SIZE];
    } bigBlob;

    ULONG ulSdpVersion = BTH_SDP_VERSION;

    bigBlob.b.pRecordHandle   = &recordHandle;
    bigBlob.b.pSdpVersion     = &ulSdpVersion;
    bigBlob.b.fSecurity       = 0;
    bigBlob.b.fOptions        = 0;
    bigBlob.b.ulRecordLength  = SDP_RECORD_SIZE;

    memcpy (bigBlob.b.pRecord, fLan ? rgbSdpRecordLAN : rgbSdpRecordDUN, SDP_RECORD_SIZE);

    bigBlob.b.pRecord[SDP_CHANNEL_OFFSET] = (unsigned char)port;

    BLOB blob;
    blob.cbSize    = sizeof(BTHNS_SETBLOB) + SDP_RECORD_SIZE - 1;
    blob.pBlobData = (PBYTE) &bigBlob;

    WSAQUERYSET Service;
    memset (&Service, 0, sizeof(Service));
    Service.dwSize = sizeof(Service);
    Service.lpBlob = &blob;
    Service.dwNameSpace = NS_BTH;

    int iRet = BthNsSetService (&Service, RNRSERVICE_REGISTER,0);
    if (iRet != ERROR_SUCCESS) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: BthNsSetService fails with status %d" CRLF, iRet));
        return 0;
    }

    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Created SDP record 0x%08x, channel %d" CRLF, recordHandle, port));
    return recordHandle;
}

static void DeRegisterSDP (ULONG recordHandle) {
    ULONG ulSdpVersion = BTH_SDP_VERSION;

    BTHNS_SETBLOB delBlob;
    memset (&delBlob, 0, sizeof(delBlob));
    delBlob.pRecordHandle = &recordHandle;
    delBlob.pSdpVersion = &ulSdpVersion;

    BLOB blob;
    blob.cbSize    = sizeof(BTHNS_SETBLOB);
    blob.pBlobData = (PBYTE) &delBlob;

    WSAQUERYSET Service;

    memset (&Service, 0, sizeof(Service));
    Service.dwSize = sizeof(Service);
    Service.lpBlob = &blob;
    Service.dwNameSpace = NS_BTH;

    int iErr = BthNsSetService (&Service, RNRSERVICE_DELETE, 0);
    RETAILMSG (1, (L"BLUETOOTH GATEWAY: removed SDP record 0x%08x (%d)" CRLF, recordHandle, iErr));
}

static DWORD WINAPI PortThread (LPVOID lparg) {
    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Entered port monitor thread" CRLF));
	TCHAR line[1024] = {0};

    HANDLE hFile = ((PortThreadArgs *)lparg)->hFile;
    int fLan = ((PortThreadArgs *)lparg)->fLan;
    LocalFree (lparg);

    ULONG recordHandle = 0;

    // Register SDP handle, etc.

    SetCommMask(hFile, EV_RLSD);
	wsprintf(line, L"LAN Access Profle is running");
	if(g_hList)
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);

    for ( ; ; ) {
        DWORD Mask;
        DWORD ModemStatus;

        if (! GetCommModemStatus (hFile, &ModemStatus)) {
            break;
        }

        if (ModemStatus & MS_RLSD_ON) {
            BT_ADDR bt;

            DWORD dwSizeOut = 0;
            if (! DeviceIoControl (hFile, IOCTL_BLUETOOTH_GET_PEER_DEVICE, NULL, 0, &bt, sizeof(bt), &dwSizeOut, NULL))
                RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to retrieve peer device name, error = %d" CRLF, GetLastError ()));
            else
                RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Connection detected from %04x%08x" CRLF, GET_NAP(bt), GET_SAP(bt)));

            if (recordHandle)
                DeRegisterSDP (recordHandle);

			if(g_hList)
			{
				wsprintf(line, L"Connection: %04x%08x", GET_NAP(bt), GET_SAP(bt));
				SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
			}
            recordHandle = 0;
        } else {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: No connection detected" CRLF));

            if (! recordHandle)
			{
                recordHandle = RegisterSDP (hFile, fLan);
				wsprintf(line, L"Disconnected");
				if(g_hList)
					SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
			}
        }

        if (! WaitCommEvent(hFile, &Mask, NULL )) {
            break;
        }
    }

    if (recordHandle)
        DeRegisterSDP (recordHandle);

    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Exiting port monitor thread" CRLF));

    return 0;
}

static void ClosePort (Port *pPort) {
    if (pPort->hFile != INVALID_HANDLE_VALUE)
        CloseHandle (pPort->hFile);

    pPort->hFile = INVALID_HANDLE_VALUE;

    if (pPort->hThread) {
        WaitForSingleObject (pPort->hThread, INFINITE);
        CloseHandle (pPort->hThread);
    }

    pPort->hThread = NULL;

    WCHAR szKeyName[_MAX_PATH];

    StringCchPrintf (STRING_AND_COUNTOF(szKeyName), L"software\\microsoft\\bluetoothgateway\\com%d\\unimodem", pPort->index);
    RegDeleteKey (HKEY_LOCAL_MACHINE, szKeyName);

    StringCchPrintf (STRING_AND_COUNTOF(szKeyName), L"software\\microsoft\\bluetoothgateway\\com%d", pPort->fModem ? pPort->index2 : pPort->index);
    RegDeleteKey (HKEY_LOCAL_MACHINE, szKeyName);

    if (pPort->fModem) {
        StringCchPrintf (STRING_AND_COUNTOF(szKeyName), L"software\\microsoft\\bluetoothgateway\\com%d", pPort->index2);
        RegDeleteKey (HKEY_LOCAL_MACHINE, szKeyName);
    }

    DisableAndRemove (pPort->szDevName);

    if (pPort->hDevice)
        DeactivateDevice (pPort->hDevice);

    if (pPort->hDevice2)
        DeactivateDevice (pPort->hDevice2);

    pPort->hDevice = NULL;
    pPort->hDevice2 = NULL;
}

static void OpenPort (Port *pPort, int fa, int fe, unsigned int uiRasAuth) {
    ASSERT (pPort->hDevice == NULL);
    ASSERT (pPort->hDevice2 == NULL);

    PORTEMUPortParams pp;

    memset (&pp, 0, sizeof(pp));
    pp.flocal = TRUE;
    pp.channel = pPort->channel;
    if (! pp.channel)
        pp.channel = 0xfe;

    if (fa)
        pp.uiportflags |= RFCOMM_PORT_FLAGS_AUTHENTICATE;

    if (fe)
        pp.uiportflags |= RFCOMM_PORT_FLAGS_ENCRYPT;

    WCHAR szKeyName[_MAX_PATH];

    StringCchPrintf (STRING_AND_COUNTOF(szKeyName), L"software\\microsoft\\bluetoothgateway\\com%d", pPort->index);

    HKEY hk;
    DWORD dwDisp = 0;

    if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, 0, KEY_WRITE, NULL, &hk, &dwDisp)) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to create registry key %s, error = %d" CRLF, szKeyName, GetLastError ()));
        return;
    }

    RegSetValueEx (hk, L"dll", 0, REG_SZ, (BYTE *)L"btd.dll", sizeof(L"btd.dll"));
    RegSetValueEx (hk, L"prefix", 0, REG_SZ, (BYTE *)L"COM", sizeof(L"COM"));

    DWORD dw = pPort->index;
    RegSetValueEx (hk, L"index", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

    dw = (DWORD) &pp;
    RegSetValueEx (hk, L"context", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

    if (! pPort->fModem) {
        HKEY hk2;

        if (ERROR_SUCCESS != RegCreateKeyEx (hk, L"unimodem", 0, NULL, 0, KEY_WRITE, NULL, &hk2, &dwDisp)) {
            RegCloseKey (hk);
            RegDeleteKey (HKEY_LOCAL_MACHINE, szKeyName);
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to create unimodem subkey of %s, error = %d" CRLF, szKeyName, GetLastError ()));
            return;
        }

        RegSetValueEx (hk2, L"friendlyname", 0, REG_SZ, (BYTE *)pPort->szDevName, (wcslen (pPort->szDevName) + 1) * sizeof(WCHAR));
        RegSetValueEx (hk2, L"tsp", 0, REG_SZ, (BYTE *)L"unimodem.dll", sizeof(L"unimodem.dll"));
        dw = 0;
        RegSetValueEx (hk2, L"devicetype", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

        RegCloseKey (hk2);
    }

    RegCloseKey (hk);

    pPort->hDevice = ActivateDevice (szKeyName, 0);

    if (! pPort->hDevice) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to register port %d, error = %d" CRLF, pPort->index, GetLastError ()));
        RegDeleteKey (HKEY_LOCAL_MACHINE, szKeyName);
        return;
    }

    if (pPort->fModem) {
        wsprintf (szKeyName, L"software\\microsoft\\bluetoothgateway\\com%d", pPort->index2);

        if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, 0, KEY_WRITE, NULL, &hk, &dwDisp)) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to create registry key %s, error = %d" CRLF, szKeyName, GetLastError ()));
            ClosePort (pPort);
            return;
        }

        RegSetValueEx (hk, L"dll", 0, REG_SZ, (BYTE *)L"btdun.dll", sizeof(L"btdun.dll"));
        RegSetValueEx (hk, L"prefix", 0, REG_SZ, (BYTE *)L"COM", sizeof(L"COM"));

        dw = pPort->index2;
        RegSetValueEx (hk, L"index", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

        dw = (DWORD) pPort->index;
        RegSetValueEx (hk, L"context", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

        HKEY hk2;

        if (ERROR_SUCCESS != RegCreateKeyEx (hk, L"unimodem", 0, NULL, 0, KEY_WRITE, NULL, &hk2, &dwDisp)) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to create unimodem subkey of %s, error = %d" CRLF, szKeyName, GetLastError ()));
            ClosePort (pPort);
            return;
        }

        RegSetValueEx (hk2, L"friendlyname", 0, REG_SZ, (BYTE *)pPort->szDevName, (wcslen (pPort->szDevName) + 1) * sizeof(WCHAR));
        RegSetValueEx (hk2, L"tsp", 0, REG_SZ, (BYTE *)L"unimodem.dll", sizeof(L"unimodem.dll"));
        dw = 0;
        RegSetValueEx (hk2, L"devicetype", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

        RegCloseKey (hk2);
        RegCloseKey (hk);

        pPort->hDevice2 = ActivateDevice (szKeyName, 0);

        if (! pPort->hDevice) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to register port %d, error = %d" CRLF, pPort->index2, GetLastError ()));
            ClosePort (pPort);
            return;
        }
    }

    Sleep (3000);	// Tapi need to process device insertion

    if (ERROR_SUCCESS != AddAndEnable (pPort->szDevName, uiRasAuth)) {
        ClosePort (pPort);
        return;
    }

    WCHAR szFile[40];
    StringCchPrintf (STRING_AND_COUNTOF(szFile), L"COM%d:", pPort->index);

    pPort->hFile = CreateFile (szFile, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (pPort->hFile == INVALID_HANDLE_VALUE) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed to open port %d, error = %d" CRLF, pPort->index, GetLastError ()));
        ClosePort (pPort);
        return;
    }

    PortThreadArgs *pArgs = (PortThreadArgs *)LocalAlloc (LMEM_FIXED, sizeof(PortThreadArgs));

    if (pArgs) {
        pArgs->fLan = ! pPort->fModem;
        pArgs->hFile = pPort->hFile;

        pPort->hThread = CreateThread (NULL, 0, PortThread, (LPVOID)pArgs, 0, NULL);
    }

    if (! pPort->hThread) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Failed attach thread to port %d, error = %d" CRLF, pPort->index, GetLastError ()));
        ClosePort (pPort);
    }
}

//
//	Configuration management and parsing
//
//
//#define DEFAULT_CONFIG_FILE L"AUTH=OFF\nENCRYPT=OFF\nPORT=btline0,7\nPORT=btline1,8,9\n"
#define DEFAULT_CONFIG_FILE L"AUTH=OFF\nENCRYPT=OFF\nPORT=btline0,7\n"

static void PrintDefaultHeader (FILE *fp, WCHAR *sz) {
    fwprintf (fp, L";\n; This is Bluetooth Gateway configuration file %s.\n;\n", sz);
    fwprintf (fp, L"; It is automatically regenerated on every update,\n");
    fwprintf (fp, L"; do not store any information (including comments)\n");
    fwprintf (fp, L"; aside of information inside defined keywords.\n;\n");
    fwprintf (fp, L"; The keywords legal in this file are:\n");
    fwprintf (fp, L";   AUTH={ON|OFF} require authentication on connection\n");
    fwprintf (fp, L";   ENCRYPT={ON|OFF} require encryption to be on on connection\n");
    fwprintf (fp, L";   RASAUTH={NONE|PAP CHAP MSCHAP MSCHAPv2}\n");
    fwprintf (fp, L";   PORT=name,number[:channel][,number] use COMx: port, use pre-defined channel\n");
    fwprintf (fp, L";              if given, otherwise auto-allocate\n");
    fwprintf (fp, L";              if second index is specified, use modem emulator on it\n");
    fwprintf (fp, L";   PINaddress=sequence of 1 to 16 double hex characters\n");
    fwprintf (fp, L";       e. g. PIN000101020304=30 31 32 32 34\n");
    fwprintf (fp, L";   KEYaddress=sequence of 16 double hex characters - same as PIN\n;\n");
}

static void MakeDefaultConfig (WCHAR *szName) {
    HKEY hk;
    int fReg = FALSE;
    WCHAR buffer[512];

    if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\gateway", 0, KEY_READ, &hk)) {
        DWORD dwType = 0;
        DWORD dwSize = sizeof(buffer);

        if (ERROR_SUCCESS == RegQueryValueEx (hk, L"config", NULL, &dwType, (BYTE *)buffer, &dwSize)) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Found registry definition for default gateway config in %s" CRLF, szName));
            fReg = TRUE;
        }

        RegCloseKey (hk);
    }

    if (! fReg) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Making default gateway config in %s" CRLF, szName));
        wcscpy (buffer, DEFAULT_CONFIG_FILE);
    }

    FILE *fp = _wfopen (szName, L"w");
    if (! fp) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Could not create configuration: failed to open %s! Error = %d" CRLF, szName, GetLastError ()));
        return;
    }

    PrintDefaultHeader (fp, L"(default configuration)");

    fwprintf (fp, L"%s", buffer);

    fclose (fp);
}

static void GetConfigFileName (WCHAR *szName, int cchName) {
    ASSERT (cchName > (CONFIG_NAME_SZ + 2));
    
    HKEY hk;
    bool fFoundName = false;
    
    // Check for the location of the config file in the registry
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\gateway", 0, KEY_READ, &hk)) {        
        DWORD dwType;
        DWORD dwSize = cchName * sizeof(WCHAR);

        if ((ERROR_SUCCESS == RegQueryValueEx(hk, L"ConfigFile", NULL, &dwType, (BYTE *)szName, &dwSize)) &&
            (dwType == REG_SZ)) {
            fFoundName = true;
        }

        RegCloseKey (hk);
    }
    
    // If the config file was not specified in the registry, use the default location
    if (! fFoundName) {
        StringCchPrintfW(szName, cchName, L"\\%s", CONFIG_NAME);
    }
    
    // If the file has not been created yet, create it with the default configuration
    if (0xffffffff == GetFileAttributes(szName)) {
        MakeDefaultConfig(szName);
    }
}

static Security *Get (Security *pList, BT_ADDR bt) {
    while (pList) {
        if (pList->bt == bt)
            return pList;
        pList = pList->pNext;
    }

    return NULL;
}

static Port *Get (Port *pList, int index) {
    while (pList) {
        if (pList->index == index)
            return pList;
        pList = pList->pNext;
    }

    return NULL;
}

static void Free (Security *pList) {
    while (pList) {
        Security *pNext = pList->pNext;
        delete pList;
        pList = pNext;
    }
}

static void Free (Port *pList) {
    while (pList) {
        Port *pNext = pList->pNext;
        delete pList;
        pList = pNext;
    }
}

static Security *GetRegistrySecurity (void) {
    HKEY hKey;

    Security *pResult = NULL;

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"comm\\security\\bluetooth", 0, KEY_READ, &hKey))
        return NULL;

    for (DWORD dwIndex = 0 ; ; ++dwIndex) {
        unsigned char link_key[16];
        DWORD clink_key = sizeof(link_key);

        WCHAR szName[32];
        DWORD cName = 32;

        DWORD dwType = 0;

        WCHAR *p = szName + 3;

        LONG res = RegEnumValue (hKey, dwIndex, szName, &cName, NULL, &dwType, NULL, NULL);
        if (res == ERROR_NO_MORE_ITEMS)
            break;

        BT_ADDR bt;

        if ((res == ERROR_SUCCESS) && (dwType == REG_BINARY) && (cName == 15) &&
                (wcsnicmp (szName, L"KEY", 3) == 0) && GetBA(&p, &bt)) {
            Security *pNew = Get (pResult, bt);
            if (! pNew) {
                pNew = new Security;

                if (! pNew) {
                    RETAILMSG (1, (L"BLUETOOTH GATEWAY:: OUT OF MEMORY! Data may be lost!\n"));
                    break;
                }

                pNew->bt = bt;
                pNew->pNext = pResult;
                pResult = pNew;
            }

            if (wcsnicmp (szName, L"KEY", 3) == 0) {
                pNew->cKey = 16;
                if (ERROR_SUCCESS != BthGetLinkKey (&bt, pNew->acKey)) {
                    delete pNew;
                } else {
                    RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Found link key for %04x%08x\n", GET_NAP(bt), GET_SAP(bt)));
                }
            } else {
                // PINs are exclusively managed by the file.
               RETAILMSG (1, (L"BLUETOOTH GATEWAY:: Found and ignored PIN code for %04x%08x in registry\n", GET_NAP(bt), GET_SAP(bt)));
            }
        }
    }

    RegCloseKey (hKey);

    return pResult;
}

//
//	Config file structure
//
//	PORT=name,index[:channel][,modem port index]
//	PIN<address>=sequence1..16
//  KEY<address>=sequence16
//  ENCRYPT={ON|OFF}
//  AUTHN={ON|OFF}
//  RASAUTH={NONE|PAP CHAP MSCHAP MSCHAPv2}
//	sequence={0xXX}+
//
static int Parse (WCHAR *szFileName, Config *pConfig) {
    Security *pRegistry = GetRegistrySecurity ();

    Security *pFileS = NULL;
    Port *pFileP = NULL;

    int fSyntaxError = FALSE;

    int fa_old = pConfig->fa;
    int fe_old = pConfig->fe;

    unsigned int rasauth_old = pConfig->uiRasAuth;

    pConfig->uiRasAuth = 0;

    FILE *fp = _wfopen (szFileName, L"r");
    if (fp) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Using config file %s" CRLF, szFileName));

        WCHAR szBuffer[256];
        int line = 0;
        while (! feof (fp)) {
            ++line;

            if (! fgetws (szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), fp))
                break;

            WCHAR *p = szBuffer;
            PREFAST_ASSERT (p);
            
            while (iswspace(*p))
                ++p;

            if ((*p == '\0') || (*p == ';'))
                continue;

            WCHAR *pKey=p;
            while ((*p != '\0') && (*p != '=') && (! iswspace(*p)))
                ++p;

            WCHAR *pKeyEnd = p;

            while (iswspace(*p))
                ++p;

            if ((*p != '=') || (*p == '\0')) {
                fSyntaxError = TRUE;
                ASSERT(0);
                RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected \'=\'" CRLF, line));
                break;
            }

            ++p;

            while (iswspace(*p))
                ++p;

            WCHAR *pVal = p;

            PREFAST_ASSERT (pKey);
            PREFAST_ASSERT (pKeyEnd);
            
            *pKeyEnd = L'\0';

            if (wcsicmp (pKey, L"PORT") == 0) {
                WCHAR *pName=pVal;
                while (*pVal && (! iswspace(*pVal)) && (*pVal != ','))
                    ++pVal;

                while (iswspace(*pVal)) {
                    *pVal = '\0';
                    ++pVal;
                }

                if (*pVal != ',') {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected PORT=name,0..9[:1-31][,0..9]" CRLF, line));
                    break;
                }

                *pVal = '\0';

                if (wcslen (pName) >= _MAX_PATH) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected PORT=name,0..9[:1-31][,0..9]" CRLF, line));
                    break;
                }

                ++pVal;

                while (iswspace(*pVal))
                    ++pVal;

                unsigned int index = 0;
                unsigned int channel = 0;

                if ((! GetDI (&pVal, &index)) || (index > 9)) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected PORT=name,0..9[:1-31][,0..9]" CRLF, line));
                    break;
                }

                while (iswspace (*pVal))
                    ++pVal;

                if (*pVal == ':') {
                    ++pVal;

                    while (iswspace (*pVal))
                        ++pVal;

                    if ((! GetDI (&pVal, &channel)) || (channel == 0) || (channel > 31)) {
                        fSyntaxError = TRUE;
                        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected PORT=name,0..9[:1-31][,0..9]" CRLF, line));
                        break;
                    }
                }

                while (iswspace (*pVal))
                    ++pVal;

                unsigned int index2 = 0;
                int fHaveIndex2 = FALSE;

                if (*pVal == ',') {	// Emulator port
                    ++pVal;

                    while (iswspace (*pVal))
                        ++pVal;

                    if ((! GetDI (&pVal, &index2)) || (index2 > 9)) {
                        fSyntaxError = TRUE;
                        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected PORT=name,0..9[:1-31][,0..9]" CRLF, line));
                        break;
                    }

                    fHaveIndex2 = TRUE;
                }

                while (iswspace (*pVal))
                    ++pVal;

                if ((*pVal != '\0') && (*pVal != ';')) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected PORT=name,0..9[:1-31][,0..9]" CRLF, line));
                    break;
                }

                Port *pNewPort = new Port;
                pNewPort->pNext = pFileP;
                pFileP = pNewPort;
                pNewPort->index = index;
                pNewPort->channel = channel;
                wcscpy (pNewPort->szDevName, pName);
                if (fHaveIndex2) {
                    pNewPort->fModem = TRUE;
                    pNewPort->index2 = index2;
                }

                RETAILMSG(1, (L"BLUETOOTH GATEWAY:: <%s> Port %d channel %d" CRLF, szFileName, pNewPort->index, pNewPort->channel));
                if (pNewPort->fModem)
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Modem emulation port %d" CRLF, pNewPort->index2));
            } else if ((wcsnicmp (pKey, L"PIN", 3) == 0) || (wcsnicmp (pKey, L"KEY", 3) == 0)) {
                BT_ADDR bt;
                p = pKey + 3;
                if (! GetBA(&p, &bt)) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected PIN/KEY<bt_addr>=xx xx xx..." CRLF, line));
                    break;
                }

				unsigned char value[16] = {0};
				int i;
                for (i = 0 ; i < 16 ; ++i) {
                    if (! GetUx (&pVal, value+i, 2, FALSE))
                        break;
                }

                while (iswspace(*pVal))
                    ++pVal;

                if (((*pVal != L'\0') && (*pVal != L';')) || (((pKey[0] == 'K') || (pKey[0] == 'k')) && (i != 16))) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected sequence of hex digits" CRLF, line));
                    break;
                }

                Security *pNewS = Get (pFileS, bt);

                if (! pNewS) {
                    pNewS = new Security;
                    pNewS->pNext = pFileS;
                    pFileS = pNewS;
                    pNewS->bt = bt;
                }

                if (pKey[0] == L'K') {
                    pNewS->cKey = 16;
                    memcpy (pNewS->acKey, value, 16);

                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: <%s> KEY %04x%08x" CRLF, szFileName, GET_NAP(pFileS->bt), GET_SAP(pFileS->bt)));
                } else {
                    pNewS->cPin = i;
                    memcpy (pNewS->acPin, value, i);

                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: <%s> PIN %04x%08x" CRLF, szFileName, GET_NAP(pFileS->bt), GET_SAP(pFileS->bt)));
                }
            } else if (wcsicmp (pKey, L"AUTH") == 0) {
                if (wcsnicmp (pVal, L"ON", 2) == 0) {
                    pConfig->fa = TRUE;
                    pVal += 2;
                } else if (wcsnicmp (pVal, L"OFF", 3) == 0) {
                    pConfig->fa = FALSE;
                    pVal += 3;
                } else {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected AUTH={ON|OFF}" CRLF, line));
                    break;
                }
                
                while (iswspace (*pVal))
                    ++pVal;

                if ((*pVal != '\0') && (*pVal != ';')) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected AUTH={ON|OFF}" CRLF, line));
                    break;
                }
            } else if (wcsicmp (pKey, L"RASAUTH") == 0) {
                if (wcsnicmp (pVal, L"NONE", 4) == 0) {
                    pConfig->uiRasAuth = 0;
                    pVal += 4;
                } else {
                    pConfig->uiRasAuth = bmProhibitAll;

                    for ( ; ; ) {
                        if (wcsnicmp (pVal, L"MSCHAPv2", 8) == 0) {
                            pConfig->uiRasAuth &= ~RASEO_ProhibitMsCHAP2;
                            pVal += 8;
                        } else if (wcsnicmp (pVal, L"MSCHAP", 6) == 0) {
                            pConfig->uiRasAuth &= ~RASEO_ProhibitMsCHAP;
                            pVal += 6;
                        } else if (wcsnicmp (pVal, L"CHAP", 4) == 0) {
                            pConfig->uiRasAuth &= ~RASEO_ProhibitCHAP;
                            pVal += 4;
                        } else if (wcsnicmp (pVal, L"PAP", 3) == 0) {
                            pConfig->uiRasAuth &= ~RASEO_ProhibitPAP;
                            pVal += 3;
                        } else {
                            fSyntaxError = TRUE;
                            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, unknown RAS auth type" CRLF, line));
                            break;
                        }

                        while (iswspace (*pVal))
                            ++pVal;

                        if ((*pVal == '\0') || (*pVal == ';'))
                            break;
                    }

                    if ((pConfig->uiRasAuth == bmProhibitAll) && (! fSyntaxError)) {
                        fSyntaxError = TRUE;
                        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected RAS auth type" CRLF, line));
                        break;
                    }

                    if (fSyntaxError)
                        break;
                }
                
                while (iswspace (*pVal))
                    ++pVal;

                if ((*pVal != '\0') && (*pVal != ';')) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected RASAUTH={NONE|PAP CHAP MSCHAP MSCHAPv2}" CRLF, line));
                    break;
                }
            } else if (wcsicmp (pKey, L"ENCRYPT") == 0) {
                if (wcsnicmp (pVal, L"ON", 2) == 0) {
                    pConfig->fe = TRUE;
                    pVal += 2;
                } else if (wcsnicmp (pVal, L"OFF", 3) == 0) {
                    pConfig->fe = FALSE;
                    pVal += 3;
                } else {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected ENCRYPT={ON|OFF}" CRLF, line));
                    break;
                }
                
                while (iswspace (*pVal))
                    ++pVal;

                if ((*pVal != '\0') && (*pVal != ';')) {
                    fSyntaxError = TRUE;
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, expected ENCRYPT={ON|OFF}" CRLF, line));
                    break;
                }
            } else {
                fSyntaxError = TRUE;
                RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Syntax error near line %d, unrecognized token %s" CRLF, line, pKey));
                break;
            }
        }

        fclose (fp);

    } else
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: config file %s not found" CRLF, szFileName));

    if (fSyntaxError) {
        Free (pFileS);
        Free (pFileP);
        Free (pRegistry);

        return FALSE;
    }

    ServerSetAuthentication (pConfig->uiRasAuth);

    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Merging configuration : ports" CRLF));

    Port *pP = pConfig->pPortList;
    while (pP) {
        pP->fOld = TRUE;
        pP = pP->pNext;
    }


    while (pFileP) {
        Port *pNext = pFileP->pNext;
        Port *pExist = Get (pConfig->pPortList, pFileP->index);

        if (pExist && (fa_old == pConfig->fa) && (fe_old == pConfig->fe) && (rasauth_old == pConfig->uiRasAuth) &&
                ((pExist->channel == pFileP->channel) || (pFileP->channel == 0))) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Port %d on channel %d already exists" CRLF, pFileP->index, pFileP->channel));

            pExist->fOld = FALSE;

            delete pFileP;
            pFileP = pNext;

            continue;
        }

        if (pExist) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Port %d on channel %d => channel %d %s %s" CRLF, pFileP->index, pExist->channel, pFileP->channel, pConfig->fa ? L"AUTH" : L"", pConfig->fe ? L"ENCRYPT" : L""));

            ClosePort (pExist);
            pExist->channel = pFileP->channel;
            pExist->fOld = FALSE;

            delete pFileP;
            pFileP = pNext;

            OpenPort (pExist, pConfig->fa, pConfig->fe, pConfig->uiRasAuth);

            continue;
        }

        pFileP->pNext = pConfig->pPortList;
        pConfig->pPortList = pFileP;
        OpenPort (pFileP, pConfig->fa, pConfig->fe, pConfig->uiRasAuth);

        pFileP = pNext;
    }

    Port *pParentP = NULL;
    pP = pConfig->pPortList;

    while (pP) {
        if (pP->fOld) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Port %d on channel %d is closed" CRLF, pP->index, pP->channel));

            if (! pParentP)
                pConfig->pPortList = pP->pNext;
            else
                pParentP->pNext = pP->pNext;

            ClosePort (pP);
            delete pP;
            pP = pParentP ? pParentP->pNext : pConfig->pPortList;
            continue;
        }

        pParentP = pP;
        pP = pP->pNext;
    }

    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Merging configuration : security" CRLF));

    Security *pS = pConfig->pSecurityList;
    while (pS) {
        pS->fDeleteKEY = TRUE;
        pS->fDeletePIN = TRUE;
        pS = pS->pNext;
    }


    // New = file + registry - old...

    while (pRegistry) {
        Security *pNext = pRegistry->pNext;
        Security *pExist = Get (pConfig->pSecurityList, pRegistry->bt);
        Security *pFileSec = Get (pFileS, pRegistry->bt);

        if (pFileSec) // Registry ALWAYS superceedes file for link keys
            pFileSec->cKey = 0;

        if (pExist && (pRegistry->cKey == 16)) {
            if ((pExist->acKey == 0) || memcmp (pExist->acKey, pRegistry->acKey, 16))
                RETAILMSG(1, (L"BLUETOOTH GATEWAY:: KEY for %04x%08x is updated" CRLF, GET_NAP(pRegistry->bt), GET_SAP(pRegistry->bt)));

            pExist->cKey = 16;
            memcpy (pExist->acKey, pRegistry->acKey, 16);
            delete pRegistry;
            pRegistry = pNext;
            continue;
        }

        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: New KEY for %04x%08x" CRLF, GET_NAP(pRegistry->bt), GET_SAP(pRegistry->bt)));

        pRegistry->pNext = pConfig->pSecurityList;
        pConfig->pSecurityList = pRegistry;
        pRegistry = pNext;
    }

    while (pFileS) {
        Security *pNext = pFileS->pNext;
        Security *pExist = Get (pConfig->pSecurityList, pFileS->bt);

        if (pExist) {
            if (pFileS->cKey) {
                pExist->fDeleteKEY = FALSE;
                if (memcmp (pFileS->acKey, pExist->acKey, 16)) {
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: KEY for %04x%08x is updated" CRLF, GET_NAP(pFileS->bt), GET_SAP(pFileS->bt)));

                    pExist->cKey = 16;
                    memcpy (pExist->acKey, pFileS->acKey, 16);

                    BthSetLinkKey (&pExist->bt, pExist->acKey);
                }
            }

            if (pFileS->cPin) {
                pExist->fDeletePIN = FALSE;
                if ((pFileS->cPin != pExist->cPin) || memcmp (pFileS->acPin, pExist->acPin, 16)) {
                    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: PIN for %04x%08x is updated" CRLF, GET_NAP(pFileS->bt), GET_SAP(pFileS->bt)));

                    pExist->cPin = pFileS->cPin;
                    memcpy (pExist->acPin, pFileS->acPin, pFileS->cPin);

                    BthSetPIN (&pExist->bt, pExist->cPin, pExist->acPin);
                }
            }

            delete pFileS;
            pFileS = pNext;
            continue;
        }

        if (pFileS->cKey) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: New KEY for %04x%08x" CRLF, GET_NAP(pFileS->bt), GET_SAP(pFileS->bt)));
            BthSetLinkKey (&pFileS->bt, pFileS->acKey);
        }

        if (pFileS->cPin) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: New PIN for %04x%08x" CRLF, GET_NAP(pFileS->bt), GET_SAP(pFileS->bt)));
            BthSetPIN (&pFileS->bt, pFileS->cPin, pFileS->acPin);
        }

        pFileS->pNext = pConfig->pSecurityList;
        pConfig->pSecurityList = pFileS;
        pFileS = pNext;
    }

    Security *pParentS = NULL;
    pS = pConfig->pSecurityList;

    while (pS) {
        if (pS->fDeletePIN) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: PIN for %04x%08x is retired" CRLF, GET_NAP(pS->bt), GET_SAP(pS->bt)));
            if (pS->cPin)
                BthRevokePIN (&pS->bt);
            pS->cPin = 0;
            pS->fDeletePIN = FALSE;
        }

        if (pS->fDeleteKEY) {
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: KEY for %04x%08x is retired" CRLF, GET_NAP(pS->bt), GET_SAP(pS->bt)));
            if (pS->cKey)
                BthRevokeLinkKey (&pS->bt);
            pS->cKey = 0;
            pS->fDeleteKEY = FALSE;
        }

        if ((pS->cKey == 0) && (pS->cPin == 0)) {
            if (! pParentS)
                pConfig->pSecurityList = pS->pNext;
            else
                pParentS->pNext = pS->pNext;

            delete pS;
            pS = pParentS ? pParentS->pNext : pConfig->pSecurityList;
            continue;
        }

        pParentS = pS;
        pS = pS->pNext;
    }

    return TRUE;
}

static Config *ReadConfig (void) {
    Config *pConfig = new Config;
    WCHAR szFileName[_MAX_PATH];
    GetConfigFileName (szFileName, _countof(szFileName));
    Parse (szFileName, pConfig);

    return pConfig;
}

static void WriteByteSequence (FILE *fp, int c, unsigned char *p) {
    while (c > 0) {
        fputc (hxd((*p) >> 4), fp);
        fputc (hxd((*p) & 0xf), fp);
        fputwc (L' ', fp);

        ++p;
        --c;
    }
    fputc ('\n', fp);
}

static void Write (Config *pConfig) {
    WCHAR szFileName[_MAX_PATH];
    GetConfigFileName (szFileName, _countof(szFileName));
    FILE *fp = _wfopen (szFileName, L"w");
    if (! fp) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Could not save configuration: failed to open %s! Error = %d" CRLF, szFileName, GetLastError ()));
        return;
    }

    PrintDefaultHeader (fp, L"");

    fwprintf (fp, L"AUTH=%s\t\t; Legal values are ON or OFF\n", pConfig->fa ? L"ON":L"OFF");
    fwprintf (fp, L"ENCRYPT=%s\t\t; Legal values are ON or OFF\n", pConfig->fe ? L"ON":L"OFF");

    if (pConfig->uiRasAuth) {
        fwprintf (fp, L"RASAUTH=");
        if ((pConfig->uiRasAuth & RASEO_ProhibitPAP) == 0)
            fwprintf (fp, L"PAP ");
        if ((pConfig->uiRasAuth & RASEO_ProhibitCHAP) == 0)
            fwprintf (fp, L"CHAP ");
        if ((pConfig->uiRasAuth & RASEO_ProhibitMsCHAP) == 0)
            fwprintf (fp, L"MSCHAP ");
        if ((pConfig->uiRasAuth & RASEO_ProhibitMsCHAP2) == 0)
            fwprintf (fp, L"MSCHAPv2 ");

        fwprintf (fp, L"\t\t; Legal values are NONE or a list of any of PAP CHAP MSCHAP MSCHAPv2\n");
    }

    Port *pP = pConfig->pPortList;
    while (pP) {
        fwprintf (fp, L"; configure COM%d:\n", pP->index);
        WCHAR szChan[20];
        WCHAR szNdx2[20];

        szChan[0] = '\0';
        szNdx2[0] = '\0';

        if (pP->channel)
            StringCchPrintf (STRING_AND_COUNTOF(szChan), L":%d", pP->channel);

        if (pP->fModem)
            StringCchPrintf (STRING_AND_COUNTOF(szNdx2), L",%d", pP->index2);

        fwprintf (fp, L"PORT=%s,%d%s%s\n", pP->szDevName, pP->index, szChan, szNdx2);

        pP = pP->pNext;
    }

    Security *pS = pConfig->pSecurityList;
    while (pS) {
        if (pS->cPin) {
            fwprintf (fp, L"; set PIN code for device %04x%08x\n", GET_NAP(pS->bt), GET_SAP(pS->bt));
            fwprintf (fp, L"PIN%04x%08x=", GET_NAP(pS->bt), GET_SAP(pS->bt));
            WriteByteSequence (fp, pS->cPin, pS->acPin);
        }

        if (pS->cKey) {
            fwprintf (fp, L"; set link key for device %04x%08x\n", GET_NAP(pS->bt), GET_SAP(pS->bt));
            fwprintf (fp, L"KEY%04x%08x=", GET_NAP(pS->bt), GET_SAP(pS->bt));
            WriteByteSequence (fp, pS->cKey, pS->acKey);
        }

        pS = pS->pNext;
    }

    fclose (fp);
    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: configuration saved to file %s" CRLF, szFileName));
}

static void UpdateConfig (Config *pConfig) {
    WCHAR szFileName[_MAX_PATH];
    GetConfigFileName (szFileName, _countof(szFileName));
    if (Parse (szFileName, pConfig))
        Write (pConfig);
}

HANDLE hExit;
Config *pConfig;
static DWORD WINAPI AthThread (LPVOID lparg) {
    RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Entered paring thread" CRLF));

    HANDLE hEvent = CreateEvent (NULL, FALSE, FALSE, L"system/events/bluetooth/gateway/refresh");
    if (! hEvent) {
        CloseHandle (hExit);
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Could not create event! Error = %d" CRLF, GetLastError ()));
        return 0;
    }

    HANDLE hSecurity = CreateEvent (NULL, TRUE, FALSE, BTH_NAMEDEVENT_PAIRING_CHANGED);

    HANDLE hWaitArray[3];
    hWaitArray[0] = hEvent;
    hWaitArray[1] = hExit;
    hWaitArray[2] = hSecurity;

    DWORD cWaitEvents = hSecurity ? 3 : 2;

    for ( ; ; ) {
        DWORD dwRes = WaitForMultipleObjects (cWaitEvents, hWaitArray, FALSE, INFINITE);

        if ((dwRes == WAIT_OBJECT_0) || (dwRes == (WAIT_OBJECT_0 + 2))) { // Refresh config
            UpdateConfig (pConfig);
            DisplayPPP ();
            continue;
        }

        if (dwRes != (WAIT_OBJECT_0 + 1))
            RETAILMSG(1, (L"BLUETOOTH GATEWAY:: fatal error %d" CRLF, GetLastError ()));

        break;
    }


    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;
	HANDLE hAth;

    hExit     = CreateEvent (NULL, FALSE, FALSE, L"system/events/bluetooth/gateway/exit");
    if (! hExit) {
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Could not create event! Error = %d" CRLF, GetLastError ()));
        return 0;
    }
    if (SetGlobalPPPConfig() != 0) {
        CloseHandle (hExit);
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Initialization of PPP server failed!" CRLF));
        return 0;
    }

    WSADATA wsd;
    if (WSAStartup (MAKEWORD(1,0), &wsd)) {
        CloseHandle (hExit);
        RETAILMSG(1, (L"BLUETOOTH GATEWAY:: Initialization of socket subsystem failed! Error = %d" CRLF, WSAGetLastError ()));
        return 0;
    }


	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}
    pConfig = ReadConfig ();
    DisplayPPP ();

    hAth = CreateThread (NULL, 0, AthThread, 0, 0, 0);


	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BTGW));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    RETAILMSG(1, (CRLF CRLF CRLF CRLF L"BLUETOOTH GATEWAY:: cleaning up and exiting..." CRLF CRLF CRLF CRLF));

	SetEvent(hExit);
	WaitForSingleObject(hAth, INFINITE);

    while (pConfig->pPortList) {
        Port *pThis = pConfig->pPortList;

        pConfig->pPortList = pConfig->pPortList->pNext;

        ClosePort (pThis);
        delete pThis;
    }

    while (pConfig->pSecurityList) {
        Security *pThis = pConfig->pSecurityList;
        pConfig->pSecurityList = pConfig->pSecurityList->pNext;

        if (pThis->cPin)
            BthRevokePIN (&pThis->bt);

        delete pThis;
    }

    delete pConfig;

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BTGW));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_BTGW, szWindowClass, MAX_LOADSTRING);

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

#ifdef WIN32_PLATFORM_PSPC
    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }
#endif // WIN32_PLATFORM_PSPC

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

#ifndef SHELL_AYGSHELL
    if (g_hWndCommandBar)
    {
        CommandBar_Show(g_hWndCommandBar, TRUE);
    }
#endif // !SHELL_AYGSHELL

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
	static HWND hlist = 0;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
#ifndef SHELL_AYGSHELL
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
#endif // !SHELL_AYGSHELL
#ifdef WIN32_PLATFORM_PSPC
                case IDM_OK:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;
#endif // WIN32_PLATFORM_PSPC
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
#ifndef SHELL_AYGSHELL
            g_hWndCommandBar = CommandBar_Create(g_hInst, hWnd, 1);
            CommandBar_InsertMenubar(g_hWndCommandBar, g_hInst, IDR_MENU, 0);
            CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            SHMENUBARINFO mbi;

            memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            if (!SHCreateMenuBar(&mbi)) 
            {
                g_hWndMenuBar = NULL;
            }
            else
            {
                g_hWndMenuBar = mbi.hwndMB;
            }

            // Initialize the shell activate info structure
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
#endif // SHELL_AYGSHELL
			RECT r;
//			GetWindowRect(hWnd, &r);
			GetClientRect(hWnd, &r);
//			r.top +=  GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYBORDER); //GetSystemMetrics(SM_CYMENU) +
			r.top +=  GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYBORDER); //GetSystemMetrics(SM_CYMENU) +
//			r.bottom +=  GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYBORDER); //GetSystemMetrics(SM_CYMENU) +
			r.bottom +=  GetSystemMetrics(SM_CXVSCROLL);
//			r.left += GetSystemMetrics(SM_CXBORDER);

			g_hList = hlist = CreateWindow(L"Listbox", 0, (LBS_STANDARD & ~LBS_SORT) | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP,
								 r.left, r.top, r.right - r.left/* - GetSystemMetrics(SM_CXBORDER)*/,
								 r.bottom - r.top/* - GetSystemMetrics(SM_CYBORDER)*/,
								 hWnd,(HMENU)0, g_hInst, 0);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
#ifndef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndCommandBar);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndMenuBar);
#endif // SHELL_AYGSHELL
            PostQuitMessage(0);
            break;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
			if(hlist)
				DestroyWindow(hlist);

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
#ifndef SHELL_AYGSHELL
            RECT rectChild, rectParent;
            int DlgWidth, DlgHeight;	// dialog width and height in pixel units
            int NewPosX, NewPosY;

            // trying to center the About dialog
            if (GetWindowRect(hDlg, &rectChild)) 
            {
                GetClientRect(GetParent(hDlg), &rectParent);
                DlgWidth	= rectChild.right - rectChild.left;
                DlgHeight	= rectChild.bottom - rectChild.top ;
                NewPosX		= (rectParent.right - rectParent.left - DlgWidth) / 2;
                NewPosY		= (rectParent.bottom - rectParent.top - DlgHeight) / 2;
				
                // if the About box is larger than the physical screen 
                if (NewPosX < 0) NewPosX = 0;
                if (NewPosY < 0) NewPosY = 0;
                SetWindowPos(hDlg, 0, NewPosX, NewPosY,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
#endif // SHELL_AYGSHELL

            return (INT_PTR)TRUE;

        case WM_COMMAND:
#ifndef SHELL_AYGSHELL
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            if (LOWORD(wParam) == IDOK)
#endif
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}
