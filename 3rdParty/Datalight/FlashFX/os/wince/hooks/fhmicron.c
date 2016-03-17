/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains the FlashFX Hooks Layer implementations for the
    Micron NTM.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    NOTE:  This is a sample hooks module for use with the ntmicron NTM,
           intended to be customized for the given target hardware.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmicron.c $
    Revision 1.12  2010/07/16 15:31:27Z  garyp
    Updated so the hooks module provides the maximum number of chips
    count to the NTM.
    Revision 1.11  2010/07/04 21:46:53Z  garyp
    Added the function FfxHookNTMicronConfigure().  Refactored to work
    in multichip configurations.
    Revision 1.10  2010/06/25 20:06:34Z  glenns
    Add support for Micron M60 boot blocks
    Revision 1.9  2010/01/10 22:34:26Z  garyp
    Updated to specify the alignment requirement.
    Revision 1.8  2009/12/13 06:05:04Z  garyp
    Updated so the entire module is conditioned on FFXCONF_NANDSUPPORT.
    Revision 1.7  2009/12/13 00:14:43Z  garyp
    Major update to support more configurable project settings.  The entire
    spare area format is now under control of the hooks interface.  Added the
    function FfxHookNTMicronGetPageStatus().
    Revision 1.6  2009/10/14 01:23:18Z  keithg
    Removed now obsolete fxio.h include file.
    Revision 1.5  2009/10/06 22:47:44Z  garyp
    Added the FfxHookNTMicronPageRead/Write() functions.  Minor code cleanup.
    Documentation updated.
    Revision 1.4  2009/09/24 19:45:20Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2009/04/08 20:32:49Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.2  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/09/13 00:24:44Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <nandconf.h>
#include <nandctl.h>
#include <fhmicron.h>
#include <ecc.h>

/*  Should this module be copied into the Project Directory and customized
    by the OEM, the following "virtual header" could be split out into a
    separate header file, should the settings need to be shared by more 
    than one module.
*/
/*-------------------------------------------------------------------------*/
/*                          Virtual Header START                           */
/*                                                                         */  
    /*  The "M60" Settings are meaningful only if Micron M60 flash is being
        used, and they mark the chip/block boundary at which the M60 on-die
        ECC should be used.  This allows the blocks preceding that boundary
        to use the legacy 1-bit ECC, as is required by a number of platform
        specific BOOTROMs and vendor flash programming tools.  Setting these
        values both to zero means that the M60 on-die ECC will be used on 
        the entire array (if the flash is M60).
    */        
    #define MAX_CHIPS                 (1)   /* Max chips supported -- CS0 to CS(MAX_CHIPS-1) */
    #define SUPPORT_CACHE_MODE_READS  TRUE  /* Must be TRUE for checkin */  
    #define SUPPORT_CACHE_MODE_WRITES TRUE  /* Must be TRUE for checkin */  
    #define SUPPORT_DUAL_PLANE_OPS    TRUE  /* Must be TRUE for checkin */  
    #define M60_EDC4_FIRST_CHIP       (0)   /* Must be zero for checkin */
    #define M60_EDC4_FIRST_BLOCK      (0)   /* Must be zero for checkin */
    
    struct sNTMHOOK
    {
        unsigned    nChip;          /* The current chip select */
        unsigned    nValidChipMask; /* Mask of valid chips per hook instance */
    };
/*                                                                         */  
/*                          Virtual Header END                             */  
/*-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------
    Public: FfxHookNTMicronCreate()

    Create a NTM Hook instance which is associated with the Micron NTM.

    Parameters:
        hDev    - The Device handle
        pParams - A pointer to the FFXMICRONPARAMS structure to use.

    Return Value:
        Returns an NTMHOOKHANDLE if successful, or NULL otherwise.
 -------------------------------------------------------------------*/
