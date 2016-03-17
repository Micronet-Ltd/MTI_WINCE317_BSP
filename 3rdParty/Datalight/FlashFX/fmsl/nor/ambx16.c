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

    "Flash Interface Module For MirrorBit Devices In x16 Mode"
    This FIM is for parts with uniform sectors.
    Provides the Flash Interface Module (FIM) for the following flash
    parts in a 16 bit wide configuration:

    AMD:
    Am29LV640MH/L
    Am29LV640MU
    Am29LV641MH/L
    Am29LV128M
    Am29LV256M

    Spansion:
    S29GL512N

    $DOCTODO
       Currently handles all window sizes by mapping offsets for
       each command.  Could add code to improve performance by mapping
       offsets only once per window write when window size is not less
       than specified minimum (see AMDX8.C).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ambx16.c $
    Revision 1.12  2009/04/03 05:40:38Z  keithg
    Fixed bug 2521: removed obsolete READ_BACK_VERIFY.
    Revision 1.11  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2006/10/02 17:57:51Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.9  2006/09/12 23:19:08Z  rickc
    In WriteBufferedBytes, monitor last address written to;
    previously monitoring first address, which is not to spec.
    Also, disabled read back verify, which was incorrectly checked
    in as enabled.
    Revision 1.8  2006/08/31 21:20:05Z  rickc
    Fixed small bugs found in testing
    Revision 1.7  2006/08/31 17:34:49Z  pauli
    Corrected copy/paste error passing wrong parameter to FfxFimNorWindowMap.
    Initialized some locals to appease the gnu compiler.
    Revision 1.6  2006/08/31 17:20:58Z  pauli
    Completed updated to use the new FfxFimNorWindowCreate,
    FfxFimNorWindowMap functions.  Eliminated the use of the
    AMD_SEND_CMD macro.
    Revision 1.5  2006/08/30 00:10:38Z  pauli
    Began updating to use the new FfxFimNorWindowCreate,
    FfxFimNorWindowMap functions.
    Revision 1.4  2006/08/19 02:36:08Z  Garyp
    Fixed more typos.
    Revision 1.3  2006/08/19 02:11:34Z  Garyp
    One more time...
    Revision 1.2  2006/08/19 01:18:11Z  Garyp
    Updated to build cleanly.
    Revision 1.1  2006/08/11 23:20:48Z  rickc
    Initial revision
---------------------------------------------------------------------------*/


#include <flashfx.h>
#include <fimdev.h>
#include "nor.h"

/*
        Due to the unusual nature by which the status
        bit is toggled on these chips, making the code thread-safe
        is problematic.  Therefore, if this FIM is used, you must
        ensure that you only mount one VBF Disk on the Device,
        OR, you must modify the FIM to disable the EraseSuspend
        functionality.  Please contact Datalight for more information
        on these issues.  By default, INCLUDE_ERASE_SUSPEND will be undefined.
*/
#undef INCLUDE_ERASE_SUSPEND

/*  Configure for a x16 layout
*/
#define FLASH_INTERLEAVE    1       /* Number of flash chips interleaved    */
#define FLASH_BUS_WIDTH     16      /* Overall data bus width in bits       */

#include "flashcmd.h"
#include "amdcmd.h"

/*  Am29LV640MH/L, Am29LV640MU, Am29LV641MH/L, Am29LV128M, and Am29LV256M
    up to 100K cycles 15 sec
*/
#define ERASE_TIMEOUT   20000   /* 20 seconds */

/*  Am29LV640MH/L (210us/word * 16 words) = 3360us rev C (rev E has TBD)
    Am29LV640MU   (210us/word * 16 words) = 3360us rev B1 (rev B3 has TBD)
    Am29LV641MH/L (210us/word * 16 words) = 3360us rev B1 (rev B5 has TBD)
    Am29LV128M    (210us/word * 16 words) = 3360us rev B (rev B3 has TBD)
    Am29LV256M    (210us/word * 16 words) = 3360us rev B1 (rev B6 has TBD)

    Senior Member Technical Staff, Field Applications, AMD says,
       "The worst case TBD on programming is now 1ms"
       so (1ms/word * 16 words) = 16ms
*/
#define WRITE_TIMEOUT   30      /* 30ms */


#define ERASE_SUSPEND_TIMEOUT (1)
#define MINIMUM_ERASE_INCREMENT (100)   /* microseconds */


/*  Status indicators
*/
#define AMD_DONE_LOW            MAKEFLASHCMD(ASTAT_OK)   /* DQ7 Data# polling (low device) */
#define AMD_FAIL_LOW            MAKEFLASHCMD(ASTAT_FAIL) /* DQ5 exceeded timing limits (low) */
#define AMD_ERASE_BEGUN_LO      MAKEFLASHCMD(ASTAT_ERASEBEGUN)  /* DQ3 on low chip */
#define AMD_BUFF_ABORT_LOW      MAKEFLASHCMD(ASTAT_BUFFERABORT) /* DQ1 Write-to-Buffer Abort (low) */

/*  Latch address offsets
*/
#define LATCH_OFFSET1           0xAAAUL
#define LATCH_OFFSET2           0x554UL

