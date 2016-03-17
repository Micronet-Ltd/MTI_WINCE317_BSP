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
                                Descripton

    This module contains the FIM code for AMD uniform sector flash which is
    organized in a 4x8 interleaved fashion.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: asu4x8.c $
    Revision 1.9  2009/04/03 05:40:39Z  keithg
    Fixed bug 2521: removed obsolete READ_BACK_VERIFY.
    Revision 1.8  2007/11/02 02:09:22Z  Garyp
    Eliminated the inclusion of limits.h.
    Revision 1.7  2006/10/02 17:57:51Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.6  2006/08/31 19:23:09Z  pauli
    Corrected an inverted condition when scanning for multiple chips.
    Revision 1.5  2006/08/31 04:58:55Z  pauli
    Updated to use the new FfxFimNorWindowCreate/FfxFimNorWindowMap
    functions.  Eliminated the use of the AMD_SEND_CMD macro.
    Revision 1.4  2006/05/20 20:35:18Z  Garyp
    Minor fixes to deal with renamed or obsolete structure fields.
    Revision 1.3  2006/02/11 00:13:01Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.2  2006/02/08 18:21:16Z  Garyp
    Modified to use new FfxHookWindowMap/Size() functions.  Updated debugging
    code.
    Revision 1.1  2005/10/14 02:07:54Z  Pauli
    Initial revision
    Revision 1.13  2005/05/13 17:47:21Z  garyp
    Cleaned up the erase zone size define.
    Revision 1.12  2005/05/12 16:33:04Z  garyp
    Updated to use the standard flash ID process.
    Revision 1.11  2005/05/10 13:25:04Z  garyp
    Modified to use generalized AMD_SEND_CMD macro and standard data types.
    Revision 1.10  2005/05/08 16:19:16Z  garyp
    Updated to use new AMD flash command macros.  Formatted for readability.
    No functional changes.
    Revision 1.9  2005/01/23 00:43:26Z  billr
    Can't include limits.h in a kernel build.
    Revision 1.8  2004/12/30 23:00:17Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.7  2004/09/17 18:29:04Z  jaredw
    Removed checking in erase resume to see if flash chips had already finished
    erase.  Sending the erase command to a finished chip has no effect.
    Revision 1.6  2004/09/15 18:16:03Z  jaredw
    Fixed bug where flash was being reset before array search loop
    causing wrap-around to not be detected.
    Revision 1.5  2004/08/05 16:33:32Z  billr
    Move optional functions to the end of FIMDEVICE.
    Revision 1.4  2004/08/04 21:55:42Z  billr
    Eliminate ulInterleaved in ExtndMediaInfo, no longer used or needed.
    Revision 1.3  2004/08/04 21:24:36Z  billr
    Eliminate ulFimFlags in ExtndMediaInfo, no longer used or needed.
    Revision 1.2  2004/07/16 21:57:08Z  jaredw
    Fixed bug when one chip was done erasing erase suspend wrongly
    detected an error (AMD_STATUS_MASK) wrong value was A8 now 88
    Revision 1.1  2004/07/06 20:10:12Z  jaredw
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fimdev.h>
#include "nor.h"

/*  Configure for a 4x8 layout
*/
#define FLASH_INTERLEAVE    4       /* Number of flash chips interleaved    */
#define FLASH_BUS_WIDTH     32      /* Overall data bus width in bits       */

#include "flashcmd.h"
#include "amdcmd.h"

/*  ulEraseResult value; used internally to clarify intent.
*/
#define ERASE_IN_PROGRESS       (0)

/*  The minimum amount of time (in microseconds) to let a suspended
    erase progress to ensure that some forward progress is made.
*/
#define MINIMUM_ERASE_INCREMENT (1000)

/*  For AMD Am29F080B up to 1 million cycles 8 seconds
*/
#define ERASE_TIMEOUT           (10 * 1000UL)

/*  AMD Am29F080B 300us
*/
#define WRITE_TIMEOUT           (1) /* 500us */

/*  Erase suspend timeout: <1ms ~20us
*/
#define ERASE_SUSPEND_TIMEOUT   (10)

/*  If this is set to TRUE, then some alternate code (DidOperationComplete)
    will be called after program and erase operations.  This code will
    check for error status from each chip.  This is only useful in debug
    builds, when trying to track down errors.

    This define must appear before the prototypes.
*/
#define FULL_ERROR_CHECKING   FALSE

