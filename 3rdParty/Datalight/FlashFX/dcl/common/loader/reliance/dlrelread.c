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

    This module provides the ability to read a file off a Reliance formatted
    disk into memory.  This functionality is typically used in a boot loader
    environment.

    It has the following limitations:
       1)
       2)
       3) The filename must be
       4) The file is read sequentially.  There is no random access or
          llseek capability.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrelread.c $
    Revision 1.9  2009/05/21 18:33:29Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.8  2009/02/13 00:53:54Z  garyp
    Partial migration to standard structure and type naming.
    Revision 1.7  2009/01/11 01:18:15Z  brandont
    Updated to use DCL types and defines.
    Revision 1.6  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/10/05 19:27:55Z  brandont
    Removed conditional for DCLCONF_RELIANCEREADERSUPPORT.
    Revision 1.4  2007/10/03 00:47:23Z  brandont
    Added include for dlreader.h.
    Revision 1.3  2007/10/02 23:13:14Z  brandont
    Renamed FILESTATE_SUCCESS to DL_LOADER_FILESTATE_SUCCESS.
    Renamed FILESTATE_EOF to DL_LOADER_FILESTATE_EOF.
    Renamed FILESTATE_IO_ERROR to DL_LOADER_FILESTATE_IO_ERROR.
    Revision 1.2  2007/09/27 20:24:03Z  jeremys
    Renamed DCLCONF_RELIANCESUPPORT to DCLCONF_RELIANCEREADERSUPPORT.
    Revision 1.1  2007/09/27 01:23:22Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlloader.h>
#include <dlreader.h>
#include <dlreaderio.h>

#include "dlrelopts.h"
#include "dlrelbase.h"
#include "dlreltypes.h"

#include <dlrelread.h>
#include "dlrelrdhlp.h"

typedef struct sDCLREADERDISK
{
    DCLREADERIO    *psReaderDevice;
    void           *pVolume;
    D_UINT32        ulBlockSize;
} DCLREADERDISK;


static DCLSTATUS DeviceReadSectors(DCLREADERDISK *pDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, D_UCHAR *pBuffer);


/*-------------------------------------------------------------------
    Protected: DclRelReaderFileOpen()

    Open the specified file.

    Parameters:
        hDisk          - The handle for the disk
        pszFileName    - A pointer to the null terminated name

    Return Value:
        Returns the file handle of successful, else NULL.
-------------------------------------------------------------------*/
DCLREADERFILEHANDLE DclRelReaderFileOpen(
    DCLREADERINSTANCE   hDisk,
    const char         *pszFileName)
{
    DclAssert(hDisk);
    DclAssert(pszFileName);

    if(hDisk && pszFileName)
        return (DCLREADERFILEHANDLE)RelReaderFileOpen(hDisk->pVolume, pszFileName);
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Protected: DclRelReaderFileClose()

    Close a file opened with DclRelReaderFileOpen().

    Parameters:
        hFile - The file handle

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclRelReaderFileClose(
    DCLREADERFILEHANDLE hFile)
{
    DclAssert(hFile);

    if(hFile)
        RelReaderFileClose((PRELFILEINFO) hFile);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclRelReaderFileRead()

    Read data from a file on a Reliance formatted disk.  Should an
    error or EOF condition occur, this function may return fewer
    bytes than was requested.

    Parameters:
        hFile       - The file handle
        pBuffer     - A pointer to the user's I/O buffer.
        ulLength    - The number of objects to read from the stream

    Return Value:
        Returns the number of bytes read.
-------------------------------------------------------------------*/
D_UINT32 DclRelReaderFileRead(
    DCLREADERFILEHANDLE hFile,
    D_BUFFER           *pBuffer,
    D_UINT32            ulLength)
{
    DclAssert(hFile);
    DclAssert(pBuffer);

    if(hFile && pBuffer)
        return RelReaderFileRead((PRELFILEINFO) hFile, ulLength, pBuffer);
    else
        return 0;
}


/*-------------------------------------------------------------------
    Protected: DclRelReaderFileState()

    Get file state information for the specified file, and clear
    the state.

    One of the following values will be returned:

        DL_LOADER_FILESTATE_SUCCESS    - No error
        DL_LOADER_FILESTATE_EOF        - End-of-file has been reached
        DL_LOADER_FILESTATE_IO_ERROR   - Error reading from the disk

    Parameters:
        hFile       - The file handle

    Return Value:
        Returns the FILESTATE information.
-------------------------------------------------------------------*/
D_UINT16 DclRelReaderFileState(
    DCLREADERFILEHANDLE hFile)
{
    DclAssert(hFile);

    return RelReaderFileState((PRELFILEINFO) hFile);
}


                /*------------------------------*\
                 *                              *
                 *  Internal Helper Functions   *
                 *                              *
                \*------------------------------*/


/*-------------------------------------------------------------------
    Local: DeviceReadSectors()

    Read sectors from the disk.

    Parameters:
        pDisk           - The disk instance to read from.
        ulStartSector   - The sector to start reading from.
        ulCount         - The number of sectors to read.
        pBuffer         - The buffer to receive the data read.

    Return Value:
        Returns the number of sectors read.
-------------------------------------------------------------------*/
static DCLSTATUS DeviceReadSectors(
    DCLREADERDISK  *pDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulCount,
    D_UCHAR        *pBuffer)
{
    DCLSTATUS   DclStatus;

    DCLPRINTF(2, ("DeviceReadSectors: Start=%lU Count=%lU pBuff=%P\n",
               ulStartSector, ulCount, pBuffer));
    DclAssert(pDisk);
    DclAssert(pBuffer);
    DclAssert(ulCount);


    DclStatus = pDisk->psReaderDevice->fnIoRead(
            pDisk->psReaderDevice,
            ulStartSector,
            ulCount,
            pBuffer);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("Block Device read failed!\n"));
    }


    return DclStatus;
}



                /*----------------------------------*\
                 *                                  *
                 * Reliance Reader Public Interface *
                 *                                  *
                \*----------------------------------*/