/*  Used to identify the flash chips
*/
#define WORD_LENGTH             16
#define LOW_WORD                0x0000FFFFUL

/*  See the "Autoselect Codes" table in the
*/

/*  Am29LV640MH/L datasheet for these address
*/

/*  pin values.  These are WORD address
*/

/*  pins.  This and the interleave
*/

/*  configuration account for the         A3  A0
*/

/*  additional two low bits.               |  |
*/
#define DEVICE_CODE1_OFFSET_X16  0x02     /* 00010b */
#define DEVICE_CODE2_OFFSET_X16  0x1C     /* 11100b */
#define DEVICE_CODE3_OFFSET_X16  0x1E     /* 11110b */

#define END_OF_LIST_UL      0xFFFFFFFFUL
#define END_OF_LIST         0xFFFF

#define WRITE_BUFFER_PAGE_SIZE 32       /* in bytes */

/*  If this is set to TRUE, buffered writes will be used.
*/
#define BUFFERED_WRITES TRUE

/*  If this is set to TRUE then the Unlock Bypass mode will be used during
    programming.  This will save several bus cycles for consecutive data
    writes.  This will take longer for small, random data writes.  This
    option will have no effect if BUFFERED_WRITES is set to TRUE.
*/
#define UNLOCK_BYPASS   TRUE

#define ERASE_IN_PROGRESS 0

/*  Internal prototypes
*/

static D_BOOL ReadDeviceCodes(PEXTMEDIAINFO pEMI, D_UINT32 ulAddress, ID_4_CYCLE *pChipID, FFXFIMBOUNDS *pBounds);
static D_BOOL DidOperationComplete(PFLASHDATA pMedia, D_UINT32 ulExpectedData, D_BOOL fSleep, DCLTIMER * ptimer);

#if BUFFERED_WRITES
static D_BOOL WriteBufferedBytes(PEXTMEDIAINFO pEMI, D_UINT32 ulStart, D_UINT16 * puData, D_UINT32 ulLength);
#else
static D_BOOL WriteBytes(PEXTMEDIAINFO pEMI, D_UINT32 ulStart, D_UINT16 * puData, D_UINT32 ulLength);
#endif

/*  data for individual flash devices
*/
typedef struct _ChipIdent
{
    D_UINT32        ulChipSize;
    D_UINT32        ulEraseBlockSize;
    ID_4_CYCLE      IdCode;
} _ChipIdent;

typedef struct tagFIMEXTRA
{
    int             iSuspend;
    DCLTIMER        tErase;
    D_UINT32        ulEraseStart;
    D_UINT32        ulEraseResult;
    D_UINT32        ulTimeoutRemaining;
    PFLASHDATA      pMedia;
} FIMEXTRA;

static struct _ChipIdent ChipIdentAmdM[] =
{
    {0x00800000UL, 0x00010000UL, {0x0001, 0x227E, 0x220C, 0x2201}},   /* AMD 29LV640MH/L   */
    {0x00800000UL, 0x00010000UL, {0x0001, 0x227E, 0x2213, 0x2201}},   /* AMD 29LV640MU     */
    {0x00800000UL, 0x00010000UL, {0x0001, 0x227E, 0x2213, 0x2201}},   /* AMD 29LV641MH/L   */
    {0x00800000UL, 0x00010000UL, {0x0001, 0x227E, 0x2212, 0x2200}},   /* AMD 29LV128M      */
    {0x02000000UL, 0x00010000UL, {0x0001, 0x227E, 0x2212, 0x2201}},   /* AMD 29LV256M      */
    {0x04000000UL, 0x00020000UL, {0x0001, 0x227E, 0x2223, 0x2201}},   /* Spansion 29GL512N */
    {END_OF_LIST_UL, END_OF_LIST_UL, {END_OF_LIST, END_OF_LIST, END_OF_LIST, END_OF_LIST}}
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
    D_UINT32        ulAddress = 0;
    ID_4_CYCLE      idCodeNextChip;
    ID_4_CYCLE      idCode1stChip;
    D_UINT16        uDeviceIndex;
    D_BOOL          fValidIdCode;
    PFLASHDATA      pMedia;
    PFLASHDATA      pLatch1;   /*  media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;   /*  media pointer at latch offset 2 */
    FFXFIMBOUNDS    bounds;

    /*  Initialize the extended media structure.
    */
    pEMI->uDeviceType = DEV_NOR | DEV_MERGE_WRITES | DEV_NOT_MLC;
    pEMI->ulTotalSize = 0;
    pEMI->ulDeviceSize = 0;
    pEMI->uPageSize = 0;
    pEMI->uSpareSize = 0;