#if FULL_ERROR_CHECKING
static D_BOOL DidOperationComplete(PFLASHDATA pMedia, D_UINT32 ulExpectedData, DCLTIMER * ptimer);
#endif

#define AMD_DONE_CHIP0      MAKEFLASHCMD(0x0, 0x0, 0x0, ASTAT_OK)
#define AMD_DONE_CHIP1      MAKEFLASHCMD(0x0, 0x0, ASTAT_OK, 0x0)
#define AMD_DONE_CHIP2      MAKEFLASHCMD(0x0, ASTAT_OK, 0x0, 0x0)
#define AMD_DONE_CHIP3      MAKEFLASHCMD(ASTAT_OK, 0x0, 0x0, 0x0)
#define AMD_FAIL_CHIP0      MAKEFLASHCMD(0x0, 0x0, 0x0, ASTAT_FAIL)
#define AMD_FAIL_CHIP1      MAKEFLASHCMD(0x0, 0x0, ASTAT_FAIL, 0x0)
#define AMD_FAIL_CHIP2      MAKEFLASHCMD(0x0, ASTAT_FAIL, 0x0, 0x0)
#define AMD_FAIL_CHIP3      MAKEFLASHCMD(ASTAT_FAIL, 0x0, 0x0, 0x0)

/*  Latch address offsets for am29F080x4 devices
*/
#define LATCH_OFFSET1       0x1554UL
#define LATCH_OFFSET2       0x0AA8UL

/*  All parts supported in this FIM have 64KB erase zones --
    multiply appropriately for the interleave value.
*/
#define ZONE_SIZE           (0x00010000UL * FLASH_INTERLEAVE) /* 256KB */

/*  macro to wait for ready status after program operation
*/
#if FULL_ERROR_CHECKING
#define WAIT_FOR_WRITE_DONE(MediaPtr, CurrentData, pt)                      \
   if(!DidOperationComplete((MediaPtr), (CurrentData), (pt)))              \
   {                                                                        \
      DclError();                                                           \
                                                                            \
      *MediaPtr = AMDCMD_READ_MODE;                                         \
      return FALSE;                                                         \
    }
#else
#define WAIT_FOR_WRITE_DONE(MediaPtr, CurrentData, pt)                      \
   while((*(MediaPtr) & AMDSTAT_DONE) != ((CurrentData) & AMDSTAT_DONE))   \
   {                                                                        \
      if(DclTimerExpired(pt))                                               \
      {                                                                     \
         /* One more check for done in case we got scheduled out */         \
         if((*(MediaPtr) & AMDSTAT_DONE) != ((CurrentData) & AMDSTAT_DONE)) \
         {                                                                  \
            DclError();                                                     \
                                                                            \
            *MediaPtr = AMDCMD_READ_MODE;                                   \
            return FALSE;                                                   \
         }                                                                  \
      }                                                                     \
   }
#endif

typedef struct tagFIMEXTRA
{
    int             iSuspend;
    DCLTIMER        tErase;
    D_UINT32        ulEraseStart;
    D_UINT32        ulEraseResult;
    D_UINT32        ulTimeoutRemaining;
    PFLASHDATA      pMedia;
} FIMEXTRA;

