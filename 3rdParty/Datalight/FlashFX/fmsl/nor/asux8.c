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

    This module contains the FIM code for AMD (and compatible) flash which
    uses uniform sectors and is organized in an 8-bit wide, non-interleaved
    fashion.

    $DOCTODO
    1. Update to support erase suspend/resume.
    2. Update the write/erase timeout values.
    3. Fully test the support for the Fujitsu flash parts.
    4. Test the support for the AmC0XXDFLKA flash memory PC card.
    5. Include support of device am29F010.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: asux8.c $
    Revision 1.10  2009/02/26 22:22:04Z  keithg
    Added type casts to latch offset assignments to placate compiler
    warnings of signed/unsigned mismatches.
    Revision 1.9  2008/07/31 16:32:23Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.8  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2006/10/02 17:57:51Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.6  2006/08/31 19:23:37Z  pauli
    Corrected an inverted condition when scanning for multiple chips.
    Revision 1.5  2006/08/31 05:29:43Z  pauli
    Updated to use the new FfxFimNorWindowCreate/FfxFimNorWindowMap
    functions.  Eliminated the use of the AMD_SEND_CMD macro.
    Revision 1.4  2006/05/20 20:36:11Z  Garyp
    Minor fixes to deal with renamed or obsolete structure fields.
    Revision 1.3  2006/02/10 21:06:51Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.2  2006/02/08 18:21:15Z  Garyp
    Modified to use new FfxHookWindowMap/Size() functions.  Updated debugging
    code.
    Revision 1.1  2005/10/14 02:07:56Z  Pauli
    Initial revision
    Revision 1.4  2005/05/16 19:54:44Z  garyp
    Removed some invalid asserts.  The media pointer can legally be zero on
    some platforms.
    Revision 1.3  2005/05/13 02:49:32Z  garyp
    Cleaned up the erase zone size define.
    Revision 1.2  2005/05/12 04:25:09Z  garyp
    Updated to build cleanly (in some picky environments).
    Revision 1.1  2005/05/11 20:22:52Z  garyp
    Initial revision
---------------------------------------------------------------------------*/


#include <flashfx.h>
#include <fimdev.h>
#include "nor.h"

/*  Configure for a 1x8 layout
*/
#define FLASH_INTERLEAVE    1       /* Number of flash chips interleaved    */
#define FLASH_BUS_WIDTH     8       /* Overall data bus width in bits       */

#include "flashcmd.h"
#include "amdcmd.h"


/*  Copied from the old fim.h file.  These numbers should be examined
    against data sheets for the part that this FIM supports!
*/
#define WRITE_TIMEOUT           (1)

/*  Copied from the old fim.h file.  These numbers should be examined
    against data sheets for the part that this FIM supports!
*/
#define ERASE_TIMEOUT           (20* 1000UL)

#define ERASED_VALUE            0xFF

/*  Latch address offsets
*/
#define LATCH_OFFSET1           0x555
#define LATCH_OFFSET2           0x2AA

/*  Minimum window sizes - AMD 80/16 x8=2k and AMD 40 x8=32K
*/
#define MIN_WINDOW_SIZE_X8_80   0x800L
#define MIN_WINDOW_SIZE_X8_40   0x8000L

/*  Latch offset masks
*/
#define LATCH_OFFSET_MASK_X8_80  (~(MIN_WINDOW_SIZE_X8_80 - 1))
#define LATCH_OFFSET_MASK_X8_40  (~(MIN_WINDOW_SIZE_X8_40 - 1))

static D_BOOL PollForWriteCompletion(PFLASHDATA pMedia, unsigned char ucData);

typedef struct tagFIMEXTRA
{
    D_UINT32       ulLatchOffsetMask;
    D_BOOL          fSmallWindow;
} FIMEXTRA;

/*  All parts supported in this FIM have 64KB erase zones --
    multiply appropriately for the interleave value.
*/
#define ZONE_SIZE           (0x00010000UL * FLASH_INTERLEAVE) /* 64KB */

/*  Define FIM specific ID flags
*/
#define IDFLAG_SMALLOFFSET      IDFLAG_FIM0

