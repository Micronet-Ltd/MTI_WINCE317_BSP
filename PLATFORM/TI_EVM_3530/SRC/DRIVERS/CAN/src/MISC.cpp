// ================================================================================================
//
// Giant Steps Ltd.
// 
// Misc. Functions
//
// ================================================================================================

#include <Windows.h>

#include "MISC.h"
#include "Debug.h"


// Globals and Externals ==========================================================================

// Timeout for Read / Write Operations, Default 1 Hr
extern DWORD	g_dwReadTimeout;
extern DWORD	g_dwWriteTimeout;

// Misc. Functions ================================================================================

// Function name	: RegGetParameters
// Description	    : Get all values from registry in one call
// Return type		: BOOL RegGetParameters 
// Argument         : PCANREGSETTINGS pCANREGSETTINGS

BOOL RegGetParameters ( PCANREGSETTINGS pCANREGSETTINGS )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +RegGetParameters\r\n" ) ) );

	BOOL				bRetVal	= TRUE;
	DWORD				dwSize	= 0,
						dwType	= REG_DWORD;
	unsigned __int32	uData	= 0;
	HKEY				hKey	= NULL;


	if ( ERROR_SUCCESS != RegOpenKeyEx ( HKEY_LOCAL_MACHINE, 
										 CANDRVREG, 
										 0, 
										 0, 
										 &hKey ) )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegOpenKeyEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;
	}

	// BRP
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_BRP, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->BRP = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;		
	}

	// BTLMODE
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_BTL, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->BTLMODE = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;		
	}

	// SAM
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SAMPLES, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		//pCANREGSETTINGS->BTLMODE = uData;
		// ##### Bug ! which erased BTLMODE by SAM value !!!
		pCANREGSETTINGS->SAM = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;		
	}

	// PHSEG2
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_PHASESEG2, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->PHSEG2 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;		
	}

	// PHSEG1
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_PHASESEG1, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->PHSEG1 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;

		goto END;		
	}

	// PRSEG
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_PROPSEG, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->PRSEG = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SJW
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SJW, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SJW = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// WAKFIL
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_WAKFIL, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->WAKFIL = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// RTO
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_RTO, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->RTO = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// WTO
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_WTO, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->WTO = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// RXMB0
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_RXMB0,
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->RXMB0 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// RXMB1
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_RXMB1,
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->RXMB1 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDMASK0
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDMASK0, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDMASK0 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDMASK0
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDMASK0, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDMASK0 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDMASK1
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDMASK1, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDMASK1 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDMASK1
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDMASK1, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDMASK1 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDFILTER0
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDFILTER0, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDFILTER0 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// FILSWITCH0
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_FILSWITCH0, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->FILSWITCH0 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDFILTER0
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDFILTER0, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDFILTER0 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDFILTER1
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDFILTER1, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDFILTER1 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// FILSWITCH1
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_FILSWITCH1, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->FILSWITCH1 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDFILTER1
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDFILTER1, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDFILTER1 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDFILTER2
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDFILTER2, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDFILTER2 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// FILSWITCH2
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_FILSWITCH2, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->FILSWITCH2 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDFILTER2
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDFILTER2, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDFILTER2 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDFILTER3
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDFILTER3, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDFILTER3 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// FILSWITCH3
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_FILSWITCH3, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->FILSWITCH3 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDFILTER3
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDFILTER3, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDFILTER3 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDFILTER4
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDFILTER4, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDFILTER4 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// FILSWITCH4
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_FILSWITCH4, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->FILSWITCH4 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDFILTER4
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDFILTER4, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDFILTER4 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// SIDFILTER5
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_SIDFILTER5, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->SIDFILTER5 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// FILSWITCH5
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_FILSWITCH5, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->FILSWITCH5 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}

	// EIDFILTER5
	dwSize = sizeof ( uData );

	if ( ERROR_SUCCESS == RegQueryValueEx ( hKey, 
											CAN_EIDFILTER5, 
											NULL, 
											&dwType, 
											( BYTE * )&uData, 
											&dwSize ) )
	{
		pCANREGSETTINGS->EIDFILTER5 = uData;
	}
	else
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: RegQueryValueEx, FAILED!!! err#0x%04X\r\n" ), \
				   GetLastError ( ) ) );

		bRetVal = FALSE;	
	}


END:

	if ( NULL != hKey )
	{
		RegCloseKey ( hKey );
	}

	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -RegGetParameters, bRetVal=0x%04X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: ValidateCNFParameters
// Description	    : Checks Configuration Parameters Validity
// Return type		: BOOL ValidateCNFParameters 
// Argument         : PCANREGSETTINGS pCANREGSETTINGS