static AMDCHIPPARAMS ChipTable[] =
{
    {ID_AM29F080(0)},
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
    D_UINT32        ulAddress = 0UL;
    PFLASHDATA      pMedia;
    PFLASHDATA      pLatch1;   /*  media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;   /*  media pointer at latch offset 2 */
    unsigned        i;
    FLASHIDCODES    ID;
    FFXFIMBOUNDS    bounds;

    DclAssert(pEMI);

    pEMI->uDeviceType = DEV_NOR | DEV_MERGE_WRITES | DEV_NOT_MLC;
    pEMI->ulEraseZoneSize = ZONE_SIZE;

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
        FFXPRINTF(1, ("FIM asu4x8:   Invalid flash ID: Data0/1=%08lx/%08lx\n",
            ID.data0, ID.data1));

        *pMedia = AMDCMD_READ_MODE;

        return FALSE;
    }

    /*  search the device codes list for the detected ID code
    */
    for(i = 0; ChipTable[i].uIDFlags != IDFLAG_ENDOFLIST; i++)
    {
        if((ID.idDev == ChipTable[i].idDev) &&
            (ID.idMfg == ChipTable[i].idMfg))
        {
            pEMI->ulDeviceSize = ChipTable[i].ulChipSize * FLASH_INTERLEAVE;

            FFXPRINTF(1, ("FIM asu4x8:   Mfg/Dev=%02x/%02x supported\n", ID.idMfg, ID.idDev));

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
        FFXPRINTF(1, ("FIM asu4x8:   Mfg/Dev=%02x/%02x unsupported\n", ID.idMfg, ID.idDev));

        /*  If the device is not supported, restore the original state
            and return device not found.
        */
        *pMedia = AMDCMD_READ_MODE;

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
        FLASHIDCODES    ID2;

        /*  The mount address must always be a multiple of the zone size.
        */
        DclAssert(!(ulAddress % pEMI->ulEraseZoneSize));

        /*  Get media pointers at this address.
        */
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
            break;
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + LATCH_OFFSET1, &bounds, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + LATCH_OFFSET2, &bounds, (volatile void **)&pLatch2))
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

    /*  Record the size of the flash array that we found
    */
    pEMI->ulTotalSize = ulAddress;

    pEMI->pFimExtra = DclMemAllocZero(sizeof *pEMI->pFimExtra);
    if(!pEMI->pFimExtra)
        return FALSE;

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
    D_UINT16        uProgramIndex;
    D_UINT32       *pulDataPtr = (D_UINT32 *)pBuffer;
    PFLASHDATA      pMedia;     /*  media pointer where data is written */
    PFLASHDATA      pLatch1;    /*  media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;    /*  media pointer at latch offset 2 */
    D_BOOL          fSuccess = TRUE;
    DCLTIMER        timer;

    DclAssert(pEMI);
    DclAssert(ulStart % sizeof(PFLASHDATA) == 0L);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert(ulStart < pEMI->ulTotalSize);
    DclAssert(pEMI->ulTotalSize - ulStart >= uLength);

    /*  To guarantee that we send all commands to the same erase zone,
        the LATCH_OFFSET values must be less than the erase zone size.
    */
    DclAssert(LATCH_OFFSET1 < pEMI->ulEraseZoneSize);
    DclAssert(LATCH_OFFSET2 < pEMI->ulEraseZoneSize);

    while(uLength)
    {
        D_UINT32    ulWindowSize;

        /*  ulBaseLatch is used to send commands and read status from
            the applicable erase zone.  This ensures that we are writing
            to the correct chip (if the array has multiple linear
            chips) or hardware partition (for RWW devices).
        */
        ulBaseLatch = ulStart & (~(ZONE_SIZE - 1));

        /*  Get a media pointer at LATCH_OFFSET1
        */
        if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET1, (volatile void **)&pLatch1))
        {
            fSuccess = FALSE;
            break;
        }

        /*  Get a media pointer at LATCH_OFFSET2
        */
        if(!FfxFimNorWindowMap(pEMI->hDev, ulBaseLatch | LATCH_OFFSET2, (volatile void **)&pLatch2))
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

            /*  Program ulThisLength / 4 bytes into flash
            */
            for(uProgramIndex = 0; uProgramIndex <
                (ulThisLength / FLASH_BUS_BYTES); ++uProgramIndex)
            {
                /*  Write command sequence coded in-line since timing critical
                */
                *pLatch1 = AMDCMD_UNLOCK1;
                *pLatch2 = AMDCMD_UNLOCK2;
                *pLatch1 = AMDCMD_PROGRAM;

                pMedia[uProgramIndex] = pulDataPtr[uProgramIndex];
                DclTimerSet(&timer, WRITE_TIMEOUT);

                /*  Reset flash and return FALSE if the operation fails
                */
                WAIT_FOR_WRITE_DONE(&pMedia[uProgramIndex], pulDataPtr[uProgramIndex], &timer);
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
            pulDataPtr      += ulThisLength / FLASH_BUS_BYTES;

            DclAssert(ulStart);
            DclAssert(pulDataPtr);
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
    ulBaseLatch = ulStart & (~(ZONE_SIZE - 1));

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
    DclTimerSet(&pEMI->pFimExtra->tErase, ERASE_TIMEOUT);

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

    return TRUE;
}


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

    DclAssert(pEMI);
    pFimExtra = pEMI->pFimExtra;
    DclAssert(pFimExtra);
    pMedia = pFimExtra->pMedia;

    /*  Save the remaining timeout period.
    */
    pFimExtra->ulTimeoutRemaining = DclTimerRemaining(&pFimExtra->tErase);

    *pMedia = AMDCMD_ERASE_SUSPEND;

    /*  Wait a while for the flash to go into erase suspend.
    */
    DclTimerSet(&pFimExtra->tErase, ERASE_SUSPEND_TIMEOUT);
    while(!DclTimerExpired(&pFimExtra->tErase))
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

        /*  It's possible that one or more of the chips finished its
            erase before EraseSuspend was called, however with AMD flash
            sending the erase resume command to a chip that has already
            finished has no effect.  Send Erase Resume command in all cases
            Erase Poll will report any error situations.
        */
        *pMedia = AMDCMD_ERASE2;

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

    DclAssert(pEMI);
    pFimExtra = pEMI->pFimExtra;
    DclAssert(pFimExtra);

    /*  ErasePoll() may be called multiple times even after the
        operation has completed.  Only check the flash if it
        has not already been seen to have finished its operation.
    */
    if(pFimExtra->ulEraseResult == ERASE_IN_PROGRESS)
    {
        /*  The erase is not done until all chips' status registers have
            the DONE bit set.
        */
        pMedia = pFimExtra->pMedia;

        /*  initial read of flash status
        */
        ulFlashStatus = *pMedia;
        if((ulFlashStatus & AMDSTAT_DONE) == AMDSTAT_DONE)
        {
            /*  The erase has completed successfully.  One erase zone
                has been erased.
            */
            pFimExtra->ulEraseResult = ZONE_SIZE;

            /*  return the flash to Read Array mode.
            */
            *pMedia = AMDCMD_READ_MODE;
        }
        else if(DclTimerExpired(&pFimExtra->tErase) && (*pMedia & AMDSTAT_DONE) != AMDSTAT_DONE)
        {
            /*  The erase hasn't finished, and the timeout has elapsed.
                Try to return the flash to Read Array mode by suspending
                the erase, and return a failure indication.

                The extra check of status after the timeout check may
                look a little odd -- wasn't it just checked above?  This
                is to handle perverse platforms on which power management
                can suspend the CPU any old time, but the flash keeps
                erasing.  Yes, this really happens.
            */
            EraseSuspend(pEMI);
            DclError();
            pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
        }
        else
        {
            /*  Check for error status in the high byte device
            */
            if(((ulFlashStatus & AMD_DONE_CHIP3) != AMD_DONE_CHIP3)
               && (ulFlashStatus & AMD_FAIL_CHIP3))
            {
                /*  According to flowchart diagrams in the AMD data sheets,
                    we must do a separate read to make sure the operation hasn't
                    just completed.

                */
                ulFlashStatus = *pMedia;
                if((ulFlashStatus & AMD_DONE_CHIP3) != AMD_DONE_CHIP3)
                {
                    DclError();
                    pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
                }
            }

            /*  Check for error status in the byte number 2 device
            */
            if(((ulFlashStatus & AMD_DONE_CHIP2) != AMD_DONE_CHIP2)
               && (ulFlashStatus & AMD_FAIL_CHIP2))
            {
                /*  Separate read to make sure the operation hasn't just completed
                */
                ulFlashStatus = *pMedia;
                if((ulFlashStatus & AMD_DONE_CHIP2) != AMD_DONE_CHIP2)
                {
                    DclError();
                    pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
                }
            }

            /*  Check for error status in the byte number 1 device
            */
            if(((ulFlashStatus & AMD_DONE_CHIP1) != AMD_DONE_CHIP1)
               && (ulFlashStatus & AMD_FAIL_CHIP1))
            {
                /*  Separate read to make sure the operation hasn't just completed
                */
                ulFlashStatus = *pMedia;
                if((ulFlashStatus & AMD_DONE_CHIP1) != AMD_DONE_CHIP1)
                {
                    DclError();
                    pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
                }
            }

            /*  Check for error status in the low byte device
            */
            if(((ulFlashStatus & AMD_DONE_CHIP0) != AMD_DONE_CHIP0)
               && (ulFlashStatus & AMD_FAIL_CHIP0))
            {
                /*  Separate read to make sure the operation hasn't just completed
                */
                ulFlashStatus = *pMedia;
                if((ulFlashStatus & AMD_DONE_CHIP0) != AMD_DONE_CHIP0)
                {
                    DclError();
                    pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
                }
            }
        }
    }
    return pFimExtra->ulEraseResult;
}


