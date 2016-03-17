/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
/*
================================================================================

 Copyright (c) 2008, 2013 Micronet LTD. All rights reserved.

 History of Changes:
 Vladimir Zatulovsky CE500
 Anna R.              03.2009  Adaptation to M5000 platform.
 Igor Lantsman UI
 Ran Meyerstein CDS
 Vladimir Zatulovsky CE300/CE317
================================================================================
*/
//
//  File:  menu.c
//
#include <eboot.h>
#include <bsp_logo.h>
#include <swupdate.h>

//------------------------------------------------------------------------------
//
//  Define:  dimof
//
#ifdef dimof
#undef dimof
#endif
#define dimof(x)                (sizeof(x)/sizeof(x[0]))

//------------------------------------------------------------------------------

static VOID SetMacAddressD(OAL_BLMENU_ITEM *pMenu);
static VOID SetWiFiMacAddressD(OAL_BLMENU_ITEM *pMenu);
static VOID SetCdsParamsD(OAL_BLMENU_ITEM *pMenu);
static VOID ShowSettings(OAL_BLMENU_ITEM *pMenu);
static VOID ShowNetworkSettings(OAL_BLMENU_ITEM *pMenu);
static VOID SetKitlMode(OAL_BLMENU_ITEM *pMenu);
static VOID SetKitlType(OAL_BLMENU_ITEM *pMenu);
//static VOID SetDeviceID(OAL_BLMENU_ITEM *pMenu);
//static VOID SaveSettings(OAL_BLMENU_ITEM *pMenu);
static VOID SetRetailMsgMode(OAL_BLMENU_ITEM *pMenu);

#if BUILDING_EBOOT_SD
static VOID ShowSDCardSettings(OAL_BLMENU_ITEM *pMenu);
static VOID EnterSDCardFilename(OAL_BLMENU_ITEM *pMenu);
#endif

//------------------------------------------------------------------------------

