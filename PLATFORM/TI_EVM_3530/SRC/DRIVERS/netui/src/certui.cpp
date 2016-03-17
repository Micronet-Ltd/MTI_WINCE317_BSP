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
// certui.cpp : Defines the entry point for the application.
//

#include "windows.h"
#include "windowsx.h"
#include <commctrl.h>
#include "wcommctl.h"
#include <wincrypt.h>
#include <certui.h>
#include <certuip.h>
#include <winscard.h>
#include <netuip.h>
#include "resource.h"

#define PC_NUM_COLUMNS	2
#define ARRAYSIZEOF(x) (sizeof(x)/sizeof(x[0]))

static HINSTANCE g_hCrypt32;
static HINSTANCE g_hCoredll;
static LONG g_refCount;

PFNCRYPTENUMPROVIDERSW pCryptEnumProvidersW;
PFNCRYPTACQUIRECONTEXTW pCryptAcquireContextW;
PFNCRYPTRELEASECONTEXT pCryptReleaseContext;
PFNCRYPTGETUSERKEY pCryptGetUserKey;
PFNCRYPTGETKEYPARAM pCryptGetKeyParam;
PFNCRYPTDESTROYKEY pCryptDestroyKey;
PFNCRYPTGETPROVPARAM pCryptGetProvParam;
PFNCERTCREATECERTIFICATECONTEXT pCertCreateCertificateContext;
PFNCERTFREECERTIFICATECONTEXT pCertFreeCertificateContext;
PFNCERTDUPLICATECERTIFICATECONTEXT pCertDuplicateCertificateContext;
PFNCERTOPENSTORE pCertOpenStore;
PFNCERTCLOSESTORE pCertCloseStore;
PFNCERTENUMCERTIFICATESINSTORE pCertEnumCertificatesInStore;
PFNCERTFINDCERTIFICATEINSTORE pCertFindCertificateInStore;
PFNCERTGETNAMESTRINGW pCertGetNameStringW;
PFNCERTGETCERTIFICATECONTEXTPROPERTY pCertGetCertificateContextProperty;
PFNCERTSETCERTIFICATECONTEXTPROPERTY pCertSetCertificateContextProperty;
PFNCERTADDCERTIFICATECONTEXTTOSTORE pCertAddCertificateContextToStore;
PFNCERTADDENCODEDCERTIFICATETOSTORE pCertAddEncodedCertificateToStore;
PFNCERTCOMPARECERTIFICATENAME pCertCompareCertificateName;
PFNCERTNAMETOSTRW pCertNameToStrW;
PFNCERTGETENHANCEDKEYUSAGE pCertGetEnhancedKeyUsage;
PFNCRYPTFINDOIDINFO pCryptFindOIDInfo;


