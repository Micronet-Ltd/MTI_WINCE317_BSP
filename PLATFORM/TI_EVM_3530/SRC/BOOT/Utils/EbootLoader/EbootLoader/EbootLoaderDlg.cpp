// EbootLoaderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EbootLoader.h"
#include "EbootLoaderDlg.h"

#include "SerLoaderApi.h"
#include <strsafe.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
IMPLEMENT_DYNAMIC(CEbootLoaderDlg, CDialog)

HANDLE			g_hPort = (void*)-1;
// CAboutDlg dialog used for App About

//char catalog_number[RECORD_LENGTH] = "MCE504-0001-001";
//char identification[RECORD_LENGTH] = "4-C-0-00-00-00-X0-000-30678";
//char ser[RECORD_LENGTH]			= "499601";

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CEbootLoaderDlg dialog

CEbootLoaderDlg::CEbootLoaderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEbootLoaderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CEbootLoaderDlg::~CEbootLoaderDlg()
{
}
void CEbootLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC1, m_Status);
	DDX_Control(pDX, IDC_CATALOG, m_catalog);
	DDX_Control(pDX, IDC_DEVID, m_devid);
	DDX_Control(pDX, IDC_SER, m_ser);
	DDX_Control(pDX, IDC_RMAC, m_mac1);
	DDX_Control(pDX, IDC_RMAC2, m_mac2);
	DDX_Control(pDX, IDC_RMAC3, m_mac3);
	DDX_Control(pDX, IDC_RMAC4, m_mac4);
	DDX_Control(pDX, IDC_RMAC5, m_mac5);
	DDX_Control(pDX, IDC_RMAC6, m_mac6);
	DDX_Control(pDX, IDC_CATALOG2, m_catalog2);
	DDX_Control(pDX, IDC_DEVID2, m_devid2);
	DDX_Control(pDX, IDC_SER2, m_ser2);
	DDX_Control(pDX, IDC_IMG, m_ImgVer);
	DDX_Control(pDX, IDC_EBOOT, m_EbootVer);
	DDX_Control(pDX, IDC_XLDR, m_XldrVer);
	DDX_Control(pDX, IDC_HW, m_HwVer);
	DDX_Control(pDX, IDC_SWR, m_SwRights);
	DDX_Control(pDX, IDC_COMBO1, m_Com);
	DDX_Control(pDX, IDC_COMBO2, m_NetType);
	DDX_Control(pDX, IDC_COMBO3, m_FileType);
	DDX_Control(pDX, IDC_COMBO4, m_ErType);
	DDX_Control(pDX, IDC_COMBO5, m_MaxFreq);
	DDX_Control(pDX, IDC_BUTTON20, m_Erase);
	DDX_Control(pDX, IDC_NANDINFO, m_NandInfo);
}

BEGIN_MESSAGE_MAP(CEbootLoaderDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CEbootLoaderDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CEbootLoaderDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CEbootLoaderDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CEbootLoaderDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CEbootLoaderDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CEbootLoaderDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CEbootLoaderDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CEbootLoaderDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CEbootLoaderDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CEbootLoaderDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CEbootLoaderDlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, &CEbootLoaderDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &CEbootLoaderDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &CEbootLoaderDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, &CEbootLoaderDlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON16, &CEbootLoaderDlg::OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON17, &CEbootLoaderDlg::OnBnClickedButton17)
	ON_BN_CLICKED(IDC_BUTTON18, &CEbootLoaderDlg::OnBnClickedButton18)
	ON_BN_CLICKED(IDC_BUTTON19, &CEbootLoaderDlg::OnBnClickedButton19)
	ON_BN_CLICKED(IDC_BUTTON20, &CEbootLoaderDlg::OnBnClickedButton20)
	ON_BN_CLICKED(IDC_BUTTON22, &CEbootLoaderDlg::OnBnClickedButton22)
	ON_BN_CLICKED(IDC_BUTTON23, &CEbootLoaderDlg::OnBnClickedButton23)
END_MESSAGE_MAP()


// CEbootLoaderDlg message handlers

