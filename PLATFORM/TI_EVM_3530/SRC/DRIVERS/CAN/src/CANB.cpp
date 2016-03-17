// ================================================================================================
//
// Giant Steps Ltd.
// 
// Driver and CAN Bus relative functions
//
// ================================================================================================

#include <Windows.h>
#include <nkintr.h>
//#include <OalIntr.h>


#include "MISC.h"
#include "MCP2515.h"
#include "Debug.h"
#include <ceddk.h>
#include <ceddkex.h>
#include <gpio.h>


// Globals and Externals ==========================================================================

// Event used for Signaling of Interrupt Service Thread
HANDLE	g_hIntrEvent				= NULL;

// Event used to signal that Rx Queue contains at least one Message
HANDLE	rxqNotEmptyEvent			= NULL;

// Event used to Signal that Tx Operation completed successfully
HANDLE	g_hTxB0CompleteEvent		= NULL;
HANDLE	g_hTxB1CompleteEvent		= NULL;
HANDLE	g_hTxB2CompleteEvent		= NULL;

// Event used to Signal that Tx Buffer can be used by next Writer
HANDLE	g_hTxB0ReadyEvent			= NULL;
HANDLE	g_hTxB1ReadyEvent			= NULL;
HANDLE	g_hTxB2ReadyEvent			= NULL;

// Event used to Abort Tx,
// and release Writer from waiting for Results
HANDLE	g_hTxB0AbortEvent			= NULL;
HANDLE	g_hTxB1AbortEvent			= NULL;
HANDLE	g_hTxB2AbortEvent			= NULL;

HANDLE	g_hEchoEvent    			= NULL;

// Critical Section to defend g_dwCANERRORS from concurrently accesses
CRITICAL_SECTION	g_csERRORSMANIPULATIONS;
CRITICAL_SECTION	g_csEcho;

// CAN Last Error, will indicate the current error state of MCP2515,
// all Errors will be encoded in single 32 Bit Variable
// Reading this Variable will automatically Clean it
DWORD	g_dwCANERRORS		= 0;

// CANBPowerOff saves MCP2515 Operation Mode 
// to be restored later in CANBHandleWAKUP
static CAN_OPMODE LastOpMode = CAN_OPMODE_CONFIG;
static BOOL WaitForOST = FALSE;

// Timeout for Read / Write Operations
DWORD	g_dwReadTimeout			= 0;
DWORD	g_dwWriteTimeout		= 0;

// For saving parameters when CAN controller power shut down
GENCANCONFIG   gCANCNF;

/*
CANCONFIG      gCANCONFIG;
CANMASK        gCANMASK;
CANFILTER      gCANFILTER;
CANRWTIMEOUT   gCANRWTIMEOUT;
CANRXBOPMODE   gCANRXB0OPMODE;
CANRXBOPMODE   gCANRXB1OPMODE;
*/

CANSENDPACKET	 gtCANSENDPACKET;

static BOOL saveCNF(PGENCANCONFIG pCANCNF);
static BOOL restoreCNF(PGENCANCONFIG pCANCNF);

//{Received message queue (FIFO) object

#define RXQ_LEN 1024
CANRECEIVEPACKET rxq[RXQ_LEN], *rxqTail, *rxqHead, *rxqBottom = &rxq[0], *rxqTop = &rxq[RXQ_LEN-1];
CRITICAL_SECTION rxqAccess;
long rxqCapacity;

#define rxqAdvance( pointer ) \
{ \
	if( pointer < rxqTop ) \
	{ \
		pointer ++ ; \
	} \
	else \
	{ \
		pointer = rxqBottom; \
	} \
}

static void rxqReset( void )
{
	EnterCriticalSection(&rxqAccess);
	rxqTail = rxqBottom;
	rxqHead = rxqTop;
	rxqCapacity = RXQ_LEN;
	ResetEvent(rxqNotEmptyEvent);
	EnterCriticalSection(&g_csERRORSMANIPULATIONS);
	g_dwCANERRORS &= ~CANERR_OVERFLOW;
	LeaveCriticalSection(&g_csERRORSMANIPULATIONS);
	LeaveCriticalSection(&rxqAccess);
}

static void rxqGet( CANRECEIVEPACKET *rxqMsg )
{
	EnterCriticalSection(&rxqAccess);
	memcpy(rxqMsg, rxqTail, sizeof(CANRECEIVEPACKET));
	rxqAdvance(rxqTail);
	InterlockedIncrement(&rxqCapacity);
	if( rxqCapacity < RXQ_LEN ) SetEvent(rxqNotEmptyEvent);
	LeaveCriticalSection(&rxqAccess);
}

static void rxqPut( CANRECEIVEPACKET *rxqMsg )
{
//	EnterCriticalSection(&rxqAccess);
	if( rxqCapacity > 0 )
	{
		rxqAdvance(rxqHead);
		memcpy(rxqHead, rxqMsg, sizeof(CANRECEIVEPACKET));
		InterlockedDecrement(&rxqCapacity);
		SetEvent(rxqNotEmptyEvent);
	}
	else
	{
		memcpy(rxqHead, rxqMsg, sizeof(CANRECEIVEPACKET));
		EnterCriticalSection(&g_csERRORSMANIPULATIONS);
		g_dwCANERRORS |= CANERR_OVERFLOW;
		LeaveCriticalSection(&g_csERRORSMANIPULATIONS);
	}
//	LeaveCriticalSection(&rxqAccess);
}

void rxqInit( void )
{
	InitializeCriticalSection(&g_csERRORSMANIPULATIONS);
	InitializeCriticalSection(&rxqAccess);
	rxqReset();
}

static void rxqDeinit( void )
{
	DeleteCriticalSection(&rxqAccess);
}
//}

// Tx / Rx Specific Functions =====================================================================

// Function name	: PrepareRxPacket
// Description	    : Converts CAN Bus Packet to user friendly format
// Return type		: BOOL PrepareRxPacket 
// Argument         : PCANRECEIVEPACKET pCANRECEIVEPACKET
// Argument         : PRXBnPACKET_DT pRXPACKET
// Argument         : FRXBID ID 

