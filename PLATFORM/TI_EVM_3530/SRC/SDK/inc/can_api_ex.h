#ifndef __CAN_API_H
#define __CAN_API_H

#include <windows.h>
#include "CanOpenLib_HW.h"

// MCP2515 Buffers Masks and Filters Organization =================================================
// This scheme shows dependances between Masks and Filters of RXB0 and RXB1.
// Actualy upper layer of CAN Bus Implementation should take care of this dependances ONLY
// when planing addresses for Devices on the CAN Bus and packets filtering.
//
//
//
//		[TXB0] [TXB1] [TXB2]										   Acceptance Mask
//		  |		 |		|											   |	*RXM1*
//		  |		 |		|											   |
//		  |		 |		|											   Acceptance Filter---
//		  |		 |		|											   |	-RXF2-		   |
//		  |		 |		|											   |				   | a
//	  {Message Queue Control}				  Acceptance Mask		   Acceptance Filter---| c
//		Tx Buffer priority	 					  *RXM0*	 |		   |	-RXF3-		   | c
//				 |				    						 |		   |				   | e
//				 |				  		  ---Acceptance Filter		   Acceptance Filter---| p
//				 |				  	   a |		  -RXF0-	 |		   |	-RXF4-		   | t
//				 |				  	   c |					 |		   |				   |
//				 |				  	   c |---Acceptance Filter		   Acceptance Filter---|
//				 |				  	   e |		  -RXF1-					-RXF5-	       |
//				 |					   p |		   |  							|		   |
//				 |					   t |		   |  							|		   |
//				 |					     |		   |   Message Assembly Buffer	|		   |
//				 |					   [RXB0]<------------------[MAB]------------------->[RXB1]
//				 |												  |
//				 |________________________________________________|
//										|	|
//										|	|
//								{###################}
//										|   |
//										|   |
//					    CAN Bus		~~~~*~~~|~~~~~~~~ CAN Bus
//								~~~~~~~~~~~~*~~~~
//
//
//
// Scheme of how Masks and Filters are applied to CAN Frames ======================================
// This scheme shows how packet acceptance mechanism uses Masks and Filters
// when Standard or Extended frame received in MAB ( Message Assembly Buffer )
//
//
//	1. Standard Data Frame:
//
//
//		Mask	: Both parts of Mask - Standard and Extended are applied on frame,
//				  0 - appropriate bit not used for packet acceptance mechanism
//				  1 - otherwise
//		Filter	: SID part of Filter are applied on SID part of frame,
//				  EID part of Filter are applied on Two first Data Bytes of frame.
//			 --------------------------------------------------------------------
//	Mask	|10					   0|17											0|
//			 --------------------------------------------------------------------
//			 --------------------------------------------------------------------
//	Filter	|SID10				SID0|EID17									 EID0|
//			 --------------------------------------------------------------------
//			 --------------------------------------------------------------------
//	Frame	|SID10				SID0|**|7	  Data Byte	   0|7	  Data Byte		0|
//			 --------------------------------------------------------------------
//
//	** - EID17 and EID16 mask and filter bits are not used.
//
//
//	2. Extended Data Frame:
//
//
//		Mask	: Both parts of Mask - Standard and Extended are applied on frame,
//				  0 - appropriate bit not used for packet acceptance mechanism
//				  1 - otherwise
//		Filter	: SID part of Filter are applied on SID part of frame,
//				  EID part of Filter are applied on EID part of frame.
//			 --------------------------------------------------------------------
//	Mask	|10					   0|17											0|
//			 --------------------------------------------------------------------
//			 --------------------------------------------------------------------
//	Filter	|SID10				SID0|EID17									 EID0|
//			 --------------------------------------------------------------------
//			 --------------------------------------------------------------------
//	Frame	|SID10				SID0|EID17									 EID0|
//			 --------------------------------------------------------------------
//
// ================================================================================================


// Rx Buffers ID ==================================================================================