BOOL CEbootLoaderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	int i;
	CString	str, str1;
	int index; 
	for( i = 1; i < 100; ++i )
	{
		str.Format( _T("%d"), i );
		index = m_Com.AddString(str);
		m_Com.SetItemData(index, i);
	}

	m_Com.SetCurSel(2);//for me
	
	index = m_NetType.AddString(_T("Ethernet"));
	m_NetType.SetItemData( index, 1 );
	m_NetType.SetCurSel( index );
	index = m_NetType.AddString(_T("WiFi"));
	m_NetType.SetItemData( index, 2 );
	index = m_NetType.AddString(_T("RNDIS"));
	m_NetType.SetItemData( index, 3 );

	index = m_FileType.AddString(_T("xldr"));
	m_FileType.SetItemData( index, sw_type_xldr );
	index = m_FileType.AddString(_T("eboot"));
	m_FileType.SetItemData( index, sw_type_eboot );
	index = m_FileType.AddString(_T("mnfr"));
	m_FileType.SetItemData( index, sw_type_mnfr );
	index = m_FileType.AddString(_T("logo"));
	m_FileType.SetItemData( index, sw_type_logo );
	m_FileType.SetCurSel( index );
	index = m_FileType.AddString(_T("nk"));
	m_FileType.SetItemData( index, sw_type_nk );

	index = m_ErType.AddString(L"0");
	m_ErType.SetItemData(index, 0);
	index = m_ErType.AddString(L"1");
	m_ErType.SetItemData(index, 1);
	m_ErType.SetCurSel( index );
//	str.Empty();
//	for( i = 1; i <= SW_RIGHTS_BLOCK_SIZE; ++i )
//	{
//		str1.Format(_T("%02x"), i);
//		str.Append(str1);
//	}
//	m_SwRights.SetWindowText( str.GetBuffer(0) );

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEbootLoaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEbootLoaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEbootLoaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CEbootLoaderDlg::OnBnClickedButton1()
{
	CString str(_T("COM3"));
	DWORD num = 3;

	int index = m_Com.GetCurSel();
	if( -1 != index )
	{
		num = (DWORD)m_Com.GetItemData(index);

//		if( num < 10 )
//			str.Format(_T("COM%d:"), num);
//		else
//			str.Format(_T("\\\\.\\COM%d"), num);
		str.Format(_T("COM%d"), num);
	}

	if( LdrOpen( num, &g_hPort ) )
		str.Append(_T(" - Cannot open"));
	m_Status.SetWindowText( str.GetBuffer(0) );
}

void CEbootLoaderDlg::OnBnClickedButton3()
{
	//Prot.Close( g_hPort );
	LdrClose( g_hPort );
	g_hPort = (void*)-1;
}

void CEbootLoaderDlg::OnBnClickedButton2()
{
	//LdrGetParamBlock
	m_Status.SetWindowText( _T("") );
	m_catalog.SetWindowText( _T("") );
	m_devid.SetWindowText( _T("") );
	m_ser.SetWindowText( _T("") );
	m_catalog2.SetWindowText( _T("") );
	m_devid2.SetWindowText( _T("") );
	m_ser2.SetWindowText( _T("") );

	manufacture_block manuf_block = {0};
	if( LdrGetParamBlock(g_hPort, &manuf_block) )
	{
		m_Status.SetWindowText( _T("GetParamBlock failed") );
		return;
	}

	m_Status.SetWindowText( _T("OK") );

	TCHAR str[RECORD_LENGTH+1] = {0};
	StringCbPrintf( str, sizeof(str),_T("%S"), manuf_block.PCB_catalog_number );
	m_catalog.SetWindowText(str);
	StringCbPrintf( str, sizeof(str),_T("%S"), manuf_block.PCB_identification );
	m_devid.SetWindowText(str);
	StringCbPrintf( str, sizeof(str),_T("%S"), manuf_block.PCB_serial_number );
	m_ser.SetWindowText(str);

	StringCbPrintf( str, sizeof(str),_T("%S"), manuf_block.device_catalog_number );
	m_catalog2.SetWindowText(str);
	StringCbPrintf( str, sizeof(str),_T("%S"), manuf_block.device_identification );
	m_devid2.SetWindowText(str);
	StringCbPrintf( str, sizeof(str),_T("%S"), manuf_block.device_serial_number );
	m_ser2.SetWindowText(str);

	CFile f(_T("mnfr.bin"), CFile::modeCreate | CFile::modeWrite);
	f.Write((void*)&manuf_block, sizeof(manuf_block));
	f.Close();
}