/*  Note that while this table only lists AMD chip IDs, the identification
    code will also recognize the equivalent Fujitsu parts.
*/
static AMDCHIPPARAMS ChipTable[] =
{
    {ID_AM29F040(IDFLAG_SMALLOFFSET)    },
    {ID_AM29F080(0)                     },
    {ID_AM29F016(0)                     },
    {ID_AM29F032B(0)                    },
    {ID_AM29LV081(0)                    },
    {ID_AM29LV017B(0)                   },
    {ID_AM29LV033C(0)                   },
    {ID_AM29LV065D(0)                   },
    {ID_AMC0XXDFLKA(0)                  },
    {ID_ENDOFLIST}
};


/*-------------------------------------------------------------------
    Mount()

    Description
        Determines if the media is indeed supported.  If so the
        ExtndMediaInfo structure is updated to reflect it.

    Parameters
        pEMI - A pointer to the ExtndMediaInfo structure to use.

    Return Value
        Returns TRUE if successful, else FALSE
-------------------------------------------------------------------*/
static D_BOOL Mount(
    PEXTMEDIAINFO   pEMI)
{
    D_UINT32        ulAddress = 0L;
    D_UINT32        ulBaseLatch;
    D_UINT32        ulLatchOffsetMask;
    PFLASHDATA      pMedia;
    PFLASHDATA      pLatch1;   /*  media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;   /*  media pointer at latch offset 2 */
    unsigned        i;
    FLASHIDCODES    ID;
    FFXFIMBOUNDS    bounds;

    DclAssert(pEMI);

    pEMI->uDeviceType       = DEV_NOR | DEV_MERGE_WRITES | DEV_NOT_MLC;
    pEMI->ulEraseZoneSize   = ZONE_SIZE;

    /*  Assume AMD 80/16/32 byte mode and initialize the latch offsets.
    */
    ulLatchOffsetMask = (D_UINT32) LATCH_OFFSET_MASK_X8_80;

    /*  Get the array bounds and map the window.
    */
    FfxDevGetArrayBounds(pEMI->hDev, &bounds);
    if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
        return FALSE;
    if(!FfxFimNorWindowCreate(pEMI->hDev, LATCH_OFFSET1, &bounds, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowCreate(pEMI->hDev, LATCH_OFFSET2, &bounds, (volatile void **)&pLatch2))
        return FALSE;

    /*  read manufacturer and device codes from the interleave
    */
    *pMedia = AMDCMD_READ_MODE;
    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_IDENTIFY;
    GETFLASHIDCODES(&ID, pMedia);
    if(!ISVALIDFLASHIDCODE(&ID))
    {
        FFXPRINTF(1, ("FIM asux8:    Invalid flash ID: Data0/1=%02x/%02x\n",
            ID.data0, ID.data1));

        *pMedia = AMDCMD_READ_MODE;

        return FALSE;
    }

    for(i = 0; ChipTable[i].uIDFlags != IDFLAG_ENDOFLIST; i++)
    {
        if((ID.idDev == ChipTable[i].idDev) &&
            ((ID.idMfg == ChipTable[i].idMfg) ||
            ((ChipTable[i].idMfg == ID_AMD) && (ID.idMfg == ID_FUJITSU))))
        {
            pEMI->ulDeviceSize = ChipTable[i].ulChipSize * FLASH_INTERLEAVE;

            FFXPRINTF(1, ("FIM asux8:    Mfg/Dev=%02x/%02x supported\n", ID.idMfg, ID.idDev));

            /*  This FIM does not support boot blocks, so ensure that
                we're not finding a part that supports them.
            */
            DclAssert(!ChipTable[i].uLowBootBlocks);
            DclAssert(!ChipTable[i].uHighBootBlocks);

            break;
        }
    }

    if(ChipTable[i].uIDFlags == IDFLAG_ENDOFLIST)
    {
        FFXPRINTF(1, ("FIM asux8:    Mfg/Dev=%02x/%02x unsupported\n", ID.idMfg, ID.idDev));

        /*  If the device is not supported, restore the original state
            and return device not found.
        */
        *pMedia = AMDCMD_READ_MODE;

        return FALSE;
    }

    /*  Note: This entire section of code needs to be reviewed it is
        only being kept because it was here in the original amdx8 FIM
        implementation.
    */
    if(ChipTable[i].uIDFlags & IDFLAG_SMALLOFFSET)
    {
        /*  Assume AMD 40 byte x8 mode and change device size
            and latch offsets.
        */
        ulLatchOffsetMask = (D_UINT32) LATCH_OFFSET_MASK_X8_40;

        /*  Check for AMD 40 byte x8 mode.
        */
        *pMedia = AMDCMD_READ_MODE;
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pLatch1 = AMDCMD_IDENTIFY;
        GETFLASHIDCODES(&ID, pMedia);
        if(!ISVALIDFLASHIDCODE(&ID))
        {
            FFXPRINTF(1, ("FIM asux8:    Invalid flash ID: Data0/1=%02x/%02x\n",
                ID.data0, ID.data1));

            *pMedia = AMDCMD_READ_MODE;

            return FALSE;
        }
    }

    *pMedia = AMDCMD_READ_MODE;

    for(ulAddress = pEMI->ulDeviceSize;
        ulAddress < MAX_ARRAY;
        ulAddress += pEMI->ulDeviceSize)
    {
        FLASHIDCODES    ID2;

        /*  The mount address must always be a multiple of the zone size.
        */
        DclAssert(!(ulAddress % pEMI->ulEraseZoneSize));

        /*  Get media pointers at this address.
        */
        ulBaseLatch = ulAddress & ulLatchOffsetMask;
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
            break;
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulBaseLatch + LATCH_OFFSET1, &bounds, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulBaseLatch + LATCH_OFFSET2, &bounds, (volatile void **)&pLatch2))
            return FALSE;

        /*  Check for wrap around
        */
        GETFLASHIDCODES(&ID2, pMedia);
        if((ID2.data0 == ID.data0) && (ID2.data1 == ID.data1))
            break;

        /*  Send chip ID commands.  If chip ID is found, check if it is
            identical to the initial chip.
        */
        *pMedia = AMDCMD_READ_MODE;
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pLatch1 = AMDCMD_IDENTIFY;
        GETFLASHIDCODES(&ID2, pMedia);
        if((ID2.data0 != ID.data0) || (ID2.data1 != ID.data1))
            break;

        /*  Reset the flash to read mode.  The first device in the array is
            not included in this loop, so we don't need any special cases.
        */
        *pMedia = AMDCMD_READ_MODE;
     }

    /*  Reset the first chips to read mode
    */
    if(!FfxFimNorWindowCreate(pEMI->hDev, 0, &bounds, (volatile void **)&pMedia))
        return FALSE;

    *pMedia = AMDCMD_READ_MODE;

    pEMI->ulTotalSize = ulAddress;

    pEMI->pFimExtra = DclMemAllocZero(sizeof *pEMI->pFimExtra);
    if(!pEMI->pFimExtra)
        return FALSE;

    pEMI->pFimExtra->ulLatchOffsetMask = ulLatchOffsetMask;

    return TRUE;
}