BOOL LoadCAPI2(void)
{
	if (InterlockedIncrement(&g_refCount) != 1)
		return g_hCrypt32 != 0;

	DEBUGCHK(g_hCrypt32 == NULL);
	g_hCoredll = GetModuleHandleW(L"coredll.dll");
	g_hCrypt32 = LoadLibraryW(L"crypt32.dll");
	
	if (g_hCoredll && g_hCrypt32)
	{
		pCryptEnumProvidersW = (PFNCRYPTENUMPROVIDERSW) GetProcAddressW(g_hCoredll, L"CryptEnumProvidersW");
		pCryptGetProvParam = (PFNCRYPTGETPROVPARAM) GetProcAddressW(g_hCoredll, L"CryptGetProvParam");
		pCryptAcquireContextW = (PFNCRYPTACQUIRECONTEXTW) GetProcAddressW(g_hCoredll, L"CryptAcquireContextW");
		pCryptReleaseContext = (PFNCRYPTRELEASECONTEXT) GetProcAddressW(g_hCoredll, L"CryptReleaseContext");
		pCryptGetUserKey = (PFNCRYPTGETUSERKEY) GetProcAddressW(g_hCoredll, L"CryptGetUserKey");
		pCryptGetKeyParam = (PFNCRYPTGETKEYPARAM) GetProcAddressW(g_hCoredll, L"CryptGetKeyParam");
		pCryptDestroyKey = (PFNCRYPTDESTROYKEY) GetProcAddressW(g_hCoredll, L"CryptDestroyKey");
		
		pCertCreateCertificateContext = (PFNCERTCREATECERTIFICATECONTEXT) GetProcAddressW(g_hCrypt32, L"CertCreateCertificateContext");
		pCertFreeCertificateContext = (PFNCERTFREECERTIFICATECONTEXT)GetProcAddressW(g_hCrypt32, L"CertFreeCertificateContext");
		pCertDuplicateCertificateContext = (PFNCERTDUPLICATECERTIFICATECONTEXT)GetProcAddressW(g_hCrypt32, L"CertDuplicateCertificateContext");
		pCertOpenStore = (PFNCERTOPENSTORE)GetProcAddressW(g_hCrypt32, L"CertOpenStore");
		pCertCloseStore = (PFNCERTCLOSESTORE)GetProcAddressW(g_hCrypt32, L"CertCloseStore");
		pCertEnumCertificatesInStore = (PFNCERTENUMCERTIFICATESINSTORE)GetProcAddressW(g_hCrypt32, L"CertEnumCertificatesInStore");
		pCertFindCertificateInStore = (PFNCERTFINDCERTIFICATEINSTORE)GetProcAddressW(g_hCrypt32, L"CertFindCertificateInStore");
		pCertGetNameStringW = (PFNCERTGETNAMESTRINGW)GetProcAddressW(g_hCrypt32, L"CertGetNameStringW");
		pCertGetCertificateContextProperty = (PFNCERTGETCERTIFICATECONTEXTPROPERTY)GetProcAddressW(g_hCrypt32, L"CertGetCertificateContextProperty");
		pCertSetCertificateContextProperty = (PFNCERTSETCERTIFICATECONTEXTPROPERTY)GetProcAddressW(g_hCrypt32, L"CertSetCertificateContextProperty");
		pCertAddEncodedCertificateToStore = (PFNCERTADDENCODEDCERTIFICATETOSTORE)GetProcAddressW(g_hCrypt32, L"CertAddEncodedCertificateToStore");
		pCertAddCertificateContextToStore = (PFNCERTADDCERTIFICATECONTEXTTOSTORE) GetProcAddressW(g_hCrypt32, L"CertAddCertificateContextToStore");
		pCertCompareCertificateName = (PFNCERTCOMPARECERTIFICATENAME)GetProcAddressW(g_hCrypt32, L"CertCompareCertificateName");
		pCertNameToStrW = (PFNCERTNAMETOSTRW)GetProcAddressW(g_hCrypt32, L"CertNameToStrW");
		pCertGetEnhancedKeyUsage = (PFNCERTGETENHANCEDKEYUSAGE)GetProcAddressW(g_hCrypt32, L"CertGetEnhancedKeyUsage");
		pCryptFindOIDInfo = (PFNCRYPTFINDOIDINFO)GetProcAddressW(g_hCrypt32, L"CryptFindOIDInfo");
		
		if ( !pCryptAcquireContextW || !pCryptReleaseContext || !pCryptGetUserKey
			|| !pCryptEnumProvidersW || !pCryptGetProvParam
			|| !pCryptGetKeyParam || !pCryptDestroyKey
			|| !pCertCreateCertificateContext
			|| !pCertFreeCertificateContext
			|| !pCertDuplicateCertificateContext
			|| !pCertOpenStore || !pCertCloseStore
			|| !pCertEnumCertificatesInStore
			|| !pCertGetNameStringW 
			|| !pCertNameToStrW
			|| !pCertGetCertificateContextProperty  || !pCertSetCertificateContextProperty
			|| !pCertAddCertificateContextToStore
			|| !pCertCompareCertificateName || !pCertNameToStrW
			|| !pCertGetEnhancedKeyUsage || !pCryptFindOIDInfo
		    )
		{
			FreeLibrary(g_hCrypt32);
			g_hCrypt32 = 0;
		}
	}
	return (g_hCrypt32 != 0);
}

// should be called once for every instance of LoadCAPI2 (whether or not it succeeds)
void
FreeCAPI2(void)
{
	LONG refCount;
	if ((refCount = InterlockedDecrement(&g_refCount)) != 0)
		return;

	DEBUGCHK(refCount >= 0);
	if (g_hCrypt32)
	{
		FreeLibrary(g_hCrypt32);
		pCertFreeCertificateContext =  NULL;
		pCertDuplicateCertificateContext =  NULL;
		pCertGetNameStringW =  NULL;
		pCertGetCertificateContextProperty =  NULL;
		g_hCrypt32 = 0;
	}
}
BOOL
CALLBACK
PickCertDialogProc(
	HWND hwndDlg, 
	UINT uMsg, 
	WPARAM wParam, 
	LPARAM lParam);


