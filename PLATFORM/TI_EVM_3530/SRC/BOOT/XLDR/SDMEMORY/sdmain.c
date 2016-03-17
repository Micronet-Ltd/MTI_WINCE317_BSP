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
//  This file contains X-Loader implementation for OMAP37XX
//
#include <bsp.h>
#include <blcommon.h>
#include <constants.h>
#include <bsp_version.h>
#include <boot_args.h>
#include <i2c.h>
#include <oal_i2c.h>

//------------------------------------------------------------------------------

#define XLDRMSGINIT         OEMDebugInit()
#define XLDRMSGDEINIT       OEMDebugDeinit()
#define XLDRMSG(msg)        OEMWriteDebugString(msg)
#define XLDRHEX(val, len)   OEMWriteDebugHex(val, len)


//------------------------------------------------------------------------------
//  Global variables
ROMHDR * volatile const pTOC = (ROMHDR *)-1; 

const volatile DWORD dwOEMHighSecurity      = OEM_HIGH_SECURITY_GP;

BOOT_CFG g_bootCfg;

//------------------------------------------------------------------------------
//  External Functions

extern VOID PlatformSetup();
extern VOID JumpTo();
extern VOID OEMDebugDeinit();
extern VOID EnableCache_GP();
extern VOID EnableCache_HS();

extern UINT32 BLSDCardDownload(BOOT_CFG *pBootCfg, OAL_KITL_DEVICE *pBootDevices);
extern BOOL   BLSDCardReadData(ULONG size, UCHAR *pData);

//------------------------------------------------------------------------------
//  Local Functions

static BOOL SetupCopySection(ROMHDR *const pTableOfContents);
static void OutputNumHex(unsigned long n, long depth);

static void xShowT2Reg(void* hTwl, DWORD RegAddr);

//#define XSHOW_T2_REG(reg) { \
//    xShowT2Reg(hTwl, reg, TEXT(#reg)); \
//}

#define XSHOW_T2_REG(reg) { xShowT2Reg(hTwl, reg);}
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
UINT32 sdram_short_check(void *start, UINT32 len)
{
#if 0
	UINT32 *pdw, *pdw2, *pdw_phys;
	UINT32 *pdwe;
	
	UINT16 *pw, *pwe, *pw_phys, Valw;
	UINT8 	*pb, *pbe, *pb2, *pb_phys, Valb;
	
	UINT32	offset, offset2;
	UINT32	i, Val;

	XLDRMSG(L"\r\nRAM short test  start:\r\n");
	if( (UINT32)start < DEVICE_RAM_CA	)
	{
		return 0;
	}
//zero mem
	pw	= (UINT16*)start;
	pwe = (UINT16*)((UINT32)start + len);
	
	offset	= 1;	

	while( pw < pwe )
	{
		*((UINT16*)LVAtoPA(pw)) = 	0;
		offset 					<<= 1;
		pw						= (UINT16*)(offset + (UINT32)start);
	};
	
	pb = (UINT8*)start;
	pbe = (UINT8*)((UINT32)start + len);
					
	Valb = 0x55;
				
	for( i = 0; i < 2; ++i )
	{
		pb_phys		= (UINT8*)LVAtoPA(pb);
		*pb_phys	= Valb;
		*(UINT8*)BSP_LAN9115_REGS_PA	= ~Valb;
			
		if( *pb_phys != Valb )
		{

			XLDRMSG(L"Failed address ");
			XLDRHEX(pb_phys, 8 );		
			XLDRMSG(L"(");
			XLDRHEX(*pb_phys, 2 );		
			XLDRMSG(L" - ");
			XLDRHEX(Valb, 2 );		
			XLDRMSG(L" w)\r\n");
		}
			
		Valb		= ~Valb;
		*pb_phys	= Valb;
		*(UINT8*)BSP_LAN9115_REGS_PA	= ~Valb;
			
		if( *pb_phys != Valb )
		{
			XLDRMSG(L"Failed address ");
			XLDRHEX(pb_phys, 8 );		
			XLDRMSG(L"(");
			XLDRHEX(*pb_phys, 2 );		
			XLDRMSG(L" - ");
			XLDRHEX(Valb, 2 );		
			XLDRMSG(L" w)\r\n");
		}
		offset	= 1 << i;	
		pb2		= (UINT8*)((UINT32)start + offset);
		
		while( pbe > pb2 )
		{
			pb_phys	= (UINT8*)LVAtoPA(pb2);

			if( *pb_phys == Valb )
			{
				XLDRMSG(L"Failed address ");
				XLDRHEX(pb_phys, 8 );		
				XLDRMSG(L" - ");
				XLDRHEX(LVAtoPA(pb), 8 );	
				XLDRMSG(L" w (");
				XLDRHEX(Valb, 2 );		
				XLDRMSG(L")\r\n");
			}
				
			offset	<<= 1;
			pb2		= (UINT8*)((UINT32)start + offset);
		}
		
		pb2		= 0;
		Valb	= ~Valb;
		
		pb += (1<<i);
	}
	
	//word		
	pw		= (UINT16*)( (UINT32)start + 2 );
	pw_phys = (UINT16*)LVAtoPA(pw);

	Valw		= 0x55AA;
	*pw_phys	= Valw;
	*(UINT16*)BSP_LAN9115_REGS_PA =  ~Valw;		
	
	if( *pw_phys == Valw )
	{
		Valw		= ~Valw;
		*pw_phys 	= Valw;
		*(UINT16*)BSP_LAN9115_REGS_PA =  ~Valw;		

		if( *pw_phys != Valw )
		{
			XLDRMSG(L"Failed address ");
			XLDRHEX(pw_phys, 8 );		
			XLDRMSG(L"(");
			XLDRHEX(*pw_phys, 4 );		
			XLDRMSG(L" - ");
			XLDRHEX(Valw, 4 );		
			XLDRMSG(L" w)\r\n");
		}
		else
		{
			pw_phys	= (UINT16*)LVAtoPA( (UINT16*)((UINT32)start + 4) );
			
			if( *pw_phys == Valw )
			{
				XLDRMSG(L"Failed address ");
				XLDRHEX(pw_phys, 8 );		
				XLDRMSG(L" - ");
				XLDRHEX(LVAtoPA(pw), 8 );	
				XLDRMSG(L" w (");
				XLDRHEX(Valw, 4 );		
				XLDRMSG(L")\r\n");
			}
		}
	}
	else
	{
		XLDRMSG(L"Failed address ");
		XLDRHEX(pw_phys, 8 );		
		XLDRMSG(L"(");
		XLDRHEX(*pw_phys, 4 );		
		XLDRMSG(L" - ");
		XLDRHEX(Valw, 4 );		
		XLDRMSG(L" w)\r\n");
	}
			
	pdw 	= (UINT32*)start;
	pdwe 	= (UINT32*)((UINT32)start + len);

	offset	= (UINT32)start - DEVICE_RAM_CA + 2;	
	
	Val = 0xAA55AA55;
	do
	{
		pdw_phys = (UINT32*)LVAtoPA(pdw);
		
		Val = ~Val;
		*pdw_phys = Val;
		*(UINT32*)BSP_LAN9115_REGS_PA = ~Val;

		if(*pdw_phys != Val)
		{
			XLDRMSG(L"Failed address ");
			XLDRHEX(pdw_phys, 8 );		
			XLDRMSG(L"(");
			XLDRHEX(*pdw_phys, 8 );		
			XLDRMSG(L" - ");
			XLDRHEX(Val, 8 );		
			XLDRMSG(L" w)\r\n");
		}
		
		Val			= ~Val;		
		*pdw_phys	= Val;
		*(UINT32*)BSP_LAN9115_REGS_PA = ~Val;

		offset2 = offset << 1;
		pdw2	= (UINT32*)(offset2 + + (UINT32)start);
				
		while(pdw2 < pdwe)
		{
			pdw_phys = (UINT32*)LVAtoPA(pdw2);
			if(*pdw_phys == Val)
			{
				//err	= 1;
				XLDRMSG(L"Failed address ");
				XLDRHEX(pdw_phys, 8 );		
				XLDRMSG(L" - ");
				XLDRHEX(LVAtoPA(pdw), 8 );	
				XLDRMSG(L" w (");
				XLDRHEX(Val, 8 );		
				XLDRMSG(L")\r\n");
			}

			offset2 <<= 1;
			pdw2 = (UINT32*)(offset2 + (UINT32)start);
		};
			
		pdw_phys = (UINT32*)LVAtoPA(pdw);
		*(UINT32*)BSP_LAN9115_REGS_PA = ~Val;

		if(*pdw_phys != Val)
		{
			XLDRMSG(L"Failed address ");
			XLDRHEX(pdw_phys, 8 );		
			XLDRMSG(L"(");
			XLDRHEX(*pdw_phys, 8 );		
			XLDRMSG(L" - ");
			XLDRHEX(Val, 8 );		
			XLDRMSG(L" w)\r\n");
		}

		pdw2	= 0;
		Val		= ~Val;

		offset <<= 1;
		pdw		= (UINT32*)(offset + (UINT32)start);

	}while(pdw < pdwe);
#endif
	return 1;
}

UINT32 sdram_check(void *start, UINT32 len)
{
#if 0
	UINT32 *pdw;
	UINT32 *pdwe;
	UINT16 *pw;
	UINT16 *pwe;
	UINT8 *pb;
	UINT8 *pbe;
	UINT32 Tmp, TmpAddr, i;
	
//	void *start = (void*)DEVICE_RAM_CA;
//	UINT32 len = DEVICE_RAM_SIZE << 1;
	
//	if(access == 4)
	XLDRMSG(L"Start RAM test 32 bits:\r\n");

	pdwe = (UINT32*)((UINT32)start + len);
		
	for(i = 0; i < 2; i++)
	{
		pdw = (UINT32 *)start;
		
		do
		{
			TmpAddr = (UINT32)LVAtoPA(pdw);
			*(UINT32*)TmpAddr = (i ? ~TmpAddr : TmpAddr);

		}while(++pdw < pdwe);
		
		pdw = (UINT32*)start;
		do
		{
			TmpAddr = (UINT32)LVAtoPA(pdw);
			Tmp = i ? ~TmpAddr : TmpAddr;

			*(UINT32*)BSP_LAN9115_REGS_PA = ~Tmp;

			if(*(UINT32*)TmpAddr != Tmp)
			{
				XLDRMSG(L"Failed address ");
				XLDRHEX(TmpAddr, 8 );		
				XLDRMSG(L"(");
				XLDRHEX(*(UINT32*)TmpAddr, 8 );
				if(i)
					XLDRMSG(L" inv");
				XLDRMSG(L")\r\n");
			}
		}while(++pdw < pdwe);
	}

//	else if(access == 2)
	XLDRMSG(L"Start RAM test 16 bits:\r\n");

	pwe = (UINT16*)((UINT32)start + len);
		
	for(i = 0; i < 2; ++i)
	{
		pw = (UINT16*)start;

		do
		{
			TmpAddr = (UINT32)LVAtoPA(pw);
			*(UINT16*)TmpAddr = (UINT16)(i ? ~TmpAddr : TmpAddr);
		}while(++pw < pwe);
		
		pw = (UINT16 *)start;
		do
		{
			TmpAddr = (UINT32)LVAtoPA(pw);
			Tmp = i ? ~TmpAddr : TmpAddr;
			*(UINT16*)BSP_LAN9115_REGS_PA = (UINT16)(~Tmp);

			if(*(UINT16*)TmpAddr != (UINT16)Tmp)
			{
				XLDRMSG(L"Failed address ");
				XLDRHEX(TmpAddr, 8 );		
				XLDRMSG(L"(");
				XLDRHEX(*(UINT16*)TmpAddr, 4 );		
				if(i)
					XLDRMSG(L" inv");
				XLDRMSG(L")\r\n");
			}
		}while(++pw < pwe);
	}
//	access = 1;
	XLDRMSG(L"Start RAM test 8 bits:\r\n");
	pbe = (UINT8 *)((UINT32)start + len);

	for(i = 0; i < 2; ++i)
	{

		pb = (UINT8 *)start;
		
		do
		{
			TmpAddr = (UINT32)LVAtoPA(pb);
			*(UINT8*)TmpAddr = (UINT8)(i ? ~TmpAddr : TmpAddr);
		}while(++pb < pbe);
		
		pb = (UINT8 *)start;
		do
		{
			TmpAddr = (UINT32)LVAtoPA(pb);
			Tmp = (UINT8)(i ? ~TmpAddr : TmpAddr);
			*(UINT8*)BSP_LAN9115_REGS_PA = (UINT8)(~Tmp);

			if(*(UINT8*)TmpAddr != (UINT8)Tmp)
			{
				//errrrrrrrrr
				XLDRMSG(L"Failed address ");
				XLDRHEX(TmpAddr, 8 );		
				XLDRMSG(L"(");
				XLDRHEX(*(UINT8*)TmpAddr, 2 );		
				if(i)
					XLDRMSG(L" inv");
				XLDRMSG(L")\r\n");
			}
		}while(++pb < pbe);
	}
#endif

	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

INT	SerReadWithCount(UINT8* pKey, UINT32 Counter)
{
	INT		err = 0;
	UINT8	key = (UINT8)OEM_DEBUG_READ_NODATA;
	UINT32  counter = 0;
	while( (key = (UINT8)OEMReadDebugByte()) == (UINT8)OEM_DEBUG_READ_NODATA && 
			( counter++ < Counter )							);

	if( key == (UINT8)OEM_DEBUG_READ_NODATA	||
		key == (UINT8)OEM_DEBUG_COM_ERROR	) 
	{
		err = key;
		*pKey = 0;
	}
	else
		*pKey = key;

    return err;
}


//------------------------------------------------------------------------------
//
//  Function:  XLDRMain
//
VOID XLDRMain()
{
    OMAP_GPTIMER_REGS *pTimerRegs;
    UINT8 key = 0;
    
    // Setup global variables
    if (!SetupCopySection(pTOC)) goto CleanUp;

    //  Enable cache based on device type
    if( dwOEMHighSecurity == OEM_HIGH_SECURITY_HS )
    {
        EnableCache_HS();
    }
    else
    {
        EnableCache_GP();
    }
    
    PlatformSetup();

    // Initialize debug serial output
    XLDRMSGINIT;

    //OALLogSetZones( 
    //           (1<<OAL_LOG_VERBOSE)  |
    //           (1<<OAL_LOG_INFO)     |
    //           (1<<OAL_LOG_ERROR)    |
    //           (1<<OAL_LOG_WARN)     |
    //           (1<<OAL_LOG_FUNC)     |
    //           (1<<OAL_LOG_IO)       |
    //           0);

    // Print information...
    XLDRMSG(
        TEXT("\r\nCE317 SD X-Loader for DM/AM3730\r\n")
        TEXT("Built ") TEXT(__DATE__) TEXT(" at ") TEXT(__TIME__) TEXT("\r\n")
        TEXT("Version ") MLDR_VERSION_STRING TEXT("\r\n"));

    //XLDRMSG(TEXT("XLDRMain: Setup the platform\r\n\r\n"));

#if 0
	SerReadWithCount(&key, 40000);
	if('9' == key)
	{
		UINT32 counter = 0;
		XLDRMSG(L"Please, release the key...\r\n");
		while(counter < 100000)
		{
			key = (UINT8)OEMReadDebugByte();
			//if(key == (UINT8)OEM_DEBUG_COM_ERROR)
			//{
			//	XLDRMSGDEINIT;
			//	XLDRMSGINIT;
			//	XLDRMSG(L"UART reinit\r\n");
			//}
			//else 
			if(key == (UINT8)OEM_DEBUG_READ_NODATA)
				counter++;
			else
				counter = 0;
		}
	    
		sdram_short_check((void*)DEVICE_RAM_CA, DEVICE_RAM_SIZE << 1);
		sdram_check((void*)DEVICE_RAM_CA, DEVICE_RAM_SIZE << 1);
	}
#endif
//-----
    pTimerRegs = (OMAP_GPTIMER_REGS *)OALPAtoUA(OMAP_GPTIMER1_REGS_PA);
    // Soft reset GPTIMER
    OUTREG32(&pTimerRegs->TIOCP, SYSCONFIG_SOFTRESET);
    while ((INREG32(&pTimerRegs->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0)
        ;

    // Enable posted mode
    OUTREG32(&pTimerRegs->TSICR, GPTIMER_TSICR_POSTED);
    // Start timer
    OUTREG32(&pTimerRegs->TCLR, GPTIMER_TCLR_AR|GPTIMER_TCLR_ST);
    
    // need to init OAL tick functions

#if 0
    {
        // dump SDRC registers
        OMAP_SDRC_REGS* pSdrc = OALPAtoUA(OMAP_SDRC_REGS_PA);

        XLDRMSG(TEXT("SDRC_POWER 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_POWER), 8);
        XLDRMSG(TEXT("\r\nSDRC_MCFG_0 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_MCFG_0), 8);
        XLDRMSG(TEXT("\r\nSDRC_MCFG_1 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_MCFG_1), 8);
        XLDRMSG(TEXT("\r\nSDRC_SHARING 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_SHARING), 8);
        XLDRMSG(TEXT("\r\nSDRC_RFR_CTRL_0 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_RFR_CTRL_0), 8);
        XLDRMSG(TEXT("\r\nSDRC_RFR_CTRL_1 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_RFR_CTRL_1), 8);
        XLDRMSG(TEXT("\r\nSDRC_ACTIM_CTRLA_0 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_ACTIM_CTRLA_0), 8);
        XLDRMSG(TEXT("\r\nSDRC_ACTIM_CTRLA_1 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_ACTIM_CTRLA_1), 8);
        XLDRMSG(TEXT("\r\nSDRC_ACTIM_CTRLB_0 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_ACTIM_CTRLB_0), 8);
        XLDRMSG(TEXT("\r\nSDRC_ACTIM_CTRLB_1 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_ACTIM_CTRLB_1), 8);
        XLDRMSG(TEXT("\r\nSDRC_MR_0 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_MR_0), 8);
        XLDRMSG(TEXT("\r\nSDRC_DLLA_CTRL 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_DLLA_CTRL), 8);
        XLDRMSG(TEXT("\r\nSDRC_DLLB_CTRL 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_DLLB_CTRL), 8);
        XLDRMSG(TEXT("\r\nSDRC_EMR2_0 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_EMR2_0), 8);
        XLDRMSG(TEXT("\r\nSDRC_EMR2_1 0x"));
        OutputNumHex(INREG32(&pSdrc->SDRC_EMR2_1), 8);
        XLDRMSG(TEXT("\r\n"));
    }
#endif

#if 0
    {
        // test memory at IMAGE_STARTUP_IMAGE_PA
        DWORD * pImageStartupAddrPa = (DWORD *) IMAGE_STARTUP_IMAGE_PA;
        DWORD i;
    
        #define MEMORY_TEST_LENGTH_DWORDS   (0x000c0000 / sizeof(DWORD))
        for (i = 0; i < MEMORY_TEST_LENGTH_DWORDS; i++)
        {
            *(pImageStartupAddrPa + i) = i;
        }    
        for (i = 0; i < MEMORY_TEST_LENGTH_DWORDS; i++)
        {
            if (*(pImageStartupAddrPa + i) != i)
            {
                XLDRMSG(TEXT("Bad BL Image Memory!\r\n"));
                break;
            }
        }
    }    
#endif
    
    memset(&g_bootCfg, 0, sizeof(g_bootCfg));
    wcscpy(g_bootCfg.filename, L"ebootsd.nb0");

    XLDRMSG(TEXT("open ebootsd.nb0 file\r\n"));
            
    if (BL_ERROR == BLSDCardDownload(&g_bootCfg, NULL))
    {
        XLDRMSG(TEXT("SD boot failed to open file\r\n"));
        goto CleanUp;
    }

    XLDRMSG(TEXT("read ebootsd.nb0 file\r\n"));

    if (BL_ERROR == BLSDCardReadData(0x000c0000, (UCHAR *) IMAGE_STARTUP_IMAGE_PA))
    {
        XLDRMSG(TEXT("SD boot failed to read file\r\n"));
        goto CleanUp;
    }

    XLDRMSG(TEXT("jumping to ebootsd image\r\n"));

    // Wait for serial port
    XLDRMSGDEINIT;

    // Jump to image
    JumpTo((VOID*)IMAGE_STARTUP_IMAGE_PA);
    
CleanUp:
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

    if (pTableOfContents == (ROMHDR *const) -1) goto CleanUp;

    for (loop = 0; loop < pTableOfContents->ulCopyEntries; loop++)
        {
        pCopyEntry = (COPYentry *)(pTableOfContents->ulCopyOffset + loop*sizeof(COPYentry));

        count = pCopyEntry->ulCopyLen;
        pDst = (UINT8*)pCopyEntry->ulDest;
        pSrc = (UINT8*)pCopyEntry->ulSource; 
        while (count-- > 0) *pDst++ = *pSrc++;
        count = pCopyEntry->ulDestLen - pCopyEntry->ulCopyLen;
        while (count-- > 0) *pDst++ = 0;
        }

    rc = TRUE;

CleanUp:    
    return rc;
}

/*****************************************************************************
*
*
*   @func   void    |   OutputNumHex | Print the hex representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.
*
*   @parm   long | depth |
*               Minimum number of digits to print.
*
*/
static void OutputNumHex(unsigned long n, long depth)
{
    if (depth) 
    {
        depth--;
    }
    
    if ((n & ~0xf) || depth) 
    {
        OutputNumHex(n >> 4, depth);
        n &= 0xf;
    }
    
    if (n < 10) 
    {
        OEMWriteDebugByte((BYTE)(n + '0'));
    } 
    else 
    { 
        OEMWriteDebugByte((BYTE)(n - 10 + 'A'));
    }
}


/*****************************************************************************
*
*
*   @func   void    |   OutputNumDecimal | Print the decimal representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.
*
*/
static void OutputNumDecimal(unsigned long n)
{
    if (n >= 10) 
    {
        OutputNumDecimal(n / 10);
        n %= 10;
    }
    OEMWriteDebugByte((BYTE)(n + '0'));
}

//------------------------------------------------------------------------------
//
//  Function:  NKvDbgPrintfW
//
//
VOID NKvDbgPrintfW(LPCWSTR pszFormat, va_list vl)
{
    // Stubbed out to shrink XLDR binary size
    TCHAR c;

    while (*pszFormat) 
    {
        c = *pszFormat++;
        switch ((BYTE)c) 
        { 
            case '%':
                c = *pszFormat++;
                switch (c) 
                { 
                    case 'x':
                        OutputNumHex(va_arg(vl, unsigned long), 0);
                        break;

                    case 'B':
                        OutputNumHex(va_arg(vl, unsigned long), 2);
                        break;

                    case 'H':
                        OutputNumHex(va_arg(vl, unsigned long), 4);
                        break;

                    case 'X':
                        OutputNumHex(va_arg(vl, unsigned long), 8);
                        break;

                    case 'd': 
                    {
                        long    l;

                        l = va_arg(vl, long);
                        if (l < 0) 
                        { 
                            OEMWriteDebugByte('-');
                            l = - l;
                        }
                        OutputNumDecimal((unsigned long)l);
                        break;
                    }

                    case 'u':
                        OutputNumDecimal(va_arg(vl, unsigned long));
                        break;

                    case 's':
                        OEMWriteDebugString(va_arg(vl, LPWSTR));
                        break;

                    case '%':
                        OEMWriteDebugByte('%');
                        break;

                    case 'c':
                        c = va_arg(vl, TCHAR);
                        OEMWriteDebugByte((BYTE)c);
                        break;

                    default:
                        OEMWriteDebugByte(' ');
                        break;
                }
                break;

            case '\n':
                OEMWriteDebugByte('\r');
                // fall through

            default:
                OEMWriteDebugByte((BYTE)c);
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:  NKDbgPrintfW
//
//
VOID NKDbgPrintfW(LPCWSTR pszFormat, ...)  
{
    // Stubbed out to shrink XLDR binary size
    va_list vl;

    va_start(vl, pszFormat);
    NKvDbgPrintfW(pszFormat, vl);
    va_end(vl);
}

//------------------------------------------------------------------------------



static void xShowT2Reg(void* hTwl, DWORD RegAddr)
{
    UINT8 Value = 0;
    OALTritonRead(hTwl, RegAddr, &Value);
    OALLog(L"T2 reg(0x%X) = 0x%X\r\n", RegAddr, Value);
}