/*-------------------------------------------------------------------
    Unmount()

    Description
        This function dismounts the FIM and releases any allocated
        resources.

    Parameters
        pEMI - A pointer to the ExtndMediaInfo structure to use.

    Return Value
        None
-------------------------------------------------------------------*/
static void Unmount(
    PEXTMEDIAINFO   pEMI)
{
    DclAssert(pEMI);
    DclAssert(pEMI->pFimExtra);

    DclMemFree(pEMI->pFimExtra);
    pEMI->pFimExtra = NULL;
}


/*-------------------------------------------------------------------
    Read()

    Description
        Read a given number of bytes of data from the media.

    Parameters
        pEMI    - A pointer to the ExtndMediaInfo structure to use
        ulStart - Starting offset in bytes to begin the access
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return Value
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL Read(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    DclAssert(pEMI);

    return FfxFimNorRead(pEMI, ulStart, uLength, pBuffer);
}


/*-------------------------------------------------------------------
    Write()

    Description
        Writes a given number of bytes of data out to the media.
        It does not return until the data is programmed.

    Parameters
        pEMI    - A pointer to the ExtndMediaInfo structure to use.
        ulStart - Starting offset in bytes to begin the access
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return Value
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL Write(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    D_UINT32        ulBaseLatch;
    D_UCHAR        *pcDataPtr = (D_UCHAR *)pBuffer;
    PFLASHDATA      pMedia;     /*  media pointer where data is written */
    PFLASHDATA      pLatch1;    /*  media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;    /*  media pointer at latch offset 2 */
    D_BOOL          fSuccess = TRUE;

    DclAssert(pEMI);
    DclAssert(ulStart % sizeof(D_UINT32) == 0L);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert(ulStart < pEMI->ulTotalSize);
    DclAssert(pEMI->ulTotalSize - ulStart >= uLength);

    while(uLength)
    {
        D_UINT32    ulWindowSize;
        D_UINT16    u;

        ulBaseLatch = ulStart & pEMI->pFimExtra->ulLatchOffsetMask;

        /*  Get a media pointer at LATCH_OFFSET1 and LATCH_OFFSET2.
        */
        if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch + LATCH_OFFSET1, (volatile void **)&pLatch1))
        {
            fSuccess = FALSE;
            break;
        }
        if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch + LATCH_OFFSET2, (volatile void **)&pLatch2))
        {
            fSuccess = FALSE;
            break;
        }

        /*  Get the media pointer and the max size we can access with it
        */
        ulWindowSize = FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia);
        if(!ulWindowSize)
        {
            fSuccess = FALSE;
            break;
        }

        /*  Move each window worth of data into the flash memory.
        */
        while(ulWindowSize && uLength)
        {
            D_UINT32    ulThisLength = DCLMIN(uLength, ulWindowSize);

            /*  Start from a known state
            */
            *pMedia = AMDCMD_READ_MODE;

            /*  Program ulThisLength bytes into flash
            */
            for(u = 0; u < ulThisLength; u++)
            {
                /*  Don't waste time programming all 1's
                */
                if(pcDataPtr[u] != (D_UCHAR)0xFF)
                {
                    /*  Write command sequence coded in-line since timing critical
                    */
                    *pLatch1 = AMDCMD_UNLOCK1;
                    *pLatch2 = AMDCMD_UNLOCK2;
                    *pLatch1 = AMDCMD_PROGRAM;

                    pMedia[u] = pcDataPtr[u];

                    if(!PollForWriteCompletion(&pMedia[u], pcDataPtr[u]))
                    {
                        *pMedia = AMDCMD_READ_MODE;
                        return FALSE;
                    }
                }
            }

            /*  If we have written everything, get outta here.
            */
            uLength -= (D_UINT16)ulThisLength;
            if(uLength == 0)
                break;

            /*  Go to the next offset
            */
            ulWindowSize    -= ulThisLength;
            ulStart         += ulThisLength;
            pcDataPtr       += ulThisLength;

            DclAssert(ulStart);
            DclAssert(pcDataPtr);
        }

        if(!fSuccess)
            break;
    }

    /*  Be sure we leave the flash in read mode.
    */
    *pMedia = AMDCMD_READ_MODE;

    return fSuccess;
}