BOOL PrepareRxPacket ( PCANRECEIVEPACKET pCANRECEIVEPACKET, PRXBnPACKET_DT pRXPACKET, FRXBID ID )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +PrepareRxPacket\r\n" ) ) );

	BOOL bRetVal = TRUE;


	// Zeroing
	memset ( pCANRECEIVEPACKET, 0x00, sizeof ( CANRECEIVEPACKET ) );

	// Set Filter Hit ID
	pCANRECEIVEPACKET->FilHitID = ID;

	// Set Standard ID
	pCANRECEIVEPACKET->SID	= ( pRXPACKET->SIDH << 3 );
	pCANRECEIVEPACKET->SID |= ( pRXPACKET->SIDL );

	// Check if Extended ID
	if ( 1 == pRXPACKET->IDE )
	{
		// Set Extended ID
		pCANRECEIVEPACKET->bIsEXT = TRUE;

		// Check if Remote Transmission Request if Extended Packet
		if ( 1 == pRXPACKET->RTR )
		{
			pCANRECEIVEPACKET->bIsRTR = TRUE;
		}

		pCANRECEIVEPACKET->EID  = ( pRXPACKET->EIDH << 16 );
		pCANRECEIVEPACKET->EID |= ( pRXPACKET->EID8 << 8 );
		pCANRECEIVEPACKET->EID |= ( pRXPACKET->EID0 );
	}
	else if ( 1 == pRXPACKET->SRR )
	{
		// Remote Transmission Request if Standard Packet
		pCANRECEIVEPACKET->bIsRTR = TRUE;
	}

	// Fill Data Length
	if ( 0 != pRXPACKET->DLC )
	{
		pCANRECEIVEPACKET->DataLength = pRXPACKET->DLC;

		// Fill Data
		memcpy ( pCANRECEIVEPACKET->DATA, pRXPACKET->DATA, pRXPACKET->DLC );
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -PrepareRxPacket, bRetVal=0x%04X\r\n" ), bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: PrepareTxPacket
// Description	    : Validate Data from user and Build Packet to be send via CAN Bus
// Return type		: BOOL PrepareTxPacket 
// Argument         : PCANSENDPACKET pCANSENDPACKET
// Argument         : PTXBnPACKET_DT pTXPACKET

BOOL PrepareTxPacket ( PCANSENDPACKET pCANSENDPACKET, PTXBnPACKET_DT pTXPACKET )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +ValidateTxPacket\r\n" ) ) );

	BOOL bRetVal = TRUE;


	// Set Standard ID
	if ( 0x7FF < pCANSENDPACKET->SID )
	{
		bRetVal = FALSE;
		
		RETAILMSG( 1 /*CAN_DRV_DBG*/, (_T("CAN: PrepareTxPacket, invalid SID=0x%04X\r\n" ), pCANSENDPACKET->SID));
		goto END;
	}
	else
	{
		pTXPACKET->SIDH = ( ( pCANSENDPACKET->SID & 0x07F8 ) >> 3 );
		pTXPACKET->SIDL = ( ( pCANSENDPACKET->SID & 0x0007 ) );
	}

	// Set Extended ID
	pTXPACKET->EXIDE = pCANSENDPACKET->bIsEXT;

	if ( TRUE == pTXPACKET->EXIDE )
	{
		if ( 0x3FFFF < pCANSENDPACKET->EID )
		{
			RETAILMSG( 1 /*CAN_DRV_DBG*/, (_T("CAN: PrepareTxPacket, invalid EID=0x%08X\r\n" ), pCANSENDPACKET->EID));
			bRetVal = FALSE;
			
			goto END;
		}
		else
		{
			pTXPACKET->EIDH = ( ( pCANSENDPACKET->EID & 0x00030000 ) >> 16 );
			pTXPACKET->EID8 = ( ( pCANSENDPACKET->EID & 0x0000FF00 ) >> 8 );
			pTXPACKET->EID0 = ( ( pCANSENDPACKET->EID & 0x000000FF ) );
		}

	}

	// Set Remote Transmit Request
	pTXPACKET->RTR = pCANSENDPACKET->bIsRTR;

	if ( FALSE == pTXPACKET->RTR ) 
	{
		if ( ( 0x00 == pCANSENDPACKET->DataLength ) || ( 0x08 < pCANSENDPACKET->DataLength ) )
		{
			RETAILMSG( 1 /*CAN_DRV_DBG*/, (_T("CAN: PrepareTxPacket, invalid length=0x%04X\r\n" ), pCANSENDPACKET->DataLength));
			bRetVal = FALSE;
		
			goto END;
		}
		else
		{
			// Set Data Length and Data
			pTXPACKET->DLC = (pCANSENDPACKET->DataLength < 8)?pCANSENDPACKET->DataLength:8;

			memcpy(pTXPACKET->DATA, pCANSENDPACKET->DATA, pTXPACKET->DLC);
		}
	}
	else
	{
		// In Remote Transmit Request Data Length = 0
		// ##### Michael
		// Wrong !!! There is no DATA in Remote Frame, but DLC may be set!!!
		// according to CAN 2.0B specifications
		//pTXPACKET->DLC = 0;
		// Set Data Length code:
		pTXPACKET->DLC = (pCANSENDPACKET->DataLength < 8)?pCANSENDPACKET->DataLength:8;

	}


END:

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -ValidateTxPacket, bRetVal=0x%04X\r\n" ), bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: PutMsgToQueue 
// Description	    : Stores incoming CAN Bus message in internal queue
// Return type		: DWORD WINAPI 
// Argument         : LPVOID lpParameter

void PutMsgToQueue(INTRCODE IF_code)
{
	unsigned int	 uRXBnSIDH = 0, uRXBnCTRL = 0, uRXnIF_mask = 0, uRXnIF_status = 0, uFILHITMASK	= 0;
	RXBnCTRL_DT		 tRXBCTRL		= {0};
	RXBnPACKET_DT	 tRXBPACKET		= {0};
	CANRECEIVEPACKET tCANRXPACKET	= {0};
	//DWORD            dwRet;

	DEBUGMSG(ZONE_STEPS, (_T("CAN: +PutMsgToQueue\r\n")));

	__try
	{
		if(INTRFC_RXB0 == IF_code)
		{
			uRXBnCTRL = RXB0CTRL_ADDR;
			uRXBnSIDH = RXB0SIDH_ADDR;

			uRXnIF_mask = ON << RX0I_bit;
			uRXnIF_status = OFF << RX0I_bit;

			uFILHITMASK = 0x01;
		}
		else if(INTRFC_RXB1 == IF_code)
		{
			uRXBnCTRL = RXB1CTRL_ADDR;
			uRXBnSIDH = RXB1SIDH_ADDR;

			uRXnIF_mask = ON << RX1I_bit;
			uRXnIF_status = OFF << RX1I_bit;

			uFILHITMASK = 0x07;
		}
		else
			__leave;

		RETAILMSG(0, (_T("CAN: PutMsgToQueue, Rx Complete message is got\r\n")));
		if(!MCP2515ReadSPI((unsigned int *)&tRXBCTRL, uRXBnCTRL, RXBnCTRL_LEN))
			__leave;

		if(!MCP2515ReadSPI((unsigned int *)&tRXBPACKET, uRXBnSIDH, RXBnPACKET_LEN))
			__leave;

		// Clean RXnIF Flag
		MCP2515BitModifySPI(CANINTF_ADDR, uRXnIF_mask, uRXnIF_status);

		if(PrepareRxPacket(&tCANRXPACKET, &tRXBPACKET, (FRXBID)(tRXBCTRL.FILHIT & uFILHITMASK)))
		{
			
			// If Echo mode is On - send it
			if( g_EchoHandle != NULL )
			{
		     gtCANSENDPACKET.TxBP		= TXB_PRIORITY_LOW;
		     gtCANSENDPACKET.bIsRTR		= tCANRXPACKET.bIsRTR;
		     gtCANSENDPACKET.bIsEXT		= tCANRXPACKET.bIsEXT;
		     gtCANSENDPACKET.DataLength	= tCANRXPACKET.DataLength;
			 gtCANSENDPACKET.SID         = tCANRXPACKET.SID;
			 gtCANSENDPACKET.EID         = tCANRXPACKET.EID;
			 memcpy( gtCANSENDPACKET.DATA, tCANRXPACKET.DATA, tCANRXPACKET.DataLength );

			 if( g_hEchoEvent != NULL )
			  SetEvent(g_hEchoEvent);
			}

			rxqPut(&tCANRXPACKET);
		}
		else
		{
			RETAILMSG(1, (_T("CAN: PutMsgToQueue, PrepareRxPacket FAILED!!!\r\n")));
		}
	}
	__finally
	{
		DEBUGMSG(ZONE_STEPS, (_T("CAN: -PutMsgToQueue\r\n" )));
	}

	return;
}

// Function name	: CANBReceiveMsg
// Description	    : Gets One Message from Internal Message Queue
// Return type		: DWORD CANBDReceiveMsg 
// Argument         : HANDLE hAbortEvent
// Argument         : PCANRECEIVEPACKET pCANRECEIVEPACKET

DWORD CANBReceiveMsg(HANDLE hAbortEvent, PCANRECEIVEPACKET pCANRECEIVEPACKET)
{
	DWORD	dwRetVal = -1, dwCount = 0;
	HANDLE	hEvents[2] = {hAbortEvent, rxqNotEmptyEvent};


	RETAILMSG(CAN_DRV_DBG, (_T("CAN: +CANBReceiveMsg (%d)\r\n"), g_dwReadTimeout));

	dwCount = sizeof(hEvents)/sizeof(HANDLE);

	switch(WaitForMultipleObjects(dwCount, hEvents, 0, g_dwReadTimeout))
	{
		case WAIT_FAILED: // General WaitForMultipleObject Failure
			RETAILMSG(1, (_T("CAN: CANBReceiveMsg, WaitForMultipleObjects FAILED!!! err#0x%04X\r\n"), GetLastError()));
			break;
		case WAIT_TIMEOUT:
			SetLastError(ERROR_TIMEOUT);
			EnterCriticalSection(&g_csERRORSMANIPULATIONS);
			g_dwCANERRORS |= CANERR_RXTIMEOUT;
			LeaveCriticalSection (&g_csERRORSMANIPULATIONS);
			break;
		case WAIT_OBJECT_0: // hAbortEvent
			SetLastError(ERROR_OPERATION_ABORTED);
			RETAILMSG(1, (_T("CAN: CANBReceiveMsg, hAbortEvent!!!\r\n")));
			break;
		case WAIT_OBJECT_0 + 1: // rxqNotEmptyEvent
		{
			RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBReceiveMsg, rxqNotEmptyEvent!!!\r\n")));
			rxqGet(pCANRECEIVEPACKET);
			dwRetVal = pCANRECEIVEPACKET->DataLength;
			if( g_dwCANERRORS &   CANERR_OVERFLOW )
			{
				EnterCriticalSection(&g_csERRORSMANIPULATIONS);
				g_dwCANERRORS &= ~CANERR_OVERFLOW;
				LeaveCriticalSection(&g_csERRORSMANIPULATIONS);
				SetLastError(ERROR_INTERNAL_ERROR);
				RETAILMSG(1, (_T("CAN: CANBReceiveMsg, Rx queue is overflowed!!!\r\n")));
			}
			break;
		}
	}


	RETAILMSG(CAN_DRV_DBG, (_T("CAN: -CANBReceiveMsg, dwRetVal=0x%04X\r\n" ), dwRetVal));

	return dwRetVal;
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBTransmitMsg
// Description	    : Send Message via CAN Bus
// Return type		: DWORD CANBTransmitMsg 
// Argument         : HANDLE hAbortEvent
// Argument         : PCANSENDPACKET pCANSENDPACKET

DWORD CANBTransmitMsg(HANDLE hAbortEvent, PCANSENDPACKET pCANSENDPACKET)
{
	int		TxBufNum, iTimeOut = 0;
	DWORD	dwRetVal = -1, dwCount = 0, dwErrors	= 0;
	HANDLE	hTxReady = 0, hTxCompletion[3] = {0}, hTxReadyEvents[4]	= { hAbortEvent,
																		g_hTxB0ReadyEvent,
																		g_hTxB1ReadyEvent,
																		g_hTxB2ReadyEvent };
	unsigned __int8	uTXBnSIDH = 0, uTXBnCTRL = 0, uTXnI_m = 0, uTXnI_s = 0, uBufferPriority	= TXB_PRIORITY_LOWEST;
	TXBnPACKET_DT tTXPACKET	= {0};


	__try
	{
		RETAILMSG(CAN_DRV_DBG, (_T("CAN: +CANBTransmitMsg(%d, %d)\r\n"), g_dwWriteTimeout, GetTickCount()));

		// Check the Data validity from user and make Tx Packet
		if(!PrepareTxPacket(pCANSENDPACKET, &tTXPACKET))
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			RETAILMSG(1, (_T("CAN: CANBTransmitMsg, Invalid Tx Packet!!!\r\n")));
			__leave;
		}

		dwCount = sizeof(hTxReadyEvents)/sizeof(HANDLE);

		// Timeout handling
		iTimeOut = g_dwWriteTimeout + GetTickCount();

		// Obtain free Tx Buffer
		switch(WaitForMultipleObjects(dwCount, hTxReadyEvents, 0, g_dwWriteTimeout))
		{
			case WAIT_FAILED: // General WaitForMultipleObject Failure
			{
				RETAILMSG(1, (_T("CAN: CANBTransmitMsg, WaitForMultipleObjects FAILED!!! err#0x%04X\r\n"), GetLastError()));
				__leave;
			}
			case WAIT_TIMEOUT : // Write Timeout occur
			{
				SetLastError(ERROR_TIMEOUT);
				dwErrors |= CANERR_TXTIMEOUT;
				__leave;
			}
			case WAIT_OBJECT_0: // hAbortEvent
			{
				SetLastError(ERROR_OPERATION_ABORTED);
				RETAILMSG(1, (_T("CAN: CANBTransmitMsg, hAbortEvent !!!\r\n")));
				__leave;
			}
			case WAIT_OBJECT_0 + 1: // g_hTxB0ReadyEvent
			{
				RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Select TxB0 (%d)\r\n"), GetTickCount()));
				TxBufNum			= 0;
				hTxReady			= g_hTxB0ReadyEvent;
				hTxCompletion[ 1 ]	= g_hTxB0AbortEvent;
				hTxCompletion[ 2 ]	= g_hTxB0CompleteEvent;
				uTXBnCTRL			= TXB0CTRL_ADDR;
				uTXBnSIDH			= TXB0SIDH_ADDR;
				uTXnI_m				= ON << TX0I_bit;
				uTXnI_s				= TX0I_bit;
				break;
			}
			case WAIT_OBJECT_0 + 2: // g_hTxB1ReadyEvent
			{
				RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Select TxB1 (%d)\r\n"), GetTickCount()));
				TxBufNum			= 1;
				hTxReady			= g_hTxB1ReadyEvent;
				hTxCompletion[ 1 ]	= g_hTxB1AbortEvent;
				hTxCompletion[ 2 ]	= g_hTxB1CompleteEvent;
				uTXBnCTRL			= TXB1CTRL_ADDR;
				uTXBnSIDH			= TXB1SIDH_ADDR;
				uTXnI_m				= ON << TX1I_bit;
				uTXnI_s				= TX1I_bit;
				break;
			}
			case WAIT_OBJECT_0 + 3: // g_hTxB2ReadyEvent
			{
				RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Select TxB2 (%d)\r\n"), GetTickCount()));
				TxBufNum			= 2;
				hTxReady			= g_hTxB2ReadyEvent;
				hTxCompletion[ 1 ]	= g_hTxB2AbortEvent;
				hTxCompletion[ 2 ]	= g_hTxB2CompleteEvent;
				uTXBnCTRL			= TXB2CTRL_ADDR;
				uTXBnSIDH			= TXB2SIDH_ADDR;
				uTXnI_m				= ON << TX2I_bit;
				uTXnI_s				= TX2I_bit;
				break;
			}
		}

		hTxCompletion[0] = hAbortEvent;
		dwCount = sizeof(hTxCompletion)/sizeof(HANDLE);

		// Set Transmit Buffer Priority
		if((TXB_PRIORITY_LOWEST <= pCANSENDPACKET->TxBP ) && (TXB_PRIORITY_HIGHEST >= pCANSENDPACKET->TxBP))
			uBufferPriority = pCANSENDPACKET->TxBP;

		RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Set TxB priority (%d)\r\n"), GetTickCount()));
		MCP2515BitModifySPI(uTXBnCTRL, 3 << TXP_bit, uBufferPriority << TXP_bit);

		RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Write packet to TxB (%d)\r\n"), GetTickCount()));
		// Write Data to Specific Tx Buffer
		if(!MCP2515WriteSPI((unsigned int * )&tTXPACKET, uTXBnSIDH, TXBnPACKET_LEN - sizeof(tTXPACKET.DATA) + tTXPACKET.DLC))
			__leave;

		RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Initiate Tx (%d)\r\n"), GetTickCount()));
		// Initiate Data Transmit by setting TXREQ bit
		MCP2515BitModifySPI(uTXBnCTRL, ON << TXREQ_bit, ON << TXREQ_bit);

		iTimeOut -= GetTickCount();

		if(iTimeOut > 0)
		{
			RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Wait for Tx complition\r\n")));
			switch(WaitForMultipleObjects(dwCount, hTxCompletion, 0, iTimeOut))
			{
				case WAIT_FAILED: // General WaitForMultipleObject Failure
					RETAILMSG(1, (_T("CAN: CANBTransmitMsg, WaitForMultipleObjects FAILED!!! err#0x%04X\r\n"), GetLastError()));
					break;
				case WAIT_TIMEOUT: // Write Timeout occur
					SetLastError(ERROR_TIMEOUT);
					dwErrors |= CANERR_TXTIMEOUT;
					RETAILMSG(1, (_T("CAN: CANBTransmitMsg, Write Timeout occur!!! err#0x%04X\r\n"), GetLastError()));
					break;
				case WAIT_OBJECT_0: // hAbortEvent
					SetLastError(ERROR_OPERATION_ABORTED);
					RETAILMSG(1, (_T("CAN: CANBTransmitMsg, hAbortEvent !!!\r\n")));
					break;
				case WAIT_OBJECT_0 + 1: // g_hTxBnAbortEvent
					SetLastError(ERROR_IO_DEVICE);
					RETAILMSG(1, (_T("CAN: CANBTransmitMsg, Abort Transfer !!! TXBnCTRL=0x%04X\r\n"), uTXBnCTRL));
					__leave;
				case WAIT_OBJECT_0 + 2: // g_hTxBnCompleteEvent
					dwRetVal = tTXPACKET.DLC;
					RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Transfer Complete!!! TXBnCTRL=0x%04X\r\n"), uTXBnCTRL));
					__leave;
			}
		}
		else
		{
			SetLastError(ERROR_TIMEOUT);
			dwErrors |= CANERR_TXTIMEOUT;
		}

		RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBTransmitMsg, Clean TxB%d, %d\r\n"), TxBufNum, GetTickCount()));
		// Clean TXREQ bit
//		RETAILMSG(1, (_T("CAN: CANBTransmitMsg, Clean Tx REQ %X, %X, %d\r\n"), ON << TXREQ_bit, OFF << TXREQ_bit, GetTickCount()));
		MCP2515BitModifySPI(uTXBnCTRL, ON << TXREQ_bit, OFF << TXREQ_bit);

		// Clean TXnIF
//		RETAILMSG(0, (_T("CAN: CANBTransmitMsg, Clean Tx IF %X, %X %d\r\n"), uTXnI_m, (OFF << uTXnI_s), GetTickCount()));
		MCP2515BitModifySPI(CANINTF_ADDR, uTXnI_m, (OFF << uTXnI_s));
	}
	__finally
	{
		// Set Event to Signaled State to release next Writer
		if(hTxReady)
			SetEvent(hTxReady);

		EnterCriticalSection(&g_csERRORSMANIPULATIONS);
		g_dwCANERRORS |= dwErrors;
		LeaveCriticalSection ( &g_csERRORSMANIPULATIONS );

		RETAILMSG(CAN_DRV_DBG, (_T("CAN: -CANBTransmitMsg, dwRetVal=0x%04X %d\r\n"), dwRetVal, GetTickCount()));
	}

	return dwRetVal;
}

// Interrupts Handling Functions ==================================================================

// Function name	: CANBHandleMERR
// Description	    : Handle MERR Condition
// Return type		: BOOL CANBHandleMERR 
// Argument         : TXBID ID

void CANBHandleMERR(TXBID ID)
{
	DEBUGMSG(ZONE_STEPS, (_T("CAN: +CANBHandleMERR\r\n")));

	DWORD			dwErrors	= 0;
	HANDLE			hAbortEvent	= 0;
	unsigned __int8 uAddress	= 0;
	TXBnCTRL_DT		tTXBnCTRL	= { 0 };

	switch(ID)
	{
		case TXB0 :
			uAddress	= TXB0CTRL_ADDR;
			hAbortEvent = g_hTxB0AbortEvent;
			break;
		case TXB1 :
			uAddress	= TXB1CTRL_ADDR;
			hAbortEvent = g_hTxB1AbortEvent;
			break;
		case TXB2 :
			uAddress	= TXB2CTRL_ADDR;
			hAbortEvent = g_hTxB2AbortEvent;
			break;
		default :
			DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBHandleMERR, Not Supported ID=0x%04X!!!\r\n"), ID));
			return;
	}

	// Read TXBnCTRL to determine if there was an error
	if(!MCP2515ReadSPI((unsigned int *)&tTXBnCTRL, uAddress, TXBnCTRL_LEN))
		return;

	// Set Current Error State of MCP2515
	if(tTXBnCTRL.TXERR)
		dwErrors |= CANERR_TXERR_TXB;

	if(tTXBnCTRL.MLOA)
		dwErrors |= CANERR_MLOA_TXB;

	// Sometimes MCP2515 wrongly raises ABTF flag (even though ABAT is not requested).
	// So ignore ABTF condition, if ABAT is not used.
#if 0
	if(tTXBnCTRL.ABTF)
	{
		CANCTRL_DT tCANCTRL = {0};
		MCP2515ReadSPI((unsigned int *)&tCANCTRL, CANCTRL_ADDR, CANCTR_LEN);
		if(tCANCTRL.ABAT)
		{
			MCP2515BitModifySPI(CANCTRL_ADDR, ABAT_m, 0);
			RETAILMSG(1, (L"CAN: CANBHandleMERR, CANCTRL=0x%04X!!!\r\n", tCANCTRL));
			dwErrors |= CANERR_ABTF_TXB;
		}
	}
#endif

	EnterCriticalSection(&g_csERRORSMANIPULATIONS);
	g_dwCANERRORS |= dwErrors;
	LeaveCriticalSection(&g_csERRORSMANIPULATIONS);

	if(dwErrors)
	{
		// Clean TXREQ of specific TXBnCTRL in which error was detected
		MCP2515BitModifySPI(uAddress, ON << TXREQ_bit, OFF << TXREQ_bit);

		// Release Writer from waiting for Tx Results
		SetEvent(hAbortEvent);
	}

	MCP2515BitModifySPI(CANINTF_ADDR, ON << MERR_bit, OFF << MERR_bit);

	DEBUGMSG(ZONE_STEPS, (_T("CAN: -CANBHandleMERR\r\n")));

	return;
}

// Function name	: CANBHandleERR
// Description	    : Handle EFLG Conditions
// Return type		: BOOL CANBHandleERR 

void CANBHandleERR(void)
{
	DWORD	dwErrors	= 0;
	EFLG_DT tEFLG		= {0};

	DEBUGMSG(ZONE_STEPS, (_T("CAN: +CANBHandleERR\r\n")));

	// Read TXBnCTRL to determine in wich TX Buffer were Errors
	if(!MCP2515ReadSPI((unsigned int *)&tEFLG, EFLG_ADDR,EFLG_LEN))
		return;

	if(tEFLG.EWARN)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, EWARN\r\n")));

		dwErrors |= CANERR_ERRWARNING;
	}

	if(tEFLG.RXWAR)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, RXWAR\r\n")));

		dwErrors |= CANERR_WARNIG_RX;
	}

	if(tEFLG.TXWAR)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, TXWAR\r\n")));

		dwErrors |= CANERR_WARNIG_TX;
	}

	if(tEFLG.RXEP)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, RXEP\r\n")));

		dwErrors |= CANERR_PASSIVE_RX;
	}

	if(tEFLG.TXEP)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, TXEP\r\n")));

		dwErrors |= CANERR_PASSIVE_TX;
	}

	if(tEFLG.RX0OVR)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, RX0OVR\r\n")));

		dwErrors |= CANERR_OVR_RXB;

		// Clean RX0OVR
		MCP2515BitModifySPI(EFLG_ADDR, ON << RX0OVR_bit, OFF << RX0OVR_bit);
	}

	if(tEFLG.RX1OVR)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, RX1OVR\r\n")));

		dwErrors |= CANERR_OVR_RXB;

		// Clean RX1OVR
		MCP2515BitModifySPI(EFLG_ADDR, ON << RX1OVR_bit, OFF << RX1OVR_bit);
	}

	if(tEFLG.TXBO)
	{
		DEBUGMSG(ZONE_CAN, (_T("CAN: CANBHandleERR, TXBO\r\n")));

		SetEvent(g_hTxB0AbortEvent);
		SetEvent(g_hTxB1AbortEvent);
		SetEvent(g_hTxB2AbortEvent);

		dwErrors |= CANERR_BUSOFF;
	}

	EnterCriticalSection(&g_csERRORSMANIPULATIONS);
	g_dwCANERRORS |= dwErrors;
	LeaveCriticalSection(&g_csERRORSMANIPULATIONS);

	// Clean ERRIF
	MCP2515BitModifySPI(CANINTF_ADDR, ON << ERRI_bit, OFF << ERRI_bit);

	DEBUGMSG(ZONE_STEPS, (_T("CAN: -CANBHandleERR\r\n")));

	return;
}

