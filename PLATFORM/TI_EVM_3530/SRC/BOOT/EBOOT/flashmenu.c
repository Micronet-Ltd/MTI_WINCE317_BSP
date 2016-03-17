// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky, Igor Lantsman
//
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
//
//  File:  menu.c
//
#include <eboot.h>
#include <bsp_logo.h>

//------------------------------------------------------------------------------
//
//  Define:  dimof
//
#ifdef dimof
#undef dimof
#endif
#define dimof(x)                (sizeof(x)/sizeof(x[0]))

//------------------------------------------------------------------------------

static VOID ShowFlashGeometry(OAL_BLMENU_ITEM *pMenu);
static VOID EraseFlash(OAL_BLMENU_ITEM *pMenu);
//static VOID EraseBlock(OAL_BLMENU_ITEM *pMenu);
static VOID ReserveBlock(OAL_BLMENU_ITEM *pMenu);
//static VOID SetBadBlock(OAL_BLMENU_ITEM *pMenu);
//static VOID DumpFlash(OAL_BLMENU_ITEM *pMenu);
static VOID FormatFlash(OAL_BLMENU_ITEM *pMenu);

extern WCHAR g_txt[];
//------------------------------------------------------------------------------

OAL_BLMENU_ITEM g_menuFlash[] = {
    {
        L'1', L"Show flash geometry", ShowFlashGeometry,
        NULL, NULL, NULL
    }, {
        L'2', L"Erase flash", EraseFlash,
        NULL, NULL, NULL
     }, {
        L'3', L"Reserve block range", ReserveBlock,
        NULL, NULL, NULL
   }, {
        L'4', L"Format flash", FormatFlash,
        NULL, NULL, NULL
    }, {
        L'0', L"Exit and Continue", NULL,
        NULL, NULL, NULL
    }, {
        0, NULL, NULL,
        NULL, NULL, NULL
    }
};


//------------------------------------------------------------------------------

