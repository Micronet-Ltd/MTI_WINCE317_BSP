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

    This is the NAND Technology Module (NTM) which supports the LSI NAND
    Flash Controller as described by the Phoenix-2 Math Learning Platform
    Functional Design Specification.

    ToDo:  - Several places allocate an entire page on the stack.  This should
             never be done.
           - DclMemMove() appears to be used for no good reason -- should be
             using DclMemCopy() instead.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntlsi.c $
    Revision 1.38  2010/12/14 00:19:52Z  glenns
    Ensure that uEdcCapability is appropriately set if default FlashFX
    EDC mechanism is in use.
    Revision 1.37  2010/12/09 23:41:16Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.36  2010/07/06 18:20:18Z  garyp
    Removed some unused code.
    Revision 1.35  2010/07/06 03:00:32Z  garyp
    Eliminated some deprecated symbols.
    Revision 1.34  2009/12/11 21:08:48Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.
    Revision 1.33  2009/10/14 22:45:08Z  keithg
    Removed now obsolete fxio.h include file.
    Revision 1.32  2009/10/06 19:13:54Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.31  2009/07/24 23:13:05Z  garyp
    Merged from the v4.0 branch.  Modified so the PageWrite() functionality
    allows a page to be written with ECC, but no tags.  As before, if ECC is
    turned off, then tags may not be written.  Added a default IORequest()
    function.  Documentation updated.
    Revision 1.30  2009/04/09 03:42:56Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.29  2009/04/02 14:53:13Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.28  2009/03/25 01:38:09Z  glenns
    - Fix Bugzilla #2464: See documentation in bugzilla report for details.
    Revision 1.27  2009/03/05 16:57:49Z  thomd
    Add blockwise support for Large Flash Addressing
    Revision 1.26  2009/03/04 18:46:47Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    - Added code to properly report bit error corrections to the error
      manager.
    Revision 1.25  2009/02/17 07:55:22Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.24  2009/01/27 20:59:14Z  thomd
    Fail create if chip's EDC requirement > 1
    Revision 1.23  2009/01/26 23:49:29Z  glenns
    - Modified to accomodate variable names changed to meet
      Datalight coding standards.
    - Updated PageRead to accomodate FFXSTAT_FIMCORRECTABLEDATA
      status.
    Revision 1.22  2009/01/23 17:21:26Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.21  2009/01/16 23:57:47Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in nine places.
    Revision 1.20  2008/09/02 05:59:46Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.19  2008/08/20 00:17:25Z  keithg
    Updated to use the DCL memmove function.
    Revision 1.18  2008/06/16 16:55:18Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.17  2008/03/23 20:24:17Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to take a
    tag length parameter.  Modified the GetPageStatus() functionality to return
    the tag length, if any.
    Revision 1.16  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/09/21 04:35:46Z  pauli
    Resolved Bug 1455.  Added the ability to report that BBM should or should
    not be used based on the characteristics of the NAND part identified.
    Revision 1.14  2007/09/12 21:18:50Z  Garyp
    Updated to use some renamed functions.
    Revision 1.13  2007/08/02 22:45:35Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.12  2007/04/07 03:26:23Z  Garyp
    Removed some unnecessary asserts.  Documentation updated.
    Revision 1.11  2007/03/31 01:17:30Z  rickc
    Fix for bug 928.  Removed requirement that structure had to match on-media
    spare area format.
    Revision 1.10  2007/03/01 20:07:53Z  timothyj
    Changed references to local uPagesPerBlock to use the value now in the
    FimInfo structure.  Modified call to FfxDevApplyArrayBounds() to pass and
    receive on return a return block index in lieu of byte offset, for LFA
    support.  Changed references to the chipclass device size to use chip
    blocks (table format changed, see nandid.c).
    Revision 1.9  2007/02/13 23:17:54Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.8  2007/02/05 18:24:39Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.7  2007/01/03 23:35:26Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was located.
    Removed FFXNANDMFG (replaced references with references to the constant
    FFXNANDCHIP returned, above).
    Revision 1.6  2006/11/17 17:42:15Z  Garyp
    Corrected debug code to use the right printf() routine.
    Revision 1.5  2006/11/08 03:38:48Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.4  2006/08/16 21:45:23Z  johnb
    This is an update from code reviews.  Modified checks for command
    completions to add timeout.  Changed page write to be a power safe
    version but uses Software ECC algorithm.  Previous version is still
    contained but is conditionally compiled.
    Revision 1.3  2006/07/10 22:57:22Z  Garyp
    Minor debug code tweak.
    Revision 1.2  2006/06/30 00:05:21Z  Garyp
    Eliminated C++ style comments.
    Revision 1.1  2006/06/13 22:53:42Z  johnb
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandcmd.h>
#include <deverr.h>
#include <errmanager.h>
#include "nandid.h"
#include "nand.h"
#include "ntm.h"
#include <ecc.h>

#define NTMNAME "NTLSI"


/******************************************************************************
    The following declarations and definitions probably belong in a header
    defining the interface to NAND controller project hooks.
******************************************************************************/

/*  Private data structure for specific NAND controller.
*/
typedef struct nand_controller NAND_CONTROLLER;

/*  Argument to ChipSelect() to deselect all chips.
*/
#define CHIP_SELECT_NONE (~0U)

/* Using Datalight Software ECC Algorithm on Page writes.
   Set to 0 to use LSI Hardware ECC Algorithm
   Set to 1 to use Datalight Software ECC Algorithm

   Default to using Datalight Software ECC Algorithm
*/
#define USE_DL_SOFTWARE_ECC 1

static NAND_CONTROLLER *InitController(FFXDEVHANDLE hDev);
static void UninitController(NAND_CONTROLLER *pCtrlr);
static void ChipSelect(NAND_CONTROLLER *pCtrlr, unsigned int uChip);
static void ResetChip(NAND_CONTROLLER *pCtrlr);
static D_BOOL ReadID(NAND_CONTROLLER *pCtrlr, void *pBuffer, unsigned uLength);
static void ReadStatus(NAND_CONTROLLER *pCtrlr, D_UINT32 *pBuffer);
static D_BOOL ReadPage(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress, D_BUFFER *pBuffer, unsigned uLength, D_BUFFER *pTag, unsigned uUserSize);
static D_BOOL ReadPageNoData(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress);
static D_BOOL ReadSpare(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress, D_BUFFER *pTag, unsigned uUserSize);
static D_BOOL ReadSpareNoData(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress);
static D_BOOL ProgramPage(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress, const D_BUFFER *pBuffer, unsigned uLength);
static D_BOOL ProgramSpare(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress, const D_BUFFER *pTag, unsigned uUserSize);
static D_BOOL EraseBlock(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress);
#if 0
static void TagEncode(NAND_CONTROLLER *pCtrlr, const D_BUFFER *pTag, unsigned uUserSize, D_UINT32 *pSpare);
static ECC_RESULT TagDecode(NAND_CONTROLLER *pCtrlr, D_BUFFER *pBuffer, D_BUFFER *pTag, unsigned uUserSize, D_UINT32 ulAddress);
#endif
static D_BOOL ReadyWait(NAND_CONTROLLER *pCtrlr);
static DCLECCRESULT CheckECC(NAND_CONTROLLER *pCtrlr, void *pPage, void *pSpare);
static FFXIOSTATUS ReadSpareArea( NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS WriteSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);

/******************************************************************************
    End of "probably belong in a header."
******************************************************************************/


/*  Timeout in msec. for NAND flash operations.  The longest operation is
    erase, with typical spec of 2 msec.  It's okay if this is generous,
    timeout generally means something is badly wrong, and wasting a few
    msec. doesn't matter at that point.
*/
#define NAND_TIMEOUT (5)

/*  Private data structure for this NTM.
*/
struct tagNTMDATA
{
    const FFXNANDCHIP   *pChipInfo;      /* chip information                    */
    unsigned int    uUserSize;
    D_UINT16        uLinearPageAddrMSB;  /* how much to >> linear address to get chip page index  */
    D_UINT16        uChipPageIndexLSB;   /* how much to << chip page index before sending to chip */
    D_UINT16        uLinearBlockAddrLSB; /* how much to >> linear address to get block index */
    D_UINT16        uLinearChipAddrMSB;  /* how much to >> linear address to get chip select */
    NTMINFO         NtmInfo;            /* information visible to upper layers */
    FFXDEVHANDLE    hDev;
    NAND_CONTROLLER *pCtrlr;
    unsigned        uTagOffset;         /* Offset in spare area for the tag     */
    unsigned        uFlagsOffset;       /* Offset in spare area for the flags   */
    unsigned        uECC1Offset;        /* Offset in spare area for the ECC1    */
    unsigned        uECC2Offset;        /* Offset in spare area for the ECC2    */
};


/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_lsi =
{
    Create,
    Destroy,
    PageRead,
    PageWrite,
    HiddenRead,
    HiddenWrite,
    SpareRead,
    SpareWrite,
    BlockErase,
    GetPageStatus,
    GetBlockStatus,
    SetBlockStatus,
    ParameterGet,
    ParameterSet,
    RawPageRead,
    RawPageWrite,
    IORequest
};

static D_BOOL IdentifyFlash(NTMDATA *pNTM, unsigned uChip);
static unsigned int CheckStatus(NAND_CONTROLLER *pCtrlr);


/*  Number of chips supported by the controller
*/
#define NCHIPS (2)