/*-------------------------------------------------------------------
    EraseStart()

    Description
        Attempts to initiate an erase operation.  If it is started
        successfully, the only FIM functions that can then be called
        are EraseSuspend() and ErasePoll().  The operation must s
        subsequently be monitored by calls to ErasePoll().

        If it is not started successfully, those functions may not
        be called.  The flash is restored to a readable state if
        possible, but this cannot always be guaranteed.

    Parameters
        pEMI    - A pointer to the ExtndMediaInfo structure to use.
        ulStart - Starting offset in bytes to begin the erase.  This
                  must be on a physical erase zone boundary.
        uLength - Number of bytes to erase.  This must be the exact
                  total length of one or more physical erase zones
                  starting at ulStart.

    Return Value
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseStart(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT32        ulLength)
{
    D_UINT32        ulBaseLatch;
    PFLASHDATA      pMedia;
    PFLASHDATA      pLatch1;    /*  media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;    /*  media pointer at latch offset 2 */
    DCLTIMER        timer;

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert(pEMI);
    DclAssert(pEMI->pFimExtra);
    DclAssert(ulStart < pEMI->ulTotalSize);
    DclAssert(ulLength <= pEMI->ulTotalSize - ulStart);
    DclAssert((ulStart + ulLength) <= pEMI->ulTotalSize);
    DclAssert(ulLength >= pEMI->ulEraseZoneSize);
    DclAssert(ulLength % pEMI->ulEraseZoneSize == 0);
    (void)ulLength;

    /*  Get media pointers
    */
    ulBaseLatch = ulStart & pEMI->pFimExtra->ulLatchOffsetMask;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch + LATCH_OFFSET1, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch + LATCH_OFFSET2, (volatile void **)&pLatch2))
        return FALSE;

    /*  Send erase commands
    */
    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_ERASE1;
    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_ERASE2;

    DclTimerSet(&timer, ERASE_TIMEOUT);

    /*  Wait till the byte is done programming -
        indicated when data bit 7 is 1
    */
    while((*pMedia & AMDSTAT_DONE) != AMDSTAT_DONE)
    {
        if(DclTimerExpired(&timer))
            break;
    }

    /*  Check for failure -
        indicated when bit 5 set and bit 7 still wrong
    */
    if(((*pMedia & AMDSTAT_FAIL) == AMDSTAT_FAIL) &&
       ((*pMedia & AMDSTAT_DONE) != AMDSTAT_DONE))
        goto FailErase;

    /*  Check for erased value
    */
    if(*pMedia != ERASED_VALUE)
        goto FailErase;

    /*  Leave the block in read mode
    */
    *pMedia = AMDCMD_READ_MODE;

    /*  Erase worked
    */
    return TRUE;

  FailErase:

    FFXPRINTF(1, ("FIM erase failure!\n"));

    /*  Clear the error status, reset to read mode and return
    */
    *pMedia = AMDCMD_READ_MODE;
    return FALSE;
}


