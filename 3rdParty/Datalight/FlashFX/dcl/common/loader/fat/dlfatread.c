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

    This module provides the ability to read a file off a FAT formatted disk
    into memory.  This functionality is typically used in a boot loader
    environment.

    It has the following limitations:
       1) Only FAT12 or FAT16 disk formats are supported.
       2) Files can only be read from the root directory
       3) The filename must be in single byte ASCII 8.3 format
       4) The file is read sequentially.  There is no random access or
          llseek capability.
       5) Only 512-byte sectors are supported.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfatread.c $
    Revision 1.12  2009/06/25 23:34:14Z  garyp
    Final merge from the v4.0 branch --  Updated to use the cleaned up and
    refactored BPB structures. 
    Revision 1.11  2009/05/21 18:28:21Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.10  2009/02/13 20:23:58Z  garyp
    Resolved bug 2351 -- Dependencies on structure packing.  General cleanup
    of documentation and code formatting.  Partial migration to standard
    structure and type naming.
    Revision 1.9  2008/06/05 01:19:05Z  brandont
    Corrected warning for duplicate definition of FILENAME_MAX.
    Revision 1.8  2008/06/03 21:43:07Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.7  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.6  2007/10/05 02:04:21Z  brandont
    Removed conditional for DCLCONF_FATREADERSUPPORT.
    Revision 1.5  2007/10/03 01:17:52Z  brandont
    Renamed FILEINFO to sDlLoaderFile.
    Revision 1.4  2007/10/03 00:47:23Z  brandont
    Added include for dlreader.h.
    Revision 1.3  2007/10/02 23:13:13Z  brandont
    Renamed FILESTATE_SUCCESS to DL_LOADER_FILESTATE_SUCCESS.
    Renamed FILESTATE_EOF to DL_LOADER_FILESTATE_EOF.
    Renamed FILESTATE_IO_ERROR to DL_LOADER_FILESTATE_IO_ERROR.
    Revision 1.2  2007/09/27 20:02:07Z  jeremys
    Renamed DCLCONF_FATSUPPORT to DCLCONF_FATREADERSUPPORT.
    Revision 1.1  2007/09/27 01:24:08Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlloader.h>
#include <dlreader.h>
#include <dlreaderio.h>

#include <dlfatapi.h>
#include <dlfatread.h>
#include <dlpartid.h>
#include <dlfat.h>

/*  This setting must always be '1' as support for multiple open
    files is not yet implemented.
*/
#define MAX_OPEN_FILES  1

/*  Maximum number of characters in a file name. Only 8.3 names are
    supported.  Only single byte characters are supported.
*/
#define DL_LOADER_FILENAME_MAX    13


/*  Definitions and macros used by the FAT reader.
*/
#define FAT16_ENTRIES_PER_SECTOR 0x100
#define IS_FAT_EOF(c)            ((c) >= FAT16_MAXCLUSTERS)
#define VALID_CLUSTER(c)         (((c) >= 2) && (!IS_FAT_EOF(c)))
#define DIR_ENTRIES_PER_SECTOR   (FAT_SECTORLEN / FAT_DIRENTRYSIZE)

/* ToDo: If this implementation works with FAT12 and FAT16, how is it that
         the VALID_CLUSTER() macro is implemented based on FAT16 specific
         geometry?  If it is correct, it at least needs a comment.
*/

typedef struct sDCLREADERFILE
{
    struct sDCLREADERFILE  *pFINext;        /* pointer to next one of these in some list */
    DCLREADERINSTANCE       hDisk;
    D_UINT32        ulFileSize;             /* file size from DIR                     */
    D_UINT32        ulCurrentFilePosition;  /* byte offset into file on disk          */
    D_UINT32        ulCurrentSectorOffset;  /* where we are in the file    */
    D_UINT16        uFileState;             /* state flags                            */
    D_UINT16        uStartingCluster;       /* starting cluster of file on disk       */
    D_UINT16        uCurrentCluster;        /* current cluster number being accessed  */
    D_UINT16        uSectorsLeftInCluster;  /* sectors remaining in above cluster     */
    D_UINT16        uBytesLeftInBuffer;     /* bytes remaining in buffer below        */
    D_UINT16        uPadding;               /* padding                                */
    D_UCHAR         acFileBuff[FAT_SECTORLEN];  /* disk I/O buffer for this file          */
} DCLREADERFILE;

typedef struct sDCLREADERDISK
{
    DCLREADERIO    *psReaderDevice;
    D_UINT32        ulFirstFATStart;    /* Sector of first FAT             */
    D_UINT32        ulRootDirStart;     /* Sector of root DIR              */
    D_UINT32        ulFileDataStart;    /* Sector of data area on disk     */
    D_UINT16        uRootDirSectors;    /* number of DIR entries in root DIR   */
    D_UINT16        uBufferedSector;    /* sector contained in the scratch buffer */
    D_UCHAR         acSectorBuff[FAT_SECTORLEN];        /* scratch I/O buffer                  */
    DCLREADERFILE   fi[MAX_OPEN_FILES];
    DCLREADERFILE  *pFIFreeList;
    DCLREADERFILE  *pFIUsedList;
    DCLFATBPB       bpb;
    D_BYTE          bFSID;
    D_UINT32        ulBlockSize;
} DCLREADERDISK;


