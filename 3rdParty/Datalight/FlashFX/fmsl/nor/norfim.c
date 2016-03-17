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


---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: norfim.c $
    Revision 1.35  2010/09/13 21:06:18Z  glenns
    Fix bug 2774: Correct boolean conditional in DidWriteComplete
    where checking if buffered-write feature is supported.
    Revision 1.34  2010/06/09 17:19:55Z  glenns
    Fix erroneous DclMemFree call resulting from mount error.
    Revision 1.33  2010/04/28 16:42:39Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.32  2010/02/19 00:25:42Z  glenns
    Fix typo in comments for last checkin.
    Revision 1.31  2010/01/30 21:06:54Z  glenns
    Fix Bug 2830: Refactor code to account for CFI rules regarding
    calculation of CFI, command and address latch offsets; fix Bug
    2099: provied a way for IdChip, Create and GetChipData and
    Create procedures to know whether AMD- or  Intel-style command
    sets are working; fix Bug 2774: gate check of BUFFER_ABORT 
    errors in DidWriteComplete on whether FIM supports buffered
    writes; removed redundant DclAssert call; refactored Create 
    procedure to eliminate possible memory leaks; fixed up comment
    styles and cleaned up hard tabs.
    Revision 1.30  2009/09/10 17:59:50Z  glenns
    Fix Bugzilla #2829: Added a conditional compile directive to gate
    out devices that have 16-bit ID codes from the norfim chip table
    when the flash bus width is only 8 bits.
    Revision 1.29  2009/04/06 14:22:46Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.28  2009/04/03 15:58:38Z  glenns
    - Fix Bugzilla #1253: Assign the ffxStat element of the FFXIOSTATUS
      return value to FFXSTAT_FIM_UNSUPPORTEDIOREQUEST
      in the default cases of the IORequest switch statement.
    Revision 1.27  2009/04/03 05:40:40Z  keithg
    Fixed bug 2521: removed obsolete READ_BACK_VERIFY.
    Revision 1.26  2008/05/28 17:30:53Z  Glenns
    Fix for Bugzilla #1993- removed an assertion from ErasePoll
    function that was testing state information that should not be
    tested at that point. See discussion in Bugzilla #1993.
    Revision 1.25  2008/05/01 19:56:22Z  Glenns
    Modified the write procedures and "DidWriteComplete" to support
    a more accurate representation the standard flowchart for how
    AMD-style devices detect write completion.
    Revision 1.24  2008/03/25 21:01:20Z  Garyp
    Removed an unnecessary "OptionGet" call.  Cleaned up the function
    headers.  Removed hard tabs.
    Revision 1.23  2008/02/03 02:02:18Z  keithg
    comments updated to support autodoc
    Revision 1.22  2008/01/30 23:28:06Z  Glenns
    Removed a change from last checkin which was there for testing purposes
    only.
    Revision 1.21  2008/01/30 21:48:07Z  Glenns
    Added support for AMD Advanced Sector Protection block-locking mechanism.
    Revision 1.20  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.19  2007/10/11 17:02:07Z  pauli
    Updated to initialize the FIMINFO.uPagesPerBlock field if the identified
    part has a page size (i.e. Sibley).
    Revision 1.18  2007/10/01 23:13:45Z  pauli
    Resolved Bug 355: Updated to handle misaligned data buffers and report a
    required alignment of 1.  Eliminated PNORFIMDATA.  Removed inappropriate
    casts of pointers to PFLASHDATA.
    --- Added comments ---  pauli [2007/10/01 23:25:41Z]
    Resolved Bug 1488.
    Revision 1.17  2007/08/02 23:14:50Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.16  2007/06/12 21:37:05Z  rickc
    Added 2nd and 3rd device IDs to the lookup table, factored out duplicate
    part listings.
    Revision 1.15  2007/04/17 22:54:17Z  rickc
    Switch undefined NOR config from compile-time to run-time failure
    Revision 1.14  2007/03/02 21:01:45Z  timothyj
    Added computation of simulated uPagesPerBlock to FFXPARAM_PAGE_SIZE handler.
    Revision 1.13  2007/03/01 20:23:51Z  timothyj
    Modified call to FfxDevApplyArrayBounds() to pass and receive on return
    a return block index in lieu of byte offset, in support of NAND LFA
    development.
    Revision 1.12  2007/02/06 19:50:13Z  rickc
    Added Samsung part, removed local constant INCLUDE_ERASE_SUSPEND
    Revision 1.11  2007/02/01 03:30:56Z  Garyp
    Updated to allow erase-suspend support to be compiled out of the product.
    Revision 1.10  2007/01/31 00:10:15Z  rickc
    Added default configuration
    Revision 1.9  2007/01/30 23:43:03Z  rickc
    Added support for x4 interleave
    Revision 1.8  2007/01/26 20:02:52Z  rickc
    Fixed compile warnings
    Revision 1.7  2007/01/24 20:10:30Z  rickc
    Added Samsung parts
    Revision 1.6  2007/01/24 19:00:52Z  rickc
    Added Samsung parts to table
    Revision 1.5  2007/01/10 19:05:24Z  rickc
    Fixed build warning
    Revision 1.4  2007/01/09 23:16:22Z  rickc
    Fixed compile error
    Revision 1.3  2007/01/09 20:10:55Z  rickc
    Fixed compile error
    Revision 1.2  2007/01/08 18:57:42Z  rickc
    Added JEDEC ID ability for parts not CFI-compliant, general rename from
    "CFI" to more-fitting "NOR" names, fixed problem with EraseResume
    Revision 1.1  2007/01/02 22:45:30Z  rickc
    Initial revision
    Previous revision history from original files cfix16.c and cfi2x16.c
    Revision 1.11  2006/12/29 00:00:00Z  rickc
    Removed primary table ver 0.0, cleaned up code dealing with this
    info for readability
    Revision 1.10  2006/12/28 22:53:56Z  rickc
    Fixed problem with unlocking Intel boot blocks, code cleanup for readability
    Revision 1.9  2006/12/28 00:09:20Z  rickc
    EraseResume returns error code if fails
    Revision 1.8  2006/12/14 19:53:20Z  rickc
    Added Intel Standard command set, fixed write buffer size bug
    Revision 1.7  2006/12/13 15:46:39Z  rickc
    Fixed Hugarian notation
    Revision 1.6  2006/12/12 23:03:01Z  rickc
    Clean-up compile warnings for MSVC6 tools
    Revision 1.5  2006/12/07 20:17:05Z  rickc
    Added support for Intel M18 flash
    Revision 1.4  2006/11/27 22:25:18Z  rickc
    Build only if NOR or Sibley support is included
    Revision 1.3  2006/11/27 16:40:22Z  rickc
    Fixed build problems
    Revision 1.2  2006/11/21 20:54:31Z  rickc
    Fixed WriteBytes() for Intel flash
    Revision 1.1  2006/11/17 23:35:10Z  rickc
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT   /* File-wide */

#include <fimdev.h>
#include "nor.h"

#ifndef FFXCONF_NORCONFIG
#define FFXCONF_NORCONFIG NORFLASH_DEFAULT
#endif

/*  This stuff must appear before including norfim.h, because norfim.h
    includes flashcmd.h, which performs compile-time calculations using
    FLASH_BUS_WIDTH.
*/

#if FFXCONF_NORCONFIG == NORFLASH_x8
    #define FLASH_INTERLEAVE    1
    #define FLASH_BUS_WIDTH     8
#elif FFXCONF_NORCONFIG == NORFLASH_2x8
    #define FLASH_INTERLEAVE    2
    #define FLASH_BUS_WIDTH     16
#elif FFXCONF_NORCONFIG == NORFLASH_4x8
    #define FLASH_INTERLEAVE    4
    #define FLASH_BUS_WIDTH     32
#elif FFXCONF_NORCONFIG == NORFLASH_x16
    #define FLASH_INTERLEAVE    1
    #define FLASH_BUS_WIDTH     16
#elif FFXCONF_NORCONFIG == NORFLASH_2x16
    #define FLASH_INTERLEAVE    2
    #define FLASH_BUS_WIDTH     32
#else
    #define FLASH_INTERLEAVE    1
    #define FLASH_BUS_WIDTH     16
#endif

#include "norfim.h"


/*  Chip table is used to define parts that either are not CFI compliant or
    do not use CFI in the ways used in this FIM.  If the part is properly
    identified using this table, the FIM will not attempt to ID the chip using
    CFI commands, so this table will override the CFI info.  These parts are
    define in norfim.h
*/
static NORCHIPPARAMS NorfimChipTable[] =
{
    {ID_28F008SAL},
    {ID_28F016S5},
    {ID_AM29F080},
        
/*  If the flash bus width is only 8 bits, leave out of this list devices we 
    know to be 16 bits wide. This is important because these devices have 
    16-bit ID codes, and if we try to include them with a flash bus width of
    only 8 bits, compilers will complain about integer truncation.
*/

#if !(FLASH_CHIP_WIDTH == 8)

    {ID_S29WS128N},
    {ID_S29WS256N},
    {ID_K8A3215EBA},
    {ID_K8A3215ETA},
    {ID_K8A6415EBA},
    {ID_K8A6415ETA},
    {ID_K8A2815EBA},
    {ID_K8A2815ETA},
    {ID_K8A5615EBA},
    {ID_K8A5615ETA},
    {ID_K8C5x15EBM},
    {ID_K8C5x15ETM},
    {ID_K8C1x15EBM},
    {ID_K8C1x15ETM},
    {ID_K8D1716UBC},
    {ID_K8D1716UTC},
    {ID_K8D3216UBC},
    {ID_K8D3216UTC},
    {ID_K8D6316UBM},
    {ID_K8D6316UTM},
    {ID_K8F5x15EBM},
    {ID_K8F5x15ETM},
    {ID_K8F1x15EBM},
    {ID_K8F1x15ETM},
    {ID_K8P2x15UQB},
    {ID_K8P5615UQB},
    {ID_K8S3215EBE},
    {ID_K8S3215ETE},
    {ID_K8S6415EBE},
    {ID_K8S6415ETE},
    {ID_K8S2815EBE},
    {ID_K8S2815ETE},
    {ID_K8S5615EBE},
    {ID_K8S5615ETE},

#endif /* !(FLASH_BUS_WIDTH == 8) */

    {ID_ENDOFLIST}
};


