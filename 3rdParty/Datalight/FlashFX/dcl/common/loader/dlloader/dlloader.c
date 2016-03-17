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

    This module provides generic functionality to initialize the Datalight
    loader and load a file off a FAT or Reliance disk into memory.  This
    typically used in a boot loader environment.

    This interface is designed such that client code calling these functions
    only needs to include dlloader.h.

    Set the DCLCONF_FATREADERSUPPORT and/or DCLCONF_RELIANCEREADERSUPPORT
    values to TRUE or FALSE to enable or disable support for the respective
    file system.  It is possible to set both value to TRUE to build a loader
    module that can load a file off either kind of disk.

    Note that this interface provides sequential file access only.  Random
    access and llseek() functionality is not supported.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlloader.c $
    Revision 1.12  2009/06/25 22:49:21Z  garyp
    Updated to use the new DclSignOn() calling convention.
    Revision 1.11  2009/05/21 18:37:09Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.10  2007/12/18 20:14:31Z  brandont
    Updated function headers.
    Revision 1.9  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.8  2007/10/03 00:47:24Z  brandont
    Added include for dlreader.h.
    Revision 1.7  2007/10/02 23:13:13Z  brandont
    Renamed FILESTATE_IO_ERROR to DL_LOADER_FILESTATE_IO_ERROR.
    Revision 1.6  2007/10/02 01:47:21Z  brandont
    Updated references to DCLCONF_FATSUPPORT and DCLCONF_RELIANCESUPPORT to
    refer to DCLCONF_FATREADERSUPPORT and DCLCONF_RELIANCEREADERSUPPORT
    respectively.  Updated function headers.  Changed DclPrintf calls to
    DCLPRINTF calls.
    Revision 1.5  2007/10/02 00:25:50Z  brandont
    Removed superfluous variable use in DclLoaderDeinitialize.
    Removed invalid assert from DclLoaderDeinitialize
    Revision 1.4  2007/10/02 00:13:51Z  brandont
    Removed FfxProjMain prototype.  Changed a few bitwise AND opertions to
    logical AND operations for asserting that only one type of file system is
    in use for a single instance of the reader.
    Revision 1.3  2007/10/01 23:31:14Z  brandont
    Removed call to FfxProjMain in DclLoaderInitialize.
    Revision 1.2  2007/09/27 20:02:56Z  jeremys
    Renamed DCLCONF_RELIANCESUPPORT to DCLCONF_RELIANCEREADERSUPPORT and
    DCLCONF_FATSUPPORT to DCLCONF_FATREADERSUPPORT.
    Revision 1.1  2007/09/27 01:17:40Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#if DCLCONF_FATREADERSUPPORT || DCLCONF_RELIANCEREADERSUPPORT

#include <dlloader.h>
#include <dlreader.h>

#if DCLCONF_FATREADERSUPPORT
#include <dlfatread.h>
#endif

#if DCLCONF_RELIANCEREADERSUPPORT
#include <dlrelread.h>
#endif


/*  This structure manages an instance of the Datalight loader
*/
struct sDlLoaderInstance
{
    DCLREADERINSTANCE hReader;
    D_BOOL            fIsReliance;
    D_BOOL            fIsFat;
};


/*  The max number of bytes that we can transfer at one time
*/
#define  MAX_BYTES_TO_READ (32*1024L)


/*-------------------------------------------------------------------
    Public: DclLoaderFileOpen()

    Open a file for use by the loader.  The specified file name must
    be in a form appropriate for the file system that is being used.

    Parameters:
        hLoader        - The loader handle.
        pszName        - The null terminated file name.

    Return Value:
        Returns a file handle if successful, else NULL.
-------------------------------------------------------------------*/
DCLLOADERFILEHANDLE DclLoaderFileOpen(
    DCLLOADERHANDLE     hLoader,
    const char         *pszName)
{
    DCLREADERFILEHANDLE hFile = NULL;

    DclAssert(hLoader);
    DclAssert(pszName);
    DclAssert(hLoader->fIsFat || hLoader->fIsReliance);
    DclAssert(!(hLoader->fIsFat && hLoader->fIsReliance));

  #if DCLCONF_FATREADERSUPPORT
    if(hLoader->fIsFat)
        hFile = DclFatReaderFileOpen(hLoader->hReader, pszName);
  #endif

  #if DCLCONF_RELIANCEREADERSUPPORT
    if(hLoader->fIsReliance)
        hFile = DclRelReaderFileOpen(hLoader->hReader, pszName);
  #endif

    return hFile;
}


