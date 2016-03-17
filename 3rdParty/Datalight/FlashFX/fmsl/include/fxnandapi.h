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

    This header contains NAND specific symbols, structures, types which may
    be accessed from outside the FMSL.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxnandapi.h $
    Revision 1.5  2011/03/03 23:25:06Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.4  2009/12/13 00:58:51Z  garyp
    Added more stuff, primarily from the now obsolete nthelp.h, which needs
    to be visible to project hooks code.
    Revision 1.3  2009/10/07 17:29:45Z  garyp
    Added stuff from the now obsolete ntspare.h.
    Revision 1.2  2009/10/06 21:22:14Z  garyp
    Removed an incorrect "const" modifier.
    Revision 1.1  2009/10/06 19:13:56Z  garyp
    Initial revision
    Revision 1.10  2009/07/31 22:50:10Z  garyp
    Commented out an unused structure.
    Revision 1.9  2009/02/25 22:31:22Z  glenns
    Added new macros for page status for use with MLC devices that may suffer
    high bit error rates.
    Revision 1.8  2009/02/07 18:44:50Z  glenns
    Fixed typo.  Added macros to locate CRC values in the default MLC spare
    area.
    Revision 1.7  2009/01/17 00:01:51Z  glenns
    Added macros for a "DefaultMLC" spare area format which includes a 64-byte
    spare area and ability to use fields in separate 16-byte segments for
    differing purposes.
    Revision 1.6  2008/03/23 20:40:03Z  Garyp
    Added the ISWITHIN1BIT() macro and simplified the various other IS....()
    macros for which it is the workhorse.
    Revision 1.5  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/09/25 21:47:37Z  pauli
    Resolved Bug 1455: Added enumeration of spare area formats.
    Revision 1.3  2007/09/12 20:16:02Z  Garyp
    Added the offsets of reserved bytes in the spare area layouts.
    Revision 1.2  2006/03/14 06:52:49Z  garyp
    Modified to support a new spare format for "OffsetZero" style flash which
    does not conflict with the factory bad block mark.
    Revision 1.1  2006/03/12 04:23:06Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXNANDAPI_H_INCLUDED
#define FXNANDAPI_H_INCLUDED

#include <fxiosys.h>
#include <nandid.h>
#include <nandconf.h>
#include <nandctl.h>
#include <ecc.h>

#define LEGACY_ENCODED_TAG_SIZE         (4)
#define LEGACY_TAG_SIZE                 (2)

/*-------------------------------------------------------------------
    16-byte spare area layout for FlashFX versions prior to v3.0.
-------------------------------------------------------------------*/
#define NSLEGACY_HIDDEN_OFFSET          (0)
#define NSLEGACY_BLOCKSTATUS_OFFSET     (4)
#define NSLEGACY_FACTORYBAD_OFFSET      (5) /* As defined by SSFDC */
#define NSLEGACY_ECC2_OFFSET            (8)
#define NSLEGACY_ECC1_OFFSET           (12)
#define NSLEGACY_GOOD_ECC               (0x7FFFFFFFL)

/*-------------------------------------------------------------------
    16-byte spare area layout for SSFDC style flash.  The
    "AnyBitZero" Toshiba style flash turns into this after we
    format, since we write the factory mark at SSFDC offset 5.
-------------------------------------------------------------------*/
#define NSSSFDC_TAG_OFFSET              (0)
#define NSSSFDC_TAG_LENGTH              (4)
#define NSSSFDC_RESERVED_1_OFFSET       (4) /* Used by pre FFX-v3.0 */
#define NSSSFDC_FACTORYBAD_OFFSET       (5)
#define NSSSFDC_MARKEDBAD_OFFSET        (6)
#define NSSSFDC_FLAGS_OFFSET            (7)
#define NSSSFDC_ECC2_OFFSET             (8)
#define NSSSFDC_RESERVED_2_OFFSET      (11) /* Not used by default */
#define NSSSFDC_ECC1_OFFSET            (12)
#define NSSSFDC_RESERVED_3_OFFSET      (15) /* Not used by default */
#define NSSSFDC_FACTORYBAD_LENGTH       (1)
#define NSSSFDC_ECC_LENGTH              (3)