/*-------------------------------------------------------------------
    Public: Create()

    The function creates a FIM instance.

    Parameters:
        hDev - The device handle

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
static FIMHANDLE Create(
    FFXDEVHANDLE    hDev,
    const FIMINFO **ppFimInfo)
{
    DclAssert(hDev);
    DclAssert(ppFimInfo);

    if(FFXCONF_NORCONFIG == NORFLASH_DEFAULT)
    {
        /*  If FFXCONF_NORCONFIG is not defined, this will fail at run-time.
            This is to avoid having to add this to every ffxconf.h
        */
        FFXPRINTF(1, ("NORFIM: ERROR - FFXCONF_NORCONFIG not properly defined.\n"));
        DclError();
        return NULL;
    }
    else
    {
        D_UINT32        ulAddress = 0;
        PFLASHDATA      pMedia;
        FFXFIMBOUNDS    bounds;
        NORFIMDATA     *pFim = NULL;
        NORFIMDATA     *pFimTemp = NULL;
        
        /*  Allocate memory for FIM structure, and for the pFimTemp structure
            used to scan for multiple chips in the array.        
        */
        pFim = DclMemAllocZero(sizeof *pFim);
        if(!pFim)
        {
            FFXPRINTF(1, ("NORFIM: ERROR - Can't allocate FIM structure.\n"));
            DclError();
            return NULL;
        }
        pFimTemp = DclMemAllocZero(sizeof *pFimTemp);
        if(!pFimTemp)
        {
            DclMemFree(pFim);
            FFXPRINTF(1, ("NORFIM: ERROR - Can't allocate FIM structure.\n"));
            DclError();
            return NULL;
        }

        pFim->hDev = hDev;

        /*  Get the array bounds and map the window.
        */
        FfxDevGetArrayBounds(pFim->hDev, &bounds);
        if(!FfxFimNorWindowCreate(pFim->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
            goto MountError;

        /*  Check if chip can be identified
        */
        if(!IdChip(pFim, pMedia, TRUE, TRUE, TRUE))
            goto MountError;

        /*  Set the FimInfo fields that have not already been set in IdChip().
        */
        pFim->FimInfo.uDeviceType       = pFim->uDeviceType;
        pFim->FimInfo.uDeviceFlags      = pFim->uDeviceFlags;
        pFim->FimInfo.uPageSize         = pFim->uPageSize;
        pFim->FimInfo.uSpareSize        = 0;
        pFim->FimInfo.uAlignSize        = 1; /* arbitrary buffer alignment allowed with this FIM */
        pFim->FimInfo.ulPhysicalBlocks  = pFim->ulChipSize / pFim->FimInfo.ulBlockSize;
        pFim->FimInfo.ulTotalBlocks     = pFim->ulChipSize / pFim->FimInfo.ulBlockSize;
        pFim->FimInfo.ulChipBlocks      = pFim->ulChipSize / pFim->FimInfo.ulBlockSize;

        /*  Set the pages per block if we have a page size.
        */
        if(pFim->FimInfo.uPageSize)
            pFim->FimInfo.uPagesPerBlock = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.uPageSize);

        /*  Search for additional chips
        */
        for(ulAddress = pFim->ulChipSize;
            ulAddress < MAX_ARRAY;
            ulAddress += pFim->ulChipSize)
        {
            if(!FfxFimNorWindowCreate(pFim->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
                break;


            /*  The previous chip found is still in ID mode.  If this call
                to IdChip returns TRUE, we have wrapped around in memory  and
                no more flash chips exist in the array.
            */
            if(IdChip(pFimTemp, pMedia, FALSE, pFim->fCfiCompliant, pFim->fJedecID))
                break;

            /*  Now attempt IdChip with the query command.  If this returns TRUE,
                we have found a new chip in the flash array that can be identified
            */
            if(!IdChip(pFimTemp, pMedia, TRUE, pFim->fCfiCompliant, pFim->fJedecID))
                break;

            /*  Compare chip info with original chip.  If no match, this chip is
                different and cannot be managed with this instance of the FIM.
            */
            if(!ChipInfoCompare(pFim, pFimTemp))
                break;

            /*  Place this chip back into read array mode. At this point we have
                filled in pFimTemp, so we know whether it is Intel- or AMD-style
                flash.
            */
            if (pFimTemp->fAmdFlash)
            {
                *pMedia = AMDCMD_READ_MODE;
            }

            else if(pFimTemp->fIntelFlash)
            {
                *pMedia = INTLCMD_READ_MODE;
                *pMedia = INTLCMD_CLEAR_STATUS;
                *pMedia = INTLCMD_READ_MODE;
            }

            /*  Add the appropriate data from this newly found flash chip into
                the FimInfo structure.
            */
            pFim->FimInfo.ulPhysicalBlocks  += pFim->ulChipSize / pFim->FimInfo.ulBlockSize;
            pFim->FimInfo.ulTotalBlocks     += pFim->ulChipSize / pFim->FimInfo.ulBlockSize;
        }

        /*  Temporary FIM structure no longer needed. Release it.
        */
        if (pFimTemp)
        {
            DclMemFree(pFimTemp);
            pFimTemp = NULL;
        }

        /*  Place the first chip back into read array mode. At this point we have
            filled in the pFim, so we know whether it is Intel- or AMD-style flash.
        */
        if(!FfxFimNorWindowCreate(pFim->hDev, 0, &bounds, (volatile void **)&pMedia))
            goto MountError;
        if (pFim->fAmdFlash)
        {
            *pMedia = AMDCMD_READ_MODE;
        }

        else if(pFim->fIntelFlash)
        {
            *pMedia = INTLCMD_READ_MODE;
            *pMedia = INTLCMD_CLEAR_STATUS;
            *pMedia = INTLCMD_READ_MODE;
        }

        /*  Unlock blocks if needed
        */
        if(pFim->eFimBlockLockSupport)
        {
            if(!FimUnlockBlocks(ulAddress, &bounds, pFim))
            {
                DclError();
                goto MountError;
            }
        }

        DisplayFimInfo(pFim);

        *ppFimInfo = &pFim->FimInfo;
        return (FIMHANDLE)pFim;

      MountError:

        DisplayFimInfo(pFim);

        if(pFim)
            DclMemFree(pFim);
        if(pFimTemp)
            DclMemFree(pFimTemp);
        return NULL;
    }
}


/*-------------------------------------------------------------------
    Public: Destroy()

    Destroy a FIM instance and release any allocated resources.

    Parameters:
        pFim - The FIM instance handle

    Return Value:
        None
-------------------------------------------------------------------*/
static void Destroy(
    FIMHANDLE pFim)
{
    DclAssert(pFim);
    DclMemFree(pFim);
    return;
}


/*-------------------------------------------------------------------
    Public: ParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        pFim      - The FIM instance handle
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the buffer in which to store the
                    value.  May be NULL.
        ulBuffLen - The size of buffer.  May be zero if pBuffer is
                    NULL.

    Return Value:
        TRUE if the parameter ID is valid, the parameter value is
        available and either pBuffer is NULL or ulBuffLen is the
        appropriate size for the option value, otherwise FALSE.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterGet(
    FIMHANDLE   pFim,
    FFXPARAM    id,
    void       *pBuffer,
    D_UINT32    ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NORFIM: ParameterGet() pFim=%P ID=%x pBuff=%P Len=%lU\n",
        pFim, id, pBuffer, ulBuffLen));

    FFXPRINTF(1, ("NORFIM: ParameterGet() Unsupported FFXPARAM ID %u\n", id));

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NORFIM: ParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: ParameterSet()

        Set a FlashFX system parameter.  If the parameter ID is not
        recognized for this layer, the request will be passed to the
        next lower software layer.

    Parameters:
        pFim      - The FIM instance handle
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        TRUE if the parameter ID is valid and successfully set,
        otherwise FALSE.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterSet(
    FIMHANDLE   hFim,
    FFXPARAM    id,
    const void *pBuffer,
    D_UINT32    ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;
    NORFIMDATA     *pFim = (NORFIMDATA*)hFim;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NORFIM: ParameterSet() pFim=%P ID=%x pBuff=%P Len=%lU\n",
        pFim, id, pBuffer, ulBuffLen));

    switch(id)
    {
        case FFXPARAM_PAGE_SIZE:
            if(ulBuffLen == sizeof pFim->FimInfo.uPageSize)
            {
                D_UINT16    uPageSize = *(D_UINT16*)pBuffer;

                DclAssert(uPageSize);
                if(pFim->FimInfo.uPageSize && (pFim->FimInfo.uPageSize != uPageSize))
                {
                    /*  The page size cannot be changed once it is set.
                    */
                    FFXPRINTF(1, ("NORFIM: ParameterSet() Page size already set. Existing=%U, New=%U\n",
                                   pFim->FimInfo.uPageSize, uPageSize));
                    break;
                }
                else
                {
                    pFim->FimInfo.uPageSize = uPageSize;
                    pFim->FimInfo.uPagesPerBlock = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.uPageSize);
                }
                ffxStat = FFXSTAT_SUCCESS;
            }
            break;

        default:
            FFXPRINTF(1, ("NORFIM: ParameterSet() Unsupported FFXPARAM ID %u\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NORFIM: ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: IORequest()

    Parameters:
        pFim - The FIM instance handle
        pIOR - A pointer to the FFXIOREQUEST structure

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS IORequest(
    FIMHANDLE       hFim,
    FFXIOREQUEST   *pIOR)
{
    FFXIOSTATUS     ioStat;
    NORFIMDATA     *pFim = (NORFIMDATA*)hFim;
    D_BOOL          fSuccess;

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEINDENT),
        "NORFIM: IORequest() pFim=%P Func=%x\n", pFim, pIOR->ioFunc));

    DclProfilerEnter("NORFIM: IORequest", 0, 0);

    DclAssert(pFim);

    DclMemSet(&ioStat, 0, sizeof ioStat);

    switch(pIOR->ioFunc)
    {

        case FXIOFUNC_FIM_ERASE_BOOT_BLOCK_START:
        {
            FFXIOR_FIM_ERASE_BOOT_BLOCK_START *pReq = (FFXIOR_FIM_ERASE_BOOT_BLOCK_START*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            pFim->fInBootBlocks = TRUE;

            if(!EraseStart(pFim,
                pReq->ulStartBB * pFim->FimInfo.ulBootBlockSize))
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }
            break;
        }

        case FXIOFUNC_FIM_ERASE_START:
        {
            FFXIOR_FIM_ERASE_START *pReq = (FFXIOR_FIM_ERASE_START*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            pFim->fInBootBlocks = FALSE;

            if(!EraseStart(pFim,
                pReq->ulStartBlock * pFim->FimInfo.ulBlockSize))
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }

            break;
        }

        case FXIOFUNC_FIM_ERASE_POLL:
        {
            FFXIOR_FIM_ERASE_POLL   *pReq = (FFXIOR_FIM_ERASE_POLL*)pIOR;
            D_UINT32                ulLength;

            (void)pReq;
            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            ulLength = ErasePoll(pFim);
            if(ulLength == ERASE_FAILED)
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                if(ulLength)
                {
                    ioStat.ulCount = 1;
                    ioStat.ffxStat = FFXSTAT_SUCCESS;
                }
                else
                {
                    ioStat.ffxStat = FFXSTAT_FIM_ERASEINPROGRESS;
                }
            }

            break;
        }

      #if FFXCONF_ERASESUSPENDSUPPORT
        case FXIOFUNC_FIM_ERASE_SUSPEND:
        {
            if(pFim->fEraseSuspendSupported)
            {
                FFXIOR_FIM_ERASE_SUSPEND *pReq = (FFXIOR_FIM_ERASE_SUSPEND*)pIOR;

                (void)pReq;
                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat.ulFlags = IOFLAGS_BLOCK;

                if(!EraseSuspend(pFim))
                    ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
                else
                    ioStat.ffxStat = FFXSTAT_SUCCESS;

                break;
            }
            else
            {
                ioStat.ulFlags = IOFLAGS_BLOCK;
                ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                break;
            }
        }

        case FXIOFUNC_FIM_ERASE_RESUME:
        {
            if(pFim->fEraseSuspendSupported)
            {
                FFXIOR_FIM_ERASE_RESUME *pReq = (FFXIOR_FIM_ERASE_RESUME*)pIOR;

                (void)pReq;
                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat.ulFlags = IOFLAGS_BLOCK;

                if(!EraseResume(pFim))
                    ioStat.ffxStat = FFXSTAT_FIM_ERASERESUMEFAILED;
                else
                    ioStat.ffxStat = FFXSTAT_SUCCESS;

                break;
            }
            else
            {
                ioStat.ulFlags = IOFLAGS_BLOCK;
                ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                break;
            }
        }
      #endif

        case FXIOFUNC_FIM_READ_GENERIC:
        {
            FFXIOR_FIM_READ_GENERIC *pReq = (FFXIOR_FIM_READ_GENERIC*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulCount);
            DclAssert(pFim->FimInfo.uPageSize);

            /*  All these operations are page ops.
            */
            ioStat.ulFlags = IOFLAGS_PAGE;

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_FIM_READ_PAGES:
                {
                    DclAssert(pReq->pPageData);

                    if(!Read(pFim,
                        pReq->ulStartPage * pFim->FimInfo.uPageSize,
                        pReq->ulCount * pFim->FimInfo.uPageSize,
                        pReq->pPageData))
                    {
                        ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                    }
                    else
                    {
                        ioStat.ffxStat = FFXSTAT_SUCCESS;
                        ioStat.ulCount = pReq->ulCount;
                    }

                    break;
                }

                default:
                {
                    FFXPRINTF(1, ("NORFIM: IORequest() Bad Read Generic SubFunction %x\n", pReq->ioSubFunc));
                    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                    DclError();
                    break;
                }
            }

            break;
        }

        case FXIOFUNC_FIM_WRITE_GENERIC:
        {
            FFXIOR_FIM_WRITE_GENERIC *pReq = (FFXIOR_FIM_WRITE_GENERIC*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulCount);
            DclAssert(pFim->FimInfo.uPageSize);

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_FIM_WRITE_PAGES:
                {
                    DclAssert(pReq->pPageData);

                    if(!Write(pFim,
                        pReq->ulStartPage * pFim->FimInfo.uPageSize,
                        pReq->ulCount * pFim->FimInfo.uPageSize,
                        pReq->pPageData))
                    {
                        ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                    }
                    else
                    {
                        ioStat.ffxStat = FFXSTAT_SUCCESS;
                        ioStat.ulCount = pReq->ulCount;
                    }

                    break;
                }

                default:
                {
                    FFXPRINTF(1, ("NORFIM: IORequest() Bad Write Generic SubFunction %x\n", pReq->ioSubFunc));
                    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                    DclError();
                    break;
                }
            }

            break;
        }

        case FXIOFUNC_FIM_READ_CONTROLDATA:
        {
            FFXIOR_FIM_READ_CONTROLDATA *pReq = (FFXIOR_FIM_READ_CONTROLDATA*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulLength);
            DclAssert(pReq->pData);

            ioStat.ulFlags = IOFLAGS_BYTE;

            if(pFim->ulPrimaryCmdSet == CMDSET_INTEL_EXTENDED_M18)
                fSuccess = ReadControl(pFim, pReq->ulOffset, pReq->ulLength, pReq->pData);
            else
                fSuccess = Read(pFim, pReq->ulOffset, pReq->ulLength, pReq->pData);

            if(fSuccess)
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                ioStat.ulCount = pReq->ulLength;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            }

            break;
        }

        case FXIOFUNC_FIM_WRITE_CONTROLDATA:
        {
            FFXIOR_FIM_WRITE_CONTROLDATA *pReq = (FFXIOR_FIM_WRITE_CONTROLDATA*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulLength);
            DclAssert(pReq->pData);

            ioStat.ulFlags = IOFLAGS_BYTE;

            if(pFim->ulPrimaryCmdSet == CMDSET_INTEL_EXTENDED_M18)
                fSuccess = WriteControl(pFim, pReq->ulOffset, pReq->ulLength, pReq->pData);
            else
                fSuccess = Write(pFim, pReq->ulOffset, pReq->ulLength, pReq->pData);

            if(fSuccess)
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                ioStat.ulCount = pReq->ulLength;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            }

            break;
        }

        default:
        {
            FFXPRINTF(1, ("NORFIM: IORequest() Bad Function %x\n", pIOR->ioFunc));
            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            DclError();
            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEUNDENT),
        "NORFIM: IORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: Read()

    Performs transfers data from the RAM into the client
    buffer.  This is performed by a simple call to memmove.

    Parameters:
        pFim     - The FIM instance handle
        ulStart  - Starting offset in bytes to begin the access
        ulLength - Number of bytes to transfer
        pBuffer  - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
#define NORFIM_READ_LOOP    (0)
#define NORFIM_READ_DUFFS   (1)
#define NORFIM_READ_MEMCPY  (2)
#define NORFIM_READ_MODE    NORFIM_READ_LOOP
static D_BOOL Read(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart,
    D_UINT32        ulLength,
    D_BUFFER       *pBuffer)
{
    DclAssert(pFim);
    DclAssert(ulLength);
    DclAssert(DCLISALIGNED(ulStart, sizeof(FLASHDATA)));
    DclAssert(DCLISALIGNED(ulLength, sizeof(FLASHDATA)));
    DclAssert(pBuffer);
    DclAssert(ulStart < pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);
    DclAssert((ulStart + ulLength) <= pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);

    while(ulLength)
    {
        PFLASHDATA  pMedia;
        D_UINT32    ulWindowSize;

        /*  Get the media pointer and the max size we can access with it
        */
        ulWindowSize = FfxFimNorWindowMap(pFim->hDev, ulStart, (volatile void **)&pMedia);
        if(!ulWindowSize)
            break;
        DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED(ulWindowSize, sizeof(FLASHDATA)));

        while(ulWindowSize && ulLength)
        {
            D_UINT32    ulThisLength = DCLMIN(ulLength, ulWindowSize);

            DclAssert(DCLISALIGNED(ulThisLength, sizeof(FLASHDATA)));

            if(DCLISALIGNED((D_UINTPTR)pBuffer, sizeof(FLASHDATA)))
            {
              #if NORFIM_READ_MODE == NORFIM_READ_DUFFS
                /*  Perform the read using Duff's Device.
                */
                FLASHDATA  *pTo = (FLASHDATA *)pBuffer;
                D_INT32    lCount = (D_INT32)(ulThisLength / sizeof(FLASHDATA));

                DclAssert((ulThisLength / sizeof(FLASHDATA)) >= D_INT32_MAX);

                switch(lCount % 8)
                {
                    case 0:     do {    *pTo++ = *pMedia++;
                    case 7:             *pTo++ = *pMedia++;
                    case 6:             *pTo++ = *pMedia++;
                    case 5:             *pTo++ = *pMedia++;
                    case 4:             *pTo++ = *pMedia++;
                    case 3:             *pTo++ = *pMedia++;
                    case 2:             *pTo++ = *pMedia++;
                    case 1:             *pTo++ = *pMedia++;
                                } while ((lCount -= 8) > 0);
                }
                pBuffer += ulThisLength;

              #elif NORFIM_READ_MODE == NORFIM_READ_MEMCPY
                /*  Perform the read using memcpy.
                */
                DclMemCpy(pBuffer, (void *)pMedia, ulThisLength);
                pBuffer += ulThisLength;
                pMedia  += ulThisLength / sizeof(FLASHDATA);

              #elif NORFIM_READ_MODE == NORFIM_READ_LOOP
                /*  Perform the read in units of FLASHDATA.
                */
                FLASHDATA  *pTo = (FLASHDATA *)pBuffer;
                unsigned    i;

                for(i = 0; i < (ulThisLength / sizeof(FLASHDATA)); i++)
                    *pTo++ = *pMedia++;

                pBuffer += ulThisLength;
              #else
                #error "Invalid NORFIM_READ_MODE: " NORFIM_READ_MODE
              #endif
            }
            else
            {
                /*  The buffer is misaligned, use a bounce buffer to tranfer
                    the data from the flash.
                */
                FLASHDATA   From;
                unsigned    i;

                for(i = 0; i < ulThisLength / sizeof(FLASHDATA); i++)
                {
                    From = *pMedia++;

                    DclMemCpy(pBuffer, &From, sizeof(FLASHDATA));
                    pBuffer += sizeof(FLASHDATA);
                }
            }

            ulLength     -= ulThisLength;
            ulWindowSize -= ulThisLength;
            ulStart      += ulThisLength;
        }    }

    return (ulLength == 0);
}