/*-------------------------------------------------------------------
    Protected: DclRelReaderDiskClose()

    Close down the Reliance Reader and releases any allocated resources.

    Parameters:
        hReader  - The Reliance Reader handle

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclRelReaderDiskClose(
    DCLREADERINSTANCE hDisk)
{
    DclAssert(hDisk);
    DclAssert(hDisk->pVolume);

    RelReaderVolumeDismount((PRELDISKINST)hDisk->pVolume);

    hDisk->pVolume = NULL;
    DclMemFree(hDisk);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclRelReaderDiskOpen()

    Prepare the Reliance Reader for use.

    Parameters:
        psReaderDevice - A pointer to the DCLREADERIO structure
        pDiskHandle    - A pointer to the location to store the
                         disk handle

    Return Value:
        Returns a DCLSTATUS code indicating the success or failure of
        the operation.

        Upon successful completion, the Reliance reader handle for the
        disk will be stored in the location specified by pDiskHandle.
-------------------------------------------------------------------*/
DCLSTATUS DclRelReaderDiskOpen(
    DCLREADERIO        *psReaderDevice,
    DCLREADERINSTANCE  *ppDiskHandle)
{
    DCLREADERDISK      *pDisk = NULL;
    DCLSTATUS           DclStatus;
    RELSTATUS           TfsError;
    D_UINT32            ulTotalBlocks;

    /*
    */
    pDisk = DclMemAllocZero(sizeof *pDisk);
    if(!pDisk)
        return DCLSTAT_MEMALLOCFAILED;
    pDisk->psReaderDevice = psReaderDevice;


    /*
    */
    DclStatus = pDisk->psReaderDevice->fnIoGetParameters(
            pDisk->psReaderDevice,
            &pDisk->ulBlockSize,
            &ulTotalBlocks);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto InitFailure;
    }


    /*  under reliance a successful return is 0 so if not 0
    */
    TfsError = RelReaderVolumeMount(
            (PRELDISKINST *)&pDisk->pVolume,
            (PFNDEVICEREADSECTORS)DeviceReadSectors,
            pDisk->ulBlockSize,
            pDisk);
    if(TfsError)
    {
        goto InitFailure;
    }

    /*  Success -- store the reader handle
    */
    *ppDiskHandle = pDisk;

    return DCLSTAT_SUCCESS;

  InitFailure:
    if(pDisk)
    {
        DclMemFree(pDisk);
    }

    return DCLSTAT_FAILURE;
}
