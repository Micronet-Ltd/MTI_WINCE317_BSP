#ifndef __CANB_H
#define __CANB_H

#include "CANBD.h"
#include "canErrors.h"
#include "CanOpenLib_HW.h"
#include "CANBDIOctl.h"
#include <pm.h>

// Definitions ====================================================================================

#define  CAN_DEVICE_COOKIE                     'canS'
#define       CAN_DRV_DBG      0

// Tx Buffers ID
enum TXBID { TXB0	=	0,		// Tx Buffer #0
			 TXB1	=	1,		// Tx Buffer #1
			 TXB2	=	2 };	// Tx Buffer #2


// Used to Set/Get Bit Timing configuration
typedef struct
{
	unsigned __int32 PRSEG;			// Propagation Segment Length
	unsigned __int32 PHSEG1;		// Phase Segment #1 Length
	unsigned __int32 PHSEG2;		// Phase Segment #2 Length
	unsigned __int32 SAM;			// Sample Point Configuration
	unsigned __int32 BTLMODE;		// Phase Segment #2 Bit Time Length
	unsigned __int32 BRP;			// Baud Rate Prescaler
	unsigned __int32 SJW;			// Synchronization Jump Width Length

	unsigned __int32 WAKFIL;		// Wake-Up Filter

	unsigned __int32 RTO;			// Read Time Out for ReadFile
	unsigned __int32 WTO;			// Write Time Out for ReadFile

	unsigned __int32 RXMB0;			// Receive Buffer #0 Operation Mode
	unsigned __int32 RXMB1;			// Receive Buffer #1 Operation Mode

	unsigned __int32 SIDMASK0;		// SID Mask #0
	unsigned __int32 EIDMASK0;		// EID Mask #0
	unsigned __int32 SIDMASK1;		// SID Mask #1
	unsigned __int32 EIDMASK1;		// EID Mask #1

	unsigned __int32 SIDFILTER0;	// Filter #0 for Standard ID
	unsigned __int32 FILSWITCH0;	// Filter applied only to 0 - Standard / 1 - Extended Frames
	unsigned __int32 EIDFILTER0;	// Filter #0 for Extended ID

	unsigned __int32 SIDFILTER1;	// Filter #1 for Standard ID
	unsigned __int32 FILSWITCH1;	// Filter applied only to 0 - Standard / 1 - Extended Frames
	unsigned __int32 EIDFILTER1;	// Filter #1 for Extended ID

	unsigned __int32 SIDFILTER2;	// Filter #2 for Standard ID
	unsigned __int32 FILSWITCH2;	// Filter applied only to 0 - Standard / 1 - Extended Frames
	unsigned __int32 EIDFILTER2;	// Filter #2 for Extended ID

	unsigned __int32 SIDFILTER3;	// Filter #3 for Standard ID
	unsigned __int32 FILSWITCH3;	// Filter applied only to 0 - Standard / 1 - Extended Frames
	unsigned __int32 EIDFILTER3;	// Filter #3 for Extended ID

	unsigned __int32 SIDFILTER4;	// Filter #4 for Standard ID
	unsigned __int32 FILSWITCH4;	// Filter applied only to 0 - Standard / 1 - Extended Frames
	unsigned __int32 EIDFILTER4;	// Filter #4 for Extended ID

	unsigned __int32 SIDFILTER5;	// Filter #5 for Standard ID
	unsigned __int32 FILSWITCH5;	// Filter applied only to 0 - Standard / 1 - Extended Frames
	unsigned __int32 EIDFILTER5;	// Filter #5 for Extended ID


}CANREGSETTINGS, *PCANREGSETTINGS;


typedef struct _CAN_INIT_CONTEXT
{
	UINT32              cookie;
    HANDLE              hSPI;
    HANDLE              hGpio;
	CRITICAL_SECTION    csSPIMCP2515;
	HANDLE	            hIntrServThread;
	HANDLE              hIntrEvent;
	DWORD               sysIntr;
	UINT                irqNum;
	HANDLE	            hEchoThread;
	HANDLE              hEchoEvent;

} CAN_INIT_CONTEXT, *PCAN_INIT_CONTEXT;


typedef struct __CAN_OPEN_CONTEXT {

    PCAN_INIT_CONTEXT pCANHead;                // @field Pointer back to our APD_INIT_CONTEXT

} CAN_OPEN_CONTEXT, *PCAN_OPEN_CONTEXT;


void rxqInit( void );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBTransmitMsg
// Description	    : Send Message via CAN Bus
// Return type		: DWORD CANBTransmitMsg 
// Argument         : HANDLE
// Argument         : PCANSENDPACKET

DWORD CANBTransmitMsg ( HANDLE, PCANSENDPACKET );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBReceiveMsg
// Description	    : Pulls One Message from Internal Message Queue
// Return type		: DWORD CANBReceiveMsg 
// Argument         : HANDLE
// Argument         : PCANRECEIVEPACKET

