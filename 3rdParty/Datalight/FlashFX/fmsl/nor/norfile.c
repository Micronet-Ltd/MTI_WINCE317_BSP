/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This FIM emulates NOR flash using C Standard file I/O functions.  It is
    typically only used for testing and debugging purposes.

    The following configuration option settings are used:

    FFXOPT_FIM_FILENAME       - This is an optional setting that is a pointer
                                to a null-terminated filename.  If this value
                                is not set, a filename in the form "FFXDEVn.DAT"
                                will be used, where 'n' is the device number.

    FFXOPT_DEVICE_BLOCKSIZE   - This is an optional setting that defines the
                                erase zone size to use.  If this value is not
                                supplied, a 256KB erase zone size will be used.

    For a given project, a customized fhoption.c module may be used to provide
    project specific values for the settings described above.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: norfile.c $
    Revision 1.24  2012/04/12 14:35:50Z  johnb
    Removed use of NUM_CHIPS.
    Revision 1.23  2012/03/01 21:00:25Z  johnb
    Set device size if file already exists.
    Revision 1.22  2011/11/17 23:48:45Z  garyp
    Fixed error handling to not try to do file operations with an invalid handle.
    Don't use the 'b' mode when opening/creating files.
    Revision 1.21  2010/04/29 00:04:24Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.20  2009/12/10 19:45:19Z  keithg
    Added support for obtaining the chip size, device type, page size,
    and determining the number of chips based on project option code.
    Revision 1.19  2009/07/31 19:24:43Z  garyp
    Merged from the v4.0 branch.  Include fxdriverfwapi.h.
    Revision 1.18  2009/04/06 14:15:18Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.17  2009/01/19 04:55:11Z  keithg
    Limited formated DclSNPrintf() to something reasonable to prevent
    possible stack corruption.
    Revision 1.16  2008/07/24 00:02:53Z  keithg
    Added support for Sibley style flash.
    Revision 1.15  2008/04/07 22:44:39Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.14  2008/04/03 23:56:22Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.13  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.12  2007/08/03 00:12:54Z  timothyj
    Changed units of reserved space and maximum size to be in terms
    of KB instead of bytes.
    Revision 1.11  2007/03/15 01:54:56Z  Garyp
    Modified to no longer use DclOsFilePutChar().
    Revision 1.10  2006/08/31 18:48:17Z  pauli
    Removed reference to EXTMEDIAINFO.ulWindowSize.
    Revision 1.9  2006/08/28 22:08:44Z  pauli
    Updated comments.
    Revision 1.8  2006/07/30 00:53:35Z  Garyp
    Conditioned some code to properly handle NUM_CHIPS == 1.
    Revision 1.7  2006/06/07 01:57:14Z  Pauli
    Updated boot block handling to work with non boot block parts.
    Revision 1.6  2006/05/20 20:56:16Z  Garyp
    Added support for emulating boot blocks, as well as emulating multiple
    chips.
    Revision 1.5  2006/04/30 15:48:49Z  Garyp
    Modified to use the standard DCL file I/O interface.
    Revision 1.4  2006/03/06 22:15:24Z  Garyp
    Modified to use standard device settings.
    Revision 1.3  2006/02/11 00:12:59Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.2  2006/02/08 18:36:09Z  Garyp
    Updated to use new FfxHookOptionGet() parameters.  Updated debugging code.
    Revision 1.1  2005/10/25 23:11:36Z  Pauli
    Initial revision
    Revision 1.2  2005/10/26 00:11:36Z  Garyp
    Updated to use some renamed fields.
    Revision 1.1  2005/10/14 03:08:04Z  Garyp
    Initial revision
    Revision 1.12  2005/05/06 21:04:04Z  garyp
    Removed some unnecessary prototypes and moved the FIMDEVICE declaration to
    the end of the file.
    Revision 1.11  2004/12/30 17:32:49Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.10  2004/12/01 23:44:38Z  GaryP
    Minor error handling cleanup.
    Revision 1.9  2004/11/26 21:53:24Z  GaryP
    Documentation fixes.
    Revision 1.8  2004/10/31 01:00:30Z  GaryP
    Modified to build cleanly with MSVC6 tools.
    Revision 1.7  2004/10/25 07:32:04Z  GaryP
    Modified to use default values for the filename, size, and erase zone
    size if they are not specified.  Use the existing file size if it already
    exists, and if not create the file at the specified length.  Updated to
    build cleanly with BC45 and MSVC6 tools.
    Revision 1.6  2004/10/05 19:53:27Z  billr
    Fix warnings.
    Revision 1.5  2004/09/29 19:15:21Z  billr
    Remove project-specific code. Use FfxHookOptionGet().
    Revision 1.4  2004/08/05 16:36:30Z  billr
    Move optional functions to the end of FIMDEVICE.
    Revision 1.3  2004/08/04 21:57:48Z  billr
    Eliminate ulInterleaved in ExtndMediaInfo, no longer used or needed.
    Revision 1.2  2004/08/04 21:26:22Z  billr
    Eliminate ulFimFlags in ExtndMediaInfo, no longer used or needed.
    Revision 1.1  2004/07/27 20:27:54Z  BillR
    Initial revision
    ---------------------
    Bill Roman 2004-07-16
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriverfwapi.h>
#include <fxoption.h>
#include <fimdev.h>
#include "nor.h"