/*-------------------------------------------------------------------
    Public: DclLoaderFileClose()

    Close a file that was opened with DlLoaderFileOpen().

    Parameters:
        hLoader        - The loader handle.
        hFile          - The file handle.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclLoaderFileClose(
    DCLLOADERHANDLE      hLoader,
    DCLLOADERFILEHANDLE  hFile)
{
    DclAssert(hLoader);
    DclAssert(hFile);
    DclAssert(hLoader->fIsFat || hLoader->fIsReliance);
    DclAssert(!(hLoader->fIsFat && hLoader->fIsReliance));

    if(!hLoader || !hFile)
        return;

  #if DCLCONF_FATREADERSUPPORT
    if(hLoader->fIsFat)
        DclFatReaderFileClose(hFile);
  #endif

  #if DCLCONF_RELIANCEREADERSUPPORT
    if(hLoader->fIsReliance)
        DclRelReaderFileClose(hFile);
  #endif

    return;
}


/*-------------------------------------------------------------------
    Public: DclLoaderFileRead()

    Read data from a file using a handle returned by DclLoaderFileOpen().

    Parameters:
        hLoader        - The loader handle.
        hFile          - The file handle.
        uByteCount     - The number of bytes to read.
        pBuffer        - A pointer to the destination buffer.

    Return Value:
        Returns the number of bytes read.
-------------------------------------------------------------------*/
unsigned int DclLoaderFileRead(
    DCLLOADERHANDLE     hLoader,
    DCLLOADERFILEHANDLE hFile,
    unsigned int        uByteCount,
    unsigned char      *pBuffer)
{
    D_UINT32            ulBytesLeftToRead = uByteCount;
    D_UINT32            ulBytesToReadNow;
    D_UINT32            ulBytesJustRead = 0;

    DclAssert(hLoader);
    DclAssert(hFile);
    DclAssert(pBuffer);
    DclAssert(hLoader->fIsFat || hLoader->fIsReliance);
    DclAssert(!(hLoader->fIsFat && hLoader->fIsReliance));

    if(!hLoader || !hFile || !pBuffer)
        return 0;

    while(ulBytesLeftToRead)
    {
        if(ulBytesLeftToRead > MAX_BYTES_TO_READ)
            ulBytesToReadNow = MAX_BYTES_TO_READ;
        else
            ulBytesToReadNow = ulBytesLeftToRead;

      #if DCLCONF_FATREADERSUPPORT
        if(hLoader->fIsFat)
        {
            ulBytesJustRead = DclFatReaderFileRead(hFile, pBuffer, ulBytesToReadNow);
        }
      #endif

      #if DCLCONF_RELIANCEREADERSUPPORT
        if(hLoader->fIsReliance)
        {
            ulBytesJustRead = DclRelReaderFileRead(hFile, pBuffer, ulBytesToReadNow);
        }
      #endif

        DclAssert(ulBytesJustRead <= D_UINT16_MAX);

        /*  The following casts are here only to mollify the BC45 compiler,
            but we know it's safe because MAX_BYTES_TO_READ is kept
            under 64K.
        */
        pBuffer += (D_UINT16) ulBytesJustRead;
        ulBytesLeftToRead -= ulBytesJustRead;

        if(ulBytesJustRead != ulBytesToReadNow)
            return uByteCount - (int)ulBytesLeftToRead;
    }

    return uByteCount;
}