extern OAL_BLMENU_ITEM g_menuFlash[];
extern	VOID BlSerMsgLoop();
//------------------------------------------------------------------------------
VOID OALBLMenuSelectDeviceD(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuEnableD(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuSetIpAddressD(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuSetIpMaskD(OAL_BLMENU_ITEM *pMenu);
static VOID SaveSettingsD(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuShowD(OAL_BLMENU_ITEM *pMenu);

UINT32 LoadIni();

/////////////////////////////////////////////////////////
static OAL_BLMENU_ITEM g_menuNetwork[] = {
    {
        L'1', L"Show Current Settings", ShowNetworkSettings,
        NULL, NULL, NULL
    }, {
        L'2', L"Set WiFi MAC address", SetWiFiMacAddressD,
        NULL, NULL, NULL
    }, {
        L'3', L"Enable/disable DHCP", OALBLMenuEnableD,
        L"DHCP", &g_bootCfg.kitlFlags, (VOID*)OAL_KITL_FLAGS_DHCP
    }, {
        L'4', L"Set IP address", OALBLMenuSetIpAddressD,
        L"Device", &g_bootCfg.ipAddress, NULL
    }, {
        L'5', L"Set IP mask", OALBLMenuSetIpMaskD,
        L"Device", &g_bootCfg.ipMask, NULL
    }, {
        L'6', L"Enable/disable KITL", OALBLMenuEnableD,
        L"KITL", &g_bootCfg.kitlFlags, (VOID*)OAL_KITL_FLAGS_ENABLED
    }, {
        L'0', L"Exit and Continue", NULL,
        NULL, NULL, NULL
    }, {
        0, NULL, NULL,
        NULL, NULL, NULL
    }
};

#if BUILDING_EBOOT_SD
static OAL_BLMENU_ITEM g_menuSDCard[] = {
    {
        L'1', L"Show Current Settings", ShowSDCardSettings,
        NULL, NULL, NULL
    }, {
        L'2', L"Enter Filename", EnterSDCardFilename,
        NULL, NULL, NULL
    }, {
        L'0', L"Exit and Continue", NULL,
        NULL, NULL, NULL
    }, {
        0, NULL, NULL,
        NULL, NULL, NULL
    }
};
#endif

#if BUILDING_EBOOT_SD
static OAL_BLMENU_ITEM g_menuMain[] = {
    {
        L'1', L"Show Current Settings", ShowSettings,
        NULL, NULL, NULL
    }, {
        L'2', L"Select Boot Device", OALBLMenuSelectDeviceD,
        L"Boot", &g_bootCfg.bootDevLoc, g_bootDevices
    }, {
        L'3', L"Select KITL (Debug) Device", OALBLMenuSelectDeviceD,
        L"Debug", &g_bootCfg.kitlDevLoc, g_kitlDevices
    }, {
        L'4', L"Network Settings", OALBLMenuShowD,
        L"Network Settings", &g_menuNetwork, NULL
    }, {
        L'5', L"SDCard Settings", OALBLMenuShowD,
        L"SDCard Settings", &g_menuSDCard, NULL
//    }, {
//        L'6', L"Set Device ID", SetDeviceID,
//        NULL, NULL, NULL
    }, {
        L'6', L"Flash Management", OALBLMenuShowD,
        L"Flash Management", &g_menuFlash, NULL
    }, {
        L'7', L"Set CDS Parameters", SetCdsParamsD,
        NULL, NULL, NULL
    }, {
        L'a', L"Save Settings", SaveSettingsD,
        NULL, NULL, NULL
//    }, {
//        L'9', L"Enable/Disable OAL Retail Messages", SetRetailMsgMode,
//        NULL, NULL, NULL
    }, {
        L'0', L"Exit and Continue", NULL,
        NULL, NULL, NULL
    }, {
        0, NULL, NULL,
        NULL, NULL, NULL
    }
};
#else
static OAL_BLMENU_ITEM g_menuMain[] = {
    {
        L'1', L"Show Current Settings", ShowSettings,
        NULL, NULL, NULL
    }, {
        L'2', L"Select Boot Device", OALBLMenuSelectDeviceD,
        L"Boot", &g_bootCfg.bootDevLoc, g_bootDevices
    }, {
        L'3', L"Network Settings", OALBLMenuShowD,
        L"Network Settings", &g_menuNetwork, NULL
    }, {
        L'4', L"Flash Management", OALBLMenuShowD,
        L"Flash Management", &g_menuFlash, NULL
    }, {
        L'5', L"Set CDS Parameters", SetCdsParamsD,
        NULL, NULL, NULL
    }, {
        L'a', L"Save Settings", SaveSettingsD,
        NULL, NULL, NULL
    }, {
        L'0', L"Exit and Continue", NULL,
        NULL, NULL, NULL
    }, {
        0, NULL, NULL,
        NULL, NULL, NULL
    }
};
#endif

static OAL_BLMENU_ITEM g_menuRoot = {
    0, NULL, OALBLMenuShowD,
    L"Main Menu", g_menuMain, NULL
};

KeypadKey KeyStr[] = 
{
	{ KEY_S50,	L"ESC"	}, // L"DECLINE"} must be 0,
	{ KEY_S30,	L"LEFT"	}, // Show Current Settings
	{ KEY_S20,	L"RIGHT"}, // Select Boot Device
	{ KEY_S60,	L"UP"	}, // Network Settings
	{ KEY_S00,	L"DOWN"	}, // Flash Management
	{ KEY_S40,	L"ACCEPT"},// Set CDS Parameters
	{ KEY_S70,	L"PUSH"	}, // must be 0xa
	{ 0,		L""		}  // the end

};
int PUSH_INDEX = PUSH_KEY_INDEX(KeyStr);

WCHAR g_txt[MAX_LINE_LEN] = {0};
//----------------------------------------------------------------------------------------------------------
VOID	Strn2Wstr(CHAR* pStr, WCHAR* pWstr, int len)
{
	while(-1 != --len)
	{
		pWstr[len] = pStr[len]; 
	}
}
//----------------------------------------------------------------------------------------------------------
CHAR*	TrimLeft( CHAR* pTxt, UINT32 MaxLen )
{
	while( MaxLen && ( ('\t' == *pTxt) || (' ' == *pTxt)	|| 
						 ('\r' == *pTxt) || ('\n' == *pTxt)	||
						 ('\0' == *pTxt)					) )
	{
		pTxt++;
		MaxLen--;
	}

	if(0 == MaxLen || 0 == *pTxt )
		return 0;
	return pTxt;
}
//----------------------------------------------------------------------------------------------------------
BOOL GetLine(CHAR* pTxt, CHAR* pDest, UINT32 MaxLen)
{
	UINT32 Len;
	CHAR* ptr = pTxt;
	Len = strcspn(pTxt, "\r\n\0");
	if(0 == ptr || MaxLen < Len)
		return 0;
	strncpy(pDest, pTxt, Len);
	pDest[Len] = 0;

	return 1;
}
//----------------------------------------------------------------------------------------------------------
CHAR* GetValByName( const CHAR* Name, CHAR* Line, UINT32 MaxLen )
{
	UINT32	NLen	= strlen(Name);
	if( 0 == strncmp(Line, Name, NLen) )
	{
		return TrimLeft( Line + NLen, MaxLen - NLen);
	}

	return 0;
}
//----------------------------------------------------------------------------------------------------------
BOOL GetValStr(const CHAR* Name, CHAR* pTxt, CHAR* pDest, UINT32 Len)
{
	CHAR buf[MAX_LINE_LEN] = {0};
	CHAR* ptr = 0, *next;
	
	pTxt[MAX_INI_LEN-1] = 0;

	next = pTxt;

	while((0 < next) && (next < pTxt + MAX_INI_LEN - 1))
	{		
		//OALLog(L"GetValStr:\t%x\r\n", next);
		memset(buf, sizeof(buf), 0);
		if(!GetLine( next, buf, MAX_LINE_LEN - 1 ))
			return 0;
		//Strn2Wstr(buf, g_txt, strlen(buf) + 1);
		if( ptr = GetValByName( Name, buf, MAX_LINE_LEN - 1 ) )
		{
			//Strn2Wstr(ptr, g_txt, strlen(ptr) + 1);
			strncpy(pDest, ptr, Len);
			return 1;
		}
		next += strlen(buf);
		if( next < pTxt + MAX_INI_LEN - 1 )
			next = TrimLeft( next, MAX_INI_LEN + pTxt - next - 1 ); 
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------
BOOL PasswordCheck()
{
	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);
	CHAR	password[16] = {0};
	UINT32	ret = 0;

	if( GetValStr("PASS:", pBuff, password, sizeof(password) - 1) )
	{
		if(0 == strcmp("8491", password))
			ret = 1;
	}

	return ret;
}
//----------------------------------------------------------------------------------------------------------
VOID StartMenu()
{
	ShowLogo(-1, 0);
    
	if(0 != OALReadKeyPad())
	{
		TextWriteW( StartX, StartY, L"Please, release the key...", TX_TOP, CL_TEXT );
		while(0 != OALReadKeyPad() );
		ShowLogo(-1, 0);
	}
}
//----------------------------------------------------------------------------------------------------------
UINT32 GetKeyPressed()
{
	UINT64 keysDown;
	UINT32 key;
	
	while(1)
	{
		while( 0 == (keysDown = OALReadKeyPad()) );
		
		if(keysDown != OALReadKeyPad())//another test
			continue;

		key = 0;
		while(KeyStr[key].Key)
		{
			if(KeyStr[key].Key & keysDown)
				return key;
			key++;
		}
	}
	return key;
}
VOID OALBLMenuSetIpMaskD(OAL_BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    UINT32 *pIp = pMenu->pParam2;
    UINT32 ip;
    WCHAR buffer[16];
	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);

	UINT32 x = StartX, y = StartY;
	UINT32 key;

	StartMenu();

    // First check input parameters    
    if (title == NULL || pIp == NULL) 
	{
 		TextWriteW( x, y, L"ERROR: OALBLMenuSetIpMask: Invalid parameters", TX_TOP, CL_TEXT );
		return;
    }

    // Get actual value
    ip = *pIp;
	if( !GetValStr("IP MASK:", pBuff, (CHAR*)buffer, sizeof(buffer)) )
	{
		TextWriteW( x, y, L"IP mask is not found", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);//to show result
		return;
	}
	Strn2Wstr((CHAR*)buffer, buffer, dimof(buffer));
	// Convert string to IP address
    if(0 == OALKitlStringToIP(buffer)) 
	{
		OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"'%s' isn't valid IP mask", buffer);
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);//to show result
		return;
    }

	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Set %s IP mask (actual %s)?", title, OALKitlIPtoString(ip));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"New IP mask %s ['PUSH'- YES]:", buffer);
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	key = GetKeyPressed();

	if(PUSH_INDEX == key)//PUSH
	{
		TextWriteW( x + 400, y, L"YES", TX_TOP, CL_TEXT );
		// Save new setting
		ip = OALKitlStringToIP(buffer);
		*pIp = ip;
	}
	else   
	{
		TextWriteW( x + 400, y, L"NO", TX_TOP, CL_TEXT );
	}

	y += 20;

	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s IP mask set to %s", title, OALKitlIPtoString(ip));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	OALStall(SHOW_DELAY);    
}

//------------------------------------------------------------------------------

VOID OALBLMenuSetIpAddressD(OAL_BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    UINT32 *pIp = pMenu->pParam2;
    UINT32 ip;
    WCHAR buffer[16];
	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);

	UINT32 x = StartX, y = StartY;
	UINT32 key;

	StartMenu();
    // First check input parameters    
    if (title == NULL || pIp == NULL) 
	{
 		TextWriteW( x, y, L"ERROR: OALBLMenuSetIpAddressD: Invalid parameters", TX_TOP, CL_TEXT );
		return;
     }

    // Get actual value
    ip = *pIp;

	if( !GetValStr("IP ADDR:", pBuff, (CHAR*)buffer, sizeof(buffer)) )
	{
		TextWriteW( x, y, L"IP address is not found", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);//to show result
		return;
	}
	Strn2Wstr((CHAR*)buffer, buffer, dimof(buffer));
	// Convert string to IP address
//    ip = OALKitlStringToIP(buffer);
    if(0 == OALKitlStringToIP(buffer)) 
	{
		OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"'%s' isn't valid IP address", buffer);
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);//to show result
		return;
    }

	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Set %s IP address (actual %s)?", title, OALKitlIPtoString(ip));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"New IP address %s ['PUSH'- YES]:", buffer);
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	key = GetKeyPressed();

	if(PUSH_INDEX == key)//PUSH
	{
		TextWriteW( x + 400, y, L"YES", TX_TOP, CL_TEXT );
		// Save new setting
		ip = OALKitlStringToIP(buffer);
		*pIp = ip;
	}
	else   
	{
		TextWriteW( x + 400, y, L"NO", TX_TOP, CL_TEXT );
	}
	y += 20;

	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s IP address set to %s", title, OALKitlIPtoString(ip));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	OALStall(SHOW_DELAY);//to show result   
}