// Function name	: CANBHandleWAKUP
// Description	    : Handle Wake-Up Condition
// Return type		: void 

void CANBHandleWAKUP(void)
{
	DEBUGMSG(ZONE_STEPS, (_T("CAN: +CANBHandleWAKUP\r\n")));

	// Disable MCP2515 awakening
	MCP2515BitModifySPI(CANINTE_ADDR, 1<<WAKI_bit, 0<<WAKI_bit);
	// Clean WAKIF
	MCP2515BitModifySPI(CANINTF_ADDR, 1<<WAKI_bit, 0<<WAKI_bit);
	// Restore previous Operation Mode
	RETAILMSG(CAN_DRV_DBG, (L"CAN: CANBHandleWAKUP restores LastOpMode (=%d).\r\n", LastOpMode));
	MCP2515SetModeSPI((MCPOPMODE)LastOpMode);

	DEBUGMSG(ZONE_STEPS, (_T("CAN: -CANBHandleWAKUP\r\n" )));

	return;
}

// Function name	: CANBIntrHandler
// Description	    : Actual Interrupt Handler
// Return type		: BOOL CANBIntrHandler 

void CANBIntrHandler(void)
{
	unsigned __int8	uINTFLAG	= 0;
	CANINTF_DT		tCANINTF	= {0};
	
	RETAILMSG(0, (_T("CAN: +CANBIntrHandler\r\n")));

	do
	{
		/*
		The MCP2515 utilizes an oscillator startup timer (OST)
		that holds the MCP2515 in reset to insure that the
		oscillator has stabilized before the internal state
		machine begins to operate. The OST maintains reset
		for the first 128 OSC1 clock cycles after power-up or
		wake up from sleep mode occurs. It should be noted
		that no SPI operations should be attempted until after
		the OST has expired.
		*/
		if (WaitForOST)
		{
			Sleep(2); // it's strange, but 128 microseconds (128*max_Tosc) are not enough
			WaitForOST = FALSE;
		}
		MCP2515ReadSPI((unsigned int *)&tCANINTF, CANINTF_ADDR, CANINTF_LEN);

		memcpy(&uINTFLAG, &tCANINTF, sizeof ( unsigned __int8 ) );

		if(0 == (0xFF & uINTFLAG))
			break;

		//RETAILMSG(1, (L" CANINTF=0x%02X\r\n", uINTFLAG));

		if(tCANINTF.MERRF)
		{
			CANBHandleMERR(TXB0);
			CANBHandleMERR(TXB1);
			CANBHandleMERR(TXB2);

			DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_MERRF\r\n")));
		}
		if(tCANINTF.ERRIF)
		{
			CANBHandleERR();

			DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_ERR\r\n")));
		}
		if(tCANINTF.WAKIF)
		{
			CANBHandleWAKUP();

			DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_WAKUP\r\n")));
		}
		if(tCANINTF.RX0IF)
		{
			PutMsgToQueue(INTRFC_RXB0);

			DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_RXB0\r\n")));
		}
		if(tCANINTF.RX1IF)
		{
			PutMsgToQueue(INTRFC_RXB1);

			DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_RXB1\r\n")));
		}
		if(tCANINTF.TX0IF || tCANINTF.TX1IF || tCANINTF.TX2IF)
		{
			if(tCANINTF.TX0IF)
			{
//				MCP2515BitModifySPI(TXB0CTRL_ADDR, ON << TXREQ_bit, OFF << TXREQ_bit);
				MCP2515BitModifySPI(CANINTF_ADDR,  ON << TX0I_bit,  OFF << TX0I_bit);

				SetEvent(g_hTxB0CompleteEvent);

				DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_TXB0\r\n")));
			}
			if(tCANINTF.TX1IF)
			{
//				MCP2515BitModifySPI(TXB1CTRL_ADDR, ON << TXREQ_bit, OFF << TXREQ_bit);
				MCP2515BitModifySPI(CANINTF_ADDR,  ON << TX1I_bit,  OFF << TX1I_bit);

				SetEvent(g_hTxB1CompleteEvent);

				DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_TXB1\r\n")));
			}
			if(tCANINTF.TX2IF)
			{
//				MCP2515BitModifySPI(TXB2CTRL_ADDR, ON << TXREQ_bit, OFF << TXREQ_bit);
				MCP2515BitModifySPI(CANINTF_ADDR,  ON << TX2I_bit,  OFF << TX2I_bit);

				SetEvent(g_hTxB2CompleteEvent);

				DEBUGMSG(ZONE_ERROR, (_T("CAN: CANBIntrHandler, INTRFC_TXB2\r\n")));
			}
		}
	}while(1);

	DEBUGMSG(ZONE_STEPS, (_T("CAN: -CANBIntrHandler\r\n")));

	return ;
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANB_DismissIST 
// Description	    : 

void CANB_DismissIST( CAN_INIT_CONTEXT *pCtx )
{
	DEBUGMSG ( ZONE_STEPS, ( _T ( "CAN: +CANB_DismissIST\r\n" ) ) );

	//InterruptDisable(pCtx->sysIntr);
	GPIOInterruptDisable(pCtx->hGpio, pCtx->irqNum);

	// Close IST Event g_hIntrEvent, and
	// this will terminate WaitForSingleObject in IST
	if ( NULL != pCtx->hIntrEvent )
	{
		CloseHandle ( pCtx->hIntrEvent );
		pCtx->hIntrEvent = NULL;
	}

	DEBUGMSG ( ZONE_STEPS, ( _T ( "CAN: -CANB_DismissIST\r\n" ) ) );

	return;
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBIntrServThread 
// Description	    : Interrupt Service Thread
// Return type		: DWORD WINAPI 
// Argument         : LPVOID lpParameter

DWORD WINAPI CANBIntrServThread ( LPVOID lpParameter )
{
  CAN_INIT_CONTEXT   *pCtx = (CAN_INIT_CONTEXT *)lpParameter;

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBIntrServThread, lpParameter=0x%08X\r\n" ), \
			   lpParameter ) );

	CeSetThreadPriority(GetCurrentThread(), 100);
	//InitializeCriticalSection(&g_csERRORSMANIPULATIONS);
	//rxqInit();

#if 0

	if ( FALSE == CANBCreateEvents ( ) )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBIntrServThread, CANBDCreateEvents FAILED!!!\r\n" ) ) );

		goto END;
	}


	if ( FALSE == InterruptInitialize ( SYSINTR_CAN_BUS, 
										g_hIntrEvent, 
										NULL, 
										0 ) )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBIntrServThread, InterruptInitialize FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		goto END;
	}
#endif

	do
	{
		switch(WaitForSingleObject(pCtx->hIntrEvent, INFINITE))
		{
			case WAIT_OBJECT_0: // g_hIntrEvent
				RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBIntrServThread, Interrupt !!!\r\n")));
				CANBIntrHandler();
				//InterruptDone(pCtx->sysIntr);
				GPIOInterruptDone(pCtx->hGpio, pCtx->irqNum);
				break;
			default :
				RETAILMSG(1, (_T("CAN: CANBIntrServThread, CANBIntrServThread EXIT...\r\n")));
				goto END;

		}
	}
	while(1);


END:

	EnterDeadState();

	DEBUGMSG ( ZONE_ERROR, \
			   ( _T ( "CAN: CANBIntrServThread, FATAL ERROR!!!\r\n" ) ) );

	EnterCriticalSection ( &g_csERRORSMANIPULATIONS );
	g_dwCANERRORS |= CANERR_FATALERROR;
	LeaveCriticalSection ( &g_csERRORSMANIPULATIONS );

	CANBCloseEvents ( );
	rxqDeinit();
	DeleteCriticalSection(&g_csERRORSMANIPULATIONS);

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBIntrServThread\r\n" ) ) );

	return ( 0 );
}