/*-------------------------------------------------------------------
    Normally, our NORFILE FIM is used to Emulate standard NOR flash, but we now
    have some customers who are using Sibley, and wish to use simulation for
    image analysis, and overalll behavioral testing.  If this support is enabled, we will
    include the special functions ReadControl() and WriteControl()
----------------------------------------------------------------------*/
#define NORFILE_EMULATE_ISWF  FALSE /* Should be FALSE for checkin*/

#if NORFILE_EMULATE_ISWF

#define FLASH_PAGE_SIZE                1024

#define FLASH_CONTROL_VALID_SIZE       16 /* valid size in Bytes */
#define FLASH_CONTROL_INVALID_SIZE     16 /* invalid size in Bytes */

#define PAGE_MASK       0xFFFFFC00 /* 22 bits for page address */
#define OFFSET_MASK     0x000003FF /* 10 bits for offset (bits 0-9, bit 9 is always 0 for 1x16) */

#endif  /* NORFILE_EMULATE_ISWF */

/*-------------------------------------------------------------------
    Default FIM settings that are used if they are not specified as
    run-time configuration options.
-------------------------------------------------------------------*/
#define DEFAULT_TOTALSIZEKB   (8192UL)
#define DEFAULT_ERASEZONESIZE (256*1024UL)



/*-------------------------------------------------------------------
    Boot block configuration
-------------------------------------------------------------------*/
#define EMULATE_BOOT_BLOCKS  TRUE   /* should be TRUE for checkin */

#if EMULATE_BOOT_BLOCKS
  #define BOOTBLOCK_DIVISOR   (8)   /* a regular block contains n boot blocks */
  #define BOOTBLOCKS_LOW      (1)   /* normal blocks at the start which are boot blocks (1 max) */
  #define BOOTBLOCKS_HIGH     (1)   /* normal blocks at the end which are boot blocks (1 max) */
#endif


typedef struct tagFIMEXTRA
{
    DCLFSFILEHANDLE hFile;
    char           *pszFileName;
  #if EMULATE_BOOT_BLOCKS
    D_UINT16        uHighBootBlockStart;
    unsigned        fInBootBlocks : 1;
  #endif
} FIMEXTRA;

#define PUTCHAR(c, h)                           \
{                                               \
    unsigned char chr = (c);                    \
    DclOsFileWrite(&chr, sizeof(chr), 1, h);    \
}


