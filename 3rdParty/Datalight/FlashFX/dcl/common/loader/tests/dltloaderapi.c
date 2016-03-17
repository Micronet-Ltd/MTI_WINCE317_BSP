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

    This module tests the Datalight Loader at loader level.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltloaderapi.c $
    Revision 1.11  2009/05/29 20:18:16Z  johnbr
    Added parameter descriptions to DclLoaderTestLoader()
    Revision 1.10  2009/05/21 18:37:11Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.9  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.8  2007/10/31 22:06:39Z  jeremys
    Added a multiple open file test.
    Revision 1.7  2007/10/31 00:50:05Z  jeremys
    Moved the seed checking code into a seperate function so that
    it can be used in the sequential I/O test for the I/O layer.
    Revision 1.6  2007/10/30 01:23:47Z  jeremys
    Removed some #if'd out code.  Fixed an overload issue in the
    seed checking code due to an unsigned type.
    Revision 1.5  2007/10/30 00:16:23Z  jeremys
    Fixed a bug that caused a memory access violation.
    Revision 1.4  2007/10/29 21:04:25Z  jeremys
    Added additional parameters and an additional sequential read and
    verify test to this module.
    Revision 1.3  2007/10/06 03:03:00Z  brandont
    Changed the file name the test uses to file1.txt.
    Revision 1.2  2007/10/05 23:29:40Z  brandont
    Added include for dltloader.h.  Renamed DlLoaderTestLoader to
    DclLoaderTestLoader.
    Revision 1.1  2007/10/05 03:07:02Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlloader.h>
#include <dltloader.h>
#include "dltloaderutil.h"


/*-------------------------------------------------------------------
    Protected: DlLoaderTestCheckSeed()

    ToDo:

    Parameters:
        ToDo:

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
DCLSTATUS DlLoaderTestCheckSeed(
    D_BUFFER   *pBuffer,
    D_UINT32    ulBufferSize,
    D_UINT32    ulBufferOffset,
    D_UINT32    ulSerializationSeed)
{
    D_UINT32    ulByteOffset;
    DCLSTATUS   DclStatus = DCLSTAT_FAILURE;

    /*  Check the buffer one byte at a time.  This is necessary because
        neither ulBufferSize, nor ulFileSize need be a multiple of
        sizeof(D_UINT32).  Data is presumed to be stored in little endian
        on disk.
    */
    for(ulByteOffset = 0; ulByteOffset < ulBufferSize; ulByteOffset++)
    {
        D_UINT32    ulValueIndex;
        D_UINT32    ulValueOffset;
        D_UINT32    ulSeed;
        D_UCHAR     ucExpectedByte;

        ulValueIndex = (ulBufferOffset + ulByteOffset) / sizeof(D_UINT32);
        ulValueOffset = (ulBufferOffset + ulByteOffset) % sizeof(D_UINT32);

        ulSeed = ulValueIndex + ulSerializationSeed;

        ucExpectedByte = (D_CHAR)(ulSeed >> (ulValueOffset * 8));

        if(*(pBuffer + ulByteOffset) != ucExpectedByte)
        {
            goto Failure;
        }

    }

    DclStatus = DCLSTAT_SUCCESS;

Failure:

    return DclStatus;
}