typedef enum 		
{ 
	RXB0	=	0,					// Rx Buffer #0
	RXB1	=	1 
}RXBID;				// Rx Buffer #1

// Rx Buffers Mask ID =============================================================================

typedef enum 		
{ 
	MRXB0	=	0,					// Mask for Rx Buffer #0
	MRXB1	=	1 
}MRXBID;				// Mask for Rx Buffer #1

// Rx Buffers Filter ID ===========================================================================

typedef enum 
{ 
	FRXB0	=	0,					// Filter for Rx Buffer #0
	FRXB1	=	1,					// Filter for Rx Buffer #1
	FRXB2	=	2,					// Filter for Rx Buffer #2
	FRXB3	=	3,					// Filter for Rx Buffer #3
	FRXB4	=	4,					// Filter for Rx Buffer #4
	FRXB5	=	5 
}FRXBID;				// Filter for Rx Buffer #5

// MCP2515 CAN Bus Controller (user accessible) Modes of Operation ================================

typedef enum 
{
	CAN_OPMODE_NORMAL	= 0,	// Normal Operation mode.
	CAN_OPMODE_SLEEP    = 1, 
	CAN_OPMODE_LOOPBACK	= 2,	// Internal Loopback mode.
	CAN_OPMODE_LISTEN	= 3, 
	CAN_OPMODE_CONFIG	= 4		// Configuration mode.
}CAN_OPMODE;

// Rx Buffer Operation Mode =======================================================================

typedef enum 	
{ 
	RXB_OPMODE_ALL		= 0,	// Receive all valid messages using either
								// Standard or Extended Identifier that
								// meet filter criteria. This is Default.
	RXB_OPMODE_SID		= 1,	// Receive only valid messages with 
								// Standard Identifier that
								// meet filter criteria.
	RXB_OPMODE_EID		= 2,	// Receive only valid messages with 
								// Extended Identifier that
								// meet filter criteria.
	RXB_OPMODE_SNIFFER	= 3 
}RXBOPMODE;	// Turn Mask/Filters Off,
												// receive any messages.


// Tx Buffer Priority Internal to CAN Controller ==================================================

typedef enum  
{ 
	TXB_PRIORITY_LOWEST	= 0,	// Lowest Message Priority.This is Default.
	TXB_PRIORITY_LOW		= 1,	// Lowest-Intermediate Message Priority.
	TXB_PRIORITY_HIGH	= 2,	// High-Intermediate Message Priority.
	TXB_PRIORITY_HIGHEST	= 3 
}TXBPRIORITY;	// Highest Message Priority.


// Custom Structures ==============================================================================

// Used to set CAN Configuration: Bit Timing and Wake Up Filter
// MCP2515 Data Sheet explains using of this parameters
typedef struct
{
	unsigned __int32 PRSEG;			// Propagation Segment Length,
									// Usage: PRSEG = [ 0 -> 7 ]
	unsigned __int32 PHSEG1;		// Phase Segment #1 Length,
									// Usage: PHSEG1 = [ 0 -> 7 ]
	unsigned __int32 PHSEG2;		// Phase Segment #2 Length,
									// Usage: PHSEG2 = [ 0 -> 7 ]
	unsigned __int32 SAM;			// Sample Point Configuration,
									// Usage: SAM = [ 0 -> 1 ]
	unsigned __int32 BTLMODE;		// Phase Segment #2 Bit Time Length,
									// Usage: BTLMODE = [ 0 -> 1 ]
	unsigned __int32 BRP;			// Baud Rate Prescaler,
									// Usage: BRP = [ 0 -> 63 ]
	unsigned __int32 SJW;			// Synchronization Jump Width Length,
									// Usage: SJW = [ 0 -> 3 ]
	unsigned __int32 WAKFIL;		// Wake-Up Filter,
									// Usage: WAKFIL = [ 0 -> 1 ]
}CANCONFIG, *PCANCONFIG;