/*-------------------------------------------------------------------
    Public: Mount()

    Parameters:
       pEMI - A pointer to the ExtndMediaInfo structure to use.

    Notes:
       Attempts to open the file

    Return Value:
       TRUE  - If the media is supported, the full structure is updated.
       FALSE - If the media is NOT supported
-------------------------------------------------------------------*/
static D_BOOL Mount(
    PEXTMEDIAINFO   pEMI)
{
    PFIMEXTRA       pFim;
    #define         FILENAME_LENGTH  16
    char            szFilename[FILENAME_LENGTH];
    FFXDEVSETTINGS  DevSettings;
    DCLSTATUS       dclStat;

    DclAssert(pEMI);

    pFim = DclMemAllocZero(sizeof(*pEMI->pFimExtra));
    if(!pFim)
    {
        DclError();
        return FALSE;
    }

    if(!FfxHookOptionGet(FFXOPT_FIM_FILENAME, pEMI->hDev,
                         &pFim->pszFileName, sizeof pFim->pszFileName))
    {
        DclSNPrintf(szFilename, FILENAME_LENGTH, "FFXDEV%U.DAT", pEMI->uDriveNum);
        pFim->pszFileName = &szFilename[0];
    }

    if(!FfxHookOptionGet(FFXOPT_DEVICE_SETTINGS, pEMI->hDev, &DevSettings, sizeof DevSettings))
    {
        DevSettings.ulReservedBottomKB = 0;
        DevSettings.ulReservedTopKB = 0;
        DevSettings.ulMaxArraySizeKB = DEFAULT_TOTALSIZEKB;
    }

    if(DevSettings.ulMaxArraySizeKB == D_UINT32_MAX)
        DevSettings.ulMaxArraySizeKB = DEFAULT_TOTALSIZEKB;

    if(!FfxHookOptionGet(FFXOPT_DEVICE_TYPE, pEMI->hDev, &pEMI->uDeviceType, sizeof(pEMI->uDeviceType)))
        pEMI->uDeviceType   = DEV_NOR;

    if(!FfxHookOptionGet(FFXOPT_DEVICE_BLOCKSIZE, pEMI->hDev, &pEMI->ulEraseZoneSize, sizeof(pEMI->ulEraseZoneSize)))
        pEMI->ulEraseZoneSize = DEFAULT_ERASEZONESIZE;

    if(!FfxHookOptionGet(FFXOPT_DEVICE_PAGESIZE, pEMI->hDev, &pEMI->uPageSize, sizeof(pEMI->uPageSize)))
        pEMI->uPageSize = 0;

    if(!FfxHookOptionGet(FFXOPT_DEVICE_CHIPSIZE, pEMI->hDev, &pEMI->ulDeviceSize, sizeof(pEMI->ulDeviceSize)))
        pEMI->ulDeviceSize = 0;

    if(DevSettings.ulMaxArraySizeKB == D_UINT32_MAX)
        DevSettings.ulMaxArraySizeKB = DEFAULT_TOTALSIZEKB;

    /*  Try to open an existing file
    */
    dclStat = DclFsFileOpen(pFim->pszFileName, "r+", &pFim->hFile);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  Behave like real flash -- regardless how the project
            may be configured, the real flash length supersedes
            the configuration options.

            Get the file length, and use that as the emulated flash length.
        */
        DclOsFileSeek(pFim->hFile, 0, DCLFSFILESEEK_END);
        if (pEMI->ulDeviceSize == 0)
        {
            pEMI->ulDeviceSize = DclOsFileTell(pFim->hFile);
            DclOsFileSeek(pFim->hFile, 0, DCLFSFILESEEK_SET);
        }

        DclPrintf("FFX: NORFILE: Using existing file length of %lUKB\n", pEMI->ulDeviceSize / 1024L);
    }
    else
    {
        /*  The file does not exist, so create it and use the length
            specified by the configuration parameters.
        */
        dclStat = DclFsFileOpen(pFim->pszFileName, "w+", &pFim->hFile);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            pEMI->ulDeviceSize = DevSettings.ulMaxArraySizeKB * 1024UL;

            /*  Force the file to the length we want...
            */
            DclOsFileSeek(pFim->hFile, pEMI->ulDeviceSize - 1, DCLFSFILESEEK_SET);
            PUTCHAR((unsigned char)~0, pFim->hFile);
        }
        else
        {
            DclPrintf("FFX: NORFILE: Unable to create file '%s', status=%lX\n", 
                pFim->pszFileName, dclStat);
        }
    }

    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  If we didnt get a device size, make one up
        */
        if(!pEMI->ulDeviceSize)
            pEMI->ulDeviceSize = DclOsFileTell(pFim->hFile);
        DclAssert(pEMI->ulDeviceSize);
        DclAssert(pEMI->ulEraseZoneSize);

        /*  Truncate the size to an integral number of erase zones.
        */
        pEMI->ulDeviceSize -= pEMI->ulDeviceSize % pEMI->ulEraseZoneSize;
        pEMI->ulTotalSize   = pEMI->ulDeviceSize;
        pEMI->uSpareSize    = 0;
        pEMI->pFimExtra     = pFim;

        if( (pEMI->ulDeviceSize % pEMI->ulEraseZoneSize) != 0)
        {
            DclPrintf("FFX: NORFILE: Device geometry is not valid\n");
            goto ErrorCleanup;
        }

        DclPrintf("FFX: NORFILE: File='%s' DevSize=%lUKB ChipSize=%lUKB BlockSize=%lUKB\n",
            pFim->pszFileName,
            pEMI->ulTotalSize / 1024UL,
            pEMI->ulDeviceSize / 1024UL,
            pEMI->ulEraseZoneSize / 1024UL);

      #if EMULATE_BOOT_BLOCKS
        pEMI->ulBootBlockSize = pEMI->ulEraseZoneSize / BOOTBLOCK_DIVISOR;
        pEMI->uBootBlockCountLow = BOOTBLOCKS_LOW * BOOTBLOCK_DIVISOR;
        pEMI->uBootBlockCountHigh = BOOTBLOCKS_HIGH * BOOTBLOCK_DIVISOR;

        if(pEMI->uBootBlockCountHigh)
        {
            pFim->uHighBootBlockStart = (D_UINT16)
                ((pEMI->ulDeviceSize / pEMI->ulBootBlockSize) - pEMI->uBootBlockCountHigh);

            DclAssert(pFim->uHighBootBlockStart != D_UINT16_MAX);
        }

        DclPrintf("FFX: NORFILE: BootBlockSize=%lUKB Low=%U High=%U\n",
            pEMI->ulBootBlockSize / 1024UL, pEMI->uBootBlockCountLow, pEMI->uBootBlockCountHigh);
      #endif

        return TRUE;
    }

  ErrorCleanup:

    DclError();

    DclMemFree(pFim);

    return FALSE;
}


