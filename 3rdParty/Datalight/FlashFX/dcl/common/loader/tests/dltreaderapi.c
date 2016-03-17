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

    This module tests the Datalight Loader at the file system reader level.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltreaderapi.c $
    Revision 1.8  2009/05/21 18:37:11Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.7  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.6  2007/10/30 00:10:17Z  jeremys
    Fixed a bug that caused a memory access violation.
    Revision 1.5  2007/10/29 19:54:34Z  jeremys
    Changed this module so that it takes additional parameters.
    Revision 1.4  2007/10/06 03:01:30Z  brandont
    Changed the file name the test uses to file1.txt.  Changed the
    expected behavior of when requesting file state after a previous
    EOF state but when reading zero bytes.
    Revision 1.3  2007/10/05 23:29:40Z  brandont
    Added include for dltloader.h.  Renamed DlLoaderTestReader to
    DclLoaderTestReader.  Updated calling arguments for RdrTstDiskOpen.
    Removed fIsReliance argument from DclLoaderTestReader.
    Revision 1.2  2007/10/05 19:54:59Z  brandont
    Added disabled conditional compile for testing reader support with
    multiple open files.
    Revision 1.1  2007/10/05 03:10:42Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlloader.h>
#include <dlreader.h>
#include <dlreaderio.h>
#include <dlfatread.h>
#include <dlrelread.h>
#include <dltloader.h>

#include "dltreaderutil.h"


/*-------------------------------------------------------------------
    Local: TestReaderAPI()

    Open a single file, read its contents and perform a bounds check,
    then close it.

    Parameters:
        psIoInstance    - The IO instance.
        fIsReliance     - TRUE if the media is formatted with Reliance,
                          FALSE if the media is formatted with FAT

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
static DCLSTATUS TestReaderAPI(
    DCLREADERINSTANCE   psReaderInstance,
    D_BOOL              fIsReliance,
    char               *szFileName,
    D_UINT32            ulFileSize,
    D_UINT32            ulBufferSize)
 {
    D_UINT16            uState = 0;
    D_UINT32            ulReadBytes;
    DCLREADERFILEHANDLE hFile = NULL;
    DCLSTATUS           DclStatus = DCLSTAT_FAILURE;
    D_BUFFER           *pBuffer = NULL;
    D_UINT32            ulReadIndex;

    /*  Allocate an I/O buffer.
    */
    pBuffer = DclMemAlloc(ulBufferSize);
    if(!pBuffer)
    {
        goto Failure;
    }

    /*  Open the file.
    */
    hFile = RdrTstFileOpen(fIsReliance, psReaderInstance, szFileName);
    if(!hFile)
    {
        goto Failure;
    }
    uState = RdrTstFileState(fIsReliance, hFile);
    if(uState != DL_LOADER_FILESTATE_SUCCESS)
    {
        goto Failure;
    }

    /*  Attempt to read zero bytes at the beginning of the file.
    */
    ulReadBytes = RdrTstFileRead(fIsReliance, hFile, pBuffer, 0);
    uState = RdrTstFileState(fIsReliance, hFile);
    if((uState != DL_LOADER_FILESTATE_SUCCESS) || (ulReadBytes != 0))
    {
        goto Failure;
    }

    /*  Read it's contents.
    */
    for(ulReadIndex = 0; ulReadIndex < ulFileSize/ulBufferSize; ulReadIndex++)
    {
        ulReadBytes = RdrTstFileRead(fIsReliance, hFile, pBuffer, ulBufferSize);
        uState = RdrTstFileState(fIsReliance, hFile);
        if((uState != DL_LOADER_FILESTATE_SUCCESS) || (ulReadBytes != ulBufferSize))
        {
            goto Failure;
        }
    }
    if(ulFileSize % ulBufferSize)
    {
        ulReadBytes = RdrTstFileRead(fIsReliance, hFile, pBuffer, ulFileSize % ulBufferSize);
        uState = RdrTstFileState(fIsReliance, hFile);
        if((uState != DL_LOADER_FILESTATE_SUCCESS) || (ulReadBytes != ulFileSize % ulBufferSize))
        {
            goto Failure;
        }
    }

    /*  Attempt to read zero bytes at the end of the file.
    */
    ulReadBytes = RdrTstFileRead(fIsReliance, hFile, pBuffer, 0);
    uState = RdrTstFileState(fIsReliance, hFile);
    if((uState != DL_LOADER_FILESTATE_SUCCESS) || (ulReadBytes != 0))
    {
        goto Failure;
    }

    /*  Attempt to read beyond the end of the file.
    */
    ulReadBytes = RdrTstFileRead(fIsReliance, hFile, pBuffer, 1);
    uState = RdrTstFileState(fIsReliance, hFile);
    if((uState != DL_LOADER_FILESTATE_EOF) || (ulReadBytes != 0))
    {
        goto Failure;
    }

    /*  Attempt to read zero bytes at the end of the file.
    */
    ulReadBytes = RdrTstFileRead(fIsReliance, hFile, pBuffer, 0);
    uState = RdrTstFileState(fIsReliance, hFile);
    if((uState != DL_LOADER_FILESTATE_SUCCESS) || (ulReadBytes != 0))
    {
        goto Failure;
    }

    DclStatus = DCLSTAT_SUCCESS;