void CEbootLoaderDlg::OnBnClickedButton4()
{
	m_Status.SetWindowText( _T("") );
	manufacture_block manuf_block = {0};

	TCHAR	str[RECORD_LENGTH+1];
	size_t	NumOfChars;
	m_catalog.GetWindowText(str, RECORD_LENGTH);
	wcstombs_s( &NumOfChars, (char*)manuf_block.PCB_catalog_number, RECORD_LENGTH, str, (RECORD_LENGTH + 1) * sizeof(TCHAR)  );
	m_devid.GetWindowText(str, RECORD_LENGTH);
	wcstombs_s( &NumOfChars, (char*)manuf_block.PCB_identification, RECORD_LENGTH, str, (RECORD_LENGTH + 1) * sizeof(TCHAR)  );
	m_ser.GetWindowText(str, RECORD_LENGTH);
	wcstombs_s( &NumOfChars, (char*)manuf_block.PCB_serial_number, RECORD_LENGTH, str, (RECORD_LENGTH + 1) * sizeof(TCHAR)  );

	m_catalog2.GetWindowText(str, RECORD_LENGTH);
	wcstombs_s( &NumOfChars, (char*)manuf_block.device_catalog_number, RECORD_LENGTH, str, (RECORD_LENGTH + 1) * sizeof(TCHAR)  );
	m_devid2.GetWindowText(str, RECORD_LENGTH);
	wcstombs_s( &NumOfChars, (char*)manuf_block.device_identification, RECORD_LENGTH, str, (RECORD_LENGTH + 1) * sizeof(TCHAR)  );
	m_ser2.GetWindowText(str, RECORD_LENGTH);
	wcstombs_s( &NumOfChars, (char*)manuf_block.device_serial_number, RECORD_LENGTH, str, (RECORD_LENGTH + 1) * sizeof(TCHAR)  );

	if( LdrSetParamBlock(g_hPort, &manuf_block) )
	{
		m_Status.SetWindowText( _T("SetParamBlock failed") );
	}
	else
		m_Status.SetWindowText( _T("OK") );
}

void CEbootLoaderDlg::OnBnClickedButton5()
{
	unsigned short mac[3] = {0};
	m_Status.SetWindowText( _T("") );

	DWORD err;
	DWORD who = 0;
	int index = m_NetType.GetCurSel();
	if( -1 != index )
	{
		who = m_NetType.GetItemData(index);
	}
	
	if( 1 == who )
	{
		if( err = LdrGetEtherMac(g_hPort, mac) )
			m_Status.SetWindowText( _T("GetEtherMac failed") );
	}
	else if(2  == who)
	{
		if( err = LdrGetWiFiMac(g_hPort, mac) )
			m_Status.SetWindowText( _T("GetWiFiMac failed") );
	}
	else if(3  == who)
	{
		if( err = LdrGetRndisMac(g_hPort, mac) )
			m_Status.SetWindowText( _T("GetRndisMac failed") );
	}
	else
		err = -1;
		
	if(!err)
		m_Status.SetWindowText( _T("OK") );
	
	CString str;
//	str.Format( _T("%02x:%02x:%02x:%02x:%02x:%02x"),	mac[0] & 0xFF, mac[0]>>8, mac[1] & 0xFF,
//														mac[1] >> 8, mac[2] & 0xFF, mac[2] >> 8 ); 

	str.Format( _T("%02x"), mac[0] & 0xFF );
	m_mac1.SetWindowText(str.GetBuffer(0));
	str.Format( _T("%02x"), mac[0] >> 8 );
	m_mac2.SetWindowText(str.GetBuffer(0));
	str.Format( _T("%02x"), mac[1] & 0xFF );
	m_mac3.SetWindowText(str.GetBuffer(0));
	str.Format( _T("%02x"), mac[1] >> 8 );
	m_mac4.SetWindowText(str.GetBuffer(0));
	str.Format( _T("%02x"), mac[2] & 0xFF );
	m_mac5.SetWindowText(str.GetBuffer(0));
	str.Format( _T("%02x"), mac[2] >> 8 );
	m_mac6.SetWindowText(str.GetBuffer(0));
}