/*-------------------------------------------------------------------
    Public: Write()

    Parameters:
        pFim     - The FIM instance handle
        ulStart  - Starting offset in bytes to begin the access
        ulLength - Number of bytes to transfer
        pBuffer  - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
static D_BOOL Write(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart,
    D_UINT32        ulLength,
    const D_BUFFER *pBuffer)
{
    PFLASHDATA      pMedia = NULL;
    D_BOOL          fSuccess = TRUE;

    DclAssert(pFim);
    DclAssert(ulLength);
    DclAssert(pBuffer);
    DclAssert(ulStart < pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);
    DclAssert((ulStart + ulLength) <= pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);
    DclAssert(DCLISALIGNED(ulStart, sizeof(FLASHDATA)));
    DclAssert(DCLISALIGNED(ulLength, sizeof(FLASHDATA)));

    /* Start with flash in a known state
    */
    ResetFlash(pFim, ulStart);

    while(ulLength)
    {
        D_UINT32    ulWindowSize;

        /*  Get the window size
        */
        ulWindowSize = FfxFimNorWindowMap(pFim->hDev, ulStart, (volatile void **)&pMedia);
        DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED(ulWindowSize, sizeof(FLASHDATA)));
        if(!ulWindowSize)
        {
            fSuccess = FALSE;
            break;
        }

        while(ulWindowSize && ulLength)
        {
            D_UINT32    ulThisLength = DCLMIN(ulLength, ulWindowSize);

            DclAssert(DCLISALIGNED(ulThisLength, sizeof(FLASHDATA)));
            if(pFim->ulBufferWriteSize > 0)
              fSuccess = WriteBufferedBytes(pFim, ulStart, pBuffer, ulThisLength);
            else
               fSuccess = WriteBytes(pFim, ulStart, pBuffer, ulThisLength);

            if(!fSuccess)
            {
                DclError();
                break;
            }

            /*  If we have written everything, get outta here.
            */
            ulLength -= ulThisLength;
            if(ulLength == 0)
                break;

            /*  Go to the next offset
            */
            ulWindowSize    -= ulThisLength;
            ulStart         += ulThisLength;
            pBuffer         += ulThisLength;

        }

        if(!fSuccess)
            break;
    }

    /* Leave flash in a known state
    */
    ResetFlash(pFim, ulStart - ulLength);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: EraseStart()

    Attempts to initiate an erase operation.  If it is started
    successfully, the only FIM functions that can then be called
    are EraseSuspend() and ErasePoll().  The operation must
    subsequently be monitored by calls to ErasePoll().

    If it is not started successfully, those functions may not
    be called.  The flash is restored to a readable state if
    possible, but this cannot always be guaranteed.

    Parameters:
        pFim    - The FIM instance handle
        ulStart - Starting offset in bytes to begin the erase.  This
                  must be on a physical erase zone boundary.

    Return Value:
        TRUE  - If the erase was started successfully
        FALSE - If the erase failed
-------------------------------------------------------------------*/
static D_BOOL EraseStart(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart)
{
    D_UINT32        ulBaseLatch;
    PFLASHDATA      pMedia;
    PFLASHDATA      pLatch1;    /* media pointer at latch offset 1 */
    PFLASHDATA      pLatch2;    /* media pointer at latch offset 2 */

    DclAssert(pFim);
    DclAssert(ulStart < pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);
    DclAssert(!pFim->fEraseInProgress);
    DclAssert(!pFim->fEraseSuspended);

    /* AMD-specific erase commands
    */
    if (pFim->fAmdFlash)
    {
        /*  ulBaseLatch is used to send commands and read status from
            the applicable erase zone.  This ensures that we are writing
            to the correct chip (if the array has multiple linear
            chips) or hardware partition (for RWW devices).
        */
        if(pFim->fInBootBlocks)
            ulBaseLatch = ulStart & (~(pFim->FimInfo.ulBootBlockSize - 1));
        else
            ulBaseLatch = ulStart & (~(pFim->FimInfo.ulBlockSize - 1));

        /*  To guarantee that we send all commands to the same erase zone,
            the AMD_LATCH_OFFSET values must be less than the erase zone size.
        */
        DclAssert(AMD_LATCH_OFFSET1 < pFim->FimInfo.ulBlockSize);
        DclAssert(AMD_LATCH_OFFSET2 < pFim->FimInfo.ulBlockSize);

        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch, (volatile void **)&pMedia))
            return FALSE;
        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET1, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET2, (volatile void **)&pLatch2))
            return FALSE;
        DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch1, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch2, sizeof(FLASHDATA)));

        /*  Send erase commands
        */
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pLatch1 = AMDCMD_ERASE1;
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pMedia  = AMDCMD_ERASE2;
    }


    /* Intel-specific erase commands
    */
    else if(pFim->fIntelFlash)
    {
        if(!FfxFimNorWindowMap(pFim->hDev, ulStart, (volatile void **)&pMedia))
            return FALSE;
        DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));

        /*  Clear any status from previous operations, then check the status.
        */
        *pMedia = INTLCMD_CLEAR_STATUS;
        *pMedia = INTLCMD_READ_STATUS;

        if((*pMedia & pFim->ChipCommand.statusMask) == INTLSTAT_DONE)
        {
            /*  The flash appears to be ready, so start the erase.
            */
            *pMedia = INTLCMD_ERASE_START;
            *pMedia = INTLCMD_ERASE_RESUME;
        }
        else
        {
            /*  There's probably something horrid going on like an erase that
                timed out and got suspended.  Try to return the flash to Read
                Array mode and return a failure indication.
            */
            *pMedia = INTLCMD_READ_MODE;
            pFim->ulEraseResult = ERASE_FAILED;
            return FALSE;
        }

    }

    /*  Set the timeout for the operation now that it's started.
    */
    DclTimerSet(&pFim->tErase, pFim->ulTimeoutEraseBlockMillisec);

    /*  Record the erase address for ErasePoll(), EraseSuspend, and
        EraseResume() to use.
    */
    pFim->ulEraseStart = ulStart;

    /*  Clear the erase result to signify erase in progress.
    */
    pFim->ulEraseResult = ERASE_IN_PROGRESS;

    /*  Remember the media address, save repeated calls to
        FfxFimNorWindowMap() from ErasePoll().
    */
    pFim->pMedia = pMedia;

    pFim->fEraseInProgress = TRUE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: ErasePoll()

    Monitor the status of an erase begun with EraseStart().

    If the erase fails, attempts to return the flash to its
    normal read mode.  Depending on the type of flash, this may
    or may not be possible.  If it is possible, it may be
    achieved by suspending the erase operation rather than by
    terminating it.  In this case, it may be possible to read
    the flash, but not to erase it further.

    This function may be called with the flash either in read
    array mode or in read status mode.

    Parameters:
        pFim - The FIM instance handle

    Return Value:
        If the erase is still in progress, returns 0.  The only FIM
        functions that can then be called are EraseSuspend() and
        ErasePoll().

        If the erase completed successfully, returns 1, as we will
        always erase only one block.  The flash is in normal read mode.

        If the erase failed, returns ERASE_FAILED, which is a
        value that could never be a valid erase length.  The flash
        is returned to normal read mode if possible, but this may not
        be possible in all cases (for example, if the flash does not
        support suspending an erase, and the operation times out).
-------------------------------------------------------------------*/
static D_UINT32 ErasePoll(
    NORFIMDATA     *pFim)
{
    FLASHDATA       ulFlashStatus;
    FLASHDATA       ulToggleCheck;

    DclAssert(pFim);
    DclAssert(pFim->pMedia);
    DclAssert(!pFim->fEraseSuspended);

    if(pFim->fEraseInProgress == TRUE)
    {
        /* AMD-specific
        */
        if (pFim->fAmdFlash)
        {
            ulFlashStatus = *pFim->pMedia;
            ulToggleCheck = *pFim->pMedia;
            if((ulFlashStatus == (FLASHDATA)0xFFFFFFFF) && (ulToggleCheck == (FLASHDATA)0xFFFFFFFF))
            {
                /*  The erase has completed successfully.
                */
                pFim->ulEraseResult = 1;
                pFim->fEraseInProgress = FALSE;
            }
            else if(DclTimerExpired(&pFim->tErase))
            {
                /*  Operation might have just finished
                */
                ulFlashStatus = *pFim->pMedia;
                ulToggleCheck = *pFim->pMedia;
                if((ulFlashStatus != (FLASHDATA)0xFFFFFFFF) || (ulToggleCheck != (FLASHDATA)0xFFFFFFFF))
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
                  #if FFXCONF_ERASESUSPENDSUPPORT
                    if(pFim->fEraseSuspendSupported)
                        EraseSuspend(pFim);
                  #endif
                    pFim->ulEraseResult = ERASE_FAILED;
                    pFim->fEraseInProgress = FALSE;
                }
            }
            else
            {
#if (FLASH_INTERLEAVE == 4)
                if(((ulFlashStatus & AMD_DONE_CHIP3) != AMD_DONE_CHIP3) &&
                    (ulFlashStatus & AMD_FAIL_CHIP3))
                {
                    ulFlashStatus = *pFim->pMedia;
                    if((ulFlashStatus & AMD_DONE_CHIP3) != AMD_DONE_CHIP3)
                    {
                        DclError();
                        pFim->ulEraseResult = ERASE_FAILED;
                        pFim->fEraseInProgress = FALSE;
                    }
                }
                if(((ulFlashStatus & AMD_DONE_CHIP2) != AMD_DONE_CHIP2) &&
                    (ulFlashStatus & AMD_FAIL_CHIP2))
                {
                    ulFlashStatus = *pFim->pMedia;
                    if((ulFlashStatus & AMD_DONE_CHIP2) != AMD_DONE_CHIP2)
                    {
                        DclError();
                        pFim->ulEraseResult = ERASE_FAILED;
                        pFim->fEraseInProgress = FALSE;
                    }
                }
#endif
#if (FLASH_INTERLEAVE >= 2)
                if(((ulFlashStatus & AMD_DONE_CHIP1) != AMD_DONE_CHIP1) &&
                    (ulFlashStatus & AMD_FAIL_CHIP1))
                {
                    ulFlashStatus = *pFim->pMedia;
                    if((ulFlashStatus & AMD_DONE_CHIP1) != AMD_DONE_CHIP1)
                    {
                        DclError();
                        pFim->ulEraseResult = ERASE_FAILED;
                        pFim->fEraseInProgress = FALSE;
                    }
                }
#endif
                if(((ulFlashStatus & AMD_DONE_CHIP0) != AMD_DONE_CHIP0) &&
                    (ulFlashStatus & AMD_FAIL_CHIP0))
                {
                    ulFlashStatus = *pFim->pMedia;
                    if((ulFlashStatus & AMD_DONE_CHIP0) != AMD_DONE_CHIP0)
                    {
                        DclError();
                        pFim->ulEraseResult = ERASE_FAILED;
                        pFim->fEraseInProgress = FALSE;
                    }
                }
            }
        }

        /* Intel-specific
        */
        else if(pFim->fIntelFlash)
        {
            if((*pFim->pMedia & INTLSTAT_DONE) == INTLSTAT_DONE)
            {
                /*  On normal completion, the status register has the DONE
                    bit set, and all other meaningful bits clear.
                */
                if((*pFim->pMedia & pFim->ChipCommand.statusMask) == INTLSTAT_DONE)
                {
                    /*  The erase has completed successfully.
                    */
                    pFim->ulEraseResult = 1;
                    pFim->fEraseInProgress = FALSE;
                }
                else
                {
                    pFim->ulEraseResult = ERASE_FAILED;
                }

                /*  Whether or not error, reset the flash.
                */
                ResetFlash(pFim, pFim->ulEraseStart);
            }
            else if(DclTimerExpired(&pFim->tErase) && (*pFim->pMedia & INTLSTAT_DONE) != INTLSTAT_DONE)
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
              #if FFXCONF_ERASESUSPENDSUPPORT
                if(pFim->fEraseSuspendSupported)
                    EraseSuspend(pFim);
              #endif
                pFim->ulEraseResult = ERASE_FAILED;
                pFim->fEraseInProgress = FALSE;
            }
        }

    }

    return pFim->ulEraseResult;
}


#if FFXCONF_ERASESUSPENDSUPPORT
/*-------------------------------------------------------------------
    Public: EraseSuspend()

    Suspend an erase operation currently in progress, and return
    the flash to normal read mode.  When this function returns,
    the flash may be read.

    Parameters:
        pFim - The FIM instance handle

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseSuspend(
    NORFIMDATA     *pFim)
{
    D_BOOL          fResult = TRUE;

    DclAssert(pFim);
    DclAssert(pFim->pMedia);

    /*  Save the remaining timeout period.
    */
    pFim->ulTimeoutRemaining = DclTimerRemaining(&pFim->tErase);

    /*  put flash in erase suspend mode
    */
    *pFim->pMedia = pFim->ChipCommand.eraseSuspend;

    /*  Wait a while for the flash to go into erase suspend.
    */
    DclTimerSet(&pFim->tErase, ERASE_SUSPEND_TIMEOUT);
    while(!DclTimerExpired(&pFim->tErase))
    {
        if (pFim->fAmdFlash)
        {
            if((*pFim->pMedia & AMDSTAT_DONE) == AMDSTAT_DONE)
                break;
        }
        else if(pFim->fIntelFlash)
        {
            if((*pFim->pMedia & INTLSTAT_DONE) == INTLSTAT_DONE)
                break;
        }
    }

    /*  Check the status after a possible timeout.  A higher priority
        thread could have preempted between setting the timer or
        checking the status in the loop and checking for expiration.
    */
    if (pFim->fAmdFlash)
    {
        if((*pFim->pMedia & AMDSTAT_DONE) != AMDSTAT_DONE)
        {
            DclError();
            pFim->ulEraseResult = ERASE_FAILED;
            fResult = FALSE;
        }
    }
    else if(pFim->fIntelFlash)
    {
        if((*pFim->pMedia & INTLSTAT_DONE) != INTLSTAT_DONE)
        {
            DclError();
            pFim->ulEraseResult = ERASE_FAILED;
            fResult = FALSE;
        }
    }

    /*  Return the flash to Read Array mode whether or not the suspend
        command appeared to have worked (it can't hurt).
    */
    ResetFlash(pFim, pFim->ulEraseStart);

    return fResult;
}


/*-------------------------------------------------------------------
    Public: EraseResume()

    Resumes an erase that was successfully suspended by
    EraseSuspend().  Once it is resumed, the only FIM functions
    that can be called are EraseSuspend() and ErasePoll().

    Parameters:
        pFim - The FIM instance handle

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseResume(
    NORFIMDATA     *pFim)
{
    PFLASHDATA      pMedia;
    D_UINT32        stat;

    DclAssert(pFim);
    DclAssert(pFim->pMedia);

    /*  If an error occurred in EraseSuspend() the final result of the
        erase was already recorded.  Only operate on the flash if this
        hasn't happened yet.
    */
    if(pFim->ulEraseResult == ERASE_IN_PROGRESS)
    {
        /*  Erases are suspended to perform other operations, so it's
            necessary to remap the window now.
        */
        if(!FfxFimNorWindowMap(pFim->hDev, pFim->ulEraseStart, (volatile void **)&pMedia))
            return FALSE;
        DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));

        pFim->pMedia = pMedia;

        /* Intel-specific parts, clear status from other operation
        */
        if(pFim->fIntelFlash)
        {
            *pMedia = INTLCMD_CLEAR_STATUS;
            *pMedia = INTLCMD_READ_STATUS;
        }

        /*  It's possible that the chip finished its erase before
            EraseSuspend was called.  If the chips is showing error
            status, there's no point in trying to resume.
        */
        if (pFim->fAmdFlash)
        {
            /*  It's possible that one or more of the chips finished its
                erase before EraseSuspend was called, however with AMD flash
                sending the erase resume command to a chip that has already
                finished has no effect.  Send Erase Resume command in all cases
                Erase Poll will report any error situations.
            */
            *pMedia = AMDCMD_ERASE2;
        }
        else if(pFim->fIntelFlash)
        {
            stat = *pMedia & pFim->ChipCommand.statusMask;

            if(stat != INTLSTAT_DONE)
            {
                /*  Change any OK status codes to READSTATUS commands, and
                    any ERASESUSPENDED status codes to ERASERESUME commands.
                */
                TRANSFORMSTATUS(&stat, ISTAT_OK, ICMD_READSTAT);
                TRANSFORMSTATUS(&stat, ISTAT_ERASESUSPENDED, ICMD_ERASERESUME);

                if(ISCOMPLETETRANSFORMATION(stat, ICMD_READSTAT, ICMD_ERASERESUME))
                {
                    /*  If everything was changed to one code or the other,
                        we're good, so issue the command.
                    */
                    *pMedia = (FLASHDATA)stat;
                }
                else
                {
                    /*  Presumably errored out in some fashion.  ErasePoll() will
                        report that the erase failed.
                    */
                    DclError();
                    return FALSE;
                }
            }
        }

        /*  Guarantee a minimum erase increment.
        */
        _sysdelay(MINIMUM_ERASE_INCREMENT);

        /*  Restart the timer.  Note that this will be done in the (unlikely)
            case that both chips had already finished when they were suspended.
            This is harmless, as ErasePoll() checks their status before checking
            for timeout.
        */
        DclTimerSet(&pFim->tErase, pFim->ulTimeoutRemaining);
    }

    return TRUE;
}
#endif /* FFXCONF_ERASESUSPENDSUPPORT */