DWORD CANBReceiveMsg ( HANDLE, PCANRECEIVEPACKET );

// Function name	: CANBIntrServThread 
// Description	    : Interrupt Service Thread
// Return type		: DWORD WINAPI 
// Argument         : LPVOID

DWORD WINAPI CANBIntrServThread ( LPVOID );

// Function name	: CANB_DismissIST 
// Description	    : 

DWORD WINAPI CANBEchoThread ( LPVOID );


void CANB_DismissIST(CAN_INIT_CONTEXT *pCtx);

// Function name	: EnterDeadState 
// Description	    : 

void EnterDeadState();

// Function name	: CANBCreateEvents
// Description	    : Creates all required events
// Return type		: BOOL CANBCreateEvents 

BOOL CANBCreateEvents(CAN_INIT_CONTEXT *pCxt);

// Function name	: CANBCloseEvents
// Description	    : Closes all events
// Return type		: void CANBCloseEvents 

void CANBCloseEvents ( );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetOpMode
// Description	    : Sets MCP2515 Operation Mode
// Return type		: BOOL CANBSetOpMode 
// Argument         : CAN_OPMODE

BOOL CANBSetOpMode ( CAN_OPMODE );
//
// Function name	: CANBPowerOn
// Description	    : Generates awakening
// Return type		: void 
//

CAN_OPMODE  CANBGetOpMode(void);

//void CANBPowerOn(CEDEVICE_POWER_STATE devPrevPwrState);
void CANBPowerOn(CAN_INIT_CONTEXT *pCtx, CEDEVICE_POWER_STATE devPrevPwrState);
//
// Function name	: CANBPowerOff
// Description	    : Saves current MCP2515 Operation Mode and switches it into the Sleep Mode
// Return type		: void 
// Argument         : BOOL EnableAwakening
//
void CANBPowerOff(CAN_INIT_CONTEXT *pCtx, CEDEVICE_POWER_STATE devPwrState, BOOL EnableAwakening);

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetCNF
// Description	    : Set CAN Configuration, serves IO_Control calls
// Return type		: BOOL CANBSetCNF 
// Argument         : PCANCONFIG

BOOL CANBSetCNF ( PCANCONFIG );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetCNF
// Description	    : Get CAN Configuration, serves IO_Control calls
// Return type		: BOOL CANBGetCNF 
// Argument         : PCANCONFIG

BOOL CANBGetCNF ( PCANCONFIG );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetMask
// Description	    : Set Rx Buffer Mask
// Return type		: BOOL CANBSetMask 
// Argument         : PCANMASK

BOOL CANBSetMask ( PCANMASK );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetMask
// Description	    : Get Rx Buffer Mask
// Return type		: BOOL CANBGetMask 
// Argument         : PCANMASK

BOOL CANBGetMask ( PCANMASK );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetFilter
// Description	    : Set Filter of Rx Buffers
// Return type		: BOOL CANBSetFilter 
// Argument         : PCANFILTER

BOOL CANBSetFilter ( PCANFILTER );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetFilter
// Description	    : Get Filter of Rx Buffers
// Return type		: BOOL CANBGetFilter 
// Argument         : PCANFILTER

BOOL CANBGetFilter ( PCANFILTER );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetTimeOuts
// Description	    : Set Time Outs
// Return type		: BOOL CANBSetTimeOuts 
// Argument         : PCANRWTIMEOUTS

BOOL CANBSetTimeOuts ( PCANRWTIMEOUT );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetTimeOuts
// Description	    : Get Time Outs
// Return type		: BOOL CANBGetTimeOuts 
// Argument         : PCANRWTIMEOUTS

BOOL CANBGetTimeOuts ( PCANRWTIMEOUT );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBSetRXM
// Description	    : Set Receive Buffer Operating Mode
// Return type		: BOOL CANBSetRXM 
// Argument         : PCANRXBOPMODE

BOOL CANBSetRXM ( PCANRXBOPMODE );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetRXM
// Description	    : Get Receive Buffer Operating Mode
// Return type		: BOOL CANBGetRXM 
// Argument         : PCANRXBOPMODE

BOOL CANBGetRXM ( PCANRXBOPMODE );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBResetMCP2515
// Description	    : Reset the MCP2515 Device
// Return type		: BOOL CANBResetMCP2515 

BOOL CANBResetMCP2515 ( );

// ------------------------------------------------------------------------------------------------

// Function name	: CANBGetErrors
// Description	    : Get Errors
// Return type		: BOOL CANBGetErrors 
// Argument         : DWORD *

BOOL CANBGetErrors ( DWORD * );

// ------------------------------------------------------------------------------------------------

extern volatile int InPowerHandler;
extern volatile HANDLE	g_EchoHandle;

#endif //__CANB_H