VOID OALBLMenuEnableD(OAL_BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    UINT32 *pFlags = pMenu->pParam2;
    UINT32 mask = (UINT32)pMenu->pParam3;
    BOOL flag;
    WCHAR key;
	UINT32 x = StartX, y = StartY;

	StartMenu();
 
    // First check input parameter   
    if (title == NULL || pFlags == NULL) 
	{
		TextWriteW( x, y, L"ERROR: OALBLMenuEnableD: Invalid parameters", TX_TOP, CL_TEXT );
		return;
    }
    if (mask == 0) mask = 0xFFFF;

    flag = (*pFlags & mask) != 0;
    
	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s %s (actually %s) ['PUSH'- YES]: ",
							(flag ? L"Disable" : L"Enable"),  title, (flag ? L"enabled" : L"disabled"));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	key = GetKeyPressed();

	if(PUSH_INDEX == key)
	{
        flag = !flag;
 		TextWriteW( x + 400, y, L"YES", TX_TOP, CL_TEXT );
		y += 20;
		OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s %s\r\n", title, (flag ? L"enabled" : L"disabled"));
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
    } 
	else 
	{
 		TextWriteW( x + 400, y, L"NO", TX_TOP, CL_TEXT );
		y += 20;
 		OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s stays %s", title, (flag ? L"enabled" : L"disabled"));
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
    }


    // Save value
    if (flag) {
        *pFlags |= mask;
    } else {
        *pFlags &= ~mask;
    }
	OALStall(SHOW_DELAY);//to show result
}

//------------------------------------------------------------------------------

VOID OALBLMenuSelectDeviceD(OAL_BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    DEVICE_LOCATION *pDevLoc = pMenu->pParam2;
    OAL_KITL_DEVICE *aDevices = pMenu->pParam3, *pDevice;
    DEVICE_LOCATION devLoc[OAL_MENU_MAX_DEVICES];
    UINT32 i;
    WCHAR key;

	UINT32 x = StartX, y = StartY;

	StartMenu();
    // First check input parameters    
    if (title == NULL || pDevLoc == NULL || aDevices == NULL) 
	{
		TextWriteW( x, y, L"ERROR: OALBLMenuSelectDeviceD: Invalid parameters", TX_TOP, CL_TEXT );
		return;
    }

	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Select %s Device", title);
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
	y += 40;

    i = 0;
    pDevice = aDevices;
    while (pDevice->name != NULL && i < OAL_MENU_MAX_DEVICES) 
	{
        switch (pDevice->ifcType) 
		{
			case Internal:
				devLoc[i].IfcType = pDevice->ifcType;
				devLoc[i].BusNumber = 0;
				devLoc[i].LogicalLoc = pDevice->id;
				devLoc[i].PhysicalLoc = 0;

				OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"[%s]", KeyStr[i+1].KeyName );
 				TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
				TextWriteW( x + 100, y, (WCHAR*)(OALKitlDeviceName(&devLoc[i], aDevices)), TX_TOP, CL_TEXT );
				y += 20;
				i++;
				pDevice++;
			break;
        default:
            pDevice++;
            break;
        }        
    }    
	TextWriteW( x,		y, L"[ESC]", TX_TOP, CL_TEXT );
	TextWriteW( x + 100,y, L"Exit and Continue", TX_TOP, CL_TEXT );
	y += 20;

	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Selection (actual %s):", OALKitlDeviceName(pDevLoc, aDevices));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	do
	{
		key = GetKeyPressed();
		TextWriteW( x + 350, y, L"            ", TX_TOP, CL_TEXT );
		TextWriteW( x + 350, y, KeyStr[key].KeyName, TX_TOP, CL_TEXT );
	}while(key > i);
	
	y += 20;
  
	if(0 == key)//ESC - don't save
		return;

	memcpy(pDevLoc, &devLoc[key - 1], sizeof(DEVICE_LOCATION));

	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s device set to %s", title, OALKitlDeviceName(pDevLoc, aDevices));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
	OALStall(SHOW_DELAY);//to show result

}

//------------------------------------------------------------------------------