/*-------------------------------------------------------------------
    Public: IdChip()

    Returns chip to read mode.

    Parameters:
        pFim       - Pointer to FIM data
        pMedia     - Pointer into flash
        ulUseCFI   - Attempt ID with CFI
        ulUseJedec - Attempt ID with Jedec

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL IdChip(
    NORFIMDATA     *pFim,
    PFLASHDATA      pMedia,
    D_UINT32        ulQueryAndGetData,
    D_UINT32        ulUseCFI,
    D_UINT32        ulUseJedec)
{

    PFLASHDATA      pLatch1;
    PFLASHDATA      pLatch2;
    PFLASHDATA      pCfiCmdLatch;
    D_UINT32        i = 0;

    if(ulUseJedec)
    {
        /* Attempt to ID first with JEDEC so that the table of parts can override
            the CFI support.  This is so known parts that claim to be CFI compliant,
            but in reality are not, can be instead handled by a hard-coded table.
        */
        if(ulQueryAndGetData)
        {
            /* Intel commands
            */
            *pMedia = INTLCMD_READ_MODE;
            *pMedia = INTLCMD_CLEAR_STATUS;
            *pMedia = INTLCMD_READ_MODE;
            *pMedia = INTLCMD_IDENTIFY;
            if(ReadDeviceCodes(pMedia, &pFim->ID))
            {
                for(i = 0; NorfimChipTable[i].ulIDFlags != D_UINT32_MAX; i++)
                {
                   if(DoIDsMatch(pFim->ID, NorfimChipTable[i].ID))
                   {
                        FFXPRINTF(1, ("NORFIM: Mfg/Dev1/Dev2/Dev3=%04x/%04x/%04x/%04x supported\n",
                            pFim->ID.manufacture, pFim->ID.device1, pFim->ID.device2, pFim->ID.device3));
                        pFim->fJedecID = TRUE;
                        return GetChipData(pFim, pMedia, TRUE);
                    }
                }
            }

            /* AMD commands
            */
            pLatch1 = DclPtrAddByte(pMedia, AMD_LATCH_OFFSET1);
            pLatch2 = DclPtrAddByte(pMedia, AMD_LATCH_OFFSET2);

            *pMedia    = AMDCMD_READ_MODE;
            *pLatch1   = AMDCMD_UNLOCK1;
            *pLatch2   = AMDCMD_UNLOCK2;
            *pLatch1   = AMDCMD_IDENTIFY;
            if(ReadDeviceCodes(pMedia, &pFim->ID))
            {
                for(i = 0; NorfimChipTable[i].ulIDFlags != D_UINT32_MAX; i++)
                {
                    if(DoIDsMatch(pFim->ID, NorfimChipTable[i].ID))
                    {
                        FFXPRINTF(1, ("NORFIM: Mfg/Dev1/Dev2/Dev3=%04x/%04x/%04x/%04x supported\n",
                            pFim->ID.manufacture, pFim->ID.device1, pFim->ID.device2, pFim->ID.device3));
                        pFim->fJedecID = TRUE;
                        return GetChipData(pFim, pMedia, FALSE);
                    }
                }
            }
        }
        else
        {
            if(ReadDeviceCodes(pMedia, &pFim->ID))
            {
                for(i = 0; NorfimChipTable[i].ulIDFlags != D_UINT32_MAX; i++)
                {
                    if(DoIDsMatch(pFim->ID, NorfimChipTable[i].ID))
                    {
                        FFXPRINTF(1, ("NORFIM: Mfg/Dev1/Dev2/Dev3=%04x/%04x/%04x/%04x supported\n",
                            pFim->ID.manufacture, pFim->ID.device1, pFim->ID.device2, pFim->ID.device3));
                        pFim->fJedecID = TRUE;
                        return TRUE;
                    }
                }
            }
        }
    }

    if(ulUseCFI)
    {
        /*  Calculate pointer to CFI command latch:
        */
        pCfiCmdLatch = DclPtrAddByte(pMedia, CFI_OFFSET_QUERY_ADDRESS);

        /* Check if the part is CFI compliant.  This will only be attempted if the
            part was not found in the NOR part table using Jedec IDs.
        */
        if(ulQueryAndGetData)
        {
            /* Start in read mode
            */
            *pMedia = INTLCMD_READ_MODE;

            /* Send CFI command and check if part is CFI compatible
            */
            *pCfiCmdLatch = CFI_COMMAND_QUERY;
        }

        if (*(pMedia + CFI_OFFSET_QUERY_ASCII_STRING_QRY)      == MAKEFLASHDUPCMD('Q') &&
            *(pMedia + CFI_OFFSET_QUERY_ASCII_STRING_QRY + CFI_FIELD_INTERVAL)  == MAKEFLASHDUPCMD('R') &&
            *(pMedia + CFI_OFFSET_QUERY_ASCII_STRING_QRY + 2*CFI_FIELD_INTERVAL)  == MAKEFLASHDUPCMD('Y'))
        {
            FFXPRINTF(1, ("NORFIM: CFI query string found\n"));
            pFim->fCfiCompliant = TRUE;
            if(ulQueryAndGetData)
                return GetChipData(pFim, pMedia, TRUE);
            else
                return TRUE;
        }

        /*  Didn't find query string with Intel-style reset command. Try again
            with AMD-style reset command, just in case that is the problem.
        */
        else
        {
            if(ulQueryAndGetData)
            {
                /* Start in read mode
                */
                *pMedia = AMDCMD_READ_MODE;

                /* Send CFI command and check if part is CFI compatible
                */
                *pCfiCmdLatch = CFI_COMMAND_QUERY;
                FFXPRINTF(1,("NORFIM: CFI offset query address: %x\n", CFI_OFFSET_QUERY_ADDRESS));
            }
            if (*(pMedia + CFI_OFFSET_QUERY_ASCII_STRING_QRY)      == MAKEFLASHDUPCMD('Q') &&
                *(pMedia + CFI_OFFSET_QUERY_ASCII_STRING_QRY + CFI_FIELD_INTERVAL)  == MAKEFLASHDUPCMD('R') &&
                *(pMedia + CFI_OFFSET_QUERY_ASCII_STRING_QRY + 2*CFI_FIELD_INTERVAL)  == MAKEFLASHDUPCMD('Y'))
            {
                FFXPRINTF(1, ("NORFIM: CFI query string found\n"));
                pFim->fCfiCompliant = TRUE;
                if(ulQueryAndGetData)
                    return GetChipData(pFim, pMedia, FALSE);
                else
                    return TRUE;
            }
        }

        FFXPRINTF(1, ("NORFIM: CFI query string not found\n"));
        return FALSE;
    }
    return FALSE;
}


