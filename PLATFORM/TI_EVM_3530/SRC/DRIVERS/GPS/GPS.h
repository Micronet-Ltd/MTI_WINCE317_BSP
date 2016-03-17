// Copyright 2009 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//   File:  GPSDrvr.h
//
//   Automotive Input/Output Driver header
//  
//
//   Created by Anna Rayer  September 2009
//-----------------------------------------------------------------------------/*

#ifndef __GPS_H
#define __GPS_H

#include <pm.h>

#ifdef __cplusplus
extern "C" {
#endif



//==============================================================================================
//            local definitions
//==============================================================================================


//#define GPS_PORT_NAME                         L"COM7:" //SDK header
//#define GPS_VPORT							L"COV1:"


//==============================================================================================
//            IOControls definitions
//==============================================================================================

typedef struct __COMM_EVENTS    
{
    HANDLE  hCommEvent;         // @field Indicates serial events from PDD to MDD
    ULONG   fEventMask;         // @field Event Mask requestd by application 
    ULONG   fEventData;         // @field Event Mask Flag. 
    ULONG   fAbort;             // @field TRUE after SetCommMask( 0 )
    CRITICAL_SECTION EventCS;   //  CommEvent access and related sign atom
} COMM_EVENTS, *PCOMM_EVENTS;

typedef struct __CV_OPEN_INFO CV_OPEN_INFO, *PCV_OPEN_INFO;


//==============================================================================================
//            structure definitions
//==============================================================================================

typedef struct _GPS_INIT_CONTEXT
{
	UINT32               cookie;
	HANDLE               hGpio;
	DWORD                priority256;
	DWORD                powerMask;
    CEDEVICE_POWER_STATE powerState;
	DWORD                dwGPSVersion;
	DWORD                StartupOn;
	DWORD				 fOn;	
	DWORD				 fEnableExternalGPS;
	TCHAR				 szPortName[16];
} GPS_INIT_CONTEXT, *PGPS_INIT_CONTEXT;

// @struct  CV_INDEP_INFO | Hardware Independent Serial Driver Head Information.
typedef struct __CV_INDEP_INFO {
	CRITICAL_SECTION	TransmitCritSec1;       // @field Protects tx action
    CRITICAL_SECTION	ReceiveCritSec1;        // @field Protects rx action
    CRITICAL_SECTION	StatCritSec;        // @field Protects state action

    HANDLE          hSerialEventOff;   // @field Serial event powered off
    HANDLE          hSerialEventAct;     // @field Serial event usb is ready
    HANDLE          hInactEvent;     // @field Serial event usb is not ready
	HANDLE			hReadEvent;
	HANDLE          hTransmitEvent; // @field transmit event, both rx and tx
    DCB             DCB;            // @field DCB (see Win32 Documentation.
    COMMTIMEOUTS    CommTimeouts;   // @field Time control field. 
    DWORD           OpenCnt;        // @field Protects use of this port 
    DWORD           KillRxThread:1; // @field Flag to terminate SerialDispatch thread.
    DWORD           XFlow:1;        // @field True if Xon/Xoff flow ctrl.
    DWORD           StopXmit:1;     // @field Stop transmission flag.
    DWORD           SentXoff:1;     // @field True if XOFF sent.
    DWORD           DtrFlow:1;      // @field True if currently DTRFlowed   
    DWORD           RtsFlow:1;      // @field True if currently RTSFlowed
    DWORD           fAbortRead:1;   // @field Used for PURGE
    DWORD           fAbortTransmit:1;// @field Used for PURGE
    DWORD           Reserved:24;    // @field remaining bits.
    ULONG           fEventMask;     // @field Sum of event mask for all opens

	DWORD			SetResetDTR;
	DWORD			SetResetRTS;

    DWORD           fOpened;    //
	LIST_ENTRY      OpenList;       // @field Head of linked list of OPEN_INFOs    
    CRITICAL_SECTION OpenCS;        // @field Protects Open Linked List + ref counts

    PCV_OPEN_INFO   pAccessOwner;   // @field Points to whichever open has acess permissions
	DWORD				DevState;
	GPS_INIT_CONTEXT*	pGpsContext;

} CV_INDEP_INFO, *PCV_INDEP_INFO;

typedef struct __CV_OPEN_INFO 
{
    PCV_INDEP_INFO  pSerialHead;    // @field Pointer back to our CV_INDEP_INFO
    DWORD           AccessCode;     // @field What permissions was this opened with
    DWORD           ShareMode;      // @field What Share Mode was this opened with
	LONG			WaitUsers;
	COMM_EVENTS     CommEvents;     // @field Contains all info for serial event handling
	
	HANDLE			hPort;
    LIST_ENTRY      llist;          // @field Linked list of OPEN_INFOs
} 
CV_OPEN_INFO, *PCV_OPEN_INFO;

typedef enum
{
	OFF_ST		= 0,
	NORMAL_ST,
	NOTREADY_ST,
	REOPEN_ST
} DEV_PSTATE;

typedef enum
{
	NO_ACT = 0,
	DEV_OFF,
	DEV_ON,
	PWR_OFF,
	PWR_ON,
	NORMAL_ON
};


#define E_OF_CHAR           0xd
#define ERROR_CHAR          0xd
#define BREAK_CHAR          0xd
#define EVENT_CHAR          0xd
#define X_ON_CHAR           0x11
#define X_OFF_CHAR          0x13

#define X_FLOW_CTRL         0x1

#define READ_TIMEOUT				250
#define READ_TIMEOUT_MULTIPLIER     10
#define READ_TIMEOUT_CONSTANT       100
#define WRITE_TIMEOUT_MULTIPLIER    10
#define WRITE_TIMEOUT_CONSTANT      100

#define RX_BUFFER_SIZE		32*1024

 #ifdef __cplusplus
}
#endif

#endif
