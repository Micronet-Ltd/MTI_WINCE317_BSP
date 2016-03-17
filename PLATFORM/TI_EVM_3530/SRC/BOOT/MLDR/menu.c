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
 Igor Lantsman UI
 Vladimir Zatulovsky CE300/CE317
================================================================================
*/
//
//  File:  menu.c
//
#include <eboot.h>

//------------------------------------------------------------------------------
//
//  Define:  dimof
//
#ifdef dimof
#undef dimof
#endif
#define dimof(x)                (sizeof(x)/sizeof(x[0]))
#define OAL_MENU_MAX_DEVICES    8

//------------------------------------------------------------------------------
void OALEraseEntireFlash(OAL_BLMENU_ITEM *pMenu);
void OALSetBlockBad(OAL_BLMENU_ITEM *pMenu);
static OAL_BLMENU_ITEM g_menuMain[] = {
	{L'1', L"Select Operation", OALBLMenuSelectDevice, L"Burn or load", &g_bootCfg.bootDevLoc, g_bootDevices},
	{L'2', L"Erase Entire flash", OALEraseEntireFlash, 0, 0, 0},
	{L'3', L"Erase Entire flash (with saving boot parameters)", OALEraseEntireFlash, (void*)_1bit_to_1bit, 0, 0},
	{L'4', L"Set bad block", OALSetBlockBad, 0, 0, 0},
	{L'0', L"Exit and Continue", 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0}
};

static OAL_BLMENU_ITEM g_menuRoot = {
    0, NULL, OALBLMenuShow, L"Prepare platform for boot", g_menuMain, NULL
};

extern UINT32 g_ecctype;

void OALSetBlockBad(OAL_BLMENU_ITEM *pMenu)
{
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    BLOCK_ID blockId;
    WCHAR szInputLine[16];

    UNREFERENCED_PARAMETER(pMenu);

	// Open FMD
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if(!hFMD)
	{
        OALLog(L" Oops, can't open FMD driver\r\n");
		return;
	}

    if(!FMD_GetInfo(&flashInfo))
	{
        OALLog(L" Oops, can't get flash geometry info\r\n");
        FMD_Deinit(hFMD);
		return;
	}

    OALLog(L"\r\n Block Number: ");

    if(OALBLMenuReadLine(szInputLine, dimof(szInputLine)) == 0)
	{
        FMD_Deinit(hFMD);
		return;
	}

    // Get sector number
    blockId = OALStringToUINT32(szInputLine);

    // Check sector number
    if(blockId < 4 || blockId >= flashInfo.dwNumBlocks) 
	{
        OALLog(L" Oops, incorrect block number\r\n");
        FMD_Deinit(hFMD);
		return;
	}

    FMD_SetBlockStatus(blockId, BLOCK_STATUS_BAD);

    OALLog(L"\r\n Done\r\n");

    FMD_Deinit(hFMD);

    return;
}

void OALEraseEntireFlash(OAL_BLMENU_ITEM *pMenu)
{
	EraseEntireFlash((UINT32)pMenu->pParam1);
}

//------------------------------------------------------------------------------
//
//  Function:  OALKitlDeviceType
//
OAL_KITL_TYPE OALKitlDeviceType(DEVICE_LOCATION *pDevLoc, OAL_KITL_DEVICE *pDevice)
{
    BOOL rc = FALSE;
    OAL_KITL_TYPE type = OAL_KITL_TYPE_NONE;
    
    KITL_RETAILMSG(1, ("+OALKitlDeviceType(%d/%d/%x, 0x%x)\r\n", pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc, pDevice));

    // Look for driver in list
    while (pDevice->name != NULL && !rc)
	{
		KITL_RETAILMSG(0, ("OALKitlDeviceType(%d/%d/%x, 0x%x)\r\n", pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc, pDevice));
        // Is it our device? Then move
        if (pDevLoc->IfcType != pDevice->ifcType) {
            // Move to next driver
            pDevice++;
            continue;
        }            

        // Identify device and read its address/interrupt depends on bus type
        switch (pDevLoc->IfcType) {
        case Internal:
            if (pDevLoc->LogicalLoc != pDevice->id)
				break;
            if ((UINT32)pDevLoc->PhysicalLoc != pDevice->resource)
				break;
            type = pDevice->type;
            rc = TRUE;
            break;
        }

        // If we don't identify device skip it
        if (!rc) {
            pDevice++;
            continue;
        }
            
        // We get all we need
        break;
    }


    KITL_RETAILMSG(1, ("-OALKitlDeviceType(type = %d)\r\n", type));
    return type;
}