#if FULL_ERROR_CHECKING

/*------------------------------------------------------------------------
    DidOperationComplete()

    Description
        Polls until the ready bit is set, the operation fails, or the
        timeout has expired.

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
    DCLTIMER       *ptimer)
{
    D_UINT32        ulFlashStatus;

    /*  initial read of flash status
    */
    ulFlashStatus = *pMedia;

    /*  Wait till the byte is done programming - indicated when
        data bit 7 equals original data bit 7
    */
    while((ulFlashStatus & AMDSTAT_DONE) != (ulExpectedData & AMDSTAT_DONE))
    {
        /*  Check for error status in the high byte device
        */
        if(((ulFlashStatus & AMD_DONE_CHIP3) != (ulExpectedData & AMD_DONE_CHIP3))
           && (ulFlashStatus & AMD_FAIL_CHIP3))
        {
            /*  According to flowchart diagrams in the AMD data sheets,
                we must do a separate read to make sure the operation hasn't
                just completed.
            */
            ulFlashStatus = *pMedia;
            if((ulFlashStatus & AMD_DONE_CHIP3) != (ulExpectedData & AMD_DONE_CHIP3))
            {
                DclError();
                return FALSE;
            }
        }

        /*  Check for error status in the byte number 2 device
        */
        if(((ulFlashStatus & AMD_DONE_CHIP2) != (ulExpectedData & AMD_DONE_CHIP2))
           && (ulFlashStatus & AMD_FAIL_CHIP2))
        {
            /*  Separate read to make sure the operation hasn't just completed
            */
            ulFlashStatus = *pMedia;
            if((ulFlashStatus & AMD_DONE_CHIP2) != (ulExpectedData & AMD_DONE_CHIP2))
            {
                DclError();
                return FALSE;
            }
        }

        /*  Check for error status in the byte number 1 device
        */
        if(((ulFlashStatus & AMD_DONE_CHIP1) != (ulExpectedData & AMD_DONE_CHIP1))
           && (ulFlashStatus & AMD_FAIL_CHIP1))
        {
            /*  Separate read to make sure the operation hasn't just completed
            */
            ulFlashStatus = *pMedia;
            if((ulFlashStatus & AMD_DONE_CHIP1) != (ulExpectedData & AMD_DONE_CHIP1))
            {
                DclError();
                return FALSE;
            }
        }

        /*  Check for error status in the low byte device
        */
        if(((ulFlashStatus & AMD_DONE_CHIP0) != (ulExpectedData & AMD_DONE_CHIP0))
           && (ulFlashStatus & AMD_FAIL_CHIP0))
        {
            /*  Separate read to make sure the operation hasn't just completed
            */
            ulFlashStatus = *pMedia;
            if((ulFlashStatus & AMD_DONE_CHIP0) != (ulExpectedData & AMD_DONE_CHIP0))
            {
                DclError();
                return FALSE;
            }
        }

        if(DclTimerExpired(ptimer))
        {
            /*  Operation might have just finished
            */
            ulFlashStatus = *pMedia;
            if((ulFlashStatus & AMDSTAT_DONE) != (ulExpectedData & AMDSTAT_DONE))
            {
                DclError();
                return FALSE;
            }
        }

        /*  subsequent read of flash status - to be evaluated by while statement
        */
        ulFlashStatus = *pMedia;

    }

    /*  You might think it's a good idea to check that the flash status we just
        read was equivalent to the expected value, but that requires another
        read of the media.

        From the Am29LV640D/Am29LV641D data sheet (revision B+4), pg 29:
        "Even if the device has completed the program or erase operation and
        DQ7 has valid data, the data outputs on DQ0-DQ6 may be still invalid.
        Valid data on DQ0-DQ7 will appear on successive read cycles."

        From NEC: "Dual Operation Flash Memory 32M Bits A Series",
        Document No. M14914EJ3V0IF00 (3rd edition) October 2000, pg 30:
        "Upon completion of automatic program/erase, after the data output
        to I/O7 changes from the complement to the true value, I/O7 changes
        asynchronously like I/O0 to I/O6 while /OE is maintained at low
        level."
    */
    return TRUE;
}


#endif /* if FULL_ERROR_CHECKING */


/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_asu4x8 =
{
    Mount,
    Unmount,
    Read,
    Write,
    EraseStart,
    ErasePoll,
    EraseSuspend,
    EraseResume
};