/*-------------------------------------------------------------------
    Public: Unmount()

    Parameters:
        pEMI - A pointer to the ExtndMediaInfo structure to use.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static void Unmount(
    PEXTMEDIAINFO pEMI)
{
    DclAssert(pEMI);
    DclAssert(pEMI->pFimExtra);

    DclFsFileClose(pEMI->pFimExtra->hFile);
    DclMemFree(pEMI->pFimExtra);
    pEMI->pFimExtra = NULL;
}


/*-------------------------------------------------------------------
    Public: Read()

    Parameters:
       pEMI    - A pointer to the ExtndMediaInfo structure to use.
       ulStart - Starting offset in bytes to begin the access
       uLength - Number of bytes to transfer
       pBuffer - Pointer to client supplied transfer area

    Notes:
       Read a given number of bytes of data from the media.

    Return Value:
       TRUE  - If the access was successful
       FALSE - If the access failed
-------------------------------------------------------------------*/
static D_BOOL Read(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    DclAssert(pEMI);
    DclAssert(pEMI->pFimExtra);
    DclAssert(pBuffer);
    DclAssert(ulStart < pEMI->ulTotalSize);
    DclAssert(uLength <= pEMI->ulTotalSize - ulStart);

    if(DclOsFileSeek(pEMI->pFimExtra->hFile, ulStart, DCLFSFILESEEK_SET) == 0
       && DclOsFileRead(pBuffer, uLength, 1, pEMI->pFimExtra->hFile) == 1)
    {
        return TRUE;
    }

    FFXPRINTF(1, ("FFX: NORFILE: Error reading from file\n"));

    return FALSE;
}


/*-------------------------------------------------------------------
    Public: Write()

    Parameters:
       pEMI    - A pointer to the ExtndMediaInfo structure to use.
       ulStart - Starting offset in bytes to begin the access
       uLength - Number of bytes to transfer
       pBuffer - Pointer to client supplied transfer area

    Notes:
       Writes a given number of bytes of data out to the media.
       It does not return until the data is programmed and verified.

    Return Value:
       TRUE  - If the access was successful
       FALSE - If the access failed
-------------------------------------------------------------------*/
static D_BOOL Write(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    #define MERGE_SIZE (128)
    D_UCHAR         mergebuf[MERGE_SIZE];
    D_UCHAR        *pData = pBuffer;

    DclAssert(pEMI);
    DclAssert(pEMI->pFimExtra);
    DclAssert(pBuffer);
    DclAssert(ulStart < pEMI->ulTotalSize);
    DclAssert(uLength <= pEMI->ulTotalSize - ulStart);

    while(uLength)
    {
        D_UINT16 len = DCLMIN(uLength, MERGE_SIZE);

        if(DclOsFileSeek(pEMI->pFimExtra->hFile, ulStart, DCLFSFILESEEK_SET) == 0 &&
           DclOsFileRead(mergebuf, len, 1, pEMI->pFimExtra->hFile) == 1)
        {
            unsigned int    i;

            for(i = 0; i < len; ++i)
                mergebuf[i] &= *pData++;

            if(DclOsFileSeek(pEMI->pFimExtra->hFile, ulStart, DCLFSFILESEEK_SET) != 0 ||
               DclOsFileWrite(mergebuf, len, 1, pEMI->pFimExtra->hFile) != 1)
            {
                FFXPRINTF(1, ("FFX: NORFILE: Error writing to file\n"));
                return FALSE;
            }
        }
        else
        {
            FFXPRINTF(1, ("FFX: NORFILE: Error reading from file\n"));
            return FALSE;
        }

        uLength -= len;
        ulStart += len;
    }
    return TRUE;
}