    /*  Get the array bounds and map the window.
    */
    FfxDevGetArrayBounds(pEMI->hDev, &bounds);
    if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
        return FALSE;
    if(!FfxFimNorWindowCreate(pEMI->hDev, LATCH_OFFSET1, &bounds, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowCreate(pEMI->hDev, LATCH_OFFSET2, &bounds, (volatile void **)&pLatch2))
        return FALSE;

    /*  read manufacturer and device codes from each chip in the interleave
    */
    *pMedia = AMDCMD_READ_MODE;
    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_IDENTIFY;
    if(ReadDeviceCodes(pEMI, ulAddress, &idCode1stChip, &bounds) == FALSE)
    {
        return FALSE;
    }

    /*  search the device codes list for the detected ID code
    */
    for(uDeviceIndex = 0;
        ChipIdentAmdM[uDeviceIndex].ulChipSize != END_OF_LIST_UL;
        uDeviceIndex++)
    {
        if(DclMemCmp((D_UCHAR *)(&idCode1stChip),
            (D_UCHAR *)(&(ChipIdentAmdM[uDeviceIndex].IdCode)), sizeof(ID_4_CYCLE)) == 0)
        {
            pEMI->ulDeviceSize = ChipIdentAmdM[uDeviceIndex].ulChipSize;
            pEMI->ulEraseZoneSize = ChipIdentAmdM[uDeviceIndex].ulEraseBlockSize;

            FFXPRINTF(1, ("FIM ambx16:  Mfg=%X Dev1=%X Dev2=%X Dev3=%X supported\n",
                    idCode1stChip.uManufCode, idCode1stChip.uDeviceCode1,
                    idCode1stChip.uDeviceCode2, idCode1stChip.uDeviceCode3));

            break;
        }
    }

    if(ChipIdentAmdM[uDeviceIndex].ulChipSize == END_OF_LIST_UL)
    {
        /*  If the device is not supported, we already messed with it, so let's
            hope it's flash that we can reset to read mode.
        */
        *pMedia =  AMDCMD_READ_MODE;

        FFXPRINTF(1, ("FIM ambx16:  Mfg=%X Dev1=%X Dev2=%X Dev3=%X unsupported\n",
                    idCode1stChip.uManufCode, idCode1stChip.uDeviceCode1,
                    idCode1stChip.uDeviceCode2, idCode1stChip.uDeviceCode3));

        return FALSE;
    }

    /*  Search for additional devices in a linear array.  Leave the first
        chips in Identify (autoselect) mode to detect wrap around (aliasing).
        Since we've already identified the first devices, start at the next
        offset.
    */
    for(ulAddress = pEMI->ulDeviceSize;
        ulAddress < MAX_ARRAY;
        ulAddress += pEMI->ulDeviceSize)
    {
        /*  The mount address must always be a multiple of the zone size.
        */
        DclAssert((ulAddress % pEMI->ulEraseZoneSize) == 0);

        /*  Get media pointers at this address.
        */
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
            break;
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + LATCH_OFFSET1, &bounds, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + LATCH_OFFSET2, &bounds, (volatile void **)&pLatch2))
            return FALSE;

        /*  Check for wrap around by reading the manufacturer and device codes
            from each chip in the interleave
        */
        fValidIdCode = ReadDeviceCodes(pEMI, ulAddress, &idCodeNextChip, &bounds);
        if(fValidIdCode && DclMemCmp((D_UCHAR *)&idCode1stChip,
                     (D_UCHAR *)&idCodeNextChip, sizeof(ID_4_CYCLE)) == 0)
        {
            /*  We aliased back to the first chip (interleave) in the array
            */
            break;
        }

        /*  Send chip ID commands.  If chip ID is found, check if it is
            identical to the initial chip.
        */
        *pMedia = AMDCMD_READ_MODE;
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pLatch1 = AMDCMD_IDENTIFY;
        if(ReadDeviceCodes(pEMI, ulAddress, &idCodeNextChip, &bounds) == FALSE)
        {
            /*  the high and low words reported different ID codes
            */
            break;
        }

        /*  If this device ID not same as first device, exit
        */
        if(DclMemCmp((D_UCHAR *)&idCode1stChip, (D_UCHAR *)&idCodeNextChip, sizeof(ID_4_CYCLE)) != 0)
        {
            break;
        }

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

    /*  Record the size of the flash array that we found
    */
    pEMI->ulTotalSize = ulAddress;

    /*  attempt to allocate the FIMEXTRA structure
    */
    pEMI->pFimExtra = DclMemAllocZero(sizeof *pEMI->pFimExtra);
    if(pEMI->pFimExtra)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
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
    PFLASHDATA      pMedia;
    D_BOOL          fSuccess = TRUE;
    D_UINT16       *puDataPtr = (D_UINT16 *)pBuffer;

    DclAssert(ulStart % sizeof(D_UINT16) == 0L);
    DclAssert(uLength);
    DclAssert(pBuffer);

    /*  We always write four bytes at a time
    */
    DclAssert(uLength % sizeof(D_UINT16) == 0L);

    while(uLength)
    {
        D_UINT32    ulWindowSize;

        /*  Get the media pointer and the max size we can access with it
        */
        ulWindowSize = FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia);
        if(!ulWindowSize)
        {
            fSuccess = FALSE;
            break;
        }

        /*  Start from a known state
        */
        *pMedia = AMDCMD_READ_MODE;