// ------------------------------------------------------------------------------------------------

// Represents Masks and Filters, for both Standard and Extended Identifiers
typedef struct
{
	BOOL	bUpdate;		// Used for both Mask and Filter to Set/Get operation:
							// When Set operation is performed:
							// bUpdate = TRUE to indecate that SID and EID should be set, 
							// driver will check ONLY range correctness of the SID and EID.
							// When Get operation is performed:
							// bUpdate = TRUE to indecate that SID and EID should be obtained.

	unsigned __int32 SID;	// Standard Identifier, 0 <= [ should be ] <= 0x7FF		( CAN Spec. )
	unsigned __int32 EID;	// Extended Identifier, 0 <= [ should be ] <= 0x3FFFF	( CAN Spec. )

	BOOL	bFISATO;		// Relevant ONLY when Filters Set/Get operation performed:
							// TRUE  - Filter IS Applied To extended frames Only
							// FALSE - Filter IS Applied To standard frames Only
							// When Set operation is performed, bFISATO will configure 
							// Filter to required functionality.
							// When Get operation is performed, bFISATO will obtain 
							// Filters functionality previously configured by Set operation.

}CANSEID, *PCANSEID;

// ------------------------------------------------------------------------------------------------

// Used to Set/Get Mask of specific Mask ID.
typedef struct
{
	CANSEID	Mask[ 2 ];		// One Mask for each Rx Buffer, total Two Rx Buffers RXB0 and RXB1
							// Index in array will represent required Mask - MRXB0/1
							// For RXB0
							// MRXB0
							// For RXB1
							// MRXB1
							// Usage: CANMASK.Mask[MRXB0] or CANMASK.Mask[MRXB1]

}CANMASK, *PCANMASK;

// ------------------------------------------------------------------------------------------------

// Used to Set/Get Filter of specific Filter ID.
typedef struct
{
	CANSEID	Filter[ 6 ];	// There are Six Filters for both Rx Buffers
							// Index in array will represent required Filter - FRXB0/1/2/3/4/5
							// For RXB0:
							// FRXB0
							// FRXB1
							// For RXB1:
							// FRXB2
							// FRXB3
							// FRXB4
							// FRXB5
							// Usage: CANFILTER.Filter[FRXB0] -> CANFILTER.Filter[FRXB5]

}CANFILTER, *PCANFILTER;

// ------------------------------------------------------------------------------------------------

// Used to Set/Get Time Outs for ReadFile/WriteFile calls
typedef struct
{
	DWORD dwReadTimeOut;	// Time Out for Reading in mSec.
	DWORD dwWriteTimeOut;	// Time Out for Writing in mSec.

}CANRWTIMEOUT, *PCANRWTIMEOUT;

// ------------------------------------------------------------------------------------------------

// Used to Set/Get Operation Mode of specific Rx Buffer
typedef struct
{
	RXBID		RxBufferID;	// Specifies Rx Buffer ID, RXB0 or RXB1

	RXBOPMODE	Mode;		// Rx Buffer Operation Mode:
							// RXB_OPMODE_ALL
							// RXB_OPMODE_SID
							// RXB_OPMODE_EID
							// RXB_OPMODE_SNIFFER

}CANRXBOPMODE, *PCANRXBOPMODE;

// ------------------------------------------------------------------------------------------------

// Used by ReadFile
typedef struct
{
	unsigned __int16	SID;		// Standard Identifier
									// keeps value of standard identifier

	unsigned __int32	EID;		// Extended Identifier, 
									// keeps valid value only if bIsEXT is TRUE
									
	BOOL				bIsRTR;		// Remote Transfer Request,
									// keeps TRUE if current packet is Remote Transfer Request,
									// otherwise FALSE

	BOOL				bIsEXT;		// Extended Packet,
									// keeps TRUE if current packet is Extended CAN Frame
									// otherwise FALSE

	unsigned __int8		DataLength; // Count of bytes received from CAN Bus and 
									// stored in the DATA buffer.

	unsigned __int8		DATA[ 8 ];	// 8 Data Bytes, 
									// DataLength indicates quantity of data bytes

	FRXBID				FilHitID;	// Filter Hit, 
									// ID of filter that accepted the current message

}CANRECEIVEPACKET, *PCANRECEIVEPACKET;

