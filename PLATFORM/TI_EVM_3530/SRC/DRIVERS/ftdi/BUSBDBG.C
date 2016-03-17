/*++

Copyright (c) 2000  Future Technology Devices International Ltd.

Module Name:

    BusbDbg.c

Abstract:

    Native USB device driver for FTDI FT8U245
    Debug output logic .
	This entire module is a noop in the free build

Environment:

    kernel mode only

Revision History:

    18/09/00    awm     Adapted from BULKUSB sample.

--*/


/*++

Copyright (c) 1997-1998  Microsoft Corporation

Module Name:

    BusbDbg.c 

Abstract:

    Debug output logic .
	This entire module is a noop in the free build

Environment:

    kernel mode only

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  Copyright (c) 1997-1998 Microsoft Corporation.  All Rights Reserved.


Revision History:

    1/27/98: created

--*/


#if DEBUG
//
// Header file defines the proper LocalAlloc if its a DEBUG build
//

#ifndef WINCE
//#include "precomp.h"
#else
#include "BUSBDBG.H"
#include "ftdi_ioctl.h"
#include "ftdi_debug.h"
//#undef ZONE_INIT
#include <serdbg.h>
#include <celog.h> 

#endif

// begin, data/code  used only in DBG build

//  may be overridden  in registry in DBG buils only
// higher == more verbose, default is 1, 0 turns off all
// count outstanding allocations via ExAllocatePool

static int gLocalAllocCount = 0;

FT_DBGDATA gDbgBuf = { 0, 0, 0, 0 }; 

// ptr to global debug data struct; txt buffer is only allocated in DBG builds
PFT_DBGDATA gpDbg = &gDbgBuf; 

HLOCAL 
FT_LocalAlloc(
        UINT uFlags,
        UINT uSize
        )
{
	gLocalAllocCount++;
	DEBUGMSG2(ZONE_FUNCTION, TEXT("FT_LocalAlloc() gLocalAllocCount = inc %d\n"), gLocalAllocCount);
	return LocalAlloc(uFlags, uSize);
}


HLOCAL 
FT_LocalFree(
        HLOCAL h
        )
{
	gLocalAllocCount--;
	DEBUGMSG2(ZONE_FUNCTION, TEXT("FT_LocalFree() gLocalAllocCount = dec %d\n"), gLocalAllocCount);
	return LocalFree(h);
}

#endif // end , if DBG