        /*  Move each window worth of data into the flash memory
            (Allow WriteBufferedBytes to handle buffer alignment to page)
        */
        while(ulWindowSize && uLength)
        {
            D_UINT32    ulThisLength = DCLMIN(uLength, ulWindowSize);

#if BUFFERED_WRITES
            fSuccess = WriteBufferedBytes(pEMI, ulStart, puDataPtr, ulThisLength);
#else
            fSuccess = WriteBytes(pEMI, ulStart, puDataPtr, ulThisLength);
#endif
            if(!fSuccess)
            {
                DclError();
                break;
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
            puDataPtr       += ulThisLength / FLASH_BUS_BYTES;

            DclAssert(ulStart);
            DclAssert(puDataPtr);
        }

        if(!fSuccess)
            break;
    }

    /*  Be sure we leave the flash in the read mode
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
    PFIMEXTRA       pFimExtra;
    PFLASHDATA      pMedia;
    PFLASHDATA      pBase;      /*  media pointer at base latch */
    PFLASHDATA      pLatch1;    /*  media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;    /*  media pointer at latch offset 2 */

    DclAssert(pEMI);
    pFimExtra = pEMI->pFimExtra;
    DclAssert(pFimExtra);
    (void)ulLength;

    /*  ulBaseLatch is used to send commands and read status from
        the applicable erase zone.  This ensures that we are writing
        to the correct chip (if the array has multiple linear
        chips) or hardware partition (for RWW devices).
    */
    ulBaseLatch = ulStart & (~(pEMI->ulEraseZoneSize - 1));

    /*  To guarantee that we send all commands to the same erase zone,
        the LATCH_OFFSET values must be less than the erase zone size.
    */
    DclAssert(LATCH_OFFSET1 < pEMI->ulEraseZoneSize);
    DclAssert(LATCH_OFFSET2 < pEMI->ulEraseZoneSize);

    /*  Get media pointers
    */
    if(!FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch, (volatile void **)&pBase))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET1, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET2, (volatile void **)&pLatch2))
        return FALSE;

    /*  Send erase commands
    */
    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_ERASE1;
    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pBase = AMDCMD_ERASE2;

    /*  Set the timeout for the operation now that it's started.
    */
    DclTimerSet(&pFimExtra->tErase, ERASE_TIMEOUT);

    /*  Record the erase address for ErasePoll(), EraseSuspend, and
        EraseResume() to use.
    */
    pFimExtra->ulEraseStart = ulStart;

    /*  Clear the erase result to signify erase in progress.
    */
    pFimExtra->ulEraseResult = ERASE_IN_PROGRESS;

    /*  Remember the media address, save repeated calls to
        FfxFimNorWindowMap() from ErasePoll().
    */
    pFimExtra->pMedia = pMedia;

    /*  Erase Start Worked
    */
    return TRUE;

}

#ifdef INCLUDE_ERASE_SUSPEND

/*-------------------------------------------------------------------
    EraseSuspend()

    Description
        Suspend an erase operation currently in progress, and return
        the flash to normal read mode.  When this function returns,
        the flash may be read.

        If the flash does not support suspending erases, this
        function is not implemented, and the EraseSuspend entry in
        the FIMDEVICE structure must be NULL.

    Parameters
        pEMI     - A pointer to the ExtndMediaInfo structure to use

    Return Value
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseSuspend(
    PEXTMEDIAINFO   pEMI)
{
    D_BOOL          fResult = TRUE;
    PFIMEXTRA       pFimExtra;
    PFLASHDATA      pMedia;
    DCLTIMER        timer;

    DclAssert(pEMI);
    pFimExtra = pEMI->pFimExtra
    DclAssert(pFimExtra);
    pMedia = pFimExtra->pMedia;

    /*  Save the remaining timeout period.
    */
    pFimExtra->ulTimeoutRemaining = DclTimerRemaining(&pFimExtra->tErase);

    /*  put flash in erase suspend mode
    */
    *pMedia = AMDCMD_ERASE_SUSPEND;

    /*  Wait a while for the flash to go into erase suspend.
    */
    DclTimerSet(&timer, ERASE_SUSPEND_TIMEOUT);
    while(!DclTimerExpired(&timer))
    {
        if((*pMedia & AMDSTAT_DONE) == AMDSTAT_DONE)
            break;
    }

    /*  Check the status after a possible timeout.  A higher priority
        thread could have preempted between setting the timer or
        checking the status in the loop and checking for expiration.
    */
    if((*pMedia & AMDSTAT_DONE) != AMDSTAT_DONE)
    {
        /*  It really timed out.  This is a Bad Thing.  Record the failure.
        */
        DclError();
        pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
        fResult = FALSE;
    }

    /*  Return the flash to Read Array mode whether or not the suspend
        command appeared to have worked (it can't hurt).
    */
    *pMedia = AMDCMD_READ_MODE;

    return fResult;
}