void CEbootLoaderDlg::OnBnClickedButton6()
{
	unsigned short mac[3] = {0};
	TCHAR str[3] = {0};
	
	m_Status.SetWindowText( _T("") );

	TCHAR* ptr;
	
	m_mac1.GetWindowText(str, 3);
	mac[0] = (UCHAR)_tcstoul(str, &ptr, 16);
	m_mac2.GetWindowText(str, 3);
	mac[0] += ((UCHAR)_tcstoul(str, &ptr, 16) << 8 );

	m_mac3.GetWindowText(str, 3);
	mac[1] = (UCHAR)_tcstoul(str, &ptr, 16);
	m_mac4.GetWindowText(str, 3);
	mac[1] += ((UCHAR)_tcstoul(str, &ptr, 16) << 8);
	
	m_mac5.GetWindowText(str, 3);
	mac[2] = (UCHAR)_tcstoul(str, &ptr, 16);
	m_mac6.GetWindowText(str, 3);
	mac[2] += ((UCHAR)_tcstoul(str, &ptr, 16) << 8);

	DWORD	err = 0;
	DWORD	who = 0;
	int index = m_NetType.GetCurSel();
	if( -1 != index )
	{
		who = m_NetType.GetItemData(index);
	}
	
	if( 1 == who )
	{
		if( err = LdrSetEtherMac(g_hPort, mac) )
			m_Status.SetWindowText( _T("CMD_SET_ETHER_MAC failed") );
	}
	else if(2  == who)
	{
		if( err = LdrSetWiFiMac(g_hPort, mac) )
			m_Status.SetWindowText( _T("CMD_SET_WIFI_MAC failed") );
	}
	else if(3  == who)
	{
		if( err = LdrSetRndisMac(g_hPort, mac) )
			m_Status.SetWindowText( _T("CMD_SET_RNDIS_MAC failed") );
	}
	else
	{
		m_Status.SetWindowText( _T("Error!") );
		err = -1;
	}

	if( !err)
		m_Status.SetWindowText( _T("OK") );

}

void CEbootLoaderDlg::OnBnClickedButton7()
{
	m_Status.SetWindowText( _T("") );
	if( LdrCleanRegistry(g_hPort) )
		m_Status.SetWindowText( _T("CMD_ERASE_REGISTRY failed") );
	else
		m_Status.SetWindowText( _T("OK") );
}

void CEbootLoaderDlg::OnBnClickedButton8()
{
	m_Status.SetWindowText( _T("") );
	if(LdrExitLoaderMode(g_hPort) )
		m_Status.SetWindowText( _T("CMD_IMG_FLASH_LAUNCH failed") );
	else
		m_Status.SetWindowText( _T("OK") );
}

void CEbootLoaderDlg::OnBnClickedButton9()
{
	m_Status.SetWindowText( _T("") );
	DWORD OldTimeout = LdrGetWaitTimeout(g_hPort);

	LdrSetWaitTimeout(g_hPort,60000);

	if(LdrFormatFlash(g_hPort) )
		m_Status.SetWindowText( _T("CMD_FORMAT_FLASH failed") );
	else
		m_Status.SetWindowText( _T("OK") );

	LdrSetWaitTimeout(g_hPort,OldTimeout);
}