VOID OALBLMenuShowD(OAL_BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    OAL_BLMENU_ITEM *aMenu = pMenu->pParam2, *pItem;
    WCHAR key;

	UINT32 x = StartX, y = StartY;

    while (TRUE) 
	{        
		StartMenu();
		y = StartY;

		TextWriteW( x, y, (WCHAR*)title, TX_TOP, CL_TEXT );
		y += 20;

        // Print menu items
        for (pItem = aMenu; pItem->key != 0; pItem++) 
		{
			int ind = 0;
			if(pItem->key <= '9')
				key = pItem->key - '0';
			else if(pItem->key == 'a')
				key = 6;
			else
				key = 0;
			OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"[%s]", KeyStr[key].KeyName);
			TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
			TextWriteW( x + 72, y, (WCHAR*)pItem->text, TX_TOP, CL_TEXT );
			y += 20;
        }
		TextWriteW( x, y, L"Selection: ", TX_TOP, CL_TEXT );

        while (TRUE) 
		{
			key = GetKeyPressed();
			TextWriteW( x + 100, y, KeyStr[key].KeyName, TX_TOP, CL_TEXT );

			key = (key < PUSH_INDEX) ? (key += '0') : (key += 'a' - PUSH_INDEX);
			
//			OALLog(L" Pressed:    %d(%d)\r\n", key, PUSH_INDEX);

            for(pItem = aMenu; pItem->key != 0; pItem++) 
			{
                if (pItem->key == key) 
					break;			
			}
			// If we find it, break loop
            if(pItem->key) 
				break;
        }

        // When action is NULL return back to parent menu
        if (pItem->pfnAction == NULL) 
			break;
        
        // Else call menu action
        pItem->pfnAction(pItem);
    } 
}
//-------------------------------------------------------------------------------
UINT32	StopSwUpdate()
{
    BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);

	if(g_bootCfg.SwUpdateFlags & SW_START_UPDATE)
	{
		pArgs->SwUpdateFlags &= ~SW_START_UPDATE;
		pArgs->SwUpdateFlags |= (SW_USER_CANCEL | SW_END_UPDATE);
		g_bootCfg.SwUpdateFlags = 0;
		BLWriteBootCfg(&g_bootCfg);
		return 1;
	}
	return 0;
}
//------------------------------------------------------------------------------

VOID BLMenu(BOOL bForced)
{
    WCHAR key = 0;
    BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);
	UINT64 keysDown;

	BOOL		NoAction	= 0;
	UINT32		time;

	BlSerMsgLoop();
	
	keysDown=OALReadKeyPad();
#ifndef SHIP_BUILD
	OALLog(L"OALReadKeyPad: key = %x  \r\n",keysDown);
#endif	
	if(keysDown == (KEY_S30 | KEY_S20 | KEY_S60)) // left+right+up
	{
		time		= OALGetTickCount() + 3000;
		//OALLog(L"Format data partition waiting\r\n");
		while( (OALGetTickCount() < time) && !NoAction)
		{
			keysDown = OALReadKeyPad();
			if(keysDown != (KEY_S30 | KEY_S20 | KEY_S60))
			{
				NoAction = 1;
			}
		}
		if(!NoAction)
		{
			StopSwUpdate();
			pArgs->bFormatPartFlag	= TRUE;
			pArgs->coldBoot			= TRUE;
			TextWrite( StartX, StartY, "Flash data partition formatted.", TX_BOTTOM, CL_TEXT );
		}
	}
	if(keysDown == (KEY_S60 | KEY_S00 | KEY_S20)) //up+down+right
	{
		time		= OALGetTickCount() + 3000;

		while( (OALGetTickCount() < time) && !NoAction)
		{
			keysDown = OALReadKeyPad();
			if(keysDown != (KEY_S60 | KEY_S00 | KEY_S20))
			{
				NoAction = 1;
			}
		}
		if(!NoAction)
		{
			StopSwUpdate();
			pArgs->bHiveCleanFlag	= TRUE;
			pArgs->coldBoot			= TRUE;
			TextWrite( StartX, StartY, "Registry set to factory default.", TX_BOTTOM, CL_TEXT );
		}
	}
	else if(bForced || (keysDown == KEY_S60)) //up
	{		
		NoAction = LoadIni();
#ifdef SHIP_BUILD
		if( NoAction && PasswordCheck() )
#endif
		{
			if(StopSwUpdate())
				LoadIni();//reload

			OALBLMenuShowD(&g_menuRoot);
			// Invalidate arguments to force them to be reinitialized
			// with new config data generated by the boot menu
			pArgs->header.signature = 0;
		}
    }  
	else if(keysDown == KEY_S00) //down
		//if(( (pArgs->identLCD & 7) &&  (keysDown == KEY_S20) ) ||
		//	( (!pArgs->identLCD) &&  (keysDown == KEY_S9 ) )	) 
	{
#ifndef SHIP_BUILD
		OALLog(L"OALReadKeyPad: Key Pressed to start Loader Communication  \r\n");
#endif
		// we need to choose rndis transport and start LDR communications
		g_bootCfg.bootDevLoc.LogicalLoc = g_bootDevices[1].id;
		g_bootCfg.bootDevLoc.IfcType	= g_bootDevices[1].ifcType;

		g_bootCfg.kitlFlags |= OAL_KITL_FLAGS_DHCP;
		g_bootCfg.launch_manutool = 1;
	}
	
	if( (	(g_bootCfg.SwUpdateFlags & SW_START_UPDATE) && 
			!(g_bootCfg.SwUpdateFlags & SW_END_UPDATE)		)	)
	{
		OALMSG(OAL_INFO, (L"OALReadKeyPad: OS Update. SwUpdateFlags - %x  \r\n", g_bootCfg.SwUpdateFlags));

		g_bootCfg.bootDevLoc.IfcType	= Internal;
		g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA;
	}
}

//------------------------------------------------------------------------------
BOOL OALBLMenuSetMacAddressD(OAL_BLMENU_ITEM *pMenu, UINT32* pX, UINT32* pY)
{
    LPCWSTR title = pMenu->pParam1;
    UINT16 *pMac = pMenu->pParam2;
    UINT16 mac[3];
    WCHAR buffer[18];
	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);
	UINT32 key;
	UINT32 x = *pX, y = *pY;

    // First check input parameters    
    if (title == NULL || pMac == NULL) 
	{
		TextWriteW( x, y, L"ERROR: OALBLMenuSetMacAddressD: Invalid parameters", TX_TOP, CL_TEXT );
		return 0;
    }
    // Get actual setting