/*-------------------------------------------------------------------
    16-byte spare area format for the "OffsetZero" style flash.
    For 2KB-page flash, there are 4 of these items in an array.
-------------------------------------------------------------------*/
#define NSOFFSETZERO_FACTORYBAD_OFFSET  (0)
#define NSOFFSETZERO_FACTORYBAD_LENGTH  (2) /* 1 or 2 bytes based on 8/16-bit interface */
#define NSOFFSETZERO_ECC1_OFFSET        (2)
#define NSOFFSETZERO_SSFDC_BAD_RESERVED (5) /* reserved to avoid legacy SSFDC conflicts - should never be used */
#define NSOFFSETZERO_MARKEDBAD_OFFSET   (6) /* marked bad by FlashFX    */
#define NSOFFSETZERO_FLAGS_OFFSET       (7)
#define NSOFFSETZERO_ECC2_OFFSET        (8)
#define NSOFFSETZERO_RESERVED_1_OFFSET (11) /* Not used by default */
#define NSOFFSETZERO_TAG_OFFSET        (12)
#define NSOFFSETZERO_TAG_LENGTH         (4)
#define NSOFFSETZERO_RESERVED_2_OFFSET (15) /* Not used by default */
#define NSOFFSETZERO_ECC_LENGTH         (3)

/*-------------------------------------------------------------------
    64-byte spare area format suitable for typical 2K-page MLC
    devices.
-------------------------------------------------------------------*/
#define NSDEFAULTMLC_FACTORYBAD_OFFSET  (0)
#define NSDEFAULTMLC_FACTORYBAD_LENGTH  (2)
#define NSDEFAULTMLC_TAG_OFFSET         (2)
#define NSDEFAULTMLC_TAG_LENGTH         (4)
#define NSDEFAULTMLC_ECC1_OFFSET        (6)
#define NSDEFAULTMLC_FLAGS_OFFSET      (18)
#define NSDEFAULTMLC_ECC2_OFFSET       (22)
#define NSDEFAULTMLC_ECC3_OFFSET       (38)
#define NSDEFAULTMLC_ECC4_OFFSET       (54)
#define NSDEFAULTMLC_ECC_LENGTH        (10)
#define NSDEFAULTMLC_SPARESIZE         (64)
#define NSDEFAULTMLC_TAGCRC            (34)
#define NSDEFAULTMLC_MAINCRC           (50)


#if 0
---> This is kept there for historical reference purposes only <---
typedef struct
{
    D_BUFFER       abEncodedTag[LEGACY_ENCODED_TAG_SIZE];
    D_UINT8        bReserved;
    D_UINT8        bFactoryBad;     /* offset 5: not ERASED8 if factory bad */
    D_UINT8        bMarkedBad;      /* ERASED8 if good, 0x00 if bad         */
    D_UINT8        bFlags;
    D_UINT32       ulEcc2;
    D_UINT32       ulEcc1;
} LEGACYNANDSPARE;
#endif


/*-------------------------------------------------------------------
    The LEGACYNANDSPARE bFlags values and macros are constructed to
    allow a single-bit error to occur, yet still allow the proper
    original value to be determined.

    LEGACY_OLD_WRITTEN_WITH_ECC differs from LEGACY_UNWRITTEN by a
    single bit, which is why we don't use that any more.
-------------------------------------------------------------------*/
#define LEGACY_OLD_WRITTEN_WITH_ECC (0x7F)    /* Pre-FFXPro v3 compatible   */
#define LEGACY_WRITTEN_WITH_ECC     (0x0F)    /* FFXPro v3.x or later       */
#define LEGACY_WRITTEN_BY_BBM       (0xF0)    /* FFXPro v3.x or later, obsolete with v4.1 and later */
#define LEGACY_UNWRITTEN            (ERASED8)