// ------------------------------------------------------------------------------------------------

// Function name	: CANBIntrServThread 
// Description	    : Interrupt Service Thread
// Return type		: DWORD WINAPI 
// Argument         : LPVOID lpParameter

DWORD WINAPI CANBEchoThread ( LPVOID lpParameter )
{
  CAN_INIT_CONTEXT   *pCtx = (CAN_INIT_CONTEXT *)lpParameter;
  DWORD               dwRet;
  

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBEchoThread, lpParameter=0x%08X\r\n" ), \
			   lpParameter ) );

	InitializeCriticalSection(&g_csEcho);

	CeSetThreadPriority(GetCurrentThread(), 200);

	do
	{
		switch(WaitForSingleObject(pCtx->hEchoEvent, INFINITE))
		{
			case WAIT_OBJECT_0: // hEchoEvent
				RETAILMSG(CAN_DRV_DBG, (_T("CAN: CANBEchoThread\r\n")));

			// If Echo mode is On - send it
			if( g_EchoHandle != NULL )
			{
			 EnterCriticalSection(&g_csEcho);
			 dwRet = CANBTransmitMsg(g_EchoHandle, &gtCANSENDPACKET );
			 LeaveCriticalSection ( &g_csEcho );

			 if( dwRet == (DWORD)-1 )
			 {
              RETAILMSG(1, (_T("CAN: CANBEchoThread, Echo Sending FAILED!!!\r\n"))); 
			  //goto END;
			 }
			}

				break;
			default :
				RETAILMSG(1, (_T("CAN: CANBEchoThread, CANBEchoThread EXIT...\r\n")));
				goto END;

		}
	}
	while(1);


END:

	DeleteCriticalSection(&g_csEcho);
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBEchoThread\r\n" ) ) );

	return ( 0 );
}

