// Copyright (c) 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky

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
//  File:  main.c
//
//  This file contains X-Loader implementation for OMAP35XX
//
#include <bsp.h>
#include <blcommon.h>
#include <fmd.h>
#include <constants.h>
#include <bsp_version.h>
#include <omap_cpuver.h>

extern VOID OEMWriteDebugHex(unsigned long n, long depth);

//------------------------------------------------------------------------------

#ifdef SHIP_BUILD
#define XLDRMSGINIT
#define XLDRMSGDEINIT
#define XLDRMSG(msg)        
#define XLDRHEX(val, len)
#else
#define XLDRMSGINIT         OEMDebugInit()
#define XLDRMSGDEINIT       OEMDebugDeinit()
#define XLDRMSG(msg)        OEMWriteDebugString(msg)
#define XLDRHEX(val, len)   OEMWriteDebugHex(val, len)
#endif


//------------------------------------------------------------------------------
//  Global variables
ROMHDR * volatile const pTOC = (ROMHDR *)-1; 

const volatile DWORD dwOEMHighSecurity      = OEM_HIGH_SECURITY_GP;
UINT32 g_cpu_ver = CPU_FAMILY_OMAP35XX_REVISION_ES_1_0;

VersionPatternExt g_VersionPattern = {MARKER_LOW,XLDR_VERSION_MAJOR,XLDR_VERSION_MINOR,XLDR_VERSION_INCREMENTAL,__DATE__, XLDR_VERSION_STRING, MARKER_HIGH, XLDR_VERSION_BUILD, MARKER_EXT};

//------------------------------------------------------------------------------
//  External Functions

extern VOID PlatformSetup();
extern VOID JumpTo();
extern VOID OEMDebugDeinit();
extern VOID EnableCache_GP();
extern VOID EnableCache_HS();

//------------------------------------------------------------------------------
//  Local Functions

static BOOL SetupCopySection(ROMHDR *const pTableOfContents);

//------------------------------------------------------------------------------
UINT32 sdram_short_check(void *start, UINT32 len, ram_tst_result* pRes);

