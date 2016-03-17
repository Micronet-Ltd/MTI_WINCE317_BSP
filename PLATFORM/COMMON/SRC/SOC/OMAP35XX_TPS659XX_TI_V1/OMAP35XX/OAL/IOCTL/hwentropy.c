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
//  File:  hwentropy.c
//
//  This file implements the IOCTL_HAL_GET_HWENTROPY handler.
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHWEntropy
//
//  Implements the IOCTL_HAL_GET_HWENTROPY handler. This function creates a
//  64-bit value which is unique to the hardware.  This value never changes.
//
BOOL
OALIoCtlHalGetHWEntropy( 
    UINT32 code, 
    VOID *pInpBuffer, 
    UINT32 inpSize, 
    VOID *pOutBuffer,
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL    rc = FALSE;
    UINT32  size = 2*sizeof(DWORD);
    UINT32  *pID = NULL;

	IOCTL_HAL_GET_DIEID_OUT DieID = {0};

    // Check buffer size
    if (pOutSize != NULL)
		*pOutSize = size;

    if (pOutBuffer == NULL || ((UINT32)pOutBuffer & 3) || outSize < size)
        {
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: "
            L"Buffer too small or unaligned\r\n"
            ));
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto cleanUp;
    }

	rc = OALIoCtlHalGetDieID(code, 0, 0, &DieID, sizeof(DieID), 0);
	if(rc)
	{
		*(UINT32 *)pOutBuffer = DieID.dwDieID_0;
		*((UINT32 *)pOutBuffer + 1) = DieID.dwDieID_0 ^ 0xFFFFFFFF;
	}
    // Get a 64 bit (2 DWORDs) HW generated random number that never changes
    //pID = OALArgsQuery(OAL_ARGS_QUERY_HWENTROPY);
    //if( pID )
    //{
        // Copy pattern to output buffer
    //    memcpy(pOutBuffer, pID, size);

        // We are done
    //    rc = TRUE;
    //}

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));

    return rc;
}

//------------------------------------------------------------------------------