void CEbootLoaderDlg::OnBnClickedButton10()
{
	m_NandInfo.SetWindowText( _T("") );
	m_Status.SetWindowText( _T("") );
	m_EbootVer.SetWindowText( _T("") );
	m_HwVer.SetWindowText( _T("") );
	m_XldrVer.SetWindowText( _T("") );
	m_ImgVer.SetWindowText( _T("") );

	VersionInfo version = {0};
	VersionInfoExt versionInfo = {0};

	HWVersions hwVersions	= {0};
	NAND_INFO Info = {0};
	// 0 = "ES1.0", 1 = "ES2.0", 2 = "ES2.1", 3 = "ES2.0, ID determined using CRC", 4 = "ES2.1, ID determined using CRC", 0xFF = UNKNOWN
	// 0 = "ES1.0" ,0x10 = "ES2.x", 0x40 = "ES3.0" ,0x50 = "ES3.1", 0xFF = "Unknown"

	CString str;

	DWORD	err(0);

	if( !(err = LdrGetNandInfo( g_hPort, &Info)))
	{
		str.Format( _T("0x%X - 0x%X - %u - %u - %u - %u"),	Info.manufacturerId, 
															Info.deviceId,
															Info.blocks,
															Info.sectorsPerBlock,
															Info.sectorSize,
															Info.wordData);
		m_NandInfo.SetWindowText( str.GetBuffer(0) );
	}
	if( !(err = LdrGetHWVersion(g_hPort, &hwVersions)))
	{
		str.Format( _T("%02x.%02x"), hwVersions.CPUVersion, hwVersions.CoChipVersion );
		m_HwVer.SetWindowText( str.GetBuffer(0) );
	}
//	err = LdrGetVersion( g_hPort, &version, VERSION_EBOOT );
	if( !(err = LdrGetVersionExt( g_hPort, &versionInfo, sizeof(versionInfo), VERSION_EBOOT )))
	{
		str.Format( _T("%u.%u.%u.%u\t\t\t%S"), versionInfo.Version.majorVersion, 
										versionInfo.Version.minorVersion,
										versionInfo.Version.incrementalVersion,
										versionInfo.BuildVersion,
										versionInfo.Version.buildDate			);
		m_EbootVer.SetWindowText( str.GetBuffer(0) );
	}
//	err = LdrGetVersion( g_hPort, &version, VERSION_XLDR );
	if( !(err = LdrGetVersionExt( g_hPort, &versionInfo, sizeof(versionInfo), VERSION_XLDR )))
	{
		str.Format( _T("%u.%u.%u.%u\t\t\t%S"), versionInfo.Version.majorVersion, 
										versionInfo.Version.minorVersion,
										versionInfo.Version.incrementalVersion,
										versionInfo.BuildVersion,
										versionInfo.Version.buildDate			);
		m_XldrVer.SetWindowText( str.GetBuffer(0) );
	}
//	err = LdrGetVersion( g_hPort, &version, VERSION_IMG );
	DWORD OldTimeout = LdrGetWaitTimeout(g_hPort);

	LdrSetWaitTimeout(g_hPort,60000);
	if( !(err = LdrGetVersionExt( g_hPort, &versionInfo, sizeof(versionInfo), VERSION_IMG )))
	{
//		str.Format( _T("%u.%u.%u %s\t\t%S"), versionInfo.majorVersion, 
//											versionInfo.minorVersion,
//											versionInfo.incrementalVersion,
//											versionInfo.buildDate			);
		str.Format( _T("%s\t\t%S"), 	versionInfo.Version.versionString, 
										versionInfo.Version.buildDate			);
		m_ImgVer.SetWindowText( str.GetBuffer(0) );
	}
	LdrSetWaitTimeout(g_hPort,OldTimeout);
}

void CEbootLoaderDlg::OnBnClickedButton11()
{
	UCHAR buff[SW_RIGHTS_BLOCK_SIZE] = {0};

	m_Status.SetWindowText( _T("") );

	if( !LdrGetSwRights(g_hPort, buff))
	{
 		CString str;
		TCHAR	st[3] = {0};

		for( int i = 0; i < SW_RIGHTS_BLOCK_SIZE; ++i )
		{
			StringCbPrintf( st, sizeof(st),_T("%02x"), buff[i]);
			str.Append(st);
		}
		m_SwRights.SetWindowText(str.GetBuffer(0));
	
		m_Status.SetWindowText( _T("OK") );
	}
	else
		m_Status.SetWindowText( _T("CMD_GET_SW_RIGHTS failed") );
}

void CEbootLoaderDlg::OnBnClickedButton12()
{
	UCHAR	buff[SW_RIGHTS_BLOCK_SIZE] = {0};
	TCHAR	Txt[SW_RIGHTS_BLOCK_SIZE * 2 + 1] = {0};

	m_Status.SetWindowText( _T("") );
	m_SwRights.GetWindowText( Txt, SW_RIGHTS_BLOCK_SIZE * 2 + 1 );

	TCHAR	Tmp[36] = {0};
	TCHAR*	ptr;
	for( int i = 0; i < SW_RIGHTS_BLOCK_SIZE; ++i )
	{
		_tcsncpy_s(Tmp, Txt + i * 2, 2);
		buff[i] = (UCHAR)_tcstoul(Tmp, &ptr, 16);
	}

	if( LdrSetSwRights(g_hPort, buff) )
		m_Status.SetWindowText( _T("CMD_SET_SW_RIGHTS failed") );
	else
		m_Status.SetWindowText( _T("OK") );
}