//    mac[0] = pMac[0];
//    mac[1] = pMac[1];
//    mac[2] = pMac[2];

	if( !GetValStr("ETH MAC:", pBuff, (CHAR*)buffer, sizeof(buffer)) )
	{
		TextWriteW( x, y, L"MAC address is not found", TX_TOP, CL_TEXT );
		return 0;
	}

	Strn2Wstr((CHAR*)buffer, buffer, dimof(buffer));

	// Convert string to MAC address
    if(!OALKitlStringToMAC(buffer, mac)) //test
	{
		OALLogPrintf(g_txt, dimof(g_txt), L"'%s' isn't valid MAC address", buffer);
		TextWriteW( x, y, L"MAC address is not found", TX_TOP, CL_TEXT );
		return 0;
    }

    // Get actual setting
    mac[0] = pMac[0];
    mac[1] = pMac[1];
    mac[2] = pMac[2];

	// Print prompt
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Set %s MAC address (actual %s)?", title, OALKitlMACtoString(mac));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"New MAC address %s ['PUSH'- YES]:", buffer);
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	key = GetKeyPressed();
    
	if(PUSH_INDEX == key)
	{
		OALKitlStringToMAC(buffer, mac);
	   // Save new setting
		pMac[0] = mac[0];
		pMac[1] = mac[1];
		pMac[2] = mac[2];   

		TextWriteW( x + 400, y, L"YES", TX_TOP, CL_TEXT );
		y += 20;
		OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s MAC address set to %s", title, OALKitlMACtoString(mac));
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
		key = 1;//return
     } 
	else 
	{
 		TextWriteW( x + 400, y, L"NO", TX_TOP, CL_TEXT );
		key = 0;//return
    }
	
	*pX = x;
	*pY = y;

	return key;
 }


//--------------------------------------------------------------------------------
VOID SetMacAddressD(OAL_BLMENU_ITEM *pMenu)
{
    UINT16 mac[3] = {0xFFFF, 0xFFFF, 0xFFFF};
    // We're using OALBLMenuSetMacAddress outside of a menu, but it still requires 
    // a menu item input structure
    OAL_BLMENU_ITEM DummyMacMenuItem = 
    {
        0, NULL, NULL, L"EVM 3730", mac, NULL
    };
//	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);

	UINT32 x = StartX, y = StartY;

	StartMenu();

	TextWriteW( x, y, L"Ethernet (LAN9xxx) MAC Address", TX_TOP, CL_TEXT );
	y += 20;

	memcpy(mac, g_bootCfg.EthMacAddress, sizeof(mac));
    if(OALBLMenuSetMacAddressD(&DummyMacMenuItem, &x, &y ))
	{
		// Make sure we don't come out of OALBLMenuSetMacAddress with the original 0xFFFF FFFF FFFF or with all zeros
		if(((mac[0]== 0xFFFF) && (mac[1] == 0xFFFF) && (mac[2] == 0xFFFF)) || 
		   ((mac[0]== 0x0) && (mac[1] == 0x0) && (mac[2] == 0x0)))
		{
			TextWriteW( x, y, L"WARNING: MAC address not programmed!", TX_TOP, CL_TEXT );
		}
		else
		{
			BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);

			memcpy((void*)g_bootCfg.EthMacAddress, mac, sizeof(pArgs->EthMacAddress));
			BLWriteBootCfg(&g_bootCfg);
			memcpy((void*)pArgs->EthMacAddress, mac, sizeof(pArgs->EthMacAddress));
			LoadIni();//reload after BLWriteBootCfg
		}
	}   

	OALStall(SHOW_DELAY);//to show result
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL OALBLMenuSetWiFiMacAddressD(OAL_BLMENU_ITEM *pMenu, UINT32* pX, UINT32* pY)
{
    LPCWSTR title = pMenu->pParam1;
    UINT16 *pMac = pMenu->pParam2;
    UINT16 mac[3];
    WCHAR buffer[18];
	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);
	UINT32 key;
	UINT32 x = *pX, y = *pY;

    // First check input parameters    
    if (title == NULL || pMac == NULL) 
	{
		TextWriteW( x, y, L"ERROR: OALBLMenuSetWiFiMacAddressD: Invalid parameters", TX_TOP, CL_TEXT );
		return 0;
    }

	if( !GetValStr("WIFI MAC:", pBuff, (CHAR*)buffer, sizeof(buffer)) )
	{
		TextWriteW( x, y, L"MAC address is not found", TX_TOP, CL_TEXT );
		return 0;
	}

	Strn2Wstr((CHAR*)buffer, buffer, dimof(buffer));

	// Convert string to MAC address
    if(!OALKitlStringToMAC(buffer, mac)) //test
	{
		OALLogPrintf(g_txt, dimof(g_txt), L"'%s' isn't valid MAC address", buffer);
		TextWriteW( x, y, L"MAC address is not found", TX_TOP, CL_TEXT );
		return 0;
    }

    // Get actual setting
    mac[0] = pMac[0];
    mac[1] = pMac[1];
    mac[2] = pMac[2];

	// Print prompt
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Set %s WiFi MAC address (actual %s)?", title, OALKitlMACtoString(mac));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"New WiFi MAC address %s ['PUSH'- YES]:", buffer);
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	key = GetKeyPressed();
    
	if(PUSH_INDEX == key)
	{
		OALKitlStringToMAC(buffer, mac);
	   // Save new setting
		pMac[0] = mac[0];
		pMac[1] = mac[1];
		pMac[2] = mac[2];   

		TextWriteW( x + 424, y, L"YES", TX_TOP, CL_TEXT );
		y += 20;
		OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s WiFi MAC address set to %s", title, OALKitlMACtoString(mac));
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
		key = 1;//return
     } 
	else 
	{
 		TextWriteW( x + 424, y, L"NO", TX_TOP, CL_TEXT );
		key = 0;//return
    }
	
	*pX = x;
	*pY = y;

	return key;
 }


