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

    This module contains the Media Manager implementation which is a subset
    of VBF which abstracts the interface to various types of flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: mediamgr.c $
    Revision 1.48  2010/12/12 07:05:00Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.47  2009/08/04 01:02:50Z  garyp
    Merged from the v4.0 branch.  Minor datatype changes.  Documentation
    updated -- no functional changes.
    Revision 1.46  2009/04/17 19:14:10Z  keithg
    Fixed bug 2634, replaced unintentional removal of buffer used in the
    READBACKVERIFY code.
    Revision 1.45  2009/04/09 02:58:23Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.44  2009/03/31 16:30:05Z  davidh
    Function headers updated for AutoDoc
    Revision 1.43  2009/03/12 22:28:41Z  billr
    Resolve bug 2124: Page buffers are allocated on the stack.
    Revision 1.42  2009/02/17 22:45:04Z  billr
    Fix compiler warning about unreferenced formal parameter.
    Revision 1.41  2009/01/29 17:01:40Z  billr
    Resolve bug 1601: VBF write protects the disk after an uncorrectable ECC
    error.  Implement new Media Manager function FfxMediaMgrCopyBadBlock().
    Allow per-instance data for media-specific functions.
    Revision 1.40  2009/01/23 22:28:20Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Comment
    changes only.
    Revision 1.39  2009/01/08 18:18:27Z  billr
    FfxMediaMgrReadLnuHeader() now returns a full FFXIOSTATUS.
    Revision 1.38  2009/01/08 00:50:13Z  billr
    FfxMediaMgrReadHeader() now returns a full FFXIOSTATUS.
    Revision 1.37  2008/06/03 19:15:51Z  thomd
    Added fFormatNoErase; removed ffxconf LFA conditions
    Revision 1.36  2008/04/08 00:22:58Z  keithg
    Added debug assertions.
    Revision 1.35  2008/03/23 20:43:09Z  Garyp
    Made the code conditional on FFXCONF_VBFSUPPORT.  Updated to accommodate
    interfaces which have changed to support variable length tags.
    Revision 1.34  2008/01/13 07:20:38Z  keithg
    Function header updates to support autodoc.
    Revision 1.33  2007/11/02 02:25:51Z  Garyp
    Eliminated the inclusion of limits.h.
    Revision 1.32  2007/11/01 22:52:03Z  pauli
    Corrected trace printf format strings.
    Revision 1.31  2007/11/01 00:57:19Z  Garyp
    Major update to eliminate the inconsistent use of D_UINT16 types.  All block
    indexes and counts which were D_UINT16 are now consistently unsigned.
    Replaced the use of Return_t and IOReturn_t with FFXSTATUS and FFXIOSTATUS.
    Improved error handling.
    Revision 1.30  2007/10/25 16:51:25Z  billr
    Fix compiler warning about unused variable in some build configurations.
    Revision 1.29  2007/10/23 21:17:45Z  billr
    Resolve bug 1556: Write interruption tests fail.
    Revision 1.28  2007/10/16 23:39:04Z  Garyp
    Minor type change to keep MSVC6 happy.
    Revision 1.27  2007/10/10 23:07:34Z  billr
    Resolve bug 412.
    Revision 1.26  2007/10/08 19:31:23Z  billr
    Resolve bug 970.
    Revision 1.25  2007/08/31 23:22:26Z  pauli
    Updated FfxMMBuildEUH() and FfxMMParseEUH() to handle the case where the
    region number/client address field is all FF's (for a spare unit).
    Revision 1.24  2007/07/19 21:05:55Z  timothyj
    Made large addressing support a compile-time configurable option.
    Revision 1.23  2007/06/25 22:06:26Z  timothyj
    Fixed large flash array region-number vs. client-offset flag usage.
    Revision 1.22  2007/04/21 21:34:15Z  Garyp
    Added a typecast so we build cleanly.
    Revision 1.21  2007/04/20 23:43:10Z  timothyj
    Added fLargeAddressing parameter, to indicate whether the on-media format
    should use large flash array compatible addressing.
    Revision 1.20  2007/04/12 00:38:04Z  timothyj
    Changed interfaces to FfxMediaMgrFormatUnit() and FfxMediaMgrReadHeader()
    to take raw block offsets in lieu of linear byte offsets.  Moved functions
    that became NOR-specific after changes to support large NAND arrays out of
    this module and into mmnor.c and mmiswf.c (RawUnitSize, UnitLinearAddress,
    and DataBlockAddress).
    Revision 1.19  2007/04/07 03:35:38Z  Garyp
    Modified the "ParameterGet" function so that the buffer size for a given
    parameter can be queried without actually retrieving the parameter.
    Revision 1.18  2006/11/08 20:30:49Z  Garyp
    Added ParameterGet/Set() functionality.  Cleaned up some bogus
    initialization logic.
    Revision 1.17  2006/10/18 17:39:53Z  billr
    Revert changes made in revision 1.16.
    Revision 1.15  2006/10/05 23:27:03Z  Garyp
    Added FfxMMDataBlockNum().
    Revision 1.14  2006/08/20 00:09:16Z  Garyp
    Added debug code.
    Revision 1.13  2006/04/29 18:49:17Z  Garyp
    Eliminated partitionStartUnit from the EUH (now a reserved field).
    Revision 1.12  2006/03/20 20:28:20Z  Garyp
    Fixed a problem with NOR formatting where the MergeWrites functionality
    was not handled properly (introduced recently).
    Revision 1.11  2006/03/15 21:39:30Z  Garyp
    Removed a reference to a VBF function.
    Revision 1.10  2006/03/05 03:01:55Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.9  2006/02/26 01:59:52Z  Garyp
    Split out the NOR and NAND tag handling logic.
    Revision 1.8  2006/02/22 20:09:23Z  Garyp
    Fixed to build cleanly.
    Revision 1.7  2006/02/21 02:53:11Z  Garyp
    Changed EUH fields back to using block terminology, rather than pages, to
    be consistent with other internal VBF fields (from a client perspective,
    VBF operates on pages).
    Revision 1.6  2006/02/17 00:37:58Z  billr
    Merge support for flash partitioning.
    Revision 1.5  2006/02/08 21:49:10Z  Garyp
    Modified to use the updated FML interface.
    Revision 1.4  2006/01/09 19:29:44Z  billr
    Merge Sibley support from v2.01.
    Revision 1.3  2006/01/05 02:06:51Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.2  2005/12/25 14:47:29Z  Garyp
    Added/updated debugging code -- no functional changes.
    Revision 1.1  2005/12/08 03:08:52Z  Garyp
    Initial revision
    Revision 1.5  2005/11/28 20:26:48Z  Garyp
    Added an undocumented feature which allows the read-back verify
    functionality to be enabled, but in a less verbose fashion.
    Revision 1.4  2005/11/14 20:29:38Z  Garyp
    Eliminated VBF_BLOCK_OVERHEAD vestiges.
    Revision 1.3  2005/10/26 03:16:29Z  Garyp
    Eliminated the use of STD_NAND_DATA_SIZE.  Changed some terminology from
    "blocks" to "pages" for clarity.  Began changing from a compile-time block
    size to a run-time page size paradigm.
    Revision 1.2  2005/10/22 08:13:25Z  garyp
    Debug code updated.
    Revision 1.1  2005/10/14 02:37:50Z  Garyp
    Initial revision
    Revision 1.2  2005/10/12 04:56:38Z  Garyp
    Added ReadBackVerify.
    Revision 1.1  2005/10/02 02:58:40Z  Garyp
    Initial revision
    Revision 1.3  2005/08/24 18:56:00Z  pauli
    Added default definition of FX_FULLSOURCE to prevent undefined macro
    warnings.
    Revision 1.2  2005/08/21 04:46:25Z  garyp
    Eliminated // comments.
    Revision 1.1  2005/08/03 19:31:44Z  pauli
    Initial revision
    Revision 1.4  2005/08/03 19:31:44Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 03:52:37Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.1  2005/07/25 21:11:10Z  pauli
    Initial revision
    Revision 1.28  2005/09/09 21:28:17Z  johnb
    Modified achSig size to +1 so that the ADS 1.2 compiler will not complain.
    Revision 1.27  2005/08/31 22:38:41Z  Cheryl
    Define achSig as the VBF_SIGNATURE.  This gets around an
    obfuscation problem with string handling.
    Revision 1.26  2005/08/27 21:22:56Z  Cheryl
    Move specific flash type functions to their own files.  Start renaming
    remaining functions with FfxMediaMgr prefix
    Revision 1.25  2005/01/23 00:27:57Z  billr
    Can't include limits.h in a kernel build.
    Revision 1.24  2004/12/30 23:17:54Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.23  2004/09/14 22:56:57Z  GaryP
    Changed some data types to mollify the MSVC6 compiler.
    Revision 1.22  2004/08/30 20:47:15Z  billr
    Resolve IR 4488: NAND hidden address wrong if VBF unit size > 256K.
    Minor comment cleanup.
    Revision 1.21  2004/01/14 20:14:26Z  garys
    Merge from FlashFXMT
    Revision 1.18.1.6  2004/01/14 20:14:26  garyp
    Fixed some types in the BuildEUH(), ParseEUH(), and checksum functions
    so we build cleanly under DOS.
    Revision 1.18.1.5  2003/12/16 22:04:38Z  garyp
    Eliminated the inclusion of vbfdata.h.
    Revision 1.18.1.4  2003/12/08 18:21:39Z  billr
    Eliminate statically allocated buffer.
    Revision 1.18.1.3  2003/11/27 00:32:15Z  billr
    Changes for thread safety. Compiles (Borland/DOS), not yet tested.
    Revision 1.18.1.2  2003/11/03 05:54:38Z  garyp
    Re-checked into variant sandbox.
    Revision 1.19  2003/11/03 05:54:38Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.18  2003/09/02 17:14:31Z  billr
    No functional changes. Comment change only.
    Revision 1.17  2003/06/11 18:52:01Z  billr
    checksum() needed to specify unsigned characters. Comment changes.
    Revision 1.16  2003/06/10 23:31:04Z  dennis
    Refactored BBM and MGM FIMs for better obfuscation.  BBM format logic is now
    in bbminit() function in BBMFMT.C.  Moved BBM.H and TRACE.H to ffx\include
    directory.  Reduced the clear text tokens in obfuscated files (using cobf
    -hi switch and by refactoring header files)  Changed definition of vbf
    writeint to pacify obfuscator.
    Revision 1.15  2003/06/09 23:52:10  billr
    Implement FIM "merge writes" characteristic for writing and formatting (was
    previously only done for discards).
    Revision 1.14  2003/05/29 06:33:14Z  garyp
    Added a typecast to mollify the CE3 ARM compiler
    Revision 1.13  2003/05/23 00:10:37Z  billr
    Remove isNAND flag from the EUH. The Media Manager now gets this
    information via oemmediainfo() during vbfmount().
    Revision 1.12  2003/05/21 02:29:32Z  garyp
    Updated to include TSTWINT.H instead of VBFWINT.H.  Eliminated
    the use of NO_DISPLAY.
    Revision 1.11  2003/05/19 18:05:12Z  billr
    Media Manager now modifies its behavior to accommodate MLC flash
    and flash parts that don't allow writing a '1' where a '0' was previously
    programmed. A FIM controls this by setting bits in uDeviceType in
    the ExtndMediaInfo structure.
    Revision 1.10  2003/05/15 00:09:28Z  billr
    WriteToLnu() and ReadFromLnu() buffers are now void* rather than D_UCHAR*.
    Revision 1.9  2003/05/14 17:40:25Z  billr
    Add new Media Manager function ExpendLnuAllocations().
    Revision 1.8  2003/05/14 17:29:30Z  billr
    No functional changes. Use static instead of static. Remove some
    obsolete prototypes.
    Revision 1.7  2003/05/13 18:29:04Z  billr
    Add debug trace facility and fxtrace tool (DOS only at present).
    Revision 1.6  2003/05/01 02:03:32Z  billr
    Reorganize headers: include/vbf.h is now nothing but the public interface.
    VBF internal information has moved to core/vbf/_vbf.h.  core/vbf/_vbflowl.h
    is obsolete.  A new header include/vbfconf.h contains definitions the OEM
    may want to change.  There were a few changes to types, and to names of
    manifest constants to accommodate this.
    Revision 1.5  2003/04/29 17:57:43Z  billr
    Clean up compiler warnings in WinCE build.
    Revision 1.4  2003/04/25 22:00:16Z  billr
    Fix the reserved space issue the right way. The Media Manager takes
    care of the offset, the Allocator always starts with linear unit number 0.
    Revision 1.3  2003/04/22 00:38:08Z  dennis
    Added more write interruption tests and code to deal with them.
    GCs will now fully clean a region if called by a background process
    without disturbing temporal order.  GC "granularity" is decreased.
    Revision 1.2  2003/04/10 20:54:37  billr
    Merge from VBF4 branch.
    Revision 1.1.1.3  2003/04/10 20:54:37Z  billr
    On the way to resolving the ReadEUH() and FormatUnit() mess.  Add write
    interruptions. Mark discards correctly on NOR. Remove some dead code.
    Revision 1.1.1.2  2003/04/07 23:51:05Z  billr
    Make ValidNANDAlloc() calculate ECC correctly.
    Revision 1.1  2003/04/04 22:40:24Z  billr
    Initial revision
    ---------------------
    Bill Roman 2003-03-14
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_VBFSUPPORT