void CEbootLoaderDlg::OnBnClickedButton18()
{
	DWORD OldTimeout = LdrGetWaitTimeout(g_hPort);

	LdrSetWaitTimeout(g_hPort,60000);

	if( !LdrConnect( g_hPort, 0) )
	{
		m_Status.SetWindowText( _T("Connected") );
	}
	else
	{
		m_Status.SetWindowText( _T("Connection failed") );
	}
	LdrSetWaitTimeout(g_hPort,OldTimeout);
}

void CEbootLoaderDlg::OnBnClickedButton13()
{
	DWORD OldTimeout = LdrGetWaitTimeout(g_hPort);

	LdrSetWaitTimeout(g_hPort,20000);

	if( !LdrConnect( g_hPort, 1) )
	{
		LdrSetWaitTimeout(g_hPort,OldTimeout);
		ram_tst_result tr;
		DWORD err;
		if(err = LdrGetRamTestResult( g_hPort, &tr ))
			m_Status.SetWindowText( _T("LdrGetRamTestResult failed") );
		else 
			m_Status.SetWindowText( _T("Connected") );

	}
	else
	{
		m_Status.SetWindowText( _T("Connection failed") );
	}
	LdrSetWaitTimeout(g_hPort,OldTimeout);
}

void CEbootLoaderDlg::OnBnClickedButton14()
{
	if( LdrReset(g_hPort))
		m_Status.SetWindowText( _T("LdrReset failed") );
	else
		m_Status.SetWindowText( _T("OK") );
}

void CEbootLoaderDlg::OnBnClickedButton15()
{
	ram_tst_result tr;
	if( LdrGetRamTestResult( g_hPort, &tr ))
		m_Status.SetWindowText( _T("LdrGetRamTestResult failed") );
	else
	{
		if( tr.err )
		{
			CString str;
			str.Format( _T("RAM error: address1 0x%x, address2 0x%x, val1 0x%x, val2 0x%x"),
						tr.address, tr.address_dest, tr.value_written, tr.value_read );					
			m_Status.SetWindowText( str.GetBuffer(0) );
		}
		else
			m_Status.SetWindowText( _T("RAM test is OK") );
	}
}

void CEbootLoaderDlg::OnBnClickedButton16()
{
	m_Status.SetWindowText( _T("") );
	if(LdrFormatDataPartition(g_hPort) )
		m_Status.SetWindowText( _T("LdrFormatDataPartition failed") );
	else
		m_Status.SetWindowText( _T("OK") );
}

void CEbootLoaderDlg::OnBnClickedButton17()
{
	DWORD type = m_FileType.GetItemData( m_FileType.GetCurSel() );
	CString str;
	DWORD ret = 0;

	LdrSetWaitTimeout(g_hPort, 120000);
	DWORD tick = GetTickCount();
	ret = LdrBurnSDFile(g_hPort, (sw_type_data)type);
	tick = GetTickCount() - tick;


	if(ret)
	{
		str.Format(_T("LdrBurnSDFile failed err 0x%x. %d ms"), ret, tick);
		m_Status.SetWindowText( str.GetBuffer(0) );
	}
	else
	{
		str.Format(_T("LdrBurnSDFile OK. %d ms"), tick);
		m_Status.SetWindowText( str.GetBuffer(0) );
	}
}

