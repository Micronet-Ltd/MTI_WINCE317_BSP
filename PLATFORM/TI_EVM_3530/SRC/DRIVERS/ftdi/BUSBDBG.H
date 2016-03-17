/*++

Copyright (c) 2000-2004  Future Technology Devices International Ltd.

Module Name:

    BusbDbg.h

Abstract:

    Native USB device driver for FTDI FT8U232
    Debug macros

Environment:

    kernel mode

Revision History:

    18/09/00    awm     Adapted from BULKUSB sample.
    05/04/02    awm     Added ioctls.
    14/01/03    awm     Added IOCTL_FT_GET_DEVICE_INFO.
    12/06/03    awm     Added IOCTL_FT_STOP_IN_TASK and
                        IOCTL_FT_RESTART_IN_TASK.
    23/01/04    awm     Added IOCTL_FT_GET_LOCATION_ID.

--*/


/*++

Copyright (c) 1997-1998  Microsoft Corporation

Module Name:

    BusbDbg.h

Abstract:

	Debug macros 

Environment:

    Kernel mode

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  Copyright (c) 1997-1998 Microsoft Corporation.  All Rights Reserved.

Revision History:

    11/18/97 : created

--*/

#ifndef BUSBDBG_INCD
#define BUSBDBG_INCD

#if DEBUG
#include <windows.h>

// Defines for debug verbosity level,
//   i.e., test FT_KdPrintLevel( n, format....) against these values
//  Example:
//
// FT_KdPrint( DBGLVL_MEDIUM ,("Entering DriverEntry(), RegistryPath=\n    %ws\n", RegistryPath->Buffer ));
// 
//  The above will print iff gDebugLevel >= DBGLVL_MEDIUM
//
#define DBGLVL_OFF				0		// if gDebugLevel set to this, there is NO debug output	
#define DBGLVL_MINIMUM			1		// minimum verbosity	
#define DBGLVL_DEFAULT			2		// default verbosity level if no registry override
#define DBGLVL_MEDIUM			3		// medium verbosity
#define DBGLVL_HIGH				4		// highest 'safe' level (without severely affecting timing )
#define DBGLVL_MAXIMUM			5		// maximum level, may be dangerous



#ifndef DBGSTR_PREFIX
#define DBGSTR_PREFIX "FTD2XX: " 
#endif


// registry path used for parameters global to all instances of the driver
#define FT_REGISTRY_PARAMETERS_PATH  \
    L"\\REGISTRY\\Machine\\System\\CurrentControlSet\\SERVICES\\FTD2XX\\Parameters"



#define DPRINT DbgPrint

#define TRAP() DbgBreakPoint();


#define FT_DBGOUTSIZE		512


typedef struct _FT_DBGDATA {

	// mirrors device extension pending io count
	ULONG PendingIoCount;

	// count of pipe errors detected during the life of this device instance
	ULONG PipeErrorCount;

	// count of pipe resets performed during the life of this device instance
	ULONG ResetPipeCount;

	// count of pipe resets performed during the life of this device instance
	ULONG AbortPipeCount;

} FT_DBGDATA, *PFT_DBGDATA;

//these declared in debug 'c' file
extern int gDebugLevel; 
extern int gExAllocCount;
extern PFT_DBGDATA gpDbg; 


static const PCHAR szIrpMajFuncDesc[] =
{  // note this depends on corresponding values to the indexes in wdm.h
   "IRP_MJ_CREATE",
   "IRP_MJ_CREATE_NAMED_PIPE",
   "IRP_MJ_CLOSE",
   "IRP_MJ_READ",
   "IRP_MJ_WRITE",
   "IRP_MJ_QUERY_INFORMATION",
   "IRP_MJ_SET_INFORMATION",
   "IRP_MJ_QUERY_EA",
   "IRP_MJ_SET_EA",
   "IRP_MJ_FLUSH_BUFFERS",
   "IRP_MJ_QUERY_VOLUME_INFORMATION",
   "IRP_MJ_SET_VOLUME_INFORMATION",
   "IRP_MJ_DIRECTORY_CONTROL",
   "IRP_MJ_FILE_SYSTEM_CONTROL",
   "IRP_MJ_DEVICE_CONTROL",
   "IRP_MJ_INTERNAL_DEVICE_CONTROL",
   "IRP_MJ_SHUTDOWN",
   "IRP_MJ_LOCK_CONTROL",
   "IRP_MJ_CLEANUP",
   "IRP_MJ_CREATE_MAILSLOT",
   "IRP_MJ_QUERY_SECURITY",
   "IRP_MJ_SET_SECURITY",
   "IRP_MJ_POWER",          
   "IRP_MJ_SYSTEM_CONTROL", 
   "IRP_MJ_DEVICE_CHANGE",  
   "IRP_MJ_QUERY_QUOTA",    
   "IRP_MJ_SET_QUOTA",      
   "IRP_MJ_PNP"            
};
//IRP_MJ_MAXIMUM_FUNCTION defined in wdm.h