/*-------------------------------------------------------------------
    EraseResume()

    Description
        Resumes an erase that was successfully suspended by
        EraseSuspend().  Once it is resumed, the only FIM functions
        that can be called are EraseSuspend() and ErasePoll().

        If the flash does not support suspending erases, this
        function is not implemented, and the EraseSuspend entry
        in the FIMDEVICE structure must be NULL.

    Parameters
        pEMI     - A pointer to the ExtndMediaInfo structure to use

    Return Value
        None
-------------------------------------------------------------------*/
static void EraseResume(
    PEXTMEDIAINFO   pEMI)
{
    PFIMEXTRA       pFimExtra;
    PFLASHDATA      pMedia;

    DclAssert(pEMI);
    pFimExtra = pEMI->pFimExtra;
    DclAssert(pFimExtra);

    /*  If an error occurred in EraseSuspend() the final result of the
        erase was already recorded.  Only operate on the flash if this
        hasn't happened yet.
    */
    if(pFimExtra->ulEraseResult == ERASE_IN_PROGRESS)
    {
        /*  Erases are suspended to perform other operations, so it's
            necessary to remap the window now.
        */
        if(!FfxFimNorWindowMap(pEMI->hDev, pFimExtra->ulEraseStart, (volatile void **)&pMedia))
            return;

        pFimExtra->pMedia = pMedia;

        /*  It's possible that one or both of the chips finished its
            erase before EraseSuspend was called.

            If one of the chips is showing error status, there's no point
            in trying to resume.
        */
        switch (*pMedia & AMDSTAT_MASK)
        {
            case (AMDSTAT_DONE | AMDSTAT_ERASEBEGUN):

                /*  erase is done on both chips ErasePoll() will report it
                */
                break;
            case (AMDSTAT_DONE):

                /*  both chips are not done erasing resume them both
                */
                *pMedia = AMDCMD_ERASE2;
                break;
            default:

                /*  Presumably errored out in some fashion.  ErasePoll() will
                    report that the erase failed.
                */
                DclError();
                break;
        }

        /*  Guarantee a minimum erase increment.
        */
        _sysdelay(MINIMUM_ERASE_INCREMENT);

        /*  Restart the timer.  Note that this will be done in the (unlikely)
            case that both chips had already finished when they were suspended.
            This is harmless, as ErasePoll() checks their status before checking
            for timeout.
        */
        DclTimerSet(&pFimExtra->tErase, pFimExtra->ulTimeoutRemaining);
    }
}

#endif /* INCLUDE_ERASE_SUSPEND */

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
    PFIMEXTRA       pFimExtra;
    PFLASHDATA      pMedia;
    D_UINT32        ulFlashStatus;
    D_UINT32        ulToggleCheck;

    DclAssert(pEMI);
    pFimExtra = pEMI->pFimExtra;
    DclAssert(pFimExtra);

    /*  ErasePoll() may be called multiple times even after the
        operation has completed.  Only check the flash if it
        has not already been seen to have finished its operation.
    */
    if(pFimExtra->ulEraseResult == ERASE_IN_PROGRESS)
    {
        /*  The erase was still in progress the last time it was checked.
            It is expected to have been left in Read Status mode by the
            last call to ErasePoll() or EraseResume().  Don't issue a
            Read Status command here (think about what happens if the
            flash was unexpectedly reset).

            The erase is not done until both chips' DQ6 bit stops toggling
            DQ6 was chosen to use to poll for erase completion because it was
            found that using DQ7 sometimes yieled incorrect results
        */
        pMedia = pFimExtra->pMedia;

        /*  initial read of flash status
        */
        ulFlashStatus = *pMedia;

        /*  second read to check if toggled
        */
        ulToggleCheck = *pMedia;
        if((ulFlashStatus & AMDSTAT_TOGGLEDONE) == (ulToggleCheck & AMDSTAT_TOGGLEDONE))
        {
            /*  The erase has completed successfully.  One erase zone
                has been erased.
            */
            pFimExtra->ulEraseResult = pEMI->ulEraseZoneSize;

            /*  return the flash to
                Read Array mode.
            */
            *pMedia = AMDCMD_READ_MODE;
        }
        else if(DclTimerExpired(&pFimExtra->tErase))
        {
            /*  Operation might have just finished
            */
            ulFlashStatus = *pMedia;
            ulToggleCheck = *pMedia;
            if((ulFlashStatus & AMDSTAT_TOGGLEDONE) != (ulToggleCheck & AMDSTAT_TOGGLEDONE))
            {
                DclError();

                /*  The erase hasn't finished, and the timeout has elapsed.
                    Try to return the flash to Read Array mode by suspending
                    the erase, and return a failure indication.

                    The extra check of status after the timeout check may
                    look a little odd -- wasn't it just checked above?  This
                    is to handle perverse platforms on which power management
                    can suspend the CPU any old time, but the flash keeps
                    erasing.  Yes, this really happens.
                */
#ifdef INCLUDE_ERASE_SUSPEND
                EraseSuspend(pEMI);
#endif /*INCLUDE_ERASE_SUSPEND*/
                pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
            }
        }
        else
        {
            /*  Check for error status in the low word device
            */
            if(((ulFlashStatus & AMD_DONE_LOW) != AMD_DONE_LOW) &&
                (ulFlashStatus & AMD_FAIL_LOW))
            {
                /*  Separate read to make sure the operation hasn't just completed
                */
                ulFlashStatus = *pMedia;
                if((ulFlashStatus & AMD_DONE_LOW) != AMD_DONE_LOW)
                {
                    DclError();
                    pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
                }
            }
        }
    }

    return pFimExtra->ulEraseResult;
}