/*-------------------------------------------------------------------
    Public: ReadDeviceCodes()

    Reads the manufacturer and device codes from the chip(s)

    Parameters:
        pMedia   - Pointer to flash
        lpChipID - Location to save the manufacturer and device
                   ID codes.

    Return Value:
        TRUE  -
        FALSE -
-------------------------------------------------------------------*/
static D_BOOL ReadDeviceCodes(
    PFLASHDATA      pMedia,
    NORFLASHID     *lpChipID)
{

    lpChipID->manufacture   = *pMedia;
    lpChipID->device1       = *(pMedia + NORFIM_DEVICE_CODE1_OFFSET);
    lpChipID->device2       = *(pMedia + NORFIM_DEVICE_CODE2_OFFSET);
    lpChipID->device3       = *(pMedia + NORFIM_DEVICE_CODE3_OFFSET);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DoIDsMatch()

    Compares two NORFLASHIDs

    Parameters:

    Return Value:
       TRUE  - IDs match
       FALSE - IDs don't match
-------------------------------------------------------------------*/
static D_BOOL   DoIDsMatch(
    NORFLASHID id,
    NORFLASHID idTable)
{
    if(id.manufacture != MAKEFLASHDUPCMD(idTable.manufacture))
        return FALSE;
    if((idTable.device1 != ID_NA) && (id.device1 != MAKEFLASHDUPCMD(idTable.device1)))
        return FALSE;
    if((idTable.device2 != ID_NA) && (id.device2 != MAKEFLASHDUPCMD(idTable.device2)))
        return FALSE;
    if((idTable.device3 != ID_NA) && (id.device3 != MAKEFLASHDUPCMD(idTable.device3)))
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: GetChipData()

    Parameters:
        pFim - Pointer to FIM data

    Return Value:
        Returns FALSE if CFI info is not supported or the info
        appear bogus
-------------------------------------------------------------------*/
static D_BOOL   GetChipData(
    NORFIMDATA     *pFim,
    PFLASHDATA      pMedia,
    D_BOOL          fUseIntelReadCmd)
{
    D_UINT32        i               = 0;
    D_UINT32        ulTableIndex    = 0;
    D_UINT32        ulBootBlockFlag = 0;

    if(!(pFim->fJedecID || pFim->fCfiCompliant))
    {
        DclError();
        return FALSE;
    }

    /* *************************************************************************
        Retrieve info from either the JEDEC table or CFI table
       *************************************************************************
    */
    if(pFim->fJedecID)
    {
        for(ulTableIndex = 0; NorfimChipTable[ulTableIndex].ulIDFlags != D_UINT32_MAX; ulTableIndex++)
        {
            if(DoIDsMatch(pFim->ID, NorfimChipTable[ulTableIndex].ID))
            {
                pFim->ulPrimaryCmdSet               = NorfimChipTable[ulTableIndex].ulPrimaryCmdSet;
                pFim->ulChipSize                    = NorfimChipTable[ulTableIndex].ulChipSize * FLASH_INTERLEAVE;
                pFim->FimInfo.ulBlockSize           = NorfimChipTable[ulTableIndex].ulBlockSize * FLASH_INTERLEAVE;
                pFim->FimInfo.ulBootBlockSize       = NorfimChipTable[ulTableIndex].ulBootBlockSize * FLASH_INTERLEAVE;
                pFim->FimInfo.uBootBlockCountLow    = NorfimChipTable[ulTableIndex].uLowBootBlocks;
                pFim->FimInfo.uBootBlockCountHigh   = NorfimChipTable[ulTableIndex].uHighBootBlocks;
                pFim->ulBufferWriteSize             = NorfimChipTable[ulTableIndex].ulBufferWriteSize * FLASH_INTERLEAVE;
                if ((D_BOOL)(NorfimChipTable[ulTableIndex].ulIDFlags & NORFIM_IDFLAGS_INTELBLOCKLOCK))
                    pFim->eFimBlockLockSupport = IntelInstantBlockLock;
                pFim->fEraseSuspendSupported        = (D_BOOL)!(NorfimChipTable[ulTableIndex].ulIDFlags & NORFIM_IDFLAGS_NOERASESUSPEND);
                pFim->fAmdUnlockBypass              = (D_BOOL)!(NorfimChipTable[ulTableIndex].ulIDFlags & NORFIM_IDFLAGS_NOUNLOCKPYPASS);
                pFim->ulTimeoutWriteWordMillisec    = NORFIM_TIMEOUT_WRITE;
                pFim->ulTimeoutWriteBufferMillisec  = NORFIM_TIMEOUT_WRITEBUFFFER;
                pFim->ulTimeoutEraseBlockMillisec   = NORFIM_TIMEOUT_ERASE;
                break;
            }
        }
    }
    else if(pFim->fCfiCompliant)
    {
        PFLASHDATA pCfiCmdLatch = DclPtrAddByte(pMedia, CFI_OFFSET_QUERY_ADDRESS);
        /* Start in read mode
        */
        if (fUseIntelReadCmd)
            *pMedia = INTLCMD_READ_MODE;
        else
            *pMedia = AMDCMD_READ_MODE;

        /* Send CFI command and check if part is CFI compatible
        */
        *pCfiCmdLatch = CFI_COMMAND_QUERY;

        /* Get Command Sets and Extended Table Addresses
        */
        pFim->ulPrimaryCmdSet            = (CHIP0 & *(pMedia + CFI_OFFSET_PRIMARY_COMMAND_SET));
        pFim->ulPrimaryCmdSet           |= (CHIP0 & *(pMedia + CFI_OFFSET_PRIMARY_COMMAND_SET + CFI_FIELD_INTERVAL)) << 8;
        pFim->ulPrimaryTableAddress      = (CHIP0 & *(pMedia + CFI_OFFSET_PRIMARY_EXTENDED_TABLE_ADDRESS));
        pFim->ulPrimaryTableAddress     |= (CHIP0 & *(pMedia + CFI_OFFSET_PRIMARY_EXTENDED_TABLE_ADDRESS + CFI_FIELD_INTERVAL)) << 8;
        pFim->ulAlternateTableAddress    = (CHIP0 & *(pMedia + CFI_OFFSET_ALTERNATE_EXTENDED_TABLE_ADDRESS));
        pFim->ulAlternateTableAddress   |= (CHIP0 & *(pMedia + CFI_OFFSET_ALTERNATE_EXTENDED_TABLE_ADDRESS + CFI_FIELD_INTERVAL)) << 8;
        pFim->ulAlternateCmdSet          = (CHIP0 & *(pMedia + CFI_OFFSET_ALTERNATE_COMMAND_SET));
        pFim->ulAlternateCmdSet         |= (CHIP0 & *(pMedia + CFI_OFFSET_ALTERNATE_COMMAND_SET + CFI_FIELD_INTERVAL)) << 8;

        pFim->ulPrimaryTableAddress *= CFI_FIELD_INTERVAL;
        pFim->ulAlternateTableAddress *= CFI_FIELD_INTERVAL;
        /* Check that the Primary vendor table really begins at pFim->ulPrimaryTableAddress
        */
        DclAssert(*(pMedia + pFim->ulPrimaryTableAddress)       == MAKEFLASHDUPCMD('P'));
        DclAssert(*(pMedia + pFim->ulPrimaryTableAddress + CFI_FIELD_INTERVAL)   == MAKEFLASHDUPCMD('R'));
        DclAssert(*(pMedia + pFim->ulPrimaryTableAddress + 2*CFI_FIELD_INTERVAL)   == MAKEFLASHDUPCMD('I'));

        /* Get version numbers of the primary and alternate tables
        */
        pFim->ulPrimaryTableMajorVersion = CHIP0 & *(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_MAJOR_VERSION);
        pFim->ulPrimaryTableMinorVersion = CHIP0 & *(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_MINOR_VERSION);

        if(pFim->ulAlternateCmdSet)
        {
            /* Currently Alternate vendor info is unsupported, and it is unknown what is contained
               in a part that has this info, so fail if this exists
            */
            pFim->ulAlternateTableMajorVersion = CHIP0 & *(pMedia + pFim->ulAlternateTableAddress + CFI_OFFSET_MAJOR_VERSION);
            pFim->ulAlternateTableMinorVersion = CHIP0 & *(pMedia + pFim->ulAlternateTableAddress + CFI_OFFSET_MINOR_VERSION);
            FFXPRINTF(1, ("NORFIM: Alternate command set %u currently not supported.\n",pFim->ulAlternateCmdSet));
            DclError();
            return FALSE;
        }
    }


    /* *************************************************************************
        Setup commands, status, and other info for specific parts based on
         command set used
       *************************************************************************
    */
    switch(pFim->ulPrimaryCmdSet)
    {
        case CMDSET_INTEL_STANDARD:
        case CMDSET_INTEL_EXTENDED:
            pFim->ChipCommand.programWord       = INTLCMD_PROGRAM;
            pFim->ChipCommand.bufferWriteStart  = INTLCMD_BUFFERED_WRITE;
            pFim->ChipCommand.bufferWriteCommit = INTLCMD_CONFIRM;
            pFim->ChipCommand.eraseSuspend      = INTLCMD_ERASE_SUSPEND;
            pFim->ChipCommand.statusMask        = INTLSTAT_STATUS_MASK;
            pFim->uPageSize                     = 0;
            pFim->ulIntelControlValidSize       = 0;
            pFim->ulIntelControlInvalidSize     = 0;
            pFim->uDeviceType                   = DEV_NOR & DEVTYPE_MASK;
            pFim->uDeviceFlags                  = NORFIM_MLC_FLASH & DEVFLAGS_MASK;
            pFim->fIntelFlash                   = TRUE;
            break;

        case CMDSET_INTEL_EXTENDED_M18:
            pFim->ChipCommand.programWord       = INTLCMD_PROGRAM_M18;
            pFim->ChipCommand.bufferWriteStart  = INTLCMD_BUFFERED_WRITE_M18;
            pFim->ChipCommand.bufferWriteCommit = INTLCMD_CONFIRM;
            pFim->ChipCommand.eraseSuspend      = INTLCMD_ERASE_SUSPEND;
            pFim->ChipCommand.statusMask        = (FLASHDATA)INTLSTAT_STATUS_MASK_M18; /* Typecast for x8 config, not available for M18 */

            if(pFim->fCfiCompliant)
            {
                pFim->uPageSize = (D_UINT16)(CHIP0 & *(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_M18_INTEL_PRI_PROGRAMMING_REGION_SIZE));
                if(pFim->uPageSize != 0)
                    pFim->uPageSize  = (1 << pFim->uPageSize);
                pFim->uPageSize *= FLASH_INTERLEAVE;
                pFim->ulIntelControlValidSize = FLASH_INTERLEAVE * (CHIP0 & *(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_M18_INTEL_PRI_CONTROL_MODE_VALID_SIZE));
                pFim->ulIntelControlInvalidSize = FLASH_INTERLEAVE * (CHIP0 & *(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_M18_INTEL_PRI_CONTROL_MODE_INVALID_SIZE));
            }
            else
            {
                pFim->uPageSize                     = NorfimChipTable[ulTableIndex].uPageSize * FLASH_INTERLEAVE;
                pFim->ulIntelControlValidSize       = NorfimChipTable[ulTableIndex].ulIntelControlValidSize * FLASH_INTERLEAVE;
                pFim->ulIntelControlInvalidSize     = NorfimChipTable[ulTableIndex].ulIntelControlInvalidSize * FLASH_INTERLEAVE;
            }
            pFim->uDeviceType                   = DEV_ISWF & DEVTYPE_MASK;
            pFim->uDeviceFlags                  = NORFIM_MLC_FLASH & DEVFLAGS_MASK;
            pFim->fIntelFlash                   = TRUE;
            break;

        case CMDSET_AMD_STANDARD:
            pFim->ChipCommand.programWord       = AMDCMD_PROGRAM;
            pFim->ChipCommand.bufferWriteStart  = AMDCMD_WRITETOBUFFER;
            pFim->ChipCommand.bufferWriteCommit = AMDCMD_BUFFERTOFLASH;
            pFim->ChipCommand.eraseSuspend      = AMDCMD_ERASE_SUSPEND;
            pFim->ChipCommand.statusMask        = AMDSTAT_MASK;
            pFim->uPageSize                     = 0;
            pFim->ulIntelControlValidSize       = 0;
            pFim->ulIntelControlInvalidSize     = 0;
            pFim->uDeviceType                   = DEV_NOR & DEVTYPE_MASK;
            pFim->uDeviceFlags                  = (DEV_MERGE_WRITES | NORFIM_MLC_FLASH) & DEVFLAGS_MASK;
            pFim->fAmdFlash                     = TRUE;

            if(pFim->fCfiCompliant)
            {
                /* Check for vaild Primary Table version, due to Samsung parts that appear to
                    not follow AMD/Spansion version numbers.  These parts can be manually
                    entered into norfim.h.
                */
                if((pFim->ulPrimaryTableMajorVersion == '0' && pFim->ulPrimaryTableMinorVersion == '0') ||
                   (pFim->ulPrimaryTableMajorVersion == '1' && pFim->ulPrimaryTableMinorVersion == '0'))
                {
                    FFXPRINTF(1, ("NORFIM: Primary Table version not supported\n"));
                    DclError();
                    return FALSE;
                }
            }
            break;

        default:
            FFXPRINTF(1, ("NORFIM: Primary command set %u currently not supported.\n",pFim->ulPrimaryCmdSet));
            DclError();
            return FALSE;
    }


    /* *************************************************************************
        Get CFI flash geometery and timing info if using CFI
       *************************************************************************
    */
    if(pFim->fCfiCompliant)
    {
        /* Get chip size, stored in CFI table as 2^N
        */
        pFim->ulChipSize = FLASH_INTERLEAVE * (1 << (CHIP0 & *(pMedia + CFI_OFFSET_DEVICE_SIZE)));

        /* Get number of erase block regions, 1 = uniform blocks
        */
        pFim->ulNumEraseBlockRegions = CHIP0 & *(pMedia + CFI_OFFSET_ERASE_BLOCK_REGIONS);
        if(pFim->ulNumEraseBlockRegions == 0 || pFim->ulNumEraseBlockRegions > FIM_MAX_ERASE_BLOCK_REGIONS)
        {
            FFXPRINTF(1, ("NORFIM: Erase block region %u not supported.\n", pFim->ulNumEraseBlockRegions));
            DclError();
            return FALSE;
        }

        /* Get the info for the erase block regions
        */
        for(i = 0; i < pFim->ulNumEraseBlockRegions; i++)
        {
            /* Get block count for this region
            */
            pFim->FimEraseBlks[i].ulNumEraseBlocks =
                CHIP0 & *(pMedia + CFI_OFFSET_ERASE_BLOCK_REGION_INFO_BASE + (CFI_SIZE_ERASE_BLOCK_REGION_IN_BUSWIDTH * i));
            pFim->FimEraseBlks[i].ulNumEraseBlocks |=
                (CHIP0 & *(pMedia + CFI_OFFSET_ERASE_BLOCK_REGION_INFO_BASE + (CFI_SIZE_ERASE_BLOCK_REGION_IN_BUSWIDTH * i) + CFI_FIELD_INTERVAL)) << 8;
            pFim->FimEraseBlks[i].ulNumEraseBlocks += 1;

            /* Get block size for this region, multiply by FLASH_INTERLEAVE to account for both chips
            */
            pFim->FimEraseBlks[i].ulBlockSizeKB =
                CHIP0 & *(pMedia + CFI_OFFSET_ERASE_BLOCK_REGION_INFO_BASE + (CFI_SIZE_ERASE_BLOCK_REGION_IN_BUSWIDTH * i) + 2*CFI_FIELD_INTERVAL);
            pFim->FimEraseBlks[i].ulBlockSizeKB |=
                (CHIP0 & *(pMedia + CFI_OFFSET_ERASE_BLOCK_REGION_INFO_BASE + (CFI_SIZE_ERASE_BLOCK_REGION_IN_BUSWIDTH * i) + 3*CFI_FIELD_INTERVAL)) << 8;
            pFim->FimEraseBlks[i].ulBlockSizeKB *= 256;
            pFim->FimEraseBlks[i].ulBlockSizeKB *= FLASH_INTERLEAVE;

            /* Store the largest erase zone size, as this is not a boot block
            */
            if(pFim->FimInfo.ulBlockSize < pFim->FimEraseBlks[i].ulBlockSizeKB)
                pFim->FimInfo.ulBlockSize  = pFim->FimEraseBlks[i].ulBlockSizeKB;
        }


        /* If more than 1 erase block region, assume the smallest is the boot block size
        */
        if(pFim->ulNumEraseBlockRegions > 1)
        {
            pFim->FimInfo.ulBootBlockSize = pFim->FimEraseBlks[0].ulBlockSizeKB;
            for(i = 1; i < pFim->ulNumEraseBlockRegions; i++)
            {
                if(pFim->FimInfo.ulBootBlockSize > pFim->FimEraseBlks[i].ulBlockSizeKB)
                    pFim->FimInfo.ulBootBlockSize = pFim->FimEraseBlks[i].ulBlockSizeKB;
            }
        }

        /* Determine if part has high/low/both boot blocks.  Assume that if at this point ulBootBlockSize > 0,
            the part has some sort of boot blocks somewhere on the flash.
        */
        if(pFim->FimInfo.ulBootBlockSize > 0)
        {
            if(pFim->fIntelFlash)
            {
                /* Intel stores erase zones in order, so a simple comparison of the first and last erase zone
                    will determine the boot block locations
                */
                if(pFim->FimEraseBlks[0].ulBlockSizeKB == pFim->FimInfo.ulBootBlockSize)
                {
                    pFim->FimInfo.uBootBlockCountLow = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                }
                if(pFim->FimEraseBlks[(pFim->ulNumEraseBlockRegions - 1)].ulBlockSizeKB == pFim->FimInfo.ulBootBlockSize)
                {
                    pFim->FimInfo.uBootBlockCountHigh = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                }
            }
            else if(pFim->fAmdFlash)
            {
                ulBootBlockFlag = CHIP0 & *(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_AMD_PRI_BOOT_BLOCK_FLAG);

                switch(ulBootBlockFlag)
                {
                    case CFI_BOOT_BLOCK_BOTTOM:
                        pFim->FimInfo.uBootBlockCountLow    = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                        pFim->FimInfo.uBootBlockCountHigh   = 0;
                        break;
                    case CFI_BOOT_BLOCK_TOP:
                        pFim->FimInfo.uBootBlockCountLow    = 0;
                        pFim->FimInfo.uBootBlockCountHigh   = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                        break;
                    case CFI_BOOT_BLOCK_TOP_AND_BOTTOM:
                        pFim->FimInfo.uBootBlockCountLow    = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                        pFim->FimInfo.uBootBlockCountHigh   = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                        break;
                    case CFI_BOOT_BLOCK_TOP_AND_BOTTOM_WP:
                        pFim->FimInfo.uBootBlockCountLow    = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                        pFim->FimInfo.uBootBlockCountHigh   = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.ulBootBlockSize);
                        break;
                    case CFI_BOOT_BLOCK_UNIFORM:
                        /* Should never see this inside this block of code because to get here
                            there has to be more than one erase zone region
                        */
                        DclError();
                        pFim->FimInfo.uBootBlockCountLow    = 0;
                        pFim->FimInfo.uBootBlockCountHigh   = 0;
                        break;
                    default:
                        FFXPRINTF(1, ("NORFIM: ulBootBlockFlag %u not supported.\n", ulBootBlockFlag));
                        DclError();
                        return FALSE;
                }
           }
        }


        /* Determine if erase suspend is supported by flash part
        */
        if(pFim->fIntelFlash)
            if(*(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_INTEL_PRI_ERASE_SUSPEND) & CFI_INTEL_PRI_ERASE_SUSPEND_MASK)
                pFim->fEraseSuspendSupported = TRUE;
        if(pFim->fAmdFlash)
            if(*(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_AMD_PRI_ERASE_SUSPEND))
                pFim->fEraseSuspendSupported = TRUE;


        /* Determine if part supports some form of block or sector protection
        */
        if(pFim->fIntelFlash)
        {
            if(*(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_INTEL_PRI_INSTANT_BLOCK_LOCKING) & CFI_INTEL_PRI_INSTANT_BLOCK_LOCKING_MASK)
                pFim->eFimBlockLockSupport = IntelInstantBlockLock;
        }
        else if (pFim->fAmdFlash)
        {
            if ((*(pMedia + pFim->ulPrimaryTableAddress + CFI_OFFSET_AMD_SECTOR_PROTECT_SCHEME))
                 == CFI_AMD_ADVANCED_SECTOR_PROTECTION_VAL)
                pFim->eFimBlockLockSupport = AmdAdvancedSectorProtection;
        }
        else
        {
            pFim->eFimBlockLockSupport = NoProtection;
        }

        /* Use unlock bypass for all CFI compliant parts
        */
        if(pFim->fAmdFlash)
            pFim->fAmdUnlockBypass = TRUE;


        /* Get maximum buffer write, store in table as 2^n
        */
        pFim->ulBufferWriteSize  = (CHIP0 & *(pMedia + CFI_OFFSET_BUFFER_WRITE_SIZE));
        pFim->ulBufferWriteSize |= (CHIP0 & *(pMedia + CFI_OFFSET_BUFFER_WRITE_SIZE + 1)) << 8;
        if(pFim->ulBufferWriteSize != 0)
            pFim->ulBufferWriteSize  = (1 << pFim->ulBufferWriteSize) * FLASH_INTERLEAVE;


        /* Get timeout info.  Maximum timeout values are calculated as a multiple of
            the typical timeout values, with values stored as 2^n

           Write timeout values are stored in microseconds, but our timer is millisecond resolution,
            so convert to milliseconds and add one for any decimal truncation.  Timeouts will be a
            fraction of a millisecond higher than actual values, but will not effect performance as
            long as these values are only used for timeouts
        */
        pFim->ulTimeoutWriteWordMillisec  = (1 << (CHIP0 & *(pMedia + CFI_OFFSET_TYPICAL_TIMEOUT_WRITE_WORD)));
        pFim->ulTimeoutWriteWordMillisec *= (1 << (CHIP0 & *(pMedia + CFI_OFFSET_MAX_TIMEOUT_WRITE_WORD)));
        pFim->ulTimeoutWriteWordMillisec  = (pFim->ulTimeoutWriteWordMillisec / 1000) + 1;

        if(pFim->ulBufferWriteSize)
        {
            pFim->ulTimeoutWriteBufferMillisec  = (1 << (CHIP0 & *(pMedia + CFI_OFFSET_TYPICAL_TIMEOUT_WRITE_BUFFER)));
            pFim->ulTimeoutWriteBufferMillisec *= (1 << (CHIP0 & *(pMedia + CFI_OFFSET_MAX_TIMEOUT_WRITE_BUFFER)));
            pFim->ulTimeoutWriteBufferMillisec  = (pFim->ulTimeoutWriteBufferMillisec / 1000) + 1;
        }

        pFim->ulTimeoutEraseBlockMillisec  = (1 << (CHIP0 & *(pMedia + CFI_OFFSET_TYPICAL_TIMEOUT_ERASE_BLOCK)));
        pFim->ulTimeoutEraseBlockMillisec *= (1 << (CHIP0 & *(pMedia + CFI_OFFSET_MAX_TIMEOUT_ERASE_BLOCK)));
    }


    /* *************************************************************************
        Debug sanity checks
       *************************************************************************
    */
    DclAssert(pFim->ulChipSize);
    DclAssert(pFim->FimInfo.ulBlockSize);
    DclAssert((pFim->ulChipSize % pFim->FimInfo.ulBlockSize) == 0);

    if(pFim->FimInfo.ulBootBlockSize)
    {
        DclAssert((pFim->ulChipSize % pFim->FimInfo.ulBootBlockSize) == 0);
        DclAssert((pFim->FimInfo.ulBlockSize % pFim->FimInfo.ulBootBlockSize) == 0);
        DclAssert(pFim->FimInfo.uBootBlockCountLow || pFim->FimInfo.uBootBlockCountHigh);
    }

    /* Timing sanity checks
    */
    DclAssert(pFim->ulTimeoutWriteWordMillisec);
    DclAssert(pFim->ulTimeoutEraseBlockMillisec);
    if(pFim->ulBufferWriteSize || pFim->ulTimeoutWriteBufferMillisec)
    {
        DclAssert(pFim->ulTimeoutWriteBufferMillisec);
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Pulic: ResetFlash()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void ResetFlash(
    NORFIMDATA     *pFim,
    D_UINT32        ulAddress)
{
    PFLASHDATA      pMedia = NULL;

    if(!FfxFimNorWindowMap(pFim->hDev, ulAddress, (volatile void **)&pMedia))
        return;
    DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));

    if (pFim->fAmdFlash)
    {
        *pMedia = AMDCMD_READ_MODE;
    }

    else if(pFim->fIntelFlash)
    {
        *pMedia = INTLCMD_READ_MODE;
        *pMedia = INTLCMD_CLEAR_STATUS;
        *pMedia = INTLCMD_READ_MODE;
    }
    return;
}



/*-------------------------------------------------------------------
    Pulic: DisplayFimInfo()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void DisplayFimInfo(
    NORFIMDATA     *pFim)
{
    D_UINT32        i = 0;

    /* Vendor and command set info
    */
    FFXPRINTF(1, ("NORFIM: ulPrimaryCmdSet:                 0x%08x\n", pFim->ulPrimaryCmdSet));
    FFXPRINTF(1, ("NORFIM: ulPrimaryTableAddress:           0x%08x\n", pFim->ulPrimaryTableAddress));
    FFXPRINTF(1, ("NORFIM: ulAlternateCmdSet:               0x%08x\n", pFim->ulAlternateCmdSet));
    FFXPRINTF(1, ("NORFIM: ulAlternateTableAddress:         0x%08x\n", pFim->ulAlternateTableAddress));

    if(pFim->ulPrimaryTableMajorVersion)
    {
        FFXPRINTF(1, ("NORFIM: Primary table version:           %c.%c\n",
            pFim->ulPrimaryTableMajorVersion, pFim->ulPrimaryTableMinorVersion));
    }
    if(pFim->ulAlternateTableMajorVersion)
    {
        FFXPRINTF(1, ("NORFIM: Alternate table version:         %c.%c\n",
            pFim->ulAlternateTableMajorVersion, pFim->ulAlternateTableMinorVersion));
    }

    FFXPRINTF(1, ("NORFIM: fEraseSuspendSupported:          %u\n", pFim->fEraseSuspendSupported));
    FFXPRINTF(1, ("NORFIM: EFimBlockLockSupport:      %u\n", pFim->eFimBlockLockSupport));

    FFXPRINTF(1, ("NORFIM: Manufacture                      0x%08x\n", pFim->ID.manufacture));
    FFXPRINTF(1, ("NORFIM: Device ID                        0x%08x/0x%08x/0x%08x\n",
        pFim->ID.device1, pFim->ID.device2, pFim->ID.device3));

    /* Chip geometry
    */
    FFXPRINTF(1, ("NORFIM: ulNumEraseBlockRegions:          %u\n", pFim->ulNumEraseBlockRegions));
    for(i = 0; i < pFim->ulNumEraseBlockRegions; i++)
    {
        FFXPRINTF(1, ("NORFIM:   Region %u blocks                0x%08x\n",
            i, pFim->FimEraseBlks[i].ulNumEraseBlocks));
        FFXPRINTF(1, ("NORFIM:   Region %u block size            0x%08x\n",
            i, pFim->FimEraseBlks[i].ulBlockSizeKB));
    }
    FFXPRINTF(1, ("NORFIM: ulChipSize:                      0x%08x\n", pFim->ulChipSize));
    FFXPRINTF(1, ("NORFIM: ulTotalBlocks:                   0x%08x\n", pFim->FimInfo.ulTotalBlocks));
    FFXPRINTF(1, ("NORFIM: ulChipBlocks:                    0x%08x\n", pFim->FimInfo.ulChipBlocks));
    FFXPRINTF(1, ("NORFIM: ulBlockSize:                     0x%08x\n", pFim->FimInfo.ulBlockSize));
    FFXPRINTF(1, ("NORFIM: ulBootBlockSize:                 0x%08x\n", pFim->FimInfo.ulBootBlockSize));
    FFXPRINTF(1, ("NORFIM: uBootBlockCountLow:              0x%08x\n", pFim->FimInfo.uBootBlockCountLow));
    FFXPRINTF(1, ("NORFIM: uBootBlockCountHigh:             0x%08x\n", pFim->FimInfo.uBootBlockCountHigh));
    FFXPRINTF(1, ("NORFIM: ulBufferWriteSize:               0x%08x\n", pFim->ulBufferWriteSize));

    /* Chip timing
    */
    FFXPRINTF(1, ("NORFIM: ulTimeoutWriteWordMillisec:      %u\n",  pFim->ulTimeoutWriteWordMillisec));
    FFXPRINTF(1, ("NORFIM: ulTimeoutWriteBufferMillisec:    %u\n",  pFim->ulTimeoutWriteBufferMillisec));
    FFXPRINTF(1, ("NORFIM: ulTimeoutEraseBlockMillisec:     %u\n",  pFim->ulTimeoutEraseBlockMillisec));

}