/*-------------------------------------------------------------------
    Public: EraseStart()

    Parameters:
       pEMI    - A pointer to the ExtndMediaInfo structure to use.
       ulStart - Starting offset in bytes to begin the erase.  This
                 must be on a physical erase zone boundary.
       uLength - Number of bytes to erase.  This must be the exact
                 total length of one or more physical erase zones starting
                 at ulStart.

    Notes:
       Attempts to initiate an erase operation.  If it is started
       successfully, the only FIM functions that can then be called are
       EraseSuspend() and ErasePoll().  The operation must subsequently
       be monitored by calls to ErasePoll().

       If it is not started successfully, those functions may not be
       called.  The flash is restored to a readable state if possible,
       but this cannot always be guaranteed.

    Return Value:
       TRUE  - If the erase was started successfully
       FALSE - If the erase failed
-------------------------------------------------------------------*/
static D_BOOL EraseStart(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT32        ulLength)
{
    PFIMEXTRA       pFim;
  #if EMULATE_BOOT_BLOCKS
    D_UINT32        ulChipBB;   /* boot block # within the chip */
  #endif

    DclAssert(pEMI);
    pFim = pEMI->pFimExtra;
    DclAssert(pFim);

    /*  Verify offset and length parameters within the media boundaries.
    */
    DclAssert(ulStart < pEMI->ulTotalSize);
    DclAssert(ulLength <= pEMI->ulTotalSize - ulStart);

  #if EMULATE_BOOT_BLOCKS
    if(pEMI->ulBootBlockSize)
        ulChipBB = (ulStart % pEMI->ulDeviceSize) / pEMI->ulBootBlockSize;
    else
        ulChipBB = 0;

    /*  See if the start address falls within the range for the low
        or high boot blocks.
    */
    if((ulChipBB < pEMI->uBootBlockCountLow) ||
       (pEMI->uBootBlockCountHigh &&
        (ulChipBB >= pFim->uHighBootBlockStart)))
    {
        DclAssert(ulStart % pEMI->ulBootBlockSize == 0);
        DclAssert(ulLength % pEMI->ulBootBlockSize == 0);
        DclAssert(ulLength >= pEMI->ulBootBlockSize);

        /*  Never erase more than a single boot block.
        */
        ulLength = pEMI->ulBootBlockSize;

        pFim->fInBootBlocks = TRUE;
    }
    else
  #endif
    {
        DclAssert(ulStart % pEMI->ulEraseZoneSize == 0);
        DclAssert(ulLength % pEMI->ulEraseZoneSize == 0);
        DclAssert(ulLength >= pEMI->ulEraseZoneSize);

        /*  Never erase more than a single zone.
        */
        ulLength = pEMI->ulEraseZoneSize;

      #if EMULATE_BOOT_BLOCKS
        pFim->fInBootBlocks = FALSE;
      #endif
    }

    if(DclOsFileSeek(pFim->hFile, ulStart, DCLFSFILESEEK_SET) == 0)
    {
        while(ulLength)
        {
            PUTCHAR((unsigned char)~0, pFim->hFile);
            --ulLength;
        }
    }
    else
    {
        FFXPRINTF(1, ("FFX: NORFILE: Error writing to file\n"));
    }
    return (ulLength == 0);
}