Failure:

    if(hFile)
    {
        RdrTstFileClose(fIsReliance, hFile);
    }
    if(pBuffer)
    {
        DclMemFree(pBuffer);
    }

    return DclStatus;
}


/*-------------------------------------------------------------------
    Protected: DclLoaderTestReader()

    This function tests the either the Reliance or FAT file system
    reader using the IO instance provided, ensuring that the file
    system reader functions properly.

    Parameters:
        psIoInstance    - The IO instance.
        fIsReliance     - TRUE if the media is formatted with Reliance,
                          FALSE if the media is formatted with FAT

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
DCLSTATUS DclLoaderTestReader(
    DCLREADERIO            *psIoInstance,
    LOADERTESTPARAMS      **psTests,
    D_UINT32                ulTestCount)
{
    DCLSTATUS               DclStatus = DCLSTAT_FAILURE;
    DCLREADERINSTANCE       psReaderInstance = NULL;
    D_BOOL                  fIsReliance = FALSE;
    D_UINT32                ulTestIndex;

    DclAssert(psIoInstance);
    DclAssert(psIoInstance->fnIoDestroy);
    DclAssert(psIoInstance->fnIoGetParameters);
    DclAssert(psIoInstance->fnIoRead);

    /*  Sign on
    */
    DclPrintf("DclLoaderTestReader: Reader interface test\n");


    /*  Create a reader instance for the specified file system.
    */
    DclStatus = RdrTstDiskOpen(psIoInstance, &fIsReliance, &psReaderInstance);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }


    /*  Iterate through the test cases.
    */
    for(ulTestIndex = 0;  ulTestIndex < ulTestCount;  ulTestIndex++)
    {
        D_UINT32    ulReadCount;

        /*  Each test case can specify mutiple reads of the same file.  Loop
            through each read.
        */
        for(ulReadCount = 0;  ulReadCount < psTests[ulTestIndex]->ulFileReadCount; ulReadCount++)
        {
            /*  Do a basic API test.
            */
            DclStatus = TestReaderAPI(
                            psReaderInstance,
                            fIsReliance,
                            psTests[ulTestIndex]->szFileName,
                            psTests[ulTestIndex]->ulFileSize,
                            psTests[ulTestIndex]->ulBufferSize);
            if(DclStatus != DCLSTAT_SUCCESS)
            {
                goto Failure;
            }
        }
    }


Failure:

    if(psReaderInstance != NULL)
    {
        RdrTstDiskClose(fIsReliance, psReaderInstance);
    }

    if(DclStatus == DCLSTAT_SUCCESS)
    {
        DclPrintf("DclLoaderTestReader: Passed\n");
    }
    else
    {
        DclPrintf("DclLoaderTestReader: FAILED\n");
    }

    return DclStatus;
}