/*-------------------------------------------------------------------
    ErasePoll()

    Description
        Monitor the status of an erase begun with EraseStart().

        If the erase fails, attempts to return the flash to its
        normal read mode.  Depending on the type of flash, this
        may or may not be possible.  If it is possible, it may be
        achieved by suspending the erase operation rather than by
        terminating it.  In this case, it may be possible to read
        the flash, but not to erase it further.

        This function may be called with the flash either in read
        array mode or in read status mode.

    Parameters
        pEMI     - A pointer to the ExtndMediaInfo structure to use

    Return Value
        If the erase is still in progress, returns 0.  The only FIM
        functions that can then be called are EraseSuspend() and
        ErasePoll().

        If the erase completed successfully, returns the length of
        the erase zone actually erased.  This may be less than the
        ulLength value supplied to EraseStart().  The flash is in
        normal read mode.

        If the erase failed, returns FIMMT_ERASE_FAILED, which is a
        value that could never be a valid erase length.  The flash
        is returned to normal read mode if possible, but this may not
        be possible in all cases (for example, if the flash does not
        support suspending an erase, and the operation times out).
-------------------------------------------------------------------*/
static D_UINT32 ErasePoll(
    PEXTMEDIAINFO   pEMI)
{
    DclAssert(pEMI);

    /*  Since EraseStart() will always erase exactly one erase zone at
        a time, and it will always erase the whole zone before returning,
        simply return the length of one erase zone here.
    */
    return pEMI->ulEraseZoneSize;
}


/*-------------------------------------------------------------------
    PollForWriteCompletion()

    Description
        This function polls for completion of a write operation
        and checks for failure codes.

    Parameters
        pMedia  - A pointer to the flash byte being written
        ucData  - The data value that was written

    Return Value
        Returns TRUE if successful, or FALSE if the operation timed
        out or otherwise failed.
-------------------------------------------------------------------*/
static D_BOOL PollForWriteCompletion(
    PFLASHDATA      pMedia,
    unsigned char   ucData)
{
    DCLTIMER        timer;

    DclTimerSet(&timer, WRITE_TIMEOUT);

    while(!DclTimerExpired(&timer))
    {
        /*  Wait till the byte is done programming - indicated when
            data bit 7 equals original data bit 7
        */
        if((*pMedia & AMDSTAT_DONE) == (ucData & AMDSTAT_DONE))
             break;
    }

    /*  Either timed out or finished between check, so check
        one last time.
    */
    if((*pMedia & AMDSTAT_DONE) != (ucData & AMDSTAT_DONE))
    {
        FFXPRINTF(1, ("FIM write timeout!\n"));
        return FALSE;
    }

    /*  Check for error - indicated when bit 5 set and bit 7 still wrong
    */
    if(((*pMedia & AMDSTAT_FAIL) == AMDSTAT_FAIL) &&
       ((*pMedia & AMDSTAT_DONE) != (ucData & AMDSTAT_DONE)))
    {
        FFXPRINTF(1, ("FIM write failure!\n"));
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_asux8 =
{
    Mount,
    Unmount,
    Read,
    Write,
    EraseStart,
    ErasePoll,
    NULL,               /* EraseSuspend/Resume not yet supported */
    NULL
};