/*  MLC devices may need more than 8 bits of Hamming distance to
    unambiguously identify page status:
*/
#define MLC_WRITTEN_WITH_ECC       (0UL)
#define MLC_UNWRITTEN              (ERASED32)


#define ISWITHIN1BIT(a, b)          DCLISPOWEROF2((a) ^ (b))
#define ISOLDWRITTENWITHECC(stat)   (ISWITHIN1BIT((stat), LEGACY_OLD_WRITTEN_WITH_ECC))
#define ISWRITTENWITHECC(stat)      (ISWITHIN1BIT((stat), LEGACY_WRITTEN_WITH_ECC))
#define ISWRITTENBYBBM(stat)        (ISWITHIN1BIT((stat), LEGACY_WRITTEN_BY_BBM))
#define ISUNWRITTEN(stat)           (ISWITHIN1BIT((stat), LEGACY_UNWRITTEN))


/*  ECC modes
*/
typedef enum
{
    ECC_MODE_READ = 'R',
    ECC_MODE_WRITE = 'W'
} ECCMODE;


/*  Spare area formats supported by the NTM helper functions.
*/
typedef enum
{
    NSF_UNKNOWN = 0,
    NSF_OFFSETZERO,
    NSF_SSFDC
} NANDSPAREFMT;


/*---------------------------------------------------------
    An NTM Hook Handle is a pointer to any hook specific
    information which is necessary.  It is opaque so far
    as the actual NTM is concerned.
---------------------------------------------------------*/
typedef struct sNTMHOOK *NTMHOOKHANDLE;

typedef struct sNTMINFO NTMINFO;

typedef struct tagNTMDATA *NTMHANDLE;

