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
//  File:  format.c
//
//  This file implements the IOCTL_HAL_QUERY_FORMAT_PARTITION handler.
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalQueryFormatPartition
//
BOOL OALIoCtlHalQueryFormatPartition(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = 0;
    UINT32 *pQueryFormat;

    if(pOutSize)
		*pOutSize = sizeof(UINT32);

    // Check buffer size
    if(!pOutBuffer || outSize < sizeof(UINT32)) 
	{
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(1, (L"WARN: OALIoCtlHalQueryFormatPartition: Buffer too small\r\n"));
		goto cleanUp;
	}

    *(BOOL*)pOutBuffer = 0;
    pQueryFormat = OALArgsQuery(OAL_ARGS_QUERY_FORMATPART);
    if(pQueryFormat && *pQueryFormat)
	{
        RETAILMSG(1, (L"*** Format TFAT ***\r\n"));
        *(BOOL*)pOutBuffer = 1;

        // Clear the flag for next boot time(warm-boot) unless it is set again.
        *pQueryFormat = 0;
	}

    rc = 1;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