/*  State information about the NAND controller.  Since the OMAP5912 has
    exactly one of these, the single structure may be allocated statically.
*/
static struct nand_controller
{
    void *pBase;
    D_UINT32 nnd_config;  /* copy of NND_CTRL, keeping it in RAM may be faster */
    D_BOOL fEccEnabled;
} ctrlr;

/* Controller registers and bit fields
*/
#define NND(offset)      ((D_UINT32 *) ((char *) ctrlr.pBase + offset))

#define NND_CONFIG      NND(0x00)
#define NFC_CONFIG_WIDTH_8BIT  0x00000000 /* 8-bit bus                          */
#define NFC_CONFIG_WIDTH_16BIT 0x00000001 /* 16-bit bus                         */
#define NFC_CONFIG_PAGE_256    0x00000000 /* Page size (256 byte)               */
#define NFC_CONFIG_PAGE_512    0x00000002 /* Page size (512 byte)               */
#define NFC_CONFIG_PAGE_1024   0x00000004 /* Page size (1024 byte)              */
#define NFC_CONFIG_PAGE_2048   0x00000006 /* Page size (2048 byte)              */
#define NFC_CONFIG_ECC_256     0x00000000 /* ECC block size (256 byte)          */
#define NFC_CONFIG_ECC_512     0x00000008 /* ECC block size (512 byte)          */
#define NFC_CONFIG_CS_0        0x00000000 /* NAND Flash Chip Select = 0         */
#define NFC_CONFIG_CS_1        0x00000010 /* NAND Flash Chip Select = 1         */

#define NND_WRITEPROT   NND(0x04)
#define NFC_WRITE_PROTECT      0x00000000 /* NAND Flash protected               */
#define NFC_WRITE_UNPROTECT    0x00000001 /* NAND Flash Unprotected             */

#define NND_CMDSTART    NND(0x08)
#define NFC_COMMAND_START      0x00000001 /* Command Start                      */

#define NND_COMMAND     NND(0x0C)
#define NFC_CMD_CMD1_MASK      0x000000FF /* Command 1 mask                     */
#define NFC_CMD_ADR_CYCLE_SKIP 0x00000000 /* No address cycle                   */
#define NFC_CMD_ADR_CYCLE_1    0x00000100 /* 1 address cycle                    */
#define NFC_CMD_ADR_CYCLE_2    0x00000200 /* 1 address cycle                    */
#define NFC_CMD_ADR_CYCLE_3    0x00000300 /* 1 address cycle                    */
#define NFC_CMD_ADR_CYCLE_4    0x00000400 /* 1 address cycle                    */
#define NFC_CMD_ADR_CYCLE_5    0x00000500 /* 1 address cycle                    */
#define NFC_CMD_WR_DATA        0x00000800 /* Write Date to NAND Flash           */
#define NFC_CMD_CMD2_MASK      0x000FF000 /* Command 2 mask                     */
#define NFC_CMD_CMD2_EN        0x00100000 /* Command 2 enable                   */
#define NFC_CMD_WAIT           0x00200000 /* Wait for Busy deassertion via Status Read Command */
#define NFC_CMD_RD_DATA        0x00400000 /* Read data from NAND Flash          */

#define NND_ADDRESS0    NND(0x10)
#define NFC_ADDR_0_MASK        0x000000FF /* Address register 0 mask            */
#define NND_ADDRESS1    NND(0x14)
#define NFC_ADDR_1_MASK        0x000000FF /* Address register 0 mask            */
#define NND_ADDRESS2    NND(0x18)
#define NFC_ADDR_2_MASK        0x000000FF /* Address register 0 mask            */
#define NND_ADDRESS3    NND(0x1C)
#define NFC_ADDR_3_MASK        0x000000FF /* Address register 0 mask            */
#define NND_ADDRESS4    NND(0x20)
#define NFC_ADDR_4_MASK        0x000000FF /* Address register 0 mask            */

#define NND_TX_SIZE     NND(0x24)
#define NFC_TX_SIZE_MASK       0x00000FFF /* Transfer size register mask        */

#define NND_SYSADDR     NND(0x28)
#define NFC_SYS_ADDR_MASK      0xFFFFFFFF /* System Address register mask       */

#define NND_SWAIT1      NND(0x2C)
#define NFC_SWAIT_1_MASK       0x0000FFFF /* Status Wait 1 register mask        */

#define NND_SWAIT2      NND(0x30)
#define NFC_SWAIT_2_MASK       0x0000FFFF /* Status Wait 2 register mask        */

#define NND_STATUS      NND(0x34)
#define NFC_STATUS_MASK        0x000000FF /* Status register mask               */

#define NND_INT_MASK    NND(0x38)
#define NFC_INT_MASK_MASK      0x00000001 /* Interrupt Mask register mask       */

#define NND_INT_STAT    NND(0x3C)
#define NFC_INT_STAT_MASK      0x00000001 /* Interrupt Status register mask     */

#define NND_INT_RAW     NND(0x40)
#define NFC_INT_RAW_MASK       0x00000001 /* Interrupt Status register mask     */

#define NND_ECC0        NND(0x44)
#define NFC_ECC_0_MASK         0x00FFFFFF /* Interrupt ECC 0 register mask      */

#define NND_ECC1        NND(0x48)
#define NFC_ECC_1_MASK         0x00FFFFFF /* Interrupt ECC 1 register mask      */

#define NND_ECC2        NND(0x4C)
#define NFC_ECC_2_MASK         0x00FFFFFF /* Interrupt ECC 2 register mask      */

#define NND_ECC3        NND(0x50)
#define NFC_ECC_3_MASK         0x00FFFFFF /* Interrupt ECC 3 register mask      */

#define NND_RST_STAT    NND(0x54)
#define NFC_RST_STAT_MASK      0x000000FF /* NAND Flash Controller State reset register mask */

#define NFC_CMD_RESET          0xFF
#define NFC_CMD_STATUS         0x70
#define NFC_CMD_READ_ID        0x90
#define NFC_CMD_READ_0         0x00
#define NFC_CMD_READ_1         0x01
#define NFC_CMD_READ_2         0x50
#define NFC_CMD_PROGRAM        0x80
#define NFC_CMD_PRG_CFM       (0x10<<12)
#define NFC_CMD_ERASE          0x60
#define NFC_CMD_ERASE_CFM     (0xD0<<12)






                    /*-----------------------------*\
                     *                             *
                     *     External Interface      *
                     *                             *
                    \*-----------------------------*/


/*-------------------------------------------------------------------
    Public: Create()

    This function creates an NTM instance.  Additionally, it
    configures the corresponding hardware by calling the helper
    functions and project hooks.

    Parameters:
        hDev      - An opaque pointer to a device on which this
                    module is to begin operating.  This module uses
                    this handle to acquire per-device configuration
                    and to specify this particular device to some
                    helper functions provided from other modules.
        ppNtmInfo - Output, on return points to the address of the
                    structure containing information shared with the
                    NAND module.

    Return Value:
        Returns an NTMHANDLE, the contents of which are private to
        this module, and unique to the device specified by hDev.
-------------------------------------------------------------------*/
static NTMHANDLE Create(
    FFXDEVHANDLE        hDev,
    const NTMINFO     **ppNtmInfo)
{
    NTMDATA            *pNTM = NULL;

    DclAssert(hDev);
    DclAssert(ppNtmInfo);

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if (pNTM)
    {
        pNTM->hDev = hDev;
        pNTM->pCtrlr = InitController(hDev);

        if (pNTM->pCtrlr)
        {

            if (IdentifyFlash(pNTM, 1))
            {
                D_UINT32        ulTotalBlocks;
                FFXFIMBOUNDS    bounds;

                /*  Get the array bounds now so we know how far to scan
                */
                FfxDevGetArrayBounds(hDev, &bounds);

                /*  The NtmInfo structure describes the characteristics of the
                    flash.
                */
                pNTM->NtmInfo.ulBlockSize   = pNTM->pChipInfo->pChipClass->ulBlockSize;
                pNTM->NtmInfo.uPageSize     = pNTM->pChipInfo->pChipClass->uPageSize;
                pNTM->NtmInfo.uSpareSize    = pNTM->pChipInfo->pChipClass->uSpareSize;
                pNTM->NtmInfo.uMetaSize     = FFX_NAND_TAGSIZE;
                pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_ECC;
                if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
                    pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
                if (pNTM->pChipInfo->pChipClass->fProgramOnce)
                    pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

                if( pNTM->pChipInfo->pChipClass->uEdcRequirement > 1 )
                {
                    /* This chip requires more correction capabilities
                       than this NTM can handle - fail the create.
                    */
                    DclPrintf("FFX: Insufficient EDC capabilities.\n");
                    goto CreateCleanup;
                }

                pNTM->NtmInfo.uEdcRequirement     = pNTM->pChipInfo->pChipClass->uEdcRequirement;
                
                /*  Uses DL software EDC for now. May have to adjust this
                    if the hardware EDC code is fixed/enabled/used.
                */
                pNTM->NtmInfo.uEdcCapability      = 1;
                pNTM->NtmInfo.uEdcSegmentSize     = DATA_BYTES_PER_ECC;
                
                pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
                pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

                /*  Calculate this once and store it for use later for
                    both optimization and simplification of the code
                 */
                DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
                DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
                pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);

                /*  Determine which spare area format to use.
                */
                switch(FfxNtmHelpGetSpareAreaFormat(pNTM->pChipInfo))
                {
                    case NSF_OFFSETZERO:
                    {
                        DclAssert(NSOFFSETZERO_TAG_LENGTH == LEGACY_ENCODED_TAG_SIZE);

                        pNTM->uTagOffset    = NSOFFSETZERO_TAG_OFFSET;
                        pNTM->uFlagsOffset  = NSOFFSETZERO_FLAGS_OFFSET;
                        pNTM->uECC1Offset   = NSOFFSETZERO_ECC1_OFFSET;
                        pNTM->uECC2Offset   = NSOFFSETZERO_ECC2_OFFSET;
                        break;
                    }

                    case NSF_SSFDC:
                    {
                        pNTM->uTagOffset    = NSSSFDC_TAG_OFFSET;
                        pNTM->uFlagsOffset  = NSSSFDC_FLAGS_OFFSET;
                        pNTM->uECC1Offset   = NSSSFDC_ECC1_OFFSET;
                        pNTM->uECC2Offset   = NSSSFDC_ECC2_OFFSET;
                        break;
                    }

                    default:
                    {
                        DclPrintf("FFX: The detected part requires an unsupported spare area format.\n");
                        goto CreateCleanup;
                    }
                }

                pNTM->uLinearPageAddrMSB  = pNTM->pChipInfo->pChipClass->uLinearPageAddrMSB;
                pNTM->uChipPageIndexLSB   = pNTM->pChipInfo->pChipClass->uChipPageIndexLSB;
                pNTM->uLinearBlockAddrLSB = pNTM->pChipInfo->pChipClass->uLinearBlockAddrLSB;
                pNTM->uLinearChipAddrMSB  = pNTM->pChipInfo->pChipClass->uLinearChipAddrMSB;

                ulTotalBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;

                    pNTM->NtmInfo.ulTotalBlocks = ulTotalBlocks;

                    /*  Make sure the parent has a pointer to our NTMINFO
                        structure.
                    */
                    *ppNtmInfo = &pNTM->NtmInfo;

                    /* Successfully created the NTM instance.
                    */
                    return pNTM;
                }

  CreateCleanup:

            UninitController(pNTM->pCtrlr);
        }
        DclMemFree(pNTM);
    }
    return NULL;
}