NTMHOOKHANDLE FfxHookNTMicronCreate(
    FFXDEVHANDLE        hDev,
    FFXMICRONPARAMS    *pParams)
{
    NTMHOOKHANDLE       hNtmHook;

    (void)hDev;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0), "FfxHookNTMicronCreate()\n"));

    DclAssert(hDev);
    DclAssertWritePtr(pParams, sizeof(*pParams));
    DclAssert(pParams->nStructLen == sizeof(*pParams));

    hNtmHook = DclMemAllocZero(sizeof(*hNtmHook));
    if(!hNtmHook)
    {
        FFXPRINTF(1, ("FfxHookNTMicronCreate() Unable to allocate memory!\n"));
        return NULL;
    }

    /*  Initialize the current chip select to an illegal value
    */
    hNtmHook->nChip = UINT_MAX;

    pParams->nMaxChips                  = MAX_CHIPS;
    pParams->nTagOffset                 = NSOFFSETZERO_TAG_OFFSET;
    pParams->fSupportCacheModeReads     = SUPPORT_CACHE_MODE_READS;
    pParams->fSupportCacheModeWrites    = SUPPORT_CACHE_MODE_WRITES;
    pParams->fSupportDualPlanes         = SUPPORT_DUAL_PLANE_OPS;
    pParams->nAlignment                 = sizeof(NAND_IO_TYPE);
    pParams->nM60EDC4Chip               = M60_EDC4_FIRST_CHIP;
    pParams->ulM60EDC4Block             = M60_EDC4_FIRST_BLOCK;

    return hNtmHook;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronDestroy()

    Destroy a Micron NTM Hook instance.

    Parameters:
        hNtmHook - The hook handle returned from the Create() function.

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronDestroy(
    NTMHOOKHANDLE hNtmHook)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronDestroy() hNtmHook=%P\n", hNtmHook));

    DclAssertReadPtr(hNtmHook, sizeof(hNtmHook));

    return DclMemFree(hNtmHook);
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronConfigure()

    Configure an NTM Hook instance for the given chip.  If the NTM
    instance supports multiple chips, this function will be called 
    for each chip.

    Parameters:
        hNtmHook  - The hook handle returned from the Create() function.
        nChip     - The chip being configured.
        pChipInfo - A pointer to the FFXNANDCHIP structure to use
        pabID     - A pointer to an array of NAND ID bytes, which will
                    contain at least NAND_ID_SIZE entries.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronConfigure(
    NTMHOOKHANDLE       hNtmHook,
    unsigned            nChip,
    const FFXNANDCHIP  *pChipInfo,
    D_BYTE             *pabID)
{
    FFXSTATUS           ffxStat = FFXSTAT_SUCCESS;
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronConfigure() hNtmHook=%P Chip=%u pChipInfo=%P pabID=%P\n", 
        hNtmHook, nChip, pChipInfo, pabID));

    DclAssertWritePtr(hNtmHook, sizeof(hNtmHook));
    DclAssertReadPtr(pChipInfo, sizeof(*pChipInfo));
    DclAssertWritePtr(pabID, NAND_ID_SIZE);
    DclAssert(nChip < MAX_CHIPS);
    DclAssert(!DCLBITGET(&hNtmHook->nValidChipMask, nChip));

    /*  Mark the bit for this chip so we know subsequent
        operations on it are valid.
    */        
    DCLBITSET(&hNtmHook->nValidChipMask, nChip);

    /*  Any other per-chip initialization code goes here.
    */

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronSetChipSelect()

    Select a particular NAND chip.  Any control bits are set to
    zero and all other chips are deselected.

    Parameters:
        hNtmHook - The NTM Hook handle
        nChip    - The chip number 
        nMode    - The mode to use, either MODE_READ or MODE_WRITE

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronSetChipSelect(
    NTMHOOKHANDLE       hNtmHook,
    unsigned            nChip,
    CHIP_SELECT_MODE    nMode)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronSetChipSelect() hNtmHook=%P OldChip=%u NewChip=%u Mode=%s\n",
         hNtmHook, hNtmHook->nChip, nChip, nMode == MODE_WRITE ? "MODE_WRITE" : "MODE_READ"));

    DclAssert(hNtmHook);
    DclAssert(nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, nChip));
    */        

    (void)nMode;

    hNtmHook->nChip = nChip;

    /*  Replace this with real code...
    */
    DclProductionError();

    /*
    if(all good)
        return FFXSTAT_SUCCESS;
    else
    */
        return FFXSTAT_FIMRANGEINVALID;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronReadyWait()

    Poll the BSY/RDY pin of the NAND chip until it indicates that
    the device is ready or a timeout limit is reached.  This is
    often called before a command is written to the flash to be
    sure the device will listen before the command is sent.  The
    chip selects must/will be set prior to this function call.
    This function could look at the busy pins of the device or
    send a read status command.

    The timeout limit is specified by NAND_TIME_OUT in nandconf.h.

    Parameters:
        hNtmHook - The NTM Hook handle

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronReadyWait(
    NTMHOOKHANDLE       hNtmHook)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronReadyWait() hNtmHook=%P Chip=%u\n", hNtmHook, hNtmHook->nChip));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    (void)hNtmHook;                  /* unused */

    /*  Replace this with real code...
    */
    DclProductionError();