//--------------------------------------------------------------------------------
VOID SetWiFiMacAddressD(OAL_BLMENU_ITEM *pMenu)
{
    UINT16 mac[3] = {0xFFFF, 0xFFFF, 0xFFFF};
    // We're using OALBLMenuSetMacAddress outside of a menu, but it still requires 
    // a menu item input structure
    OAL_BLMENU_ITEM DummyMacMenuItem = 
    {
        0, NULL, NULL, L"EVM 3730", mac, NULL
    };
//	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);

	UINT32 x = StartX, y = StartY;

	StartMenu();

	TextWriteW( x, y, L"WiFi MAC Address", TX_TOP, CL_TEXT );
	y += 20;

	memcpy(mac, g_bootCfg.WiFiMacAddress, sizeof(mac));

	if(OALBLMenuSetWiFiMacAddressD(&DummyMacMenuItem, &x, &y ))
	{
		// Make sure we don't come out of OALBLMenuSetWiFiMacAddressD with the original 0xFFFF FFFF FFFF or with all zeros
		if(((mac[0]== 0xFFFF) && (mac[1] == 0xFFFF) && (mac[2] == 0xFFFF)) || 
		   ((mac[0]== 0x0) && (mac[1] == 0x0) && (mac[2] == 0x0)))
		{
			TextWriteW( x, y, L"WARNING: WiFi MAC address not programmed!", TX_TOP, CL_TEXT );
		}
		else
		{
			BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);

			memcpy((void*)g_bootCfg.WiFiMacAddress, mac, sizeof(pArgs->WiFiMacAddress));
			BLWriteBootCfg(&g_bootCfg);
			memcpy((void*)pArgs->WiFiMacAddress, mac, sizeof(pArgs->WiFiMacAddress));
			LoadIni();//reload after BLWriteBootCfg
		}
	}   

	OALStall(SHOW_DELAY);//to show result
}
//------------------------------------------------------------------------------
BOOL CdsStringToParam(LPCWSTR szCDS, UINT16 cds[8])
{
    INT32 i, j;
    LPCWSTR pos;
    WCHAR ch;

    // Convert string to CDS parameters
    memset(cds, 0, 16);
    i = j = 0;
    pos = szCDS;
    while (i < 4) {
        ch = *pos;
        if (ch == L'-' || ch == L':' || ch == L'.') {
            i++;
            j = 0;
        } else {
            if (j >= 2) {
                i++;
                j = 0;
            }
            if (ch >= L'0' && ch <= L'9') {
                cds[i] = (cds[i] << 4) + (ch - L'0');
            } else if (ch >= L'a' && ch <= L'f') {
                cds[i] = (cds[i] << 4) + (ch - L'a' + 10);
            } else if (ch >= 'A' && ch <= 'F') {
                cds[i] = (cds[i] << 4) + (ch - 'A' + 10);
            } else {
                break;
            }
            j++;
        }
        pos++;
    }
    pos++; //One delimiter to frequency parameter

	if ((*pos == '1') && (*(pos+1) == '0') && (*(pos+2) == '0') && (*(pos+3) == '0')) {
		cds[i] = 1000;
		pos = pos + 4;
	}

	else if ((*pos == '8') && (*(pos+1) == '0') && (*(pos+2) == '0')) {
		cds[i] = 800;
		pos = pos + 3;
	}
	else {
		return 0;
	}

    return (*pos == L'\0');
}

//------------------------------------------------------------------------------
LPWSTR CdsParamToString(UINT16 cds[])
{
	static WCHAR szBuffer[100];

	OALLogPrintf(
		szBuffer, sizeof(szBuffer)/sizeof(WCHAR), 
		L"MJT:0x%x IT:0x%x LTT:0x%x Feature State:0x%x Max Freq:%dMHz", cds[0], cds[1], cds[2], cds[3], cds[4]);
	return szBuffer;
}
//------------------------------------------------------------------------------
BOOL OALBLMenuSetCdsParamsD(OAL_BLMENU_ITEM *pMenu, UINT32* pX, UINT32* pY)
{
    LPCWSTR title = pMenu->pParam1;
    UINT16 *pCdsParam = pMenu->pParam2;
    UINT16 lCds[8];
    WCHAR buffer[30];
	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);
	UINT32 key;
	UINT32 x = *pX, y = *pY;

    // First check input parameters    
    if (title == NULL || pCdsParam == NULL) 
	{
		TextWriteW( x, y, L"ERROR: OALBLMenuSetWiFiMacAddressD: Invalid parameters", TX_TOP, CL_TEXT );
		return 0;
    }
 
	if( !GetValStr("CDS PARAMS:", pBuff, (CHAR*)buffer, sizeof(buffer)) )
	{
		TextWriteW( x, y, L"CDS Parameters were not found", TX_TOP, CL_TEXT );
		return 0;
	}

	Strn2Wstr((CHAR*)buffer, buffer, dimof(buffer));

	// Convert string to CDS Parameters
    if(!CdsStringToParam(buffer, lCds))
	{
		OALLogPrintf(g_txt, dimof(g_txt), L"'%s' are not valid CDS Parameters", buffer);
		TextWriteW( x, y, L"CDS Parameters not found", TX_TOP, CL_TEXT );
		return 0;
    }

    // Get actual setting
    lCds[0] = pCdsParam[0];
    lCds[1] = pCdsParam[1];
    lCds[2] = pCdsParam[2];
	lCds[3] = pCdsParam[3];
	lCds[4] = pCdsParam[4];

	// Print prompt
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Set %s CDS Parameters (actual %s)?", title, CdsParamToString(lCds));
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	OALLogPrintf(	g_txt, sizeof(g_txt)/sizeof(WCHAR), L"New CDS Parameters %s ['PUSH'- YES]:", buffer);
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	key = GetKeyPressed();
    
	if(PUSH_INDEX == key)
	{
		CdsStringToParam(buffer, lCds);
		// Save new setting
		memcpy((void*)pCdsParam, lCds, sizeof(lCds));

		TextWriteW( x + 424, y, L"YES", TX_TOP, CL_TEXT );
		y += 20;
		OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%s CDS Parameters is set to %s", title, CdsParamToString(lCds));
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
		key = 1;//return
     } 
	else 
	{
 		TextWriteW( x + 424, y, L"NO", TX_TOP, CL_TEXT );
		key = 0;//return
    }
	
	*pX = x;
	*pY = y;

	return key;
 }


