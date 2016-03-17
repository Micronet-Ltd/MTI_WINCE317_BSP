/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module tests the Datalight Loader I/O interface.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltreaderioapi.c $
    Revision 1.5  2009/05/21 18:37:12Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.4  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/10/31 00:53:38Z  jeremys
    Added the sequential I/O test for the I/O layer.
    Revision 1.2  2007/10/06 01:58:22Z  brandont
    Renamed DlLoaderTestIO to DclLoaderTestIO.  Added include for
    dltloader.h.  Removed the call to destroy the IO instance.
    Revision 1.1  2007/10/05 01:49:26Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlloader.h>
#include <dlreaderio.h>
#include <dltloader.h>
#include "dltloaderutil.h"


/*-------------------------------------------------------------------
    Protected: DclLoaderTestIO()

    This function tests the specified Datalight Loader IO layer,
    ensuring that it properly implements the IO interface.

    Parameters:
        psIoInstance    - The IO instance.

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
DCLSTATUS DclLoaderTestIOSequential(
    DCLREADERIO    *psIoInstance,
    D_UINT32        ulSeed)
{
    DCLSTATUS       DclStatus;
    D_UINT32        ulBlockSize;
    D_UINT32        ulNumBlocks;
    D_UINT32        ulBlockIndex;
    D_UCHAR        *pucBuffer = NULL;

    DclPrintf("DclLoaderTestIOSequential: Loader I/O sequential test\n");

    /*  Get the drive parameters.
    */
    DclStatus = psIoInstance->fnIoGetParameters(psIoInstance, &ulBlockSize, &ulNumBlocks);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }

    /*  Read the bounding sectors of the block device.
    */
    pucBuffer = DclMemAlloc(ulBlockSize);
    if(!pucBuffer)
    {
        goto Failure;
    }

    for(ulBlockIndex = 0;  ulBlockIndex < ulNumBlocks;  ulBlockIndex++)
    {
        DclStatus = psIoInstance->fnIoRead(psIoInstance, ulBlockIndex, 1, pucBuffer);
        if(DclStatus != DCLSTAT_SUCCESS)
        {
            goto Failure;
        }

        DclStatus = DlLoaderTestCheckSeed(
                        pucBuffer,
                        ulBlockSize,
                        ulBlockIndex * ulBlockSize,
                        ulSeed);
        if(DclStatus != DCLSTAT_SUCCESS)
        {
            goto Failure;
        }
    }

    DclStatus = DCLSTAT_SUCCESS;

Failure:

    if(pucBuffer)
    {
        DclMemFree(pucBuffer);
    }

    if(DclStatus == DCLSTAT_SUCCESS)
    {
        DclPrintf("DclLoaderTestIOSequential: Passed\n");
    }
    else
    {
        DclPrintf("DclLoaderTestIOSequential: FAILED\n");
    }

    return DclStatus;
}


/*-------------------------------------------------------------------
    Protected: DclLoaderTestIO()

    This function tests the specified Datalight Loader IO layer,
    ensuring that it properly implements the IO interface.

    Parameters:
        psIoInstance    - The IO instance.

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
DCLSTATUS DclLoaderTestIO(
    DCLREADERIO    *psIoInstance)
{
    DCLSTATUS       DclStatus;
    D_UINT32        ulBlockSize;
    D_UINT32        ulNumBlocks;
    D_UCHAR        *pucBuffer = NULL;

    DclAssert(psIoInstance);
    DclAssert(psIoInstance->fnIoDestroy);
    DclAssert(psIoInstance->fnIoGetParameters);
    DclAssert(psIoInstance->fnIoRead);

    DclPrintf("DclLoaderTestIO: Loader I/O interface test\n");

    /*  Get the drive parameters.
    */
    DclStatus = psIoInstance->fnIoGetParameters(psIoInstance, &ulBlockSize, &ulNumBlocks);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }

    /*  Read the bounding sectors of the block device.
    */
    pucBuffer = DclMemAlloc(ulBlockSize);
    if(!pucBuffer)
    {
        goto Failure;
    }

    DclStatus = psIoInstance->fnIoRead(psIoInstance, 0, 1, pucBuffer);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }

    DclStatus = psIoInstance->fnIoRead(psIoInstance, ulNumBlocks-1, 1, pucBuffer);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }

    /*  Return the result of the test.
    */
  Failure:

    if(pucBuffer)
    {
        DclMemFree(pucBuffer);
    }

    if(DclStatus == DCLSTAT_SUCCESS)
    {
        DclPrintf("DclLoaderTestIO: Passed\n");
    }
    else
    {
        DclPrintf("DclLoaderTestIO: FAILED\n");
    }

    return DclStatus;
}