#if 0
    /*  Outline of typical implementation:
    */
    {
        DCLTIMER    t;

        DclTimerSet(&t, NAND_TIME_OUT);

        while(!DclTimerExpired(&t))
        {
            if(chip is ready)
                return FFXSTAT_SUCCESS;
        }

        /*  We MUST have one more check after exiting the while() loop
            because in a multithreaded system we could have timed out
            due to other threads being scheduled, but still completed
            the operation successfully.
        */
        return (chip is still not ready) ? FFXSTAT_FIMTIMEOUT : FFXSTAT_SUCCESS;
    }
    /*  End of typical implementation
    */
#endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronSetChipCommand()

    Wait for the chip to be ready and then send a command using
    FfxHookNTMicronSetChipCommandNoWait().

    Parameters:
        hNtmHook  - The NTM Hook handle
        bCommand  - The command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronSetChipCommand(
    NTMHOOKHANDLE   hNtmHook,
    D_BYTE          bCommand)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronSetChipCommand() hNtmHook=%P Chip=%u Cmd=%02x\n", 
        hNtmHook, hNtmHook->nChip, bCommand));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    ffxStat = FfxHookNTMicronReadyWait(hNtmHook);

    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = FfxHookNTMicronSetChipCommandNoWait(hNtmHook, bCommand);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronSetChipCommandNoWait()

    Send a command to the chip without waiting for status.

    Universal "send arbitrary command" helper routine used by all
    higher level chip-specific routines. This routine only takes
    care of making sure that the proper interface pins are
    manipulated so that the command is entered into a previously
    selected chip.  The appropriate chip must already be selected
    before calling this routine and subsequent address,
    confirmation and data bytes must be sent to the chips using
    calls to additional helper routines.

    Parameters:
        hNtmHook  - The NTM Hook handle
        bCommand  - The command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronSetChipCommandNoWait(
    NTMHOOKHANDLE   hNtmHook,
    D_BYTE          bCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronSetChipCommandNoWait() hNtmHook=%P Chip=%u Cmd=%02x\n", 
        hNtmHook, hNtmHook->nChip, bCommand));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    (void)hNtmHook;                  /* unused */
    (void)bCommand;

    /*  Replace this with real code...
    */
    DclProductionError();

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronDataIn()

    Perform a platform-dependent stream input operation from the
    NAND data port and return the result.

    Parameters:
        hNtmHook - The NTM Hook handle
        pBuffer  - A pointer to a buffer to be filled with the data
                   read from the device
        nCount   - Number of NAND_IO_TYPE elements in the buffer
                   to be read from the device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTMicronDataIn(
    NTMHOOKHANDLE   hNtmHook,
    NAND_IO_TYPE   *pBuffer,
    unsigned        nCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicronDataIn() hNtmHook=%P Chip=%u pBuff=%P Count=%u\n", 
        hNtmHook, hNtmHook->nChip, pBuffer, nCount));

    DclAssert(hNtmHook);
    DclAssert(pBuffer);
    DclAssert(nCount);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    (void)hNtmHook;             /* unused */
    (void)pBuffer;              /* unused */
    (void)nCount;               /* unused */

    /*  Replace this with real code...
    */
    DclProductionError();

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronDataOut()

    Perform a platform-dependent stream output operation to the
    NAND data port, storing the provided data.

    Parameters:
        hNtmHook - The NTM Hook handle
        pBuffer  - A pointer to a buffer containing the data to be
                   sent to the device
        nCount   - Number of NAND_IO_TYPE elements in the buffer
                   to be sent to the device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTMicronDataOut(
    NTMHOOKHANDLE       hNtmHook,
    const NAND_IO_TYPE *pBuffer,
    unsigned            nCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicronDataOut() hNtmHook=%P Chip=%u pBuff=%P Count=%u\n", 
        hNtmHook, hNtmHook->nChip, pBuffer, nCount));

    DclAssert(hNtmHook);
    DclAssert(pBuffer);
    DclAssert(nCount);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    /*  Force the chip to NO IDLE Mode while Reads and Writes are happening.
        Otherwise the GPMC goes to idle mode and some time does n't respond as 
        required.  Problem identified when enabled both Prefetch and Hardware 
        ECC modes together.
    */

    /*  Avoid compiler warnings
    */
    (void)hNtmHook;
    (void)pBuffer;
    (void)nCount;

    DclProductionError();

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronPageRead()

    Perform a platform-dependent page read, with or without ECC.

    If a pabECC pointer is provided, the buffer must be large enough
    to contain all the ECC bytes generated for during the operation,
    based on the style of ECC scheme being used.  See the functions
    FfxHookEccCalcStart() and FfxHookEccCalcRead().

    Parameters:
        hNtmHook  - The NTM Hook handle
        pBuffer   - A pointer to a buffer to be filled with the data
                    read from the device.
        nCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be read from the device.
        pabECC    - A pointer to an array of bytes to receive the ECC
                    for the page, or NULL if no ECC is to be performed.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronPageRead(
    NTMHOOKHANDLE   hNtmHook,
    NAND_IO_TYPE   *pBuffer,
    unsigned        nCount,
    D_BUFFER       *pabECC)
{
    size_t          nBytes = nCount * sizeof(*pBuffer);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTMicronPageRead() hNtmHook=%P Chip=%u pBuff=%P Count=%u pECCBuff=%P\n",
        hNtmHook, hNtmHook->nChip, pBuffer, nCount, pabECC));

    DclAssert(hNtmHook);
    DclAssertWritePtr(pBuffer, nCount * sizeof(*pBuffer));
    DclAssert(nCount);
    DclAssert((nBytes % DATA_BYTES_PER_ECC) == 0);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    if(pabECC)
        FfxHookEccCalcStart(hNtmHook, (const D_BUFFER*)pBuffer, nBytes, pabECC, ECC_MODE_READ);

    FfxHookNTMicronDataIn(hNtmHook, pBuffer, nCount);

    if(pabECC)
        FfxHookEccCalcRead(hNtmHook, (const D_BUFFER*)pBuffer, nBytes, pabECC, ECC_MODE_READ);

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronPageWrite()

    Perform a platform-dependent page write, with or without ECC.

    If a pSpare pointer is provided, ECC will be calculated and
    placed into the spare area at the standard locations for 
    OffsetZero style flash.

    Parameters:
        hNtmHook  - The NTM Hook handle
        pData     - A pointer to a buffer containing the data to be
                    sent to the device
        nCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be sent to the device
        pSpare    - A pointer to the spare area in which to place the
                    ECC for the data, or NULL if no ECC is to be 
                    performed.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronPageWrite(
    NTMHOOKHANDLE       hNtmHook,
    const NAND_IO_TYPE *pData,
    unsigned            nCount,
    D_BUFFER           *pSpare)
{
    size_t              nBytes = nCount * sizeof(*pData);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTMicronPageWrite() hNtmHook=%P Chip=%u pData=%P Count=%u pSpare=%P\n",
        hNtmHook, hNtmHook->nChip, pData, nCount, pSpare));

    DclAssert(hNtmHook);
    DclAssertReadPtr(pData, nCount * sizeof(*pData));
    DclAssert(nCount);
    DclAssert((nBytes % DATA_BYTES_PER_ECC) == 0);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    if(!pSpare)
    {
        FfxHookNTMicronDataOut(hNtmHook, pData, nCount);
    }
    else
    {
        D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
        size_t      nECCLen = (nBytes / DATA_BYTES_PER_ECC) * BYTES_PER_ECC;

        FfxHookEccCalcStart(hNtmHook, (const D_BUFFER*)pData, nBytes, abECC, ECC_MODE_WRITE);

        FfxHookNTMicronDataOut(hNtmHook, pData, nCount);

        FfxHookEccCalcRead(hNtmHook, (const D_BUFFER*)pData, nBytes, abECC, ECC_MODE_WRITE);

        /*  Place the ECC data into the spare area in the standard
            OffsetZero style.
        */            
        FfxNtmOffsetZeroSpareAreaECCSet(pSpare, abECC, nECCLen);   
    }
    
    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronStatusIn()

    Perform a single platform-dependent input operation from the
    NAND data port, for a status read, and returns the result.

    Parameters:
        hNtmHook - The NTM Hook handle

    Return Value:
        The status data read from the chip.