//
// Function name	: CANBPowerOn
// Description	    : Generates awakening
// Return type		: void 
//
void CANBPowerOn(CAN_INIT_CONTEXT *pCtx, CEDEVICE_POWER_STATE devPrevPwrState)
{
  BOOL    canPowerOn;
  //int     i;

    if( pCtx == NULL )
 	{
	 RETAILMSG(1, (L"CAN: CANBPowerOn: Invalid paremeter\r\n"));
	 return;
	}

    if( pCtx->cookie != CAN_DEVICE_COOKIE )
 	{
	 RETAILMSG(1, (L"CAN: CANBPowerOn: Invalid device context\r\n"));
	 return;
	}

	RETAILMSG(CAN_DRV_DBG, (L"CAN: +CANBPowerOn, Context=0x%08X, pwrState = %d\r\n", pCtx, devPrevPwrState));

	InPowerHandler = 1;

	canPowerOn  = MCP2515PowerStatus(pCtx);

	// ##### Michael
	if( !canPowerOn )// Switch On MCP2515 Power
	{
	  MCP2515PowerOn(pCtx);
	  CANBResetMCP2515();

	  // restore previous configuration
	  restoreCNF(&gCANCNF);

	  CANBSetOpMode(LastOpMode);
	}

	CAN_OPMODE OpMode = CANBGetOpMode();
	if (OpMode == CAN_OPMODE_SLEEP)
	{
		WaitForOST = TRUE;
		RETAILMSG(CAN_DRV_DBG, (L"CAN: CANBPowerOn generates awakening.\r\n"));
		// Enable wake up interrupt
		MCP2515BitModifySPI(CANINTE_ADDR, 1<<WAKI_bit, 1<<WAKI_bit);
		// Generate wake up attempt
		MCP2515BitModifySPI(CANINTF_ADDR, 1<<WAKI_bit, 1<<WAKI_bit);
	}
	else
	{
		RETAILMSG(CAN_DRV_DBG, (L"CAN: CANBPowerOn is invoked, but MCP2515 is not in the Sleep Mode.\r\n"));
	}

	RETAILMSG(CAN_DRV_DBG, (L"CAN: -CANBPowerOn, Context=0x%08X\r\n", pCtx));
	InPowerHandler = 0;
}
//
// Function name	: CANBPowerOff
// Description	    : Saves current MCP2515 Operation Mode and switches it into the Sleep Mode
// Return type		: void 
// Argument         : BOOL EnableAwakening
//
void CANBPowerOff(CAN_INIT_CONTEXT *pCtx, CEDEVICE_POWER_STATE devPwrState, BOOL EnableAwakening)
{
	BOOL        canPowerOn;
	CAN_OPMODE  OpMode;
	//int        i;

    if( pCtx == NULL )
 	{
	 RETAILMSG(1, (L"CAN: CANBPowerOff: Invalid paremeter\r\n"));
	 return;
	}

    if( pCtx->cookie != CAN_DEVICE_COOKIE )
 	{
	 RETAILMSG(1, (L"CAN: CANBPowerOff: Invalid device context\r\n"));
	 return;
	}

	if( devPwrState == D0 )
     return;

	RETAILMSG(CAN_DRV_DBG, (L"CAN: +CANBPowerOff, Context=0x%08X, pwrState = %d\r\n", pCtx, devPwrState));

	InPowerHandler = 1;

    canPowerOn  = MCP2515PowerStatus(pCtx);

	      // ##### Michael
    if( ( devPwrState == D3 )||( devPwrState == D4 ) )// Switch Off MCP2515 Power
    {
     if( canPowerOn )
	 {
      LastOpMode = CANBGetOpMode();

	  // Set OpMode to OPMODE_CONFIGURATION
	  CANBSetOpMode(CAN_OPMODE_CONFIG);

      // Save Controller configuration
	  saveCNF(&gCANCNF);

	  MCP2515PowerOff(pCtx);
	 }
	 else
	 {
		 RETAILMSG(CAN_DRV_DBG, (L"CAN: +CANBPowerOff: CAN power is already Off! \r\n")); 
	 }
    }
	else
	{
       // Can be situaltion when CAN resumes with power state differ than D0
       // D1 or D2 for example

        if( !canPowerOn )
		{
		  MCP2515PowerOn(pCtx);
		  CANBResetMCP2515(); // Sets OpMode to Config

		  // restore previous configuration
		  restoreCNF(&gCANCNF);

		  CANBSetOpMode(LastOpMode);
		}

		OpMode = CANBGetOpMode();
		if (OpMode == CAN_OPMODE_SLEEP)
		{
			RETAILMSG(CAN_DRV_DBG, (L"CAN: CANBPowerOff is invoked, but MCP2515 is already in the Sleep Mode.\r\n"));
		}
		else
		{
			if (EnableAwakening)
			{
				RETAILMSG(CAN_DRV_DBG, (L"CAN: CANBPowerOff enables MCP2515 awakening.\r\n"));
				MCP2515BitModifySPI(CANINTE_ADDR, 1<<WAKI_bit, 1<<WAKI_bit);
				WaitForOST = TRUE;
			}
			else
			{
				RETAILMSG(CAN_DRV_DBG, (L"CAN: CANBPowerOff disables MCP2515 awakening.\r\n"));
				MCP2515BitModifySPI(CANINTE_ADDR, 1<<WAKI_bit, 0<<WAKI_bit);
			}
			RETAILMSG(CAN_DRV_DBG, (L"CAN: CANBPowerOff switches MCP2515 into the Sleep Mode.\r\n"));
			MCP2515BitModifySPI(CANCTRL_ADDR, REQOP_m, OPMODE_SLEEP<<REQOP_s);
			LastOpMode = OpMode;
		}
	}

  InPowerHandler = 0;

  RETAILMSG(CAN_DRV_DBG, (L"CAN: -CANBPowerOff, Context=0x%08X\r\n", pCtx));
}



