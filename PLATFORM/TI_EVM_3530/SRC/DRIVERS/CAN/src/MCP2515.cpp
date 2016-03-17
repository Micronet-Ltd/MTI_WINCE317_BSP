// ================================================================================================
//
// Giant Steps Ltd.
// 
// MCP2515 CAN Bus Controller relative functions
//
// ================================================================================================

#include <Windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <initguid.h>

//#include <Platform.h>
#include <bsp.h>

#include "MISC.h"
#include <SPI.h>
#include <gpio.h>
#include "MCP2515.h"
#include "Debug.h"


// Globals ========================================================================================

// SPI Object

volatile HANDLE  g_hSPI = NULL;
//volatile HANDLE  g_hGpio = NULL;

// Virtual Address Base to Chip Select of 
// MCP2515 Reset and SPI Switch
//volatile WORD		*g_pCSBaseAddr		=	NULL;

// Critical Section to defend MCP2515 SPI from concurrent accesses
CRITICAL_SECTION g_csSPIMCP2515;

volatile int InPowerHandler;

// Functions ======================================================================================

// Function name	: MCP2515Init
// Description	    : Initialize MCP2515 to default settings and 
//					  Bit Timing according configuration from Registry
// Return type		: BOOL MCP2515Init 
// Argument         : PCANREGSETTINGS pCANREGSETTINGS