/*-------------------------------------------------------------------
                  Prototypes for internal functions
-------------------------------------------------------------------*/
static D_UINT32         GetSectorOffsetOfCluster(DCLREADERDISK *pDisk, D_UINT16 uClusterIndex);
static D_UINT16         FindNextCluster(DCLREADERDISK *pDisk, D_UINT16 uClusterIndex);
static D_BOOL           LoadNextCluster(DCLREADERFILE *pFile, D_UINT16 uSectorsPerCluster);
static D_UINT16         ReadNextFileSegments(DCLREADERFILE *pFile, D_UCHAR *pBuffer, D_UINT16 uBytesToRead);
static D_UINT16         ReadFileData(DCLREADERFILE *pFile, D_UCHAR *pBuffer, D_UINT16 uBytesToRead);
static DCLREADERFILE *  AllocateFileStructure(DCLREADERDISK *pDisk);
static D_BOOL           FreeFileStructure(DCLREADERDISK *pDisk, DCLREADERFILE *pFileToFree);
static D_BOOL           FindFileEntry(const char *pszFileName, DCLREADERFILE *pFile);
static D_BOOL           ExpandDotName(const char *pszFileName, D_UCHAR * pszName);
static D_BOOL           NamesMatch(D_UCHAR *pNameOne, D_UCHAR *pNameTwo);
static DCLSTATUS        DeviceReadSectors(DCLREADERDISK *pDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, D_UCHAR *pBuffer);



                /*------------------------------*\
                 *                              *
                 *       Public Interface       *
                 *                              *
                \*------------------------------*/