/*-------------------------------------------------------------------
    Public: WriteBufferedBytes()

    Write the specified bytes to flash using "buffered" write
    commands.  ulStart and ulLength must specify memory within
    one window.

    Parameters:
       pFim     - Pointer to FIM data
       ulStart  - The offset in bytes to begin programming data
       pBuffer  - The data to write
       ulLength - The length to write

    Return Value:
       TRUE  - If successful
       FALSE - If failed
-------------------------------------------------------------------*/
static D_BOOL WriteBufferedBytes(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart,
    const D_BUFFER *pBuffer,
    D_UINT32        ulLength)
{
    D_UINT32        ulBufferIndex = 0;
    D_UINT32        ulFlashIndex = 0;
    D_UINT32        ulDWordIndex = 0;
    D_UINT32        ulBaseLatch = 0;
    D_UINT32        ulBufferSize = 0;
    PFLASHDATA      pMedia = NULL;
    PFLASHDATA      pLatch1 = NULL;
    PFLASHDATA      pLatch2 = NULL;
    FLASHDATA      *pData;
    FLASHDATA       lastData;
    FLASHDATA       bounce; /*  for alignment fix-ups */
    D_UINT32        ulByteCount = 0;
    DCLTIMER        timer;

    DclAssert(pFim);
    DclAssert(pBuffer);
    DclAssert(ulLength);
    DclAssert(DCLISALIGNED(ulStart, sizeof(FLASHDATA)));
    DclAssert(DCLISALIGNED(ulLength, sizeof(FLASHDATA)));

    /* Start at a known state
    */
    ResetFlash(pFim, ulStart);

    /*  Get media pointers.

        We don't have to worry about the window size because that is already
        handled by Write().  It is probably the case that we do not need to do
        this mapping, but that level of attention and reworking of the code
        is beyond the scope of the window mapping update.
    */
    if(!FfxFimNorWindowMap(pFim->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;
    DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));

    if(pFim->fAmdFlash)
    {
        /*  ulBaseLatch is used to send commands and read status from
            the applicable erase zone.  This ensures that we are writing
            to the correct chip (if the array has multiple linear
            chips) or hardware partition (for RWW devices).
        */
        ulBaseLatch = ulStart & (~(pFim->FimInfo.ulBlockSize - 1));

        /*  To guarantee that we send all commands to the same erase zone,
            the LATCH_OFFSET values must be less than the erase zone size.
        */
        DclAssert(AMD_LATCH_OFFSET1 < pFim->FimInfo.ulBlockSize);
        DclAssert(AMD_LATCH_OFFSET2 < pFim->FimInfo.ulBlockSize);

        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET1, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET2, (volatile void **)&pLatch2))
            return FALSE;
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch1, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch2, sizeof(FLASHDATA)));
    }

    /*  Set buffer size
    */
    if(pFim->fIntelControlMode)
        ulBufferSize = pFim->ulIntelControlValidSize - (ulStart % (pFim->ulIntelControlValidSize + pFim->ulIntelControlInvalidSize));
    else
        ulBufferSize = pFim->ulBufferWriteSize - (ulStart % pFim->ulBufferWriteSize);

    /*  Program ulLength bytes into flash via buffer
    */
    for(ulBufferIndex = 0;
        ulBufferIndex < (ulLength / FLASH_BUS_BYTES);
        ulBufferIndex += (ulByteCount / FLASH_BUS_BYTES))
    {
        /*  Issue Write to Buffer Command
        */
        if(pFim->fAmdFlash)
        {
            *pLatch1 = AMDCMD_UNLOCK1;
            *pLatch2 = AMDCMD_UNLOCK2;
        }
        pMedia[ulBufferIndex] = pFim->ChipCommand.bufferWriteStart;

        ulByteCount = ulLength - (ulBufferIndex * FLASH_BUS_BYTES);

        if(pFim->fIntelControlMode)
        {
            if(ulByteCount > (pFim->ulBufferWriteSize>>1))
               ulByteCount = (pFim->ulBufferWriteSize>>1);
        }
        else
        {
            if(ulByteCount > pFim->ulBufferWriteSize)
               ulByteCount = pFim->ulBufferWriteSize;
        }

        ulByteCount = DCLMIN(ulByteCount, ulBufferSize);

        /*  In 32 bit mode, we write out dwords instead of bytes and the number
            must be written to both the low word and high word.
        */
        pMedia[ulBufferIndex] = MAKEFLASHDUPCMD((FLASHDATA)((ulByteCount / FLASH_BUS_BYTES) - 1));

        /*  Write Buffer Data
        */
        if(DCLISALIGNED((D_UINTPTR)pBuffer, sizeof(FLASHDATA)))
        {
            pData = (FLASHDATA *)pBuffer;
            for(ulDWordIndex = 0;
                ulDWordIndex < (ulByteCount / FLASH_BUS_BYTES);
                ulDWordIndex++)
            {
                pMedia[ulFlashIndex + ulDWordIndex] = pData[ulBufferIndex + ulDWordIndex];
            }
            lastData = pData[ulBufferIndex + ulDWordIndex -1];
        }
        else
        {
            for(ulDWordIndex = 0;
                ulDWordIndex < (ulByteCount / FLASH_BUS_BYTES);
                ulDWordIndex++)
            {
                DclMemCpy(&bounce, &pBuffer[(ulBufferIndex + ulDWordIndex) * sizeof(FLASHDATA)], sizeof(bounce));
                pMedia[ulFlashIndex + ulDWordIndex] = bounce;
            }
            lastData = bounce;
        }

        /*  Program Buffer to Flash
        */
        pMedia[ulBufferIndex] = pFim->ChipCommand.bufferWriteCommit;

        /*  Wait before polling
        */
        _sysdelay(WRITE_BUFFER_DELAY);

        DclTimerSet(&timer, pFim->ulTimeoutWriteBufferMillisec);

        if(!DidWriteComplete(pFim, &pMedia[ulFlashIndex + ulDWordIndex - 1], lastData, &timer))
        {
            if(DCLISALIGNED((D_UINTPTR)pBuffer, sizeof(FLASHDATA)))
            {
                pData = (FLASHDATA *)pBuffer;
                pData = &pData[ulBufferIndex + ulDWordIndex - 1];
            }
            else
            {
                DclMemCpy(&bounce, &pBuffer[(ulBufferIndex + ulDWordIndex - 1) * sizeof(FLASHDATA)], sizeof(bounce));
                pData = &bounce;
            }
            if(!HandleProgramFailure(pFim, ulStart,
                 &pMedia[ulFlashIndex + ulDWordIndex - 1], *pData))
            {
                DclError();
                return FALSE;
            }
        }

        if(pFim->fIntelControlMode)
        {
            ulFlashIndex += (ulByteCount + pFim->ulIntelControlInvalidSize) / FLASH_BUS_BYTES;
            ulBufferSize = pFim->ulIntelControlValidSize;
        }
        else
        {
            ulFlashIndex += (ulByteCount / FLASH_BUS_BYTES);
            ulBufferSize = pFim->ulBufferWriteSize;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: WriteBytes()

    Write the specified bytes to flash using "word" program commands.

    Parameters:
       pFim     - Pointer to FIM data
       ulStart  - The offset in bytes to begin programming data
       pBuffer  - The data to write
       ulLength - The length to write, in bytes

    Return Value:
       TRUE  - If successful
       FALSE - If failed
-------------------------------------------------------------------*/
static D_BOOL WriteBytes(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart,
    const D_BUFFER *pBuffer,
    D_UINT32        ulLength)
{
    D_UINT32        uProgramIndex;
    D_UINT32        ulBaseLatch;
    PFLASHDATA      pMedia;
    PFLASHDATA      pLatch1;
    PFLASHDATA      pLatch2;
    FLASHDATA       bounce; /*  for alignment fix-ups */
    FLASHDATA       lastData;
    FLASHDATA      *pData;
    DCLTIMER        timer;

    DclAssert(pFim);
    DclAssert(pBuffer);
    DclAssert(ulLength);
    DclAssert(DCLISALIGNED(ulStart, sizeof(FLASHDATA)));
    DclAssert(DCLISALIGNED(ulLength, sizeof(FLASHDATA)));

    if(!FfxFimNorWindowMap(pFim->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;
    DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));

    if(pFim->fAmdFlash)
    {
        /*  ulBaseLatch is used to send commands and read status from
            the applicable erase zone.  This ensures that we are writing
            to the correct chip (if the array has multiple linear
            chips) or hardware partition (for RWW devices).
        */
        ulBaseLatch = ulStart & (~(pFim->FimInfo.ulBlockSize - 1));

        /*  To guarantee that we send all commands to the same erase zone,
            the LATCH_OFFSET values must be less than the erase zone size.
        */
        DclAssert(AMD_LATCH_OFFSET1 < pFim->FimInfo.ulBlockSize);
        DclAssert(AMD_LATCH_OFFSET2 < pFim->FimInfo.ulBlockSize);

        /*  Use the unlock bypass command to save some bus cycles on AMD flash
        */
        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET1, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET2, (volatile void **)&pLatch2))
            return FALSE;
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch1, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch2, sizeof(FLASHDATA)));
        if(pFim->fAmdUnlockBypass)
        {
            *pLatch1 = AMDCMD_UNLOCK1;
            *pLatch2 = AMDCMD_UNLOCK2;
            *pLatch1 = AMDCMD_UNLOCK_BYPASS;
        }
    }

    /*  Divide ulLength (in bytes) by the number of bytes per write
    */
    for(uProgramIndex = 0;
        uProgramIndex < (ulLength / sizeof(FLASHDATA));
        ++uProgramIndex, pBuffer += sizeof(FLASHDATA))
    {
        if(pFim->fAmdFlash)
        {
            if(!pFim->fAmdUnlockBypass)
            {
                *pLatch1 = AMDCMD_UNLOCK1;
                *pLatch2 = AMDCMD_UNLOCK2;
            }
            *pLatch1 = pFim->ChipCommand.programWord;
        }
        else if(pFim->fIntelFlash)
        {
            *pMedia = pFim->ChipCommand.programWord;
        }

        if(DCLISALIGNED((D_UINTPTR)pBuffer, sizeof(FLASHDATA)))
        {
            pData = (FLASHDATA *)pBuffer;
        }
        else
        {
            DclMemCpy(&bounce, pBuffer, sizeof(bounce));
            pData = &bounce;
        }
        lastData = *pData;
        pMedia[uProgramIndex] = *pData;

        DclTimerSet(&timer, pFim->ulTimeoutWriteWordMillisec);

        if(!DidWriteComplete(pFim, &pMedia[uProgramIndex], lastData, &timer))
        {
            if(!HandleProgramFailure(pFim, ulStart, &pMedia[uProgramIndex], *pData))
            {
                DclError();
                return FALSE;
            }
        }
    }

    if(pFim->fAmdFlash && pFim->fAmdUnlockBypass)
    {
        *pMedia = AMDCMD_IDENTIFY;
        *pMedia = AMDCMD_BYPASS_RESET;
    }

    ResetFlash(pFim, ulStart);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DidWriteComplete()

    Polls until the write is complete, the write fails, or
    the timeout has expired.

    Parameters:
        pFim     - Pointer to FIM data
        pMedia   - The current location in the media
        lastData - Last data written to media, for data# polling
        ptimer   - Pointer to DCL timer

    Return Value:
        TRUE  - If the write completed successfully
        FALSE - If the write timed out or failed