BOOL MCP2515Init ( CAN_INIT_CONTEXT    *pCxt, PCANREGSETTINGS pCANREGSETTINGS )
{
    DWORD   config;
	BOOL	bRetVal		= FALSE;

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515Init\r\n"));

	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515Init\r\n"));


	InitializeCriticalSection ( &pCxt->csSPIMCP2515 );

#if 1
	// SPI Object
	pCxt->hSPI = SPIOpen(SPI4_DEVICE_NAME);
	if ( NULL == pCxt->hSPI )
	{
		RETAILMSG(1, (L"CAN: MCP2515Init, SPIOpen (SPI4) FAILED!!! err#0x%04X\r\n", GetLastError()));
		goto END;
	}

    g_hSPI = pCxt->hSPI;
	g_csSPIMCP2515 = pCxt->csSPIMCP2515;

    config =    MCSPI_PHA_ODD_EDGES |
                MCSPI_POL_ACTIVEHIGH |    // MCSPI_POL_ACTIVEHIGH
                MCSPI_CHCONF_CLKD(3) |        // 48 MHz/8 = 6 MHz
                MCSPI_CSPOLARITY_ACTIVELOW |
                MCSPI_CHCONF_WL(32) |         // SPI_BIT_COUNT == 32 
                MCSPI_CHCONF_TRM_TXRX |
				MCSPI_CHCONF_DMAW_DISABLE |
				MCSPI_CHCONF_DMAR_DISABLE |
                MCSPI_CHCONF_DPE0;

	if( FALSE == SPIConfigure(pCxt->hSPI, 0, config) )
	{
      RETAILMSG(1,(L"CAN: MCP2515Init, SPIConfigure (SPI4) FAILED!!! err#0x%04X\r\n", GetLastError())); 
      goto END;
	}


    // Switch Power On of CAN Controller
	GPIOClrBit(pCxt->hGpio , CAN_ON);

	// Turn CAN Transceiver On
	//GPIOSetBit(pCxt->hGpio , CAN_TR_ON);

	// 8.1 Oscillator Startup Timer	maintains reset
    // for the first 128 OSC1 clock cycles after power-up or a
    // wake-up from Sleep mode occurs.
	// freq = 16 Mhz, 128 clocks = 8 us. 
	Sleep(2);
	
	// Hardware Reset
	MCP2515HwReset(pCxt);

#endif /* 0 */

	// Test GPIO_159
#if 0
   GPIOSetMode(pCxt->hGpio , GPIO_156, GPIO_DIR_INPUT);
   GPIOSetMode(pCxt->hGpio , GPIO_158, GPIO_DIR_INPUT);
   GPIOSetMode(pCxt->hGpio , GPIO_159, GPIO_DIR_INPUT);
   GPIOSetMode(pCxt->hGpio , GPIO_161, GPIO_DIR_INPUT);
   // 156, 158, 159, 161
#endif


   	if ( FALSE == MCP2515SetModeSPI ( OPMODE_CONFIGURATION ) )
	{
		RETAILMSG(1,(L"CAN: MCP2515Init, MCP2515SetModeSPI FAILED!!! err#0x%04X\r\n", GetLastError())); 
		goto END;
	}


	if	(
		(FALSE == MCP2515SetMask(RXM0SIDH_ADDR, pCANREGSETTINGS->SIDMASK0, pCANREGSETTINGS->EIDMASK0)) || 
		(FALSE == MCP2515SetMask(RXM1SIDH_ADDR, pCANREGSETTINGS->SIDMASK1, pCANREGSETTINGS->EIDMASK1)) || 
		(FALSE == MCP2515SetFilter(RXF0SIDH_ADDR, pCANREGSETTINGS->SIDFILTER0, pCANREGSETTINGS->EIDFILTER0, pCANREGSETTINGS->FILSWITCH0)) || 
		(FALSE == MCP2515SetFilter(RXF1SIDH_ADDR, pCANREGSETTINGS->SIDFILTER1, pCANREGSETTINGS->EIDFILTER1, pCANREGSETTINGS->FILSWITCH1)) ||
		(FALSE == MCP2515SetFilter(RXF2SIDH_ADDR, pCANREGSETTINGS->SIDFILTER2, pCANREGSETTINGS->SIDFILTER2, pCANREGSETTINGS->FILSWITCH2)) ||
		(FALSE == MCP2515SetFilter(RXF3SIDH_ADDR, pCANREGSETTINGS->SIDFILTER3, pCANREGSETTINGS->SIDFILTER3, pCANREGSETTINGS->FILSWITCH3)) ||
		(FALSE == MCP2515SetFilter(RXF4SIDH_ADDR, pCANREGSETTINGS->SIDFILTER4, pCANREGSETTINGS->SIDFILTER4, pCANREGSETTINGS->FILSWITCH4)) ||
		(FALSE == MCP2515SetFilter(RXF5SIDH_ADDR, pCANREGSETTINGS->SIDFILTER5, pCANREGSETTINGS->SIDFILTER5, pCANREGSETTINGS->FILSWITCH5))
		)
	{
		RETAILMSG(1,(L"CAN: MCP2515Init, MCP2515Setxxx FAILED!!! err#0x%04X\r\n", GetLastError())); 
		goto END;
	}

	// CAN Configuration, Bit Timing Issues, Wake-Up Filter and SOF Pin
	if ( FALSE == MCP2515SetCNF ( pCANREGSETTINGS ) )
	{
		RETAILMSG(1,(L"CAN: MCP2515Init, MCP2515SetCNF FAILED!!! err#0x%04X\r\n", GetLastError())); 
		goto END;
	}

	// CLKOUT Pin Disable
	MCP2515BitModifySPI(CANCTRL_ADDR, CLKEN_m, (OFF << CLKEN_s));

	// Set Roll Over Feature
	MCP2515BitModifySPI(RXB0CTRL_ADDR, FILHIT_m, (0x04 << FILHIT_s));

	// Set Receive Buffer #0 Operating Mode //RXB_OPMODE_SNIFFER
	// Test mode
	pCANREGSETTINGS->RXMB0 = RXB_OPMODE_ALL;
	if ( FALSE == MCP2515SetRXM ( RXB0CTRL_ADDR, pCANREGSETTINGS->RXMB0 ) )
	{
		goto END;
	}

	// Set Receive Buffer #1 Operating Mode
	// Test mode
	pCANREGSETTINGS->RXMB1 = RXB_OPMODE_ALL;
	if ( FALSE == MCP2515SetRXM ( RXB1CTRL_ADDR, pCANREGSETTINGS->RXMB1 ) )
	{
		RETAILMSG(1,(L"CAN: MCP2515Init, MCP2515SetRXM FAILED!!! err#0x%04X\r\n", GetLastError())); 
		goto END;
	}

	// Enable Default Selected Interrupts
	MCP2515BitModifySPI(CANINTE_ADDR, DEFAULT_INTR_ENABLE, DEFAULT_INTR_ENABLE);

#if 0
	if ( FALSE == MCP2515SetModeSPI ( OPMODE_CONFIGURATION ) )
	{
		RETAILMSG(1,(L"CAN: MCP2515Init, MCP2515SetModeSPI FAILED!!! err#0x%04X\r\n", GetLastError())); 
		goto END;
	}
#endif

	// Enter low-power mode
	// ... Interesting to compare power consumption versus 
	//     Switch Power On of CAN Controller !?


	bRetVal	= TRUE;

END:
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515Init, bRetVal=0x%04X\r\n", bRetVal));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515Init, bRetVal=0x%04X\r\n", bRetVal));
	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515DeInit
// Description	    : DeInitializes System Resources for MCP2515
// Return type		: BOOL MCP2515DeInit 

BOOL MCP2515DeInit ( CAN_INIT_CONTEXT    *pCxt )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +MCP2515DeInit\r\n" ) ) );

	BOOL bRetVal = TRUE;


	// Turn CAN Controller Off
	GPIOSetBit(pCxt->hGpio , nCAN_RESET);
    GPIOSetBit(pCxt->hGpio , CAN_ON);
	// Turn CAN Transceiver Off
	//GPIOClrBit(pCxt->hGpio , CAN_TR_ON);


	// Close SPI bus
	SPIClose(pCxt->hSPI);
	
	DeleteCriticalSection ( &pCxt->csSPIMCP2515 );


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -MCP2515DeInit, bRetVal=0x%04X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}


// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515HwReset
// Description	    : Performes Hardware Reset for MCP2515
// Return type		: void MCP2515HwReset 