/*-------------------------------------------------------------------
    Local: DlLoaderTestLoaderSequentialIO()

    ToDo:

    Parameters:
        hLoader             - Handle to a loader instance.
        szFileName          - Name of the file to test.
        ulFileSize          - Size of the file specified by szFileName.
        ulBufferSize        - Number of bytes to read at a time.
        ulSerializationSeed - Starting number for the serialized file data.

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
static DCLSTATUS DlLoaderTestLoaderSequentialIO(
    DCLLOADERHANDLE     hLoader,
    char               *szFileName,
    D_UINT32            ulFileSize,
    D_UINT32            ulBufferSize,
    D_UINT32            ulSerializationSeed)
{
    DCLSTATUS           DclStatus = DCLSTAT_FAILURE;
    D_UINT32            ulFileIndex;
    D_BUFFER           *pBuffer = NULL;
    DCLLOADERFILEHANDLE hFile = NULL;
    D_UINT32            ulBytesRead;
    D_UINT32            ulFileState;


    /*  Allocate the read buffer.
    */
    pBuffer = DclMemAlloc(ulBufferSize);
    if(!pBuffer)
    {
        goto Failure;
    }


    /*  Open the file.
    */
    hFile = DclLoaderFileOpen(hLoader, szFileName);
    if(!hFile)
    {
        goto Failure;
    }


    /*  Read the file one (ulBufferSize) chunk at a time and check the data.
    */
    for(ulFileIndex = 0;
        ulFileIndex < (ulFileSize + ulBufferSize - 1) / ulBufferSize;
        ulFileIndex ++)
    {
        D_UINT32    ulThisReadSize;


        /*  ulFileSize does not have to be a multiple of ulBufferSize, so we
            will have to handle the case where we're reading the last chunk of
            the file, and it is smaller than ulBufferSize.
        */
        ulThisReadSize = ulBufferSize;
        if(ulFileIndex == ulFileSize / ulBufferSize)
        {
            ulThisReadSize = ulFileSize % ulBufferSize;
        }


        /*  Read one chunk.  Make sure the expected number of bytes were read
            and that no errors occured.
        */
        ulBytesRead = DclLoaderFileRead(hLoader, hFile, ulThisReadSize, pBuffer);
        ulFileState = DclLoaderFileState(hLoader, hFile);
        if(ulBytesRead != ulThisReadSize ||
           ulFileState != DL_LOADER_FILESTATE_SUCCESS)
        {
            goto Failure;
        }


        /*  Check the buffer.
        */
        DclStatus = DlLoaderTestCheckSeed(
                        pBuffer,
                        ulThisReadSize,
                        ulFileIndex * ulBufferSize,
                        ulSerializationSeed);
        if(DclStatus != DCLSTAT_SUCCESS)
        {
            goto Failure;
        }
    }


    /*  All of the file's contents have been read and verified.  Now make sure
        that the end of the file was reached.
    */
    ulBytesRead = DclLoaderFileRead(hLoader, hFile, 1, pBuffer);
    ulFileState = DclLoaderFileState(hLoader, hFile);
    if(ulBytesRead != 0 || ulFileState != DL_LOADER_FILESTATE_EOF)
    {
        goto Failure;
    }

    /*  Test passed.
    */
    DclStatus = DCLSTAT_SUCCESS;

Failure:

    if(pBuffer)
    {
        DclMemFree(pBuffer);
    }
    if(hFile)
    {
        DclLoaderFileClose(hLoader, hFile);
    }

    return DclStatus;
}