-------------------------------------------------------------------*/
static D_BOOL DidWriteComplete(
    NORFIMDATA     *pFim,
    PFLASHDATA      pMedia,
    FLASHDATA       lastData,
    DCLTIMER       *ptimer)
{
    FLASHDATA       ulFlashStatus;
    FLASHDATA       ulToggleCheck;

    if (pFim->fAmdFlash)
    {
        ulFlashStatus = *pMedia;
        if ((ulFlashStatus & AMDSTAT_DONE) == (lastData & AMDSTAT_DONE))
        {
            ulToggleCheck = *pMedia;
            ulFlashStatus = *pMedia;
            if ((ulFlashStatus & AMDSTAT_DONE) == (lastData & AMDSTAT_DONE))
                return TRUE;
            else
                return FALSE;
        }
        while(!DclTimerExpired(ptimer))
        {
            ulToggleCheck = *pMedia;

            /*  check for write-to-buffer abort errors, for devices that
                support this feature...
            */
            if ((pFim->ulBufferWriteSize > 0) && ((ulToggleCheck & AMDSTAT_BUFFERABORT) != 0))
            {
            D_UINT32 i;

                /*  Saw a write-to-buffer abort once, check twice more
                */
                for (i = 0; i < 2; i++)
                {
                    ulToggleCheck = *pMedia;
                    if ((ulToggleCheck & AMDSTAT_BUFFERABORT) == 0)
                        break;
                }

                /*  If checked twice and data not valid, bail
                */
                if ((i == 2) && ((ulToggleCheck & AMDSTAT_DONE) != (lastData & AMDSTAT_DONE)))
                    return FALSE;
            }

            /*  Now we've verified no write buffer aborts; check status
            */
#if USE_TOGGLE_POLLING
            if((ulFlashStatus & AMDSTAT_TOGGLEDONE) == (ulToggleCheck & AMDSTAT_TOGGLEDONE))

                /* Didn't toggle, double-check with Data# polling
                */
#endif
#if USE_DATA_POLLING
                if ((ulToggleCheck & AMDSTAT_DONE) == (lastData & AMDSTAT_DONE))

                    /* Data# polling match- done!
                    */
#endif
                    return TRUE;
            ulFlashStatus = ulToggleCheck;
        }
    }
    else if (pFim->fIntelFlash)
    {
         while(!DclTimerExpired(ptimer))
        {
            ulFlashStatus = *pMedia;
            if((ulFlashStatus & INTLSTAT_DONE) == INTLSTAT_DONE)
            {
                ulFlashStatus = *pMedia;
                if((ulFlashStatus & INTLSTAT_WRITE_FAIL) == 0)
                    return TRUE;
            }
        }
   }

    /* Write timed out, check one more time for success
    */
    ulFlashStatus = *pMedia;
    if (pFim->fAmdFlash)
    {
        ulToggleCheck = *pMedia;
#if USE_TOGGLE_POLLING
        if((ulFlashStatus & AMDSTAT_TOGGLEDONE) == (ulToggleCheck & AMDSTAT_TOGGLEDONE))

            /*  Didn't toggle, double-check with Data# polling
            */
#endif
#if USE_DATA_POLLING
            if ((ulToggleCheck & AMDSTAT_DONE) == (lastData & AMDSTAT_DONE))
                /*  Data# polling match- done!
                */
#endif
                return TRUE;
    }
    else if(pFim->fIntelFlash)
    {
        if((ulFlashStatus & INTLSTAT_DONE) == INTLSTAT_DONE)
        {
            ulFlashStatus = *pMedia;
            if((ulFlashStatus & INTLSTAT_WRITE_FAIL) == 0)
            {
                return TRUE;
            }
            else if(pFim->ulPrimaryCmdSet == CMDSET_INTEL_EXTENDED_M18)
            {
                /* Provide more error data for Intel M18 and compatable chips
                */
                if((ulFlashStatus & INTLSTAT_OBJ_TO_CTL) == INTLSTAT_OBJ_TO_CTL)
                {
                    FFXPRINTF(1, ("NORFIM: Attempted write with Object mode data to control mode region\n"));
                }
                else if((ulFlashStatus & INTLSTAT_REWRITE_OBJ) == INTLSTAT_REWRITE_OBJ)
                {
                    FFXPRINTF(1, ("NORFIM: Attempted rewrite to object mode region\n"));
                }
                else if((ulFlashStatus & INTLSTAT_ILLEGAL_CMD) == INTLSTAT_ILLEGAL_CMD)
                {
                    FFXPRINTF(1, ("NORFIM: Write using illegal command\n"));
                }
            }
        }
    }

    /* Write failed or timed out without completing
    */
    return FALSE;
}


/*-------------------------------------------------------------------
    Public: HandleProgramFailure()

    Determines the type of error and which device(s) failed (if any)
    in order to handle the failure appropriately

    Parameters:
        pProgramMedia - The memory address of the current location
                        in the media that is being programmed.
        ulStart       - The offset in the flash (before WindowMap).
        expectedData  - The data expected when the operation is
                        done.

    Return Value:
       TRUE  - If the operation completed successfully afterall
       FALSE - If the device was reset appropriately
-------------------------------------------------------------------*/
static D_BOOL HandleProgramFailure(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart,
    PFLASHDATA      pProgramMedia,
    FLASHDATA       expectedData)
{
    FLASHDATA       ulFlashStatus;
    D_UINT32        ulBaseLatch;
    FLASHDATA       uAbortMask = 0;
    PFLASHDATA      pMedia = NULL;
    PFLASHDATA      pLatch1 = NULL;
    PFLASHDATA      pLatch2 = NULL;

    DclAssert(DCLISALIGNED(ulStart, sizeof(FLASHDATA)));

    if(pFim->fAmdFlash)
    {
        ulFlashStatus = *pProgramMedia;
        FFXPRINTF(1, ("NORFIM: HPF:Status Read:   %lX\n", ulFlashStatus));

        if(!FfxFimNorWindowMap(pFim->hDev, ulStart, (volatile void **)&pMedia))
            return FALSE;
        DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));

        /*  Check for Write-to-Buffer Abort
        */
#if (FLASH_INTERLEAVE == 4)
        if(ulFlashStatus & AMD_BUFF_ABORT_CHIP3)
            uAbortMask |= CHIP3;
        if(ulFlashStatus & AMD_BUFF_ABORT_CHIP2)
            uAbortMask |= CHIP2;
#endif
#if (FLASH_INTERLEAVE >= 2)
        if(ulFlashStatus & AMD_BUFF_ABORT_CHIP1)
            uAbortMask |= CHIP1;
#endif
        if(ulFlashStatus & AMD_BUFF_ABORT_CHIP0)
            uAbortMask |= CHIP0;

        /*  Separate read to make sure the operation hasn't just completed
        */
        ulFlashStatus = *pProgramMedia;
        FFXPRINTF(1, ("NORFIM: HPF2:Status Read:   %lX\n", ulFlashStatus));
        if((ulFlashStatus & AMDSTAT_DONE) == (expectedData & AMDSTAT_DONE))
        {
            /*  Operation completed successfully
            */
            return TRUE;
        }

        if(!uAbortMask)
        {
            /* no Write-to-Buffer Abort detected
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
        ulBaseLatch = ulStart & (~(pFim->FimInfo.ulBlockSize - 1));

        /*  Only abort reset the device(s) that are in the abort state.  Send
            the normal reset (read array) command to the other device (if only
            one device is in the abort state).
        */
        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET1, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowMap(pFim->hDev, ulBaseLatch | AMD_LATCH_OFFSET2, (volatile void **)&pLatch2))
            return FALSE;
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch1, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED((D_UINTPTR)pLatch2, sizeof(FLASHDATA)));

        *pLatch1 = (AMDCMD_UNLOCK1 & uAbortMask) | (AMDCMD_READ_MODE & ~uAbortMask);
        *pLatch2 = (AMDCMD_UNLOCK2 & uAbortMask) | (AMDCMD_READ_MODE & ~uAbortMask);
        *pLatch1 = AMDCMD_READ_MODE;
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    Public: FimUnlockBlocks()

    Abstracted interface for FIM create procedure to be sure that
    all blocks in the detected device are unlocked and ready to be
    written. Note that this must not be called before Create has
    called GetChipData (through IdChip), because this routine uses
    the pFim->eFimBlockLockSupported member of the NORFIMDATA
    structure, and this member is not initialized before
    GetChipData has been executed.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL FimUnlockBlocks(
    D_UINT32        ulLength,
    FFXFIMBOUNDS   *pBounds,
    NORFIMDATA     *pFim)
{
    switch(pFim->eFimBlockLockSupport)
    {
        case NoProtection:
            return TRUE;
        case IntelInstantBlockLock:
            return (IntelUnlockBlocks(ulLength, pBounds, pFim));
        case AmdAdvancedSectorProtection:
            return (AmdUnlockBlocks(ulLength, pBounds, pFim));
        default:
            return FALSE;
    }
}