/*------------------------------------------------------------------------
    DidOperationComplete()

    Description
        Polls until the ready bit (DQ6) is set, the operation fails, or
        the timeout has expired.

    Parameters
        pMedia         - The current location in the media
        ulExpectedData - The data expected when the operation is done
                         This will be 0xFFFFFFFF for an erase.

    Returns
        TRUE  - If the operation completed successfully
        FALSE - If the operation timed out (or failed)
------------------------------------------------------------------------*/
static D_BOOL DidOperationComplete(
    PFLASHDATA      pMedia,
    D_UINT32        ulExpectedData,
    D_BOOL          fSleep,
    DCLTIMER       *ptimer)
{
    D_UINT32        ulFlashStatus;
    D_UINT32        ulToggleCheck;

    /* Depending on build options, parameter may not be referenced,
     * avoid compiler warning.
     */
    (void) ulExpectedData;

    /*  initial read of media to get state of DQ6
    */
    ulFlashStatus = *pMedia;
    ulToggleCheck = *pMedia;

    /*  Wait till the byte is done programming - indicated when
        data bit 6 stops toggling.
    */
    while((ulFlashStatus & AMDSTAT_TOGGLEDONE) != (ulToggleCheck & AMDSTAT_TOGGLEDONE))
    {
        if(fSleep)
        {
            DclOsSleep(0);
        }

        if(DclTimerExpired(ptimer))
        {
            /*  Operation might have just finished
            */
            ulFlashStatus = *pMedia;
            ulToggleCheck = *pMedia;
            if((ulFlashStatus & AMDSTAT_TOGGLEDONE) != (ulToggleCheck & AMDSTAT_TOGGLEDONE))
            {
                FFXPRINTF(1, ("DidOpComp: Expected Data: %lX  Status Read: %lX\n",
                                        ulExpectedData, ulFlashStatus));
                return FALSE;
            }
        }

        /*  subsequent reads of flash status - to be evaluated by while statement
        */
        ulFlashStatus = *pMedia;
        ulToggleCheck = *pMedia;

    }

    /*  You might think it's a good idea to check that the flash status we just
        read was equivalent to the expected value, but that requires another
        read of the media.

        From the Am29LV640MH/L data sheet (revision E), pg 34:
        "Even if the device has completed the program or erase operation and
        DQ7 has valid data, the data outputs on DQ0-DQ6 may be still invalid.
        Valid data on DQ0-DQ7 will appear on successive read cycles."
    */

    return TRUE;
}


/*------------------------------------------------------------------------
    HandleProgramFailure()

    Description
       Determines the type of error and which device(s) failed (if any)
       in order to handle the failure appropriately

    Parameters
       pProgramMedia  - The memory address of the current location in the
                        media that is being programmed.
       ulStart        - The offset in the flash (before WindowMap).
       ulExpectedData - The data expected when the operation is done.

    Return Value
       TRUE  - If the operation completed successfully afterall
       FALSE - If the device was reset appropriately
------------------------------------------------------------------------*/
static D_BOOL HandleProgramFailure(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    PFLASHDATA      pProgramMedia,
    D_UINT32        ulExpectedData)
{
    D_UINT32        ulFlashStatus;
    D_UINT32        ulBaseLatch;
    D_UINT16        uAbortMask;
    PFLASHDATA      pMedia = NULL;
    PFLASHDATA      pLatch1 = NULL;
    PFLASHDATA      pLatch2 = NULL;

    /*  read flash status
    */
    ulFlashStatus = *pProgramMedia;
    FFXPRINTF(2, ("HPF:Status Read:   %lX\n", ulFlashStatus));

    if(!FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;

    /*  initialize to indicate no Write-to-Buffer Abort detected
    */
    uAbortMask = 0;

    /*  Check for Write-to-Buffer Abort
    */
    if(ulFlashStatus & AMD_BUFF_ABORT_LOW)
    {
        uAbortMask = LOW_WORD;
    }

    /*  Separate read to make sure the operation hasn't just completed
    */
    ulFlashStatus = *pProgramMedia;
    FFXPRINTF(2, ("HPF2:Status Read:   %lX\n", ulFlashStatus));
    if((ulFlashStatus & AMDSTAT_DONE) == (ulExpectedData & AMDSTAT_DONE))
    {
        /*  Operation completed successfully
        */
        return TRUE;
    }

    if(!uAbortMask)            /* no Write-to-Buffer Abort detected */
    {
        /*  Clear the error status, reset to read mode and return
        */
        *pMedia = AMDCMD_READ_MODE;
        return FALSE;
    }

    /*  According to the "DQ1: Write-to-Buffer Abort" section of the
        AMD MirrorBit data sheets, "The system must issue the
        Write-to-Buffer-Abort-Reset command sequence to return the
        device to reading array data.

        See the Am29LV640MH/L data sheet (revision E) pg 37.
    */
    ulBaseLatch = ulStart & (~(pEMI->ulEraseZoneSize - 1));

    /*  Only abort reset the device(s) that are in the abort state.  Send
        the normal reset (read array) command to the other device (if only
        one device is in the abort state).
    */
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET1, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET2, (volatile void **)&pLatch2))
        return FALSE;

    *pLatch1 = (AMDCMD_UNLOCK1 & uAbortMask) | (AMDCMD_READ_MODE & ~uAbortMask);
    *pLatch2 = (AMDCMD_UNLOCK2 & uAbortMask) | (AMDCMD_READ_MODE & ~uAbortMask);
    *pLatch1 = AMDCMD_READ_MODE;

    return FALSE;
}