/*-------------------------------------------------------------------
    Public: DclLoaderFileState()

    Get the file state information for the specified file handle
    then clear the state.

    Parameters:
        hLoader     - The loader handle.
        hFile       - The file handle

    Return Value:
        DL_LOADER_FILESTATE_SUCCESS    - No error
        DL_LOADER_FILESTATE_EOF        - End-of-file has been reached
        DL_LOADER_FILESTATE_IO_ERROR   - Error reading from the disk
-------------------------------------------------------------------*/
unsigned int DclLoaderFileState(
    DCLLOADERHANDLE     hLoader,
    DCLLOADERFILEHANDLE hFile)
{
    DclAssert(hLoader);
    DclAssert(hFile);
    DclAssert(hLoader->fIsFat || hLoader->fIsReliance);
    DclAssert(!(hLoader->fIsFat && hLoader->fIsReliance));

    if(hLoader && hFile)
    {
      #if DCLCONF_FATREADERSUPPORT
        if(hLoader->fIsFat)
            return DclFatReaderFileState(hFile);
      #endif

      #if DCLCONF_RELIANCEREADERSUPPORT
        if(hLoader->fIsReliance)
            return DclRelReaderFileState(hFile);
      #endif
    }

    return DL_LOADER_FILESTATE_IO_ERROR;
}


/*-------------------------------------------------------------------
    Public: DclLoaderDestroy()

    Destroy an instance of the loader as created by DclLoaderCreate().

    Parameters:
        hLoader        - The loader handle

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclLoaderDestroy(
    DCLLOADERHANDLE hLoader)
{
    DCLPRINTF(1, ("DlLoader: Deinitializing\n"));

    if(hLoader)
    {
      #if DCLCONF_FATREADERSUPPORT
        if(hLoader->fIsFat)
            DclFatReaderDiskClose(hLoader->hReader);
      #endif

      #if DCLCONF_RELIANCEREADERSUPPORT
        if(hLoader->fIsReliance)
            DclRelReaderDiskClose(hLoader->hReader);
      #endif

        /*  Free the memory allocated to manage DlLoader instance
        */
        DclMemFree(hLoader);
    }
}


/*-------------------------------------------------------------------
    Public: DclLoaderCreate()

    Create an instance of the loader.

    Parameters:
        psReaderDevice - A pointer to the DCLREADERIO structure created
                         by a Reader I/O layer.

    Return Value:
        Returns the loader handle if successful, else NULL.
-------------------------------------------------------------------*/
DCLLOADERHANDLE DclLoaderCreate(
    DCLREADERIO        *psReaderDevice)
{
    DCLSTATUS           DclStatus;
    DCLLOADERHANDLE     psLoader = (void *)0;

    DclSignOn(FALSE);

    /*  Allocate a structure to manage this instance of the reader
    */
    psLoader = DclMemAllocZero(sizeof(*psLoader));
    if(!psLoader)
        goto InitFailure;

    DclStatus = DCLSTAT_FAILURE;

  #if DCLCONF_FATREADERSUPPORT
    /*  Attempt to mount using the FAT reader
    */
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        DclStatus = DclFatReaderDiskOpen(psReaderDevice, &psLoader->hReader);
        if(DclStatus == DCLSTAT_SUCCESS)
        {
            psLoader->fIsFat = TRUE;
        }
    }
  #endif

  #if DCLCONF_RELIANCEREADERSUPPORT
    /*  Attempt to mount using the Reliance reader
    */
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        DclStatus = DclRelReaderDiskOpen(psReaderDevice, &psLoader->hReader);
        if(DclStatus == DCLSTAT_SUCCESS)
        {
            psLoader->fIsReliance = TRUE;
        }
    }
  #endif

    if(DclStatus != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("DlLoader: Error initializing FS Reader, Status=%lX\n", DclStatus));
        goto InitFailure;
    }

    DCLPRINTF(1, ("DlLoader: Initialization complete\n"));
    return psLoader;

  InitFailure:

    DclLoaderDestroy(psLoader);
    DCLPRINTF(1, ("DlLoader: Init Failed\n"));
    return NULL;
}

#endif /* #if DCLCONF_FATREADERSUPPORT || DCLCONF_RELIANCEREADERSUPPORT */