#include <vbf.h>
#include <fxfmlapi.h>
#include <vbfint.h>
#include <mediamgr.h>

#include "mmint.h"

#if FFXCONF_NANDSUPPORT
#include "mmnand.h"
#endif

#if FFXCONF_NORSUPPORT
#include "mmnor.h"
#endif

#if FFXCONF_ISWFSUPPORT
#include "mmiswf.h"
#endif

#if FFX_MEDIAMGR_READBACKVERIFY
static D_BOOL ReadBackVerify(const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nBlockIndex, unsigned  nBlockCount, const D_BUFFER *pClientData);
#endif

/*  This is a const to get around obfuscator not treating strings
    consistently in all cases
*/
const D_UCHAR achSig[VBF_SIGNATURE_SIZE+1] = {"\xCC\xDD" "DL_FS4.00\xFF\xFF\xFF\xFF\xFF"};


/*-------------------------------------------------------------------
    Validate parameters for a data transfer: the length must not be
    zero, and all blocks must lie within a unit.
-------------------------------------------------------------------*/
#define VALID_RANGE(part, start, len)                 \
   ((len) != 0                                        \
    && (start) < (part)->uBlocksPerUnit               \
    && (len) <= ((D_UINT32)((part)->uBlocksPerUnit) - (start)))