/*------------------------------------------------------------------------
    ReadDeviceCodes()

    Description
       Reads the manufacturer and device codes from each chip in the
       interleave (high and low word devices).

    Parameters
       ulAddress - The current location in the media
       lpChipID  - Location to save the manufacturer and device ID codes.
                   Note that these value are only valid if the return value
                   is TRUE.
    Return Value
       TRUE  - If all the high and low ID codes are identical
       FALSE - If any of the high and low ID codes are different
------------------------------------------------------------------------*/
static D_BOOL ReadDeviceCodes(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulAddress,
    ID_4_CYCLE     *lpChipID,
    FFXFIMBOUNDS   *pBounds)
{
    D_UINT16        uIdCodes;
    /*D_UINT        uLowChipID;*/
    PFLASHDATA      pMedia = NULL;


    /*  read and check the manufacturer codes (both devices)
    */
    if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, pBounds, (volatile void **)&pMedia))
        return FALSE;
    uIdCodes = *(D_UINT16 *)pMedia;
    lpChipID->uManufCode = uIdCodes;

    /*  read and check the first device codes (both devices)
    */
    if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + DEVICE_CODE1_OFFSET_X16, pBounds, (volatile void **)&pMedia))
        return FALSE;
    uIdCodes = *(D_UINT16 *)pMedia;
    lpChipID->uDeviceCode1 = uIdCodes;

    /*  read and check the second device codes (both devices)
    */
    if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + DEVICE_CODE2_OFFSET_X16, pBounds, (volatile void **)&pMedia))
        return FALSE;
    uIdCodes = *(D_UINT16 *)pMedia;
    lpChipID->uDeviceCode2 = uIdCodes;


    /*  read and check the third device codes (both devices)
    */
    if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + DEVICE_CODE3_OFFSET_X16, pBounds, (volatile void **)&pMedia))
        return FALSE;
    uIdCodes = *(D_UINT16 *)pMedia;
    lpChipID->uDeviceCode3 = uIdCodes;

    /*  return TRUE if the codes are identical
    */
    return TRUE;
}


#if BUFFERED_WRITES

/*------------------------------------------------------------------------
    WriteBufferedBytes()

    Description
       Write the specified bytes to flash using "buffered" write commands.
       ulStart and ulLength must specify memory within one window.

    Parameters
       ulStart  - The offset in bytes to begin programming data
       puData   - The current location in the data
       ulLength - The length to write

    Return Value
       TRUE  - If successful
       FALSE - If failed
------------------------------------------------------------------------*/
static D_BOOL WriteBufferedBytes(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16       *puData,
    D_UINT32        ulLength)
{
    D_UINT16        uBufferIndex;
    D_UINT16        uDWordIndex;
    D_UINT32        ulBaseLatch;
    PFLASHDATA      pMedia = NULL;
    PFLASHDATA      pLatch1 = NULL;
    PFLASHDATA      pLatch2 = NULL;
    D_UCHAR         ucByteCount;
    DCLTIMER        timer;

    /*  ulBaseLatch is used to send commands and read status from
        the applicable erase zone.  This ensures that we are writing
        to the correct chip (if the array has multiple linear
        chips) or hardware partition (for RWW devices).
    */
    ulBaseLatch = ulStart & (~(pEMI->ulEraseZoneSize - 1));

    /*  To guarantee that we send all commands to the same erase zone,
        the LATCH_OFFSET values must be less than the erase zone size.
    */
    DclAssert(LATCH_OFFSET1 < pEMI->ulEraseZoneSize);
    DclAssert(LATCH_OFFSET2 < pEMI->ulEraseZoneSize);

    /*  Get media pointers.

        We don't have to worry about the window size because that is already
        handled by Write().  It is probably the case that we do not need to do
        this mapping, but that level of attention and reworking of the code
        is beyond the scope of the window mapping update.
    */
    if(!FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET1, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET2, (volatile void **)&pLatch2))
        return FALSE;

    /*  Program ulLength bytes into flash via buffer
    */
    for(uBufferIndex = 0;
        uBufferIndex < (ulLength / FLASH_BUS_BYTES);
        uBufferIndex += (ucByteCount / FLASH_BUS_BYTES))
    {
        /*  Write command sequence coded in-line since timing critical
        */
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        pMedia[uBufferIndex] = AMDCMD_WRITETOBUFFER;

        /*  Write buffer data must not cross write-buffer pages.
            See the Am29LV640MH/L data sheet (revision E) pg 26.
            Make sure the first byte count fits in a write-buffer page.
        */
        if((uBufferIndex == 0) && (ulStart % WRITE_BUFFER_PAGE_SIZE) != 0)
        {
            ucByteCount = (D_UCHAR) (WRITE_BUFFER_PAGE_SIZE -
                                     (ulStart % WRITE_BUFFER_PAGE_SIZE));
        }
        else
        {
            ucByteCount = WRITE_BUFFER_PAGE_SIZE;
        }

        /*  The last (or only) buffer might not be maximum size.
        */
        if((ulLength - (uBufferIndex * FLASH_BUS_BYTES)) < ucByteCount)
        {
            ucByteCount = (D_UCHAR) (ulLength - (uBufferIndex * FLASH_BUS_BYTES));
        }

        /*  In 32 bit mode, we write out dwords instead of bytes and the number
            must be written to both the low word and high word.
        */
        pMedia[uBufferIndex] = ((ucByteCount / FLASH_BUS_BYTES) - 1);

        /*  Write Buffer Data
        */
        for(uDWordIndex = 0;
            uDWordIndex < (ucByteCount / FLASH_BUS_BYTES);
            uDWordIndex++)
        {
            pMedia[uBufferIndex + uDWordIndex] = puData[uBufferIndex + uDWordIndex];
        }

        /*  Write Buffer Program Confirm command (Program Buffer to Flash)
        */
        pMedia[uBufferIndex] = AMDCMD_BUFFERTOFLASH;

        /*  Must wait 4 microseconds before we can begin to poll
        */
        _sysdelay(10);          /* Using 10 to be on safe side */

        DclTimerSet(&timer, WRITE_TIMEOUT);

        /* Check if write completed by checking last address loaded into buffer
        */
        if(!DidOperationComplete(&pMedia[uBufferIndex + uDWordIndex - 1],
                                 puData[uBufferIndex + uDWordIndex - 1], FALSE, &timer))
        {
            if(!HandleProgramFailure(pEMI, ulStart,
                 &pMedia[uBufferIndex + uDWordIndex - 1], puData[uBufferIndex + uDWordIndex - 1]))
            {
                DclError();
                return FALSE;
            }
        }

    }

    return TRUE;
}