void MCP2515HwReset( CAN_INIT_CONTEXT    *pCxt )
{
	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515HwReset\r\n"));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515HwReset\r\n"));

	if(!InPowerHandler) 
	 EnterCriticalSection ( &pCxt->csSPIMCP2515 );

	 // Hold in Reset State
    //*( g_pCSBaseAddr + ( CAN_MCP2515_RESET_OFFSET >> 1 ) ) = MCP2515_RESET_ON;
	GPIOClrBit(pCxt->hGpio , nCAN_RESET);

	DelayMS(2, InPowerHandler );
	//Sleep(2); // even 2 micro sec is enough

	// Release Reset State
    //*( g_pCSBaseAddr + ( CAN_MCP2515_RESET_OFFSET >> 1 ) ) = MCP2515_RESET_OFF;
	GPIOSetBit(pCxt->hGpio , nCAN_RESET);

	DelayMS(5, InPowerHandler );
	//Sleep(2); // it's strange, but 128 microseconds (128*max_Tosc) are not enough
	// May be Sleep(1) doesn't guarantee 1 ms delay. it may be less!

	if(!InPowerHandler) 
	 LeaveCriticalSection ( &pCxt->csSPIMCP2515 );

	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515HwReset\r\n"));
	RETAILMSG( CAN_DRV_DBG, (L"CAN: -MCP2515HwReset\r\n"));
}


// ##### Michael
void MCP2515PowerOff(CAN_INIT_CONTEXT    *pCxt)
{
  RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515PowerOff\r\n"));
  GPIOSetBit(pCxt->hGpio , CAN_ON);
  // Turn CAN Transceiver Off
  RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515PowerOff - Turn CAN Transceiver & Controller Off\r\n"));
  //GPIOClrBit(pCxt->hGpio , CAN_TR_ON);

  RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515PowerOff\r\n"));
}


void MCP2515PowerOn(CAN_INIT_CONTEXT    *pCxt)
{
  RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515PowerOn\r\n"));

  GPIOClrBit(pCxt->hGpio , CAN_ON);
  // Turn CAN Transceiver On
  RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515PowerOn - Turn CAN Transceiver & Controller On\r\n"));
  //GPIOClrBit(pCxt->hGpio , CAN_TR_ON);

  RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515PowerOff\r\n"));
}


BOOL MCP2515PowerStatus(CAN_INIT_CONTEXT    *pCxt)
{

 // RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515PowerStatus\r\n"));

 
  if( GPIOGetBit(pCxt->hGpio , CAN_ON) == 0 )
  {
    RETAILMSG(CAN_DRV_DBG, (L"CAN: MCP2515PowerStatus - CAN Transceiver & Controller Power On\r\n"));
	return(TRUE);
  }
  else
  {
    RETAILMSG(CAN_DRV_DBG, (L"CAN: MCP2515PowerStatus - CAN Transceiver & Controller Power Off\r\n"));
	return(FALSE);
  }

  // RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515PowerStatus\r\n"));
}



// Function name	: MCP2515ReadSPI
// Description	    : Read N <= 16 Bytes from specific address
// Return type		: BOOL MCP2515ReadSPI 
// Argument         : unsigned __int8 *pBuffer
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int8 uCount