BOOL ValidateCNFParameters ( PCANREGSETTINGS pCANREGSETTINGS )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +ValidateCNFParameters\r\n" ) ) );

	BOOL bRetVal = TRUE;


	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, PRSEG	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->PRSEG ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, PHSEG1	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->PHSEG1 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, PHSEG2	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->PHSEG2 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, BTLMODE	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->BTLMODE ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, BRP		=0x%04X\r\n" ), \
			   pCANREGSETTINGS->BRP ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, SAM		=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SAM ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, SJW		=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SJW ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateCNFParameters, WAKFIL	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->WAKFIL ) );


	if ( 7 < pCANREGSETTINGS->PRSEG )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, PRSEG=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->PRSEG ) );

		bRetVal = FALSE;
	}

	if ( 7 < pCANREGSETTINGS->PHSEG1 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, PHSEG1=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->PHSEG1) );

		bRetVal = FALSE;
	}

	if ( 7 < pCANREGSETTINGS->PHSEG2 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, PHSEG2=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->PHSEG2) );

		bRetVal = FALSE;
	}

	if ( 1 < pCANREGSETTINGS->BTLMODE )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, BTLMODE=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->BTLMODE) );

		bRetVal = FALSE;
	}

	if ( 63 < pCANREGSETTINGS->BRP )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, BRP=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->BRP) );

		bRetVal = FALSE;
	}

	if ( 1 < pCANREGSETTINGS->SAM )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, SAM=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->SAM) );

		bRetVal = FALSE;
	}

	if ( 3 < pCANREGSETTINGS->SJW )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, SJW=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->SJW) );

		bRetVal = FALSE;
	}

	if ( 1 < pCANREGSETTINGS->WAKFIL )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateCNFParameters, WAKFIL=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->WAKFIL) );

		bRetVal = FALSE;
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -ValidateCNFParameters, bRetVal=0x%04X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: ValidateMasksAndFilters
// Description	    : Checks Masks and Filters Validity
// Return type		: BOOL ValidateMasksAndFilters 
// Argument         : PCANREGSETTINGS pCANREGSETTINGS

BOOL ValidateMasksAndFilters ( PCANREGSETTINGS pCANREGSETTINGS )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +ValidateCNFParameters\r\n" ) ) );

	BOOL bRetVal = TRUE;

	
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDMASK0	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDMASK0 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDMASK0	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDMASK0 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDMASK1	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDMASK1 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDMASK1	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDMASK1 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDFILTER0	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDFILTER0 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH0	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->FILSWITCH0 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDFILTER0	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDFILTER0 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDFILTER1	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDFILTER1 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH1	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->FILSWITCH1 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDFILTER1	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDFILTER1 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDFILTER2	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDFILTER2 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH2	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->FILSWITCH2 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDFILTER2	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDFILTER2 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDFILTER3	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDFILTER3 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH3	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->FILSWITCH3 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDFILTER3	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDFILTER3 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDFILTER4	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDFILTER4 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH4	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->FILSWITCH4 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDFILTER4	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDFILTER4 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, SIDFILTER5	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->SIDFILTER5 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH5	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->FILSWITCH5 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateMasksAndFilters, EIDFILTER5	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->EIDFILTER5 ) );

	
	// Check Mask #0
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDMASK0 );
	bRetVal = CheckEID ( pCANREGSETTINGS->EIDMASK0 );

	// Check Mask #1
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDMASK1 );
	bRetVal = CheckEID ( pCANREGSETTINGS->EIDMASK1 );

	// Check Filter #0
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDFILTER0 );

	if ( 1 < pCANREGSETTINGS->FILSWITCH0 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH0=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->FILSWITCH0 ) );

		bRetVal = FALSE;
	}

	bRetVal = CheckEID ( pCANREGSETTINGS->EIDFILTER0 );

	// Check Filter #1
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDFILTER1 );

	if ( 1 < pCANREGSETTINGS->FILSWITCH1 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH1=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->FILSWITCH1 ) );

		bRetVal = FALSE;
	}

	bRetVal = CheckEID ( pCANREGSETTINGS->EIDFILTER1 );

	// Check Filter #2
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDFILTER2 );

	if ( 1 < pCANREGSETTINGS->FILSWITCH2 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH2=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->FILSWITCH2 ) );

		bRetVal = FALSE;
	}

	bRetVal = CheckEID ( pCANREGSETTINGS->EIDFILTER2 );

	// Check Filter #3
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDFILTER3 );

	if ( 1 < pCANREGSETTINGS->FILSWITCH3 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH3=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->FILSWITCH3 ) );

		bRetVal = FALSE;
	}

	bRetVal = CheckEID ( pCANREGSETTINGS->EIDFILTER3 );

	// Check Filter #4
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDFILTER4 );

	if ( 1 < pCANREGSETTINGS->FILSWITCH4 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH4=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->FILSWITCH4 ) );

		bRetVal = FALSE;
	}

	bRetVal = CheckEID ( pCANREGSETTINGS->EIDFILTER4 );

	// Check Filter #5
	bRetVal = CheckSID ( pCANREGSETTINGS->SIDFILTER5 );

	if ( 1 < pCANREGSETTINGS->FILSWITCH5 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateMasksAndFilters, FILSWITCH5=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->FILSWITCH5 ) );

		bRetVal = FALSE;
	}

	bRetVal = CheckEID ( pCANREGSETTINGS->EIDFILTER5 );


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -ValidateMasksAndFilters, bRetVal=0x%04X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: ValidateRXMParameters
// Description	    : Checks Receive Buffer Operating Mode Parameters
// Return type		: BOOL ValidateRXMParameters 
// Argument         : PCANREGSETTINGS pCANREGSETTINGS