//--------------------------------------------------------------------------------
VOID SetCdsParamsD(OAL_BLMENU_ITEM *pMenu)
{
	UINT16 lCdsParams[8]; //Includes buth temperatures and frequencies
    // We're using OALBLMenuSetCdsParamsD outside of a menu, but it still requires 
    // a menu item input structure
    OAL_BLMENU_ITEM DummyCdsMenuItem = 
    {
        0, NULL, NULL, L"EVM 3730", lCdsParams, NULL
    };
	UINT32 x = StartX, y = StartY;

#if (BSP_CDS == 1)
	StartMenu();

	TextWriteW( x, y, L"CDS Parameters", TX_TOP, CL_TEXT );
	y += 20;

	memcpy(lCdsParams, g_bootCfg.CdsParams, sizeof(g_bootCfg.CdsParams));

	if(OALBLMenuSetCdsParamsD(&DummyCdsMenuItem, &x, &y ))
	{
			BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);

			memcpy((void*)g_bootCfg.CdsParams, lCdsParams, sizeof(pArgs->CdsParams));
			BLWriteBootCfg(&g_bootCfg);
			memcpy((void*)pArgs->CdsParams, lCdsParams, sizeof(pArgs->CdsParams));
			LoadIni();//reload after BLWriteBootCfg
	}   

#else
	TextWriteW( x, y, L"CDS Not Supported", TX_TOP, CL_TEXT );
#endif

	OALStall(SHOW_DELAY);//to show result
}

//------------------------------------------------------------------------------
VOID ShowSettings(OAL_BLMENU_ITEM *pMenu)
{
	UINT32 x = StartX, y = StartY; 
    
	StartMenu();

	TextWriteW( x, y, L"Main:", TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Boot device:", TX_TOP, CL_TEXT );
	TextWriteW( x + 110, y, (WCHAR*)(OALKitlDeviceName(&g_bootCfg.bootDevLoc, g_bootDevices)), TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Debug device:", TX_TOP, CL_TEXT );
	TextWriteW( x + 110, y, (WCHAR*)(OALKitlDeviceName(&g_bootCfg.kitlDevLoc, g_kitlDevices)), TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Device ID:", TX_TOP, CL_TEXT );
    OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"%d", g_bootCfg.deviceID);
	TextWriteW( x + 110, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Retail Msgs:", TX_TOP, CL_TEXT );
	TextWriteW( x + 110, y, ((g_bootCfg.oalFlags & BOOT_CFG_OAL_FLAGS_RETAILMSG_ENABLE) ? L"enabled" : L"disabled"), TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"CDS: ", TX_TOP, CL_TEXT );
#if (BSP_CDS == 1)
	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"MJT:%d  IT:%d  LTT:%d  Feature State:0x%x Max Freq:%dMHz", 
		g_bootCfg.CdsParams[0], g_bootCfg.CdsParams[1], g_bootCfg.CdsParams[2], g_bootCfg.CdsParams[3],
		g_bootCfg.CdsParams[4]);
#else
	OALLogPrintf(g_txt, sizeof(g_txt)/sizeof(WCHAR), L"Not Supported");
#endif
	TextWriteW( x + 110, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;


#if BUILDING_EBOOT_SD
    ShowSDCardSettings(pMenu);
#endif
    //ShowNetworkSettings(pMenu);
    ShowNetworkSettings(0);
}

//------------------------------------------------------------------------------

VOID ShowNetworkSettings(OAL_BLMENU_ITEM *pMenu)
{
    UINT16 mac[3] = {0xFFFF, 0xFFFF, 0xFFFF};
	UINT32 x = StartX, y = StartY; 
    
	if(!pMenu)
	{
		y = 130;
	}
	else
		StartMenu();

	TextWriteW( x, y, L"Network:", TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x,		y, L"KITL state:", TX_TOP, CL_TEXT );
	TextWriteW( x + 110,y, ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_ENABLED) ? L"enabled" : L"disabled"), TX_TOP, CL_TEXT );
	TextWriteW( x + 200,y, L"IP address:", TX_TOP, CL_TEXT );
	TextWriteW( x + 310,y, (OALKitlIPtoString(g_bootCfg.ipAddress)), TX_TOP, CL_TEXT );
	y += 20;

	TextWriteW( x,		y, L"KITL type:", TX_TOP, CL_TEXT );
	TextWriteW( x + 110,y, ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_PASSIVE) ? L"passive" : L"active"), TX_TOP, CL_TEXT );
	TextWriteW( x + 200,y, L"IP mask:", TX_TOP, CL_TEXT );
	TextWriteW( x + 310,y, (OALKitlIPtoString(g_bootCfg.ipMask)), TX_TOP, CL_TEXT );
	y += 20;

	TextWriteW( x,		y, L"KITL mode:", TX_TOP, CL_TEXT );
	TextWriteW( x + 110,y, ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_POLL) ? L"polled" : L"interrupt"), TX_TOP, CL_TEXT );
	TextWriteW( x + 200,y, L"IP router:", TX_TOP, CL_TEXT );
	TextWriteW( x + 310,y, (OALKitlIPtoString(g_bootCfg.ipRoute)), TX_TOP, CL_TEXT );
	y += 20;
 	TextWriteW( x,		y, L"DHCP:", TX_TOP, CL_TEXT );
	TextWriteW( x + 110,y, ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_DHCP) ? L"enabled" : L"disabled"), TX_TOP, CL_TEXT );
	TextWriteW( x + 200,y, L"VMINI:", TX_TOP, CL_TEXT );
	TextWriteW( x + 310,y, ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_VMINI) ? L"enabled" : L"disabled"), TX_TOP, CL_TEXT );
	y += 20;
    
    //OALLog(L"  Eth MAC Addr:  ");
	TextWriteW( x, y, L"Eth  MAC Addr:", TX_TOP, CL_TEXT );
	memcpy(mac, g_bootCfg.EthMacAddress, sizeof(mac));
	TextWriteW( x + 116, y, (OALKitlMACtoString(mac)), TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"WiFi MAC Addr:", TX_TOP, CL_TEXT );
	memcpy(mac, g_bootCfg.WiFiMacAddress, sizeof(mac));
	TextWriteW( x + 116, y, (OALKitlMACtoString(mac)), TX_TOP, CL_TEXT );
	y += 20;

	//OALLog(L"  Note: USBFN RNDIS MAC Addr cannot be changed.\r\n");
//	TextWriteW( x, y, L"Note: USBFN RNDIS MAC Addr cannot be changed.", TX_TOP, CL_TEXT );
    
	while( 0 == OALReadKeyPad() );
}