BOOL MCP2515ReadSPI(void *pBuffer, unsigned __int8 uAddress, unsigned __int8 uCount)
{
	BOOL bRetVal = FALSE;
	UINT8  uCmd[32] = {0};
	unsigned char *pRaw	= (unsigned char*)pBuffer;
	int  len;

    RETAILMSG( 0 /*CAN_DRV_DBG*/, (_T("CAN: +MCP2515ReadSPI, Buffer = 0x%X, Address=0x%04X, Count=0x%04X\r\n"), pBuffer, uAddress, uCount));

	if((uCount <= 0) || (uCount > 16))
	{
		RETAILMSG( 1 /*CAN_DRV_DBG*/, (_T("CAN: MCP2515ReadSPI, Wrong Count=0x%04X Value !!!\r\n"), uCount));
		return(FALSE);
	}

    if (!InPowerHandler) 
	  EnterCriticalSection(&g_csSPIMCP2515);

	uCmd[0] = MCP2515SPICMD_READ;
	uCmd[1] = uAddress;

    if( (len = SPIWriteReadCS(g_hSPI, uCount, pRaw, 2, &uCmd[0] )) == (uCount+2) )
	{
		bRetVal = TRUE;
	}
	else
	{
     RETAILMSG(1 /*CAN_DRV_DBG*/, (_T("CAN: MCP2515ReadSPI, read failure, len = %d\r\n"), len ));
	}

	RETAILMSG(0 /*CAN_DRV_DBG*/, (_T("CAN: -MCP2515ReadSPI, Buffer = 0x%X, Address=0x%04X, Count=0x%04X, bRetVal=0x%04X\r\n"), pBuffer, uAddress, uCount, bRetVal));

	if (!InPowerHandler) 
	  LeaveCriticalSection(&g_csSPIMCP2515);

	return(bRetVal);
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515WriteSPI
// Description	    : Write N Bytes to specific address
// Return type		: BOOL MCP2515WriteSPI 
// Argument         : unsigned __int8 *pBuffer
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int8 uCount

BOOL MCP2515WriteSPI(void *pBuffer, unsigned __int8 uAddress, unsigned __int8 uCount)
{
	BOOL   bRetVal = FALSE;
	int    i;
	UINT8  uTempBuff[24] = { 0 };
	UINT8  uTempDataBuff[16] = { 0 };
	char   *pRaw = (char*)pBuffer;


	DEBUGMSG(ZONE_STEPS, (_T("CAN: +MCP2515WriteSPI, Address=0x%04X, Count=0x%04X\r\n"), uAddress, uCount));
	RETAILMSG( 0 /*CAN_DRV_DBG*/, (_T("CAN: +MCP2515WriteSPI, Buffer = 0x%X, Address=0x%04X, Count=%d\r\n"), pBuffer, uAddress, uCount));

	if ( ( 16 < uCount ) || ( 0 >= uCount ) )
	{
		DEBUGMSG ( ZONE_ERROR, ( _T ( "CAN: MCP2515WriteSPI, Wrong Count=0x%04X Value !!!\r\n" ), uCount ) );
        RETAILMSG( 1, ( _T ( "CAN: MCP2515WriteSPI, Wrong Count=%d Value !!!\r\n" ), uCount ) );
        return(FALSE); 
	}

	if(!InPowerHandler) 
	  EnterCriticalSection ( &g_csSPIMCP2515 );


	//DEBUGMSG_ARRAY ( ZONE_SPIE,( _T( "CAN: MCP2515WriteSPI : " ) ), pBuffer, uCount );

	uTempBuff[0] = MCP2515SPICMD_WRITE;
	uTempBuff[1] = uAddress;
	for(i=0; i<uCount; i++)
 	 uTempBuff[2+i] = pRaw[i];

    if( SPIWriteReadCS(g_hSPI, 0, &uTempDataBuff[0], uCount+2, &uTempBuff[0] ) == (uCount+2) )
	 {
	  bRetVal = TRUE;
	 } 
	else
	 {
      RETAILMSG(1 /*CAN_DRV_DBG*/, (_T("CAN: MCP2515WriteSPI, write failure\r\n")));
	 }

	DEBUGMSG ( ZONE_STEPS,( _T ( "CAN: -MCP2515WriteSPI, bRetVal=0x%04X\r\n" ), bRetVal ) );
	RETAILMSG( 0 /*CAN_DRV_DBG*/,( _T ( "CAN: -MCP2515WriteSPI, bRetVal=0x%04X\r\n" ), bRetVal ) );

	if (!InPowerHandler) 
	 LeaveCriticalSection ( &g_csSPIMCP2515 );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515RequestToSendSPI
// Description	    : Performes Rquest To Send for Specific Tx Buffer
// Return type		: BOOL MCP2515RequestToSendSPI 
// Argument         : TXBID TxBufferID

BOOL MCP2515RequestToSendSPI ( TXBID TxBufferID )
{
	BOOL			bRetVal = FALSE;
	unsigned __int8 uCmd	= 0;
	UINT8  uCmdBuff[4] = { 0 };
	UINT8  uDataBuff[4] = { 0 };

	DEBUGMSG(ZONE_STEPS, (_T("CAN: +MCP2515RequestToSendSPI, TXBID=0x%04X\r\n"), TxBufferID));
	RETAILMSG( CAN_DRV_DBG, (_T("CAN: +MCP2515RequestToSendSPI, TXBID=0x%04X\r\n"), TxBufferID));
	

	switch ( TxBufferID )
	{
	case TXB0:
		uCmdBuff[0] = ( MCP2515SPICMD_RTS | 0x01 );
		break;

	case TXB1:
		uCmdBuff[0] = ( MCP2515SPICMD_RTS | 0x02 );
		break;

	case TXB2:
		uCmdBuff[0] = ( MCP2515SPICMD_RTS | 0x04 );
		break;

	default:
		//DEBUGMSG(ZONE_STEPS,( _T ( "CAN: -MCP2515WriteSPI, bRetVal= FALSE\r\n" )) );
		RETAILMSG( 1,( _T ( "CAN: -MCP2515WriteSPI, bRetVal= FALSE\r\n" )) );
		return(FALSE);
	}

	if (!InPowerHandler)
	 EnterCriticalSection ( &g_csSPIMCP2515 );

	//SPIWrite(g_hSPI, 1, &uCmd );
	//g_pSPIObj->Enable();
	//g_pSPIObj->Write(uCmd);
	//g_pSPIObj->Disable();
    if( SPIWriteReadCS(g_hSPI, 0, &uDataBuff[0], 1, &uCmdBuff[0] ) == 1 )
	 {
	  bRetVal = TRUE;
	 } 
	else
	 {
      RETAILMSG(1 /*CAN_DRV_DBG*/, (_T("CAN: MCP2515WriteSPI, write failure\r\n")));
	 }
	
	DEBUGMSG ( ZONE_STEPS,( _T ( "CAN: -MCP2515WriteSPI, bRetVal=0x%04X\r\n" ), bRetVal ) );
	RETAILMSG( CAN_DRV_DBG,( _T ( "CAN: -MCP2515WriteSPI, bRetVal=0x%04X\r\n" ), bRetVal ) );

	if (!InPowerHandler)
	 LeaveCriticalSection ( &g_csSPIMCP2515 );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515BitModifySPI
// Description	    : Modifies specific Bit or Group of Bits
// Return type		: BOOL MCP2515BitModifySPI 
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int8 uMask
// Argument         : unsigned __int8 uData

void MCP2515BitModifySPI(unsigned __int8 uAddress, unsigned __int8 uMask, unsigned __int8 uData)
{
	UINT8  uCmd[4] = {0};
    UINT8  uDataBuff[4] = { 0 };

	DEBUGMSG(ZONE_STEPS, (_T("CAN: +MCP2515BitModifySPI, ADDR=0x%04X, MASK=0x%04X, DATA=0x%04X\r\n"), uAddress, uMask, uData));
	RETAILMSG( 0 /*CAN_DRV_DBG*/, (_T("CAN: +MCP2515BitModifySPI, ADDR=0x%04X, MASK=0x%04X, DATA=0x%04X\r\n"), uAddress, uMask, uData));

	if (!InPowerHandler) EnterCriticalSection(&g_csSPIMCP2515);

	uCmd[0] = MCP2515SPICMD_BITMODIFY;
	uCmd[1] = uAddress;
	uCmd[2] = uMask;
	uCmd[3] = uData;

	//SPIWrite(g_hSPI, 4, uCmd );
	//g_pSPIObj->Enable(InPowerHandler);
	//g_pSPIObj->Write(uCmd, 4);
	//g_pSPIObj->Disable(InPowerHandler);

    if( SPIWriteReadCS(g_hSPI, 0, &uDataBuff[0], 4, &uCmd[0] ) == 0 )
	 {
      RETAILMSG(1 /*CAN_DRV_DBG*/, (_T("CAN: MCP2515WriteSPI, write failure\r\n")));
	 }

	DEBUGMSG(ZONE_STEPS, (_T("CAN: -MCP2515BitModifySPI\r\n")));
	RETAILMSG( 0 /*CAN_DRV_DBG*/, (_T("CAN: -MCP2515BitModifySPI\r\n")));

	if (!InPowerHandler) LeaveCriticalSection(&g_csSPIMCP2515);

	return;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515ResetSPI
// Description	    : Reset internal Registers to Default State, 
//					  Sets Configuration Mode
// Return type		: void MCP2515ResetSPI 

void MCP2515ResetSPI()
{
	//unsigned __int8 uCmd	= 0;
	UINT8  uCmd[4] = { 0 };
	UINT8  uData[4] = { 0 };

	    // RETAILMSG( CAN_DRV_DBG
	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515ResetSPI\r\n"));
	RETAILMSG( CAN_DRV_DBG, (L"CAN: +MCP2515ResetSPI\r\n"));

	if (!InPowerHandler)
	 EnterCriticalSection ( &g_csSPIMCP2515 );

	uCmd[0] = MCP2515SPICMD_RESET;

	//SPIWrite(g_hSPI, 1, &uCmd );
	//g_pSPIObj->Enable ( );
	//g_pSPIObj->Write ( uCmd );
	//g_pSPIObj->Disable ( );
    if( SPIWriteReadCS(g_hSPI, 0, &uData[0], 1, &uCmd[0] ) == 0 )
	 {
      RETAILMSG(1 /*CAN_DRV_DBG*/, (_T("CAN: MCP2515WriteSPI, write failure\r\n")));
	 }


	DelayMS(2, InPowerHandler );
	//Sleep(2); // it's strange, but 128 microseconds (128*max_Tosc) are not enough

	if (!InPowerHandler)
	 LeaveCriticalSection ( &g_csSPIMCP2515 );

	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515ResetSPI\r\n"));
	RETAILMSG( CAN_DRV_DBG, (L"CAN: -MCP2515ResetSPI\r\n"));

	return;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetModeSPI
// Description	    : Set MCP2515 to specific Operation Mode
// Return type		: BOOL MCP2515SetModeSPI 
// Argument         : MCPOPMODE OpMode

BOOL MCP2515SetModeSPI ( MCPOPMODE OpMode )
{
	BOOL			bRetVal	 = TRUE;
	int				wait = 0;

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515SetModeSPI\r\n"));

    if (!InPowerHandler)
	 EnterCriticalSection ( &g_csSPIMCP2515 );

	if(MCP2515GetModeSPI() == OpMode)
	{
		goto END;
	}
	MCP2515BitModifySPI(CANCTRL_ADDR, REQOP_m, OpMode << REQOP_s);

	while(1)
	{
		if(MCP2515GetModeSPI() == OpMode)
		{
			break;
		}
		if(wait > MCP2515_MODE_CHANGE_MSEC_MAX)
		{
			SetLastError(ERROR_INTERNAL_ERROR);
			bRetVal = FALSE;
			break;
		}

		DelayMS(MCP2515_MODE_CHANGE_MSEC_POLL, InPowerHandler );
		//Sleep(MCP2515_MODE_CHANGE_MSEC_POLL);

		wait += MCP2515_MODE_CHANGE_MSEC_POLL;
	}

END:
	if (!InPowerHandler)
	 LeaveCriticalSection ( &g_csSPIMCP2515 );

	RETAILMSG(CAN_DRV_DBG, (L"CAN: MCP2515SetModeSPI, Mode=%d, Wait=%d msec\r\n", OpMode, wait));
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515SetModeSPI, bRetVal=0x%04X\r\n", bRetVal));
	return bRetVal;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetModeSPI
// Description	    : Gets MCP2515 Operation Mode
// Return type		: MCPOPMODE MCP2515GetModeSPI 

MCPOPMODE MCP2515GetModeSPI(void)
{
	CANSTAT_DT	tCANSTAT	= { 0 };

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515GetModeSPI\r\n"));
	RETAILMSG( 0 /*CAN_DRV_DBG*/, (L"CAN: +MCP2515GetModeSPI\r\n"));

	//MCP2515ReadSPI(&testWord, CANSTAT_ADDR, CANSTAT_LEN);
	//RETAILMSG(CAN_DRV_DBG, (L"CAN:  MCP2515GetModeSPI = 0x%08X\r\n", testWord));

	MCP2515ReadSPI(&tCANSTAT, CANSTAT_ADDR, CANSTAT_LEN);
	RETAILMSG(CAN_DRV_DBG, (L"CAN:  MCP2515GetModeSPI=%d\r\n", tCANSTAT.OPMOD));

	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515GetModeSPI\r\n"));
	RETAILMSG(0 /*CAN_DRV_DBG*/, (L"CAN: -MCP2515GetModeSPI\r\n"));

	return (MCPOPMODE)(tCANSTAT.OPMOD);
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetMask
// Description	    : Set Mask to Specific Rx Buffer
// Return type		: BOOL MCP2515BSetMask 
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int32 SID
// Argument         : unsigned __int32 EID

BOOL MCP2515SetMask ( unsigned __int8 uAddress, 
					  unsigned __int32 SID, 
				      unsigned __int32 EID )
{
	BOOL		bRetVal = FALSE;
	RXMnPACKET	tRXM	= { 0 };

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515SetMask\r\n"));
	RETAILMSG(CAN_DRV_DBG,(L"CAN: +MCP2515SetMask addr = 0x%02X, SID = 0x%08X, EID = 0x%08X\r\n", uAddress, SID, EID));

	if((FALSE == CheckSID(SID)) || (FALSE == CheckEID(EID)))
	{
		goto END;
	}

	if(MCP2515GetModeSPI() != OPMODE_CONFIGURATION)
	{
		goto END;
	}

	tRXM.SIDH = ( ( SID & 0x000007F8 ) >> 3 );
	tRXM.SIDL =   ( SID & 0x00000007 );
	tRXM.EIDH = ( ( EID & 0x00030000 ) >> 16 );
	tRXM.EID8 = ( ( EID & 0x0000FF00 ) >> 8 );
	tRXM.EID0 =   ( EID & 0x000000FF );

	MCP2515WriteSPI(&tRXM, uAddress, RXMnPACKET_LEN);

	bRetVal = TRUE;

END:
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515SetMask, bRetVal=0x%04X\r\n", bRetVal));
	RETAILMSG(CAN_DRV_DBG,(L"CAN: -MCP2515SetMask, bRetVal=0x%04X\r\n", bRetVal));
	return bRetVal;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetMask
// Description	    : Get Mask of specific Rx Buffer
// Return type		: BOOL MCP2515GetMask 
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int32 *pSID
// Argument         : unsigned __int32 *pEID

BOOL MCP2515GetMask ( unsigned __int8 uAddress, 
					  unsigned __int32 *pSID, 
					  unsigned __int32 *pEID )
{
	BOOL		bRetVal = FALSE;
	RXMnPACKET	tRXM	= { 0 };

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515GetMask\r\n"));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515GetMask\r\n"));

	if(MCP2515GetModeSPI() != OPMODE_CONFIGURATION)
	{
		goto END;
	}
	
	MCP2515ReadSPI(&tRXM, uAddress, RXMnPACKET_LEN);

	*pSID  = ( tRXM.SIDH << 3 );
	*pSID |= ( tRXM.SIDL );
	*pEID  = ( tRXM.EIDH << 16 );
	*pEID |= ( tRXM.EID8 << 8 );
	*pEID |= ( tRXM.EID0 );

	bRetVal = TRUE;

	RETAILMSG(CAN_DRV_DBG,(L"CAN: +MCP2515GetMask addr = 0x%02X, SID = 0x%08X, EID = 0x%08X\r\n", uAddress, *pSID, *pEID));

END:
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515GetMask, bRetVal=0x%04X\r\n", bRetVal));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515GetMask, bRetVal=0x%04X\r\n", bRetVal));
	return bRetVal;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetFilter
// Description	    : Set Mask to Specific Rx Buffer
// Return type		: BOOL MCP2515SetFilter 
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int32 SID
// Argument         : unsigned __int32 EID
// Argument         : BOOL bFISATO

BOOL MCP2515SetFilter ( unsigned __int8 uAddress, 
						unsigned __int32 SID, 
						unsigned __int32 EID, 
						BOOL bFISATO )
{
	BOOL		bRetVal	= FALSE;
	RXFnPACKET	tRXF	= { 0 };

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515SetFilter\r\n"));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515SetFilter, addr = 0x%02X, SID = 0x%08X, EID = 0x%08X, EX = %d\r\n", uAddress, SID, EID, bFISATO));

	if((FALSE == CheckSID(SID)) || (FALSE == CheckEID(EID)))
	{
		goto END;
	}

	if(MCP2515GetModeSPI() != OPMODE_CONFIGURATION)
	{
		goto END;
	}

	tRXF.SIDH  = ( ( SID & 0x000007F8 ) >> 3 );
	tRXF.SIDL  =   ( SID & 0x00000007 );
	tRXF.EIDH  = ( ( EID & 0x00030000 ) >> 16 );
	tRXF.EID8  = ( ( EID & 0x0000FF00 ) >> 8 );
	tRXF.EID0  =   ( EID & 0x000000FF );
	tRXF.EXIDE = bFISATO;

	MCP2515WriteSPI(&tRXF, uAddress, RXFnPACKET_LEN);

	bRetVal = TRUE;

END:
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515SetFilter, bRetVal=0x%04X\r\n", bRetVal));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515SetFilter, bRetVal=0x%04X\r\n", bRetVal));
	return bRetVal;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetFilter
// Description	    : Get Filter of Specific Rx Buffer
// Return type		: BOOL MCP2515GetFilter 
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int32 *pSID
// Argument         : unsigned __int32 *pEID
// Argument         : BOOL *pbFISATO

BOOL MCP2515GetFilter ( unsigned __int8 uAddress, 
						unsigned __int32 *pSID, 
						unsigned __int32 *pEID, 
						BOOL *pbFISATO )
{
	BOOL		bRetVal	= FALSE;
	RXFnPACKET	tRXF	= { 0 };

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515GetFilter\r\n"));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515GetFilter\r\n"));

	if(MCP2515GetModeSPI() != OPMODE_CONFIGURATION)
	{
		goto END;
	}

	MCP2515ReadSPI(&tRXF, uAddress, RXFnPACKET_LEN);

	*pSID  = ( tRXF.SIDH << 3 );
	*pSID |= ( tRXF.SIDL );
	*pEID  = ( tRXF.EIDH << 16 );
	*pEID |= ( tRXF.EID8 << 8 );
	*pEID |= ( tRXF.EID0 );
	*pbFISATO = tRXF.EXIDE;

	bRetVal = TRUE;

	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515GetFilter, addr = 0x%02X, SID = 0x%08X, EID = 0x%08X, EX = %d\r\n", uAddress, *pSID, *pEID, *pbFISATO));
END:
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515GetFilter, bRetVal=0x%04X\r\n", bRetVal));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515GetFilter, bRetVal=0x%04X\r\n", bRetVal));
	return bRetVal;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetRXM
// Description	    : Set Receive Buffer Opearint Mode
// Return type		: BOOL MCP2515SetRXM 
// Argument         : unsigned __int8 uAddress
// Argument         : unsigned __int32 uRXM

BOOL MCP2515SetRXM ( unsigned __int8 uAddress, 
					 unsigned __int32 uRXM )
{
	BOOL			bRetVal		= TRUE;
	RXBnCTRL_DT		tRXBnCTRL	= { 0 };
	unsigned __int8 uTEMP		= ( unsigned __int8 )uRXM;

	DEBUGMSG ( ZONE_STEPS,( _T ( "CAN: +MCP2515SetRXM\r\n" ) ) );
	RETAILMSG(CAN_DRV_DBG,( _T ( "CAN: +MCP2515SetRXM, uRXM[0x%02X] = 0x%02X\r\n" ), uAddress, uTEMP ) );

	MCP2515BitModifySPI(uAddress, RXM_m, (uTEMP << RXM_s));

	DEBUGMSG ( ZONE_STEPS,( _T ( "CAN: -MCP2515SetRXM, bRetVal=0x%04X\r\n" ), bRetVal ) );
	RETAILMSG(CAN_DRV_DBG,( _T ( "CAN: -MCP2515SetRXM, bRetVal=0x%04X\r\n" ), bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetRXM
// Description	    : Set Receive Buffer Opearint Mode
// Return type		: BOOL MCP2515SetRXM 
// Argument         : unsigned __int8 uAddress
// Argument         : RXBOPMODE *pRXM

BOOL MCP2515GetRXM ( unsigned __int8 uAddress, 
					 RXBOPMODE *pRXM )
{
	BOOL			bRetVal		= TRUE;
	RXBnCTRL_DT		tRXBnCTRL	= { 0 };

	DEBUGMSG ( ZONE_STEPS, ( _T ( "CAN: +MCP2515GetRXM\r\n" ) ) );
	RETAILMSG(CAN_DRV_DBG, ( _T ( "CAN: +MCP2515GetRXM\r\n" ) ) );

	if ( FALSE == MCP2515ReadSPI ( &tRXBnCTRL, uAddress,RXBnCTRL_LEN ) )
	{
		bRetVal = FALSE;

		goto END;
	}

	*pRXM = ( RXBOPMODE )tRXBnCTRL.RXM;

	RETAILMSG(CAN_DRV_DBG,( _T ( "CAN: +MCP2515GetRXM, RXM[0x%02X] = 0x%02X\r\n" ), uAddress, *pRXM ) );

END:

	DEBUGMSG ( ZONE_STEPS,( _T ( "CAN: -MCP2515GetRXM, bRetVal=0x%04X\r\n" ), bRetVal ) );
	RETAILMSG(CAN_DRV_DBG,( _T ( "CAN: -MCP2515GetRXM, bRetVal=0x%04X\r\n" ), bRetVal ) );
	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetCNF
// Description	    : Set CAN Configuration
// Return type		: BOOL MCP2515SetCNF 
// Argument         : PCANREGSETTINGS pCNF

BOOL MCP2515SetCNF ( PCANREGSETTINGS pCNF )
{
	BOOL		bRetVal	= FALSE;
	CNF_DT		tCAN	= { 0 };

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515SetCNF\r\n"));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515SetCNF\r\n"));

	if(MCP2515GetModeSPI() != OPMODE_CONFIGURATION)
	{
		goto END;
	}

	// CAN Configuration, Bit Timing Issues, Wake-Up Filter and SOF Pin
	tCAN.PHSEG2		=	pCNF->PHSEG2;
	tCAN.WAKFIL		=	pCNF->WAKFIL;
	tCAN.SOF		=	OFF;
	tCAN.PRSEG		=	pCNF->PRSEG;
	tCAN.PHSEG1		=	pCNF->PHSEG1;
	tCAN.SAM		=	pCNF->SAM;
	tCAN.BTLMODE	=	pCNF->BTLMODE;
	tCAN.BRP		=	pCNF->BRP;
	tCAN.SJW		=	pCNF->SJW;

	MCP2515WriteSPI(&tCAN, CNF_ADDR, CNF_LEN);

	bRetVal = TRUE;

	RETAILMSG(CAN_DRV_DBG, (L"CAN: MCP2515SetCNF: PHSEG2=0x%02X, WAKFIL=0x%02X, SOF=0x%02X, PRSEG=0x%02X, PHSEG1=0x%02X, SAM=0x%02X, BTLMODE=0x%02X, BRP=0x%02X, SJW=0x%02X\r\n", 
		                    tCAN.PHSEG2, tCAN.WAKFIL, tCAN.SOF, tCAN.PRSEG, tCAN.PHSEG1, tCAN.SAM, tCAN.BTLMODE, tCAN.BRP, tCAN.SJW ));

END:
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515SetCNF, bRetVal=0x%04X\r\n", bRetVal));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515SetCNF, bRetVal=0x%04X\r\n", bRetVal));
	return bRetVal;
}

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetCNF
// Description	    : Get CAN Configuration
// Return type		: BOOL MCP2515GetCNF 
// Argument         : PCNF_DT pCNF