/*-------------------------------------------------------------------
    Local: DlLoaderTestLoaderAPI()

    Test specific behaviors of the Loader API.

    Parameters:
        hLoader             - Handle to a loader instance.
        szFileName          - Name of the first file to test.
        szFileName2         - Name of the second file to test.
        ulFileSize          - Size of the file specified by szFileName.
        ulFileSize2         - Size of the file specified by szFileName2.

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
static DCLSTATUS DlLoaderTestLoaderAPI(
    DCLLOADERHANDLE         hLoader,
    LOADERTESTPARAMS      **psTests,
    D_UINT32                ulTestCount)
{
    DCLSTATUS               DclStatus = DCLSTAT_SUCCESS;

    /*  Test opening multiple files
    */
    if(ulTestCount > 1)
    {
        char * szFileName1 = NULL;
        char * szFileName2 = NULL;
        D_UINT32    ulTestIndex;

        szFileName1 = psTests[0]->szFileName;
        for(ulTestIndex = 1; ulTestIndex < ulTestCount; ulTestIndex++)
        {
            if(DclStrCmp(szFileName1, psTests[ulTestIndex]->szFileName) != 0)
            {
                szFileName2 = psTests[ulTestIndex]->szFileName;
                break;
            }
        }

        /*  There have to be atleast two files in the image to do this test.
        */
        if(szFileName2)
        {
            DCLLOADERFILEHANDLE  hFile1 = NULL;
            DCLLOADERFILEHANDLE  hFile2 = NULL;

            hFile1 = DclLoaderFileOpen(hLoader, szFileName1);
            if(!hFile1)
            {
                goto Failure;
            }

            hFile2 = DclLoaderFileOpen(hLoader, szFileName2);
            DclLoaderFileClose(hLoader, hFile1);
            if(hFile2)
            {
                DclLoaderFileClose(hLoader, hFile2);
                goto Failure;
            }

            hFile2 = DclLoaderFileOpen(hLoader, szFileName2);
            if(!hFile2)
            {
                goto Failure;
            }

            hFile1 = DclLoaderFileOpen(hLoader, szFileName1);
            DclLoaderFileClose(hLoader, hFile2);
            if(hFile1)
            {
                DclLoaderFileClose(hLoader, hFile1);
                goto Failure;
            }
        }
    }

    DclStatus = DCLSTAT_SUCCESS;

Failure:

    return DclStatus;
}


/*-------------------------------------------------------------------
    Protected: DclLoaderTestLoader()

    Run the Loader through a sequence of tests, each defined by an
    LOADERTESTPARAMS structure.

    Parameters:
        psIoInstance  - Io instance as created by FileReaderIoCreate()
        psTests       - Test descriptions, consisting of file names, sizes and other test parameters.
        ulTestCount   - The number of tests to execute.

    Return Value:
        Returns DCLSTAT_SUCCESS if successful, or an error value.
-------------------------------------------------------------------*/
DCLSTATUS DclLoaderTestLoader(
    DCLREADERIO            *psIoInstance,
    LOADERTESTPARAMS      **psTests,
    D_UINT32                ulTestCount)
{
    DCLSTATUS               DclStatus = DCLSTAT_FAILURE;
    DCLLOADERHANDLE         hLoader = NULL;
    D_UINT32                ulTestIndex;

    DclPrintf("DclLoaderTestLoader: Starting Loader Tests...\n");

    /*  Create the loader instance.
    */
    hLoader = DclLoaderCreate(psIoInstance);
    if(!hLoader)
    {
        goto Failure;
    }

    /*  Iterate through the sequential test cases.
    */
    for(ulTestIndex = 0;  ulTestIndex < ulTestCount;  ulTestIndex++)
    {
        D_UINT32    ulReadCount;

        /*  Each test case can specify mutiple reads of the same file.  Loop
            through each read.
        */
        for(ulReadCount = 0;  ulReadCount < psTests[ulTestIndex]->ulFileReadCount; ulReadCount++)
        {
            DclStatus = DlLoaderTestLoaderSequentialIO(
                            hLoader,
                            psTests[ulTestIndex]->szFileName,
                            psTests[ulTestIndex]->ulFileSize,
                            psTests[ulTestIndex]->ulBufferSize,
                            psTests[ulTestIndex]->ulSerializationSeed);
            if(DclStatus != DCLSTAT_SUCCESS)
            {
                goto Failure;
            }
        }
    }

    /*  Run additional API tests.
    */
    DclStatus = DlLoaderTestLoaderAPI(hLoader, psTests, ulTestCount);

Failure:

    if(hLoader)
    {
        DclLoaderDestroy(hLoader);
    }
    if(DclStatus == DCLSTAT_SUCCESS)
    {
        DclPrintf("DclLoaderTestLoader: Passed\n");
    }
    else
    {
        DclPrintf("DclLoaderTestLoader: FAILED\n");
    }

    return DclStatus;
}