BOOL ValidateRXMParameters ( PCANREGSETTINGS pCANREGSETTINGS )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +ValidateRXMParameters\r\n" ) ) );

	BOOL bRetVal = TRUE;


	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateRXMParameters, RXMB0	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->RXMB0 ) );
	DEBUGMSG ( ZONE_MCP2515, \
			   ( _T ( "CAN: ValidateRXMParameters, RXMB1	=0x%04X\r\n" ), \
			   pCANREGSETTINGS->RXMB1 ) );

	if ( 3 < pCANREGSETTINGS->RXMB0 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateRXMParameters, RXMB0=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->RXMB0 ) );

		bRetVal = FALSE;
	}

	if ( 3 < pCANREGSETTINGS->RXMB1 )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: ValidateRXMParameters, RXMB1=0x%04X WRONG!!!\r\n" ), \
				   pCANREGSETTINGS->RXMB1 ) );

		bRetVal = FALSE;
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -ValidateRXMParameters, bRetVal=0x%04X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CheckSID
// Description	    : Check the Standard Identifier Validity
// Return type		: BOOL CheckSID 
// Argument         : unsigned __int32 SID

BOOL CheckSID ( unsigned __int32 SID )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CheckSID\r\n" ) ) );

	RETAILMSG(0, ( _T ( "CAN: +CheckSID\r\n" ) ) );

	BOOL bRetVal = TRUE;


	if ( 0x000007FF < SID )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CheckSID, SID=0x%04X WRONG!!!\r\n" ), \
				   SID ) );

		RETAILMSG(0, ( _T ( "CAN: CheckSID, SID=0x%04X WRONG!!!\r\n" ), SID ) );

		bRetVal = FALSE;
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CheckSID, bRetVal=0x%04X\r\n" ), \
			   bRetVal ) );

	RETAILMSG(0, ( _T ( "CAN: -CheckSID, bRetVal=0x%04X\r\n" ), bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: CheckEID
// Description	    : Check the Standard Identifier Validity
// Return type		: BOOL CheckEID 
// Argument         : unsigned __int32 EID

BOOL CheckEID ( unsigned __int32 EID )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +CheckEID\r\n" ) ) );

	RETAILMSG(0, ( _T ( "CAN: +CheckEID\r\n" ) ) );

	BOOL bRetVal = TRUE;


	if ( 0x0003FFFF < EID )
	{
		DEBUGMSG ( ZONE_ERROR, \
				   ( _T ( "CAN: CheckSID, EID=0x%04X WRONG!!!\r\n" ), \
				   EID ) );

		RETAILMSG(0, ( _T ( "CAN: CheckSID, EID=0x%04X WRONG!!!\r\n" ),EID ) );

		bRetVal = FALSE;
	}


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -CheckEID, bRetVal=0x%04X\r\n" ), \
			   bRetVal ) );

	RETAILMSG(0, ( _T ( "CAN: -CheckEID, bRetVal=0x%04X\r\n" ), bRetVal ) );

	return ( bRetVal );
}

// ------------------------------------------------------------------------------------------------

// Function name	: SetTimeOuts
// Description	    : Set values for globals g_dwReadTimeout and g_dwWriteTimeout
// Return type		: BOOL SetTimeOuts 
// Argument         : PCANREGSETTINGS pCANREGSETTINGS

BOOL SetTimeOuts ( PCANREGSETTINGS pCANREGSETTINGS )
{
	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: +SetTimeOuts\r\n" ) ) );

	BOOL bRetVal = TRUE;


	DEBUGMSG ( ZONE_CAN, \
			   ( _T ( "CAN: SetTimeOuts, RTO=0x%04X, WTO=0x%04X\r\n" ), \
			   pCANREGSETTINGS->RTO, pCANREGSETTINGS->WTO ) );

	g_dwReadTimeout  = pCANREGSETTINGS->RTO;
	g_dwWriteTimeout = pCANREGSETTINGS->WTO;


	DEBUGMSG ( ZONE_STEPS, \
			   ( _T ( "CAN: -SetTimeOuts, bRetVal=0x%08X\r\n" ), \
			   bRetVal ) );

	return ( bRetVal );
}


// As in 'system.c'
void DelayMS(DWORD dwMilliSeconds, BOOL bInPowerHandler)
{
    if (bInPowerHandler)
    {
        DWORD StartingTickCount;

        StartingTickCount = GetTickCount();
        while (GetTickCount() - StartingTickCount < dwMilliSeconds + 1)
        {
        }       
    }
    else
    {
        Sleep(dwMilliSeconds);
    }
}