-------------------------------------------------------------------*/
NAND_IO_TYPE FfxHookNTMicronStatusIn(
    NTMHOOKHANDLE       hNtmHook)
{
    NAND_IO_TYPE        tStatus = 0;
    
    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    (void)hNtmHook;                  /* unused */

    /*  Replace this with real code...

    tStatus = ???
    */
    
    DclProductionError();

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicronStatusIn() hNtmHook=%P Chip=%u returning status %x\n", 
        hNtmHook, hNtmHook->nChip, tStatus));

    return tStatus;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronAddressOut()

    Perform a single platform-dependent output operation to the
    NAND address port, storing the provided address value.

    Parameters:
        hNtmHook - The NTM Hook handle
        bAddress - The data to store to the address port

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTMicronAddressOut(
    NTMHOOKHANDLE   hNtmHook,
    D_BYTE          bAddress)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronAddressOut() hNtmHook=%P Chip=%u Addr=%02x\n", 
        hNtmHook, hNtmHook->nChip, bAddress));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    /*  Avoid compiler warnings
    */
    (void)hNtmHook;
    (void)bAddress;

    /*  Replace this with real code...
    */
    DclProductionError();

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronGetPageStatus()

    Extract the page status bits from the supplied spare area data.

    Parameters:
        hNtmHook - The NTM Hook handle
        pSpare   - A pointer to a buffer containing the spare area 
                   data.

    Return Value:
        Returns a D_UINT32 containing the page status bits.
-------------------------------------------------------------------*/
D_UINT32 FfxHookNTMicronGetPageStatus(
    NTMHOOKHANDLE   hNtmHook,
    const D_BUFFER *pSpare)
{
    DclAssert(hNtmHook);
    DclAssert(pSpare);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    return FfxNtmHelpGetPageStatus(pSpare, NSOFFSETZERO_TAG_OFFSET, NSOFFSETZERO_FLAGS_OFFSET);
}


#endif  /* FFXCONF_NANDSUPPORT */