//------------------------------------------------------------------------------
VOID SetKitlMode(OAL_BLMENU_ITEM *pMenu)
{
    WCHAR key;

    if ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_POLL) != 0) {
        OALLog(L" Set KITL to interrupt mode [y/-]: ");
    } else {
        OALLog(L" Set KITL to polled mode [y/-]: ");
    }    

    // Get key
    key = OALBLMenuReadKey(TRUE);
    OALLog(L"%c\r\n", key);

    if (key == L'y' || key == L'Y') {
        if ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_POLL) != 0) {
            g_bootCfg.kitlFlags &= ~OAL_KITL_FLAGS_POLL;
            OALLog(L" KITL set to interrupt mode\r\n");
        } else {
            g_bootCfg.kitlFlags |= OAL_KITL_FLAGS_POLL;
            OALLog(L" KITL set to polled mode\r\n");
        }            
    }
}

//------------------------------------------------------------------------------

VOID
SetKitlType(
    OAL_BLMENU_ITEM *pMenu
    )
{
    WCHAR key;

    if ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_PASSIVE) != 0) 
        {
        OALLog(L" Set KITL to active mode [y/-]: ");
        } 
   else 
        {
        OALLog(L" Set KITL to passive mode [y/-]: ");
        }    

    // Get key
    key = OALBLMenuReadKey(TRUE);
    OALLog(L"%c\r\n", key);

    if (key == L'y' || key == L'Y') 
        {
        if ((g_bootCfg.kitlFlags & OAL_KITL_FLAGS_PASSIVE) != 0) 
            {
            g_bootCfg.kitlFlags &= ~OAL_KITL_FLAGS_PASSIVE;
            OALLog(L" KITL set to active mode\r\n");
            }
        else 
            {
            g_bootCfg.kitlFlags |= OAL_KITL_FLAGS_PASSIVE;
            OALLog(L" KITL set to passive mode\r\n");
            }            
        }
}

//------------------------------------------------------------------------------

VOID 
SetDeviceID(
    OAL_BLMENU_ITEM *pMenu
    )
{
    WCHAR  szInputLine[16];

    OALLog(
        L" Current Device ID:  %d\r\n", g_bootCfg.deviceID
    );

    OALLog(L"\r\n New Device ID: ");

    if (OALBLMenuReadLine(szInputLine, dimof(szInputLine)) == 0) 
        {
        goto cleanUp;
        }

    // Get device ID
    g_bootCfg.deviceID = OALStringToUINT32(szInputLine);

cleanUp:
    return;
}

//------------------------------------------------------------------------------

VOID SetRetailMsgMode(OAL_BLMENU_ITEM *pMenu)
{
    WCHAR key;

    if (g_bootCfg.oalFlags & BOOT_CFG_OAL_FLAGS_RETAILMSG_ENABLE) {
        OALLog(L" Disable OAL Retail Messages [y/-]: ");
    } else {
        OALLog(L" Enable OAL Retail Messages [y/-]: ");
    }    

    // Get key
    key = OALBLMenuReadKey(TRUE);
    OALLog(L"%c\r\n", key);

    if (key == L'y' || key == L'Y') {
        if (g_bootCfg.oalFlags & BOOT_CFG_OAL_FLAGS_RETAILMSG_ENABLE) 
		{
            g_bootCfg.oalFlags &= ~BOOT_CFG_OAL_FLAGS_RETAILMSG_ENABLE;
            OALLog(L" OAL Retail Messages disabled\r\n");
        }
		else 
		{
            g_bootCfg.oalFlags |= BOOT_CFG_OAL_FLAGS_RETAILMSG_ENABLE;
            OALLog(L" OAL Retail Messages enabled\r\n");
        }    
    }
}

//------------------------------------------------------------------------------

VOID SaveSettingsD(OAL_BLMENU_ITEM *pMenu)
{
    WCHAR key;
	UINT32 x = StartX, y = StartY; 
    
	StartMenu();

	TextWriteW( x, y, L" Do you want save current settings ['PUSH'- YES]? ", TX_TOP, CL_TEXT );

    // Get key
	key = GetKeyPressed();
	if(PUSH_INDEX == key)
	{
 		TextWriteW( x + 410, y, L"YES", TX_TOP, CL_TEXT );
		y += 20;
		if (BLWriteBootCfg(&g_bootCfg)) 
		{
	 		TextWriteW( x, y, L"Current settings has been saved", TX_TOP, CL_TEXT );
		} 
		else 
		{        
 			TextWriteW( x, y, L"ERROR: Settings save failed!", TX_TOP, CL_TEXT );
		}
		LoadIni();//reload after BLWriteBootCfg
	}
	else
 		TextWriteW( x + 410, y, L"NO", TX_TOP, CL_TEXT );
 
	OALStall(SHOW_DELAY);//to show result
    return;
}

//------------------------------------------------------------------------------

UINT32 OALStringToUINT32(LPCWSTR psz)
{
    UINT32 i = 0;

    // Replace the dots with NULL terminators
    while (psz != NULL && *psz != L'\0') 
        {
        if (*psz < L'0' || *psz > L'9') 
            {
            break;
            }
        i = i * 10 + (*psz - L'0');
        psz++;
        }

    return i;
}

#if BUILDING_EBOOT_SD

//------------------------------------------------------------------------------

VOID ShowSDCardSettings(OAL_BLMENU_ITEM *pMenu)
{
    OALLog(L"\r\n SDCard:\r\n");
    OALLog(
        L"  Filename:      \"%s\"\r\n", g_bootCfg.filename
    );
//  OALBLMenuReadKey(TRUE);
}

//------------------------------------------------------------------------------
#define dimof(x)                (sizeof(x)/sizeof(x[0]))

VOID EnterSDCardFilename(OAL_BLMENU_ITEM *pMenu)
{
    OALLog(L"\r\n Type new filename (8.3 format) :");
    OALBLMenuReadLine(g_bootCfg.filename, dimof(g_bootCfg.filename));
}

//------------------------------------------------------------------------------

#endif
UINT32 LoadIni()
{
	UINT32		rc;

	wcscpy(g_bootCfg.filename, L"boot.ini");
//	BOOT_CFG bootCfg = {0};
//	bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
//	wcscpy(bootCfg.filename, L"boot.ini");
	rc = BLSDCardDownload(&g_bootCfg, 0);

	if(BL_ERROR != rc)
	{
		UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);
		memset(pBuff, 0, MAX_INI_LEN);
		if( BLSDCardReadFileData(MAX_INI_LEN, pBuff))
			return 1;
	}

	return 0;
}