typedef FFXIOSTATUS (*PFNREADSPAREAREA)  (NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
typedef FFXIOSTATUS (*PFNWRITESPAREAREA) (NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);


/*---------------------------------------------------------
    Function Prototypes
---------------------------------------------------------*/
void            FfxHookEccCalcStart(  NTMHOOKHANDLE hNtmHook, const D_BUFFER *pData, size_t nLength, D_BUFFER *pabECC, ECCMODE eccMode);
void            FfxHookEccCalcRead(   NTMHOOKHANDLE hNtmHook, const D_BUFFER *pData, size_t nLength, D_BUFFER *pabECC, ECCMODE eccMode);
FFXIOSTATUS     FfxHookEccCorrectPage(NTMHOOKHANDLE hNtmHook, D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pabECC, const FFXNANDCHIP *pChipInfo);

FFXIOSTATUS     FfxNtmHelpCorrectPage(D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pabECC, const FFXNANDCHIP *pChipInfo);
void            FfxNtmHelpTagEncode(D_BUFFER *pDest, const D_BUFFER *pSrc);
D_BOOL          FfxNtmHelpTagDecode(D_BUFFER *pDest, const D_BUFFER *pSrc);

FFXSTATUS       FfxNtmHelpBuildSpareArea(NTMHOOKHANDLE hNtmHook, NTMINFO *pNtmInfo, const D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pTag, D_BOOL fUseEcc, const FFXNANDCHIP *pChipInfo);
D_BOOL          FfxNtmHelpCorrectDataSegment(unsigned nCount, D_BUFFER *pData, D_BUFFER *pEccOriginal, const D_BUFFER *pabEccCalced, FFXIOSTATUS *pIOStat);
FFXIOSTATUS     FfxNtmHelpReadPageStatus(NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, D_UINT32 ulPage, const FFXNANDCHIP *pChipInfo);
D_UINT32        FfxNtmHelpGetPageStatus( const D_BUFFER *pSpare, size_t nTagOffset, size_t nFlagsOffset);
FFXIOSTATUS     FfxNtmHelpIsBadBlock(    NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock);
FFXIOSTATUS     FfxNtmHelpSetBlockType(  NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, PFNWRITESPAREAREA pfnWriteSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
NANDSPAREFMT    FfxNtmHelpGetSpareAreaFormat(const FFXNANDCHIP *pChip);
D_BOOL          FfxNtmHelpIsRangeErased1Bit(const D_BUFFER *pData, size_t nLen);

#if FFXCONF_NANDSUPPORT_LEGACY
    FFXIOSTATUS FfxNtmLegacyIsBadBlock(  NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock);
    FFXIOSTATUS FfxNtmLegacySetBlockType(NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, PFNWRITESPAREAREA pfnWriteSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
#endif

#if FFXCONF_NANDSUPPORT_SSFDC
    void        FfxNtmSSFDCBuildSpareArea(NTMHOOKHANDLE hNtmHook, NTMINFO *pNtmInfo, const D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pTag, D_BOOL fUseEcc);
    FFXIOSTATUS FfxNtmSSFDCCorrectPage(size_t nSpareSize, D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pabECC);
    FFXIOSTATUS FfxNtmSSFDCIsBadBlock(  NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock);
    FFXIOSTATUS FfxNtmSSFDCSetBlockType(NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, PFNWRITESPAREAREA pfnWriteSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
#endif

#if FFXCONF_NANDSUPPORT_OFFSETZERO
    void        FfxNtmOffsetZeroBuildSpareArea(NTMHOOKHANDLE hNtmHook, NTMINFO *pNtmInfo, const D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pTag, D_BOOL fUseEcc);
    FFXIOSTATUS FfxNtmOffsetZeroCorrectPage(size_t nSpareSize, D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pabECC);
    FFXIOSTATUS FfxNtmOffsetZeroIsBadBlock(  NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock);
    FFXIOSTATUS FfxNtmOffsetZeroSetBlockType(NTMHANDLE hNTM, PFNREADSPAREAREA pfnReadSpareArea, PFNWRITESPAREAREA pfnWriteSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
    void        FfxNtmOffsetZeroSpareAreaECCSet(D_BUFFER *pSpare, const D_BUFFER *pabECC, size_t nECCLen);
#endif

void            FfxNtmPECCBuildSpareArea(FFXDEVHANDLE hDev, NTMINFO *pNtmInfo, FX_ECC_PARAMS *pEccParams, D_BUFFER *pSpare, const D_BUFFER *pTag, D_BOOL fUseEcc, const FFXNANDCHIP *pChipInfo);
void            FfxNtmPECCCorrectPage(  NTMINFO *pNtmInfo, FX_ECC_PARAMS *pEccParams, const D_BUFFER *pSpare, const D_BUFFER *pReferenceSpare, const FFXNANDCHIP *pChipInfo);
void            FfxNtmPECCTagEncode(    NTMINFO *pNtmInfo, D_BUFFER *pSpare, const D_BUFFER *pSrc, NANDSPAREFMT fmt);
D_BOOL          FfxNtmPECCTagDecode(    NTMINFO *pNtmInfo, D_BUFFER *pDest, const D_BUFFER *pSrc, NANDSPAREFMT fmt);
FFXIOSTATUS     FfxNtmPECCGetPageStatus(NTMHANDLE hNTM, NTMINFO *pNtmInfo, PFNREADSPAREAREA pfnReadSpareArea, D_UINT32 ulPage, const FFXNANDCHIP *pChipInfo);
FFXIOSTATUS     FfxNtmPECCIsBadBlock(   NTMHANDLE hNTM, NTMINFO *pNtmInfo, PFNREADSPAREAREA pfnReadSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock);
FFXIOSTATUS     FfxNtmPECCSetBlockType( NTMHANDLE hNTM, NTMINFO *pNtmInfo, PFNREADSPAREAREA pfnReadSpareArea, PFNWRITESPAREAREA pfnWriteSpareArea, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);



#endif  /* FXNANDAPI_H_INCLUDED */