//
//  Function:  XLDRMain
//
VOID XLDRMain()
{
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector, ix;
    BLOCK_ID block;
    UINT32 count, size;
    UINT8 *pImage;
    BSP_ARGS *pArgs							= OALPAtoUA(IMAGE_SHARE_ARGS_PA);
    OMAP_PRCM_GLOBAL_PRM_REGS * pPrmGlobal	= OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);
    ULONG dwTemp;

    // Setup global variables
    if (!SetupCopySection(pTOC))
	    goto cleanUp;

    EnableCache_GP();
    
    PlatformSetup();

    XLDRMSGINIT;
    // Print information...
    XLDRMSG(
        TEXT("\r\nMobile Data Terminal NAND X-Loader for TI DM/AM37XX")
        TEXT("\r\n")
        TEXT("Built ") TEXT(__DATE__) TEXT(" at ") TEXT(__TIME__) TEXT("\r\n")
        );
    XLDRMSG(TEXT("Version ") XLDR_VERSION_STRING TEXT("\r\n"));


	//RAM short test
    dwTemp = INREG32(&pPrmGlobal->PRM_RSTST);
 	if( !(dwTemp & (GLOBALWARM_RST | EXTERNALWARM_RST)) )
 	{
		ram_tst_result tr = {0};
		sdram_short_check((VOID*)DEVICE_RAM_CA, DEVICE_RAM_SIZE * 2 , &tr);
		memcpy( &pArgs->ram_test_res, &tr, sizeof(ram_tst_result) );
		if(tr.err)
		{
			XLDRMSG(L"RAM test error: !!!!!!!!!!!  ");
			XLDRMSG(L"\r\n address  ");
			XLDRHEX(tr.address, 8 );		
			XLDRMSG(L"\r\n address_dest  ");
			XLDRHEX(tr.address_dest, 8 );		
			XLDRMSG(L"\r\n value_written  ");
			XLDRHEX(tr.value_written, 8 );		
			XLDRMSG(L"\r\n value_read  ");
			XLDRHEX(tr.value_read, 8 );		
			XLDRMSG(L"\r\n");
		}
	}
    // Open FMD to access NAND
    regInfo.MemBase.Reg[0] = BSP_NAND_REGS_PA;
	// Init FMD with 1-bit ECC for proper XLDR skip
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
    {
        XLDRMSG(L"\r\nFMD_Init failed\r\n");
        goto cleanUp;
    }

    //  Set NAND XLDR bootsector size
    size = IMAGE_XLDR_BOOTSEC_NAND_SIZE;

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
    {
        XLDRMSG(L"\r\nFMD_GetInfo failed\r\n");
        goto cleanUp;
    }

    // Start from NAND start
    block  = 0;
    sector = 0;

    // First skip XLDR boot region.
    
    // NOTE - The bootrom will load the xldr from the first good block starting
    // at zero.  If an uncorrectable ECC error is encountered it will try the next
    // good block.  The last block attempted is the fourth physical block.  The first
    // block is guaranteed good when shipped from the factory, for the first 1000 
    // erase/program cycles.
    
    // Our programming algorithm will place four copies of the xldr into the first
    // four *good* blocks.  If one or more of the first four physical blocks is marked 
    // bad, the XLDR boot region will include the fifth physical block or beyond.  This
    // would result in a wasted block containing a copy of the XLDR that will never be 
    // loaded by the bootrom, but it simplifies the flash management algorithms.
    count = 0;
    while (count < size)
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            count += flashInfo.dwBytesPerBlock;
        block++;
        sector += flashInfo.wSectorsPerBlock;
        }

    // Set address where to place image
    pImage = (UINT8*)IMAGE_STARTUP_IMAGE_PA;

    // Read image to memory
    count = 0;
    while ((count < IMAGE_STARTUP_IMAGE_SIZE) && (block < flashInfo.dwNumBlocks))
        {
        // Skip bad blocks
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) != 0)
            {
            block++;
            sector += flashInfo.wSectorsPerBlock;
            XLDRMSG(L"#");
            continue;
            }                

        // Read sectors in block        
        ix = 0;
        while ((ix++ < flashInfo.wSectorsPerBlock) &&
                (count < IMAGE_STARTUP_IMAGE_SIZE))
            {
            // If a read fails, there is nothing we can do about it
            if (!FMD_ReadSector(sector, pImage, &sectorInfo, 1))
                {
                XLDRMSG(L"$");
                }            

            // Move to next sector
            sector++;
            pImage += flashInfo.wDataBytesPerSector;
            count += flashInfo.wDataBytesPerSector;
            }            

        XLDRMSG(L".");

        // Move to next block
        block++;
        }

    XLDRMSG(L"\r\nJumping to bootloader\r\n");

    // Wait for serial port
    XLDRMSGDEINIT;

    // Jump to image
    JumpTo((VOID*)IMAGE_STARTUP_IMAGE_PA);
    