// Function name	: saveCNF
// Description	    : Save Controller Configuration
// Return type		: BOOL  
// Argument         : PGENCANCONFIG

static BOOL saveCNF(PGENCANCONFIG pCANCNF)
{
  int i;

	if( pCANCNF == NULL )
      return(FALSE);

      // Save Controller configuration
	  if( !CANBGetCNF(&pCANCNF->CANcfg) )
       return(FALSE);

      pCANCNF->CANmask.Mask[0].bUpdate = TRUE;
      pCANCNF->CANmask.Mask[1].bUpdate = TRUE;
	  if( !CANBGetMask(&pCANCNF->CANmask) )
        return(FALSE);

     for( i = 0; i < 6; i++ )
      pCANCNF->CANfilter.Filter[i].bUpdate = TRUE;

	  if( !CANBGetFilter(&pCANCNF->CANfilter) )
		return(FALSE);

	  if( !CANBGetTimeOuts(&pCANCNF->CANtimeouts) )
		return(FALSE);

	  pCANCNF->CANrxb0opmode.RxBufferID = RXB0;
	  if( !CANBGetRXM(&pCANCNF->CANrxb0opmode) )
        return(FALSE);

	  pCANCNF->CANrxb1opmode.RxBufferID = RXB1;
	  if( !CANBGetRXM(&pCANCNF->CANrxb1opmode) )
        return(FALSE);

   return(TRUE);
}


static BOOL restoreCNF(PGENCANCONFIG pCANCNF)
{
  int i;

	if( pCANCNF == NULL )
      return(FALSE);

		  // restore previous configuration
		 if( !CANBSetCNF(&pCANCNF->CANcfg) )
		   return(FALSE);

		 pCANCNF->CANmask.Mask[0].bUpdate = TRUE;
         pCANCNF->CANmask.Mask[1].bUpdate = TRUE;
		 if( !CANBSetMask(&pCANCNF->CANmask) )
           return(FALSE);

         for( i = 0; i < 6; i++ )
          pCANCNF->CANfilter.Filter[i].bUpdate = TRUE;

		 if( !CANBSetFilter(&pCANCNF->CANfilter) )
           return(FALSE);

		 if( !CANBSetTimeOuts(&pCANCNF->CANtimeouts) )
		   return(FALSE);

         pCANCNF->CANrxb0opmode.RxBufferID = RXB0;
		 if( !CANBSetRXM(&pCANCNF->CANrxb0opmode) )
           return(FALSE);

         pCANCNF->CANrxb1opmode.RxBufferID = RXB1;
		 if( !CANBSetRXM(&pCANCNF->CANrxb1opmode) )
           return(FALSE);

   return(TRUE);
}

// Function name	: CANBSetOpMode
// Description	    : Sets MCP2515 Operation Mode
// Return type		: BOOL CANBSetOpMode 
// Argument         : CAN_OPMODE OpMode

BOOL CANBSetOpMode(CAN_OPMODE OpMode)
{
	BOOL					bRetVal			= FALSE;

	DEBUGMSG(ZONE_STEPS, (L"CAN: +CANBSetOpMode\r\n"));

	bRetVal = MCP2515SetModeSPI((MCPOPMODE)OpMode);

	if(OpMode == CAN_OPMODE_CONFIG) rxqReset();

	DEBUGMSG(ZONE_STEPS, (L"CAN: -CANBSetOpMode, bRetVal=0x%04X\r\n", bRetVal));

	return bRetVal;
}

CAN_OPMODE  CANBGetOpMode(void)
{
  return( (CAN_OPMODE)MCP2515GetModeSPI() );
}


// Function name	: CANBSetCNF
// Description	    : Set CAN Configuration, serves IO_Control calls
// Return type		: BOOL CANBSetCNF 
// Argument         : PCANCONFIG pCNF