static const PCHAR szPnpMnFuncDesc[] =
{	// note this depends on corresponding values to the indexes in wdm.h 

    "IRP_MN_START_DEVICE",
    "IRP_MN_QUERY_REMOVE_DEVICE",
    "IRP_MN_REMOVE_DEVICE",
    "IRP_MN_CANCEL_REMOVE_DEVICE",
    "IRP_MN_STOP_DEVICE",
    "IRP_MN_QUERY_STOP_DEVICE",
    "IRP_MN_CANCEL_STOP_DEVICE",
    "IRP_MN_QUERY_DEVICE_RELATIONS",
    "IRP_MN_QUERY_INTERFACE",
    "IRP_MN_QUERY_CAPABILITIES",
    "IRP_MN_QUERY_RESOURCES",
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
    "IRP_MN_QUERY_DEVICE_TEXT",
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
    "",
    "IRP_MN_READ_CONFIG",
    "IRP_MN_WRITE_CONFIG",
    "IRP_MN_EJECT",
    "IRP_MN_SET_LOCK",
    "IRP_MN_QUERY_ID",
    "IRP_MN_QUERY_PNP_DEVICE_STATE",
    "IRP_MN_QUERY_BUS_INFORMATION",
    "IRP_MN_DEVICE_USAGE_NOTIFICATION",
	"IRP_MN_SURPRISE_REMOVAL"
};

#define IRP_PNP_MN_FUNCMAX	IRP_MN_SURPRISE_REMOVAL



static const PCHAR szSystemPowerState[] = 
{
    "PowerSystemUnspecified",
    "PowerSystemWorking",
    "PowerSystemSleeping1",
    "PowerSystemSleeping2",
    "PowerSystemSleeping3",
    "PowerSystemHibernate",
    "PowerSystemShutdown",
    "PowerSystemMaximum"
};

static const PCHAR szDevicePowerState[] = 
{
    "PowerDeviceUnspecified",
    "PowerDeviceD0",
    "PowerDeviceD1",
    "PowerDeviceD2",
    "PowerDeviceD3",
    "PowerDeviceMaximum"
};