cleanUp:
    XLDRMSG(L"\r\nHALT\r\n");
    while (TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  OALPAtoVA
//
VOID* OALPAtoVA(UINT32 address, BOOL cached)
{
    return (VOID*)address;
}


//------------------------------------------------------------------------------
//
//  Function:  OALVAtoPA
//
UINT32 OALVAtoPA(VOID *pVA)
{
    return (UINT32)pVA;
}



//------------------------------------------------------------------------------
//
//  Function:  SetupCopySection
//
//  Copies image's copy section data (initialized globals) to the correct
//  fix-up location.  Once completed, initialized globals are valid. Optimized
//  memcpy is too big for X-Loader.
//
static BOOL SetupCopySection(ROMHDR *const pTableOfContents)
{
    BOOL rc = FALSE;
    UINT32 loop, count;
    COPYentry *pCopyEntry;
    UINT8 *pSrc, *pDst;

    if (pTableOfContents == (ROMHDR *const) -1) goto cleanUp;

    for (loop = 0; loop < pTableOfContents->ulCopyEntries; loop++)
        {
        pCopyEntry = (COPYentry *)(pTableOfContents->ulCopyOffset + loop*sizeof(COPYentry));

        count = pCopyEntry->ulCopyLen;
        pDst = (UINT8*)pCopyEntry->ulDest;
        pSrc = (UINT8*)pCopyEntry->ulSource; 
        while (count-- > 0) 
            *pDst++ = *pSrc++;
        count = pCopyEntry->ulDestLen - pCopyEntry->ulCopyLen;
        while (count-- > 0) 
            *pDst++ = 0;
        }

    rc = TRUE;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  NKDbgPrintfW
//
//
VOID NKDbgPrintfW(LPCWSTR pszFormat, ...)  
{
    //  Stubbed out to shrink XLDR binary size
}

//------------------------------------------------------------------------------
//
//  Function:  NKvDbgPrintfW
//
//
VOID NKvDbgPrintfW(LPCWSTR pszFormat, va_list pArgList)
{
    //  Stubbed out to shrink XLDR binary size
}


//------------------------------------------------------------------------------
UINT32 LVAtoPA( VOID* address  )
{
    OAL_ADDRESS_TABLE *pTable = g_oalAddressTable;
    UINT32 va = (UINT32)address;
    UINT32 pa = 0;

    // Virtual address must be in CACHED or UNCACHED regions.
    if (va < 0x80000000 || va >= 0xC0000000) 
    {
        goto cleanUp;
    }

    // Address must be cached, as entries in OEMAddressTable are cached address.
    va = va&~OAL_MEMORY_CACHE_BIT;

    // Search the table for address range
    while (pTable->size != 0) 
    {
        if (va >= pTable->CA && va <= pTable->CA + (pTable->size << 20) - 1) 
        {
			break;
        }
        pTable++;
    }

    // If address table entry is valid compute the PA
    if (pTable->size != 0) 
		pa = pTable->PA + va - pTable->CA;

cleanUp:
    
//	XLDRMSG(L"ADDR: !!!!!!!!!!!  ");//temp!!!!!!!
//	XLDRHEX((UINT32)pa, 8 );		
//	XLDRMSG(L"\r\n");
	return pa;
}
//---------------------------------------------------------------------------------------------------------------

UINT32 sdram_short_check(void *start, UINT32 len, ram_tst_result* pRes)
{
#if 0
	UINT32 *pdw, *pdw2, *pdw_phys;
	UINT32 *pdwe;
	
	UINT16 *pw, *pwe, *pw_phys, Valw;
	UINT8 	*pb, *pbe, *pb2, *pb_phys, Valb;
	
	UINT32	offset, offset2;
	UINT32	i, Val;

//	XLDRMSG(L"ram tst  start!!!!\r\n");//temp!!!!!!1
	if( (UINT32)start < DEVICE_RAM_CA	)
	{
		pRes->err	= 2;
		return 0;
	}
	pRes->err = 0;
//zero mem
	pw	= (UINT16*)start;
	pwe = (UINT16*)((UINT32)start + len);
	
	offset	= 1;	

	while( pw < pwe )
	{
		*((UINT16*)LVAtoPA(pw)) 	= 	0;
		offset 						<<= 1;
		pw = (UINT16*)(offset + DEVICE_RAM_CA);
	};
	
	pb = (UINT8*)start;
	pbe = (UINT8*)((UINT32)start + len);
					
	Valb = 0x55;
				
	for( i = 0; i < 2; ++i )
	{
		pb_phys		= (UINT8*)LVAtoPA(pb);
		*pb_phys	= Valb;
		*(UINT8*)BSP_LAN9115_REGS_PA = ~Valb;	
		if( *pb_phys != Valb )
		{
			pRes->err			= 1;
			pRes->value_read	= *pb_phys;
			break;
		}
			
		Valb		= ~Valb;
		*pb_phys	= Valb;
		*(UINT8*)BSP_LAN9115_REGS_PA = ~Valb;	
			
		if( *pb_phys != Valb )
		{
			pRes->err 			= 1;
			pRes->value_read	= *pb_phys;
			break;
		}
			
		offset	= 1 << i;	
		pb2		= (UINT8*)((UINT32)start + offset);
		
		while( pbe > pb2 )
		{
			pb_phys	= (UINT8*)LVAtoPA(pb2);

			if( *pb_phys == Valb )
			{
				pRes->err			= 1;
				pRes->value_read	= *pb_phys;
				break;
			}
				
			offset	<<= 1;
			pb2		= (UINT8*)((UINT32)start + offset);
		}
		
		if( pRes->err )
			break;
			
		pb2		= 0;
		Valb	= ~Valb;
		
		pb += (1<<i);
	}
	
	if( pRes->err )
	{
		pRes->address		= (UINT32)LVAtoPA(pb);
		pRes->address_dest	= (UINT32)LVAtoPA(pb2);
		pRes->value_written	= Valb;		
		return 0;
	}
	
	//word		
	pw		= (UINT16*)( (UINT32)start + 2 );
	pw_phys = (UINT16*)LVAtoPA(pw);

	Valw		= 0x55AA;
	*pw_phys	= Valw;
	*(UINT16*)BSP_LAN9115_REGS_PA = ~Valw;	
			
	if( *pw_phys == Valw )
	{
		Valw		= ~Valw;
		*pw_phys	= Valw;
		*(UINT16*)BSP_LAN9115_REGS_PA = ~Valw;	

		if( *pw_phys != Valw )
		{
			pRes->err 			= 1;
			pRes->value_read	= *pw_phys;
		}
		else
		{
			pw_phys	= (UINT16*)LVAtoPA( (UINT16*)((UINT32)start + 4) );
			
			if( *pw_phys == Valw )
			{
				pRes->err 			= 1;
				pRes->address_dest	= (UINT32)start + 4;
				pRes->value_read	= *pw_phys;
			}
		}
	}
	else
	{
		pRes->err 			= 1;
		pRes->value_read	= *pw_phys;
	}

	if(	pRes->err )
	{
		pRes->address		= (UINT32)LVAtoPA(pw);
		pRes->value_written	= Valw;
		return 0;
	}		
				
	pdw 	= (UINT32*)start;
	pdwe 	= (UINT32*)((UINT32)start + len);

	offset	= (UINT32)start - DEVICE_RAM_CA + 2;	
	
	Val = 0xAA55AA55;
	do
	{
		pdw_phys = (UINT32*)LVAtoPA(pdw);
		
		Val			= ~Val;
		*pdw_phys	= Val;
		*(UINT32*)BSP_LAN9115_REGS_PA = ~Val;	

		if(*pdw_phys != Val)
		{
			pRes->err 			= 1;
			pRes->value_read	= *pdw_phys;
			break;
		}
		
		Val			= ~Val;		
		*pdw_phys	= Val;
		*(UINT32*)BSP_LAN9115_REGS_PA = ~Val;	

		if(*pdw_phys != Val)
		{
			pRes->err 			= 1;
			pRes->value_read	= *pdw_phys;
			break;
		}
	
		offset2 = offset << 1;
		pdw2	= (UINT32*)( offset2 + DEVICE_RAM_CA);
				
		while( pdw2 < pdwe )
		{
			pdw_phys = (UINT32*)LVAtoPA(pdw2);

			if(*pdw_phys == Val)
			{
				pRes->err	= 1;
				pRes->value_read	= *pdw_phys;
				break;
			}
			offset2 <<= 1;
			pdw2 = (UINT32*)( offset2 + DEVICE_RAM_CA);
		};
		
		if( pRes->err )
			break;
		
		pdw2	= 0;
		Val		= ~Val;

		offset <<= 1;
		pdw		= (UINT32*)( offset + DEVICE_RAM_CA);

	}while(pdw < pdwe);

	if( pRes->err )
	{
		pRes->address		= (UINT32)LVAtoPA(pdw);
		pRes->address_dest	= (UINT32)LVAtoPA(pdw2);
		pRes->value_written	= Val;
		return 0;
	}
//	XLDRMSG(L"ram tst  OK!!!!");//temp!!!!!!1
#endif
	return 1;
}