BOOL CANBSetCNF ( PCANCONFIG pCNF )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBSetCNF\r\n" ) ) );

	BOOL			bRetVal			= TRUE;
	CANREGSETTINGS	tCANREGSETTINGS	= { 0 };


	tCANREGSETTINGS.PHSEG2	=	pCNF->PHSEG2;
	tCANREGSETTINGS.WAKFIL	=	pCNF->WAKFIL;
	tCANREGSETTINGS.PRSEG	=	pCNF->PRSEG;
	tCANREGSETTINGS.PHSEG1	=	pCNF->PHSEG1;
	tCANREGSETTINGS.SAM		=	pCNF->SAM;
	tCANREGSETTINGS.BTLMODE	=	pCNF->BTLMODE;
	tCANREGSETTINGS.BRP		=	pCNF->BRP;
	tCANREGSETTINGS.SJW		=	pCNF->SJW;

	if ( TRUE == ValidateCNFParameters ( &tCANREGSETTINGS ) )
	{
		if ( FALSE == MCP2515SetCNF ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;
		}
	}
	else
	{
		bRetVal = FALSE;
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBSetCNF, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetCNF
// Description	    : Get CAN Configuration, serves IO_Control calls
// Return type		: BOOL CANBGetCNF 
// Argument         : PCANCONFIG pCNF

BOOL CANBGetCNF ( PCANCONFIG pCNF )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBGetCNF\r\n" ) ) );

	BOOL	bRetVal	= TRUE;
	CNF_DT	tCNF	= { 0 };


	if ( FALSE == MCP2515GetCNF ( &tCNF ) )
	{
		bRetVal = FALSE;

		memset ( pCNF, 0x00, sizeof ( CANCONFIG ) );
	}
	else
	{
		// CAN Configuration, Bit Timing Issues, Wake-Up Filter and SOF Pin
		pCNF->PHSEG2	= tCNF.PHSEG2;
		pCNF->WAKFIL	= tCNF.WAKFIL;
		pCNF->PRSEG		= tCNF.PRSEG;
		pCNF->PHSEG1	= tCNF.PHSEG1;
		pCNF->SAM		= tCNF.SAM;
		pCNF->BTLMODE	= tCNF.BTLMODE;
		pCNF->BRP		= tCNF.BRP;
		pCNF->SJW		= tCNF.SJW;
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBGetCNF, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetMask
// Description	    : Set Rx Buffer Mask
// Return type		: BOOL CANBSetMask 
// Argument         : PCANMASK pMASK

BOOL CANBSetMask ( PCANMASK pMASK )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBSetMask\r\n" ) ) );

	BOOL			bRetVal			= TRUE;
	CANREGSETTINGS	tCANREGSETTINGS	= { 0 };

	
	// Mask for Rx Buffer #0
	if ( TRUE == pMASK->Mask[ 0 ].bUpdate )
	{
		tCANREGSETTINGS.SIDMASK0 = pMASK->Mask[ 0 ].SID;
		tCANREGSETTINGS.EIDMASK0 = pMASK->Mask[ 0 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetMask ( RXM0SIDH_ADDR, 
									   tCANREGSETTINGS.SIDMASK0, 
									   tCANREGSETTINGS.EIDMASK0 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Mask for Rx Buffer #1
	if ( TRUE == pMASK->Mask[ 1 ].bUpdate )
	{
		tCANREGSETTINGS.SIDMASK1 = pMASK->Mask[ 1 ].SID;
		tCANREGSETTINGS.EIDMASK1 = pMASK->Mask[ 1 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetMask ( RXM1SIDH_ADDR, 
									   tCANREGSETTINGS.SIDMASK1, 
									   tCANREGSETTINGS.EIDMASK1 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}


END:

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBSetMask, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetMask
// Description	    : Get Rx Buffer Mask
// Return type		: BOOL CANBGetMask 
// Argument         : PCANMASK pMASK

BOOL CANBGetMask ( PCANMASK pMASK )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBGetMask\r\n" ) ) );

	BOOL			bRetVal			= TRUE;
	CANREGSETTINGS	tCANREGSETTINGS	= { 0 };

	
	// Mask of Rx Buffer #0
	if ( TRUE == pMASK->Mask[ 0 ].bUpdate )
	{
		if ( FALSE == MCP2515GetMask ( RXM0SIDH_ADDR, 
									   &( pMASK->Mask[ 0 ].SID ), 
									   &( pMASK->Mask[ 0 ].EID ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Mask of Rx Buffer #1
	if ( TRUE == pMASK->Mask[ 1 ].bUpdate )
	{
		if ( FALSE == MCP2515GetMask ( RXM1SIDH_ADDR, 
									   &( pMASK->Mask[ 1 ].SID ), 
									   &( pMASK->Mask[ 1 ].EID ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}


END:

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBGetMask, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetFilter
// Description	    : Set Filter of Rx Buffers
// Return type		: BOOL CANBSetFilter 
// Argument         : PCANFILTER pFILTER

BOOL CANBSetFilter ( PCANFILTER pFILTER )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBSetFilter\r\n" ) ) );

	BOOL			bRetVal			= TRUE;
	CANREGSETTINGS	tCANREGSETTINGS	= { 0 };

	
	// Filter for Rx Buffer #0
	if ( TRUE == pFILTER->Filter[ 0 ].bUpdate )
	{
		tCANREGSETTINGS.SIDFILTER0 = pFILTER->Filter[ 0 ].SID;
		tCANREGSETTINGS.FILSWITCH0 = pFILTER->Filter[ 0 ].bFISATO;
		tCANREGSETTINGS.EIDFILTER0 = pFILTER->Filter[ 0 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetFilter ( RXF0SIDH_ADDR, 
										 tCANREGSETTINGS.SIDFILTER0, 
										 tCANREGSETTINGS.EIDFILTER0,
										 tCANREGSETTINGS.FILSWITCH0 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #1
	if ( TRUE == pFILTER->Filter[ 1 ].bUpdate )
	{
		tCANREGSETTINGS.SIDFILTER1 = pFILTER->Filter[ 1 ].SID;
		tCANREGSETTINGS.FILSWITCH1 = pFILTER->Filter[ 1 ].bFISATO;
		tCANREGSETTINGS.EIDFILTER1 = pFILTER->Filter[ 1 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetFilter ( RXF1SIDH_ADDR, 
										 tCANREGSETTINGS.SIDFILTER1, 
										 tCANREGSETTINGS.EIDFILTER1,
										 tCANREGSETTINGS.FILSWITCH1 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #2
	if ( TRUE == pFILTER->Filter[ 2 ].bUpdate )
	{
		tCANREGSETTINGS.SIDFILTER2 = pFILTER->Filter[ 2 ].SID;
		tCANREGSETTINGS.FILSWITCH2 = pFILTER->Filter[ 2 ].bFISATO;
		tCANREGSETTINGS.EIDFILTER2 = pFILTER->Filter[ 2 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetFilter ( RXF2SIDH_ADDR, 
										 tCANREGSETTINGS.SIDFILTER2, 
										 tCANREGSETTINGS.EIDFILTER2,
										 tCANREGSETTINGS.FILSWITCH2 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #3
	if ( TRUE == pFILTER->Filter[ 3 ].bUpdate )
	{
		tCANREGSETTINGS.SIDFILTER3 = pFILTER->Filter[ 3 ].SID;
		tCANREGSETTINGS.FILSWITCH3 = pFILTER->Filter[ 3 ].bFISATO;
		tCANREGSETTINGS.EIDFILTER3 = pFILTER->Filter[ 3 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetFilter ( RXF3SIDH_ADDR, 
										 tCANREGSETTINGS.SIDFILTER3, 
										 tCANREGSETTINGS.EIDFILTER3,
										 tCANREGSETTINGS.FILSWITCH3 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #4
	if ( TRUE == pFILTER->Filter[ 4 ].bUpdate )
	{
		tCANREGSETTINGS.SIDFILTER4 = pFILTER->Filter[ 4 ].SID;
		tCANREGSETTINGS.FILSWITCH4 = pFILTER->Filter[ 4 ].bFISATO;
		tCANREGSETTINGS.EIDFILTER4 = pFILTER->Filter[ 4 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetFilter ( RXF4SIDH_ADDR, 
										 tCANREGSETTINGS.SIDFILTER4, 
										 tCANREGSETTINGS.EIDFILTER4,
										 tCANREGSETTINGS.FILSWITCH4 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #5
	if ( TRUE == pFILTER->Filter[ 5 ].bUpdate )
	{
		tCANREGSETTINGS.SIDFILTER5 = pFILTER->Filter[ 5 ].SID;
		tCANREGSETTINGS.FILSWITCH5 = pFILTER->Filter[ 5 ].bFISATO;
		tCANREGSETTINGS.EIDFILTER5 = pFILTER->Filter[ 5 ].EID;

		if ( FALSE == ValidateMasksAndFilters ( &tCANREGSETTINGS ) )
		{
			bRetVal = FALSE;

			goto END;
		}

		if ( FALSE == MCP2515SetFilter ( RXF5SIDH_ADDR, 
										 tCANREGSETTINGS.SIDFILTER5, 
										 tCANREGSETTINGS.EIDFILTER5,
										 tCANREGSETTINGS.FILSWITCH5 ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}


END:

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBSetFilter, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetFilter
// Description	    : Get Filter of Rx Buffers
// Return type		: BOOL CANBGetFilter 
// Argument         : PCANFILTER pFILTER

BOOL CANBGetFilter ( PCANFILTER pFILTER )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBGetFilter\r\n" ) ) );

	BOOL bRetVal = TRUE;

	
	// Filter for Rx Buffer #0
	if ( TRUE == pFILTER->Filter[ 0 ].bUpdate )
	{
		if ( FALSE == MCP2515GetFilter ( RXF0SIDH_ADDR, 
										 &( pFILTER->Filter[ 0 ].SID ), 
										 &( pFILTER->Filter[ 0 ].EID ),
										 &( pFILTER->Filter[ 0 ].bFISATO ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #1
	if ( TRUE == pFILTER->Filter[ 1 ].bUpdate )
	{
		if ( FALSE == MCP2515GetFilter ( RXF1SIDH_ADDR, 
										 &( pFILTER->Filter[ 1 ].SID ), 
										 &( pFILTER->Filter[ 1 ].EID ),
										 &( pFILTER->Filter[ 1 ].bFISATO ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #2
	if ( TRUE == pFILTER->Filter[ 2 ].bUpdate )
	{
		if ( FALSE == MCP2515GetFilter ( RXF2SIDH_ADDR, 
										 &( pFILTER->Filter[ 2 ].SID ), 
										 &( pFILTER->Filter[ 2 ].EID ),
										 &( pFILTER->Filter[ 2 ].bFISATO ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #3
	if ( TRUE == pFILTER->Filter[ 3 ].bUpdate )
	{
		if ( FALSE == MCP2515GetFilter ( RXF3SIDH_ADDR, 
										 &( pFILTER->Filter[ 3 ].SID ), 
										 &( pFILTER->Filter[ 3 ].EID ),
										 &( pFILTER->Filter[ 3 ].bFISATO ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #4
	if ( TRUE == pFILTER->Filter[ 4 ].bUpdate )
	{
		if ( FALSE == MCP2515GetFilter ( RXF4SIDH_ADDR, 
										 &( pFILTER->Filter[ 4 ].SID ), 
										 &( pFILTER->Filter[ 4 ].EID ),
										 &( pFILTER->Filter[ 4 ].bFISATO ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}

	// Filter for Rx Buffer #5
	if ( TRUE == pFILTER->Filter[ 5 ].bUpdate )
	{
		if ( FALSE == MCP2515GetFilter ( RXF5SIDH_ADDR, 
										 &( pFILTER->Filter[ 5 ].SID ), 
										 &( pFILTER->Filter[ 5 ].EID ),
										 &( pFILTER->Filter[ 5 ].bFISATO ) ) )
		{
			bRetVal = FALSE;

			goto END;
		}
	}


END:

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBGetFilter, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetTimeOuts
// Description	    : Set Time Outs
// Return type		: BOOL CANBSetTimeOuts 
// Argument         : PCANRWTIMEOUTS pTO

BOOL CANBSetTimeOuts ( PCANRWTIMEOUT pTO )
{
	DEBUGMSG(ZONE_STEPS, (_T("CAN: +CANBSetTimeOuts\r\n")));

	if((pTO->dwReadTimeOut < 10) || (pTO->dwWriteTimeOut < 10))
		return 0;
	
	g_dwReadTimeout  = pTO->dwReadTimeOut;
	g_dwWriteTimeout = pTO->dwWriteTimeOut;

	DEBUGMSG(ZONE_STEPS, (_T("CAN: -CANBSetTimeOuts\r\n")));

	return 1;
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetTimeOuts
// Description	    : Get Time Outs
// Return type		: BOOL CANBGetTimeOuts 
// Argument         : PCANRWTIMEOUTS pTO

BOOL CANBGetTimeOuts ( PCANRWTIMEOUT pTO )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBGetTimeOuts\r\n" ) ) );

	BOOL bRetVal = TRUE;

	
	pTO->dwReadTimeOut  = g_dwReadTimeout;
	pTO->dwWriteTimeOut = g_dwWriteTimeout;


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBGetTimeOuts, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetRXM
// Description	    : Set Receive Buffer Operating Mode
// Return type		: BOOL CANBSetRXM 
// Argument         : PCANRXBOPMODE pRXOPMODE

BOOL CANBSetRXM ( PCANRXBOPMODE pRXOPMODE )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBSetRXM\r\n" ) ) );

	BOOL			bRetVal		= TRUE;
	unsigned __int8 uAddress	= 0;

	
	switch ( pRXOPMODE->RxBufferID )
	{
	case RXB0 :

		uAddress = RXB0CTRL_ADDR;

		break;

	case RXB1 :

		uAddress = RXB1CTRL_ADDR;

		break;

	default :

		bRetVal = FALSE;

		goto END;
	}

	bRetVal = MCP2515SetRXM ( uAddress, 
							  pRXOPMODE->Mode );

END:

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBSetRXM, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetRXM
// Description	    : Get Receive Buffer Operating Mode
// Return type		: BOOL CANBGetRXM 
// Argument         : PCANRXBOPMODE pRXOPMODE

BOOL CANBGetRXM ( PCANRXBOPMODE pRXOPMODE )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBGetRXM\r\n" ) ) );

	BOOL			bRetVal		= TRUE;
	unsigned __int8 uAddress	= 0;

	
	switch ( pRXOPMODE->RxBufferID )
	{
	case RXB0 :

		uAddress = RXB0CTRL_ADDR;

		break;

	case RXB1 :

		uAddress = RXB1CTRL_ADDR;

		break;

	default :

		bRetVal = FALSE;

		goto END;
	}

	bRetVal = MCP2515GetRXM ( uAddress, 
							  &( pRXOPMODE->Mode ) );

END:

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBGetRXM, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBResetMCP2515
// Description	    : Reset the MCP2515 Device
// Return type		: BOOL CANBResetMCP2515 

BOOL CANBResetMCP2515 ( )
{
	DEBUGMSG(ZONE_STEPS, (L"CAN: +CANBResetMCP2515\r\n"));

	MCP2515ResetSPI();	// MCP2515HwReset();  //alternatively

	//The API does not cover the following settings. So they must be configured here.
	{
	// CLKOUT Pin Disable
	MCP2515BitModifySPI(CANCTRL_ADDR, CLKEN_m, (OFF << CLKEN_s));
	// Set Roll Over Feature
	MCP2515BitModifySPI(RXB0CTRL_ADDR, FILHIT_m, (0x04 << FILHIT_s));
	// Enable Default Selected Interrupts
	MCP2515BitModifySPI(CANINTE_ADDR, DEFAULT_INTR_ENABLE, DEFAULT_INTR_ENABLE);
	}

	DEBUGMSG(ZONE_STEPS, (L"CAN: +CANBResetMCP2515\r\n"));

	return TRUE;
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetErrors
// Description	    : Get Errors
// Return type		: BOOL CANBGetErrors 
// Argument         : DWORD *pdwERRORS

BOOL CANBGetErrors ( DWORD *pdwERRORS )
{
	DEBUGMSG ( ZONE_STEPS, ( _T ( "CAN: +CANBGetErrors\r\n" ) ) );
	RETAILMSG(CAN_DRV_DBG, (_T("CAN: +CANBGetErrors\r\n")));

	BOOL bRetVal = TRUE;


	EnterCriticalSection ( &g_csERRORSMANIPULATIONS );

	*pdwERRORS		= g_dwCANERRORS;

	// In case there was FATAL Error detected, 
	// all errors in Driver will be saved.
	if ( 0 == ( CANERR_FATALERROR & g_dwCANERRORS ) )
	{
		g_dwCANERRORS	= 0;
	}

	LeaveCriticalSection ( &g_csERRORSMANIPULATIONS );


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBGetErrors, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	RETAILMSG(CAN_DRV_DBG, (_T("CAN: -CANBGetErrors = 0x%08X\r\n"), g_dwCANERRORS ));

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBCreateEvents
// Description	    : Creates all required events
// Return type		: BOOL CANBCreateEvents 

BOOL CANBCreateEvents(CAN_INIT_CONTEXT  *pCxt)
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBCreateEvents\r\n" ) ) );

	BOOL bRetVal = TRUE;


	g_hIntrEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hIntrEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	pCxt->hIntrEvent = g_hIntrEvent;

	g_hTxB0ReadyEvent = CreateEvent ( NULL, FALSE, TRUE, NULL );

	if ( NULL == g_hTxB0ReadyEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB1ReadyEvent = CreateEvent ( NULL, FALSE, TRUE, NULL );

	if ( NULL == g_hTxB1ReadyEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB2ReadyEvent = CreateEvent ( NULL, FALSE, TRUE, NULL );

	if ( NULL == g_hTxB2ReadyEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB0CompleteEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hTxB0CompleteEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB1CompleteEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hTxB1CompleteEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB2CompleteEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hTxB2CompleteEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB0AbortEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hTxB0AbortEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB1AbortEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hTxB1AbortEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	g_hTxB2AbortEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hTxB2AbortEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	rxqNotEmptyEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == rxqNotEmptyEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

    g_hEchoEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	if ( NULL == g_hEchoEvent )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CANBCreateEvents, CreateEvent FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	pCxt->hEchoEvent = g_hEchoEvent;


END:

	if ( FALSE == bRetVal )
	{
		CANBCloseEvents ( );
	}
	
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBCreateEvents, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CANBCloseEvents
// Description	    : Closes all events
// Return type		: void CANBCloseEvents 

void CANBCloseEvents ( )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CANBCloseEvents\r\n" ) ) );


	// Close IST Event g_hIntrEvent
	if ( NULL != g_hIntrEvent )
	{
		CloseHandle ( g_hIntrEvent );
		g_hIntrEvent = NULL;
	}

	// Close Tx Buffer Ready Event
	if ( NULL != g_hTxB0ReadyEvent )
	{
		CloseHandle ( g_hTxB0ReadyEvent );
		g_hTxB0ReadyEvent = NULL;
	}

	// Close Tx Buffer Ready Event
	if ( NULL != g_hTxB1ReadyEvent )
	{
		CloseHandle ( g_hTxB1ReadyEvent );
		g_hTxB1ReadyEvent = NULL;
	}

	// Close Tx Buffer Ready Event
	if ( NULL != g_hTxB2ReadyEvent )
	{
		CloseHandle ( g_hTxB2ReadyEvent );
		g_hTxB2ReadyEvent = NULL;
	}

	// Close Tx Complete Event
	if ( NULL != g_hTxB0CompleteEvent )
	{
		CloseHandle ( g_hTxB0CompleteEvent );
		g_hTxB0CompleteEvent = NULL;
	}

	// Close Tx Complete Event
	if ( NULL != g_hTxB1CompleteEvent )
	{
		CloseHandle ( g_hTxB1CompleteEvent );
		g_hTxB1CompleteEvent = NULL;
	}

	// Close Tx Complete Event
	if ( NULL != g_hTxB2CompleteEvent )
	{
		CloseHandle ( g_hTxB2CompleteEvent );
		g_hTxB2CompleteEvent = NULL;
	}

	// Close Tx Abort Event
	if ( NULL != g_hTxB0AbortEvent )
	{
		CloseHandle ( g_hTxB0AbortEvent );
		g_hTxB0AbortEvent = NULL;
	}

	// Close Tx Abort Event
	if ( NULL != g_hTxB1AbortEvent )
	{
		CloseHandle ( g_hTxB1AbortEvent );
		g_hTxB1AbortEvent = NULL;
	}

	// Close Tx Abort Event
	if ( NULL != g_hTxB2AbortEvent )
	{
		CloseHandle ( g_hTxB2AbortEvent );
		g_hTxB2AbortEvent = NULL;
	}

	// Close Rx Msg Queue Not Empty Event
	if ( NULL != rxqNotEmptyEvent )
	{
		CloseHandle ( rxqNotEmptyEvent );
		rxqNotEmptyEvent = NULL;
	}

	if ( NULL != g_hEchoEvent )
	{
		CloseHandle ( g_hEchoEvent );
		g_hEchoEvent = NULL;
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CANBCloseEvents\r\n" ) ) );
}