/*-------------------------------------------------------------------
    Public: ErasePoll()

    Parameters:
       pEMI - A pointer to the ExtndMediaInfo structure to use.

    Notes:
       Monitor the status of an erase begun with EraseStart().

       If the erase fails, attempts to return the flash to its normal
       read mode.  Depending on the type of flash, this may or may
       not be possible.  If it is possible, it may be achieved by
       suspending the erase operation rather than by terminating it.
       In this case, it may be possible to read the flash, but not to
       erase it further.

       This function may be called with the flash either in read array
       mode or in read status mode.

    Return Value:
       If the erase is still in progress, returns 0.  The only FIM
       functions that can then be called are EraseSuspend() and
       ErasePoll().

       If the erase completed successfully, returns the length of the
       erase zone actually erased.  This may be less than the ulLength
       value supplied to EraseStart().  The flash is in normal read
       mode.

       If the erase failed, returns FIMMT_ERASE_FAILED, which is a
       value that could never be a valid erase length.  The flash is
       returned to normal read mode if possible, but this may not be
       possible in all cases (for example, if the flash does not
       support suspending an erase, and the operation times out).
-------------------------------------------------------------------*/
static D_UINT32 ErasePoll(
    PEXTMEDIAINFO pEMI)
{
    DclAssert(pEMI);

    /*  BOGUS!  Could do better checking here for proper sequence
        of operations (like ErasePoll() after EraseStart() failed).
    */

  #if EMULATE_BOOT_BLOCKS
    if(pEMI->pFimExtra->fInBootBlocks)
        return pEMI->ulBootBlockSize;
    else
  #endif
        return pEMI->ulEraseZoneSize;
}


#if NORFILE_EMULATE_ISWF
/*-------------------------------------------------------------------
    Public: ReadControl()

    Read a given number of bytes of data from the media in control mode.

    This function will handle formatting the data properly (ie, reading
    the programmed areas of flash to the buffer )

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use
        ulStart - high 22 bits are the physical address of page start.
                  offset into page: bits 0-9 (bit 9 must be zero for 1x16)
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL ReadControl(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    D_UINT16        uThisLength = 0;
    D_UINT32        ulFlashOffset = 0;
    D_UINT32        ulOffsetInPage = 0;
    D_UINT16       *puDataPtr = (D_UINT16 *)pBuffer;
    D_BOOL          fSuccess = TRUE;

    DclAssert(pEMI);
    DclAssert(ulStart % sizeof(D_UINT32) == 0L);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert(puDataPtr);
    DclAssert(uLength % sizeof(*puDataPtr) == 0L);
    DclAssert(ulStart % sizeof(*puDataPtr) == 0L);

    /*  extract page address
    */
    ulFlashOffset = ulStart & PAGE_MASK;

    /*  Get the offset within the page and check that it is with in the
        allowed size.
    */
    ulOffsetInPage = ulStart & OFFSET_MASK;

    /*  This should be less than FLASH_PAGE_SIZE/2 values will be coming in
        0-512.
    */
    DclAssert(ulOffsetInPage < (FLASH_PAGE_SIZE/2));
    if(ulOffsetInPage >= (FLASH_PAGE_SIZE/2))
    {
        return FALSE;
    }

    /*  Convert the offset within the page to account for the invalid regions.
    */
    ulOffsetInPage = ((ulOffsetInPage / FLASH_CONTROL_VALID_SIZE) *
                      (FLASH_CONTROL_VALID_SIZE + FLASH_CONTROL_INVALID_SIZE))+
                      (ulOffsetInPage % FLASH_CONTROL_VALID_SIZE);

    ulFlashOffset += ulOffsetInPage;

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert((ulFlashOffset + uLength) <= pEMI->ulTotalSize);

    /*  Align the first read to the control mode region size
    */
    uThisLength = (D_UINT16) DCLMIN(uLength,
                      (FLASH_CONTROL_VALID_SIZE -
                      (ulFlashOffset % (FLASH_CONTROL_VALID_SIZE + FLASH_CONTROL_INVALID_SIZE))));

    while(uLength)
    {
        if(DclOsFileSeek(pEMI->pFimExtra->hFile, ulFlashOffset, DCLFSFILESEEK_SET) == 0
        && DclOsFileRead(puDataPtr, uThisLength, 1, pEMI->pFimExtra->hFile) == 1)
        {
            /*  Go to the next offset
            */
            ulFlashOffset += (uThisLength + FLASH_CONTROL_INVALID_SIZE);
            puDataPtr += (uThisLength);
            uLength -= uThisLength;

            /*  Recalculate the length of the next request
            */
            if(uLength < FLASH_CONTROL_VALID_SIZE)
            {
                uThisLength = uLength;
            }
            else
            {
                uThisLength = FLASH_CONTROL_VALID_SIZE;
            }
        }
        else
        {
            FFXPRINTF(1, ("FFX: NORFILE: Error reading from file\n"));
            return FALSE;
        }
    }
    return TRUE;
}