/*
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	int fRet;
	PCCERT_CONTEXT pCert;
	HCERTSTORE	hStore = CertOpenSystemStore(NULL, "MY");
	InitCommonControls();
	v_hInst = hInstance;

	if (hStore)
	{
	
 	// TODO: Place code here.
	pCert = (PCCERT_CONTEXT)DialogBoxParam(hInstance, (const char *)IDD_PICKCERT , NULL, PickCertDialogProc,  (LONG) hStore);
	if (pCert && pCert != (PCCERT_CONTEXT)-1)
		MessageBox(NULL, "Picked Cert", "Pick Cert Dialog", MB_OK);
	else
		MessageBox(NULL, "error picking cert", "Pick Cert Dialog", MB_OK);
	}

	return 0;
}
*/
// InitListViewColumns - adds columns to a list-view control. 
// Returns TRUE if successful, or FALSE otherwise. 
// hWndListView - handle to the list-view control. 
static
BOOL InitListViewColumns(HWND hWndListView) 
{ 
    TCHAR szText[256];     // temporary buffer 
    LVCOLUMN lvc;
    RECT rect;
    int iCol;
	int Labels[PC_NUM_COLUMNS] = {IDS_FRIENDLY_NAME, IDS_ISSUER};

 #if 0
	HBITMAP hBmp;
	HICON hIcon;
	HIMAGELIST hImg;
	// create image list
    //hImg = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
    //    GetSystemMetrics(SM_CYSMICON), 0, 1, 1);
	 hImg = ImageList_Create(16, 16, 0, 1, 1);


	hBmp = LoadBitmap(v_hInst, MAKEINTRESOURCE(IDB_PKEY));
	hIcon = LoadIcon(v_hInst, MAKEINTRESOURCE(IDI_ICON1));

	ImageList_AddIcon(hImg, hIcon);
	// Assign the image lists to the list-view control. 
    ListView_SetImageList(hWndListView, hImg, LVSIL_SMALL); 
	// DeleteObject(hBmp);
#endif

    // Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text, and subitem
    // members of the structure are valid. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    GetClientRect(hWndListView, &rect);
    
    // Add the columns. 
    for (iCol = 0; iCol < PC_NUM_COLUMNS; iCol++) 
	{ 
        lvc.iSubItem = iCol;
        lvc.cx = (rect.right - rect.left)/PC_NUM_COLUMNS;           // width of column in pixels
        lvc.fmt = LVCFMT_LEFT;  // left-aligned column
		lvc.pszText = szText;
		LoadString(v_hInst, Labels[iCol], 
                szText, ARRAYSIZEOF(szText));
        if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1) 
            return FALSE; 
    } 
    return TRUE; 
} 

#define CRYPTUI_MAX_STRING_SIZE	256
static
BOOL PopulateListView(HWND hCtl, PNETUI_PICKCERT pPickCert)
{
	PCCERT_CONTEXT pCert = NULL;
	LVITEM lvI;
	int index = 0;
	TCHAR rgch[CRYPTUI_MAX_STRING_SIZE];

	// Initialize LVITEM members that are common to all items. 
	lvI.state = 0; 
	lvI.stateMask = 0; 

	for (index = 0; index < (int) pPickCert->nCerts; index++)
	{
	
        pCert = pPickCert->ppCerts[index];
	// Initialize LVITEM members that are different for each item. 
   		lvI.iItem = index;
		lvI.mask = LVIF_PARAM | LVIF_STATE ; //LVIF_IMAGE 
		lvI.iImage = 0;
		lvI.iSubItem = 0;
		lvI.lParam = index;
		//lvI.pszText = LPSTR_TEXTCALLBACK; // sends an LVN_GETDISPINFO
											   // message.									  
		
		if(ListView_InsertItem(hCtl, &lvI) == -1)
			break;

		lvI.mask = LVIF_TEXT;

		pCertGetNameStringW(
				pCert,
                CERT_NAME_FRIENDLY_DISPLAY_TYPE,
                0,//CERT_NAME_ISSUER_FLAG,
                NULL,
                rgch,
                ARRAYSIZEOF(rgch));

		lvI.iSubItem = 0;	// subject name
		lvI.pszText = rgch;

		ListView_SetItemText(hCtl, index, 0, rgch);
		
		pCertGetNameStringW(
				pCert,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                CERT_NAME_ISSUER_FLAG,
                NULL,
                rgch,
                ARRAYSIZEOF(rgch));

		lvI.iSubItem = 1;   // issuer name
		
		ListView_SetItemText(hCtl, index, 1, rgch);

	}
	if (pPickCert->iSelected >= 0 && pPickCert->iSelected < pPickCert->nCerts)
	{
	    // mark one element as selected
	    ListView_SetItemState(hCtl, pPickCert->iSelected, LVIS_SELECTED, LVIS_SELECTED);
	}
	return TRUE;
}