// ------------------------------------------------------------------------------------------------

// Used by WriteFile
typedef struct
{
	TXBPRIORITY			TxBP;		// Transmit Buffer Message Priority, internal to MCP2515.
									// TXB_PRIORITY_LOWEST
									// TXB_PRIORITY_LOW
									// TXB_PRIORITY_HIGH
									// TXB_PRIORITY_HIGHEST
									// When packet is placed to internal Tx buffer of MCP2515
									// priority could be assigned to packet, 
									// packet with higher priority will be send by MCP2515 first.

	unsigned __int16	SID;		// Standard Identifier

	unsigned __int32	EID;		// Extended Identifier, 
									// have meaning only when bIsEXT is TRUE

	BOOL				bIsRTR;		// Remote Transfer Request,
									// bIsRTR is TRUE when current packet should be send 
									// as Remote Transfer Request Packet.
									// otherwise FALSE

	BOOL				bIsEXT;		// Extended Packet,
									// bIsEXT is TRUE when current packet should be send
									// as Extended CAN Frame
									// otherwise FALSE

	unsigned __int8		DataLength; // Count of bytes to send, 
									// stored in DATA buffer

	unsigned __int8		DATA[ 8 ];	// 8 Data Bytes,
									// DataLength indicates quantity of bytes to send

}CANSENDPACKET, *PCANSENDPACKET;


// General configuration structure, used in functions 'MIC_CAN_getConfig' / 'MIC_CAN_setConfig'

typedef struct
{
  CANCONFIG     CANcfg;
  CANMASK       CANmask;
  CANFILTER     CANfilter;
  CANRWTIMEOUT  CANtimeouts;
  CANRXBOPMODE  CANrxb0opmode;
  CANRXBOPMODE  CANrxb1opmode;
   
}GENCANCONFIG, *PGENCANCONFIG;


//------------------------------------------------------------------------------
// Function name	: canPortGetConfig
// Description	    : This function gets current CAN configuration.
// Return type		: canOpenStatus
// Argument         : handle – valid handle from canPortOpen;
//			          pointer to GENCANCONFIG structure
//
// Notes            : This function when calling set CAN to Configuration mode and
//                    return to previous mode when exiting.
//                    So, no need to call canPortGoBusOff/canPortGoBusOn
//------------------------------------------------------------------------------

CANOPENLIB_HW_API canOpenStatus __stdcall  canPortGetConfig(canPortHandle handle, PGENCANCONFIG pGENCANCONFIG);


//------------------------------------------------------------------------------
// Function name	: canPortSetConfig
// Description	    : This function sets CAN configuration.
// Return type		: canOpenStatus
// Argument         : handle – valid handle from canPortOpen;
//			          pointer to GENCANCONFIG structure
//
// Note             : before calling this functions set CAN bus in configuration mode
//                    using 'canPortGoBusOff' function. 
//------------------------------------------------------------------------------

CANOPENLIB_HW_API canOpenStatus __stdcall  canPortSetConfig(canPortHandle handle, PGENCANCONFIG pGENCANCONFIG);



//------------------------------------------------------------------------------
// Function name	: canPortReset
// Description	    : This function resets CAN controller.
// Return type		: canOpenStatus
// Argument         : handle – valid handle from canPortOpen;
//------------------------------------------------------------------------------

CANOPENLIB_HW_API canOpenStatus __stdcall  canPortReset(canPortHandle handle);


// ------------------------------------------------------------------------------------------------


#endif //__CAN_API_H