#else /* not BUFFERED_WRITES */

/*------------------------------------------------------------------------
    WriteBytes()

    Description
       Write the specified bytes to flash using "word" program commands.

    Parameters
       ulStart  - The offset in bytes to begin programming data
       puData   - The current location in the data
       ulLength - The length to write

    Return Value
       TRUE  - If successful
       FALSE - If failed
------------------------------------------------------------------------*/
static D_BOOL WriteBytes(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT32       *puData,
    D_UINT32        ulLength)
{
    D_UINT16        uProgramIndex;
    D_UINT32        ulBaseLatch;
    PFLASHDATA      pMedia;
    PFLASHDATA      pLatch1;
    PFLASHDATA      pLatch2;
    DCLTIMER        timer;

    /*  ulBaseLatch is used to send commands and read status from
        the applicable erase zone.  This ensures that we are writing
        to the correct chip (if the array has multiple linear
        chips) or hardware partition (for RWW devices).
    */
    ulBaseLatch = ulStart & (~(pEMI->ulEraseZoneSize - 1));

    /*  To guarantee that we send all commands to the same erase zone,
        the LATCH_OFFSET values must be less than the erase zone size.
    */
    DclAssert(LATCH_OFFSET1 < pEMI->ulEraseZoneSize);
    DclAssert(LATCH_OFFSET2 < pEMI->ulEraseZoneSize);

    if(!FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET1, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET2, (volatile void **)&pLatch2))
        return FALSE;

#if UNLOCK_BYPASS

    /*  Use the unlock bypass command to save some bus cycles
    */
    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_UNLOCK_BYPASS;
#endif

    /*  Divide ulLength (in bytes) by the number of bytes per write
    */
    for(uProgramIndex = 0;
        uProgramIndex < (ulLength / sizeof(D_UINT16));
        ++uProgramIndex, ++puData)
    {
        /*  Write command sequence coded in-line since timing critical
        */
#if !UNLOCK_BYPASS
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
#endif
        *pLatch1 = AMDCMD_PROGRAM;

        pMedia[uProgramIndex] = *puData;

        DclTimerSet(&timer, WRITE_TIMEOUT);

        if(!DidOperationComplete(&pMedia[uProgramIndex], *puData, FALSE, &timer))
        {
            if(!HandleProgramFailure(pEMI, ulStart, &pMedia[uProgramIndex], *puData))
            {
                DclError();
                return FALSE;
            }
        }
    }

#if UNLOCK_BYPASS
    /*  Clear (reset) the unlock bypass mode
    */
    *pMedia = AMDCMD_IDENTIFY;
    *pMedia = AMDCMD_BYPASS_RESET;
#endif

    return TRUE;
}

#endif


/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_ambx16 =
{
    Mount,
    Unmount,
    Read,
    Write,
    EraseStart,
    ErasePoll,
#ifdef INCLUDE_ERASE_SUSPEND
    EraseSuspend,
    EraseResume
#else
    NULL,
    NULL
#endif /*INCLUDE_ERASE_SUSPEND*/
};