BOOL
CALLBACK
PickCertDialogProc(
	HWND hDlg, 
	UINT uMsg, 
	WPARAM wParam, 
	LPARAM lParam)
{
	BOOL fRet = TRUE;
	PNETUI_PICKCERT pPickCert ;
	//HCERTSTORE hStore= (HCERTSTORE) lParam;
	LONG wCmdId;
	LONG wCmdCbn;
	LVITEM lvI;

	HWND hCtlList = GetDlgItem(hDlg, IDC_CERTLISTVIEW);
	switch (uMsg) {
    case WM_NOTIFY :
        break;
    case WM_INITDIALOG :
        {
            pPickCert = (PNETUI_PICKCERT) lParam;
		    SetWindowLong (hDlg, DWL_USER, (LONG)pPickCert);
		    ListView_SetExtendedListViewStyle(hCtlList,  LVS_EX_FULLROWSELECT );

		    if (!pPickCert)
		        fRet = FALSE;
		    else
           	    fRet = InitListViewColumns(hCtlList);
			if (fRet)
			{
				PopulateListView(hCtlList, pPickCert);
			}
        }
        break;

    case WM_COMMAND :
        wCmdId  = LOWORD(wParam);
        wCmdCbn = HIWORD(wParam);

        switch ( wCmdId ) {
        
        case IDCANCEL:
            EndDialog(hDlg,FALSE);
            break;

        case IDOK:
			{ 
				lvI.iItem = -1;
				lvI.lParam = 0;
				lvI.iSubItem = 0;
				lvI.mask = LVIF_PARAM;
				if (hCtlList)
					lvI.iItem = ListView_GetNextItem(hCtlList, -1, LVNI_SELECTED);
				if (lvI.iItem != -1 )
				{
					ListView_GetItem(hCtlList, &lvI);
				}
				pPickCert = (PNETUI_PICKCERT)GetWindowLong(hDlg, DWL_USER);
				if (pPickCert)
				    pPickCert->iSelected = lvI.lParam;
				
				EndDialog(hDlg, TRUE);

			break;
			}
        case IDC_BTN_CERTVIEW:
        {
        	// Details button
		lvI.iItem = -1;
		lvI.lParam = 0;
		lvI.iSubItem = 0;
		lvI.mask = LVIF_PARAM;
		lvI.iItem = ListView_GetNextItem(hCtlList, -1, LVNI_SELECTED);
		ListView_GetItem(hCtlList, &lvI);
		pPickCert = (PNETUI_PICKCERT)GetWindowLong(hDlg, DWL_USER);
		if (pPickCert  && lvI.iItem != -1 && pPickCert->ppCerts[lvI.lParam])
		{
			ShowCertificate(hDlg, pPickCert->ppCerts[lvI.lParam]);
		}
		break;
	
        }
		default:
			fRet = FALSE;
			break;
		}
	default:
		fRet = FALSE;
		break;
	}
	return fRet;
}

BOOL
PickCertificate (HWND hParent, PNETUI_PICKCERT pPickCert)
{
	HRSRC hRsrc;
	HGLOBAL hGlob;
   	 int iRet = -1;
    	HCURSOR hCur;
	
	DEBUGMSG (ZONE_USERPWD, (TEXT("PickCertificate: Enter\r\n")));

	hRsrc  = FindResource (v_hInst, MAKEINTRESOURCE(IDD_PICKCERT), RT_DIALOG);
    	if (hRsrc == NULL)
       	return FALSE;
	hGlob = LoadResource (v_hInst, hRsrc);
    	if (hGlob == NULL) 
        	return FALSE;
	// We need the common control functions
	if (!ValidCommCtrlFunc(InitCommonControls)) {
		DEBUGMSG (ZONE_ERROR|ZONE_USERPWD,
				  (TEXT("!PickCertificate: Common Controls not loaded\r\n")));
		return FALSE;
	}


	// Initialize the common controls. We need the ListView window class
	CallCommCtrlFunc(InitCommonControls)();

    	if (LoadCAPI2())
    	{

	    	hCur = SetCursor(NULL);
		iRet = DialogBoxParam (v_hInst, MAKEINTRESOURCE(IDD_PICKCERT),
							   hParent, PickCertDialogProc, (LONG)pPickCert);
	    	SetCursor(hCur);
    	}
	else
	{
        	DEBUGMSG(ZONE_ERROR, (TEXT("PickCertificate: Cannot load Crypt32.dll\r\n")));

	}
    	FreeCAPI2();
    	if (iRet == -1) {
        	DEBUGMSG(ZONE_ERROR, (TEXT("PickCertificate: DialogBoxParam returned err %u\r\n"),
                              GetLastError()));
        return FALSE;
    }
    else
        return TRUE;
}

