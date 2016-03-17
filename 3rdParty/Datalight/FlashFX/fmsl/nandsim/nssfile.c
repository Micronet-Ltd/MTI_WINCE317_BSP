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
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This module implements the NAND Simulator File Storage Mechanism.  This
    mechanism will store simulated NAND data files.  The DCL file system
    services are used to perform open/read/write/etc. file operations.  Due
    to the limitations of that interface, multiple files will be used as
    needed to manage all of the simulated flash data.

    The base name and path of the file used for storage can be configured by
    servicing the FFXOPT_NANDSIM_FILE option request in your project.  The
    default base name is "FfxSim_DevN" where "N" is the device number.  If
    multiple files are needed, the specified name will be appended with
    "_FileNNNN" where "NNNN" is a four-digit decimal number for each file.

    If the specified or default files exist, the File Storage Mechanism
    indicates that a new storage areas was not created.  If files do not
    exist, it indicates that a new storage area was created.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nssfile.c $
    Revision 1.16  2009/07/18 01:07:16Z  garyp
    Merged from the v4.0 branch.  Headers updated.
    Revision 1.15  2009/02/06 08:36:12Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.14  2008/04/07 15:50:31  brandont
    Updated to use the new DCL file system services.
    Revision 1.13  2008/04/03 23:30:44Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.12  2008/03/25 20:02:52Z  Garyp
    Minor datatype changes.
    Revision 1.11  2008/02/03 04:32:50Z  keithg
    comment updates to support autodoc
    Revision 1.10  2007/12/27 00:29:21Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator
    code is now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.9  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/08/22 19:35:46Z  pauli
    Added a name field to the storage layer interface.
    --- Added comments ---  pauli [2007/08/22 20:07:09Z]
    Resolve Bug 1084: extended the default file storage layer to use multiple
    files to support storing more than 2GB of data.
    Revision 1.7  2007/08/01 18:32:12Z  timothyj
    Fixed problem where only the first byte of the erase buffer was filled with
    0xFF.
    Revision 1.6  2007/06/26 19:12:17Z  timothyj
    Added Erase method, for more accurate simulation purposes and to enable
    storage mechanism optimizations.
    Fixed call to DclSNPrintf (see BZ #1113).
    Account for existing file size not matching (previously resulted in failed
    format).
    Revision 1.5  2007/06/21 19:22:02Z  pauli
    Fixed an inverted condition in an error check that prevented the
    simulator from using a file smaller than 2GB.
    Revision 1.4  2007/04/26 00:13:47Z  timothyj
    Updated to use DCL 64-bit macros in lieu of requiring native 64-bit
    compiler support.
    Revision 1.3  2006/08/26 00:30:03Z  Pauli
    Added compile time setting to allow enabling/disabling the NAND simulator.
    Revision 1.2  2006/07/27 00:54:28Z  DeanG
    Force binary mode
    Revision 1.1  2006/07/26 20:25:28Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <nandsim.h>


#if FFXCONF_NANDSUPPORT

static NsStorageCreate     Create;
static NsStorageDestroy    Destroy;
static NsStorageRead       Read;
static NsStorageWrite      Write;
static NsStorageErase      Erase;

NsStorageInterfaces NANDSIM_FileStorage =
{
    Create,
    Destroy,
    Read,
    Write,
    Erase,
    "File"
};


struct tagNsStorageData
{
    /*  Number of files
    */
    unsigned long   ulFiles;

    /*  Size of each of the individual files except for the
        last one.
    */
    long            lFileSize;

    /*  Array of handles to each file
    */
    DCLFSFILEHANDLE  *phFile;

    /*  Erase Buffer
    */
    D_BUFFER        pEraseBuffer[32768];
};


static unsigned long FileAndOffset(NsStorageHandle hStorage, D_UINT64 *pullOffset, long *plFileOffset);


/*-------------------------------------------------------------------
    NAND Simulator Storage Interface
*/


/*-------------------------------------------------------------------
    Public: Create

    This function performs whatever initialization the storage
    mechanism needs.  The storage mechanism must determine if a
    storage area already exists.

    Parameters:
        hDev      - An opaque pointer to a device on which this
                    module is to begin operating.  This module uses
                    this handle to acquire per-device configuration.
        ullSize   - Total size of the storage area to create, in bytes.
                    This is used only if the storage area does not already
                    exist.

        pfCreate  - Return parameter indicating if the storage area was
                    created.  How this is determined is specific to each
                    storage mechanism implementation.

                    For this file storage mechanism, a new area is created
                    when the files are created.

    Return:
        Returns a handle to new instance of the storage mechanism that
        is used for subsequent storage mechanism calls.
-------------------------------------------------------------------*/
static NsStorageHandle Create(
    FFXDEVHANDLE    hDev,
    D_UINT64        ullSize,
    D_BOOL         *pfCreated)
{
    NsStorageData  *pStorageData;
    D_BOOL          fSuccess = FALSE;
    char            szDefaultBaseName[14];
    char           *szBaseName = NULL;
    char           *szFullName = NULL;
    unsigned        uFullNameLength;
    long            lFileSize;
    unsigned long   ulCurFile;
    long            lLastFileOffset;
    D_UINT64        ullTotalFound;
    DCLSTATUS DclStatus;


    FFXPRINTF(1, ("NAND Simulator File Storage Create\n"));

    DclProductionAssert(pfCreated);
    if(!pfCreated)
    {
        return NULL;
    }

    /*  Allocate our private data.
    */
    pStorageData = DclMemAllocZero(sizeof(*pStorageData));
    if(!pStorageData)
    {
        FFXPRINTF(1, ("Failed to allocated memory for storage layer data.\n"));
        return NULL;
    }
    DclMemSet(pStorageData->pEraseBuffer, ERASED8, sizeof(pStorageData->pEraseBuffer));
    pStorageData->lFileSize = D_INT32_MAX;

    /*  Determine how many files are needed to manage the data and allocate
        an array of handles.
    */
    pStorageData->ulFiles = FileAndOffset(pStorageData, &ullSize, &lLastFileOffset);
    if (lLastFileOffset)
    {
        pStorageData->ulFiles++;
    }
    FFXPRINTF(1, ("Using %lU file(s) to store %llX bytes of data.\n", pStorageData->ulFiles, ullSize));
    pStorageData->phFile = DclMemAllocZero(sizeof(*pStorageData->phFile) * pStorageData->ulFiles);
    if(!pStorageData->phFile)
    {
        FFXPRINTF(1, ("Failed to allocate file handle array.\n"));
        DclMemFree(pStorageData);
        return NULL;
    }

    /*  Setup the base file name and memory space for the full name.
    */
    if(!FfxHookOptionGet(FFXOPT_NANDSIM_FILE, hDev, &szBaseName, sizeof(szBaseName)))
    {
        DclSNPrintf(szDefaultBaseName, sizeof(szDefaultBaseName), "FfxSim_Dev%1u", (*hDev)->Conf.nDevNum);
        szBaseName = szDefaultBaseName;
    }
    uFullNameLength = DclStrLen(szBaseName) + 10;
    szFullName = DclMemAlloc(uFullNameLength);
    if(!szFullName)
    {
        FFXPRINTF(1, ("Failed to allocate memory for the file name.\n"));
        DclMemFree(pStorageData->phFile);
        DclMemFree(pStorageData);
        return NULL;
    }
    FFXPRINTF(1, ("Base file name: %s\n", szBaseName));

    /*  Start out assuming we can open the files
    */
    *pfCreated = FALSE;
    DclUint64AssignUint32(&ullTotalFound, 0);
    for (ulCurFile = 0;
         ulCurFile < pStorageData->ulFiles;
         ulCurFile++)
    {
        if(pStorageData->ulFiles > 1)
        {
            DclSNPrintf(szFullName, uFullNameLength, "%s_File%04lU", szBaseName, ulCurFile);
        }
        else
        {
            DclStrCpy(szFullName, szBaseName);
        }

        /*  Attempt to open the files.
        */
        DclStatus = DclFsFileOpen(szFullName, "r+b", &pStorageData->phFile[ulCurFile]);
        if(!DclStatus)
        {
            /*  File already exists
            */

            /*  Check to see if we've already created any new ones (if so, this is an error)
            */
            if (*pfCreated)
            {
                FFXPRINTF(1, ("Some files in the array are missing (others already exist).\n"));
                break;
            }

            /*  Check to see if the size matches
            */

            /*  Tell the file pointer to seek to the end of the file.
            */
            if(DclOsFileSeek(pStorageData->phFile[ulCurFile], 0, DCLFSFILESEEK_END) != 0)
            {
                FFXPRINTF(1, ("Failed to seek to end of file.\n"));
                break;
            }

            /*  Determine the size of the file by retrieving the offset of the
                pointer into the file.  The pointer will be at the end of the
                file, which was set by the seek function.
            */
            lFileSize = DclOsFileTell(pStorageData->phFile[ulCurFile]);
            if (((lFileSize != pStorageData->lFileSize) && (ulCurFile < pStorageData->ulFiles - 1))
                || ((lFileSize != lLastFileOffset) && (ulCurFile == pStorageData->ulFiles - 1)))
            {
                FFXPRINTF(1, ("File %s does not match configured file size. Found %lX, expected %lX\n", szFullName, lFileSize, pStorageData->lFileSize));
                break;
            }
            DclUint64AddUint32(&ullTotalFound, lFileSize);
        }
        else
        {
            DclStatus = DclFsFileOpen(szFullName, "w+b", &pStorageData->phFile[ulCurFile]);
            if(DclStatus)
            {
                /*  Failed to create the file.
                */
                FFXPRINTF(1, ("Failed to create the file.\n"));
                break;
            }

            if (ulCurFile == 0)
            {
                /*  Indicate if we created a new storage area.
                */
                *pfCreated = TRUE;
            }
            else
            {
                if (!*pfCreated)
                {
                    FFXPRINTF(1, ("Some files in the array are missing (others already exist).\n"));
                    break;
                }
            }
        }

        fSuccess = TRUE;
    }

    /*  Free the file name memory space now that we no longer need it.
    */
    DclMemFree(szFullName);

    /*  Make sure we found all the data we expected.
    */
    if(!*pfCreated && !DclUint64IsEqualUint64(&ullTotalFound, &ullSize))
    {
        FFXPRINTF(1, ("Data mismatch: found %llX, expected %llX.\n", ullTotalFound, ullSize));
        fSuccess = FALSE;
    }

    if(!fSuccess)
    {
        /*  We didn't make it through the loop, clean up.
        */
        while (ulCurFile--)
        {
            if (pStorageData->phFile[ulCurFile])
            {
                DclFsFileClose(pStorageData->phFile[ulCurFile]);
            }
        }
        DclMemFree(pStorageData->phFile);
        DclMemFree(pStorageData);
        pStorageData = NULL;
    }



    return pStorageData;
}


/*-------------------------------------------------------------------
    Public: Destroy

    This function is called to destroy an instance of the storage
    mechanism.  It should perform whatever cleanup is required for
    this storage mechanism implementation.

    Parameters:
        hStorage  - a handle to the storage mechanism.

    Return:
        (none)
-------------------------------------------------------------------*/
static void Destroy(
    NsStorageHandle hStorage)
{
    D_UINT32 ulCurFile;

    DclProductionAssert(hStorage);
    if(hStorage)
    {
        /*  Close the files.
        */
        for (ulCurFile = 0;
             ulCurFile < hStorage->ulFiles;
             ulCurFile++)
        {
            DclFsFileClose(hStorage->phFile[ulCurFile]);
        }

        /*  Free our private data.
        */
        DclMemFree(hStorage->phFile);
        DclMemFree(hStorage);
    }
}


/*-------------------------------------------------------------------
    Public: Read

    This function is called to read from the storage mechanism.

    Parameters:
        hStorage  - a handle to the storage mechanism.
        ullOffset - the byte offset to read from.
        pBuffer   - the buffer to populate with data from the storage mechanism
        ulLength  - the number of bytes to read

    Return:
        None, failure is catastrophic.
-------------------------------------------------------------------*/
static void Read(
    NsStorageHandle hStorage,
    D_UINT64        ullOffset,
    D_BUFFER       *pBuffer,
    D_UINT32        ulLength )
{
    size_t          ulBytes;
    size_t          BytesToRead;
    int             status;
    unsigned int    ulCurFile;
    long            lCurOffset;


    DclProductionAssert(hStorage);
    DclProductionAssert(pBuffer);

    /*  Determine the starting file number and the offset within that file
    */
    ulCurFile = FileAndOffset(hStorage, &ullOffset, &lCurOffset);

    while (ulLength)
    {
        status = DclOsFileSeek(hStorage->phFile[ulCurFile], lCurOffset, DCLFSFILESEEK_SET);
        DclProductionAssert(status == 0);
        DclProductionAssert(lCurOffset == DclOsFileTell(hStorage->phFile[ulCurFile]));

        BytesToRead = DCLMIN(ulLength, (D_UINT32)(hStorage->lFileSize - lCurOffset));

        ulBytes = DclOsFileRead(pBuffer, 1, BytesToRead, hStorage->phFile[ulCurFile]);
        if(ulBytes != BytesToRead)
        {
            DclPrintf("Read Failed: expected %lX, got %lX\n", BytesToRead, ulBytes);
        }
        DclProductionAssert(ulBytes == BytesToRead);

        ulLength -= ulBytes;
        pBuffer += ulBytes;

        /*  If there's still data to read, go to the beginning of the next file
        */
        if (ulLength)
        {
            ulCurFile++;
            lCurOffset = 0;
        }
    }
}


/*-------------------------------------------------------------------
    Public: Write

    This function is called to write data to the storage mechanism.

    Parameters:
        hStorage  - a handle to the storage mechanism.
        ullOffset - the byte offset to write to.
        pBuffer   - the buffer containing data to write to the storage mechanism
        ulLength  - the number of bytes to write

    Return:
        None, failure is catastrophic.
-------------------------------------------------------------------*/
static void Write(
    NsStorageHandle hStorage,
    D_UINT64        ullOffset,
    D_BUFFER const *pBuffer,
    D_UINT32        ulLength)
{
    size_t          ulBytes;
    size_t          BytesToWrite;
    int             status;
    unsigned int    ulCurFile;
    long            lCurOffset;


    DclProductionAssert(hStorage);
    DclProductionAssert(pBuffer);

    /*  Determine the starting file number and the offset within that file
    */
    ulCurFile = FileAndOffset(hStorage, &ullOffset, &lCurOffset);

    while (ulLength)
    {
        status = DclOsFileSeek(hStorage->phFile[ulCurFile], lCurOffset, DCLFSFILESEEK_SET);
        DclProductionAssert(status == 0);
        DclProductionAssert(lCurOffset == DclOsFileTell(hStorage->phFile[ulCurFile]));

        BytesToWrite = DCLMIN(ulLength, (D_UINT32)(hStorage->lFileSize - lCurOffset));

        ulBytes = DclOsFileWrite(pBuffer, 1, BytesToWrite, hStorage->phFile[ulCurFile]);
        DclProductionAssert(ulBytes == BytesToWrite);

        ulLength -= ulBytes;
        pBuffer += ulBytes;

        /*  If there's still data to write, go to the beginning of the next file
        */
        if (ulLength)
        {
            ulCurFile++;
            lCurOffset = 0;
        }
    }
}


/*-------------------------------------------------------------------
    Public: Erase

    This function is called to erase data on the storage mechanism.

    Parameters:
        hStorage  - a handle to the storage mechanism.
        ullOffset - the byte offset to erase.
        ulLength  - the number of bytes to erase

    Return:
        None, failure is catastrophic.
-------------------------------------------------------------------*/
static void Erase(
    NsStorageHandle hStorage,
    D_UINT64        ullOffset,
    D_UINT32        ulLength)
{
    D_UINT32        ulLengthToErase;

    DclProductionAssert(hStorage);

    while (ulLength)
    {
        ulLengthToErase = DCLMIN(ulLength, sizeof(hStorage->pEraseBuffer));

        Write(hStorage, ullOffset, hStorage->pEraseBuffer, ulLengthToErase);

        ulLength -= ulLengthToErase;

        DclUint64AddUint32(&ullOffset, ulLengthToErase);
    }
}


/*-------------------------------------------------------------------
    Local utility/helper functions
*/


/*-------------------------------------------------------------------
    Public: FileAndOffset

    This function converts a storage offset into a file index
    and file offset.

    Parameters:
        hStorage     - a handle to the storage mechanism.
        pullOffset   - Storage-relative offset to convert (typically the
                       value passed into Read, Write, Erase, etc.).

        plFileOffset - (OPTIONAL) On return, the file-relative offset
                       corresponding to *pullOffset.


    Return:
        Returns the index of the file containing the data at *pullOffset.
-------------------------------------------------------------------*/
static unsigned long FileAndOffset(
    NsStorageHandle hStorage,
    D_UINT64       *pullOffset,
    long           *plFileOffset)
{
    D_UINT64    ullFile;

    DclUint64AssignUint64(&ullFile, pullOffset);
    DclUint64DivUint32(&ullFile, hStorage->lFileSize);
    DclProductionAssert(!DclUint64GreaterUint32(&ullFile, D_UINT32_MAX));

    if (plFileOffset)
    {
        D_UINT64    ullFileOffset;

        DclUint64AssignUint64(&ullFileOffset, pullOffset);
        DclUint64ModUint32(&ullFileOffset, hStorage->lFileSize);
        DclProductionAssert(!DclUint64GreaterUint32(&ullFileOffset, D_INT32_MAX));

        *plFileOffset = DclUint32CastUint64(&ullFileOffset);
    }

    return DclUint32CastUint64(&ullFile);

}


#endif  /* FFXCONF_NANDSUPPORT */