/*-------------------------------------------------------------------
    Public: WriteControl()

    Writes a given number of bytes of data out to the media. (control mode)
    It does not return until the data is programmed.

    This function will handle formatting the data properly (ie, writing
    to only the appropriate areas of the buffer)

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use.
        ulStart - high 22 bits are the physical address of page start.
                  offset into page: bits 0-9 (bit 9 must be zero for 1x16)
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL WriteControl(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    D_UINT32        ulFlashOffset = 0;
    D_UINT32        ulOffsetInPage = 0;
    D_UCHAR        *pcDataPtr = (D_UCHAR *)pBuffer;
    D_BOOL          fSuccess = TRUE;
    D_UCHAR        *pMedia = NULL;
    D_UINT16        len;

    D_UCHAR         mergebuf[FLASH_CONTROL_VALID_SIZE];

    DclAssert(pEMI);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert(ulStart % sizeof(D_UINT32) == 0L);
    DclAssert(pcDataPtr);
    DclAssert(uLength % sizeof(*pcDataPtr) == 0L);
    DclAssert(ulStart % sizeof(*pcDataPtr) == 0L);

    /*  First get the address for the start of the page.
    */
    ulFlashOffset = ulStart & PAGE_MASK;

    /*  Get the offset within the page and check that it is with in the
        allowed size.
    */
    ulOffsetInPage = ulStart & OFFSET_MASK;

    /*  This should be less than FLASH_PAGE_SIZE / 2 values will be coming
        in 0 - FLASH_PAGESIZE / 2
    */
    DclAssert(ulOffsetInPage < (FLASH_PAGE_SIZE / 2));
    if(ulOffsetInPage >= (FLASH_PAGE_SIZE / 2))
    {
        return FALSE;
    }

    /*  Convert the offset within the page to account for the invalid regions.
    */
    ulOffsetInPage = ((ulOffsetInPage / FLASH_CONTROL_VALID_SIZE) *
                     (FLASH_CONTROL_VALID_SIZE + FLASH_CONTROL_INVALID_SIZE))+
                     (ulOffsetInPage % FLASH_CONTROL_VALID_SIZE);

    ulFlashOffset += ulOffsetInPage;

    /*  Verify user address and length parameters within the media boundaries.
    */
    DclAssert((ulFlashOffset + uLength) <= pEMI->ulTotalSize);

    len = (D_UINT16) DCLMIN(uLength,
                      (FLASH_CONTROL_VALID_SIZE -
                      (ulFlashOffset % (FLASH_CONTROL_VALID_SIZE + FLASH_CONTROL_INVALID_SIZE))));

    while(uLength)
    {
        if(DclOsFileSeek(pEMI->pFimExtra->hFile, ulFlashOffset, DCLFSFILESEEK_SET) == 0 &&
           DclOsFileRead(mergebuf, len, 1, pEMI->pFimExtra->hFile) == 1)
        {
            unsigned int    i;

            for(i = 0; i < len; ++i)
                mergebuf[i] &= *pcDataPtr++;

            if(DclOsFileSeek(pEMI->pFimExtra->hFile, ulFlashOffset, DCLFSFILESEEK_SET) != 0 ||
               DclOsFileWrite(mergebuf, len, 1, pEMI->pFimExtra->hFile) != 1)
            {
                FFXPRINTF(1, ("FFX: NORFILE: Error writing to file\n"));
                return FALSE;
            }
        }
        else
        {
            FFXPRINTF(1, ("FFX: NORFILE: Error reading from file\n"));
            return FALSE;
        }

            /*  If we have written everything, get outta here.
            */
            ulFlashOffset   += ((D_UINT16)len + FLASH_CONTROL_INVALID_SIZE);
            uLength -= len;
            len = DCLMIN(uLength, FLASH_CONTROL_VALID_SIZE);

    }

    return TRUE;
}

#endif  /* NORFILE_EMULATE_ISWF */


/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_norfile =
{
    Mount,
    Unmount,
    Read,
    Write,
    EraseStart,
    ErasePoll,
  #if NORFILE_EMULATE_ISWF
    NULL,
    NULL,
    ReadControl,
    WriteControl
  #else
    NULL,
    NULL
  #endif
};