LPCWSTR OALKitlDeviceName(DEVICE_LOCATION *pDevLoc, OAL_KITL_DEVICE *pDevice)
{
    BOOL rc = FALSE;
    static WCHAR name[64];
    LPCWSTR pName = NULL;
    
    KITL_RETAILMSG(ZONE_KITL_OAL, ("+OALKitlDeviceName(%d/%d/%x, 0x%x)\r\n", pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc, pDevice));

    // Look for driver in list
    while (pDevice->name != NULL && !rc)
	{
		KITL_RETAILMSG(0, ("OALKitlDeviceName(%d/%d/%x, 0x%x)\r\n", pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc, pDevice));
        // Is it our device? Then move
        if (pDevLoc->IfcType != pDevice->ifcType) {
            // Move to next driver
            pDevice++;
            continue;
        }            

        // Identify device and read its address/interrupt depends on bus type
        switch (pDevLoc->IfcType) {
        case Internal:
            if (pDevLoc->LogicalLoc != pDevice->id)
				break;
            if ((UINT32)pDevLoc->PhysicalLoc != pDevice->resource)
				break;
            pName = pDevice->name;
            rc = TRUE;
            break;
        }

        // If we don't identify device skip it
        if (!rc) {
            pDevice++;
            continue;
        }
            
        // We get all we need
        break;
    }


    KITL_RETAILMSG(ZONE_KITL_OAL, (
        "-OALKitlDeviceName(name = 0x%x('%s')\r\n", pName, pName
    ));
    return pName;
}
//------------------------------------------------------------------------------

VOID OALBLMenuHeader(LPCWSTR format, ...)
{
    va_list pArgList;
    UINT32 i;

    va_start(pArgList, format);

    OALLog(L"\r\n");
    for (i = 0; i < 80; i++) OALLog(L"-");
    OALLog(L"\r\n ");
    OALLogV(format, pArgList);
    OALLog(L"\r\n");
    for (i = 0; i < 80; i++) OALLog(L"-");
    OALLog(L"\r\n");
}

//------------------------------------------------------------------------------

VOID OALBLMenuShow(OAL_BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    OAL_BLMENU_ITEM *aMenu = pMenu->pParam2, *pItem;
    WCHAR key;

    while(1)
	{
        OALBLMenuHeader(L"%s", title);

        // Print menu items
        for(pItem = aMenu; pItem->key != 0; pItem++)
		{
            OALLog(L" [%c] %s\r\n", pItem->key, pItem->text);
        }
        OALLog(L"\r\n Selection: ");

        while(1)
		{
            // Get key
            key = OALBLMenuReadKey(1);
            // Look for key in menu
            for(pItem = aMenu; pItem->key != 0; pItem++)
			{
                if(pItem->key == key)
					break;
            }
            // If we find it, break loop
            if(pItem->key != 0)
				break;
        }

        // Print out selection character
        OALLog(L"%c\r\n", key);
        
        // When action is NULL return back to parent menu
        if(pItem->pfnAction == 0)
			break;
        
        // Else call menu action
        pItem->pfnAction(pItem);
    } 

}
//------------------------------------------------------------------------------