typedef enum
{
    FLASH_NAND, FLASH_NOR, FLASH_ISWF
} FlashType_t;


/*-------------------------------------------------------------------
                    Prototypes for local functions
-------------------------------------------------------------------*/
static D_UINT16     checksum(const void *, ptrdiff_t);

#define DUMP_EUH_DEBUG 3

#if D_DEBUG >= DUMP_EUH_DEBUG
static void DumpEUH(const EUH * pEUH);
#define DUMP_EUH(pEUH) DumpEUH((pEUH))
#else
#define DUMP_EUH(pEUH) do { (void) 0; } while (0)
#endif



                /*---------------------------------*\
                 *                                 *
                 *    Public MediaMgr Interface    *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Protected: FfxMediaMgrInstanceCreate()

    Create a VBF Media Manager instance.

    Determine characteristics of the media that will affect Media
    Manager behavior and save the information in the partition
    structure for later reference.

    Parameters:
        hFML - The FML handle

    Return Value:
        Returns an HMEDIAMGR handle if successful, NULL otherwise.
-------------------------------------------------------------------*/
HMEDIAMGR FfxMediaMgrInstanceCreate(
    FFXFMLHANDLE        hFML)
{
    FFXFMLINFO          FmlInfo;
    FFXFMLDEVINFO       FmlDevInfo;
    MEDIAMGRINSTANCE   *pMM;

    DclAssert(hFML);

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return NULL;

    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
        return NULL;

    pMM = DclMemAllocZero(sizeof *pMM);
    if(!pMM)
    {
        FFXPRINTF(1, ("Error allocating MM instance data\n"));
        DclError();
        return NULL;
    }

    pMM->hFML = hFML;

    /*  Record media characteristics
    */
    pMM->uPageSize  = FmlInfo.uPageSize;
    pMM->uSpareSize = FmlInfo.uSpareSize;
    pMM->uDevType   = FmlInfo.uDeviceType;

    switch(FmlInfo.uDeviceType)
    {
      #if FFXCONF_NANDSUPPORT
        case DEVTYPE_NAND:
        case DEVTYPE_AND:
            /*  NAND devices have to allow at least as much metadata as
                our basic tag size.
            */
            if(FmlDevInfo.uMetaSize < FFX_NAND_TAGSIZE)
                goto ErrorCleanup;

            /*  If the metadata size does not exactly match our tag size,
                make sure variable length tags are supported, otherwise
                we cannot initialize.
            */
            if((FmlDevInfo.uMetaSize != FFX_NAND_TAGSIZE)
                && (!(FmlDevInfo.uDeviceFlags & DEV_VARIABLE_LENGTH_TAGS)))
                goto ErrorCleanup;

            if (FfxMMNandDeviceInit(pMM) != FFXSTAT_SUCCESS)
                goto ErrorCleanup;
            break;
      #endif

      #if FFXCONF_NORSUPPORT
        case DEVTYPE_ROM:
        case DEVTYPE_RAM:
        case DEVTYPE_ATA:
        case DEVTYPE_NOR:
            FfxMMNorDeviceInit(pMM);
            break;
      #endif

      #if FFXCONF_ISWFSUPPORT
        case DEVTYPE_ISWF:
            FfxMMISWFDeviceInit(pMM);
            break;
      #endif

        default:
            goto ErrorCleanup;
    }

    /*  No FIM should ever set both DEV_MERGE_WRITES and DEV_PGM_ONCE,
        it is impossible to satisfy both at once.  If they are both set,
        DEV_PGM_ONCE takes precedence, as the preliminary spec sheet
        that contained this restriction warned that violating it would
        produce unerasable bits.
    */
    if((FmlDevInfo.uDeviceFlags & (DEV_MERGE_WRITES | DEV_PGM_ONCE)) == (DEV_MERGE_WRITES | DEV_PGM_ONCE))
    {
        DclError();
        FmlDevInfo.uDeviceFlags &= (D_UINT16)(~DEV_MERGE_WRITES);
    }

    /*  Some flash parts won't accept writing a '1' to a bit already
        programmed to '0'.  It's necessary to merge new data with
        existing data for such flash.
    */
    pMM->mergeWrites = (FmlDevInfo.uDeviceFlags & DEV_MERGE_WRITES) != 0;

    /*  NOTE: This flag applies to MLC NOR only, it has nothing to do
        with MLC NAND.

        "Multi-level cell" (MLC) flash stores more than one bit per
        cell.  Attempting to program one bit can corrupt one or more
        other nearby bits that share the cell.  (The design of the Media
        Manager assumes "nearby" to mean within an aligned sixteen bit
        region.)  To deal with this possible corruption, the Media
        Manager defaults to using two separate writes to discard an
        allocation.  If the flash is known not to be MLC a single write
        may be used, improving discard performance.
    */
    pMM->multiLevelCell = (FmlDevInfo.uDeviceFlags & DEV_NOT_MLC) == 0;

    /*  NOTE: This flag has different meanings for NOR and NAND.

        Some preliminary spec sheets for NOR flash parts prohibit
        programming a bit to zero more than once.

        MLC NAND flash does not allow more than one program operation
        per page before erasing; on some parts it may actually be
        physically impossible.
    */
    pMM->programOnce = (FmlDevInfo.uDeviceFlags & DEV_PGM_ONCE) != 0;

    return pMM;

  ErrorCleanup:
    DclError();

    DclMemFree(pMM);

    return NULL;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrInstanceDestroy()

    Destroy a Media Manager instance.

    Parameters:
        hMM - The handle for the Media Manager instance to destroy.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxMediaMgrInstanceDestroy(
    HMEDIAMGR       hMM)
{
    DclAssert(hMM);

    if (hMM->pMmOps->Destroy)
        hMM->pMmOps->Destroy(hMM);

    DclMemFree(hMM);

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrWriteToLnu()

    Writes data to a unit in a manner appropriate to the media
    type.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nBlockIndex - starting block (allocation index) in unit
        nBlockCount - block count to write, must be non-zero.
        nBlockKey   - retrieval key (i.e., client address) for the
                      first block.  Keys for successive blocks are
                      formed by incrementing this value.
        pClientData - buffer containing data to write

    Return Value:
        Returns an FFXIOSTATUS structure indicating operation result.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxMediaMgrWriteToLnu(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex,
    unsigned            nBlockCount,
    unsigned            nBlockKey,
    const void         *pClientData)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACEINDENT),
        "FfxMediaMgrWriteToLnu() LNU=%U BlkIndex=%u BlkCount=%u BlkKey=%u\n",
        lnu, nBlockIndex, nBlockCount, nBlockKey));

    DclProfilerEnter("FfxMediaMgrWriteToLnu", 0, nBlockCount);

    DclAssert(pVBF);
    DclAssert(VALID_RANGE(pVBF, nBlockIndex, nBlockCount));
    DclAssert(pVBF->uAllocationBlockSize == pVBF->hMM->uPageSize);
    DclAssert(pVBF->hMM->pMmOps->WriteBlocks);

    ioStat = pVBF->hMM->pMmOps->WriteBlocks(pVBF, lnu, nBlockIndex,
                                            nBlockCount, nBlockKey, pClientData);

  #if FFX_MEDIAMGR_READBACKVERIFY
    if(IOSUCCESS(ioStat, nBlockCount))
    {
        if(!ReadBackVerify(pVBF, lnu, nBlockIndex, nBlockCount, pClientData))
            ioStat.ffxStat = FFXSTAT_MEDIAMGR_VERIFYFAILED;
    }
  #endif