/*-------------------------------------------------------------------
    Public: AmdUnlockBlocks()

    Unlocks each block within the usable area of the flash array.
    Does not unlock the blocks within the reserved space.  This
    should only be called by Create(). Uses algorithm unique to
    Amd Advanced Sector Protection (ASP) mechanism.

    ASP has several levels of protection:

    1) There is a "lock register", which configures the protection
       strategy and either enables or disables certain features of
       the scheme in in use. Care must be taken with this register
       because it is one-time programmable, and once programmed it
       cannot be changed.
    2) There is a universal PPB lock bit for the entire device that
       must be cleared in order to be able to modify the PPB bits.
    3) Each sector has a "persistent protection bit" (PPB) that can
       be modified by software only under conditions favorably
       dictated by the status of #1 and #2 above.
    4) Each sector has a "dynamic protection bit" that is either
       enabled or disabled at power-up according to a setting in
       the lock register.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL AmdUnlockBlocks(
    D_UINT32        ulLength,
    FFXFIMBOUNDS   *pBounds,
    NORFIMDATA     *pFim)
{
    D_UINT32        ulIndex;
    PFLASHDATA      pMedia = 0;
    PFLASHDATA      pLatch1, pLatch2;
    D_UINT32        ulTotalBlocks;
    FLASHDATA       statBits;
  #if 0
    D_BOOL          bPasswordProtect = FALSE;
    D_BOOL          bPersistentProtect = FALSE;
    D_BOOL          bPPBEraseDisabled = FALSE;
    D_BOOL          bPPBitsFrozen = TRUE;
  #endif

    FFXPRINTF(1, ("NORFIM: AMDUnlockBlocks() start\n"));

    /*  ulLength should never be zero at this point, only unlocking after an
        array has been found
    */
    if(ulLength == 0)
        return FALSE;

    /*  Adjust the length by applying the array bounds.  This will give us
        the correct amount of usable flash that we should be unlocking.
        Then start unlocking at the reserved bottom.  Because we are dealing
        with absolute offsets in the flash array, the ending offset will be
        the usable length plus the reserved bottom.
    */
    ulTotalBlocks = FfxDevApplyArrayBounds(ulLength / pFim->FimInfo.ulBlockSize, pFim->FimInfo.ulBlockSize, pBounds);
    if(ulTotalBlocks == D_UINT32_MAX)
        return FALSE;


    ulLength = ulTotalBlocks * pFim->FimInfo.ulBlockSize;

    ulIndex = pBounds->ulReservedBottomKB * 1024UL;
    if(!FfxFimNorWindowCreate(pFim->hDev, ulIndex, pBounds, (volatile void **)&pMedia))
        return FALSE;
    if(!FfxFimNorWindowCreate(pFim->hDev, ulIndex | AMD_LATCH_OFFSET1, pBounds, (volatile void **)&pLatch1))
        return FALSE;
    if(!FfxFimNorWindowCreate(pFim->hDev, ulIndex | AMD_LATCH_OFFSET2, pBounds, (volatile void **)&pLatch2))
        return FALSE;

    /*  Look at the sector protection bits in the ASP-enabled parts. These bits
        come programmed from the factory in some state of protection and may be
        interfering with our attempt to erase them.
        The first step is to read the lock register to see what is in it. Care
        must be taken not to write this register, because parts of it are
        one-time-programmable and once written, they cannot be changed.
    */

    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_LKREG_CMDSET_ENTRY;
    statBits = *pMedia;
    *pMedia  = AMDCMD_PROT_CMDSET_EXIT1;
    *pMedia  = AMDCMD_PROT_CMDSET_EXIT2;

  #if 0
    if (!(statBits & AMD_ASP_PASSWORD_PROTECTION_ENABLE_MASK))
        bPasswordProtect = TRUE;
    if (!(statBits & AMD_ASP_PERSISTENT_PROTECTION_ENABLE_MASK))
        bPersistentProtect = TRUE;
    if (!(statBits & AMD_ASP_PPB_ERASECOMMAND_ENABLE_MASK))
        bPPBEraseDisabled = TRUE;
  #endif

    /*  Now let's get the global protection freeze bit:
    */

    *pLatch1 = AMDCMD_UNLOCK1;
    *pLatch2 = AMDCMD_UNLOCK2;
    *pLatch1 = AMDCMD_GBL_PROTFREEZE_CMDSET_ENTRY;
    statBits = *pMedia;
    *pMedia  = AMDCMD_PROT_CMDSET_EXIT1;
    *pMedia  = AMDCMD_PROT_CMDSET_EXIT2;
  #if 0    
    if (!(statBits & AMD_ASP_GLOBAL_PPB_PROTECTION_MASK))
        bPPBitsFrozen = FALSE;
  #endif

    /*  Now go through the sectors one by one and be sure they are
        unlocked:
    */

    while(ulIndex < (ulLength + pBounds->ulReservedBottomKB * 1024UL))
    {
        if(!FfxFimNorWindowCreate(pFim->hDev, ulIndex, pBounds, (volatile void **)&pMedia))
            return FALSE;
        if(!FfxFimNorWindowCreate(pFim->hDev, ulIndex | AMD_LATCH_OFFSET1, pBounds, (volatile void **)&pLatch1))
            return FALSE;
        if(!FfxFimNorWindowCreate(pFim->hDev, ulIndex | AMD_LATCH_OFFSET2, pBounds, (volatile void **)&pLatch2))
            return FALSE;

        /*  Now let's read: Current sector PPB bit
            3) Current sector DYB bit
        */

        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pLatch1 = AMDCMD_NV_SECTPROT_CMDSET_ENTRY;
        statBits = *pMedia;
        *pMedia  = AMDCMD_PROT_CMDSET_EXIT1;
        *pMedia  = AMDCMD_PROT_CMDSET_EXIT2;

        /*  If the PPB bit is set, we may be limited in what we can do about
            it. If we are password- protected, or if PPB Erase command is
            disabled by the lock register, or the global PPB freeze bit is set,
            we will not be able to erase this sector and we will have to punt.
        */

        if (!(statBits & AMD_ASP_PPB_BIT_MASK))
        {
            
        /*  For the time being, if the PPB is set don't try to go any further.
            In the future we may want to revisit this if we decide to expand
            our support for flash protection mechanisms, but for now assume
            that if the PPB is set, someone has already used this device for
            something else and maybe doesn't want it erased. Some sort of 
            logic could be developed using the data read from the chip just
            above this while-loop (and that's why it is there) but for now
            let's table that. If and when we decide to do such things, this
            is the place to put it.
        */
                return FALSE;
        }

        /*  Handle Volatile protection bit (DYB) for this sector. If the
            protection bit is enabled, try to disable it.
        */

        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pLatch1 = AMDCMD_VOL_SECTPROT_CMDSET_ENTRY;
        statBits = *pMedia;
        *pMedia  = AMDCMD_PROT_CMDSET_EXIT1;
        *pMedia  = AMDCMD_PROT_CMDSET_EXIT2;

        if (!(statBits & AMD_ASP_DYB_BIT_MASK))
        {
            *pLatch1 = AMDCMD_UNLOCK1;
            *pLatch2 = AMDCMD_UNLOCK2;
            *pLatch1 = AMDCMD_VOL_SECTPROT_CMDSET_ENTRY;
            *pMedia = AMDCMD_VOL_SECTPROT_CMD_ADJUST;
            *pMedia = AMDCMD_VOL_SECTPROT_CLEAR;
            *pMedia  = AMDCMD_PROT_CMDSET_EXIT1;
            *pMedia  = AMDCMD_PROT_CMDSET_EXIT2;
        }

        /*  Verify volatile protection bit disabled:
        */
        *pLatch1 = AMDCMD_UNLOCK1;
        *pLatch2 = AMDCMD_UNLOCK2;
        *pLatch1 = AMDCMD_VOL_SECTPROT_CMDSET_ENTRY;
        statBits = *pMedia;
        *pMedia  = AMDCMD_PROT_CMDSET_EXIT1;
        *pMedia  = AMDCMD_PROT_CMDSET_EXIT2;
        if (!(statBits & AMD_ASP_DYB_BIT_MASK))
            return FALSE;

        /*  Increment index, if part has boot blocks, increment by bootblock
            size, which shouldn't have any ill effects except some inefficency
            when unlocking the standard erase blocks.
        */

        if(pFim->FimInfo.ulBootBlockSize)
            ulIndex += pFim->FimInfo.ulBootBlockSize;
        else
            ulIndex += pFim->FimInfo.ulBlockSize;

    }

    FFXPRINTF(1, ("NORFIM: AMDUnlockBlocks() exit\n"));

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: IntelUnlockBlocks()

    Unlocks each block within the usable area of the flash array.
    Does not unlock the blocks within the reserved space.  This
    should only be called by Create().

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL IntelUnlockBlocks(
    D_UINT32        ulLength,
    FFXFIMBOUNDS   *pBounds,
    NORFIMDATA     *pFim)
{
    D_UINT32        ulIndex, ulBlockStatus;
    PFLASHDATA      pMedia = 0;
    DCLTIMER        timer;
    D_UINT32        ulTotalBlocks;

    FFXPRINTF(1, ("NORFIM: IntelUnlockBlocks() start\n"));

    /*  ulLength should never be zero at this point, only unlocking after an
        array has been found
    */
    if(ulLength == 0)
        return FALSE;

    /*  Adjust the length by applying the array bounds.  This will give us
        the correct amount of usable flash that we should be unlocking.
        Then start unlocking at the reserved bottom.  Because we are dealing
        with absolute offsets in the flash array, the ending offset will be
        the usable length plus the reserved bottom.
    */
    ulTotalBlocks = FfxDevApplyArrayBounds(ulLength / pFim->FimInfo.ulBlockSize, pFim->FimInfo.ulBlockSize, pBounds);
    if(ulTotalBlocks == D_UINT32_MAX)
        return FALSE;

    ulLength = ulTotalBlocks * pFim->FimInfo.ulBlockSize;

    ulIndex = pBounds->ulReservedBottomKB * 1024UL;

    while(ulIndex < (ulLength + pBounds->ulReservedBottomKB * 1024UL))
    {
        if(!FfxFimNorWindowCreate(pFim->hDev, ulIndex, pBounds, (volatile void **)&pMedia))
            return FALSE;

        *pMedia = INTLCMD_LOCKUNLOCK;
        *pMedia = INTLCMD_CONFIRM;

        DclTimerSet(&timer, (pFim->ulTimeoutEraseBlockMillisec));

        while(!(*pMedia & INTLSTAT_DONE))
        {
            if(DclTimerExpired(&timer))
            {
                /*  Check once for for done in case we timed out due to
                    context switching or power management
                */
                if(!(*pMedia & INTLSTAT_DONE))
                {
                    /*  Clear the error status, reset to read mode and return
                    */
                    *pMedia = INTLCMD_READ_MODE;
                    *pMedia = INTLCMD_CLEAR_STATUS;
                    *pMedia = INTLCMD_READ_MODE;
                    return FALSE;
                }
            }
        }

        /*  This block should be unlocked now, but let's make sure
        */
        *pMedia = INTLCMD_IDENTIFY;
        ulBlockStatus = pMedia[INTEL_BLOCK_LOCK_STATUS_INDEX];

        if((ulBlockStatus & BLOCK_LOCKED) || (ulBlockStatus & BLOCK_LOCKED_DOWN))
        {
            /*  Clear the error status, reset to read mode and return
            */
            *pMedia = INTLCMD_READ_MODE;
            *pMedia = INTLCMD_CLEAR_STATUS;
            *pMedia = INTLCMD_READ_MODE;
            return FALSE;
        }

        /*  Block is not locked - Reset, and try next block!
        */
        *pMedia = INTLCMD_READ_MODE;
        *pMedia = INTLCMD_CLEAR_STATUS;
        *pMedia = INTLCMD_READ_MODE;

        /* Increment index, if part has boot blocks, increment by bootblock size,
            which shouldn't have any ill effects except some inefficency when
            unlocking the standard erase blocks.
        */
        if(pFim->FimInfo.ulBootBlockSize)
            ulIndex += pFim->FimInfo.ulBootBlockSize;
        else
            ulIndex += pFim->FimInfo.ulBlockSize;

    }

    *pMedia = INTLCMD_READ_MODE;
    *pMedia = INTLCMD_CLEAR_STATUS;
    *pMedia = INTLCMD_READ_MODE;

    FFXPRINTF(1, ("NORFIM: IntelUnlockBlocks() exit\n"));

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: ChipInfoCompare()

    Compares the appropriate NORFIMDATA values.  Used to determine
    if a simular chip has been found in a flash array.

    Parameters:
        pFim1  - Pointer to FIM structure
        pFim2  - Pointer to FIM structure

    Return Value:
        TRUE if matches, FALSE if not
-------------------------------------------------------------------*/
static D_BOOL ChipInfoCompare(
    NORFIMDATA     *pFim1,
    NORFIMDATA     *pFim2)
{
    D_UINT32        i;

    if(pFim1->fEraseSuspendSupported        != pFim2->fEraseSuspendSupported)
        return FALSE;

    if(pFim1->ulPrimaryCmdSet               != pFim2->ulPrimaryCmdSet)
        return FALSE;
    if(pFim1->ulPrimaryTableAddress         != pFim2->ulPrimaryTableAddress)
        return FALSE;
    if(pFim1->ulAlternateCmdSet             != pFim2->ulAlternateCmdSet)
        return FALSE;
    if(pFim1->ulAlternateTableAddress       != pFim2->ulAlternateTableAddress)
        return FALSE;

    if(pFim1->ulPrimaryTableMajorVersion    != pFim2->ulPrimaryTableMajorVersion)
        return FALSE;
    if(pFim1->ulPrimaryTableMinorVersion    != pFim2->ulPrimaryTableMinorVersion)
        return FALSE;
    if(pFim1->ulAlternateTableMajorVersion  != pFim2->ulAlternateTableMajorVersion)
        return FALSE;
    if(pFim1->ulAlternateTableMinorVersion  != pFim2->ulAlternateTableMinorVersion)
        return FALSE;

    if(pFim1->ulTimeoutWriteWordMillisec    != pFim2->ulTimeoutWriteWordMillisec)
        return FALSE;
    if(pFim1->ulTimeoutWriteBufferMillisec  != pFim2->ulTimeoutWriteBufferMillisec)
        return FALSE;
    if(pFim1->ulTimeoutEraseBlockMillisec   != pFim2->ulTimeoutEraseBlockMillisec)
        return FALSE;

    if(pFim1->ulNumEraseBlockRegions        != pFim2->ulNumEraseBlockRegions)
        return FALSE;
    if(pFim1->ulChipSize                    != pFim2->ulChipSize)
        return FALSE;
    if(pFim1->ulBufferWriteSize             != pFim2->ulBufferWriteSize)
        return FALSE;

    if(pFim1->FimInfo.ulBlockSize           != pFim2->FimInfo.ulBlockSize)
        return FALSE;
    if(pFim1->FimInfo.ulBootBlockSize       != pFim2->FimInfo.ulBootBlockSize)
        return FALSE;
    if(pFim1->FimInfo.uBootBlockCountLow    != pFim2->FimInfo.uBootBlockCountLow)
        return FALSE;
    if(pFim1->FimInfo.uBootBlockCountHigh   != pFim2->FimInfo.uBootBlockCountHigh)
        return FALSE;

    for(i = 0; i < pFim1->ulNumEraseBlockRegions; i++)
    {
        if(pFim1->FimEraseBlks[i].ulNumEraseBlocks  != pFim2->FimEraseBlks[i].ulNumEraseBlocks)
            return FALSE;
        if(pFim1->FimEraseBlks[i].ulBlockSizeKB     != pFim2->FimEraseBlks[i].ulBlockSizeKB)
            return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: ReadControl()

    Read a given number of bytes of data from the media in control
    mode.

    This function will handle formatting the data properly (ie,
    reading the programmed areas of flash to the buffer )

    Parameters:
        pFim     - The FIM instance handle
        ulStart  - high 22 bits are the physical address of page
                   start.  offset into page: bits 0-9 (bit 9 must
                   be zero for 1x16)
        ulLength - Number of bytes to transfer
        pBuffer  - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL ReadControl(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart,
    D_UINT32        ulLength,
    D_BUFFER       *pBuffer)
{
    D_UINT32        ulThisLength = 0;
    D_UINT32        ulFlashOffset = 0;
    D_UINT32        ulOffsetInPage = 0;
    D_BOOL          fSuccess = TRUE;

    DclAssert(pFim);
    DclAssert(ulLength);
    DclAssert(pBuffer);
    DclAssert(ulStart < pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);
    DclAssert((ulStart + ulLength) <= pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);

    pFim->fIntelControlMode = TRUE;

    /*  extract page address
    */
    ulFlashOffset = ulStart & M18_PAGE_MASK;

    /*  Get the offset within the page and check that it is with in the
        allowed size.
    */
    ulOffsetInPage = ulStart & M18_OFFSET_MASK;
    DclAssert(ulOffsetInPage < (D_UINT32)(pFim->uPageSize / 2));

    /*  Convert the offset within the page to account for the invalid regions.
    */
    ulOffsetInPage = ((ulOffsetInPage / pFim->ulIntelControlValidSize) *
                      (pFim->ulIntelControlValidSize + pFim->ulIntelControlInvalidSize))+
                      (ulOffsetInPage % pFim->ulIntelControlValidSize);

    ulFlashOffset += ulOffsetInPage;

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert((ulFlashOffset + ulLength) <= pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);


    /*  Align the first read to the control mode region size
    */
    ulThisLength = DCLMIN(ulLength,
                      (pFim->ulIntelControlValidSize -
                      (ulFlashOffset % (pFim->ulIntelControlValidSize + pFim->ulIntelControlInvalidSize))));

    while(ulLength)
    {
        /*  FfxFimNorRead will figure out window map.
        */
        fSuccess = Read(pFim, ulFlashOffset, ulThisLength, pBuffer);

        DclAssert(fSuccess);

        if(!fSuccess)
        {
            /*  Clear the error status, reset to read mode and return
            */
            fSuccess = FALSE;
            break;
        }

        /*  Go to the next offset
        */
        ulFlashOffset += (ulThisLength + pFim->ulIntelControlInvalidSize);
        pBuffer += ulThisLength;
        ulLength -= ulThisLength;

        /*  Recalculate the length of the next request
        */
        if(ulLength < pFim->ulIntelControlValidSize)
        {
            ulThisLength = ulLength;
        }
        else
        {
            ulThisLength = pFim->ulIntelControlValidSize;
        }
    }

    pFim->fIntelControlMode = FALSE;

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: WriteControl()

    Writes a given number of bytes of data out to the media
    (control mode).  It does not return until the data is
    programmed.

    This function will handle formatting the data properly (ie,
    writing to only the appropriate areas of the buffer)

    Parameters:
        pFim     - The FIM instance handle
        ulStart  - high 22 bits are the physical address of page
                   start.  offset into page: bits 0-9 (bit 9 must
                   be zero for 1x16)
        ulLength - Number of bytes to transfer
        pBuffer  - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
static D_BOOL WriteControl(
    NORFIMDATA     *pFim,
    D_UINT32        ulStart,
    D_UINT32        ulLength,
    const D_BUFFER *pBuffer)
{
    PFLASHDATA      pMedia = NULL;
    D_BOOL          fSuccess = TRUE;
    D_UINT32        ulFlashOffset = 0;
    D_UINT32        ulOffsetInPage = 0;

    DclAssert(pFim);
    DclAssert(ulLength);
    DclAssert(pBuffer);
    DclAssert(ulStart < pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);
    DclAssert((ulStart + ulLength) <= pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);
    DclAssert(DCLISALIGNED(ulStart, sizeof(FLASHDATA)));
    DclAssert(DCLISALIGNED(ulLength, sizeof(FLASHDATA)));

    pFim->fIntelControlMode = TRUE;

    /* Start with flash in a known state
    */
    ResetFlash(pFim, ulStart);

    /*  First get the address for the start of the page.
    */
    ulFlashOffset = ulStart & M18_PAGE_MASK;

    /*  Get the offset within the page and check that it is with in the
        allowed size.
    */
    ulOffsetInPage = ulStart & M18_OFFSET_MASK;
    DclAssert(ulOffsetInPage < (D_UINT32)(pFim->uPageSize / 2));

    /*  Convert the offset within the page to account for the invalid regions.
    */
    ulOffsetInPage = ((ulOffsetInPage / pFim->ulIntelControlValidSize) *
                     (pFim->ulIntelControlValidSize + pFim->ulIntelControlInvalidSize))+
                     (ulOffsetInPage % pFim->ulIntelControlValidSize);

    ulFlashOffset += ulOffsetInPage;

    /*  Verify user address and length parameters within the media boundaries.
    */
    DclAssert((ulFlashOffset + ulLength) <= pFim->FimInfo.ulTotalBlocks * pFim->FimInfo.ulBlockSize);

    while(ulLength)
    {
        D_UINT32    ulWindowSize;

        /*  Get the window size
        */
        ulWindowSize = FfxFimNorWindowMap(pFim->hDev, ulStart, (volatile void **)&pMedia);
        if(!ulWindowSize)
        {
            fSuccess = FALSE;
            break;
        }
        DclAssert(DCLISALIGNED((D_UINTPTR)pMedia, sizeof(FLASHDATA)));
        DclAssert(DCLISALIGNED(ulWindowSize, sizeof(FLASHDATA)));

        /*  Move each window worth of data into the flash memory
            (Allow WriteBufferedBytes to handle buffer alignment to page)
        */
        while(ulWindowSize && ulLength)
        {
            D_UINT32    ulThisLength = DCLMIN(ulLength, ulWindowSize);

            DclAssert(DCLISALIGNED(ulThisLength, sizeof(FLASHDATA)));
            DclAssert(DCLISALIGNED(ulFlashOffset, sizeof(FLASHDATA)));
            fSuccess = WriteBufferedBytes(pFim, ulFlashOffset, pBuffer, ulThisLength);
            if(!fSuccess)
            {
                DclError();
                break;
            }

            /*  If we have written everything, get outta here.
            */
            ulLength -= ulThisLength;
            if(ulLength == 0)
                break;

            /*  Go to the next offset
            */
            ulWindowSize    -= ulThisLength;
            ulFlashOffset   += (ulThisLength + pFim->ulIntelControlInvalidSize);
            pBuffer         += ulThisLength;

        }

        if(!fSuccess)
            break;
    }

    /* Leave flash in a known state
    */
    ResetFlash(pFim, ulStart);

    pFim->fIntelControlMode = FALSE;

    return fSuccess;
}


/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_norfim =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    Create,
    Destroy,
    IORequest,
    ParameterGet,
    ParameterSet
};

#endif  /* FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT */

