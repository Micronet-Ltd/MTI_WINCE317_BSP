// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
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
//  File: spi.c
//
//#include <windows.h>
#include <bsp.h>
#include <spi_boot.h>

//------------------------------------------------------------------------------
SPI_BOOT_DEVICE		g_Spi[4]			= {	{0,0,(OMAP_MCSPI_REGS*)OMAP_MCSPI1_REGS_PA},
											{0,0,(OMAP_MCSPI_REGS*)OMAP_MCSPI2_REGS_PA},
											{0,0,(OMAP_MCSPI_REGS*)OMAP_MCSPI3_REGS_PA},
											{0,0,(OMAP_MCSPI_REGS*)OMAP_MCSPI4_REGS_PA}};
SPI_BOOT_INSTANCE	g_SpiInstance[16]	= {0};
//------------------------------------------------------------------------------
//
//  Function:  SPI_Init
//
//  Called by device manager to initialize device.
//
DWORD SPIInit(DWORD szContext)
{
    DWORD				rc			= 0;
    SPI_BOOT_INSTANCE*	pInstance	= 0;
    DWORD				dwCount		= 0;
	OMAP_MCSPI_REGS*	pSPIRegs;

    RETAILMSG(0, (L"+SPI_Init(0x%08x)\r\n", szContext ));

	pInstance = (SPI_BOOT_INSTANCE*)szContext;
    if(!pInstance)
	{
		return 0;
	}
	pSPIRegs = g_Spi[pInstance->DevNum - 1].pSPIRegs;
    RETAILMSG(0, (L"SPI_Init No %d pSPIRegs %x\r\n", pInstance->DevNum,  pSPIRegs));

    // Reset the SPI controller
    SETREG32(&pSPIRegs->MCSPI_SYSCONFIG, MCSPI_SYSCONFIG_SOFTRESET);

    // Wait until resetting is done
    while( !(INREG32(&pSPIRegs->MCSPI_SYSSTATUS) & MCSPI_SYSSTATUS_RESETDONE)) 
	{
        OALStall(1);

        if (dwCount++ > 0x100)
        {
            // Break out dead lock, something is wrong.
            RETAILMSG(1, (TEXT("SPI: ERROR holding in reset.\n")));
            return FALSE;
        }
    }

    // Disable all interrupts.
    OUTREG32(&pSPIRegs->MCSPI_IRQENABLE, 0);

	// Setup Module Control as master and eSpiMode to unknown so we can change it later.
    g_Spi[pInstance->DevNum - 1].eSpiMode = UNKNOWN;
    OUTREG32(&pSPIRegs->MCSPI_MODULCTRL, 0);

    // Set this driver to internal suspend mode
    OUTREG32(&pSPIRegs->MCSPI_SYSCONFIG,	MCSPI_SYSCONFIG_AUTOIDLE | MCSPI_SYSCONFIG_SMARTIDLE | 
											MCSPI_SYSCONFIG_ENAWAKEUP								); 

	g_Spi[pInstance->DevNum - 1].fInit = 1;
    // Return non-null value
    rc = (DWORD)pInstance;

    RETAILMSG(0, (L"-SPI_Init(rc = %x)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL SPIDeinit(DWORD context)
{
    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
//DWORD SPI_Open(DWORD context, DWORD accessCode, DWORD shareMode)
HANDLE SPIOpen(const WCHAR* pSpiName)
{
	HANDLE ret = 0;
	DWORD i, DevNum = 0;

	RETAILMSG(0,(L"SPIOpen+: \r\n"));

	for( i = 0; i < sizeof(g_SpiInstance)/sizeof(g_SpiInstance[0]); ++i )
	{
		if(0 == g_SpiInstance[i].DevNum)
			break;
	}
	if(i >= sizeof(g_SpiInstance)/sizeof(g_SpiInstance[0]))
		return 0;

	if( 0 == wcscmp(pSpiName, SPI1_DEVICE_NAME) )
	{
		g_SpiInstance[i].DevNum = 1;

	}
	else if( 0 == wcscmp(pSpiName, SPI2_DEVICE_NAME) )
	{
		g_SpiInstance[i].DevNum = 2;
	}
	else if( 0 == wcscmp(pSpiName, SPI3_DEVICE_NAME) )
	{
		g_SpiInstance[i].DevNum = 3;
	}
	else if( 0 == wcscmp(pSpiName, SPI4_DEVICE_NAME) )
	{
		g_SpiInstance[i].DevNum = 4;
	}
	else
		return 0;

	ret = (HANDLE)&g_SpiInstance[i];
	RETAILMSG(0,(L"SPIOpen-: ret %x\r\n", ret));

	return ret;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Close
//
//  This function closes the device context.
//
//BOOL SPI_Close(DWORD context)
VOID SPIClose(HANDLE context)
{
	if( context )
		memset((VOID*)context, 0, sizeof(SPI_BOOT_INSTANCE));
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Configure
//
//  This function configures a SPI channel
//
//BOOL SPI_Configure(DWORD context, DWORD address, DWORD config)
BOOL SPIConfigure(HANDLE hContext, DWORD address, DWORD config)
{
    BOOL				rc			= 0;
	SPI_BOOT_INSTANCE*	pInstance	= (SPI_BOOT_INSTANCE*)hContext;
	OMAP_MCSPI_REGS*	pSPIRegs;

    RETAILMSG(0, (L"SPI_Configure Addr = 0x%x  Config = 0x%x\r\n", address, config));

    //  Check channel
    if( !pInstance || address >= MCSPI_MAX_CHANNELS) 
	{
        RETAILMSG(1, (L"ERROR: SPI_Configure: Incorrect address paramer\r\n"));
        return 0;
    }
	if(!g_Spi[pInstance->DevNum - 1].fInit)
	{
		if( !SPIInit((DWORD)pInstance) )
		{
			RETAILMSG(1,(L"SPIConfigure: SPIInit failed\r\n")); 
			return 0;
		}
	}

	pSPIRegs = g_Spi[pInstance->DevNum - 1].pSPIRegs;

	if(address > 0)
	{
		switch(g_Spi[pInstance->DevNum - 1].eSpiMode)
        {
			case SLAVE:
                RETAILMSG(1, (L"ERROR: SPI_Configure: Incorrect address paramer for slave device\r\n"));
            return 0;

            case UNKNOWN:
				g_Spi[pInstance->DevNum - 1].eSpiMode = MASTER;
            break;

            default:
                break;
        }
    }

    // Configure the channel
    switch( address )
    {
        case 0:
            //  Channel 0 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pSPIRegs->MCSPI_CHCONF0);
            break;

        case 1:
            //  Channel 1 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pSPIRegs->MCSPI_CHCONF1);
            break;

        case 2:
            //  Channel 2 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pSPIRegs->MCSPI_CHCONF2);
            break;

        case 3:
            //  Channel 3 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pSPIRegs->MCSPI_CHCONF3);
            break;
//        default: already checked
//            break;
    }
    OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCONF, config);

	pInstance->config = config;

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  SPI_SetSlaveMode
//
//  This function set the SPI port to act as a slave
//
//BOOL SPI_SetSlaveMode(DWORD context)
BOOL SPISetSlaveMode(HANDLE context)
{
    SPI_BOOT_INSTANCE *pInstance	= (SPI_BOOT_INSTANCE*)context;
	OMAP_MCSPI_REGS*	pSPIRegs;

    RETAILMSG(0, (L"+SSPI_SetSlaveMode.\r\n"));

    // Check if we get correct context
    if(pInstance == NULL) 
	{
        RETAILMSG(1, (L"ERROR: SPI_SetSlaveMode: Incorrect context paramer\r\n"));
        return 0;
    }
	pSPIRegs = g_Spi[pInstance->DevNum - 1].pSPIRegs;

    // once we set eSpiMode to MASTER(someone configured channel 1 to N), we can't 
    // change it back
    if(g_Spi[pInstance->DevNum - 1].eSpiMode == MASTER)
    {
        return 0;
    }
   
    if(g_Spi[pInstance->DevNum - 1].eSpiMode == UNKNOWN)
    {
        OUTREG32(&pSPIRegs->MCSPI_MODULCTRL, MCSPI_MS_BIT);
    }

    return 1;
}


//------------------------------------------------------------------------------
//
//  Function:  SPI_Read
//
//  This function reads data from the device identified by the open context.
//  Note that this is only allowed for MCSPI_CHCONF_TRM_RXONLY channels
//
//DWORD SPI_Read(DWORD context, VOID *pBuffer, DWORD size)
DWORD SPIRead(HANDLE context, DWORD size, VOID *pBuffer)
{
    DWORD dwCount = 0;
	
    SPI_BOOT_INSTANCE*			pInstance = (SPI_BOOT_INSTANCE*)context;
    OMAP_MCSPI_CHANNEL_REGS*	pSPIChannelRegs;
    UCHAR* pData = (UCHAR*)pBuffer;
    DWORD dwWordLen;
    DWORD dwWait;

    RETAILMSG(0, (L"+SPI_Read(0x%08x, 0x%08x, 0x%08x)\r\n", context, pBuffer, size));

    // Check if we get correct context
    if(pInstance == NULL) 
	{
        //DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Read: L"Incorrect context paramer\r\n"));
        return 0;
    }
    pSPIChannelRegs = pInstance->pSPIChannelRegs;

    // Get the word length of the data
    dwWordLen = MCSPI_CHCONF_GET_WL(pInstance->config);

    if(!pInstance->exclusiveAccess)
		SETREG32(&pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    // Write out the data
    for( dwCount = 0; dwCount < size; )
    {
        //  Wait for RX register to fill
        dwWait = 0x500;
        while(dwWait && !(INREG32(&pSPIChannelRegs->MCSPI_CHSTATUS) & MCSPI_CHSTAT_RX_FULL))
        {
            OALStall(1);
            dwWait--;
        }

        //  Check if timeout occured
        if( dwWait == 0 )
        {
            //DEBUGMSG(ZONE_ERROR, (L"SPI_Read timeout\r\n"));
			dwCount = 0;
 			break;
        }


        //  Read in data on byte/word/dword boundaries
        if( dwWordLen > 16 )
        {
            *(UINT32*)(&pData[dwCount]) = INREG32(&pSPIChannelRegs->MCSPI_RX);   
            dwCount += sizeof(UINT32);
        }
        else if( dwWordLen > 8 )
        {
            *(UINT16*)(&pData[dwCount]) = (UINT16)INREG32(&pSPIChannelRegs->MCSPI_RX);   
            dwCount += sizeof(UINT16);
        }
        else
        {
            *(UINT8*)(&pData[dwCount]) = (UINT8) INREG32(&pSPIChannelRegs->MCSPI_RX);   
            dwCount += sizeof(UINT8);
        }   
    }
    
    // Disable the channel.
    if (!pInstance->exclusiveAccess)
		CLRREG32(&pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    RETAILMSG(0, (L"-SPI_Read(rc = %d)\r\n", dwCount));

	return dwCount;
}
//------------------------------------------------------------------------------
//
//  Function:  SPI_Write
//
//  This function writes data to the device.
//  Note that this is only allowed for MCSPI_CHCONF_TRM_TXONLY channels
//
//DWORD SPI_Write(DWORD context, VOID *pBuffer, DWORD size)
DWORD SPIWrite(HANDLE context, DWORD size, VOID *pBuffer)
{
    SPI_BOOT_INSTANCE*			pInstance = (SPI_BOOT_INSTANCE*)context;
    OMAP_MCSPI_CHANNEL_REGS*	pSPIChannelRegs;
    UCHAR* pData = (UCHAR*)pBuffer;
    DWORD dwWordLen;
    DWORD dwCount = 0;
    DWORD dwWait;

    RETAILMSG(0, (L"+SPI_Write(0x%08x, 0x%08x, 0x%08x)\r\n", context, pBuffer, size));

    // Check if we get correct context
    if(pInstance == NULL) 
	{
		//DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Write: L"Incorrect context paramer\r\n" ));
        return 0;
    }

    pSPIChannelRegs = pInstance->pSPIChannelRegs;


    // Get the word length of the data
    dwWordLen = MCSPI_CHCONF_GET_WL(pInstance->config);

    // Enable the channel
    if (!pInstance->exclusiveAccess)
		SETREG32(&pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    // Write out the data
    for( dwCount = 0; dwCount < size; )
    {
        //  Write out data on byte/word/dword boundaries
        if( dwWordLen > 16 )
        {
            OUTREG32(&pSPIChannelRegs->MCSPI_TX, *(UINT32*)(&pData[dwCount]));   
            dwCount += sizeof(UINT32);
        }
        else if( dwWordLen > 8 )
        {
            OUTREG32(&pSPIChannelRegs->MCSPI_TX, *(UINT16*)(&pData[dwCount]));   
            dwCount += sizeof(UINT16);
        }
        else
        {
            OUTREG32(&pSPIChannelRegs->MCSPI_TX, *(UINT8*)(&pData[dwCount]));   
            dwCount += sizeof(UINT8);
        }   

        //  Wait for TX register to empty out
        dwWait = 0x500;
        while(dwWait && !(INREG32(&pSPIChannelRegs->MCSPI_CHSTATUS) & MCSPI_CHSTAT_TX_EMPTY))
        {
            OALStall(1);
            dwWait--;
        }

        //  Check if timeout occured
        if( dwWait == 0 )
        {
            //DEBUGMSG(ZONE_ERROR, (L"SPI_Write timeout\r\n"));

			dwCount = 0;
			break;
        }
    }
    
    // Disable the channel.
    if (!pInstance->exclusiveAccess)
		CLRREG32(&pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    RETAILMSG(0, (L"-SPI_Write(rc = %d)\r\n", dwCount));
    return dwCount;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_WriteRead
//
//  This function writes and reads data to/from the device.  
//  Both buffers must be the same length.
//  Note that this is only allowed for MCSPI_CHCONF_TRM_TXRX channels
//
//DWORD SPI_WriteRead(DWORD context, DWORD size, VOID *pOutBuffer, VOID *pInBuffer		)
DWORD SPIWriteRead(HANDLE context, DWORD size, VOID *pOutData, VOID *pInData)
{
    SPI_BOOT_INSTANCE*	pInstance = (SPI_BOOT_INSTANCE*)context;
    DWORD		dwWordLen;
    DWORD		dwCount = 0;
    DWORD		dwWait;
	OMAP_MCSPI_REGS*	pSPIRegs;

    RETAILMSG(1, (L"+SPI_WriteRead(0x%08x, 0x%08x)\r\n", context, size));

    // Check if we get correct context
    if(!pInstance) 
	{
        RETAILMSG (1, (L"ERROR: SPI_Write: Incorrect context paramer\r\n"  ));
        return 0;
    }
	pSPIRegs = g_Spi[pInstance->DevNum - 1].pSPIRegs;

    // Get the word length of the data
    dwWordLen = MCSPI_CHCONF_GET_WL(pInstance->config);
    
    if (!pInstance->exclusiveAccess)
		SETREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    // Write out the data
    for( dwCount = 0; dwCount < size; )
    {
        //  Write out data on byte/word/dword boundaries
        if( dwWordLen > 16 )
        {
            OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_TX, *(UINT32*)(&((UINT8*)pOutData)[dwCount]));   
        }
        else if( dwWordLen > 8 )
        {
            OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_TX, *(UINT16*)(&((UINT8*)pOutData)[dwCount]));   
        }
        else
        {
            OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_TX, *(UINT8*)(&((UINT8*)pOutData)[dwCount]));   
        }   

        //  Wait for TX register to empty out
        dwWait = 500;
        while(dwWait && !(INREG32(&pInstance->pSPIChannelRegs->MCSPI_CHSTATUS) & MCSPI_CHSTAT_TX_EMPTY))
        {
            OALStall(1);
            dwWait--;
        }

        //  Check if timeout occured
        if( dwWait == 0 )
        {
            RETAILMSG(1, (L"SPI_WriteRead write timeout\r\n"));
            
           dwCount = 0;
		   break;
        }


        //  Wait for RX register to fill
        dwWait = 0x500;
        while(dwWait && !(INREG32(&pInstance->pSPIChannelRegs->MCSPI_CHSTATUS) & MCSPI_CHSTAT_RX_FULL))
        {
            OALStall(1);
            dwWait--;
        }

        //  Check if timeout occured
        if( dwWait == 0 )
        {
            RETAILMSG(1, (L"SPI_WriteRead read timeout\r\n"));
            
            dwCount = 0;
			break;
        }

        //  Read in data on byte/word/dword boundaries
        if( dwWordLen > 16 )
        {
            *(UINT32*)(&((UINT8*)pInData)[dwCount]) = INREG32(&pInstance->pSPIChannelRegs->MCSPI_RX);   
            dwCount += sizeof(UINT32);
        }
        else if( dwWordLen > 8 )
        {
            *(UINT16*)(&((UINT8*)pInData)[dwCount]) = (UINT16) INREG32(&pInstance->pSPIChannelRegs->MCSPI_RX);   
            dwCount += sizeof(UINT16);
        }
        else
        {
            *(UINT8*)(&((UINT8*)pInData)[dwCount]) = (UINT8) INREG32(&pInstance->pSPIChannelRegs->MCSPI_RX);   
            dwCount += sizeof(UINT8);
        }   
    }
    
    // Disable the channel.
    if (!pInstance->exclusiveAccess)
        CLRREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    return dwCount;
}



//#### ------------------------------------------------------------------------------
//
//  Function:  SPI_WriteReadCS
//
//  This function writes and reads data to/from the device.  
//  InBuffer must be at least inSize length, OutBuffer length >= inSize + outSize
//  Note that this is only allowed for MCSPI_CHCONF_TRM_TXRX channels
//  !!! chip-select is driven by the MCSPI_CHxCONF FORCE mode - Manual SPIM_CSX assertion
//  !!! sizes in bytes !
//  !!! Remeber that SPI send MSBit first !!! 
//  Write/Read bytes order is from 0 to size-1. MSbit first
//DWORD SPI_WriteReadCS(	DWORD context, 
DWORD SPIWriteReadCS(	HANDLE context, 
						DWORD inSize,
						UINT8 *pInBuffer,
						DWORD outSize,
						const UINT8 *pOutBuffer )
{
    SPI_BOOT_INSTANCE *pInstance = (SPI_BOOT_INSTANCE*)context;

	OMAP_MCSPI_CHANNEL_REGS*	pSPIChannelRegs;
    UCHAR* pInData = (UCHAR*)pInBuffer;
    UCHAR* pOutData = (UCHAR*)pOutBuffer;
    DWORD dwWordLen;
    DWORD dwCount = 0;
	DWORD bytesCnt;
    DWORD dwWait;
	UINT32   configOrig, config, moduleCtrlOrig; 
	register UINT32  regOut;
	UINT32           regIn;
	OMAP_MCSPI_REGS*	pSPIRegs;

	RETAILMSG(0, (L"\r\n+SPI_WriteReadCS(0x%08x, In = %d, Out = %d)\r\n", context, inSize, outSize));

    // Check if we get correct context
    if (pInstance == NULL) 
	{
		RETAILMSG(1, (L"ERROR: SPI_Write: Incorrect context paramer\r\n"));
        return 0;
    }

	pSPIRegs		= g_Spi[pInstance->DevNum - 1].pSPIRegs;
    pSPIChannelRegs = pInstance->pSPIChannelRegs;

    // Get the word length of the data
	// First, transmit (out) 
	// initial SPI word length depends on outSize
	configOrig = pInstance->config;
    config     = configOrig;
	dwWordLen = MCSPI_CHCONF_GET_WL(configOrig);
    //dwWordLen = MCSPI_CHCONF_GET_WL(pInstance->config);

    // Enable the channel
    if (!pInstance->exclusiveAccess)
        SETREG32(&pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

	RETAILMSG(0, (L"\r\n+SPI_WriteReadCS CHCONF = 0x%08X (0x%08X)\r\n", INREG32(&pSPIChannelRegs->MCSPI_CHCONF), configOrig ) );

	    // Set controller in single access mode
	    // Restore after ???
	moduleCtrlOrig = INREG32(&pSPIRegs->MCSPI_MODULCTRL);
	RETAILMSG(0, (L"+SPI_WriteReadCS MODULCTRL = 0x%08X (0x%08X)\r\n", INREG32(&pSPIRegs->MCSPI_MODULCTRL), moduleCtrlOrig ));

    SETREG32(&pSPIRegs->MCSPI_MODULCTRL, MCSPI_SINGLE_BIT);

	// OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);

    // Set Tx Only mode
	if( outSize > 0 )
	{
     config &= (~MCSPI_CHCONF_TRM_MASK);
	 config |= MCSPI_CHCONF_TRM_TXONLY;
     //SETREG32(&MCSPI_CHCONF, MCSPI_CHCONF_FORCE | MCSPI_CHCONF_TRM_TXONLY );
	}

	// Assert the chip select
     config &= (~MCSPI_CHCONF_FORCE_MASK);
	 config |= MCSPI_CHCONF_FORCE;

	 OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);   

    //////////////// Write only out the data
    bytesCnt = outSize;
	dwCount = 0;
	while( bytesCnt > 0 )
	{
		//outSize
		config &= (~MCSPI_CHCONF_WL_MASK);

		if( bytesCnt >= 4 )
		{
          config |= MCSPI_CHCONF_WL(32);
		  OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);   
		  regOut = (pOutData[dwCount]<<24);
		  regOut |= (pOutData[dwCount+1]<<16);
		  regOut |= (pOutData[dwCount+2]<<8);
		  regOut |= pOutData[dwCount+3];
          OUTREG32(&pSPIChannelRegs->MCSPI_TX, regOut);
          RETAILMSG(0, (L"+SPI_WriteReadCS Write32 = 0x%08X\r\n", regOut) );
		  dwCount += sizeof(UINT32);
		  bytesCnt -= sizeof(UINT32);
		}
		else if( bytesCnt >= 2 ) // 2 or 3
		{
		  //SETREG32(&pSPIChannelRegs->MCSPI_CHCONF, MCSPI_CHCONF_WL(16) );
          config |= MCSPI_CHCONF_WL(16);
          OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);
		  regOut = (pOutData[dwCount]<<8);
		  regOut |= pOutData[dwCount+1];
		  OUTREG32(&pSPIChannelRegs->MCSPI_TX, regOut);   
		  RETAILMSG(0, (L"+SPI_WriteReadCS Write16 = 0x%08X\r\n", regOut) );
		  dwCount += sizeof(UINT16);
		  bytesCnt -= sizeof(UINT16);
		}
		else if( bytesCnt == 1 )
		{
		  //SETREG32(&pSPIChannelRegs->MCSPI_CHCONF, MCSPI_CHCONF_WL(8) );
          config |= MCSPI_CHCONF_WL(8);
          OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);
		  regOut = pOutData[dwCount];
		  OUTREG32(&pSPIChannelRegs->MCSPI_TX, regOut);
		  RETAILMSG(0, (L"+SPI_WriteReadCS Write8 = 0x%08X\r\n", regOut) );
		  dwCount += sizeof(UINT8);
		  bytesCnt -= sizeof(UINT8);
		}
		else // outSize == 0
		{
		 break;
		}

        //  Wait for TX register to empty out
        dwWait = 0x500;
        while(dwWait && !(INREG32(&pSPIChannelRegs->MCSPI_CHSTATUS) & MCSPI_CHSTAT_EOT /*MCSPI_CHSTAT_TX_EMPTY*/))
        {
            OALStall(1);
            dwWait--;
        }

        //  Check if timeout occured
        if( dwWait == 0 )
        {
            //DEBUGMSG(ZONE_ERROR, (L"SPI_WriteReadCS write timeout\r\n"));
			RETAILMSG(1, (L"SPI_WriteReadCS write timeout\r\n"));
            
            goto clean;
        }

	} ////////////////// End of Write only out the data
   
	// Exchange data
    if( inSize > 0 )
	{
     config &= (~MCSPI_CHCONF_TRM_MASK);
	 config |= MCSPI_CHCONF_TRM_TXRX;
	 OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);   
     //SETREG32(&pSPIChannelRegs->MCSPI_CHCONF, MCSPI_CHCONF_FORCE | MCSPI_CHCONF_TRM_TXONLY );
	}

     
    // Write out the data
    bytesCnt = inSize;
	while( bytesCnt > 0 )
    {
		config &= (~MCSPI_CHCONF_WL_MASK);

        //  Write out data on byte/word/dword boundaries
		if( bytesCnt >= 4 )
		{
          config |= MCSPI_CHCONF_WL(32);
		  OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);   
		  regOut = (pOutData[dwCount]<<24);
		  regOut |= (pOutData[dwCount+1]<<16);
		  regOut |= (pOutData[dwCount+2]<<8);
		  regOut |= pOutData[dwCount+3];
		  OUTREG32(&pSPIChannelRegs->MCSPI_TX, regOut);   
		  RETAILMSG(0, (L"+SPI_WriteReadCS Xchng32: W = 0x%08X, ", regOut) );
		}
		else if( bytesCnt >= 2 ) // 2 or 3
		{
		  //SETREG32(&pSPIChannelRegs->MCSPI_CHCONF, MCSPI_CHCONF_WL(16) );
          config |= MCSPI_CHCONF_WL(16);
          OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);
		  regOut = (pOutData[dwCount]<<8);
		  regOut |= pOutData[dwCount+1];
		  OUTREG32(&pSPIChannelRegs->MCSPI_TX, regOut);   
		  RETAILMSG(0, (L"+SPI_WriteReadCS Xchng16: W = 0x%08X, ", regOut) );
		}
		else if( bytesCnt == 1 )
		{
		  //SETREG32(&pSPIChannelRegs->MCSPI_CHCONF, MCSPI_CHCONF_WL(8) );
          config |= MCSPI_CHCONF_WL(8);
          OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);
		  regOut = pOutData[dwCount];
		  OUTREG32(&pSPIChannelRegs->MCSPI_TX, regOut);
		  RETAILMSG(0, (L"+SPI_WriteReadCS Xchng8: W = 0x%08X, ", regOut) );
		}
		else // outSize == 0
		{
		 break;
		}

        //  Wait for TX register to empty out
        dwWait = 0x500;
        while(dwWait && !(INREG32(&pSPIChannelRegs->MCSPI_CHSTATUS) & MCSPI_CHSTAT_EOT /*MCSPI_CHSTAT_TX_EMPTY*/ ))
        {
            OALStall(1);
            dwWait--;
        }

        //  Check if timeout occured
        if( dwWait == 0 )
        {
            //DEBUGMSG(ZONE_ERROR, (L"SPI_WriteRead write timeout\r\n"));
//			RETAILMSG(1, (L"SPI_WriteReadCS write-xchng timeout, dwWait = %d, timout = %d\r\n", dwWait, pDevice->timeout*1000));
            
            goto clean;
        }


        //  Wait for RX register to fill
        dwWait = 0x500;
        while(dwWait && !(INREG32(&pSPIChannelRegs->MCSPI_CHSTATUS) & MCSPI_CHSTAT_RX_FULL))
        {
            OALStall(1);
            dwWait--;
        }

        //  Check if timeout occured
        if( dwWait == 0 )
        {
			RETAILMSG(1, (L"SPI_WriteReadCS read-xchng timeout\r\n"));
   
            goto clean;
        }


        //  Read in data on byte/word/dword boundaries
        if( bytesCnt >= 4 )
        {
			regIn =  INREG32(&pSPIChannelRegs->MCSPI_RX);
			pInData[dwCount-outSize] = (UINT8)(0x000000FF &( regIn>>24));
            pInData[dwCount+1-outSize] = (UINT8)(0x000000FF & (regIn>>16));
			pInData[dwCount+2-outSize] = (UINT8)(0x000000FF & (regIn>>8));
			pInData[dwCount+3-outSize] = (UINT8)(0x000000FF & (regIn));

			RETAILMSG(0, (L"R = 0x%08X, [%d : %08X][%d : %08X][%d : %08X][%d : %08X]\r\n", regIn, 
                          dwCount-outSize, pInData[dwCount-outSize], 
						  dwCount+1-outSize, pInData[dwCount+1-outSize], 
						  dwCount+2-outSize, pInData[dwCount+2-outSize], 
						  dwCount+3-outSize, pInData[dwCount+3-outSize]
				          ) );

		  dwCount += sizeof(UINT32);
		  bytesCnt -= sizeof(UINT32);
        }
        else if( bytesCnt >= 2 ) // 2 or 3
        {
			regIn =  INREG32(&pSPIChannelRegs->MCSPI_RX);
			pInData[dwCount-outSize] = (UINT8)(0x000000FF & (regIn>>8));
			pInData[dwCount+1-outSize] = (UINT8)(0x000000FF & (regIn));

			RETAILMSG(0, (L"R = 0x%08X, [%d : %08X][%d : %08X]\r\n", regIn, 
                          dwCount-outSize, pInData[dwCount-outSize], 
						  dwCount+1-outSize, pInData[dwCount+1-outSize]
				          ) );

		  dwCount += sizeof(UINT16);
		  bytesCnt -= sizeof(UINT16);
        }
        else if( bytesCnt == 1 )
        {
			regIn =  INREG32(&pSPIChannelRegs->MCSPI_RX);
			pInData[dwCount-outSize] = (UINT8)(0x000000FF & (regIn));

			RETAILMSG(0, (L"R = 0x%08X, [%d : %08X]\r\n", regIn, 
                          dwCount-outSize, pInData[dwCount-outSize]
				          ) );

		  dwCount += sizeof(UINT8);
		  bytesCnt -= sizeof(UINT8);
        }   
		else // outSize == 0
		{
		 break;
		}

    }
    
clean:
	// De-assert chipselect
     config &= (~MCSPI_CHCONF_FORCE_MASK );
	 OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, config);   

    // Restore registers
    OUTREG32(&pSPIRegs->MCSPI_MODULCTRL, moduleCtrlOrig);
    OUTREG32(&pSPIChannelRegs->MCSPI_CHCONF, configOrig); 

    // Disable the channel.
    if (!pInstance->exclusiveAccess)
        CLRREG32(&pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    
//    DEBUGMSG(ZONE_FUNCTION, (L"-SPI_WriteReadCS(rc = %d)\r\n", dwCount));
	RETAILMSG(0, (L"-SPI_WriteReadCS(rc = %d)\r\n", dwCount));
    return dwCount;
}


//------------------------------------------------------------------------------
//
//  Function:  CentextRestore()
//
//
/*
BOOL ContextRestore(SPI_BOOT_INSTANCE *pInstance)
{
    SPI_BOOT_INSTANCE *pDevice = pInstance->pDevice;
    DWORD   dwCount=0;
    BOOL bRetVal = FALSE;
	OMAP_MCSPI_REGS*	pSPIRegs;

    // Get Device
    pDevice = pInstance->pDevice;
	pSPIRegs		= g_Spi[pInstance->DevNum - 1].pSPIRegs;

    SETREG32(&pDevice->pSPIRegs->MCSPI_SYSCONFIG, MCSPI_SYSCONFIG_SOFTRESET);
    // Wait until resetting is done
    while ( !(INREG32(&pDevice->pSPIRegs->MCSPI_SYSSTATUS) & MCSPI_SYSSTATUS_RESETDONE))
        {
        Sleep (1);
        if (dwCount++>0x100)
            {
            // Break out dead lock, something is wrong.
            ERRORMSG (TRUE, (TEXT("SPI: ERROR holding in reset.\n")));
            return FALSE;
            }
        }

    // Disable all interrupts.
    OUTREG32(&pDevice->pSPIRegs->MCSPI_IRQENABLE, 0);
    // Clear interrupts.
    OUTREG32(&pDevice->pSPIRegs->MCSPI_IRQSTATUS, 0xFFFF);
    // Setup Module Control as master or slave
    OUTREG32(&pDevice->pSPIRegs->MCSPI_MODULCTRL, (pDevice->eSpiMode==SLAVE) ? MCSPI_MS_BIT : 0);

    OUTREG32(&pDevice->pSPIRegs->MCSPI_SYSCONFIG, MCSPI_SYSCONFIG_AUTOIDLE |
        MCSPI_SYSCONFIG_SMARTIDLE | MCSPI_SYSCONFIG_ENAWAKEUP);

    // Configure the channel
    switch( pInstance->address)
    {
        case 0:
            //  Channel 0 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pDevice->pSPIRegs->MCSPI_CHCONF0);
            OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCONF, pInstance->config);
            break;

        case 1:
            //  Channel 1 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pDevice->pSPIRegs->MCSPI_CHCONF1);
            OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCONF, pInstance->config);
            break;

        case 2:
            //  Channel 2 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pDevice->pSPIRegs->MCSPI_CHCONF2);
            OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCONF, pInstance->config);
            break;

        case 3:
            //  Channel 3 configuration
            pInstance->pSPIChannelRegs = (OMAP_MCSPI_CHANNEL_REGS*)(&pDevice->pSPIRegs->MCSPI_CHCONF3);
            OUTREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCONF, pInstance->config);
            break;

        default:
            break;
    }

    SpiDmaRestore(pInstance);
    return TRUE;
}*/
//------------------------------------------------------------------------------
//
//  Function:  SPI_LockController
//
//  Called by external clients to place the controller in single access mode
//
//BOOL SPI_LockController(DWORD context, DWORD dwTimeout)
BOOL SPILockController(DWORD context, DWORD dwTimeout)
{
    SPI_BOOT_INSTANCE *pInstance = (SPI_BOOT_INSTANCE*)context;
	OMAP_MCSPI_REGS*	pSPIRegs;

    RETAILMSG(0, (L"SPI_LockController\r\n"));
    
    // Check if we get correct context
    if(pInstance == NULL)
	{
        RETAILMSG(1, (L"ERROR: SPI_LockController: Incorrect context paramer\r\n"));
        return 0;
    }
	pSPIRegs = g_Spi[pInstance->DevNum - 1].pSPIRegs;

    // Set controller in single access mode
    SETREG32(&pSPIRegs->MCSPI_MODULCTRL, MCSPI_SINGLE_BIT);
    
    // Set flag indicating we own the controller lock
    pInstance->exclusiveAccess = TRUE;
       
    RETAILMSG(0, (L"-SPI_LockController\r\n"));
    return 1;
}
        
//------------------------------------------------------------------------------
//
//  Function:  SPI_UnlockController
//
//  Called by external clients to release controller from single access mode
//
//BOOL SPI_UnlockController(DWORD context)
BOOL SPIUnlockController(DWORD context)
{
    SPI_BOOT_INSTANCE *pInstance = (SPI_BOOT_INSTANCE*)context;
	OMAP_MCSPI_REGS*	pSPIRegs;

    RETAILMSG(0, (L"SPI_UnlockController\r\n"));
    
    // Check if we get correct context
    if (pInstance == NULL)
	{
        RETAILMSG(1, (L"ERROR: SPI_UnlockController: Incorrect context paramer\r\n"));
        return 0;
    }
   
    // Check to see if we have exclusive access
    if (!pInstance->exclusiveAccess)
    {
        RETAILMSG(1, (L"ERROR: SPI_UnlockController: Don't own SPI lock!\r\n"));
        return 0;
    }
 	pSPIRegs = g_Spi[pInstance->DevNum - 1].pSPIRegs;

    // Release single access mode
    CLRREG32(&pSPIRegs->MCSPI_MODULCTRL, MCSPI_SINGLE_BIT);
    
    // Clear lock flag
    pInstance->exclusiveAccess = FALSE;
     
    RETAILMSG(1, (L"-SPI_UnlockController\r\n"));
    return 1;
}
        
//------------------------------------------------------------------------------
//
//  Function:  SPI_EnableChannel
//
//  Called by external clients to enable a channel in single access mode
//  This function both enables the channel and asserts the chip select
//  to the target device
//
//BOOL SPI_EnableChannel(DWORD context)
BOOL SPIEnableChannel(DWORD context)
{
    BOOL rc = FALSE;
    SPI_BOOT_INSTANCE *pInstance = (SPI_BOOT_INSTANCE*)context;

    RETAILMSG(0, (L"+SPI_EnableChannel\r\n"));
    
    // Check if we get correct context
	if(pInstance == NULL)
	{
		RETAILMSG(1, (L"ERROR: SPI_EnableChannel: Incorrect context paramer\r\n"));
        return 0;
    }
    
    // Check to see if we have exclusive access
    if (!pInstance->exclusiveAccess)
    {
        RETAILMSG(0, (L"ERROR: SPI_EnableChannel: Not in single access mode!!\r\n"));
        return 0;
    }
  
    // Enable the channel
    SETREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);
    // Assert the chip select
    SETREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCONF, MCSPI_CHCONF_FORCE);
    
    RETAILMSG(0, (L"-SPI_EnableChannel\r\n"));
    return 1;  
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_DisableChannel
//
//  Called by external clients to disable a channel in single access mode
//  This function both disables the channel and de-asserts the chip select
//  to the target device
//
//BOOL SPI_DisableChannel(DWORD context)
BOOL SPIDisableChannel(HANDLE context)
{
    SPI_BOOT_INSTANCE *pInstance = (SPI_BOOT_INSTANCE*)context;

    RETAILMSG(0, (L"+SPI_DisableChannel\r\n"));
    
    // Check if we get correct context
	if(pInstance == NULL)
	{
		RETAILMSG(1, (L"ERROR: SPI_DisableChannel: Incorrect context paramer\r\n"));
        return 0;
    }
    
    // Check to see if we have exclusive access
    if(!pInstance->exclusiveAccess)
    {
		RETAILMSG(0, (L"ERROR: SPI_DisableChannel: Not in single access mode!!\r\n"));
        return 0;
    }

    // De-assert the chip select
    CLRREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCONF, MCSPI_CHCONF_FORCE);
    // Disable the channel
    CLRREG32(&pInstance->pSPIChannelRegs->MCSPI_CHCTRL, MCSPI_CHCONT_EN);

    RETAILMSG(0, (L"-SPI_DisableChannel\r\n"));
    return 1;  
}
//------------------------------------------------------------------------------