BOOL MCP2515GetCNF ( PCNF_DT pCNF )
{
	BOOL		bRetVal	= FALSE;

	DEBUGMSG(ZONE_STEPS, (L"CAN: +MCP2515GetCNF\r\n"));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: +MCP2515GetCNF\r\n"));

	if(MCP2515GetModeSPI() != OPMODE_CONFIGURATION)
	{
		goto END;
	}

	MCP2515ReadSPI(pCNF, CNF_ADDR, CNF_LEN);

	bRetVal = TRUE;

	RETAILMSG(CAN_DRV_DBG, (L"CAN: MCP2515GetCNF: PHSEG2=0x%02X, WAKFIL=0x%02X, PRSEG=0x%02X, PHSEG1=0x%02X, SAM=0x%02X, BTLMODE=0x%02X, BRP=0x%02X, SJW=0x%02X\r\n", 
		                    pCNF->PHSEG2, pCNF->WAKFIL, pCNF->PRSEG, pCNF->PHSEG1, pCNF->SAM, pCNF->BTLMODE, pCNF->BRP, pCNF->SJW ));

END:
	DEBUGMSG(ZONE_STEPS, (L"CAN: -MCP2515GetCNF, bRetVal=0x%04X\r\n", bRetVal));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -MCP2515GetCNF, bRetVal=0x%04X\r\n", bRetVal));
	return bRetVal;
}