VOID 
ShowFlashGeometry(OAL_BLMENU_ITEM *pMenu)
{
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    LPCWSTR pszType;
    BLOCK_ID block;
    UINT32 status;
    UINT32 listmode=0;
	UINT32 x = StartX, y = StartY, xx = 0;

	StartMenu();

    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL) 
	{
//        OALLog(L" Oops, can't open FMD driver\r\n");
 		TextWriteW( x, y, L"Can't open FMD driver", TX_TOP, CL_TEXT );
		return;
    }

    if (!FMD_GetInfo(&flashInfo)) 
    {
//        OALLog(L" Oops, can't get flash geometry info\r\n");
        FMD_Deinit(hFMD);
 		TextWriteW( x, y, L"Can't get flash geometry info", TX_TOP, CL_TEXT );
		return;
	}

    switch (flashInfo.flashType) 
        {
        case NAND:
            pszType = L"NAND";
            break;
        case NOR:
            pszType = L"NOR";
            break;
        default:
            pszType = L"Unknown";
        }

    OALLog(L"\r\n");
    OALLog(L" Flash Type:    %s\r\n", pszType);
    OALLog(L" Blocks:        %d\r\n", flashInfo.dwNumBlocks);
    OALLog(L" Bytes/block:   %d\r\n", flashInfo.dwBytesPerBlock);
    OALLog(L" Sectors/block: %d\r\n", flashInfo.wSectorsPerBlock);
    OALLog(L" Bytes/sector:  %d\r\n", flashInfo.wDataBytesPerSector);

	TextWriteW( x, y, L"Flash Type:", TX_TOP, CL_TEXT );
	TextWriteW( x + 200, y, (WCHAR*)pszType, TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Blocks:", TX_TOP, CL_TEXT );
	OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", flashInfo.dwNumBlocks);
	TextWriteW( x + 200, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Bytes/block:", TX_TOP, CL_TEXT );
	OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", flashInfo.dwBytesPerBlock);
	TextWriteW( x + 200, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Sectors/block:", TX_TOP, CL_TEXT );
	OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", flashInfo.wSectorsPerBlock);
	TextWriteW( x + 200, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Bytes/sector:", TX_TOP, CL_TEXT );
	OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", flashInfo.wDataBytesPerSector);
	TextWriteW( x + 200, y, g_txt, TX_TOP, CL_TEXT );
	y += 20;


   // now list bad/reserved sectors

    // First offset given
    block	= 0;
	xx		= 100;

	while (block < flashInfo.dwNumBlocks) 
    {
        // If block is bad, we have to offset it
        status = FMD_GetBlockStatus(block);

        // bad block
        if ((status & BLOCK_STATUS_BAD) != 0) 
		{
			if (listmode!=1)
            {
                OALLog(L"\r\n[bad]     ");
				y += 20;
				TextWriteW( x, y, L"[bad]", TX_TOP, CL_TEXT );
                listmode=1;
				xx = 100;
            }

            OALLog(L" %d", block);
			OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", block);
			TextWriteW( x + xx, y, g_txt, TX_TOP, CL_TEXT );
			xx += 50;
			if( xx > 400 )
			{
				y	+= 20;
				xx	= 100;
			}
			block++;
            continue;
		}

        // reserved block
        if ((status & BLOCK_STATUS_RESERVED) != 0) 
        {
            if (listmode!=2)
            {
                OALLog(L"\r\n[reserved]");
				y += 20;
				TextWriteW( x, y, L"[reserved]", TX_TOP, CL_TEXT );
                listmode=2;
				xx = 100;
            }

            OALLog(L" %d", block);
			OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", block);
			TextWriteW( x + xx, y, g_txt, TX_TOP, CL_TEXT );
			xx += 20;
			if( xx > 400 )
			{
				y	+= 20;
				xx	= 100;
			}

            block++;
            continue;
        }

        block++;
    }

    OALLog(L" Done\r\n");
	y += 20;
	TextWriteW( x, y, L"Done", TX_TOP, CL_TEXT );

    if (hFMD != NULL) 
	{
        FMD_Deinit(hFMD);
	}

	while( 0 == OALReadKeyPad() );
    return;
}

//------------------------------------------------------------------------------

VOID EraseFlash(OAL_BLMENU_ITEM *pMenu)
{
//    WCHAR key;
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    BLOCK_ID block;
    UINT32 status;
	UINT32 x = StartX, y = StartY, xx;
    UINT32 listmode=0;

	StartMenu();

	TextWriteW( x, y, L"Do you want to erase unreserved blocks ['PUSH'-YES]?", TX_TOP, CL_TEXT );
	y += 20;

	if(PUSH_INDEX != GetKeyPressed())
	{
		TextWriteW( x + 400, y, L"NO", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);
		return;
	}
	TextWriteW( x + 400, y, L"YES", TX_TOP, CL_TEXT );
	y += 20;
    
	// Open FMD
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL) 
	{
   		TextWriteW( x, y, L"Can't open FMD driver", TX_TOP, CL_TEXT );
		return;
	}

    if (!FMD_GetInfo(&flashInfo)) 
    {
       FMD_Deinit(hFMD);
 		TextWriteW( x, y, L"Can't get flash geometry info", TX_TOP, CL_TEXT );
		return;
	}

    // First offset given
    block = 0;
    while (block < flashInfo.dwNumBlocks) 
	{
        // If block is bad, we have to offset it
        status = FMD_GetBlockStatus(block);

        // Skip bad blocks
		// the status of DL is incompatible with SA status
		// try erase in any case
        // Skip reserved blocks
        if ((status & BLOCK_STATUS_RESERVED) != 0) 
		{
            OALLog(L" Skip reserved block %d\r\n", block);
 			if(listmode != 2)
			{
				y += 20;
				TextWriteW( x, y, L"Skip reserved block", TX_TOP, CL_TEXT );
				listmode = 2;
				xx = 200;
			}
			OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", block);
 			TextWriteW( x + xx, y, g_txt, TX_TOP, CL_TEXT );
			xx += 20;
			if( xx > 400 )
			{
				y	+= 20;
				xx	= 200;
			}
            block++;
            continue;
		}

        // Erase block
        if (!FMD_EraseBlock(block)) 
		{
            OALLog(L" Oops, can't erase block %d - mark as bad\r\n", block);
			y += 20;
			TextWriteW( x, y, L"Can't erase block", TX_TOP, CL_TEXT );
			OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d - mark as bad", block);
 			TextWriteW( x + 200, y, g_txt, TX_TOP, CL_TEXT );
			y += 20;
            FMD_SetBlockStatus(block, BLOCK_STATUS_BAD);
		}

        block++;
    }

    OALLog(L" Done\r\n");
	y += 20;
	TextWriteW( x, y, L"Done", TX_TOP, CL_TEXT );
	
	// Block until a keypress
	while( 0 == OALReadKeyPad() );
	
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return;
}


//------------------------------------------------------------------------------
BOOL NumsFromStr( CHAR* str, UINT32* pNum1, UINT32* pNum2, CHAR delim )
{
	CHAR* ptr = str;

	if( *ptr < '0' || '9' < *ptr)
		return 0;

	*pNum1	= atoi(str);
	ptr		= strchr(str, delim);
	if(ptr)
	{
		ptr++;
		ptr = TrimLeft(ptr, MAX_LINE_LEN + str - ptr);
	}	
	if(!ptr || *ptr < '0' || '9' < *ptr)
	{
		return 0;
	}

	*pNum2 = atoi(ptr);

	return 1;
}
//------------------------------------------------------------------------------

VOID ReserveBlock(OAL_BLMENU_ITEM *pMenu)
{
//    WCHAR key;
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    BLOCK_ID firstblock, lastblock=0;
//    WCHAR szInputLine[16];
    CHAR szInputLine[MAX_LINE_LEN];
    UINT32 status, listmode = 0;
	UCHAR* pBuff = OALPAtoCA(IMAGE_WINCE_CODE_PA);
	UINT32 x = StartX, y = StartY, xx = 0;

	StartMenu();
////////////////////////////////////////////////////////////////////////////	
	TextWriteW( x, y, L"Unsupported functionality", TX_TOP, CL_TEXT );
	OALStall(SHOW_DELAY);
	return;
/////////////////////////////////////////////////////////////////////////////
	if( !GetValStr("RESV BLOCKS:", pBuff, szInputLine, sizeof(szInputLine)) )
	{
		TextWriteW( x, y, L"Parameter is not found", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);//to show result
		return;
	}

	if( !NumsFromStr(szInputLine, &firstblock, &lastblock, '-') ) 
	{
		Strn2Wstr(szInputLine, (WCHAR*)szInputLine, MAX_LINE_LEN / sizeof(WCHAR) );
		OALLogPrintf(g_txt, MAX_LINE_LEN, L"Parameter is bad: %s", (WCHAR*)szInputLine);
		TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);//to show result
		return;
	}

	if (lastblock < firstblock) 
	{
		lastblock=firstblock;
	}

	// Open FMD
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL) 
    {
		TextWriteW( x, y, L"Can't open FMD driver", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);
		return;
	}

    if (!FMD_GetInfo(&flashInfo)) 
	{
        FMD_Deinit(hFMD);
 		TextWriteW( x, y, L"Can't get flash geometry info", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);
		return;
	}

	if (lastblock >= flashInfo.dwNumBlocks) 
	{
         FMD_Deinit(hFMD);
 		TextWriteW( x, y, L"Too big block number", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);
		return;
     }

	OALLogPrintf(g_txt, MAX_LINE_LEN, L"Mark as reserved blocks %d - %d ['PUSH'-YES]?", firstblock, lastblock);
	TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );

	if(PUSH_INDEX != GetKeyPressed())//PUSH
	{
        FMD_Deinit(hFMD);
		TextWriteW( x + 400, y, L"NO", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);
		return;
	}

	TextWriteW( x + 400, y, L"YES", TX_TOP, CL_TEXT );
	y += 20;

	while (firstblock<=lastblock)
	{
        // If block is bad, we have to offset it
        status = FMD_GetBlockStatus(firstblock);

        // Skip bad blocks
        if ((status & BLOCK_STATUS_BAD) != 0) 
		{
            OALLog(L" Skip bad block %d\r\n", firstblock);
			if(listmode != 1)
            {
        		y += 20;
				TextWriteW( x, y, L"Skip bad block", TX_TOP, CL_TEXT );
				listmode = 1;
				xx = 200;
			}
			OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", firstblock);
 			TextWriteW( x + xx, y, g_txt, TX_TOP, CL_TEXT );
 			xx += 20;
			if( xx > 400 )
			{
				y	+= 20;
				xx	= 200;
			}
           
			firstblock++;
            continue;
		}

        // Skip already reserved blocks
        if ((status & BLOCK_STATUS_RESERVED) != 0) 
		{
            OALLog(L" Skip reserved block %d\r\n", firstblock);
			if(listmode != 2)
			{
				y += 20;
				TextWriteW( x, y, L"Skip reserved block", TX_TOP, CL_TEXT );
				listmode = 2;
				xx = 200;
			}
			OALLogPrintf(g_txt, MAX_LINE_LEN, L"%d", firstblock);
 			TextWriteW( x + xx, y, g_txt, TX_TOP, CL_TEXT );
			xx += 20;
			if( xx > 400 )
			{
				y	+= 20;
				xx	= 200;
			}
            firstblock++;
            continue;
		}

        // Mark block as read-only & reserved
        if (!FMD_SetBlockStatus(firstblock, BLOCK_STATUS_READONLY|BLOCK_STATUS_RESERVED)) 
		{
			OALLogPrintf(g_txt, MAX_LINE_LEN, L"Can't mark block %d - as reserved", firstblock);
			TextWriteW( x, y, g_txt, TX_TOP, CL_TEXT );
			y += 20;
		}

        firstblock++;
        OALLog(L".");
	}

    OALLog(L" Done\r\n");
	y += 20;
	TextWriteW( x, y, L"Done", TX_TOP, CL_TEXT );
	y += 20;

//cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
	OALStall(SHOW_DELAY);

    return;
}

//------------------------------------------------------------------------------
/*
VOID DumpFlash(OAL_BLMENU_ITEM *pMenu)
{
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    WCHAR szInputLine[16];
    UINT8 buffer[2048];
    UINT32 i, j;


    // Open FMD
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL) 
        {
        OALLog(L" Oops, can't open FMD driver\r\n");
        goto cleanUp;
        }

    if (!FMD_GetInfo(&flashInfo)) 
        {
        OALLog(L" Oops, can't get flash geometry info\r\n");
        goto cleanUp;
        }

    if (flashInfo.wDataBytesPerSector > sizeof(buffer)) 
        {
        OALLog(L" Oops, sector size larger than my buffer\r\n");
        goto cleanUp;
        }

    while (TRUE) 
        {

        OALLog(L"\r\n Sector Number: ");

        if (OALBLMenuReadLine(szInputLine, dimof(szInputLine)) == 0) 
            {
            break;
            }

        // Get sector number
        sector = OALStringToUINT32(szInputLine);

        // Check sector number
        if (sector > flashInfo.dwNumBlocks * flashInfo.wSectorsPerBlock) 
            {
            OALLog(L" Oops, too big sector number\r\n");
            continue;
            }

        if (!FMD_ReadSector(sector, buffer, &sectorInfo, 1)) 
            {
            OALLog(L" Oops, sector read failed\r\n");
            continue;
            }

        OALLog(
            L"\r\nSector %d (sector %d in block %d)\r\n", sector,
            sector%flashInfo.wSectorsPerBlock, sector/flashInfo.wSectorsPerBlock
        );
        OALLog(
            L"Reserved1: %08x OEMReserved: %02x Bad: %02x Reserved2: %04x\r\n",
            sectorInfo.dwReserved1, sectorInfo.bOEMReserved,
            sectorInfo.bBadBlock, sectorInfo.wReserved2
        );

        for (i = 0; i < flashInfo.wDataBytesPerSector; i += 16) 
            {
            OALLog(L"%04x ", i);
            for (j = i; j < i + 16 && j < flashInfo.wDataBytesPerSector; j++) 
                {
                OALLog(L" %02x", buffer[j]);
                }
            OALLog(L"  ");
            for (j = i; j < i + 16 && j < flashInfo.wDataBytesPerSector; j++) 
                {
                if (buffer[j] >= ' ' && buffer[j] < 127) 
                    {
                    OALLog(L"%c", buffer[j]);
                    } 
                else 
                    {
                    OALLog(L".");
                    }
                }
            OALLog(L"\r\n");
            }

        }

cleanUp:

    if (hFMD != NULL) 
        {
        FMD_Deinit(hFMD);
        }

    return;
}

//------------------------------------------------------------------------------

VOID SetBadBlock(OAL_BLMENU_ITEM *pMenu)
{
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    BLOCK_ID blockId;
    WCHAR szInputLine[16];


    // Open FMD
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL) 
        {
        OALLog(L" Oops, can't open FMD driver\r\n");
        goto cleanUp;
        }

    if (!FMD_GetInfo(&flashInfo)) 
        {
        OALLog(L" Oops, can't get flash geometry info\r\n");
        goto cleanUp;
        }

    OALLog(L"\r\n Block Number: ");

    if (OALBLMenuReadLine(szInputLine, dimof(szInputLine)) == 0) 
        {
        goto cleanUp;
        }

    // Get sector number
    blockId = OALStringToUINT32(szInputLine);

    // Check sector number
    if (blockId >= flashInfo.dwNumBlocks) 
        {
        OALLog(L" Oops, too big block number\r\n");
        goto cleanUp;
        }

    FMD_SetBlockStatus(blockId, BLOCK_STATUS_BAD);

    OALLog(L"\r\n Done\r\n");

cleanUp:
    if (hFMD != NULL) 
        {
        FMD_Deinit(hFMD);
        }

    return;
}
*/
//------------------------------------------------------------------------------

VOID FormatFlash(OAL_BLMENU_ITEM *pMenu)
{
//    WCHAR key;
	UINT32 x = StartX, y = StartY;

	StartMenu();
	
	TextWriteW( x, y, L"Do you want to format unreserved blocks ['PUSH'-YES]?", TX_TOP, CL_TEXT );
	y += 20;

	if(PUSH_INDEX != GetKeyPressed())//PUSH
	{
		TextWriteW( x + 400, y, L"NO", TX_TOP, CL_TEXT );
		OALStall(SHOW_DELAY);
		return;
	}
	TextWriteW( x + 400, y, L"YES", TX_TOP, CL_TEXT );
	y += 20;
	TextWriteW( x, y, L"Formatting flash...", TX_TOP, CL_TEXT );
	y += 20;

	if(BLConfigureFlashPartitions(TRUE))
		TextWriteW( x, y, L"Flash format complete!", TX_TOP, CL_TEXT );
	else
		TextWriteW( x, y, L"Flash format failed.", TX_TOP, CL_TEXT );

	y += 20;
	TextWriteW( x, y, L"Done.", TX_TOP, CL_TEXT );
	OALStall(SHOW_DELAY);//to show result

//cleanUp:	
	
    return;
}