/*-------------------------------------------------------------------
    Protected: DclFatReaderDiskOpen()

    Open a disk fo ruse by the FAT Reader.  Upon successful
    completion, the FAT reader handle for the disk will be
    stored in the location specified by pDiskHandle.

    Parameters:
        psReaderDevice     - A pointer to the DCLREADERIO structure
        pDiskHandle        - A pointer to the location to store the
                             disk handle

    Return Value:
        Returns a DCLSTATUS code indicating the success or failure
        of the operation.
-------------------------------------------------------------------*/
DCLSTATUS DclFatReaderDiskOpen(
    DCLREADERIO        *psReaderDevice,
    DCLREADERINSTANCE  *ppDiskHandle)
{
    DCLREADERDISK      *pDisk = NULL;
    DCLSTATUS           DclStatus;
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

    pDisk->pFIFreeList = &pDisk->fi[0];

    pDisk->uBufferedSector = (D_UINT16)~0;

    DclStatus = DeviceReadSectors(pDisk, 0, 1, &pDisk->acSectorBuff[0]);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto InitFailure;
    }
    pDisk->uBufferedSector = 0;

    pDisk->bFSID = DclFatBPBMove(&pDisk->bpb, &pDisk->acSectorBuff[0]);
    if((pDisk->bFSID != FSID_FAT12) && (pDisk->bFSID != FSID_FAT16))
    {
        goto InitFailure;
    }

    DclFatBPBMove(&pDisk->bpb, &pDisk->acSectorBuff[0]);

    /*  Get offset of first FAT and remember sector
    */
    pDisk->ulFirstFATStart = pDisk->bpb.uReservedSectors;
    pDisk->ulRootDirStart = pDisk->ulFirstFATStart;

    /*  Get offset of root DIR and remember sector compute sectors
        in root dir.
    */
    pDisk->ulRootDirStart += pDisk->bpb.bNumFats * pDisk->bpb.uSecPerFAT;
    pDisk->ulFileDataStart = pDisk->ulRootDirStart;
    pDisk->uRootDirSectors =
        (pDisk->bpb.uMaxDirs * FAT_DIRENTRYSIZE) / FAT_SECTORLEN;

    /*  get the offset of where the disk data starts
    */
    pDisk->ulFileDataStart += pDisk->uRootDirSectors;

    /*  Success -- store the reader handle
    */
    *ppDiskHandle = pDisk;

    return DCLSTAT_SUCCESS;

  InitFailure:
    if(pDisk)
        DclMemFree(pDisk);

    return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
    Protected: DclFatReaderDiskClose()

    Close a FAT Reader disk and releases any allocated resources.

    Parameters:
        hReader  - The FAT Reader handle

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclFatReaderDiskClose(
    DCLREADERINSTANCE hDisk)
{
    DclAssert(hDisk);

    /*  close any open files here
    */

    DclMemFree(hDisk);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclFatReaderFileOpen()

    Open the file for use by the FAT Reader.  Only read only access
    is currently supported.  Only 8.3 files names in the root of the
    drive are supported.

    Parameters:
        hDisk          - The handle for the disk
        pszFileName    - A pointer to the null terminated name

    Return Value:
        Returns the file handle of successful, NULL otherwise.
-------------------------------------------------------------------*/
DCLREADERFILEHANDLE DclFatReaderFileOpen(
    DCLREADERINSTANCE   hDisk,
    const char         *pszFileName)
{
    DCLREADERFILE      *pFile;

    /*  allocate a new file structure, return NULL if none left
    */
    pFile = AllocateFileStructure(hDisk);
    if(pFile != NULL)
    {
        pFile->hDisk = hDisk;

        /*  Try and find the file system directory entry for this file, if it's
            not there then free the struc and return NULL.  Otherwise the file
            access info we need will be copied from the file directory entry
            into our file control struct and we'll be ready to go!
        */
        if(!FindFileEntry(pszFileName, pFile))
        {
            FreeFileStructure(hDisk, pFile);
            pFile = NULL;
        }
    }

    /*  return a (possibly NULL) pointer the newly allocated file structure
    */
    return pFile;
}


/*-------------------------------------------------------------------
    Protected: DclFatReaderFileClose()

    Close a file which was opened with DclFatReaderFileOpen().

    Parameters:
        hFile - The file handle.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclFatReaderFileClose(
    DCLREADERFILEHANDLE hFile)
{
    DclAssert(hFile);

    if(hFile)
        FreeFileStructure(hFile->hDisk, hFile);
}


/*-------------------------------------------------------------------
    Protected: DclFatReaderFileRead()

    Read data from a file on a FAT formatted disk.  Should an error
    or EOF condition occur, this function may return fewer bytes
    than was requested.

    Parameters:
        hFile       - The file handle
        pBuffer     - A pointer to the user's I/O buffer.
        ulLength    - The number of objects to read from the stream

    Return Value:
        Returns the number of bytes read.
-------------------------------------------------------------------*/
D_UINT32 DclFatReaderFileRead(
    DCLREADERFILEHANDLE hFile,
    D_BUFFER           *pBuffer,
    D_UINT32            ulLength)
{
    DclAssert(hFile);
    DclAssert(pBuffer);

    if(!ulLength)
        return 0;

    return ReadFileData(hFile, pBuffer, (D_UINT16)ulLength);
}


/*-------------------------------------------------------------------
    Protected: DclFatReaderFileState()

    Get file state information for the specified file, and clear
    the state.

    One of the following values will be returned...
      DL_LOADER_FILESTATE_SUCCESS    - No error
      DL_LOADER_FILESTATE_EOF        - End-of-file has been reached
      DL_LOADER_FILESTATE_IO_ERROR   - Error reading from the disk

    Parameters:
        hFile       - The file handle

    Return Value:
        Returns the FILESTATE information.
-------------------------------------------------------------------*/
D_UINT16 DclFatReaderFileState(
    DCLREADERFILEHANDLE hFile)
{
    D_UINT16            uFileState;

    DclAssert(hFile);

    uFileState = hFile->uFileState;

    /*  Clear the state
    */
    hFile->uFileState = DL_LOADER_FILESTATE_SUCCESS;

    return uFileState;
}


                /*------------------------------*\
                 *                              *
                 *  Internal Helper Functions   *
                 *                              *
                \*------------------------------*/


/*-------------------------------------------------------------------
    Returns the sector address of the start of the specified FAT
    cluster.

    uClusterIndex must be a valid cluster number.

    The sector offset of the start of the disk file area is
    determined at init time and recorded in ulFileDataStart.  The
    effective cluster number times the size of the cluster is the
    byte offset within the file data area of the start of the
    cluster.

    Parameters:
        uClusterIndex - the cluster number for which to return a
                        corresponding sector.

    Return Value:
        The logical sector address of the start of the specified cluster.
-------------------------------------------------------------------*/
static D_UINT32 GetSectorOffsetOfCluster(
    DCLREADERDISK  *pDisk,
    D_UINT16        uClusterIndex)
{
    D_UINT32        ulFileOffset;

    DclAssert(pDisk);
    DclAssert(VALID_CLUSTER(uClusterIndex));

    ulFileOffset = uClusterIndex - 2;
    ulFileOffset *= pDisk->bpb.bSecPerCluster;
    ulFileOffset += pDisk->ulFileDataStart;

    return ulFileOffset;
}


/*-------------------------------------------------------------------
    Returns the value of the specified cluster entry in the FAT.

    Parameters:
       uClusterIndex is the cluster number, the FAT entry for which we
          need to read to find the location of the next cluster in the
          file.

    Notes:
       The FAT is an array, the elements of which partition the file
       data space on the disk into clusters.  A FAT chain is a linked
       list of indices into that array that controls the allocation of
       the partitioned drive space. The end of a chain is indicated by
       a magic number.  The first two entries in a FAT are reserved
       bytes that do not govern the allocation of any region of the
       disk.

    Returns:
       If the return value is a valid cluster number then it is the
       cluster number of the next cluster in the FAT chain.  If the
       return value is invalid then it means that either the end of
       the fat chain has been reached or that the FAT is damaged.
       In either case, the allocation chain has ended.
-------------------------------------------------------------------*/
static D_UINT16 FindNextCluster(
    DCLREADERDISK  *pDisk,
    D_UINT16        uClusterIndex)
{
    D_UINT16        uSectorInFAT;
    D_UINT16        uSectorWeNeed;
    D_UINT32        ulSectorOffset;
    D_UINT16        uEntryInSector;
    D_UINT16        uClusterValue;
    D_UINT16       *puEntry;

    DclAssert(pDisk);

    /*  if we get an invalid cluster (EOF, bad cluster,...), just give what we get
    */
    if(!VALID_CLUSTER(uClusterIndex))
    {
        uClusterValue = uClusterIndex;
    }
    else
    {
        puEntry = (D_UINT16 *) & pDisk->acSectorBuff[0];
        if(pDisk->bFSID == FSID_FAT16)
        {
            uSectorInFAT = uClusterIndex / FAT16_ENTRIES_PER_SECTOR;
            uEntryInSector = uClusterIndex % FAT16_ENTRIES_PER_SECTOR;
            uSectorWeNeed = uSectorInFAT + pDisk->bpb.uReservedSectors;

            if(uSectorWeNeed != pDisk->uBufferedSector)
            {
                pDisk->uBufferedSector = (D_UINT16)~0;
                ulSectorOffset = (D_UINT32)uSectorWeNeed;
                DeviceReadSectors(pDisk, ulSectorOffset, 1, pDisk->acSectorBuff);
                pDisk->uBufferedSector = uSectorWeNeed;
            }

            uClusterValue = puEntry[uEntryInSector];
            DCLNATIVE(&uClusterValue, sizeof(uClusterValue));
        }
        else
        {
            /*  byte offset into FAT is 3/2n, get FAT sector and byte
                offset within it then read in that (first) sector.
            */
            uEntryInSector = uClusterIndex + (uClusterIndex >> 1);
            uSectorInFAT = uEntryInSector / FAT_SECTORLEN;
            uSectorWeNeed = uSectorInFAT + pDisk->bpb.uReservedSectors;
            uEntryInSector %= FAT_SECTORLEN;

            if(uSectorWeNeed != pDisk->uBufferedSector)
            {
                pDisk->uBufferedSector = (D_UINT16)~0;
                ulSectorOffset = (D_UINT32)uSectorWeNeed;
                DeviceReadSectors(pDisk, ulSectorOffset, 1, pDisk->acSectorBuff);
                pDisk->uBufferedSector = uSectorWeNeed;
            }

            /*  read the (first part) of the sector value from our buffer
            */
            DclMemCpy(&uClusterValue, &pDisk->acSectorBuff[uEntryInSector],
                    sizeof(uClusterValue));

            if(uEntryInSector == (FAT_SECTORLEN - 1))
            {
                /*  We are trying to read a 12b value from the last byte of our
                    buffer and coming up short. We have to read in the next sector
                    and get the high byte of our word from there.
                */
                pDisk->uBufferedSector = (D_UINT16)~0;
                uSectorWeNeed++;
                ulSectorOffset = (D_UINT32)uSectorWeNeed;
                DeviceReadSectors(pDisk, ulSectorOffset, 1, pDisk->acSectorBuff);
                pDisk->uBufferedSector = uSectorWeNeed;
                uClusterValue &= 0x00FF;
                uClusterValue |= (puEntry[0] << 8);
            }
            DCLNATIVE(&uClusterValue, sizeof(uClusterValue));

            /*  odd cluster numbers are in the top 12bits of the word
            */
            if((uClusterIndex & 1) == 1)
            {
                uClusterValue >>= 4;
            }
            uClusterValue &= 0x0FFF;    /* make sure we have only 12 bits */

            /*  if this is invalid (EOF, bad cluster,...) then make sure we
                can detect it. We will use test for the same values regardless
                of whether we are on a FAT12 or a FAT16 drive.
            */
            if(uClusterValue > FAT12_MAXCLUSTERS)
            {
                uClusterValue |= 0xF000;
            }
        }
    }
    return uClusterValue;
}


/*-------------------------------------------------------------------
    Supporting routine for ReadNextFileSegments().  This routine calls
    FindNextCluster() and updates the DCLREADERFILE structure.

    Parameters:
       pFile is a pointer to the DCLREADERFILE structure that holds all the
          I/O access control information needed by the FAT reader.
       uSectorsPerCluster is the number of sectors in each cluster,
          which is from the disk BPB.

    Returns:
       TRUE  - if a valid cluster was found
       FALSE - if there was a problem locating the next cluster
-------------------------------------------------------------------*/
static D_BOOL LoadNextCluster(
    DCLREADERFILE  *pFile,
    D_UINT16        uSectorsPerCluster)
{
    D_UINT16        uClusterValue;      /* validate cluster values before storing them */

    uClusterValue = FindNextCluster(pFile->hDisk, pFile->uCurrentCluster);
    pFile->uCurrentCluster = uClusterValue;
    if(VALID_CLUSTER(uClusterValue))
    {
        /*  Cluster is valid.  Update the other fields and report success.
        */
        pFile->ulCurrentSectorOffset = GetSectorOffsetOfCluster(pFile->hDisk, uClusterValue);
        pFile->uSectorsLeftInCluster = uSectorsPerCluster;
        return TRUE;
    }
    return FALSE;               /* error in FAT chain or at EOF */
}


/*-------------------------------------------------------------------
    Sequentially read the next uBytesToRead of whole sectors from a file.
    Reads could overlap cluster boundaries on entry and exit. We
    directly read as many whole clusters at a time as we can in between.

    Parameters:
       pFile is a pointer to the FILINFO structure to use.
       pcBuffer is a pointer to the user's I/O buffer.
       uBytesToRead is the number of bytes to read from the file.
             This must be an integer multiple of FAT_SECTORLEN.

    Notes:
       Max value of uBytesToRead is limited by value to
       MAX_BYTES_PER_READ_CALL by the _read() routine.  The
       ulCurrentSectorOffset value in the FILE struct must be initially
       set by _open().

    Returns:
       The number of bytes successfully read. If bytes returned is
       less than requested then there was an IO error or an error in
       the FAT chain or an attempt was made to read past the end of
       the allocated file.
-------------------------------------------------------------------*/
static D_UINT16 ReadNextFileSegments(
    DCLREADERFILE  *pFile,
    D_UCHAR        *pcBuffer,
    D_UINT16        uBytesToRead)
{
    D_UINT16        uSectorsLeftToRead;
    D_UINT16        uTotalSectorsRead;
    D_UINT16        uSectorsPerCluster; /* make a local copy */
    D_UINT32        ulSectorCount;
    DCLSTATUS       DclStatus;

    DclAssert((uBytesToRead % FAT_SECTORLEN) == 0);

    /*  Convert bytes to sectors, there can be no remainder.
        Try to read all the sectors they asked for
    */
    uSectorsPerCluster = (D_UINT16) (pFile->hDisk->bpb.bSecPerCluster);
    uSectorsLeftToRead = uBytesToRead / FAT_SECTORLEN;
    uTotalSectorsRead = 0;

    /*  Is the FAT chain valid?
    */
    if(!VALID_CLUSTER(pFile->uCurrentCluster))
    {
        goto DONE;              /* IO or FAT or DIR error */
    }

    /*  we may have the end of a cluster to read first
    */
    if(uSectorsLeftToRead > pFile->uSectorsLeftInCluster)
    {
        if(pFile->uSectorsLeftInCluster > 0)
        {
            ulSectorCount = pFile->uSectorsLeftInCluster;
            if( pcBuffer == pFile->hDisk->acSectorBuff )
            {
                pFile->hDisk->uBufferedSector = (D_UINT16)~0;
            }
            DclStatus = DeviceReadSectors(pFile->hDisk, pFile->ulCurrentSectorOffset, ulSectorCount, pcBuffer);
            if(DclStatus != DCLSTAT_SUCCESS)
            {
                goto DONE;      /* IO error */
            }
            if( pcBuffer == pFile->hDisk->acSectorBuff )
            {
                pFile->hDisk->uBufferedSector = (D_UINT16)(pFile->ulCurrentSectorOffset);
            }
            pcBuffer += ulSectorCount * pFile->hDisk->ulBlockSize;
            uTotalSectorsRead += pFile->uSectorsLeftInCluster;
            uSectorsLeftToRead -= pFile->uSectorsLeftInCluster;
            pFile->uSectorsLeftInCluster = 0;
        }

        if(uSectorsLeftToRead > 0)
        {
            /*  we have read the last of the current cluster, move on
                to the next cluster in the FAT chain.
            */
            if(!LoadNextCluster(pFile, uSectorsPerCluster))
            {
                goto DONE;      /* IO or FAT or DIR error */
            }
        }
    }

    /*  now read as many whole clusters as we can, we can't be sure
        that the file clusters are allocated sequentially so read them
        one at a time.
    */
    while(uSectorsLeftToRead >= uSectorsPerCluster)
    {
        DclAssert(pFile->uSectorsLeftInCluster == uSectorsPerCluster);

        ulSectorCount = uSectorsPerCluster;
        if( pcBuffer == pFile->hDisk->acSectorBuff )
        {
            pFile->hDisk->uBufferedSector = (D_UINT16)~0;
        }
        DclStatus = DeviceReadSectors(pFile->hDisk, pFile->ulCurrentSectorOffset, ulSectorCount, pcBuffer);
        if(DclStatus != DCLSTAT_SUCCESS)
        {
            goto DONE;          /* IO error */
        }
        if( pcBuffer == pFile->hDisk->acSectorBuff )
        {
            pFile->hDisk->uBufferedSector = (D_UINT16)(pFile->ulCurrentSectorOffset);
        }
        pcBuffer  += ulSectorCount * pFile->hDisk->ulBlockSize;
        uTotalSectorsRead += uSectorsPerCluster;
        uSectorsLeftToRead -= uSectorsPerCluster;
        pFile->uSectorsLeftInCluster = 0;

        if(uSectorsLeftToRead > 0)
        {
            /*  we just read an entire cluster, move on to the next one
            */
            if(!LoadNextCluster(pFile, uSectorsPerCluster))
            {
                goto DONE;      /* IO or FAT or DIR error */
            }
        }
    }

    /*  Either we now have to read the starting sector(s) of a new cluster or
        we may be have been called to read no more than the sectors remaining
        in the current cluster. In either case, control variables will have
        been set up correctly before hand.
    */
    if(uSectorsLeftToRead > 0)
    {
        DclAssert(uSectorsLeftToRead <= pFile->uSectorsLeftInCluster);

        ulSectorCount = uSectorsLeftToRead;
        if( pcBuffer == pFile->hDisk->acSectorBuff )
        {
            pFile->hDisk->uBufferedSector = (D_UINT16)~0;
        }
        DclStatus = DeviceReadSectors(pFile->hDisk, pFile->ulCurrentSectorOffset, ulSectorCount, pcBuffer);
        if(DclStatus != DCLSTAT_SUCCESS)
        {
            goto DONE;          /* IO error */
        }
        if( pcBuffer == pFile->hDisk->acSectorBuff )
        {
            pFile->hDisk->uBufferedSector = (D_UINT16)(pFile->ulCurrentSectorOffset);
        }
        pFile->ulCurrentSectorOffset += ulSectorCount;
        uTotalSectorsRead += uSectorsLeftToRead;
        pFile->uSectorsLeftInCluster -= uSectorsLeftToRead;
    }

    /*  return count of bytes successfully read
    */
  DONE:
    return uTotalSectorsRead * FAT_SECTORLEN;
}


/*-------------------------------------------------------------------
    Sequentially reads the next uBytesToRead bytes from a file into the
    specified user buffer.

    Parameters:
       pFile is a pointer to the DCLREADERFILE structure to use.
       pBuffer is a pointer to the user's I/O buffer.
       uBytesToRead is the number of bytes to read from the file

    Notes:
       The I/O buffer in the DCLREADERFILE structure is used to increase I/O
       efficiency for small reads and simplify upper level code as
       well as maintaining sector alignments of read operations to
       lower level code.


    Returns:
       The number of bytes successfully read.
-------------------------------------------------------------------*/
static D_UINT16 ReadFileData(
    DCLREADERFILE  *pFile,
    D_UCHAR        *pBuffer,
    D_UINT16        uBytesToRead)
{
    D_UINT16        uBytesLeftToRead;
    D_UINT16        uTotalBytesRead;
    D_UINT16        uBytesToReadNow;
    D_UINT16        uBytesJustRead;
    D_UINT16        uFileBufferOffset;

    /*  We've done nothing yet.
        If the call spans file segment boundaries, read the next segment(s)
    */
    uTotalBytesRead = 0;
    uBytesLeftToRead = uBytesToRead;


    /*  This is required to catch the case where the file pointer is already
        at or past the EOF.  We avoid adjusting the file position
        inappropriately but are still setting the EOF indicator and returning
        zero bytes read for this stream.
    */
    if(pFile->ulCurrentFilePosition >= pFile->ulFileSize)
    {
        pFile->uFileState = DL_LOADER_FILESTATE_EOF;
        goto DONE;
    }

    /*  The value of the current file pointer is now strictly less than the
        extent of the file being read.  Limit the read request to the actual
        number of bytes remaining in the file.  We must do a read since the
        number of bytes remaining is non-zero.  Even though we're truncating
        our read to not pass the EOF, set the EOF flag because we cannot read
        everything requested.
    */
    if(pFile->ulCurrentFilePosition + uBytesLeftToRead > pFile->ulFileSize)
    {
        uBytesLeftToRead = (D_UINT16)(pFile->ulFileSize - pFile->ulCurrentFilePosition);
        pFile->uFileState = DL_LOADER_FILESTATE_EOF;
    }

    while(uBytesLeftToRead > pFile->uBytesLeftInBuffer)
    {
        /*  Move what's left in the file buffer to user space and update the
            counters.  Assume that MEMMOVE can't fail.
        */
        if(pFile->uBytesLeftInBuffer > 0)
        {
            uBytesToReadNow = pFile->uBytesLeftInBuffer;
            uFileBufferOffset = FAT_SECTORLEN - pFile->uBytesLeftInBuffer;
            DclMemCpy(pBuffer, &pFile->acFileBuff[uFileBufferOffset], uBytesToReadNow);
            uBytesJustRead = uBytesToReadNow;
            pBuffer += uBytesJustRead;
            uTotalBytesRead += uBytesJustRead;
            uBytesLeftToRead -= uBytesJustRead;
            pFile->uBytesLeftInBuffer = 0;
        }

        /*  as long as there are full buffers to read
        */
        while(uBytesLeftToRead >= FAT_SECTORLEN)
        {
            /*  transfer up to our max sized block of bytes into user buffer
                and update variables
            */
            uBytesToReadNow = (uBytesLeftToRead / FAT_SECTORLEN) * FAT_SECTORLEN;
            uBytesJustRead = ReadNextFileSegments(pFile, pBuffer, uBytesToReadNow);
            DclAssert(uBytesJustRead <= uBytesToReadNow);
            pBuffer += uBytesJustRead;
            uTotalBytesRead += uBytesJustRead;
            uBytesLeftToRead -= uBytesJustRead;
            if(uBytesJustRead < uBytesToReadNow)
            {
                /*  IO or FAT or DIR error
                */
                pFile->uFileState = DL_LOADER_FILESTATE_IO_ERROR;
                goto DONE;
            }
        }                       /* while(uTotalBytesToRead >= FAT_SECTORLEN) */

        /*  if a partial buffer remains to be transferred, refill the buffer
        */
        if(uBytesLeftToRead > 0)
        {
            uBytesToReadNow = FAT_SECTORLEN;
            uBytesJustRead = ReadNextFileSegments(pFile,
                                                  &pFile->acFileBuff[0],
                                                  uBytesToReadNow);
            DclAssert(uBytesJustRead <= uBytesToReadNow);
            pFile->uBytesLeftInBuffer = uBytesJustRead;
            if(uBytesJustRead < uBytesToReadNow)
            {
                /*  VBF or FAT or DIR error
                */
                pFile->uFileState = DL_LOADER_FILESTATE_IO_ERROR;
                goto DONE;
            }
        }
    }

    /*  Either we now have to read the start of a new buffer or
        we may be have been called to read no more than the bytes remaining
        in the current buffer. In either case, control variables will have
        been set up correctly before hand.
    */
    if(uBytesLeftToRead > 0)
    {
        DclAssert(uBytesLeftToRead <= pFile->uBytesLeftInBuffer);

        /*  move what's left to read from the file buffer to user space
        */
        uFileBufferOffset = FAT_SECTORLEN - pFile->uBytesLeftInBuffer;
        pFile->uBytesLeftInBuffer -= uBytesLeftToRead;
        uTotalBytesRead += uBytesLeftToRead;
        DclMemCpy(pBuffer, &pFile->acFileBuff[uFileBufferOffset], uBytesLeftToRead);
    }

    /*  update file position and return total bytes read
    */
  DONE:
    pFile->ulCurrentFilePosition += uTotalBytesRead;

    return uTotalBytesRead;
}


/*-------------------------------------------------------------------
    Allocates a DCLREADERFILE structure

    Parameters:
       none

    Notes:
       This routine simply manipulates a pair of linked lists
       that control the classification (free or used) of elements
       of a static array of file structures.

    Returns:
       A pointer to the next available file control structure or NULL
       if there are none free.
-------------------------------------------------------------------*/
static DCLREADERFILE * AllocateFileStructure(
    DCLREADERDISK  *pDisk)
{
    DCLREADERFILE  *pFileNew;

    /*  get the next file structure from the free list (could be NULL).
    */
    pFileNew = pDisk->pFIFreeList;
    if(pFileNew != NULL)
    {
        /*  remove this file structure from the free list
        */
        pDisk->pFIFreeList = pFileNew->pFINext;

        /*  add file structure to the used list and return it
        */
        pFileNew->pFINext = pDisk->pFIUsedList;
        pDisk->pFIUsedList = pFileNew;
    }

    /*  return new (possibly NULL) file
    */
    return pFileNew;
}


/*-------------------------------------------------------------------
    Frees an allocated DCLREADERFILE structure

    Parameters:
       Pointer to the file struct to free

    Notes:
       This routine simply manipulates a pair of linked lists
       that control the classification (free or used) of elements
       of a static array of file structures.

    Returns:
       Returns TRUE on success or FALSE if the file struc was not
       found in list of allocated structs.
-------------------------------------------------------------------*/
static D_BOOL FreeFileStructure(
    DCLREADERDISK  *pDisk,
    DCLREADERFILE  *pFileToFree)
{
    DCLREADERFILE  *pFileCurrent;

    /*  allow a special case at the start of the list
    */
    if(pDisk->pFIUsedList == pFileToFree)
    {
        /*  unlink this file struc from the used list
        */
        pDisk->pFIUsedList = pFileToFree->pFINext;
    }
    else
    {
        pFileCurrent = pDisk->pFIUsedList;
        while(pFileCurrent->pFINext != NULL)
        {
            if(pFileCurrent->pFINext == pFileToFree)
            {
                /*  unlink file struc from used list
                */
                pFileCurrent->pFINext = pFileToFree->pFINext;
                break;
            }
            else
            {
                pFileCurrent = pFileCurrent->pFINext;
            }
        }
        return FALSE;
    }

    /*  add file struc to head of free list
    */
    pFileToFree->pFINext = pDisk->pFIFreeList;
    pDisk->pFIFreeList = pFileToFree;

    return TRUE;
}


/*-------------------------------------------------------------------
    Expands an 8.3 "dot name"  into "expanded" form.

    Parameters:
       Pointer to the 8.3 Cstring (ASCIIZ) file name to find ("nk.bin").

    Notes:
       The acName and acExt fields of a directory entry contain the
       file name in expanded form.  In expanded form the name and
       extension are stored separately, left justified, upper cased
       and padded to the right with spaces.  For instance, the dot
       names "nk.bin" and "file.c" appear in expanded form as
       "NK      BIN" and "FILE    C  ".

    Returns:
       Returns TRUE on success or FALSE if the file name was not found
          invalid.
-------------------------------------------------------------------*/
static D_BOOL ExpandDotName(
    const char *pszFileName,
    D_UCHAR    *pBuffer)
{
    D_INT16     i, j;

#define NUL          '\0'
#define SPACE        ' '
#define DOT          '.'

    /*  Name has to have at least one char
    */
    if((*pszFileName == DOT) || (*pszFileName == NUL))
    {
        return FALSE;
    }

    /*  MEMSET the destination array to SPACE characters
    */
    for(i = 0; i < 11; i++)
    {
        pBuffer[i] = SPACE;
    }

    /*  Copy the name part of the 8.3 name
    */
    for(i = 0; i < 8; i++)
    {
        if(pszFileName[i] == DOT)
        {
            break;
        }
        if(pszFileName[i] == NUL)
        {
            return TRUE;
        }
        pBuffer[i] = pszFileName[i];

        /*  uppercase the name if it's a lower case alpha
        */
        if((pBuffer[i] >= 'a') && (pBuffer[i] <= 'z'))
        {
            pBuffer[i] &= ~('a' - 'A');
        }
    }

    /*  We should have found a DOT, NUL or eight name chars.
        NUL case already has returned so pszFilename[i] better be a DOT
        or the file name is invalid and we are done.
    */
    if(pszFileName[i++] != DOT)
    {
        return FALSE;
    }

    /*  Copy the 3 char file name extension, if any. Start at offset 8
        in the expanded name array
    */
    for(j = 8; j < 11; i++, j++)
    {
        if(pszFileName[i] == NUL)
        {
            return TRUE;
        }
        pBuffer[j] = pszFileName[i];

        /*  uppercase the name if it's a lower case alpha
        */
        if((pBuffer[j] >= 'a') && (pBuffer[j] <= 'z'))
        {
            pBuffer[j] &= ~('a' - 'A');
        }
    }
    return (pszFileName[i] == NUL);
}


/*-------------------------------------------------------------------
    Compares two file names in expanded form.  To be equal, all eleven
    chars must match.

    Parameters:
       The two enames to compare.

    Returns:
       TRUE if names match else FALSE.
-------------------------------------------------------------------*/
static D_BOOL NamesMatch(
    D_UCHAR    *pNameOne,
    D_UCHAR    *pNameTwo)
{
    D_INT16     i;

    for(i = 0; i < 11; i++)
    {
        if(pNameOne[i] != pNameTwo[i])
            return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Tries to find the DIR entry struct matching the ASCIIZ file name
    in the root directory of the drive.  If found, the file I/O
    control structure is initialized to the values contained in the
    files directory entry.

    Parameters:
       pszFileName - A pointer to the ASCIIZ name of the file
       pFile       - A pointer to the FILE struct to initialize

    Notes:
       Only 8.3 file names in the root dir of FAT 12 or FAT 16
       drives are considered.

    Returns:
       TRUE if a matching DIR entry was found, else FALSE
-------------------------------------------------------------------*/
static D_BOOL FindFileEntry(
    const char         *pszFileName,
    DCLREADERFILE      *pFile)
{
    D_UINT32            ulDirSectorOffset;
    D_UINT16            uCurDirSector;
    D_UINT16            uCurDirInSector;
    D_BUFFER           *pRawDirEntry;
    D_UINT16            uClusterValue;
    D_UCHAR             szName[DL_LOADER_FILENAME_MAX];
    DCLSTATUS           DclStatus;

    if(!ExpandDotName(pszFileName, szName))
        return FALSE;

    /*  for each sector in the root directory
    */
    ulDirSectorOffset = pFile->hDisk->ulRootDirStart;
    for(uCurDirSector = 0;
        uCurDirSector < pFile->hDisk->uRootDirSectors;
        uCurDirSector++)
    {
        /*  read the DIR entries
        */
        pFile->hDisk->uBufferedSector = (D_UINT16)~0;
        DclStatus = DeviceReadSectors(pFile->hDisk, ulDirSectorOffset, 1, &pFile->hDisk->acSectorBuff[0]);
        if(DclStatus != DCLSTAT_SUCCESS)
            break;

        /*  update sector in scratch buffer
        */
        pFile->hDisk->uBufferedSector = (D_UINT16)(ulDirSectorOffset);

        /*  update offset of next read
        */
        ulDirSectorOffset += FAT_SECTORLEN;

        /*  get pointer to first DIR in scratch buffer
        */
        pRawDirEntry = &pFile->hDisk->acSectorBuff[0];

        /*  for each DIR entry in this sector
        */
        for(uCurDirInSector = 0;
            uCurDirInSector < DIR_ENTRIES_PER_SECTOR;
            uCurDirInSector++, pRawDirEntry += FAT_DIRENTRYSIZE)
        {
            if(pRawDirEntry[DIR_ATTR_OFF] & (DIR_ATTR_VOLUME | DIR_ATTR_SUBDIR | DIR_ATTR_UNKNOWN))
            {
                /*  found either a volume label, a sub-directry entry, an
                    LFN file name entry or something we know NOTHING about.
                    In any case we don't deal it.
                */
                continue;
            }

            if(NamesMatch(szName, &pRawDirEntry[DIR_NAME_OFF]))
            {
                /*  We found it. Get start and size of file.
                */
                DCLLE2NE(&pFile->ulFileSize, &pRawDirEntry[DIR_SIZE_OFF], sizeof(pFile->ulFileSize));
                DCLLE2NE(&uClusterValue, &pRawDirEntry[DIR_START_OFF], sizeof(uClusterValue));

                if((pFile->hDisk->bFSID == FSID_FAT12) && (uClusterValue > FAT12_MAXCLUSTERS))
                {
                    /*  if this is a bad cluster value, make sure we detect it
                    */
                    uClusterValue |= 0xF000;
                }

                if(!VALID_CLUSTER(uClusterValue))
                {
                    /*  the sought after DIR entry is unusable, give up now
                    */
                    return FALSE;
                }

                pFile->ulCurrentFilePosition = 0;
                pFile->uStartingCluster = uClusterValue;
                pFile->uCurrentCluster = uClusterValue;
                pFile->ulCurrentSectorOffset = GetSectorOffsetOfCluster(pFile->hDisk, uClusterValue);
                pFile->uSectorsLeftInCluster = pFile->hDisk->bpb.bSecPerCluster;
                pFile->uBytesLeftInBuffer = 0;
                pFile->uFileState = DL_LOADER_FILESTATE_SUCCESS;

                return TRUE;
            }
        }
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    DeviceReadSectors()

    Description
        This function reads sectors from the disk.

    Parameters
        pDisk           - The disk instance to read from.
        ulStartSector   - The sector to start reading from.
        ulCount         - The number of sectors to read.
        pBuffer         - The buffer to receive the data read.

    Return Value
        Returns the number of sectors read.
-------------------------------------------------------------------*/
static DCLSTATUS DeviceReadSectors(
    DCLREADERDISK  *pDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulCount,
    D_UCHAR        *pBuffer)
{
    DCLSTATUS       DclStatus;


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