/*-------------------------------------------------------------------
    Local: IdentifyFlash()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL IdentifyFlash(
    NTMDATA        *pNTM,
    unsigned        uChip)
{
    D_UCHAR abID[4];    /*  Manufacturer and device IDs, plus chip-
                            specific data
                         */

    /*  Select the specified chip.
    */
    ChipSelect(pNTM->pCtrlr, uChip);

    /* Reset the chip and wait for it to be ready.
    */
    ResetChip(pNTM->pCtrlr);
    if (!ReadyWait(pNTM->pCtrlr))
        return FALSE;

    /*  Read the manufacturer and device IDs from the flash.
    */
    ReadID(pNTM->pCtrlr, abID, sizeof abID);


    /*  Look up the device.
    */
    pNTM->pChipInfo = FfxNandDecodeID(abID);
    if (!pNTM->pChipInfo)
        return FALSE;

    /*  Make sure it's a small-block part, that's all that's
        supported right now.
    */
    if (pNTM->pChipInfo->pChipClass->uPageSize != 512)
        return FALSE;

    /*  There needs to be a way to find out whether a part has
        8-bit or 16-bit interface.  Some controllers need to
        know this.
    */

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: Destroy()

    This function destroys an NTM instance, and releases any
    allocated resources.

    Parameters:
        hNTM - The handle for the NTM instance to destroy.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void Destroy(
    NTMHANDLE           hNTM)
{
    DclAssert(hNTM);

    DclMemFree(hNTM);
}