void CEbootLoaderDlg::OnBnClickedButton19()
{
	m_Status.SetWindowText( _T("") );
	m_EbootVer.SetWindowText( _T("") );
	m_HwVer.SetWindowText( _T("") );
	m_XldrVer.SetWindowText( _T("") );
	m_ImgVer.SetWindowText( _T("") );
	VersionInfo versionInfo = {0};

	HWVersions hwVersions	= {0};
	// 0 = "ES1.0", 1 = "ES2.0", 2 = "ES2.1", 3 = "ES2.0, ID determined using CRC", 4 = "ES2.1, ID determined using CRC", 0xFF = UNKNOWN
	// 0 = "ES1.0" ,0x10 = "ES2.x", 0x40 = "ES3.0" ,0x50 = "ES3.1", 0xFF = "Unknown"

	CString str;
	DWORD err;
	if( !(err = LdrGetHWVersion(g_hPort, &hwVersions)) )
	{
		str.Format( _T("%02x.%02x"), hwVersions.CPUVersion, hwVersions.CoChipVersion );
		m_HwVer.SetWindowText( str.GetBuffer(0) );
	}
	if( !(err = LdrGetVersion( g_hPort, &versionInfo, VERSION_EBOOT )))
	{
		str.Format( _T("%u.%u.%u\t\t\t%S"), versionInfo.majorVersion, 
										versionInfo.minorVersion,
										versionInfo.incrementalVersion,
										versionInfo.buildDate			);
		m_EbootVer.SetWindowText( str.GetBuffer(0) );
	}
	if( !(err = LdrGetVersion( g_hPort, &versionInfo, VERSION_XLDR )))
	{
		str.Format( _T("%u.%u.%u\t\t\t%S"), versionInfo.majorVersion, 
										versionInfo.minorVersion,
										versionInfo.incrementalVersion,
										versionInfo.buildDate			);
		m_XldrVer.SetWindowText( str.GetBuffer(0) );
	}
	if( !(err = LdrGetVersion( g_hPort, &versionInfo, VERSION_IMG )))
	{
//		str.Format( _T("%u.%u.%u %s\t\t%S"), versionInfo.majorVersion, 
//											versionInfo.minorVersion,
//											versionInfo.incrementalVersion,
//											versionInfo.buildDate			);
		str.Format( _T("%s\t\t%S"), 	versionInfo.versionString, 
										versionInfo.buildDate			);
		m_ImgVer.SetWindowText( str.GetBuffer(0) );
	}
}

void CEbootLoaderDlg::OnBnClickedButton20()
{
	CString str;
	DWORD err;
	DWORD type = m_ErType.GetItemData( m_ErType.GetCurSel() );
	LdrSetWaitTimeout(g_hPort, 120000);
	DWORD tick = GetTickCount();
	err = LdrEraseFlash(g_hPort, type);
	tick = GetTickCount() - tick;


	if(err)
	{
		str.Format(_T("LdrEraseFlash failed err 0x%x. %d ms"), err, tick);
		m_Status.SetWindowText( str.GetBuffer(0) );
	}
	else
	{
		str.Format(_T("LdrEraseFlash OK. %d ms"), tick);
		m_Status.SetWindowText( str.GetBuffer(0) );
	}
}

void CEbootLoaderDlg::OnBnClickedButton22()
{
	DWORD err;
	UINT16 freq;
	CString str;
	//m_MaxFreq
	if( err = LdrGetMaxFreq(g_hPort, &freq) )
	{
		str.Format(_T("CMD_GET_MAX_FREQ failed. err = %d"), err);
		m_Status.SetWindowText(str);
		return;
	}

	str.Format(_T("MaxFreq = %d"), freq);
	m_Status.SetWindowText( str.GetBuffer(0) );

	switch (freq)
	{
	case 600:
		m_MaxFreq.SetCurSel(0);
		break;
	case 800:
		m_MaxFreq.SetCurSel(1);
		break;
	case 1000:
		m_MaxFreq.SetCurSel(2);
		break;
	default:
		str.Format(_T("Received Bad Max Frequency value = %d"), freq);
		m_Status.SetWindowText( str.GetBuffer(0) );
		break;
	}
}
void CEbootLoaderDlg::OnBnClickedButton23()
{
	CString str;
	int err;
	UINT16 freq;
	int sel = m_MaxFreq.GetCurSel();

	switch (sel)
	{
	case 0:
		freq = 600;
		break;
	case 1:
		freq = 800;
		break;
	case 2:
		freq = 1000;
		break;
	default:
		m_Status.SetWindowText( _T("Bad selection of MaxFreq") );
		return;
	}
	if( err = LdrSetMaxFreq(g_hPort, &freq) )
	{
		str.Format(_T("CMD_SET_MAX_FREQ failed. err = %d"), err);
		m_Status.SetWindowText(str);
	}
	else
	{
		str.Format(_T("Set MaxFreq %d Successfully"), freq);
		m_Status.SetWindowText( str.GetBuffer(0) );
	}
}