VOID OALBLMenuSelectDevice(OAL_BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    DEVICE_LOCATION *pDevLoc = pMenu->pParam2;
    OAL_KITL_DEVICE *aDevices = pMenu->pParam3, *pDevice;
    DEVICE_LOCATION devLoc[OAL_MENU_MAX_DEVICES];
    UINT32 i;
    WCHAR key;

    // First check input parameters    
    if(title == NULL || pDevLoc == NULL || aDevices == NULL)
	{
        OALMSG(OAL_ERROR, (L"ERROR: OALBLMenuSelectDevice: Invalid parameters\r\n"));
        goto cleanUp;
    }

    OALBLMenuHeader(L"%s", title);

    i = 0;
    pDevice = aDevices;
    while (pDevice->name != NULL && i < OAL_MENU_MAX_DEVICES) {
        switch (pDevice->ifcType)
		{
			case Internal:
				devLoc[i].IfcType = pDevice->ifcType;
				devLoc[i].BusNumber = 0;
				devLoc[i].LogicalLoc = pDevice->id;
				devLoc[i].PhysicalLoc = (PVOID)pDevice->resource;
				OALLog(L" [%d] %s\r\n", i + 1, OALKitlDeviceName(&devLoc[i], aDevices));

				i++;
				pDevice++;
				break;
			default:
				pDevice++;
				break;
        } 
    }    
    OALLog(L" [0] Exit and Continue\r\n");

    OALLog(L"\r\n Selection (actual %s): ", OALKitlDeviceName(pDevLoc, aDevices));

    do
	{
        key = OALBLMenuReadKey(TRUE);
    }while(key < L'0' || key > L'0' + i);

    OALLog(L"%c\r\n", key);

    // If user select exit don't change device
    if(key == L'0')
		goto cleanUp;
    
    memcpy(pDevLoc, &devLoc[key - L'0' - 1], sizeof(DEVICE_LOCATION));

	// 1st boot device is MMC1 (SD Card Boot)
	g_bootCfg.deviceID = 0;
	if((UINT32)pDevLoc->PhysicalLoc == IMAGE_XLDR_CODE_PA)
	{
		g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
		g_bootCfg.osPartitionSize = IMAGE_XLDR_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"xldrnand.bin");
	}
	else if((UINT32)pDevLoc->PhysicalLoc == IMAGE_EBOOT_CODE_PA)
	{
		g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
		g_bootCfg.osPartitionSize = IMAGE_EBOOT_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"ebnand.bin");
	}
	else if((UINT32)pDevLoc->PhysicalLoc == IMAGE_WINCE_CODE_PA && ///bmp
			(UINT32)pDevLoc->LogicalLoc	== OMAP_MMCHS1_REGS_PA	)
	{
		g_bootCfg.bootDevLoc.LogicalLoc = (UINT32)pDevLoc->LogicalLoc;
		g_bootCfg.osPartitionSize		= IMAGE_BOOTLOADER_BITMAP_SIZE;
		wcscpy(g_bootCfg.filename, L"logo.bmp");
	}
	else if((UINT32)pDevLoc->PhysicalLoc == IMAGE_WINCE_CODES_PA)
	{
		g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
		g_bootCfg.osPartitionSize = IMAGE_WINCE_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"nk.bin");
	}
	else
	{
		g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;
		g_bootCfg.osPartitionSize = IMAGE_WINCE_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"nk.bin");
	}

    OALLog(L" %s device set to %s\r\n", title, OALKitlDeviceName(pDevLoc, aDevices));

cleanUp:;    
}

//------------------------------------------------------------------------------

UINT32 OALBLMenuReadLine(LPWSTR szBuffer, size_t CharCount)
{
    UINT32 count;
    WCHAR key;
    
    count = 0;
    while (count < CharCount) {
        key = OALBLMenuReadKey(TRUE);
       if (key == L'\r' || key == L'\n') {
          OALLog(L"\r\n");
          break;
       } if (key == L'\b' && count > 0) {
          OALLog(L"\b \b");
          count--;
       } else if (key >= L' ' && key < 128 && count < (CharCount - 1)) {
          szBuffer[count++] = key;
          OALLog(L"%c", key);
       } 
    }
    szBuffer[count] = '\0';
    return count;
    
}

//------------------------------------------------------------------------------

WCHAR OALBLMenuReadKey(BOOL wait)
{
    CHAR key;

    while ((key = OEMReadDebugByte()) == OEM_DEBUG_READ_NODATA && wait);
    if (key == OEM_DEBUG_READ_NODATA) key = 0;
    return (WCHAR)key;
}

//------------------------------------------------------------------------------

VOID BLMenu(BOOL bForced)
{
    UINT32 time, delay = 5;
    WCHAR key = 0;
    BSP_ARGS *pArgs = OALPAtoCA(IMAGE_SHARE_ARGS_PA);
    
    // First let user break to menu
    while(!bForced && (delay > 0 && key != L' '))
	{
       OALLog(L"Hit space to enter configuration menu %d...\r\n", delay);
       time = OALGetTickCount();
       while((OALGetTickCount() - time) < 1000)
	   {
          if((key = OALBLMenuReadKey(FALSE)) == L' ')
			  break;
       }
       delay--;
    }
    
    if((bForced == TRUE) || (key == L' ')) 
	{
        OALBLMenuShow(&g_menuRoot);
        // Invalidate arguments to force them to be reinitialized
		// with new config data generated by the boot menu
        pArgs->header.signature = 0;
    }        
}

//------------------------------------------------------------------------------
//
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

//------------------------------------------------------------------------------
#define dimof(x)                (sizeof(x)/sizeof(x[0]))
//------------------------------------------------------------------------------