/*-------------------------------------------------------------------
    Public: PageRead()

    Read one or more pages of main page data, and optionally read
    the corresponding tag data from the spare area.

    The fUseEcc flag controls whether ECC-based error detection
    and correction are performed.  Note that the "raw" page read
    functionality (if implemented) is the preferred interface for
    reading without ECC protection.  If fUseEcc is FALSE, this
    function will not read tag data.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The starting page to read, relative to any
                   reserved space.
        pPages   - A buffer to receive the main page data
        pTags    - A buffer to receive the tag data.  Must be NULL
                   fUseEcc is FALSE.
        ulCount  - Number of pages to read.  The range of pages
                   must not cross an erase block boundary.
        nTagSize - The tag size to use.  Must be 0 if fUseEcc is
                   FALSE.
        fUseEcc  - If TRUE, perform error detection and correction.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pPages array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulAddress; /*  The address within one chip, with the chip number removed.*/
    D_UINT32            ulAddressableUnitAddr;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTLSI-PageRead() Page=%lU Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTLSI-PageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));

    /*  If fUseEcc is FALSE, then the tag size (and the tag pointer, asserted
        above) must be clear.
    */
    DclAssert(fUseEcc || !nTagSize);

    /*  Compute the offset
     */
    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Split the adjusted offset into a chip number and the address
        within the chip, and select the chip.  The chip number is in
        the high bits of the offset, beyond the bits corresponding to
        the size of a single chip.

        NOTE: this encoding makes it impossible to mix chip capacities
        in a single array.
    */
    DclAssert((ulAddressableUnitAddr >> (hNTM->pChipInfo->pChipClass->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) < CHIP_SELECT_NONE);
    ulAddress = ulAddressableUnitAddr;

    for (ioStat.ulCount = 0; ioStat.ulCount < ulCount; ++ioStat.ulCount)
    {
        /*  Start a Page Read operation.

            NOTE: it should be possible to use the Sequential Read feature of some
            NAND chips to avoid issuing a new Page Read every time through this
            loop.  This would require being aware of chip type here.
        */
        if (!ReadPage(hNTM->pCtrlr, ulAddress, pPages, hNTM->pChipInfo->pChipClass->uPageSize, pTags, nTagSize))
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;    /* chip should always be ready at this point */
        }

        if (!ReadyWait(hNTM->pCtrlr))
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        ReadSpare(hNTM->pCtrlr, ulAddress, spare.data, NAND512_SPARE);

        if(pTags)
            FfxNtmHelpTagDecode(pTags, &spare.data[hNTM->uTagOffset]);

        if(pPages)
        {
            /*  Check the tag data (if it was desired) and check the ECC.
            */
            switch ( CheckECC(hNTM->pCtrlr, pPages, spare.data) )
            {
                case DCLECC_VALID:
                    break;

                case DCLECC_ECCCORRECTED:
                case DCLECC_DATACORRECTED:
                    FfxErrMgrEncodeCorrectedBits(1, &ioStat);
                    ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                    ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
                    ioStat.ulCount++;
                    FFXPRINTF(1, ("Correctable ECC error\n"));
                    break;

                case DCLECC_UNCORRECTABLE:
                    FFXPRINTF(1, ("Uncorrectable ECC error\n"));
                    ioStat.ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                    break;
                default:
                    DclError();
                    ioStat.ffxStat = FFXSTAT_INTERNAL_ERROR;
                    break;
            }
        }

        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was read successfully.  Account for it.
        */
        pPages    += hNTM->pChipInfo->pChipClass->uPageSize;
        ulAddress += (hNTM->pChipInfo->pChipClass->uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
        if(nTagSize)
        {
            DclAssert(pTags);
            DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

            pTags += nTagSize;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTLSI-PageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: PageWrite()

    Write one or more logical pages of main array data, along with
    optional tag data.

    If fUseEcc is TRUE, an ECC for the main page data is generated
    and written in the spare area, along with the optional tag.  If
    fUseEcc is FALSE, no ECC and no tag is written.  The spare area
    includes a record of the state of fUseEcc.  Note that the "raw"
    page write functionality (if implemented) is the preferred
    interface for writing data without ECC.

    Parameters:
        hNTM        - The NTM handle to use.
        ulStartPage - The starting page to write, relative to any
                      reserved space.
        pPages      - A pointer to the main page data to write.
        pTags       - A buffer containing the optional tag data,
                      so long as nTagSize is valid, and fUseEcc is
                      TRUE.  Must be NULL otherwise.
        ulCount     - Number of pages to write.  The range of pages
                      must not cross an erase block boundary.
        nTagSize    - The tag size to use.  Must be 0 if pTags is
                      NULL or fUseEcc is FALSE.
        fUseEcc     - If TRUE, write the data with ECC.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the operation
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulAddress; /*  The address within one chip, with the chip number removed.*/
    D_UINT32            ulAddressableUnitAddr;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTLSI-PageWrite() Page=%lU Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTLSI-PageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));

    ulAddressableUnitAddr = ulStartPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Split the adjusted offset into a chip number and the address
        within the chip, and select the chip.  The chip number is in
        the high bits of the offset, beyond the bits corresponding to
        the size of a single chip.

        NOTE: this encoding makes it impossible to mix chip capacities
        in a single array.
    */
    DclAssert((ulAddressableUnitAddr >> (hNTM->pChipInfo->pChipClass->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) < CHIP_SELECT_NONE);

    ulAddress = ulAddressableUnitAddr;


    for (ioStat.ulCount = 0; ioStat.ulCount < ulCount; ++ioStat.ulCount)
    {
        unsigned int        uStatus;
      #if USE_DL_SOFTWARE_ECC
        DCLALIGNEDBUFFER    (BounceBuffer, data, FFX_NAND_MAXSPARESIZE+FFX_NAND_MAXPAGESIZE);
        D_BUFFER           *pPage  = BounceBuffer.data;
        D_BUFFER            abECC[MAX_ECC_BYTES_PER_PAGE];
        D_UINT16            x;
        D_UINT16            uBufferLength;

        /* Use Software ECC Algorithm.  This is the power safe
           method, but is prone to performance problems.
        */

        /*  Copy page data into memory buffer
        */
        DclMemCpy(pPage, pPages, hNTM->pChipInfo->pChipClass->uPageSize);

        uBufferLength = hNTM->pChipInfo->pChipClass->uPageSize;

        if(fUseEcc)
        {
            DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
            D_BUFFER           *pSpare = spare.data;
            D_BUFFER           *pSpareTemp = pSpare;
            unsigned            nn = 0;

            uBufferLength += hNTM->pChipInfo->pChipClass->uSpareSize;

            /*  encode tags to spare area
            */
            DclMemSet(pSpare, ERASED8, hNTM->NtmInfo.uSpareSize);

            if(pTags)
            {
                DclAssert(fUseEcc);
                DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

                FfxNtmHelpTagEncode(&pSpare[hNTM->uTagOffset], pTags);
            }

            /*  generate ECC's and put in spare area
            */
            FfxHookEccCalcStart(NULL, pPages, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_WRITE);

            FfxHookEccCalcRead(NULL, pPages, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_WRITE);

            for(x=0; x<hNTM->NtmInfo.uPageSize/NAND512_PAGE; x++)
            {
                /*  Build the ECC values!
                */
                pSpareTemp[hNTM->uFlagsOffset] = LEGACY_WRITTEN_WITH_ECC;
                pSpareTemp[hNTM->uECC1Offset + 0] = abECC[nn++];
                pSpareTemp[hNTM->uECC1Offset + 1] = abECC[nn++];
                pSpareTemp[hNTM->uECC1Offset + 2] = abECC[nn++];

                pSpareTemp[hNTM->uECC2Offset + 0] = abECC[nn++];
                pSpareTemp[hNTM->uECC2Offset + 1] = abECC[nn++];
                pSpareTemp[hNTM->uECC2Offset + 2] = abECC[nn++];

                pSpareTemp += NAND512_SPARE;
            }

            DclMemCpy(&pPage[hNTM->NtmInfo.uPageSize], pSpare, hNTM->NtmInfo.uSpareSize);
        }

        /*  Start a Page Write operation.

            NOTE: it should be possible to use the multiplane write
            feature of some NAND chips to enable writing several pages
            concurrently.  This would require being aware of chip type
            here.
        */
        if (!ProgramPage(hNTM->pCtrlr, ulAddress, pPage, uBufferLength))
        {
            FFXPRINTF(1, ("PageWrite: not ready\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;    /* chip should always be ready at this point */
        }


        if (!ReadyWait(hNTM->pCtrlr))
        {
            FFXPRINTF(1, ("PageWrite: timed out\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        /*  Check the status of the operation.
        */
        uStatus = CheckStatus(hNTM->pCtrlr);
        if (uStatus != 0)
        {
            FFXPRINTF(1, ("PageWrite: failure status 0x%02x\n", uStatus));
            ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            break;
        }

      #else
        /* Use Hardware ECC Algorithm, but this is currently
           not power safe
        */

        /*  This code is broken.  In addition to not being power-safe,
            the code assume that ECCs will always be used.  This is not
            always the case.
        */
        DclProductionError();

        /*  Start a Page Write operation.

            NOTE: it should be possible to use the multiplane write
            feature of some NAND chips to enable writing several pages
            concurrently.  This would require being aware of chip type
            here.
        */
        if (!ProgramPage(hNTM->pCtrlr, ulAddress, pPages, hNTM->pChipInfo->pChipClass->uPageSize))
        {
            FFXPRINTF(1, ("PageWrite: not ready\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;    /* chip should always be ready at this point */
        }


        if (!ReadyWait(hNTM->pCtrlr))
        {
            FFXPRINTF(1, ("PageWrite: timed out\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        /*  Check the status of the operation.
        */
        uStatus = CheckStatus(hNTM->pCtrlr);
        if (uStatus != 0)
        {
            FFXPRINTF(1, ("PageWrite: failure status 0x%02x\n", uStatus));
            ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            break;
        }

        /*
            program the spare area
        */
        if(fUseECC)
        {
            DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
            D_BUFFER           *pNS = spare.data;

            DclMemSet(spare.data, ERASED8, hNTM->NtmInfo.uSpareSize);

            if(pTags)
            {
                DclAssert(fUseEcc);
                DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

                FfxNtmHelpTagEncode(&pNS[hNTM->uTagOffset], pTags);
            }

/*            if(fUseEcc) */
            {
                D_UINT32 ulEcc1, ulEcc2;

                /*  Build the ECC values!
                */
                pNS[hNTM->uFlagsOffset] = LEGACY_WRITTEN_WITH_ECC;
                ulEcc1 = DCLMEMGET32(NND_ECC0);
                ulEcc2 = DCLMEMGET32(NND_ECC1);

                pNS[hNTM->uECC1Offset + 0] = ulEcc1[(x*2)+0].bECC0;
                pNS[hNTM->uECC1Offset + 1] = ulEcc1[(x*2)+0].bECC1;
                pNS[hNTM->uECC1Offset + 2] = ulEcc1[(x*2)+0].bECC2;

                pNS[hNTM->uECC2Offset + 0] = ulEcc2[(x*2)+1].bECC0;
                pNS[hNTM->uECC2Offset + 1] = ulEcc2[(x*2)+1].bECC1;
                pNS[hNTM->uECC2Offset + 2] = ulEcc2[(x*2)+1].bECC2;
            }

            if (!ProgramSpare(hNTM->pCtrlr, ulAddress, pNS, hNTM->NtmInfo.uSpareSize))
            {
                FFXPRINTF(1, ("PageWrite: not ready\n"));
                ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
                break;    /* chip should always be ready at this point */
            }


            if (!ReadyWait(hNTM->pCtrlr))
            {
                FFXPRINTF(1, ("PageWrite: timed out\n"));
                ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
                break;
            }

            /*  Check the status of the operation.
            */
            uStatus = CheckStatus(hNTM->pCtrlr);
            if (uStatus != 0)
            {
                FFXPRINTF(1, ("PageWrite: failure status 0x%02x\n", uStatus));
                ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                break;
            }
        }
      #endif

        /*  The page was programmed successfully.  Account for it.
        */
        pPages    += hNTM->pChipInfo->pChipClass->uPageSize;
        ulAddress += (hNTM->pChipInfo->pChipClass->uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
        if(nTagSize)
            pTags += nTagSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTLSI-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: RawPageRead()

    Read an exact image of the data on the media with no
    processing of ECCs or interpretation of the contents of
    the spare area.

    Reading the spare area is optional.

    Not all NTMs support this function; not all NAND controllers
    can support it.

    Parameters:
        hNTM    - The NTM handle to use
        ulPage  - The flash offset in pages, relative to
                  any reserved space.
        pPages  - A buffer to receive the main page data.
        pSpares - A buffer to receive the spare area data.
                  May be NULL.
        ulCount - The number of pages to read.  The range of pages
                  must not cross an erase block boundary.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS RawPageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pSpares,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIM_UNSUPPORTEDFUNCTION);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTOMAP-RawPageRead() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTOMAP-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    (void)pPages;
    (void)pSpares;
    (void)ulCount;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTOMAP-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: RawPageWrite()

    Write data to the entire page (main and spare area) with no
    ECC processing or other interpretation or formatting.

    Not all NTMs support this function; not all NAND controllers
    can support it.

    Parameters:
        hNTM    - The NTM handle to use
        ulPage  - The flash offset in pages, relative to
                  any reserved space.
        pPages  - A pointer to the main page data to write.
        pSpares - A pointer to data to write in the spare area.
        ulCount - The number of pages to write.  The range of
                  pages must not cross an erase block boundary.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS RawPageWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pSpares,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIM_UNSUPPORTEDFUNCTION);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTOMAP-RawPageWrite() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTOMAP-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(pSpares);
    DclAssert(ulCount);

    (void)pPages;
    (void)pSpares;
    (void)ulCount;

/*  RawPageWriteCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTOMAP-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SpareRead()

    Read the spare area of one page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.
        pSpare - A pointer to the buffer to fill.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of spares which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pSpareBuf)
{
    FFXIOSTATUS         ioStat;

    ioStat = ReadSpareArea(hNTM, ulPage, pSpareBuf);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SpareWrite()

    Write the spare area of one page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.
        pSpare - A pointer to the spare area data to write.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of spares which were completely written.
        The status indicates whether the read was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat;

    ioStat = WriteSpareArea(hNTM, ulPage, pSpare);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenRead()

    Read the hidden (tag) data from multiple pages.

    Parameters:
        hNTM        - The NTM handle to use
        ulStartPage - The flash offset in pages, relative to any
                      reserved space.
        ulCount     - The number of tags to read.
        pTags       - Buffer for the hidden data read.
        nTagSize    - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_UINT32            ulCount,
    D_BUFFER           *pTags,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat = { 0, FFXSTAT_SUCCESS };
    D_UINT32            ulAddress; /*  The address within one chip, with the chip number removed.*/
    D_UINT32            ulAddressableUnitAddr;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTLSI-HiddenRead() Page=%lU Count=%lU TagSize=%u\n", ulStartPage, ulCount, nTagSize));

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    ulAddressableUnitAddr = ulStartPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Split the adjusted offset into a chip number and the address
        within the chip, and select the chip.  The chip number is in
        the high bits of the offset, beyond the bits corresponding to
        the size of a single chip.

        NOTE: this encoding makes it impossible to mix chip capacities
        in a single array.
    */
    DclAssert((ulAddressableUnitAddr >> (hNTM->pChipInfo->pChipClass->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) < CHIP_SELECT_NONE);
    ulAddress = ulAddressableUnitAddr;


    for (ioStat.ulCount = 0; ioStat.ulCount < ulCount; ++ioStat.ulCount)
    {
        DCLALIGNEDBUFFER    (spare, data, NAND512_SPARE);
        D_BUFFER            *pSpare = spare.data;

        if (!ReadSpare(hNTM->pCtrlr, ulAddress, pSpare, hNTM->NtmInfo.uSpareSize))
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        if (!ReadyWait(hNTM->pCtrlr))
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        /* TagIn() decodes the tag and checks ECC if it's enabled.  It's not
           here, so ignore the ECC status it returns.
        */
        FfxNtmHelpTagDecode(pTags, &pSpare[hNTM->uTagOffset]);

        /*  The page was read successfully.  Account for it.
        */
        pTags     += nTagSize;
        ulAddress += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTLSI-HiddenRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenWrite()

    Write the hidden (tag) data for one page.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The page to write, relative to any reserved
                   space.
        pTag     - Buffer for the hidden data to write.
        nTagSize - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely written.
        The status indicates whether the read was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */
    D_UINT32            ulAddress; /*  The address within one chip, with the chip number removed.*/
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
    D_UINT32            ulAddressableUnitAddr;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTLSI-HiddenWrite() Page=%lU Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Compute the offset
     */
    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Split the adjusted offset into a chip number and the address
        within the chip, and select the chip.  The chip number is in
        the high bits of the offset, beyond the bits corresponding to
        the size of a single chip.

        NOTE: this encoding makes it impossible to mix chip capacities
        in a single array.
    */
    DclAssert((ulAddressableUnitAddr >> (hNTM->pChipInfo->pChipClass->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) < CHIP_SELECT_NONE);
    ulAddress = ulAddressableUnitAddr;

    for (ioStat.ulCount = 0; ioStat.ulCount < uCount; ++ioStat.ulCount)
    {
        unsigned int uStatus;

        DclMemSet(spare.data, ERASED8, sizeof spare.data);
        FfxNtmHelpTagEncode(&spare.data[hNTM->uTagOffset], pTag);

/*        TagEncode(hNTM->pCtrlr, pTag, nTagSize, (D_UINT32 *)spare.data); */

        if (!ProgramSpare(hNTM->pCtrlr, ulAddress, spare.data, hNTM->NtmInfo.uSpareSize))
        {
            FFXPRINTF(1, ("HiddenWrite: not ready\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;    /* chip should always be ready at this point */
        }


        if (!ReadyWait(hNTM->pCtrlr))
        {
            FFXPRINTF(1, ("HiddenWrite: timed out\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        /*  Check the status of the operation.
        */
        uStatus = CheckStatus(hNTM->pCtrlr);
        if (uStatus != 0)
        {
            FFXPRINTF(1, ("SpareWrite: failure status 0x%02x\n", uStatus));
            ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            break;
        }

        /*  The page was programmed successfully.  Account for it.
        */
        pTag += nTagSize;
        ulAddress += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTLSI-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: BlockErase()

    This function erases a block (physical erase zone) of a
    NAND chip.

    Parameters:
        hNTM    - The NTM handle
        ulBlock - The flash offset in blocks, relative to any
                  reserved space.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of blocks which were completely erased.
        The status indicates whether the erase was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS BlockErase(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_BLOCKIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */
    D_UINT32 ulAddress; /*  The address within one chip, with the chip number removed.*/
    D_UINT32            ulAddressableUnitAddr;

    ulAddressableUnitAddr = ulBlock * (hNTM->NtmInfo.ulBlockSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Split the adjusted offset into a chip number and the address
        within the chip, and select the chip.  The chip number is in
        the high bits of the offset, beyond the bits corresponding to
        the size of a single chip.

        NOTE: this encoding makes it impossible to mix chip capacities
        in a single array.
    */
    DclAssert((ulAddressableUnitAddr >> (hNTM->pChipInfo->pChipClass->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) < CHIP_SELECT_NONE);

    ulAddress = ulAddressableUnitAddr;

    for (ioStat.ulCount = 0; ioStat.ulCount < uCount; ++ioStat.ulCount)
    {
        unsigned int uStatus;

        if (!EraseBlock(hNTM->pCtrlr, ulAddress))
        {
            FFXPRINTF(1, ("BlockErase: not ready\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;    /* chip should always be ready at this point */
        }

        if (!ReadyWait(hNTM->pCtrlr))
        {
            FFXPRINTF(1, ("BlockErase: timed out\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        /*  Check the status of the operation.
        */
        uStatus = CheckStatus(hNTM->pCtrlr);
        if (uStatus != 0)
        {
            FFXPRINTF(1, ("BlockErase: failure status 0x%02x\n", uStatus));
            ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            break;
        }

        /* The Block was erased successfully.  Account for it.
        */
        ulAddress += (hNTM->pChipInfo->pChipClass->ulBlockSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetPageStatus()

    This function retrieves the page status information for the
    given page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The page to query, relative to any reserved
                 space.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulPageStatus variable will contain the page status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetPageStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulPage)
{
    FFXIOSTATUS     ioStat;

    DclAssert(hNTM);

    ioStat = FfxNtmHelpReadPageStatus(hNTM, &ReadSpareArea, ulPage, hNTM->pChipInfo);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTLSI-GetPageStatus() Page=%lX returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetBlockStatus()

    This function retrieves the block status information for the
    given erase block.

    This is an interesting problem to factor.  Only the
    controller-specific code knows for sure where in the spare
    area the block status words are (because the controller may
    impose a format).  But each chip may have its own preferred
    location at least for the factory bad block mark.

    Parameters:
        hNTM    - The NTM handle to use
        ulBlock - The flash offset, in blocks, relative to any
                  reserved space.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulBlockStatus variable will contain the block status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetBlockStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock)
{
    FFXIOSTATUS         ioStat;

    DclAssert(hNTM);

    /* TBD: Grep this module for 'NTOMAP' - found a couple while updating
     * for LFA compliance.
     */

    ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadSpareArea, hNTM->pChipInfo, ulBlock);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTLSI-GetBlockStatus() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SetBlockStatus()

    This function sets the block status information for the
    given erase block.

    Parameters:
        hNTM          - The NTM handle to use
        ulBlock       - The flash offset, in blocks, relative to any
                        reserved space.
        ulBlockStatus - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard I/O status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS SetBlockStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock,
    D_UINT32            ulBlockStatus)
{
    FFXIOSTATUS         ioStat = DEFAULT_BLOCKIO_STATUS;

    DclAssert(hNTM);

    /*  For now, the only thing this function knows how to do is mark
        a block bad -- fail if anything else is requested.
    */
    if(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadSpareArea, &WriteSpareArea,
            hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTLSI-SetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
        ulBlock, ulBlockStatus, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: ParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hNTM      - The NTM handle.
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
static FFXSTATUS ParameterGet(
    NTMHANDLE           hNTM,
    FFXPARAM            id,
    void               *pBuffer,
    D_UINT32            ulBuffLen)
{
    FFXSTATUS           ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NTLSI:ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTOMAP:ParameterGet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NTLSI:ParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: ParameterSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hNTM      - The NTM handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterSet(
    NTMHANDLE           hNTM,
    FFXPARAM            id,
    const void         *pBuffer,
    D_UINT32            ulBuffLen)
{
    FFXSTATUS           ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NTLSI:ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTOMAP:ParameterSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NTLSI:ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: IORequest()

    This function implements a generic IORequest handler at the NTM
    level.

    Parameters:
        hNTM      - The NTM handle.
        pIOR      - A pointer to the FFXIOREQUEST structure to use.

    Returns:
        Returns a standard FFXIOSTATUS structure.  The status code
        FFXSTAT_FIM_UNSUPPORTEDIOREQUEST is specifically returned
        for those requests which are not supported.
-------------------------------------------------------------------*/
static FFXIOSTATUS IORequest(
    NTMHANDLE           hNTM,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat = NULL_IO_STATUS;

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-IORequest() hNTM=%P Func=%x\n", hNTM, pIOR->ioFunc));

    DclProfilerEnter(NTMNAME"-IORequest", 0, 0);

    DclAssert(hNTM);

    switch(pIOR->ioFunc)
    {
        case FXIOFUNC_FIM_LOCK_FREEZE:
        {
            FFXIOR_FIM_LOCK_FREEZE *pReq = (FFXIOR_FIM_LOCK_FREEZE*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_LOCK_FREEZE() (STUBBED!)\n"));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "lock-freeze" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_BLOCK;

            break;
        }

        case FXIOFUNC_FIM_LOCK_BLOCKS:
        {
            FFXIOR_FIM_LOCK_BLOCKS *pReq = (FFXIOR_FIM_LOCK_BLOCKS*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_LOCK_BLOCKS() StartBlock=%lU Count-%lU (STUBBED!)\n",
                pReq->ulStartBlock, pReq->ulBlockCount));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "lock-block" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_BLOCK;

            break;
        }

        case FXIOFUNC_FIM_UNLOCK_BLOCKS:
        {
            FFXIOR_FIM_UNLOCK_BLOCKS *pReq = (FFXIOR_FIM_UNLOCK_BLOCKS*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_UNLOCK_BLOCKS() StartBlock=%lU Count-%lU (STUBBED!)\n",
                pReq->ulStartBlock, pReq->ulBlockCount));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "unlock-block" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_BLOCK;

            break;
        }

      #if FFXCONF_POWERSUSPENDRESUME
        case FXIOFUNC_FIM_POWER_SUSPEND:
        {
            FFXIOR_FIM_POWER_SUSPEND   *pReq = (FFXIOR_FIM_POWER_SUSPEND*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_SUSPEND() PowerState=%u (STUBBED!)\n", pReq->nPowerState));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "power-suspend" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_DEVICE;

            break;
        }

        case FXIOFUNC_FIM_POWER_RESUME:
        {
            FFXIOR_FIM_POWER_RESUME    *pReq = (FFXIOR_FIM_POWER_RESUME*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_RESUME() PowerState=%u (STUBBED!)\n", pReq->nPowerState));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "power-resume" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_DEVICE;

            break;
        }
      #endif

        default:
        {
            FFXPRINTF(3, (NTMNAME"-IORequest() Unsupported function %x\n", pIOR->ioFunc));

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;

            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-IORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}



                    /*-----------------------------*\
                     *                             *
                     *      Helper Functions       *
                     *                             *
                    \*-----------------------------*/


/*-------------------------------------------------------------------
    Local: ReadSpareArea()

    This function reads the spare area for the given flash offset
    into the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:
        hNTM   - A pointer to the NTMINFO structure.
        ulPage - The page address within the array (including
                 chip selects potentially encoded).
        pSpare - Buffer for the spare area.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */
    D_UINT32 ulAddress; /*  The address within one chip, with the chip number removed.*/
    D_UINT32            ulAddressableUnitAddr;

    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Split the adjusted offset into a chip number and the address
        within the chip, and select the chip.  The chip number is in
        the high bits of the offset, beyond the bits corresponding to
        the size of a single chip.

        NOTE: this encoding makes it impossible to mix chip capacities
        in a single array.
    */
    DclAssert((ulAddressableUnitAddr >> (hNTM->pChipInfo->pChipClass->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) < CHIP_SELECT_NONE);

    ulAddress = ulAddressableUnitAddr;

    for (ioStat.ulCount = 0; ioStat.ulCount < uCount; ++ioStat.ulCount)
    {
        if (!ReadSpare(hNTM->pCtrlr, ulAddress, pSpare, hNTM->pChipInfo->pChipClass->uSpareSize))
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        if (!ReadyWait(hNTM->pCtrlr))
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        /*  The page was read successfully.  Account for it.
        */
        pSpare += hNTM->pChipInfo->pChipClass->uSpareSize;
        ulAddress += (hNTM->pChipInfo->pChipClass->uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: WriteSpareArea()

    This function writes the spare area for the given flash
    offset from the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */
    D_UINT32 ulAddress; /*  The address within one chip, with the chip number removed.*/
    D_UINT32            ulAddressableUnitAddr;

    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Split the adjusted offset into a chip number and the address
        within the chip, and select the chip.  The chip number is in
        the high bits of the offset, beyond the bits corresponding to
        the size of a single chip.

        NOTE: this encoding makes it impossible to mix chip capacities
        in a single array.
    */
    DclAssert((ulAddressableUnitAddr >> (hNTM->pChipInfo->pChipClass->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) < CHIP_SELECT_NONE);

    ulAddress = ulAddressableUnitAddr;

    for (ioStat.ulCount = 0; ioStat.ulCount < uCount; ++ioStat.ulCount)
    {
        unsigned int uStatus;

        if (!ProgramSpare(hNTM->pCtrlr, ulAddress, pSpare, hNTM->pChipInfo->pChipClass->uSpareSize))
        {
            FFXPRINTF(1, ("WriteSpareArea: not ready\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;    /* chip should always be ready at this point */
        }

        if (!ReadyWait(hNTM->pCtrlr))
        {
            FFXPRINTF(1, ("WriteSpareArea: timed out\n"));
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            break;
        }

        /*  Check the status of the operation.
        */
        uStatus = CheckStatus(hNTM->pCtrlr);
        if (uStatus != 0)
        {
            FFXPRINTF(1, ("WriteSpareArea: failure status 0x%02x\n", uStatus));
            ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            break;
        }

        /*  The page was programmed successfully.  Account for it.
        */
        pSpare = (D_BUFFER *) pSpare + hNTM->pChipInfo->pChipClass->uSpareSize;
        ulAddress += (hNTM->pChipInfo->pChipClass->uSpareSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    }

    return ioStat;
}


/*  These bits are defined as follows:
    taken from the datasheet:

*/
#define STATUS_BITS 0xC1

/*-------------------------------------------------------------------
    Local: CheckStatus()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static unsigned int CheckStatus(NAND_CONTROLLER *pCtrlr)
{
        D_UINT32 ulStatus;

    ReadStatus(pCtrlr, &ulStatus);

    /*  Process the status: trim it down to one byte, and complement
        the "Ready" bit (which ought to be set).  Any remaining bits
        left indicate some sort of problem.
    */
    return ((D_UCHAR) (ulStatus & STATUS_BITS) ^ NANDSTAT_READY);
}



/******************************************************************************
*******************************************************************************
    Physical NAND Operations functions.

    These functions correspond fairly directly to the actual operations
    the NAND flash chip implements.

    Different controllers may have different orders in which parts of
    the command for the operation must be sent to the controller.
    These parts are handled here as arguments to a single function
    representing the command.

    Some machine dependencies (like number of bits in an int) are
    allowed, as this is machine-dependent code.

*******************************************************************************
******************************************************************************/




/*-------------------------------------------------------------------
    Local: InitController()

    Find out the address of the NAND controller, and set it
    up at least enough to read the device ID.

    Parameters:
        hDrive - The drive handle representing the flash
                 to use.

    Return Value:
        A pointer to a NAND_CONTROLLER structure to be passed to
        all the other NAND controller functions, or NULL if
        initialization failed.
-------------------------------------------------------------------*/
static NAND_CONTROLLER *InitController(
    FFXDEVHANDLE hDev)
{
    DCLTIMER t;

    /*  Find out the memory-mapped I/O address of the controller.
        There is no default, if the option is not available,
        initialization fails.
    */
    if (!FfxHookOptionGet(FFXOPT_FLASH_START, hDev,
                          &ctrlr.pBase, sizeof ctrlr.pBase))
    {
        return NULL;
    }

    /*  Configure the flash array
    */
    DCLMEMPUT32(NND_RST_STAT, 0);
    DCLMEMPUT32(NND_CONFIG,
                        NFC_CONFIG_WIDTH_8BIT |
                        NFC_CONFIG_PAGE_512 |
                        NFC_CONFIG_ECC_512 |
                        NFC_CONFIG_CS_1);
    DCLMEMPUT32(NND_SWAIT1, 10);     /* 24Mhz = 3 (>100ns) */
    DCLMEMPUT32(NND_SWAIT2, 24000);  /* 24Mhz=24000 (1ms) */

    /*  Reset the controller itself.
    */
    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);
    DCLMEMPUT32(NND_COMMAND, NFC_CMD_RESET);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    DclTimerSet(&t, NAND_TIMEOUT);
    while (!DclTimerExpired(&t))
    {
        if (DCLMEMGET32(NND_INT_RAW) & NFC_INT_RAW_MASK)
            break;
    }

    if (DCLMEMGET32(NND_INT_RAW) & NFC_INT_RAW_MASK)
    {

        ctrlr.nnd_config = DCLMEMGET32(NND_CONFIG);

        FFXPRINTF(1, ("LSI NAND Flash Controller\n"));
        return &ctrlr;
    }

    return NULL;
}


/*-------------------------------------------------------------------
    Local: UninitController()

    Placeholder, nothing to do in current implementation.

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()

    Return Value:
        None.
-------------------------------------------------------------------*/
static void UninitController(NAND_CONTROLLER *pCtrlr)
{
    /* nothing to do.
     */
     (void)pCtrlr;
}


/*-------------------------------------------------------------------
    Local: ChipSelect()

    Select a particular chip

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()
        uChip  - Chip number to select, in the range 0-1
                 exclusive.

    Return Value:
        TRUE if the chip became ready, FALSE if it timed out.
-------------------------------------------------------------------*/
static void ChipSelect(NAND_CONTROLLER *pCtrlr, unsigned int uChip)
{
    /*  The CHIPENn bits in NND_CONFIG select which chip to use.
    */
    if (!uChip)
        pCtrlr->nnd_config &= ~NFC_CONFIG_CS_1;
    else if (uChip == 1)
        pCtrlr->nnd_config |= NFC_CONFIG_CS_1;

    if (uChip < NCHIPS)
        DCLMEMPUT32(NND_CONFIG, pCtrlr->nnd_config);
}


/*-------------------------------------------------------------------
    Local: ResetChip()

    Issue a Reset command

    Issues the Reset command

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ResetChip(NAND_CONTROLLER *pCtrlr)
{
    DclAssert(pCtrlr == &ctrlr);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);
    DCLMEMPUT32(NND_COMMAND, NFC_CMD_RESET);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    ReadyWait(pCtrlr);
}


/*-------------------------------------------------------------------
    Local: ReadID()

    Issue a Read ID command.

    Parameters:
        pCtrlr   - NAND_CONTROLLER structure returned by
                   InitController()
        pBuffer  - Buffer to place the ID
        uLengths - length of the buffer

    Return Value:
        TRUE if the chip and controller were ready and accepted the
        command, FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL ReadID(NAND_CONTROLLER *pCtrlr, void *pBuffer, unsigned uLength)
{
    DCLALIGNEDBUFFER (id, data, FFX_NAND_MAXSPARESIZE);

    DclAssert(pCtrlr == &ctrlr);
    DclAssert(pBuffer);
    DclAssert(uLength);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */
    DCLMEMPUT32(NND_SYSADDR, id.data);
    DCLMEMPUT32(NND_COMMAND, NFC_CMD_READ_ID | NFC_CMD_ADR_CYCLE_1 | NFC_CMD_RD_DATA);
    DCLMEMPUT32(NND_TX_SIZE, uLength);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    if (!ReadyWait(pCtrlr))
        return FALSE;

    DclMemMove(pBuffer, id.data, uLength);

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ReadStatus()

    Issue a Read Status command and return the status.

    Parameters:
        pCtrlr  - NAND_CONTROLLER structure returned by
                  InitController()
        pBuffer - Buffer to contain the return status value

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ReadStatus(NAND_CONTROLLER *pCtrlr, D_UINT32 *pBuffer)
{
    DclAssert(pCtrlr == &ctrlr);
    DclAssert(pBuffer);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_STATUS);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    ReadyWait(pCtrlr);

    *pBuffer = DCLMEMGET32(NND_STATUS);
}


/*-------------------------------------------------------------------
    Local: ReadPage()

    Issues the Read Page command to read one physical page.

    Note that the controller splits the address bits into
    individual address cycle bytes differently, depending on
    whether the chip is small-block or large-block.  NND_CTRL
    field A8 governs this.

    Parameters:
        pCtrlr    - NAND_CONTROLLER structure returned by
                    InitController()
        ulAddress - the byte offset of the page, aligned on
                    a physical page boundary.  Bits representing
                    an offset within a page are discarded.
        pBuffer   -
        uLength   -
        pTag      -
        uUserSize -

    Return Value:
        TRUE if the chip and controller were ready and accepted the
        command, FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL ReadPage(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddressableUnitAddr, D_BUFFER *pBuffer, unsigned uLength, D_BUFFER *pTag, unsigned uUserSize)
{
    DCLALIGNEDBUFFER    (BounceBuffer, data, FFX_NAND_MAXSPARESIZE+FFX_NAND_MAXPAGESIZE);

    DclAssert(pCtrlr == &ctrlr);
    DclAssert(pBuffer);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */
    DCLMEMPUT32(NND_SYSADDR, BounceBuffer.data);
    DCLMEMPUT32(NND_ECC0, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC1, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC2, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC3, 0xFFFFFFFF);

#if ADDR_LINES_PER_ADDRESSABLE_UNIT > 8
    DCLMEMPUT32(NND_ADDRESS0, 0);
#else
    DCLMEMPUT32(NND_ADDRESS0, (ulAddressableUnitAddr << (ADDR_LINES_PER_ADDRESSABLE_UNIT - 8)) & 0xFF);
#endif
    DCLMEMPUT32(NND_ADDRESS1, (ulAddressableUnitAddr >> ( 9 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS2, (ulAddressableUnitAddr >> (17 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS3, (ulAddressableUnitAddr >> (25 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_READ_0 | NFC_CMD_ADR_CYCLE_4 | NFC_CMD_RD_DATA | NFC_CMD_WAIT);
/*    NFC_TX_SIZE_REG = 512+16; */
    DCLMEMPUT32(NND_TX_SIZE, uLength+uUserSize);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    if (!ReadyWait(pCtrlr))
        return FALSE;

    DclMemMove(pBuffer, BounceBuffer.data, uLength);

    if (pTag)
        DclMemMove(pTag, BounceBuffer.data+uLength, uUserSize);

    return TRUE;

}

/*-------------------------------------------------------------------
    Local: ReadPageNoData()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL ReadPageNoData(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddressableUnitAddr)
{

    DclAssert(pCtrlr == &ctrlr);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */
    DCLMEMPUT32(NND_SYSADDR, 0);
    DCLMEMPUT32(NND_ECC0, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC1, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC2, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC3, 0xFFFFFFFF);

#if ADDR_LINES_PER_ADDRESSABLE_UNIT > 8
    DCLMEMPUT32(NND_ADDRESS0, 0);
#else
    DCLMEMPUT32(NND_ADDRESS0, (ulAddressableUnitAddr << (ADDR_LINES_PER_ADDRESSABLE_UNIT - 8)) & 0xFF);
#endif
    DCLMEMPUT32(NND_ADDRESS1, (ulAddressableUnitAddr >> ( 9 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS2, (ulAddressableUnitAddr >> (17 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS3, (ulAddressableUnitAddr >> (25 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_READ_0 | NFC_CMD_ADR_CYCLE_4 | NFC_CMD_WAIT);
    DCLMEMPUT32(NND_TX_SIZE, 0);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    if (!ReadyWait(pCtrlr))
        return FALSE;

    return TRUE;

}
/*-------------------------------------------------------------------
    Local: ReadSpare()

    Issues the Read Spare (Read C) command to read the spare
    area only of one physical page.

    Note that the controller splits the address bits into
    individual address cycle bytes differently, depending on
    whether the chip is small-block or large-block.  NND_CTRL
    field A8 governs this.

    Parameters:
        pCtrlr    - NAND_CONTROLLER structure returned by
                    InitController()
        ulAddress - the byte offset of the page, aligned on
                    a physical page boundary.  Bits representing
                    an offset within a page are discarded.

    Return Value:
        TRUE if the chip and controller were ready and accepted the
        command, FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL ReadSpare(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddressableUnitAddr, D_BUFFER *pTag, unsigned uUserSize)
{
    DCLALIGNEDBUFFER (spare, data, FFX_NAND_MAXSPARESIZE);

    DclAssert(pCtrlr == &ctrlr);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */
    DCLMEMPUT32(NND_SYSADDR, spare.data);
    DCLMEMPUT32(NND_ECC0, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC1, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC2, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC3, 0xFFFFFFFF);

#if ADDR_LINES_PER_ADDRESSABLE_UNIT > 8
    DCLMEMPUT32(NND_ADDRESS0, 0);
#else
    DCLMEMPUT32(NND_ADDRESS0, (ulAddressableUnitAddr << (ADDR_LINES_PER_ADDRESSABLE_UNIT - 8)) & 0xFF);
#endif
    DCLMEMPUT32(NND_ADDRESS1, (ulAddressableUnitAddr >> ( 9 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS2, (ulAddressableUnitAddr >> (17 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS3, (ulAddressableUnitAddr >> (25 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_READ_2 | NFC_CMD_ADR_CYCLE_4 | NFC_CMD_RD_DATA | NFC_CMD_WAIT);
    DCLMEMPUT32(NND_TX_SIZE, uUserSize);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    if (!ReadyWait(pCtrlr))
        return FALSE;

    DclMemMove(pTag, spare.data, uUserSize);


    return TRUE;
}

/*-------------------------------------------------------------------
    Local: ReadSpareNoData()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL ReadSpareNoData(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddressableUnitAddr)
{
    DclAssert(pCtrlr == &ctrlr);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */
    DCLMEMPUT32(NND_SYSADDR, 0);
    DCLMEMPUT32(NND_ECC0, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC1, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC2, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC3, 0xFFFFFFFF);

#if ADDR_LINES_PER_ADDRESSABLE_UNIT > 8
    DCLMEMPUT32(NND_ADDRESS0, 0);
#else
    DCLMEMPUT32(NND_ADDRESS0, (ulAddressableUnitAddr << (ADDR_LINES_PER_ADDRESSABLE_UNIT - 8)) & 0xFF);
#endif
    DCLMEMPUT32(NND_ADDRESS1, (ulAddressableUnitAddr >> ( 9 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS2, (ulAddressableUnitAddr >> (17 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS3, (ulAddressableUnitAddr >> (25 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_READ_2 | NFC_CMD_ADR_CYCLE_4 | NFC_CMD_WAIT);
    DCLMEMPUT32(NND_TX_SIZE, 0);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    if (!ReadyWait(pCtrlr))
        return FALSE;

    return TRUE;
}

/*-------------------------------------------------------------------
    Local: ProgramPage()

    Issues the Program Page command to write one physical page.

    Note that the controller splits the address bits into
    individual address cycle bytes differently, depending on
    whether the chip is small-block or large-block.  NND_CTRL
    field A8 governs this.

    Parameters:
        pCtrlr    - NAND_CONTROLLER structure returned by
                    InitController()
        ulAddress - the byte offset of the page, aligned on
                    a physical page boundary.  Bits representing
                    an offset within a page are discarded.

    Return Value:
        TRUE if the chip and controller were ready and accepted the
        command, FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL ProgramPage(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddressableUnitAddr, const D_BUFFER *pBuffer, unsigned uLength)
{
    DCLALIGNEDBUFFER    (BounceBuffer, data, FFX_NAND_MAXSPARESIZE+FFX_NAND_MAXPAGESIZE);
    D_BOOL status;


    DclAssert(pCtrlr == &ctrlr);
    DclAssert(pBuffer);
    DclAssert(uLength > 0);

    DclMemMove(BounceBuffer.data, pBuffer, uLength);

    /* reset nand controller
    */
    ResetChip(pCtrlr);

    /* setup controller to program page
    */
    ReadPageNoData(pCtrlr, ulAddressableUnitAddr);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */
    DCLMEMPUT32(NND_SYSADDR, BounceBuffer.data);
    DCLMEMPUT32(NND_ECC0, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC1, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC2, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC3, 0xFFFFFFFF);

#if ADDR_LINES_PER_ADDRESSABLE_UNIT > 8
    DCLMEMPUT32(NND_ADDRESS0, 0);
#else
    DCLMEMPUT32(NND_ADDRESS0, (ulAddressableUnitAddr << (ADDR_LINES_PER_ADDRESSABLE_UNIT - 8)) & 0xFF);
#endif
    DCLMEMPUT32(NND_ADDRESS1, (ulAddressableUnitAddr >> ( 9 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS2, (ulAddressableUnitAddr >> (17 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS3, (ulAddressableUnitAddr >> (25 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_PROGRAM | NFC_CMD_ADR_CYCLE_4 | NFC_CMD_PRG_CFM | NFC_CMD_CMD2_EN | NFC_CMD_WR_DATA | NFC_CMD_WAIT);
    DCLMEMPUT32(NND_TX_SIZE, uLength);
    DCLMEMPUT32(NND_WRITEPROT, NFC_WRITE_UNPROTECT);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    status = ReadyWait(pCtrlr);

    DCLMEMPUT32(NND_WRITEPROT, NFC_WRITE_PROTECT);

    return status;
}


/*-------------------------------------------------------------------
    Local: ProgramSpare()

    Issues a Read Spare command to set the address pointer to the
    spare area, then issues the Program Page command to write the
    spare area of one physical page.

    Note that the controller splits the address bits into
    individual address cycle bytes differently, depending on
    whether the chip is small-block or large-block.  NND_CTRL
    field A8 governs this.

    Parameters:
        pCtrlr    - NAND_CONTROLLER structure returned by
                    InitController()
        ulAddress - the byte offset of the page, aligned on
                    a physical page boundary.  Bits representing
                    an offset within a page are discarded.

    Return Value:
        TRUE if the chip and controller were ready and accepted the
        command, FALSE if not.
-------------------------------------------------------------------*/
/*static D_BOOL ProgramSpare(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddress) */
static D_BOOL ProgramSpare(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddressableUnitAddr, const D_BUFFER *pTag, unsigned uUserSize)
{
    DCLALIGNEDBUFFER (spare, data, FFX_NAND_MAXSPARESIZE);
    D_BOOL status;

    DclAssert(pCtrlr == &ctrlr);

    DclMemMove(spare.data, pTag, uUserSize);

    /* reset nand controller
    */
    ResetChip(pCtrlr);

    /* setup controller to program spare area
    */
    ReadSpareNoData(pCtrlr, ulAddressableUnitAddr);

    /* program the spare area
    */
    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);  /* clear interrupt */
    DCLMEMPUT32(NND_SYSADDR, spare.data);
    DCLMEMPUT32(NND_ECC0, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC1, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC2, 0xFFFFFFFF);
    DCLMEMPUT32(NND_ECC3, 0xFFFFFFFF);

#if ADDR_LINES_PER_ADDRESSABLE_UNIT > 8
    DCLMEMPUT32(NND_ADDRESS0, 0);
#else
    DCLMEMPUT32(NND_ADDRESS0, (ulAddressableUnitAddr << (ADDR_LINES_PER_ADDRESSABLE_UNIT - 8)) & 0xFF);
#endif
    DCLMEMPUT32(NND_ADDRESS1, (ulAddressableUnitAddr >> ( 9 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS2, (ulAddressableUnitAddr >> (17 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS3, (ulAddressableUnitAddr >> (25 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_PROGRAM | NFC_CMD_PRG_CFM | NFC_CMD_CMD2_EN | NFC_CMD_WR_DATA | NFC_CMD_WAIT | NFC_CMD_ADR_CYCLE_4);
    DCLMEMPUT32(NND_TX_SIZE, uUserSize);
    DCLMEMPUT32(NND_WRITEPROT, NFC_WRITE_UNPROTECT);
    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    status = ReadyWait(pCtrlr);

    DCLMEMPUT32(NND_WRITEPROT, NFC_WRITE_PROTECT);

    return status;
}




/*-------------------------------------------------------------------
    Local: EraseBlock()

    Issues the Erase Block command.

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()

    Return Value:
        TRUE if the chip and controller were ready and accepted the
        command, FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL EraseBlock(NAND_CONTROLLER *pCtrlr, D_UINT32 ulAddressableUnitAddr)
{
    D_BOOL status;

    DclAssert(pCtrlr == &ctrlr);


    if (DCLMEMGET32(NND_INT_RAW) == 0)
        return FALSE;

    DCLMEMPUT32(NND_WRITEPROT, NFC_WRITE_UNPROTECT);

    DCLMEMPUT32(NND_INT_RAW, NFC_INT_RAW_MASK);
    DCLMEMPUT32(NND_ADDRESS0, (ulAddressableUnitAddr >> ( 9 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS1, (ulAddressableUnitAddr >> (17 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);
    DCLMEMPUT32(NND_ADDRESS2, (ulAddressableUnitAddr >> (25 - ADDR_LINES_PER_ADDRESSABLE_UNIT)) & 0xFF);

    DCLMEMPUT32(NND_COMMAND, NFC_CMD_ERASE | NFC_CMD_ADR_CYCLE_3 | NFC_CMD_ERASE_CFM | NFC_CMD_CMD2_EN | NFC_CMD_WAIT);

    DCLMEMPUT32(NND_CMDSTART, NFC_COMMAND_START);

    status = ReadyWait(pCtrlr);

    DCLMEMPUT32(NND_WRITEPROT, NFC_WRITE_PROTECT);

    return status;
}


/*-------------------------------------------------------------------
    Local: ReadyWait()

    Polls the R/B status of the chip, and returns when it indicates
    ready.

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()

    Return Value:
        TRUE if the chip became ready, FALSE if it timed out.
-------------------------------------------------------------------*/
static D_BOOL ReadyWait(NAND_CONTROLLER *pCtrlr)
{
    DCLTIMER t;

    (void)pCtrlr;
    DclTimerSet(&t, NAND_TIMEOUT);

    while (!DclTimerExpired(&t))
    {

        if ( DCLMEMGET32(NND_INT_RAW) != 0)
            return TRUE;
    }

    return (DCLMEMGET32(NND_INT_RAW) != 0);
}


/*-------------------------------------------------------------------
    Local: CheckECC()

    If ECC is enabled, check the main mage and spare area
    data against the controller's ECC calculated during the
    last data transfer.  Apply correction if necessary.

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()
        pPage  - main page data
        pSpare - spare area data

    Return Value:
        Returns a DCLECCRESULT value which will be one of the
        following values:
          DCLECC_VALID         - The data and old ECC value had no
                                 errors.
          DCLECC_DATACORRECTED - The data was corrected.
          DCLECC_ECCCORRECTED  - The old ECC value was corrected.
          DCLECC_UNCORRECTABLE - The data and/or old ECC has
                                 uncorrectable errors.
-------------------------------------------------------------------*/
static DCLECCRESULT CheckECC(
    NAND_CONTROLLER    *pCtrlr,
    void               *pPage,
    void               *pSpare)
{
    #define             ECC_MASK ((1 << 12) - 1) /* Assumes ECC on 512 bytes */
    D_BUFFER           *pNS = pSpare;
    D_UINT32           *ps = pSpare;
    D_UINT32            ulECC, ulSyndrome;

    if (!pCtrlr->fEccEnabled)
        return DCLECC_VALID;

    (void)pNS;

    DclAssert(pNS[NSSSFDC_FLAGS_OFFSET] == LEGACY_WRITTEN_WITH_ECC);

    /*  Get the ECC from the hardware and calculate the error
        syndrome.
     */
    ulECC = DCLMEMGET32(NND_ECC0);
    ulSyndrome = ulECC ^ ps[2];

    /*  If the syndrom is all zero, no error occurred.
    */
    if (ulSyndrome == 0)
    {
        return DCLECC_VALID;
    }

    /*  If a single-bit error occurred in the ECC itself, then the
        syndrome will have exactly one bit set.
    */
    else if (DCLISPOWEROF2(ulSyndrome))
    {
        ps[2] ^= ulSyndrome;
        return DCLECC_ECCCORRECTED;
    }

    /*  If a single-bit error occurred in the data, then there are
        exactly 12 bits set in the syndrome, and the even and odd
        halves of the syndrome are complements of each other.
    */
    else if ((ulSyndrome ^ ((ulSyndrome >> 16) & ECC_MASK)) == ECC_MASK)
    {
        /*  The odd half of the syndrome specifies the number of the
            bit that is in error.
        */
        unsigned uBit = (ulSyndrome >> 16) & ECC_MASK;
        D_UCHAR *pData = pPage;

        pData[uBit / 8] ^= (1 << uBit % 8);
        return DCLECC_DATACORRECTED;
    }

    /* In any other case, the error is uncorrectable.
    */
    else
    {
        return DCLECC_UNCORRECTABLE;
    }
}


#endif