/*  WriteCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACEUNDENT),
        "FfxMediaMgrWriteToLnu() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrReadFromLnu()

    Read blocks of data from a logical unit.

    Parameters:
        pVBF          - A pointer to the VBFDATA structure to use
        lnu           - linear unit index
        nBlockIndex   - starting block (allocation index) in unit
        nBlockCount   - block count to write
        pClientBuffer - buffer to receive data

    Return Value:
        Returns an FFXIOSTATUS structure indicating operation result.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxMediaMgrReadFromLnu(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex,
    unsigned            nBlockCount,
    void               *pClientBuffer)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACEINDENT),
        "FfxMediaMgrReadFromLnu() LNU=%U BlkIndex=%u BlkCount=%u\n",
        lnu, nBlockIndex, nBlockCount));

    DclProfilerEnter("FfxMediaMgrReadFromLnu", 0, nBlockCount);

    DclAssert(pVBF);
    DclAssert(VALID_RANGE(pVBF, nBlockIndex, nBlockCount));
    DclAssert(pVBF->uAllocationBlockSize == pVBF->hMM->uPageSize);
    DclAssert(pVBF->hMM->pMmOps->ReadBlocks);

    ioStat = pVBF->hMM->pMmOps->ReadBlocks(pVBF, lnu, nBlockIndex, nBlockCount, pClientBuffer);

/*  ReadCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACEUNDENT),
        "FfxMediaMgrReadFromLnu() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrReadLnuHeader()

    Read an EUH from a unit of a partition, unpacking the fields
    into an EUH structure.  Check it for validity by checking the
    signature and validation code.  Note that there is no
    consistency or sanity checking of the fields, as only the
    allocator understands what they mean.

    Parameters:
        pVBF - A pointer to the VBFDATA structure to use
        lnu  - linear unit index
        pEUH - where to put the unpacked unit header

    Return Value:
        Returns FFXSTAT_SUCCESS if the header was read successfully.
        The destination EUH structure contents represent the header
        from the media.

        Returns FFXSTAT_MEDIAMGR_NOEUH or FFXSTAT_MEDIAMGR_BADEUH if
        no EUH was found on the media.  This is usually due to an
        interrupted format operation.

        Otherwise a standard status code is returned.

        If the return value is not FFXSTAT_SUCCESS, the destination
        structure contents are undefined (may have been modified).
-------------------------------------------------------------------*/
FFXIOSTATUS FfxMediaMgrReadLnuHeader(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    EUH                *pEUH)
{
    FFXIOSTATUS         ioStat;
    const D_UINT32      ulStart = pVBF->uTotalBlocksPerUnit * lnu;

    /*  Note that the conversion of linear unit number to page number
        above assumes that the VBF allocation block size is equal to
        the page size.
    */
    DclAssert(pVBF->uAllocationBlockSize == pVBF->hMM->uPageSize);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACEINDENT),
        "FfxMediaMgrReadLnuHeader() LNU=%U\n", lnu));

    DclProfilerEnter("FfxMediaMgrReadLnuHeader", 0, 0);

    if (pVBF->hMM->pMmOps->ReadHeader)
        ioStat = pVBF->hMM->pMmOps->ReadHeader(pVBF->hMM, ulStart, pEUH);
    else
        ioStat = FfxMediaMgrReadHeader(pVBF->hFML, ulStart, pEUH);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
        "FfxMediaMgrReadLnuHeader() returning %lX\n", ioStat.ffxStat));

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrReadHeader()

    Read an EUH from a linear address, unpacking the fields into
    an EUH structure.  Check it for validity by checking the
    signature and validation code.  Note that there is no
    consistency or sanity checking of the fields, as only the
    allocator understands what they mean.

    Parameters:
        hFML    - The handle for the FML device
        ulStart - Starting allocation block index of the unit
        pEUH    - Where to put the unpacked unit header

    Return Value:
        Returns FFXSTAT_SUCCESS if the header was read successfully.
        The destination EUH structure contents represent the header
        from the media.

        Returns FFXSTAT_MEDIAMGR_NOEUH or FFXSTAT_MEDIAMGR_BADEUH if
        no EUH was found on the media.  This is usually due to an
        interrupted format operation.

        Otherwise a standard status code is returned.

        If the return value is not FFXSTAT_SUCCESS, the destination
        structure contents are undefined (may have been modified).
-------------------------------------------------------------------*/
FFXIOSTATUS FfxMediaMgrReadHeader(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulStart,
    EUH            *pEUH)
{
    FFXIOSTATUS     ioStat = NULL_IO_STATUS;
    FFXFMLINFO      FmlInfo;

    DclAssert(hFML);
    DclAssert(pEUH);

    ioStat.ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if (ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    DclProfilerEnter("FfxMediaMgrReadHeader", 0, 0);

    switch(FmlInfo.uDeviceType)
    {
      #if FFXCONF_NANDSUPPORT
        case DEVTYPE_NAND:
        case DEVTYPE_AND:
        {
            void *pPage = DclMemAlloc(FmlInfo.uPageSize);

            if (pPage)
            {
                ioStat = FfxMMNandReadHeader(hFML, ulStart, pEUH, pPage);
                DclMemFree(pPage);
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_OUTOFMEMORY;
            }
            break;
        }
      #endif

      #if FFXCONF_NORSUPPORT
        case DEVTYPE_ROM:
        case DEVTYPE_RAM:
        case DEVTYPE_ATA:
        case DEVTYPE_NOR:
            ioStat = FfxMMNorReadHeader(hFML, ulStart, pEUH);
            break;
      #endif

      #if FFXCONF_ISWFSUPPORT
        case DEVTYPE_ISWF:
            ioStat = FfxMMISWFReadHeader(hFML, ulStart, pEUH);
            break;
      #endif

        default:
            DclError();
            ioStat.ffxStat = FFXSTAT_UNSUPPORTEDREQUEST;
            break;
    }

    DclProfilerLeave(0UL);

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrFormatLnu()

    Given a unit and euh erase the unit and write out an erase
    unit header.  This function is shared between the client
    interface, garbage collection, and unit recovery code.

    Parameters:
        pVBF - A pointer to the VBFDATA structure to use
        lnu  - linear unit index
        pEUH - information to write into the new erase unit
               header.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrFormatLnu(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    const EUH          *pEUH)
{
    FFXSTATUS           ffxStat;
    const D_UINT32      ulStart = pVBF->uTotalBlocksPerUnit * lnu;

    /*  Note that the conversion of linear unit number to page number
        above assumes that the VBF allocation block size is equal to
        the page size.
    */
    DclAssert(pVBF->uAllocationBlockSize == pVBF->hMM->uPageSize);

    if (pVBF->hMM->pMmOps->FormatUnit)
        ffxStat = pVBF->hMM->pMmOps->FormatUnit(pVBF->hMM, ulStart, pEUH,
                                                (D_BOOL) pVBF->fLargeAddressing, FALSE);
    else
        ffxStat = FfxMediaMgrFormatUnit(pVBF->hFML, ulStart, pEUH,
                                        (D_BOOL) pVBF->fLargeAddressing, FALSE);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrFormatUnit()

    Given the address of a unit, erase it and write out an erase
    unit header. This function is shared between the client
    interface, garbage collection, and unit recovery code.

    Parameters:
        ulUnitStart      - block index of the first raw block of the
                           unit.
        pEUH             - information to write into the new erase
                           unit header.
        fLargeAddressing - indicates whether the on-media format uses
                           values that are capable of supporting
                           large flash arrays (> 2GB).
        fFormatNoErase   - skip the erase step as part of format;
                           requires parts erased by the factory

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrFormatUnit(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulUnitStart,
    const EUH      *pEUH,
    D_BOOL          fLargeAddressing,
    D_BOOL          fFormatNoErase)
{
    FFXSTATUS       ffxStat;
    FFXFMLINFO      FmlInfo;

    DclAssert(hFML);
    DclAssert(pEUH);

    ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    switch(FmlInfo.uDeviceType)
    {
  #if FFXCONF_NANDSUPPORT
        case DEVTYPE_NAND:
        case DEVTYPE_AND:
        {
            void *pPage = DclMemAlloc(FmlInfo.uPageSize);

            if (pPage)
            {
                ffxStat = FfxMMNandFormatEraseUnit(hFML, ulUnitStart, pEUH,
                                                   fLargeAddressing, fFormatNoErase, pPage);
                DclMemFree(pPage);
            }
            else
            {
                ffxStat = FFXSTAT_OUTOFMEMORY;
            }
            break;
        }
  #endif

  #if FFXCONF_NORSUPPORT
        case DEVTYPE_ROM:
        case DEVTYPE_RAM:
        case DEVTYPE_ATA:
        case DEVTYPE_NOR:
        {
            FFXFMLDEVINFO   FmlDevInfo;
            D_BOOL          fMergeWrites;

            ffxStat = FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo);
            if(ffxStat != FFXSTAT_SUCCESS)
                break;

            fMergeWrites = (FmlDevInfo.uDeviceFlags & (DEV_MERGE_WRITES | DEV_PGM_ONCE))
                == DEV_MERGE_WRITES;

            /*  "fMergeWrites" is necessary to handle some NOR flash device-
                specific characteristics.  Some NOR flash will report an error
                on an attempt to write a one to a bit previously programmed to
                zero.  Other NOR flash preliminary spec sheets claim the parts
                will be damaged by writing a zero to a bit that is already
                zero.  Note that a FIM should never report that both of these
                requirements are true!
            */
            DclAssert(fLargeAddressing == FALSE);
            ffxStat = FfxMMNorFormatEraseUnit(hFML, ulUnitStart * FmlInfo.uPageSize, pEUH, fMergeWrites, fFormatNoErase);

            break;
        }
      #endif

      #if FFXCONF_ISWFSUPPORT
        case DEVTYPE_ISWF:
        {
            DclAssert(fLargeAddressing == FALSE);
            ffxStat = FfxMMISWFFormatEraseUnit(hFML, ulUnitStart * FmlInfo.uPageSize, pEUH, fFormatNoErase);
            break;
        }
      #endif

        default:
            DclError();
            ffxStat = FFXSTAT_UNSUPPORTEDREQUEST;
            break;
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrCalcEUHDataUnitBlocks()

    Determine how many blocks of the unit are available for data
    storage after taking out the EUH and any space for the
    allocation list.

    Parameters:
        pEUH - erase unit header information.

    Return Value:
        Returns the number of blocks available for allocation in the
        unit.
-------------------------------------------------------------------*/
unsigned FfxMediaMgrCalcEUHDataUnitBlocks(
    FFXFMLHANDLE    hFML,
    const EUH      *pEUH)
{
    unsigned        nReturn;
    FFXFMLINFO      FmlInfo;

    DclAssert(pEUH);
    DclAssert(hFML);

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return 0;

    switch(FmlInfo.uDeviceType)
    {
      #if FFXCONF_NANDSUPPORT
        case DEVTYPE_NAND:
        case DEVTYPE_AND:
            nReturn = FfxMMNandCalcEUHDataUnitBlocks(pEUH);
            break;
      #endif

      #if FFXCONF_NORSUPPORT
        case DEVTYPE_ROM:
        case DEVTYPE_RAM:
        case DEVTYPE_ATA:
        case DEVTYPE_NOR:
        {
            nReturn = FfxMMNorCalcEUHDataUnitBlocks(pEUH);
            break;
        }
      #endif

      #if FFXCONF_ISWFSUPPORT
        case DEVTYPE_ISWF:
        {
            nReturn = FfxMMISWFCalcEUHDataUnitBlocks(pEUH);
            break;
        }
      #endif

        default:
            DclError();
            nReturn = 0;
            break;
    }

    return nReturn;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrReadLnuAllocations()

    Read allocation information, decode and unpack it into an
    array of Allocation structures.

    Parameters:
        pVBF             - A pointer to the VBFDATA structure to use
        lnu              - linear unit index
        nStartAlloc      - index of first allocation entry to read
        nMaxAllocs       - maximum number of allocations to read
        rgaUnitAllocList - allocation structures to receive
                           allocations

    Return Value:
        Returns an FFXIOSTATUS structure indicating operation result.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxMediaMgrReadLnuAllocations(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nStartAlloc,
    unsigned            nMaxAllocs,
    Allocation          rgaUnitAllocList[])
{
    DclAssert(pVBF->hMM->pMmOps->ReadAllocations);

    return pVBF->hMM->pMmOps->ReadAllocations(pVBF, lnu, nStartAlloc, nMaxAllocs, rgaUnitAllocList);
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrDiscardLnuAllocations()

    Mark allocations discarded if possible.

    Parameters:
        pVBF         - A pointer to the VBFDATA structure to use
        lnu          - linear unit index
        nFirstAlloc  - first allocation to mark discarded.
        nAllocCount  - number of allocations to discard.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrDiscardLnuAllocations(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nFirstAlloc,
    unsigned            nAllocCount)
{
    FFXIOSTATUS         ioStat;

    if(!VALID_RANGE(pVBF, nFirstAlloc, nAllocCount))
        return FFXSTAT_OUTOFRANGE;

    DclAssert(pVBF->hMM->pMmOps->DiscardAllocations);

    ioStat = pVBF->hMM->pMmOps->DiscardAllocations(pVBF, lnu, nFirstAlloc, nAllocCount);

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrExpendLnuAllocations()

    Mark free allocations as no longer valid for use.  They will
    be reported by ReadLnuAllocations with state ALLOC_BAD.

    The specified allocations must all have state ALLOC_FREE when
    this function is called.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nFirstAlloc - first allocation to mark discarded.
        nAllocCount - number of allocations to discard.

    Return Value:
        Returns an FFXIOSTATUS structure indicating operation result.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxMediaMgrExpendLnuAllocations(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nFirstAlloc,
    unsigned            nAllocCount)
{
    FFXIOSTATUS         ioStat = DEFAULT_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACEINDENT),
         "FfxMediaMgrExpendLnuAllocations() LNU=%U Start=%u Count=%u\n",
        lnu, nFirstAlloc, nAllocCount));

    DclAssert(pVBF->hMM->pMmOps->ExpendAllocations);

    if(VALID_RANGE(pVBF, nFirstAlloc, nAllocCount))
    {
        ioStat = pVBF->hMM->pMmOps->ExpendAllocations(pVBF, lnu, nFirstAlloc, nAllocCount);
    }
    else
    {
        ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
         "FfxMediaMgrExpendLnuAllocations() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hMM       - The MediaManager handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the buffer in which to store the
                    value.  May be NULL which causes this function
                    to return the size of buffer required to store
                    the parameter.
        ulBuffLen - The size of buffer.  May be zero if pBuffer is
                    NULL.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
        If pBuffer is NULL, but the id is valid, the returned
        FFXSTATUS value will be decodeable using the macro
        DCLSTAT_GETUINT20(), which will return the buffer length
        required to store the parameter.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrParameterGet(
    HMEDIAMGR       hMM,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxMediaMgrParameterGet() hMM=%P ID=%x pBuff=%P Len=%lU\n",
        hMM, id, pBuffer, ulBuffLen));

    DclAssert(hMM);
    DclAssert(id);

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, so pass the request to
                the layer below us.
            */
            ffxStat = FfxFmlParameterGet(hMM->hFML, id, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxMediaMgrParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrParameterSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hMM       - The MediaManager handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrParameterSet(
    HMEDIAMGR       hMM,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxMediaMgrParameterSet() hMM=%P ID=%x pBuff=%P Len=%lU\n",
        hMM, id, pBuffer, ulBuffLen));

    DclAssert(hMM);
    DclAssert(id);
    DclAssert(pBuffer);
    DclAssert(ulBuffLen);

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, so pass the request to
                the layer below us.
            */
            ffxStat = FfxFmlParameterSet(hMM->hFML, id, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxMediaMgrParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrCopyBadBlock()

    Copies an allocation block that has an uncorrectable error in its
    data in a way that preserves the allocation entry and the
    uncorrectable error indication, along with whatever data (probably
    corrupted) is available.

    This operation is only meaningful on NAND (or similar) flash.

    Parameters:
        pVBF       - A pointer to the VBFDATA structure to use
        lnuFrom    - linear unit index of the unit containing the
                     allocation block with an uncorrectable error
        nBlockFrom - block (allocation index) in unit to copy from
        lnuTo      - linear unit index of the unit to copy to
        nBlockTo   - block (allocation index) in unit to copy to

    Return Value:
        FFXSTAT_SUCCESS - the block was copied successfully.
        FFXSTAT_UNSUPPORTEDFUNCTION - not implemented for this media.
        Other FFXSTATUS values indicate the operation was attempted
        but failed.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrCopyBadBlock(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnuFrom,
    unsigned            nBlockFrom,
    LinearUnit_t        lnuTo,
    unsigned            nBlockTo)
{
    FFXSTATUS           ffxStat = FFXSTAT_UNSUPPORTEDFUNCTION;

#if FFXCONF_NANDSUPPORT

    if (!VALID_RANGE(pVBF, nBlockFrom, 1) || !VALID_RANGE(pVBF, nBlockTo, 1))
        return FFXSTAT_OUTOFRANGE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
                    "FfxMediaMgrCopyBadBlock() lnu %lU blk %u -> lnu %lU blk %u\n",
                    (D_UINT32) lnuFrom, nBlockFrom, (D_UINT32) lnuTo, nBlockTo));

    /*  This function only needs to be implemented on NAND-like media.  If it
        is not implemented, it must report "not supported".
    */
    if (pVBF->hMM->pMmOps->CopyBadBlock)
        ffxStat = pVBF->hMM->pMmOps->CopyBadBlock(pVBF, lnuFrom, nBlockFrom, lnuTo, nBlockTo);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
                    "FfxMediaMgrCopyBadBlock() returning %lX\n", ffxStat));

#endif

    return ffxStat;
}




#if FFXCONF_NANDSUPPORT

/*-------------------------------------------------------------------
    Protected: FfxMediaMgrBlankCheck()

    Checks an allocation block and determines whether it is
    entirely blank (unprogrammed).

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nBlockIndex - block (allocation index) in unit

    Return Value:
        Returns FFXSTAT_SUCCESS if the block is blank (unprogrammed),
        or FFXSTAT_MEDIAMGR_NOTBLANK if the block is not blank,
        otherwise a standard status code is returned for other types
        of errors.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrBlankCheck(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex)
{
    FFXSTATUS           ffxStat;

    if(!VALID_RANGE(pVBF, nBlockIndex, 1))
        return FFXSTAT_OUTOFRANGE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
                    "FfxMediaMgrBlankCheck() LNU=%U Block=%u\n", lnu, nBlockIndex));

    /*  This function only needs to be implemented on NAND-like media.  If it
        is not implemented, it must report "blank".
    */
    if (pVBF->hMM->pMmOps->BlankCheck)
        ffxStat = pVBF->hMM->pMmOps->BlankCheck(pVBF, lnu, nBlockIndex);
    else
        ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
                    "FfxMediaMgrBlankCheck() returning %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrVerify()

    Checks an allocation block and determines whether it is
    valid or has an uncorrectable ECC error.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nBlockIndex - block (allocation index) in unit

    Return Value:
        Returns FFXSTAT_SUCCESS if the page can be successfully
        read and contains no uncorrectable errors.  If the page
        has uncorrectable errors, FFXSTAT_FIMUNCORRECTABLEDATA
        will be returned.  Otherwise a standard status code will
        be returned in the event of some other error.
-------------------------------------------------------------------*/
FFXSTATUS FfxMediaMgrVerify(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex)
{
    FFXSTATUS           ffxStat;

    if(!VALID_RANGE(pVBF, nBlockIndex, 1))
        return FFXSTAT_OUTOFRANGE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
                    "FfxMediaMgrVerify() LNU=%U Block=%u\n", lnu, nBlockIndex));

    /*  This function only needs to be implemented on NAND-like media.  If it
        is not implemented, it must report "good".
    */
    if (pVBF->hMM->pMmOps->Verify)
        ffxStat = pVBF->hMM->pMmOps->Verify(pVBF, lnu, nBlockIndex);
    else
        ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
                    "FfxMediaMgrVerify() returning %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMediaMgrIsNand()

    Reports whether the partition requires special handling
    appropriate for NAND flash.

    Parameters:
        pVBF - A pointer to the VBFDATA structure to use

    Return Value:
        TRUE indicates that the flash needs to be treated like
        NAND, FALSE indicates that it does not.
-------------------------------------------------------------------*/
D_BOOL FfxMediaMgrIsNand(
    const VBFDATA  *pVBF)
{
    FFXFMLINFO      FmlInfo;

    FfxFmlDiskInfo(pVBF->hFML, &FmlInfo);

    return (FmlInfo.uDeviceType == DEVTYPE_NAND
            || FmlInfo.uDeviceType == DEVTYPE_AND);
}

#endif



                /*---------------------------------*\
                 *                                 *
                 *   Internal MediaMgr Interface   *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Protected: FfxMMIsErasedAlloc()

    Tests an image of an allocation entry read from the media to
    see whether it is in its erased state.

    Parameters:
        pBuffer    - an allocation entry in the on-media format
        nBufferLen - The number of bytes to check

    Return Value:
        Returns TRUE if all bits of the allocation entry are in the
        erased state, FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxMMIsErasedAlloc(
    const D_BUFFER *pBuffer,
    unsigned        nBufferLen)
{
    unsigned        k;

    for(k=0; k<nBufferLen; k++)
    {
        if(pBuffer[k] != ERASED_8)
            return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: FfxMMBuildEUH()

    Create the image of an on-media representation of an EUH.

    Parameters:
       pNewEuh          - where to put the converted EUH.  There must
                          be at least VBF_MIN_BLOCK_SIZE bytes
                          available.
       pEUH             - the EUH structure to convert to its on-
                          media representation.
       fLargeAddressing - indicates whether the on-media information
                          uses large flash array (>2GB) addressing or
                          not

    Return Value:
        None.
 -------------------------------------------------------------------*/
void FfxMMBuildEUH(
    D_BUFFER   *pNewEuh,
    const EUH  *pEUH,
    D_BOOL      fLargeAddressing)
{
#define EUH_ITEM(item, buffer)                      \
   (DclMemCpy((buffer), &(item), sizeof (item)),    \
   DCLLITTLE((buffer), sizeof (item)),              \
   sizeof (item))

    D_BUFFER        *p = pNewEuh;
    D_UINT16         sum;
    D_UINT32         regionNumber;

    DUMP_EUH(pEUH);

    DclAssert(pEUH);
    DclAssert(pNewEuh);

    /*  Store the EUH signature in the buffer.
    */
    DclMemCpy(p, achSig, VBF_SIGNATURE_SIZE);
    p += VBF_SIGNATURE_SIZE;

    /*  Copy individual fields from the EUH structure to the buffer,
        fixing endianness in the process.
    */
    /*
        For backward compatibility, the location used by the
        regionNumber is overloaded.

        When backward compatibility is no longer a concern,
        (for instance, when the VBF on-media format changes for
        some other reason), this logic and the corresponding flag
        can be removed and ALL instances of this field in the
        on-media EUH can represent a client allocation block.
    */
    regionNumber = pEUH->regionNumber;
    if(regionNumber == ERASED32)
    {
        /*  Special case for an EUH that represents a spare unit.
            In this case, the same value is stored regardless if
            large addressing is being used or not.
        */
        p += EUH_ITEM(regionNumber, p);
    }
    else
    {
        if (fLargeAddressing)
        {
            /*  For large addresses, the regionNumber is exactly
                that, but requires a flag embedded indicating such.
            */
            DclAssert((regionNumber & VBF_REGION_NUMBER_INDICATOR) == 0);
            regionNumber |= VBF_REGION_NUMBER_INDICATOR;
            p += EUH_ITEM(regionNumber, p);
        }
        else
        {
            /*  For small addresses, the on-media location that corresponds to
                regionNumber actually holds the linear byte offset address.
                Note everything within this else case is NOT large flash
                array (>2GB array support) compatible.
            */
            D_UINT32 regionSize;
            D_UINT32 unitSize;
            D_UINT32 clientAddress;

            unitSize = pEUH->unitClientBlocks * pEUH->uAllocBlockSize;
            regionSize = unitSize * pEUH->lnuPerRegion;
            clientAddress = regionSize * pEUH->regionNumber;

            /*  Ensure the product of the region number and region size fits into 31 bits.
            */
            DclAssert(pEUH->regionNumber < (VBF_REGION_NUMBER_INDICATOR / regionSize));
            DclAssert((((pEUH->regionNumber * regionSize)) & VBF_REGION_NUMBER_INDICATOR) == 0);

            p += EUH_ITEM(clientAddress, p);
        }
    }

    p += EUH_ITEM(pEUH->eraseCount, p);
    p += EUH_ITEM(pEUH->serialNumber, p);
    p += EUH_ITEM(pEUH->ulSequenceNumber, p);
    p += EUH_ITEM(pEUH->lnuTotal, p);
    p += EUH_ITEM(pEUH->lnuTag, p);
    p += EUH_ITEM(pEUH->numSpareUnits, p);
    p += EUH_ITEM(pEUH->uAllocBlockSize, p);
    p += EUH_ITEM(pEUH->lnuPerRegion, p);
    p += EUH_ITEM(pEUH->uReserved, p);
    p += EUH_ITEM(pEUH->uUnitTotalBlocks, p);
    p += EUH_ITEM(pEUH->unitClientBlocks, p);
    p += EUH_ITEM(pEUH->unitDataBlocks, p);

    /*  Calculate and store the validation code for the buffer contents
        (now that the pointer has been advanced to after the last field).
    */
    sum = checksum(pNewEuh, p - pNewEuh);
    p += EUH_ITEM(sum, p);

    DclAssert(p - pNewEuh <= pEUH->uAllocBlockSize);
    (void)p;                    /* some broken compilers (anybody recognize Borland bcc32
                                 * 5.2?)  warn that "'p' is assigned a value that is
                                 * never used" if not a debug build. */

#undef EUH_ITEM

}


/*-------------------------------------------------------------------
    Protected: FfxMMParseEUH()

    Unpack the image of an on-media representation of an EUH to
    an EUH structure.

    Parameters:
       pData - buffer containing data to be converted to an EUH.
       pEUH  - the EUH structure to hold the information retrieved
               from its on-media  representation.

    Return Value:
        Returns TRUE if the signature and checksum were correct,
        FALSE if not.
-------------------------------------------------------------------*/
D_BOOL FfxMMParseEUH(
    const D_BUFFER *pData,
    EUH            *pEUH)
{
#define EUH_ITEM(item, buffer)                      \
   (DclMemCpy(&(item), (buffer), sizeof (item)),    \
    DCLNATIVE(&(item), sizeof (item)),              \
    sizeof (item))

    const D_BUFFER   *p = pData;
    D_UINT16          sum;
    D_UINT32          regionNumber;

    /*  Validate the EUH signature from the buffer.  There's no need to
        transfer it to the EUH structure.
    */
    if(DclMemCmp(pData, achSig, VBF_SIGNATURE_SIZE) != 0)
    {
        return FALSE;
    }

    p += VBF_SIGNATURE_SIZE;

    /*  Copy individual fields from the buffer to the EUH structure,
        fixing endianness in the process.
    */
    /*
        For backward compatibility, the location used by the
        regionNumber is overloaded.

        When backward compatibility is no longer a concern,
        (for instance, when the VBF on-media format changes for
        some other reason), this logic and the corresponding flag
        can be removed and ALL instances of this field in the
        on-media EUH can represent a client allocation block.
    */
    p += EUH_ITEM(regionNumber, p);

    p += EUH_ITEM(pEUH->eraseCount, p);
    p += EUH_ITEM(pEUH->serialNumber, p);
    p += EUH_ITEM(pEUH->ulSequenceNumber, p);
    p += EUH_ITEM(pEUH->lnuTotal, p);
    p += EUH_ITEM(pEUH->lnuTag, p);
    p += EUH_ITEM(pEUH->numSpareUnits, p);
    p += EUH_ITEM(pEUH->uAllocBlockSize, p);
    p += EUH_ITEM(pEUH->lnuPerRegion, p);
    p += EUH_ITEM(pEUH->uReserved, p);
    p += EUH_ITEM(pEUH->uUnitTotalBlocks, p);
    p += EUH_ITEM(pEUH->unitClientBlocks, p);
    p += EUH_ITEM(pEUH->unitDataBlocks, p);

    /*  Get the checksum and verify it.
    */
    (void)EUH_ITEM(sum, p);     /* some broken compilers (anybody
                                 * recognize Borland bcc32 5.2?)  warn
                                 * that "Code has no effect" unless the
                                 * value is explicitly voided. */

    /*  If the regionNumber/client address field contains ERASED32, it could
        mean that this is a spare unit.  In that case, do not try to interpret
        if it is a region number or a client address.  Just return it as is,
        so it will never be mistaken for a real region number.
    */
    if(regionNumber != ERASED32)
    {
        /*  Determine whether this EUH was encoded using large addressing
            compatible region numbers or backward compatible clientAddresses
        */
        if ((regionNumber & VBF_REGION_NUMBER_INDICATOR) == VBF_REGION_NUMBER_INDICATOR)
        {
            /*  For large addresses, the regionNumber we parsed above is
                exactly that, but contains a flag embedded indicating such.
                Remove it before saving the region number in the EUH structure.
            */
            regionNumber &= ~VBF_REGION_NUMBER_INDICATOR;
        }
        else
        {
            /*  For backward compatible (small) addresses, the on-media
                location that corresponds to regionNumber actually holds
                the linear byte offset address.  Note everything within
                this else case is NOT large flash array (>2GB array support)
                compatible.
            */
            D_UINT32 unitSize = pEUH->unitClientBlocks * pEUH->uAllocBlockSize;
            D_UINT32 regionSize = unitSize * pEUH->lnuPerRegion;

            DclAssert((((regionNumber / regionSize)) & VBF_REGION_NUMBER_INDICATOR) == 0);
            DclAssert( regionNumber % regionSize == 0);

            regionNumber /= regionSize;
        }
    }

    pEUH->regionNumber = regionNumber;

    DUMP_EUH(pEUH);

    return (sum == checksum(pData, p - pData));

#undef EUH_ITEM

}


/*-------------------------------------------------------------------
    Protected: FfxMMDataBlockNum()

    Obtain the linear address of a data block specified in terms
    of partition, linear unit index, and block (allocation)
    index.

    Note that the linear unit index and block index are not range
    checked here; it's the caller's responsibility.

    Parameters:
        pVBF   - A pointer to the VBFDATA structure to use
        lnu    - Linear unit index of the unit containing the block
                 being addressed.
        nBlock - The (client) block index within the unit.

    Return Value:
        A block/page number suitable for use as an argument to
        FMLREAD/WRITE_PAGES().
-------------------------------------------------------------------*/
D_UINT32 FfxMMDataBlockNum(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlock)
{
    DclAssert(pVBF);

    /*  The first available client block in a unit is uBlocksPerUnit
        back from uTotalBlocksPerUnit (blocks in the unformatted
        unit).
    */
    return (lnu * pVBF->uTotalBlocksPerUnit) +
        (pVBF->uTotalBlocksPerUnit - pVBF->uBlocksPerUnit + nBlock);
}



                /*---------------------------------*\
                 *                                 *
                 *         Local Functions         *
                 *                                 *
                \*---------------------------------*/




/*-------------------------------------------------------------------
    Local: checksum()

    Calculates a sixteen bit sum of an object.

    Parameters:
        pData  - pointer to some object
        length - size of the object

    Return Value:
        The sum of the unsigned chars comprising the object.
-------------------------------------------------------------------*/
static D_UINT16 checksum(
    const void         *pData,
    ptrdiff_t           length)
{
    D_UINT16            sum = 0;
    const unsigned char *p = pData;

    while(length--)
    {
        sum += *p++;
    }
    return sum;
}



#if D_DEBUG >= DUMP_EUH_DEBUG
/*-------------------------------------------------------------------
    Local: DumpEUH()

    Dump the contents of an EUH to the debug device.

    Parameters:
        pEUH - A pointer to the EUH structure to dump

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DumpEUH(
    const EUH   *pEUH)
{
    DclPrintf("EUH:SeqNum %10lU  LnuTotal %10lU  LnuPerRegion %6U\n",
              pEUH->ulSequenceNumber, pEUH->lnuTotal, pEUH->lnuPerRegion);
    DclPrintf("    RegNum %10lU  TotalBlocks  %6U  BlockSize    %6U\n",
              pEUH->regionNumber, pEUH->uUnitTotalBlocks, pEUH->uAllocBlockSize);
    DclPrintf("    Erases %10lU  DataBlocks   %6U  Reserved     %6U\n",
              pEUH->eraseCount, pEUH->unitDataBlocks, pEUH->uReserved);
    DclPrintf("    SerNum %lX  ClientBlocks %6U\n",
              pEUH->serialNumber, pEUH->unitClientBlocks);
    DclPrintf("    LnuTag %lX  SpareUnits   %6U\n",
              pEUH->lnuTag, pEUH->numSpareUnits);
}
#endif



#if FFX_MEDIAMGR_READBACKVERIFY

/*-------------------------------------------------------------------
    Local: ReadBackVerify()

    Parameters:

    Returns Value:
-------------------------------------------------------------------*/
static D_BOOL ReadBackVerify(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex,
    unsigned            nBlockCount,
    const D_BUFFER     *pClientData)
{
    DCLALIGNEDBUFFER    (buffer, data, VBF_MAX_BLOCK_SIZE);
    D_BOOL              fSuccess = FALSE;
    D_UINT16            uBlockSize = pVBF->uAllocationBlockSize;

    /*  Undocumented behavior!  If the readback-verify setting is TRUE*2, it
        is enabled, but quietly.
    */
  #if FFX_MEDIAMGR_READBACKVERIFY != (TRUE * 2)
    FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEINDENT),
        "FfxMM:ReadBackVerify() Lnu=%U Start=%u Count=%u\n", lnu, nBlockIndex, nBlockCount));
  #endif

    DclAssert(pVBF->hMM->pMmOps->ReadBlocks);

    while(nBlockCount)
    {
        FFXIOSTATUS ioStat;

        ioStat = pVBF->hMM->pMmOps->ReadBlocks(pVBF, lnu, nBlockIndex, 1, buffer.data);
        if(!IOSUCCESS(ioStat, 1))
        {
            /*  Release level code -- if readback verify is turned on, we
                want it to work even when not building in debug mode.
            */
            DclPrintf("FfxMM:ReadBackVerify failed in ReadBlocks() lnu=%U Index=%U\n",
                lnu, nBlockIndex);

            DclProductionError();
            break;
        }

        if(DclMemCmp(buffer.data, pClientData, uBlockSize) != 0)
        {
            DclPrintf("FfxMM:ReadBackVerify failed -- data mismatch, lnu=%U Index=%U\n",
                lnu, nBlockIndex);

            DclProductionError();
            break;
        }

        nBlockCount--;
        nBlockIndex++;
        pClientData += uBlockSize;
    }

    if(!nBlockCount)
        fSuccess = TRUE;

  #if FFX_MEDIAMGR_READBACKVERIFY != (TRUE * 2)
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACEUNDENT),
        "FfxMM:ReadBackVerify() returning %U\n", fSuccess));
  #endif

    return fSuccess;
}


#endif

#endif  /* FFXCONF_VBFSUPPORT */