static const PCHAR szIoctlDesc[] =
{
    "IOCTL_FT_GET_CONFIG_DESCRIPTOR",
    "IOCTL_FT_RESET_DEVICE",
    "IOCTL_FT_RESET_PIPE",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "IOCTL_FT_READ_EE",
    "IOCTL_FT_WRITE_EE",
    "IOCTL_FT_ERASE_EE",
    "IOCTL_FT_SET_EVENT_NOTIFICATION",
    "IOCTL_FT_GET_EVENT_STATUS",
    "IOCTL_FT_GET_STATUS",
    "IOCTL_FT_SET_DIVISOR",
    "IOCTL_FT_GET_SERIAL_NUMBER",
    "IOCTL_FT_GET_DESCRIPTION",
    "IOCTL_FT_GET_LOCATION_ID",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "IOCTL_FT_SET_BAUD_RATE",
    "",
    "IOCTL_FT_SET_LINE_CONTROL",
    "IOCTL_FT_SET_BREAK_ON",
    "IOCTL_FT_SET_BREAK_OFF",
    "",
    "IOCTL_FT_SET_TIMEOUTS",
    "IOCTL_FT_GET_TIMEOUTS",
    "IOCTL_FT_SET_DTR",
    "IOCTL_FT_CLR_DTR",
    "",
    "IOCTL_FT_SET_RTS",
    "IOCTL_FT_CLR_RTS",
    "",
    "",
    "",
    "IOCTL_FT_SET_WAIT_MASK",
    "IOCTL_FT_WAIT_ON_MASK",
    "IOCTL_FT_PURGE",
    "",
    "",
    "IOCTL_FT_GET_CHARS",
    "IOCTL_FT_SET_CHARS",
    "",
    "IOCTL_FT_SET_FLOW_CONTROL",
    "IOCTL_FT_GET_MODEMSTATUS",
    "IOCTL_FT_GET_QUEUESTATUS",
    "IOCTL_FT_SET_LATENCY_TIMER",
    "IOCTL_FT_GET_LATENCY_TIMER",
    "IOCTL_FT_SET_BIT_MODE",
    "IOCTL_FT_GET_BIT_MODE",
    "IOCTL_FT_SET_USB_PARAMETERS",
    "IOCTL_FT_SET_EVENT_MASK",
    "IOCTL_FT_WAIT_EVENT",
    "IOCTL_FT_CLEAR_ERROR",
    "IOCTL_FT_SET_STATE",
    "IOCTL_FT_GET_STATE",
    "IOCTL_FT_GET_DEVICE_INFO",
    "IOCTL_FT_STOP_IN_TASK",
    "IOCTL_FT_RESTART_IN_TASK",
    "IOCTL_FT_SET_RESET_PIPE_RETRY_COUNT"
};





BOOLEAN
FT_GetRegistryDword(
    IN      PWCHAR    RegPath,
    IN      PWCHAR    ValueName,
    IN OUT  PULONG    Value
    );


#define FT_KdPrintCond( ilev, cond, _x_) \
	if( gDebugLevel && ( ilev <= gDebugLevel ) && ( cond )) { \
			DPRINT( DBGSTR_PREFIX ); \
			DPRINT _x_ ; \
	}



#define FT_KdPrint( ilev, _x_)  FT_KdPrintCond( ilev, TRUE, _x_ )


#define FT_TrapCond( ilev, cond ) if ( gDebugLevel && ( ilev <= gDebugLevel ) && (cond) ) TRAP()
#define FT_Trap( ilev )	  FT_TrapCond( ilev, TRUE )


#define FT_ASSERT( cond ) ASSERT( cond )

#define FT_StringForDevState( devState )  szDevicePowerState[ devState ] 

#define FT_StringForSysState( sysState )  szSystemPowerState[ sysState ] 

#define FT_StringForPnpMnFunc( mnfunc ) szPnpMnFuncDesc[ mnfunc ]

#define FT_StringForIrpMjFunc(  mjfunc ) szIrpMajFuncDesc[ mjfunc ]

#define FT_StringForIoctl( code ) szIoctlDesc[ code ]

#if WINCE
#ifdef __cplusplus
	extern "C" {
#endif

HLOCAL 
    FT_LocalAlloc(
        UINT uFlags,
        UINT uSize
        );


HLOCAL 
    FT_LocalFree(
		HLOCAL hMem
		);
#ifdef __cplusplus
	}
#endif	
#else
PVOID 
    FT_ExAllocatePool(
        IN POOL_TYPE PoolType,
        IN ULONG NumberOfBytes
        );


VOID 
    FT_ExFreePool(
        IN PVOID p
        );
#endif

#else // if not DBG

// dummy definitions that go away in the retail build

#define FT_KdPrintCond( ilev, cond, _x_) 
#define FT_KdPrint( ilev, _x_)  
#define FT_TrapCond( ilev, cond ) 
#define FT_Trap( ilev )
#define FT_ASSERT( cond )
#define FT_StringForDevState( devState )
#define FT_StringForSysState( sysState ) 
#define FT_StringForPnpMnFunc( mnfunc )
#define FT_StringForIrpMjFunc(  mjfunc ) 
#define FT_StringForIoctl( code )

#if WINCE
#define FT_LocalAlloc( flags, siz )  LocalAlloc( flags, siz )
#define FT_LocalFree( h )   LocalFree( h )
#else
#define FT_ExAllocatePool( typ, siz )  ExAllocatePool( typ, siz )
#define FT_ExFreePool( p )   ExFreePool( p )
#endif


#endif //DBG

#endif // included



