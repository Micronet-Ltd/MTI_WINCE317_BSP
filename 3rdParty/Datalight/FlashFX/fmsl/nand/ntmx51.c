/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
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

    This NAND Technology Module (NTM) supports the NAND Flash Controller in
    the Freescale i.MX51 and related chips.

    Currently native page sizes of 512 2KB and 4KB for NAND is supported.  


---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntmx51.c $
    Revision 1.10  2012/03/05 02:18:26Z  garyp
    Minor typecast fixes to operate properly with 64-bit pointers.
    Revision 1.9  2012/02/18 01:10:26Z  garyp
    Declared some variables as static to avoid global namespace contention.
    However the better long-term fix awaits.
    Revision 1.8  2012/02/09 21:51:01Z  Glenns
    Minor coding standards fixup- no functional changes.
    Revision 1.7  2011/12/06 02:36:33Z  glenns
    Fix bug introduced in last check-in regarding handling of
    correctable bit errors.
    Revision 1.6  2011/12/03 01:18:34Z  glenns
    Add static structures for controller parameters. Fix EDC
    requirment and capability settings. Fix correctable error
    reporting. Clean up some comments.
    Revision 1.5  2011/11/30 01:04:34Z  glenns
    Fix compiler warnings about signed-unsigned mismatches.
    Revision 1.4  2011/11/29 15:23:24Z  jimmb
    Duplicate revision
    Revision 1.3  2011/11/22 14:36:14Z  jimmb
    New update to the ntms supplied by GL
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <deverr.h>
#include <fxiosys.h>
#include <nandcmd.h>
#include <ecc.h>
#include <errmanager.h>
#include "nandconf.h"
#include "nandctl.h"
#include <fhmx51.h>
#include "nandid.h"
#include "nand.h"
#include "ntm.h"
#include <hardware/hwmx51.h>

#define NTMNAME "NTMX51"
#define MAX_MX51_EDC_RS 8


/*  Configuration options for this NTM.
*/
/*  Define this as TRUE for variants of the controller that have an NFC_RST
    bit in the NAND_FLASH_CONFIG1 register.
*/
#ifndef NFC_HAS_RESET
#define NFC_HAS_RESET TRUE
#endif


/*  Spare area Format

    ECC data seems organized in 16 byte planes in this hardware.
    8 bytes can be used for user's purpose, and 8 bytes are used
    for the ECC sum.
  
    0 1 2 3 4 5 6 7 8 9 A B C D E F
    |<-------------|<------------->|
        User             ECC

    First 8 Bytes User Area and consecutive 8 Bytes Ecc
    
    If the flash is large-block NAND (2KB pages), the spare area as viewed
    by the MX51 NFC shown below.

    HW ECC spare area format(2K page size)

        +-----------------------------------+
     0  |        BI(1)    |     BI(2)       |
        +-----------------+-----------------+
     2  | User Area       | User Area       |
        +-----------------+-----------------+
     4  | User Area       | User Area       |
        +-----------------+-----------------+
     6  | User Area       | User Area       |
        +-----------------+-----------------+
     8  | M-S ECC byte 2  | M-S ECC byte 1  |
        +-----------------+-----------------+
    10  | M-S ECC byte 4  | M-S ECC byte 3  |
        +-----------------+-----------------+
    12  | M-S ECC byte 6  | M-S ECC byte 5  |
        +-----------------------------------+
    14  | M-S ECC byte 8  | M-S ECC byte 7  |
        +-----------------------------------+

    Keep offset 0 and 1 free for bad block marker usage.
         offset 2-7,16-23,32-39,48-55 are user Area.
         offset 8-15,24-31,40-47,56-63 are Ecc Positions(4 * 8 = 32 Bytes)

    Here's the table mention:

Address                 F E D C B A 9 8        7 6 5 4 3 2 1 0

0xAXI_BASE+0x1000 (SB0) Reserved for user(MSB) Reserved for user (LSB)
0xAXI_BASE+0x1002 (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x1004 (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x1006 (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x1008 (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x100A (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x100C (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x100E (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x1010-0xAXI_BASE_103F(SB0) Not in Use

This means the last 8 byte per 16 byte spare block are used for ecc.
This is repeated 3 times (for SB1/SB2/SB3).


*/

/*  Spare area Format For 4K + 218B Nand

    ECC data seems organized in 16 byte planes in this hardware.
    12 bytes can be used for user's purpose, and 14 bytes are used
    for the ECC sum.
  
    0 1 2 3 4 5 6 7 8 9 A B C D E F 10 11 12 13 14 15 16 17 18 19
    |<------------------->|------------------------------------>|
        User             ECC

    First 12 Bytes User Area and consecutive 14 Bytes Ecc
    
    If the flash is large-block NAND (4KB pages), the spare area as viewed
    by the MX51 NFC shown below.

    HW ECC spare area format(2K page size)

        +-----------------------------------+
     0  |        BI(1)    |     BI(2)       |
        +-----------------+-----------------+
     2  | User Area       | User Area       |
        +-----------------+-----------------+
     4  | User Area       | User Area       |
        +-----------------+-----------------+
     6  | User Area       | User Area       |
        +-----------------+-----------------+
     8  | User Area       | User Area       |
        +-----------------+-----------------+
    10  | User Area       | User Area       |
        +-----------------+-----------------+
    12  | M-S ECC byte 2  | M-S ECC byte 1  |
        +-----------------------------------+
    14  | M-S ECC byte 4  | M-S ECC byte 3  |
        +-----------------------------------+
    16  | M-S ECC byte 6  | M-S ECC byte 5  |
        +-----------------+-----------------+
    18  | M-S ECC byte 8  | M-S ECC byte 7  |
        +-----------------+-----------------+
    20  | M-S ECC byte 10 | M-S ECC byte 9  |
        +-----------------+-----------------+
    22  | M-S ECC byte 12 | M-S ECC byte 11 |
        +-----------------+-----------------+
    24  | M-S ECC byte 14 | M-S ECC byte 13 |
        +-----------------+-----------------+


    Keep offset 0 and 1 free for bad block marker usage.
         offset 2-11,26-37,52-63,78-89 are user Area.
         offset 12-25,38-51,64-79,90-101 are Ecc Positions(4 * 14 = 56 Bytes)

    Here's the table mention:

Address                 F E D C B A 9 8        7 6 5 4 3 2 1 0

0xAXI_BASE+0x1000 (SB0) Reserved for user(MSB) Reserved for user (LSB)
0xAXI_BASE+0x1002 (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x1004 (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x1006 (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x1008 (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x100A (SB0) Reserved for user      Reserved for user
0xAXI_BASE+0x100C (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x100E (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x1010 (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x1012 (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x1014 (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x1016 (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x1018 (SB0) ECC Byte               ECC Byte
0xAXI_BASE+0x101A-0xAXI_BASE_103F(SB0) Not in Use

This means the last 14 byte per 16 byte spare block are used for ecc.
*/

/*  Define this as TRUE to include support for flash parts using native 512B page sizes. */
#ifndef FFXCONF_NAND_512BNATIVE
#define FFXCONF_NAND_512BNATIVE FALSE
#endif

/*  Define this as TRUE to include support for flash parts using native 2K page sizes. */
#ifndef FFXCONF_NAND_2KBNATIVE
#define FFXCONF_NAND_2KBNATIVE TRUE
#endif

/*  Define this as TRUE to include support for flash parts using native 4K page sizes. */
#ifndef FFXCONF_NAND_4KBNATIVE
#define FFXCONF_NAND_4KBNATIVE FALSE
#endif

#if (FFXCONF_NAND_512BNATIVE & FFXCONF_NAND_2KBNATIVE & FFXCONF_NAND_4KBNATIVE) 
#error "Cannot configure for 512B, 2KB and 4KB native support"
#endif

#if (FFXCONF_NAND_512BNATIVE & (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_4KBNATIVE ))
#error "Cannot configure for 512B, and 2KB OR 4KB native support"
#endif

#if (FFXCONF_NAND_2KBNATIVE & (FFXCONF_NAND_512BNATIVE || FFXCONF_NAND_4KBNATIVE ))
#error "Cannot configure for 2KB and 512B OR 4KB native support"
#endif

#if (FFXCONF_NAND_4KBNATIVE & (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BNATIVE ))
#error "Cannot configure for 4KB and 512B OR 2KB native support"
#endif


/*  The RAM buffer contains 16-bit words, and may not be accessed as bytes.
    It gets copied to RAM for manipulation.
    Nand Controller Logically divides RAM into 8 sections. Each section contains
    512B of main data and 64B of spare area.
*/
#if (FFXCONF_NAND_512BNATIVE)
#define SPARE_INDEX_FBBSTATUS_XX             (3)  
#define MAIN_BUFFER_SIZE                     (512)
#define SPARE_BUFFER_SIZE                    (16)
#define SPARE_INDEX_ECCSTATUS                (4)
#endif

#if (FFXCONF_NAND_2KBNATIVE)
#define SPARE_INDEX_FBBSTATUS_XX             (51)  /* Fifth byte of the last SB3 (64 / 4 * 3) + 3          */
#define MAIN_BUFFER_SIZE                     (2048)
#define SPARE_BUFFER_SIZE                    (64)
#define SPARE_INDEX_ECCSTATUS                (52)
#endif

#if (FFXCONF_NAND_4KBNATIVE)
#define SPARE_INDEX_FBBSTATUS_XX             (115) /* Fifth byte of SB7 (128 / 8 * 7) + 5  */
#define MAIN_BUFFER_SIZE                     (4096)
#define SPARE_BUFFER_SIZE                    (128)
#define SPARE_INDEX_ECCSTATUS                (116)
#endif

#define TAG_SIZE                             (2)

#define ECCSTATUS_NONE                       (ERASED8)
#define ECCSTATUS_ECC                        (0xF0)
#define MX51_SPARE_INDEX_TAGSTART_16         (0)  /* For 512_PAGE: offset 0,1,2,3 Are used for UserArea
                                                     offset 7, 8, 9, 10, 11, 12, 13, 14, 15 are Ecc pos 
                                                  */ 
#define MX51_SPARE_INDEX_TAGSTART_64         (48)
#define MX51_SPARE_INDEX_TAGSTART_128_218    (112)

#define CORRECTABLE(a, b) DCLISPOWEROF2((a) ^ (b))

/*  This indicator is used to indicate that the location potentially
    containing a factory bad block marker is invalid (has been erased or
    overwritten).  Note that this only applies for those pages in blocks that
    were NOT marked bad by the manufacturer (in other words, we don't write
    this value in blocks that actually contain a FBB marker indicating the
    block is bad - only in blocks that DON'T).
*/
#define FBBSTATUS_FBB_INVALID                0xF0




/*  Timeout in msec. for NAND flash operations.  The longest operation is
    erase, with typical spec of 2 msec, but can be as high as 174ms on some
    devices.  It's okay if this is generous, timeout generally means
    something is badly wrong, and wasting a few  msec. doesn't matter at
    that point.
*/
#define NAND_TIMEOUT                        (500)

/*  Private table for the NTM's NAND device-specific internal operations.
*/
typedef struct
{
    FFXIOSTATUS (*PageReadEcc)    (D_UINT32 ulPage,       void *pMain,       void *pTag, unsigned nTagSize,unsigned nOffSet);
    FFXIOSTATUS (*PageReadNoEcc)  (D_UINT32 ulPage,       void *pMain, void *pSpare);
    FFXIOSTATUS (*PageWriteEcc)   (D_UINT32 ulPage, const void *pMain, const void *pTag, unsigned nTagSize,unsigned nOffSet);
    FFXIOSTATUS (*PageWriteNoEcc) (D_UINT32 ulPage, const void *pMain, const void *pSpare);
    FFXIOSTATUS (*SpareRead)      (D_UINT32 ulPage,       void *pSpare);
    FFXIOSTATUS (*SpareWrite)     (D_UINT32 ulPage, const void *pSpare);
    FFXIOSTATUS (*TagRead)        (D_UINT32 ulPage,       void *pTags, unsigned nTagSize,unsigned nOffSet);
    FFXIOSTATUS (*TagWrite)       (D_UINT32 ulPage, const void *pTags, unsigned nTagSize,unsigned nOffSet);
    FFXIOSTATUS (*GetPageStatus)  (D_UINT32 ulPage,unsigned nOffSet);
    FFXIOSTATUS (*GetBlockStatus) (D_UINT32 ulPhysicalPage, D_UINT8 bFbbType);
    FFXIOSTATUS (*SetBlockStatus) (NTMHANDLE hNTM,D_UINT32 ulPhysicalPage, D_UINT32 ulBlockStatus);
}   NTMX51OPERATIONS;


/*  Private data structure for this NTM.
*/
struct tagNTMDATA
{
    FFXDEVHANDLE        hDev;
    NTMINFO             NtmInfo;    /* information visible to upper layers */
    NTMX51OPERATIONS   *pNANDOps;   /* NAND device specific operations functions  */
    const FFXNANDCHIP  *pChipInfo;
    FFXMX51PARAMS       Params;
    PNANDCTL            pNC;        /* Private project hooks information   */
    D_UINT32            ulPagesPerChip;
};

/*  State information about the NAND Flash Controller (NFC). Since
    the i.MX51 has exactly one of these, the structure may be
    allocated statically and referenced directly (no need to pass 
    a pointer around).
*/
static struct
{
    void *pBase;
    unsigned int uConfig1;
    D_UCHAR ucWidthBytes;
    NTMDATA *pNTM;
    D_BOOL fEccEnabled;
    unsigned uAddrsBytes;   /* page # and up (not offset in page) */
    unsigned fResetBeforeProgram : 1;
    D_UINT16 uEdcSetting;
    D_UINT32 ulEccBitMask;
} NFC;

/*  The structure NFC_IP is allocated statically and referenced directly
    (no need to pass apointer areound).
*/
static struct
{
    void *pIPBase;
    unsigned int uConfig2;
    unsigned int uConfig3;
}NFC_IP;



/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_mx51 =
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
    IORequest,
    NULL
};

/*  Generic operations (conceptually perhaps should be in the
    vector, but no differences among implementations)
*/
static FFXSTATUS ProcessChipStatus(unsigned int uChipStatus);
static FFXIOSTATUS EraseBlock(D_UINT32 ulPhysicalPage);

/*  Operations for flash 
*/
static FFXIOSTATUS PageReadEcc(D_UINT32 ulPage, void *pMain, void *pTag, unsigned nTagSize,unsigned nOffSet);
static FFXIOSTATUS PageReadNoEcc(D_UINT32 ulPage, void *pMain, void *pSpare);
static FFXIOSTATUS PageWriteEcc(D_UINT32 ulPage, const void *pMain, const void *pTag, unsigned nTagSize,unsigned nOffSet);
static FFXIOSTATUS PageWriteNoEcc(D_UINT32 ulPage, const void *pMain, const void *pSpare);
static FFXIOSTATUS ReadSpare(D_UINT32 ulPage, void *pSpare);
static FFXIOSTATUS WriteSpare(D_UINT32 ulPage, const void *pSpare);
static FFXIOSTATUS ReadTag(D_UINT32 ulPage, void *pTags, unsigned nTagSize,unsigned nOffSet);
static FFXIOSTATUS WriteTag(D_UINT32 ulPage, const void *pTags, unsigned nTagSize,unsigned nOffSet);
static FFXIOSTATUS ReadPageStatus(D_UINT32 ulPage,unsigned nOffSet);
static FFXIOSTATUS ReadBlockStatus(D_UINT32 ulPhysicalPage, D_UINT8 bFbbType);
static FFXIOSTATUS WriteBlockStatus(NTMHANDLE hNTM,D_UINT32 ulPhysicalPage, D_UINT32 ulBlockStatus);

static NTMX51OPERATIONS NativeOps =
{
    PageReadEcc,
    PageReadNoEcc,
    PageWriteEcc,
    PageWriteNoEcc,
    ReadSpare,
    WriteSpare,
    ReadTag,
    WriteTag,
    ReadPageStatus,
    ReadBlockStatus,
    WriteBlockStatus
};
#if FFXCONF_NAND_512BNATIVE
/*  This initialized static buffer is to be used in
    when the caller does not supply a spare buffer to
    NFCPageOut().

    This is a bad practice.  Static variables should almost never be
    used unless they are also declared as "const".  Also, why do we
    bother initializing this to ERASED8 when the code itself also
    initializes the buffer?
*/
static union
{
    D_BUFFER            data[SPARE_BUFFER_SIZE];
    DCL_ALIGNTYPE       DummyAlign;
} UnusedSpare = { { ERASED8, ERASED8, ERASED8, ERASED8,    
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        } };
#endif

#if FFXCONF_NAND_2KBNATIVE
/*  This initialized static buffer is to be used in
    when the caller does not supply a spare buffer to
    NFCPageOut().

    This is a bad practice.  Static variables should almost never be
    used unless they are also declared as "const".  Also, why do we
    bother initializing this to ERASED8 when the code itself also
    initializes the buffer?
*/
static union
{
    D_BUFFER            data[SPARE_BUFFER_SIZE];
    DCL_ALIGNTYPE       DummyAlign;
} UnusedSpare = { { ERASED8, ERASED8, ERASED8, ERASED8,    
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8} };
#endif

#if FFXCONF_NAND_4KBNATIVE
/*  This initialized static buffer is to be used in when the caller does
    not supply a spare buffer to NFCPageOut4K().

    This is a bad practice.  Static variables should almost never be
    used unless they are also declared as "const".  Also, why do we
    bother initializing this to ERASED8 when the code itself also
    initializes the buffer?
*/    
static union
{
    D_BUFFER  data[SPARE_BUFFER_SIZE];
    DCL_ALIGNTYPE  DummyAlign;
}   UnusedSpare = { {ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8,
                        ERASED8, ERASED8, ERASED8, ERASED8} };
#endif

static NTMX51OPERATIONS * IdentifyFlash(NTMHANDLE hNTM);
static NTMX51OPERATIONS * ConfigController(NTMHANDLE hNTM,PNANDCTL pNC, const FFXNANDCHIP *pChip);

static D_BOOL ReadStatus(unsigned int *puStatus);
static void   UninitController(void);
static D_BOOL IDIn(D_UCHAR aucID[NAND_ID_SIZE]);
static D_BOOL StatusIn(D_UCHAR *pucStatus);

static D_BOOL PageInHwEcc(void *pMain, void *pTag, unsigned nTagSize, FFXIOSTATUS *pStatus,unsigned nOffSet);
static D_BOOL PageOutHwEcc(const void *pMain, const void *pTag, unsigned nTagSize,unsigned nOffSet);
static void   EncodeTagHwEcc(const D_BUFFER *pTag, unsigned nTagSize, D_BUFFER pSpare[SPARE_BUFFER_SIZE],unsigned nOffSet);
static D_BOOL DecodeTagHwEcc(D_BUFFER *pTag, unsigned nTagSize, const D_BUFFER pSpare[SPARE_BUFFER_SIZE],unsigned nOffSet);

static D_BOOL NFCPageIn(void *pMain, void *pSpare);
static D_BOOL NFCPageOut(const void *pMain, const void *pSpare);
static D_BOOL NFCSpareIn(void *pSpare);
static D_BOOL NFCSpareOut(const void *pSpare);
static D_BOOL NFCCommandOut(D_UCHAR uCommand);
static D_BOOL NFCAddressOut(D_UINT32 ulAddress, unsigned uCount);
static D_BOOL NFCRowOut(D_UINT32 ulAddress, unsigned uCount);
static D_BOOL NFCColumnOut(D_UINT32 ulAddress, unsigned uCount);
static D_BOOL NFCPageAddressOut(D_UINT32 ulPage, unsigned uCount);
static D_BOOL NFCBlockAddressOut(D_UINT32 ulPage, unsigned uCount);
static D_BOOL NFCReadyWait(void);
static D_BOOL NFCWait(void);
static void   SetEccMode(D_BOOL fUseEcc);
static void   NFCIPCEnable(void);
static void   NFCIPCDisable(void);
static void   NFCClearStatus(void);
static void   SetCs(unsigned int dwCs);
#if D_DEBUG
static void   NFCAutoLaunch(unsigned oPeration);
#endif

#if NFC_HAS_RESET
static void NFCReset(void);
#else
#define NFCReset()
#endif

typedef D_UINT32 MainBuffer[MAIN_BUFFER_SIZE / sizeof (D_UINT32)];
typedef D_UINT32 SpareBuffer[SPARE_BUFFER_SIZE / sizeof (D_UINT32)];

#define NFC_MAIN_BUFFER ((MainBuffer *) ((char *) NFC.pBase))
#define NFC_SPARE_BUFFER ((SpareBuffer *) ((char *) NFC.pBase + 0x1000))
/* Regardless of the page size, the NFC transfers by 512 + xx chunks. Stupid chip! */
#define NFC_NATIVE_SIZE    (512)

#define SPARE_SEGMENTS     (MAIN_BUFFER_SIZE / NFC_NATIVE_SIZE)        
#define LAST_SPARE_SEGMENT (SPARE_SEGMENTS - 1)                    
#define SPARE_SEGMENT_SIZE (SPARE_BUFFER_SIZE / SPARE_SEGMENTS)    



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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-Create()\n"));

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if (pNTM)
    {
        pNTM->hDev = hDev;
        pNTM->Params.nStructLen = sizeof(pNTM->Params);
        pNTM->pNC = FfxHookNTMX51Create(hDev,&pNTM->Params);
        if(pNTM->pNC)
        {
            pNTM->pNANDOps = IdentifyFlash(pNTM);
            if (pNTM->pNANDOps)
            {
                D_UINT32 ulTotalBlocks;
                 FFXFIMBOUNDS bounds;

                /*  Get the array bounds now so we know how far to scan
                */
                FfxDevGetArrayBounds(hDev, &bounds);

                /*  This controller only supports a single chip (has only
                    one chip select), so the total size is the size of the
                    chip.
                */
                ulTotalBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;

                /*  Fill in the NTMINFO appropriate to the chip that
                    was found.
                */
                pNTM->NtmInfo.uDeviceFlags  |= DEV_REQUIRES_ECC;

                if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
                    pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
                if (pNTM->pChipInfo->pChipClass->fProgramOnce)
                    pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

                pNTM->NtmInfo.ulTotalBlocks = ulTotalBlocks;
                pNTM->NtmInfo.ulBlockSize   = pNTM->pChipInfo->pChipClass->ulBlockSize;
                pNTM->NtmInfo.uPageSize     = pNTM->pChipInfo->pChipClass->uPageSize;
                pNTM->NtmInfo.uSpareSize    = pNTM->pChipInfo->pChipClass->uSpareSize;

                pNTM->NtmInfo.uMetaSize     = TAG_SIZE; /* covered by spare area ECC */
                pNTM->NtmInfo.uAlignSize    = 1;        /* arbitrary buffer alignment allowed when reading/writing with this NTM */

                if( pNTM->pChipInfo->pChipClass->uEdcRequirement > MAX_MX51_EDC_RS )
                {
                    /* This chip requires more correction capabilities
                       than this NTM can handle - fail the create.
                    */
                    FFXPRINTF(1, ("NTMX51-Create() Insufficient EDC capabilities.\n"));
                    goto CreateCleanup;
                }

                /*  MX51 controller is capable of correcting 4 or 8 bit per
                    512-byte segment. This will have been set in the NFC
                    static structure by the ConfigController call from
                    IdentifyFlash, based on the device's EDC requirement.
                */

                pNTM->NtmInfo.uEdcRequirement    = pNTM->pChipInfo->pChipClass->uEdcRequirement;
                pNTM->NtmInfo.uEdcCapability     = NFC.uEdcSetting;
                pNTM->NtmInfo.uEdcSegmentSize    = NAND512_PAGE; /* EDC on 512-byte segments */
                DclAssert(pNTM->NtmInfo.uEdcRequirement <= pNTM->NtmInfo.uEdcCapability);

                /*  In case of working with NAND devices of 512B+16B, 2K+64B
                    or 4K+128B(ie 16B of spare buffer per 512 byte section of
                    main page flash) then, only ECC_MODE of 4 bits is allowed
                    and then first 8B of relavent spare buffers are reserved
                    for user and the rest 8B are reserved for ECC.
                    
                    The following may seem peculiar, but it is in fact a
                    "feature" of the MX51 controller that it can't do 8-bit
                    EDC on devices with less than a 4K page.
                */
                if (pNTM->NtmInfo.uEdcRequirement > 4)
                {
                    DclAssert (pNTM->NtmInfo.uPageSize == NAND4K_PAGE);
                    DclAssert (pNTM->NtmInfo.uSpareSize > 128);
                }
                    
                pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
                pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;


                /*  Set the tag offset:
                */
                if(pNTM->NtmInfo.uPageSize == NAND512_PAGE)
                {
                    DclAssert((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_SSFDC);
                    pNTM->Params.nTagOffset     = MX51_SPARE_INDEX_TAGSTART_16;
                }
                else if(pNTM->NtmInfo.uPageSize == NAND2K_PAGE)
                {
                    DclAssert((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_OFFSETZERO);
                    pNTM->Params.nTagOffset     = MX51_SPARE_INDEX_TAGSTART_64;
                }
                else if(pNTM->NtmInfo.uPageSize == NAND4K_PAGE)
                {
                    pNTM->Params.nTagOffset      = MX51_SPARE_INDEX_TAGSTART_128_218;     
                }
                else
                {
                    DclError();
                }

                /*  Calculate this once and store it for use later for
                    both optimization and simplification of the code
                */
                DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
                DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
                pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);

                DclAssert((pNTM->NtmInfo.ulBlockSize % 1024UL) == 0);

                pNTM->NtmInfo.ulChipBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;
                pNTM->ulPagesPerChip       = pNTM->NtmInfo.ulChipBlocks * pNTM->NtmInfo.uPagesPerBlock;

                /*  Initialize the spare area buffer used when
                    the caller does not supply their own.
                */
                DclMemSet(UnusedSpare.data, ERASED8, sizeof (UnusedSpare.data));

                /* Successfully created the NTM instance.
                */
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                                NTMNAME"-Create() returning %P\n", pNTM));
                *ppNtmInfo = &pNTM->NtmInfo;
                return pNTM;
            }

  CreateCleanup:
            UninitController();
            FfxHookNTMX51Destroy(pNTM->pNC);
        }
        DclMemFree(pNTM);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-Create() failed\n"));

    return NULL;
}

/*-------------------------------------------------------------------
    Local: IdentifyFlash()

    Read the ID information from the NAND chip, and make sure it
    is a chip suitable for use.

    Parameters:
        hNTM - The handle for the NTM instance with which to
               identify the flash.

    Return Value:
        A vector containing functions to operate the flash using
        this controller, if the chip is one that this NTM knows how
        to operate, NULL if not.
-------------------------------------------------------------------*/
static NTMX51OPERATIONS * IdentifyFlash(NTMHANDLE hNTM)
{
    D_UCHAR abID[NAND_ID_SIZE];   /*  Manufacturer and device IDs, plus chip-
                                      specific data
                                  */
    unsigned int chipNum=0;

   /*  Find out the memory-mapped I/O address of the controller
        There is no default, if the option is not available,
        initialization fails.
    */
    if (FfxHookOptionGet(FFXOPT_FLASH_START, hNTM->hDev,
                         &NFC.pBase, sizeof NFC.pBase))
    {
        if (FfxHookNTMX51GetIPBaseAdr(hNTM->pNC, &NFC_IP.pIPBase))
        {
            /* NAND Flash Reset */
            /* If there are More than 1 chips Reset all the Chips */
            for(chipNum=0; chipNum < hNTM->Params.nMaxChips; chipNum++)
            {
                SetCs(chipNum);
                NFCCommandOut(CMD_RESET);
            }
            /*  Read the manufacturer and device IDs from the flash.
            */
            if (NFCCommandOut(CMD_READ_ID)
                && NFCRowOut(0, 1)
                && IDIn(abID))
            {
                /*  Look up the chip
                */
                hNTM->pChipInfo = FfxNandDecodeID(abID);

                if (hNTM->pChipInfo)
                {
                    /*  Ensure that the flash that was found is compatible with the
                        spare area format supported by the MX51 and NTM with regard
                        to the factory bad block marks
                    */
                    if ((hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_SSFDC
                        || (hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_OFFSETZERO)
                    {
                        /* Configure the controller for the flash.
                        */
                        return (ConfigController(hNTM,hNTM->pNC, hNTM->pChipInfo));
                    }

                    FFXPRINTF(1, ("NTMX51:IdentifyFlash() Factory bad block marker method incompatible with this NTM.\n"));
                }

            }
            return NULL;
        }
    }
    return NULL;
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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-Destroy(%P)\n", hNTM));

    UninitController();
    FfxHookNTMX51Destroy(hNTM->pNC);
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
        ulPage   - The flash offset in pages, relative to any
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
    D_UINT32            ulStartPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-PageRead() Start Page=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
                    ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTMX51-PageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTags && nTagSize == TAG_SIZE) || (!pTags && !nTagSize));

    /*  If fUseEcc is FALSE, then the tag size (and the tag pointer, asserted
        above) must be clear.
    */
    DclAssert(fUseEcc || !nTagSize);

    while (ulCount)
    {
        FFXIOSTATUS status;

        if (fUseEcc)
        {
            status = hNTM->pNANDOps->PageReadEcc(ulStartPage, pPages, pTags, nTagSize,hNTM->Params.nTagOffset);
        }
        else
        {
            DclAssert(pTags == NULL);
            DclAssert(nTagSize == 0);
            status = hNTM->pNANDOps->PageReadNoEcc(ulStartPage, pPages, NULL);
        }

        ioStat.ffxStat = status.ffxStat;
        if ((ioStat.ffxStat != FFXSTAT_SUCCESS) && (ioStat.ffxStat != FFXSTAT_FIMCORRECTABLEDATA))
            break;

        /*  PAGESTATUS_DATACORRECTED could be set in ulPageStatus. Break out
            if correctable error detected.
        */
        ioStat.op.ulPageStatus |= status.op.ulPageStatus;
        if (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            ioStat.ulCount++;
            break;
        }

        /*  The page was read successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulStartPage += status.ulCount;
        if (pTags)
            pTags += status.ulCount * nTagSize;
    }

    DclProfilerLeave(0UL);
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-PageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-PageWrite() StartPage=%lX Count=%lU TagSize=%u fUseEcc=%U\n",
                    ulStartPage, ulCount, nTagSize, fUseEcc));

    DclProfilerEnter("NTMX51-PageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));

    while (ulCount)
    {
        FFXIOSTATUS status;

        if (fUseEcc)
        {
            status = hNTM->pNANDOps->PageWriteEcc(ulStartPage, pPages, pTags, nTagSize,hNTM->Params.nTagOffset);
        }
        else
        {
            DclAssert(!pTags);
            DclAssert(!nTagSize);
            status = hNTM->pNANDOps->PageWriteNoEcc(ulStartPage, pPages, NULL);
        }

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulStartPage += status.ulCount;
        if(nTagSize)
            pTags += status.ulCount * nTagSize;
    }

    DclProfilerLeave(0UL);
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
        successfully.  Portions of the pPages array beyond those
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
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-RawPageRead() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
                    ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTMX51-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    while (ulCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->PageReadNoEcc(ulPage, pPages, pSpares);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was read successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulPage += status.ulCount;
        if (pSpares)
            pSpares += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-RawPageWrite() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
                    ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTMX51-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    while (ulCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->PageWriteNoEcc(ulPage, pPages, pSpares);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulPage += status.ulCount;
        if (pSpares)
            pSpares += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
                    NTMNAME"-SpareRead() Page=%lX\n", ulPage));

    DclAssert((D_UINTPTR)pSpare % 4 == 0);

    while (uCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->SpareRead(ulPage, pSpare);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Successful read, account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ioStat.ulCount += status.ulCount;
        pSpare += status.ulCount * hNTM->NtmInfo.uSpareSize;
        ulPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-SpareRead() Page=%lX returning %s\n",
                    ulPage, FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-SpareWrite() Page=%lX\n", ulPage));

    DclAssert((D_UINTPTR)pSpare % 4 == 0);

    while (uCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->SpareWrite(ulPage, pSpare);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ioStat.ulCount += status.ulCount;
        pSpare += status.ulCount * hNTM->NtmInfo.uSpareSize;
        ulPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-HiddenRead() Start Page=%lX Count=%X TagSize=%u\n", ulStartPage, ulCount, nTagSize));

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    while (ulCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->TagRead(ulStartPage, pTags, nTagSize,hNTM->Params.nTagOffset);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  PAGESTATUS_DATACORRECTED could be set in ulPageStatus. Break out
            if correctable error detected.
        */
        ioStat.op.ulPageStatus |= status.op.ulPageStatus;
        if (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            ioStat.ulCount++;
            break;
        }

        /*  Successful read, account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pTags += status.ulCount * nTagSize;
        ulStartPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-HiddenRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenWrite()

    Write the hidden (tag) data for one page.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The page offset in bytes, relative to any
                   reserved space.
        pTag     - Buffer for the hidden data to write.
        nTagSize - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely written.
        The status indicates whether the write was entirely
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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n",
                    ulPage, *(D_UINT16*)pTag, nTagSize));

    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    while (uCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->TagWrite(ulPage, pTag, nTagSize,hNTM->Params.nTagOffset);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Successful write, account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ioStat.ulCount += status.ulCount;
        pTag += status.ulCount * nTagSize;
        ulPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    D_UINT32            ulPhysicalPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-BlockErase() Block=%lX\n", ulBlock));

    while (uCount)
    {
        FFXIOSTATUS status;

        ulPhysicalPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

        status = EraseBlock(ulPhysicalPage);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /* The block was erased successfully.  Account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ulBlock += status.ulCount;
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetPageStatus()

    This function retrieves the page status information for the
    given page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.

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
    FFXIOSTATUS         ioStat;

    DclAssert(hNTM);

    ioStat = hNTM->pNANDOps->GetPageStatus(ulPage,hNTM->Params.nTagOffset);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-GetPageStatus() Page=%lX returning %s\n",
                    ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetBlockStatus()

    This function retrieves the block status information for the
    given erase block.

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
    NTMHANDLE       hNTM,
    D_UINT32        ulBlock)
{
    FFXIOSTATUS     ioStat;
    int i;
    D_UINT32        ulPhysicalPage;

    DclAssert(hNTM);

    /*  Ensure that the flash in use is compatible with this function
    */
    DclAssert((hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_SSFDC
              || (hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_OFFSETZERO);

    /*  Convert to PHYSICAL page size

        Note!  This is NOT the emulated page size.
    */
    ulPhysicalPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    /*  Check the bad block indicator in the first two pages of the block
        only.
    */
    for (i = 0; i < 2; ++i)
    {
        ioStat = hNTM->pNANDOps->GetBlockStatus(ulPhysicalPage, (D_UINT8)(hNTM->pChipInfo->bFlags & CHIPFBB_MASK));
        if (ioStat.op.ulBlockStatus != BLOCKSTATUS_NOTBAD)
        {
            break;
        }
        ulPhysicalPage++;
    }
    ioStat.ulFlags = (ioStat.ulFlags & ~IOFLAGS_TYPEMASK) | IOFLAGS_BLOCK;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-GetBlockStatus() Block=%lX returning %s\n",
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
    NTMHANDLE       hNTM,
    D_UINT32        ulBlock,
    D_UINT32        ulBlockStatus)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_BLOCKIO_STATUS;
    D_UINT32        ulPhysicalPage;

    DclAssert(hNTM);

    /*  Convert to PHYSICAL page size

        Note!  This is NOT the emulated page size.
    */
    ulPhysicalPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    ioStat = hNTM->pNANDOps->SetBlockStatus(hNTM,ulPhysicalPage, ulBlockStatus);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-SetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
                    ulBlock, ulBlockStatus, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
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


/*-------------------------------------------------------------------
    Local: ProcessChipStatus()

    Given a status value read from a NAND chip, interpret
    its value into a FlashFX status.

    Parameters:
        uChipStatus - status read from the NAND chip

    Return Value:
        FFXIOSTATUS indicating the status of the NAND operation.
-------------------------------------------------------------------*/
static FFXSTATUS ProcessChipStatus(unsigned int uChipStatus)
{
    FFXSTATUS ffxStat;

    if ((uChipStatus & (NANDSTAT_WRITE_ENABLE | NANDSTAT_READY | NANDSTAT_ERROR))
        == (NANDSTAT_WRITE_ENABLE | NANDSTAT_READY))
    {
        ffxStat = FFXSTAT_SUCCESS;
    }
    else
    {
        if (!(uChipStatus & NANDSTAT_READY))
        {
            DclError();
            ffxStat = FFXSTAT_FIMTIMEOUT;
        }
        else if (!(uChipStatus & NANDSTAT_WRITE_ENABLE))
        {
            ffxStat = FFXSTAT_FIM_WRITEPROTECTEDBLOCK;
        }
        else
        {
            ffxStat = FFXSTAT_FIMIOERROR;
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: EraseBlock()

    Erase one physical erase block.

    Parameters:
        ulPhysicalPage - physical (never emulated) page number of
                         the first page in the block to erase

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS EraseBlock(
    D_UINT32 ulPhysicalPage)
{
    FFXIOSTATUS status = INITIAL_BLOCKIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    if (NFCCommandOut(CMD_ERASE_BLOCK)
        && NFCBlockAddressOut(ulPhysicalPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_ERASE_BLOCK_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("Erase: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: ReadSpareNative()

    Perform an entire spare area Read command for large-block NAND,
    returning the NATIVE page spare area (that which contains
    the factory bad block mark).

    Parameters:
        ulPhysicalPage - physical page number of the page containing
                         the spare area data to be read
        pSpare         - Spare Size-byte buffer to receive spare area data, aligned
                         on a 4-byte boundary.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadSpareNative(
    D_UINT32 ulPhysicalPage,
    D_BUFFER *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCPageAddressOut(ulPhysicalPage, NFC.uAddrsBytes) 
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}

/*-------------------------------------------------------------------
    Local: WriteSpareNative()

    Perform an entire spare area write operation for large-block NAND,
    programming the NATIVE page spare area (that which contains
    the factory bad block mark).
 
    Parameters:
        ulPhysicalPage - physical page number of the page containing
                         the spare area data to be read
        pSpare         - Spare Size-byte buffer containing spare area data, aligned
                         on a 4-byte boundary.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteSpareNative(
    D_UINT32 ulPhysicalPage,
    D_BUFFER *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    /*  Some NAND chips need to be reset before programming.
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCPageAddressOut(ulPhysicalPage, NFC.uAddrsBytes)
        && NFCSpareOut(pSpare)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("WriteSpareNative: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


 
/*-------------------------------------------------------------------
    Local: PageReadNoEcc()

    Perform an entire Read command for large-block NAND.

    Parameters:
        ulPage - page to read
        pMain  - Page Size-byte buffer to receive main page data.
        pSpare - Spare Size-byte buffer to receive spare area data, may
                 be NULL.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageReadNoEcc(
    D_UINT32 ulPage,
    void *pMain,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCPageAddressOut(ulPage,NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCPageIn(pMain, pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageReadEcc()

    Perform an entire Read command for large-block NAND.

    Parameters:
        ulPage - page to read.
        pMain  - Page Size-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pTag   - buffer to receive the tag; may be NULL if tag is
                 not desired.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageReadEcc(
    D_UINT32    ulPage,
    void       *pMain,
    void       *pTag,
    unsigned    nTagSize,
    unsigned    nOffSet)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCPageAddressOut(ulPage,NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && PageInHwEcc(pMain, pTag, nTagSize, &status, nOffSet))
    {
        status.ulCount = 1;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageWriteNoEcc()

    Perform an entire program command sequence for large-block NAND.

    Parameters:
        ulPage - page to write
        pMain  - Page Size-byte buffer containing main page data, aligned
                 on a 4-byte boundary.
        pSpare - Spare Size-byte buffer containing spare area data, may
                 be NULL.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWriteNoEcc(
    D_UINT32 ulPage,
    const void *pMain,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;
 
    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);
    /*  Some NAND chips need to be reset before programming.
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCPageAddressOut(ulPage, NFC.uAddrsBytes)
        && NFCPageOut(pMain, pSpare)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWriteNoEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageWriteEcc()

    Write a Page Size-byte page from large-block NAND.

    Perform an entire program command sequence for large-block NAND.

    Parameters:
        ulPage - page to write
        pMain  - Page Size-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pTag   - tag to write

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWriteEcc(
    D_UINT32    ulPage,
    const void *pMain,
    const void *pTag,
    unsigned    nTagSize,
    unsigned    nOffSet)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned    uChipStatus;

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);
    /*  Some NAND chips need to be reset before programming.
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCPageAddressOut(ulPage, NFC.uAddrsBytes)
        && PageOutHwEcc(pMain, pTag, nTagSize,nOffSet)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWriteEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: ReadSpare()

    Read a Spare Size-byte spare area from large-block NAND.

    Perform an entire Read command for large-block NAND.

    Parameters:
        ulPage - the page containing the spare area
        pSpare - buffer to receive the spare area data

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadSpare(
    D_UINT32 ulPage,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);
 


    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCPageAddressOut(ulPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(pSpare))
    {
        status.ffxStat = FFXSTAT_SUCCESS;
        status.ulCount = 1;
    }

    return status;

}


/*-------------------------------------------------------------------
    Local: WriteSpare()

    Write a Spare Size-byte spare area to large-block NAND.

    Parameters:
        ulPage - the page containing the spare area
        pSpare - Spare Size-byte buffer with data to be written to the spare
                 area, aligned on a 4-byte boundary.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteSpare(
    D_UINT32 ulPage,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    /*  Some NAND chips need to be reset before programming.
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCPageAddressOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareOut(pSpare)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("WriteSpare: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: ReadTag()

    Read tags from from large-block NAND.

    An uncorrectable ECC error or bad check byte causes an
    all-zero (invalid) tag to be returned, but still returns a
    value indicating success.

    Parameters:
        ulPage - the page containing the tag
        pTag   - buffer to receive the tags

    Return Value:
        Should tag reads ever fail?  Yes, if timeout occurs then something
        is Horribly Wrong.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadTag(
    D_UINT32            ulPage,
    void               *pTag,
    unsigned            nTagSize,
    unsigned            nOffSet)
{
    D_BOOL            uncorrectableError = FALSE;
    FFXIOSTATUS       status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    DCLALIGNEDBUFFER  (spare, data, SPARE_BUFFER_SIZE);
    
    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);
    
    /*  Turn the controller's ECC on.
    */
    SetEccMode(TRUE);
    DclMemSet(spare.data, ERASED8, sizeof(spare.data));
    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCPageAddressOut(ulPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(spare.data))
    {
        D_UINT32 ulEccStatus = DCLMEMGET32(ECC_STATUS_RESULT);

        /*  Do a quick test for no error, this should be true most of the time.
            For the tag field, we only need to check the ECC of the NFC's last section.
        */
        
        if (ulEccStatus == ECC_STATUS_NO_ERROR )
        {
            status.ffxStat = FFXSTAT_SUCCESS;
            DecodeTagHwEcc(pTag, nTagSize, spare.data,nOffSet);
        }
        else
        {
            int i = 0;
            D_UINT32 eccError=0;
            D_UINT32 ulNumBitErrors = 0;
            
            for (i = 0; i < SPARE_SEGMENTS; i++)
            {
                eccError = (ulEccStatus & NFC.ulEccBitMask);

                /*  Cast OK because the bit mask above limits checking
                    to low-order 4 bits only for the MX51 controller.
                */
                if ((eccError) > (D_UINT32)NFC.uEdcSetting)
                {
                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),    
                        "ntmx51.TagRead128(): Uncorrectable error in tag.  Page: %lu   EccStatus: %04x\n", ulPage, DCLMEMGET32(ECC_STATUS_RESULT)));

                    /*  This controller cannot discriminate between ECC
                        errors in the data area vs. spare area.
                    */
                    status.ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                    DclMemSet(pTag, 0, nTagSize);
                    NFCClearStatus();
                    uncorrectableError = TRUE;
                    break;
                }

                ulEccStatus >>= NFC_ECC_SECTION_BITS;
                ulNumBitErrors += eccError;
            }

            if (!uncorrectableError)
            {
                /*  A correctable error occurred in the data/spare area.
                    The controller has already corrected the data. Update
                    the statistics, but remember HiddenRead will look
                    to see if a correctable error has occured and set
                    the status accordingly.
                */
                (void)DecodeTagHwEcc(pTag, nTagSize, spare.data,nOffSet);
                FfxErrMgrEncodeCorrectedBits(ulNumBitErrors, &status);
                status.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                status.ffxStat = FFXSTAT_SUCCESS;
            }
        }

        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: WriteTag()

    Write tags to from large-block NAND.

    Parameters:
        ulPage - the page containing the tag
        pTag   - buffer to receive the tags

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteTag(
    D_UINT32            ulPage,
    const void         *pTag,
    unsigned            nTagSize,
    unsigned            nOffSet)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned            uChipStatus;
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    DclMemSet(spare.data, ERASED8, sizeof spare.data);

    if(nTagSize)
        EncodeTagHwEcc(pTag, nTagSize, spare.data,nOffSet);

    /*  Some NAND chips need to be reset before programming.
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCPageAddressOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareOut(spare.data)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("WriteTag: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: ReadPageStatus()

    This function retrieves the page status information for the
    given page.

    Parameters:
        ulPage - The flash page from which to retrieve the status.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulPageStatus variable will contain the page status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadPageStatus(
    D_UINT32            ulPage,
    unsigned            nOffSet)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    ioStat = ReadSpare(ulPage, spare.data);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Because only the last 16 bytes of the NAND controller's
            view of the spare area overlap with the NAND device's
            view of the spare area for Native pages, we only store
            the FlashFX bad block status information in that last
            16 byte region (See comments at the top of this module
            and in ReadBlockStatus for additional details).

            Basically, with the exception of ECC info, we treat the
            last 16 bytes of the spare area as if it were really the
            entire/only spare area available to FlashFX.
        */

        if (CORRECTABLE(spare.data[SPARE_INDEX_ECCSTATUS], ECCSTATUS_NONE))
        {
            ioStat.op.ulPageStatus = PAGESTATUS_UNWRITTEN;   /* blank/no ECC */
        }
        else if (CORRECTABLE(spare.data[SPARE_INDEX_ECCSTATUS], ECCSTATUS_ECC))
        {
            ioStat.op.ulPageStatus = PAGESTATUS_WRITTENWITHECC;
        }
        else
        {
            ioStat.op.ulPageStatus = PAGESTATUS_UNKNOWN;   /* indecipherable */
        }

        /*  If the tag area, including the check byte, is within 1 bit of
            being erased, then we know that a tag was not written.  If it
            is anything else, we know a tag was written.
        */

        /*  Move the tag check byte so it immediately follows the
            tag data (depending on byte order it might already
            be there).
        */
        spare.data[nOffSet + TAG_SIZE] = spare.data[nOffSet + TAG_SIZE];

        if(!FfxNtmHelpIsRangeErased1Bit(&spare.data[nOffSet], TAG_SIZE+1))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(TAG_SIZE);
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: ReadBlockStatus()

    This function retrieves the block status information for the
    given erase block.

    Parameters:
        ulPhysicalPage - The physical page containing the status
                         of the block to return.
        bFbbType       - flags indicating the factory bad block type for the
                         flash

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulBlockStatus variable will contain the block status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadBlockStatus(
    D_UINT32        ulPhysicalPage,
    D_UINT8         bFbbType)
{
    FFXIOSTATUS     ioStat;
    DCLALIGNEDBUFFER (spare, data, SPARE_BUFFER_SIZE);
    D_BUFFER        *pNative2KSpare;

    /*  All 2K page flash supported by this NTM uses the OFFSETZERO
        factory bad block mark type.
    */
    DclAssert(bFbbType == CHIPFBB_OFFSETZERO);
    (void)bFbbType;

    do
    {
        /*  Read the spare area.
        */
        ioStat = ReadSpareNative(ulPhysicalPage, spare.data);

        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, ("NTMX51:ReadBlockStatus() ReadSpareNative() failed.\n"));
            break;
        }
        pNative2KSpare = spare.data;


        /*  If there are any zero bits in the first two bytes (word offset 0),
            of the 'real' (chip) spare area (and for 2KB page native, this is
            distinctly NOT the spare area data as read into the spare buffer
            by the MX51 NAND flash controller), the block is bad.

            Note also the logic below is both BE and LE compatible, since
            for 8-bit flash, pNative2KSpare[0] holds the FBB mark regardless
            of the processor endianness, and for 16-bit flash, we have to check
            BOTH bytes of the word (and the order is irrelevant).  Some day I
            hope to have a clearer way to write this that is also correct for
            both.
        */
        if ((pNative2KSpare[NSOFFSETZERO_FACTORYBAD_OFFSET] != ERASED8)
                && ((NFC.ucWidthBytes == 1) || (pNative2KSpare[NSOFFSETZERO_FACTORYBAD_OFFSET + 1] != ERASED8)))
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            break;
        }

        /*  All checks are okay, the block is good.
        */
        ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

    } while(FALSE);

#if D_DEBUG
    if (ioStat.ffxStat == FFXSTAT_SUCCESS
        && (ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) != BLOCKSTATUS_NOTBAD)
    {
        D_UINT16 *puSpare = (D_UINT16 *) spare.data;
        D_UINT32 *pulSpare = (D_UINT32 *) spare.data;

        DclPrintf("Bad block status %02lX at physical page %08lX contains:\n",
                  ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE, ulPhysicalPage);
        DclPrintf("    %04X %04X %04X %04X %04X %04X %04X %04X\n",
                  puSpare[0], puSpare[1], puSpare[2], puSpare[3],
                  puSpare[4], puSpare[5], puSpare[6], puSpare[7]);
        DclPrintf("    %08lX %08lX %08lX %08lX\n",
                  pulSpare[0], pulSpare[1], pulSpare[2], pulSpare[3]);
    }
#endif

    return ioStat;

}

/*-------------------------------------------------------------------
    Local: WriteBlockStatus()

    This function sets the block status information for the
    given erase block.

    Parameters:
        ulPhysicalPage - The first physical page in the erase block
                         to be marked.
        ulBlockStatus  - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard I/O status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteBlockStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulPhysicalPage,
    D_UINT32        ulBlockStatus)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    /*  This should only be done for factory bad blocks during formatting.
        There is no need for any other kind of marking.
    */
    switch (ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        case BLOCKSTATUS_FACTORYBAD:
        {
            DCLALIGNEDBUFFER (spare, data, SPARE_BUFFER_SIZE);
            int i;

            DclMemSet(spare.data, ERASED8, sizeof spare.data);

            switch (hNTM->pChipInfo->bFlags & CHIPFBB_MASK)
            {
                /*  All 512B page flash is NSSSFDC style, mark the FBB location
                    keep offset 5 free for bad block marker usage 
                */
                case CHIPFBB_SSFDC:
                    spare.data[NSSSFDC_FACTORYBAD_OFFSET] = 0;
                    break;

                case CHIPFBB_OFFSETZERO:
                    /*  All 2KB and 4KB page flash is offsetzero style, mark the FBB location
                    */
                    spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET] = 0;
                    spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET+1] = 0;
                    break;

                default:
                    DclError();
                    break;
            }


            /*  Try to program the bad block indicator in at least one of the first
                two pages of the block.
            */
            for (i = 0; i < 2; ++i)
            {
                ioStat = WriteSpareNative(ulPhysicalPage, spare.data);
                if (ioStat.ffxStat == FFXSTAT_SUCCESS)
                    break;

                ulPhysicalPage++;
            }
        }

        default:
            /*  Unexpected block status values are an error.
            */
            DclError();
            ioStat.ffxStat = FFXSTAT_BADPARAMETER;
            break;

    }

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
    NTMHANDLE       hNTM,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
                    NTMNAME"-ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
                    hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTMX51:ParameterGet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
                    NTMNAME"-ParameterGet() returning status=%lX\n", ffxStat));

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
    NTMHANDLE       hNTM,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
                    NTMNAME"-ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
                    hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTMX51:ParameterSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
                    NTMNAME"-ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: ReadStatus()

    Read status from the NAND chip.

    Issue a status command and read the status.

    Parameters:
        puStatus - the status returned by the chip

    Return Value:
        TRUE if the status was read, FALSE if the command timed out.
-------------------------------------------------------------------*/
static D_BOOL ReadStatus(unsigned int *puStatus)
{
    D_UCHAR ucStatus = 0;

    if (NFCCommandOut(CMD_STATUS)
        && StatusIn(&ucStatus))
    {
        *puStatus = ucStatus;
        return TRUE;
    }
    return FALSE;
}

/*-------------------------------------------------------------------
    Actual operations on the controller.
-------------------------------------------------------------------*/

#if D_DEBUG
/*-------------------------------------------------------------------
    Local: ByteOrderTest()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void ByteOrderTest(void)
{
    D_UINT32 ulTest = 0x04030201;
    D_UCHAR *puc;
    D_UINT16 *pu;
    D_UINT32 *pul;
    
    DCLMEMPUT32(NFC_CONFIGURATION1, NFC_CONFIGURATION1_RBA_FIRST);
    NFCClearStatus();
    NFCAutoLaunch(AUTO_READ);
    
    puc = (D_UCHAR *) &ulTest;
    DclPrintf("Memory byte order: %02x %02x %02x %02x\n",
              puc[0], puc[1], puc[2], puc[3]);

    pu = (D_UINT16 *) &ulTest;
    DclPrintf("Memory word order: %04x %04x\n", pu[0], pu[1]);

    pul = NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];
    *pul = ulTest;

    pu = (D_UINT16 *) NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];
    DclPrintf("Buffer word order: %04x %04x\n", pu[0], pu[1]);
}
#endif



/*-------------------------------------------------------------------
    Local: ConfigController()

    Configure the NAND controller for the flash.

    Configure the NFC and this NTM to operate with the kind of flash
    that was detected.

    Parameters:
        pNC   - private project hooks info
        pChip - the chip information

    Return Value:
        A vector containing functions to operate the flash using
        this controller, if the chip is one that this NTM knows how
        to operate, NULL if not.
-------------------------------------------------------------------*/
static NTMX51OPERATIONS * ConfigController(
    NTMHANDLE           hNTM,
    PNANDCTL            pNC,
    const FFXNANDCHIP  *pChip)
{
    NTMX51OPERATIONS   *pNANDOps = NULL;
    unsigned flashWidth=0;
    unsigned uEccMode;
    NFC.fResetBeforeProgram = pChip->pChipClass->ResetBeforeProgram;
    NFC.uAddrsBytes =
        (pChip->pChipClass->uLinearChipAddrMSB - pChip->pChipClass->uLinearPageAddrMSB + 7)
        / 8;

    if(pChip->pChipClass->uEdcRequirement <= 4)
    {
        uEccMode       = NFC_CONFIG2_ECC_4_BIT_MODE;
        NFC.uEdcSetting = NFC_ECC_STATUS_RESULT_NOSER_4SB_ERR;
        NFC.ulEccBitMask     = NFC_ECC_BIT_UNCORRECTABLE_ERROR_4BIT;
        
    }
    else if (pChip->pChipClass->uEdcRequirement <= 8)
    {
       uEccMode       = NFC_CONFIG2_ECC_8_BIT_MODE;
       NFC.uEdcSetting = NFC_ECC_STATUS_RESULT_NOSER_8SB_ERR;
       NFC.ulEccBitMask = NFC_ECC_BIT_UNCORRECTABLE_ERROR_8BIT;
    }
    else
    {
        FFXPRINTF(1, ("NTMX51:ConfigController() Insufficient EDC capability."));
        return NULL;
    }
    
    pNANDOps = &NativeOps;

    /*  For 8-bit and 16-bit wide NAND. 
    */
    if((pChip->bFlags & CHIPINT_MASK) == CHIPINT_8BIT)
    {
        NFC.ucWidthBytes = CHIPINT_8BIT;
        flashWidth = NFC_CONFIGURATION3_FW_8BIT;
    }
    else if((pChip->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
    {
        NFC.ucWidthBytes = CHIPINT_16BIT;
        flashWidth = NFC_CONFIGURATION3_FW_16BIT;
    }
    else
    {
        FFXPRINTF(1, ("NTMX51:ConfigController() unknown bus width.\n"));
        return NULL;
    }
    /* NFC configuration 2      
      Set NFC_CONFIGURATION2 register
    
        PS - 2KB page size(0x01)                       = 0x00000001     
        SYM - asymmetric mode (0 << 2)                 = 0x00000000
        ECC_EN - enable ECC (1 << 3)                   = 0x00000008
        READ_CONFIRM - needed (1 << 4)                 = 0x00000010
        NUM_OF_ADDR_ERASE - (1 << 5)                   = 0x00000020
        ECC_MODE - 4 bits error correction (1 << 6)    = 0x00000040
        PPB - 128 page per block (0x02<<7)             = 0x00000100
        EDC - Extra dead cycle ( 0 << 9)               = 0x00000000   
        NUM_OF_ADDR_PHASE - 5 phases (2 << 12)         = 0x00002000 
        AUTO_PROG_DONE_MASK - disable(0 <<14)          = 0x00000000
        INT_MSK - mask interrupt (0 << 15)             = 0x00000000
        SPAS - spare area size (0x20<<16)              = 0x00200000
        STATUS_CMD - (0x70 << 24)                      = 0x70000000   
                                                       ------------
                                                         0x70202179     
    */

    NFCIPCEnable();
    NFC_IP.uConfig2=0;
     /*  Set NFC Configuration2 Register  
     */
    NFC_IP.uConfig2 = (DLBITFVAL(NFC_NFC_CONFIGURATION2_PS, pChip->pChipClass->uPageSize >> 11) | NFC_CONFIGURATION2_SYM
                      | NFC_CONFIGURATION2_ECC_EN | NFC_CONFIG2_NUM_CMD_PHASES
                      | DLBITFVAL(NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES0, (CHIP_ADDR_CYCLES_NUM - NUM_OF_BLOCK_CYCLES - 1))
                      | uEccMode
                      | DLBITFVAL(NFC_NFC_CONFIGURATION2_PPB, pChip->pChipClass->ulBlockSize/pChip->pChipClass->uPageSize >> 6)
                      | NFC_CONFIGURATION2_EDC
                      | DLBITFVAL(NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1, (CHIP_ADDR_CYCLES_NUM - 3))
                      | NFC_CONFIG2_ATO_PRG_DNE_MSK
                      | NFC_CONFIGURATION2_INT_MSK 
                      | DLBITFVAL(NFC_NFC_CONFIGURATION2_SPAS,  pChip->pChipClass->uSpareSize >>1)
                      | DLBITFVAL(NFC_NFC_CONFIGURATION2_ST_CMD, CMDREADSTATUS));

    DCLMEMPUT32(NFC_CONFIGURATION2,NFC_IP.uConfig2);
#if D_DEBUG
    DclPrintf("New NFC_CONFIGURATION2 = 0x%x\r\n",DCLMEMGET32(NFC_CONFIGURATION2));
#endif
    NFCIPCDisable();
    /*Set NFC_CONFIGURATION3 register    
        ADD_OP - address mode (0 << 0)                  = 0x00000000
        TOO - one device for CS (0 << 2)                = 0x00000000
        FW - Flash width ( 1 << 3)                      = 0x00000008      
        SB2R - status bit to record (0 << 4)            = 0x00000000
        NF_BIG - little endian ( 0 << 7)                = 0x00000000     
        SBB - status busy bit  ( 6 << 8)                = 0x00000600
        DMA_MODE - two dma signal  ( 0 << 11)           = 0x00000000
        NUM_OF_DEV - number of devices (0 << 12)        = 0x00000000    
        RBB_MODE - check status reg (0 << 15)           = 0x00000000            
        FMP - 64 (1 << 16)                              = 0x00010000
        NO_SDMA -  (1 << 20)                            = 0x00100000               
                                                        ------------
                                                          0x00110608
    */
    /*  Set NFC Configuration3 Register 
    */
    NFCIPCEnable();
    NFC_IP.uConfig3 =(NFC_CONFIGURATION3_ADD_OP   
                     | NFC_CONFIGURATION3_TOO
                     | flashWidth 
                     | DLBITFVAL(NFC_NFC_CONFIGURATION3_SB2R, STATUSERRORBIT) 
                     | DLBITFVAL(NFC_NFC_CONFIGURATION3_SBB, STATUSBUSYBIT) 
                     | NFC_CONFIGURATION3_DMA_MODE 
                     | DLBITFVAL(NFC_NFC_CONFIGURATION3_NUM_OF_DEVICES, (hNTM->Params.nMaxChips - 1))  
                     | NFC_CONFIGURATION3_RBB_MODE 
                     | NFC_CONFIGURATION3_NO_SDMA);
    DCLMEMPUT32(NFC_CONFIGURATION3,NFC_IP.uConfig3);
    DCLMEMPUT32(NFC_IPC,0); 
#if D_DEBUG
    DclPrintf("NFC_CONFIGURATION3 = 0x%x\r\n",DCLMEMGET32(NFC_CONFIGURATION3));
#endif            
    NFCIPCDisable();

    NFCIPCEnable();
    DCLMEMPUT32(NFC_DELAY_LINE,0x00);
    NFCIPCDisable();
              
    NFC.uConfig1 = (NFC_CONFIG1_SP_EN_MAIN_SPARE | 
                    NFC_CONFIGURATION1_NF_CE     | 
                    NFC_CONFIGURATION1_NFC_RST   |
                    NFC_CONFIGURATION1_RBA_FIRST |
                    NFC_CONFIGURATION1_NUM_OF_ITE | 
                    NFC_CONFIGURATION1_CS_0);

    DCLMEMPUT32(NFC_CONFIGURATION1, NFC.uConfig1);
    NFC.uConfig1 &= ~NFC_CONFIGURATION1_NFC_RST;    /* Clear the NFC Reset bit! */
#if D_DEBUG
    DclPrintf("After NFC_CONFIGURATION1 = 0x%x\r\n", DCLMEMGET32(NFC_CONFIGURATION1));
    ByteOrderTest();
#endif
    NFCIPCEnable();
    {
        unsigned int i=0;
        for(i=0;i < hNTM->Params.nMaxChips;i++)
        {
            /*  Blocks to be unlocked 
            */
            DCLREG32BF(UNLOCK_BLK_ADD(i), NFC_UNLOCK_BLK_ADD_USBA, 0);
            DCLREG32BF(UNLOCK_BLK_ADD(i), NFC_UNLOCK_BLK_ADD_UEBA, pChip->pChipClass->ulChipBlocks - 1);
            DCLMEMPUT32(NF_WR_PROT, NFC_NF_WR_PROT_CS2L_CS(i) | NFC_NF_WR_PROT_BLS_UNLOCKED  | NFC_NF_WR_PROT_WPC_UNLOCK);
            #if D_DEBUG
                DclPrintf("NF_WR_PROT for (CS %d ) = %08X\n",i, DCLMEMGET32(NF_WR_PROT));
            #endif          
          
        }
    }
    NFCIPCDisable();

    return pNANDOps;
}



/*-------------------------------------------------------------------
    Local: UninitController()

    Undo initialization of NAND controller.

    Placeholder, nothing to do in current implementation.

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void UninitController(void)
{
    /* nothing to do.
    */
}


/*-------------------------------------------------------------------
    Local: IDIn()

    Get ID data from the NAND chip.

    Transfers chip ID data from the NAND chip to memory.

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()
        aucID  - NAND_ID_SIZE buffer to receive the ID data

    Return Value:
        TRUE if transfer succeeded, FALSE if controller timed out.
-------------------------------------------------------------------*/
static D_BOOL IDIn(D_UCHAR aucID[NAND_ID_SIZE])
{
    /*  Use the zero'th buffer of the Eight.
     */
    DCLMEMPUT32(NFC_CONFIGURATION1, NFC_CONFIGURATION1_RBA_FIRST);    
    NFCClearStatus();

    /*  Get the ID data out of the chip.  The controller reads six
        bytes; depending on the chip, varying amounts may be
        meaningful.
    */
    DCLMEMPUT32(LAUNCH_NFC, FDO_ID);

    if (NFCWait())
    {
        D_UINT32 ulID;

        DclAssert(NAND_ID_SIZE > 5);

        /*  Note that because the NAND bus width is always x8 at this point,
            the ID bytes are byte-packed even if the NAND chip has an x16
            data bus.
        */
#if DCL_BIG_ENDIAN
        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0]);
        aucID[0] = (D_UCHAR) (ulID >> 24);
        aucID[1] = (D_UCHAR) (ulID >> 16);
        aucID[2] = (D_UCHAR) (ulID >> 8);
        aucID[3] = (D_UCHAR) ulID;

        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][1]);
        aucID[4] = (D_UCHAR) (ulID >> 24);
        aucID[5] = (D_UCHAR) (ulID >> 16);
#else
        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0]);
        aucID[0] = (D_UCHAR) ulID;
        aucID[1] = (D_UCHAR) (ulID >> 8);
        aucID[2] = (D_UCHAR) (ulID >> 16);
        aucID[3] = (D_UCHAR) (ulID >> 24);

        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][1]);
        aucID[4] = (D_UCHAR) ulID;
        aucID[5] = (D_UCHAR) (ulID >> 8);
#endif


#if D_DEBUG
        {
            D_UINT32 *pulID = &NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0];
            D_UINT32 *puID = (D_UINT32 *) &NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0];

            DclPrintf("ID RAM buffer contents:\n");
            DclPrintf("%08lX %08lX %08lX %04x %04X %04X %04X %04X %04X\n",
                      DCLMEMGET32(pulID), DCLMEMGET32(pulID + 1), DCLMEMGET32(pulID + 2),
                      DCLMEMGET16(puID), DCLMEMGET16(puID + 1), DCLMEMGET16(puID + 2),
                      DCLMEMGET16(puID + 3), DCLMEMGET16(puID + 4), DCLMEMGET16(puID + 5));
            DclPrintf("Returning ID bytes %02x %02x %02x %02x %02x %02x\n",
                      aucID[0], aucID[1], aucID[2], aucID[3], aucID[4], aucID[5]);
        }
#endif

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*-------------------------------------------------------------------
    Local: StatusIn()

    Get status data from the NAND chip.

    Transfers a status byte from the NAND chip

    Parameters:
        pucStatus - where to store the status byte

    Return Value:
        TRUE if transfer succeeded, FALSE if controller timed out.
-------------------------------------------------------------------*/
static D_BOOL StatusIn(D_UCHAR *pucStatus)
{
    D_UINT32 uStatus;

    DCLMEMPUT32(NFC_CONFIGURATION1, NFC_CONFIGURATION1_RBA_FIRST);
    DCLMEMPUT32(LAUNCH_NFC, FDO_STATUS);
    
    if (NFCWait())
    {
        uStatus = DCLMEMGET32(NFC_CONFIGURATION1);
#if DCL_BIG_ENDIAN
        switch(NFC.ucWidthBytes)
        {
        case 1:
            /*  The first (only) byte provided by the NAND
                device is in the upper byte (adjust for BE)
            */
            *pucStatus = (D_UCHAR) (uStatus >> 8);
            break;
        case 2:
            /*  The LSB (the only one we need for status) of the
                halfword provided by the NAND device is placed into
                the lower byte of the halfword we read from the RAM buffer
                by the NFC (based on NF_BIG flag), so no
                swapping necessary in x16 configuration
            */
            *pucStatus = (D_UCHAR) (uStatus);
            break;
        default:
            DclError();
            break;
        }
#else
        /*  No swapping necessary, the LSB is in the same
            location in the RAM buffer in LE mode,
            regardless of the width
        */
        *pucStatus = (D_UCHAR) (uStatus >> 16);
#endif

        return TRUE;
    }

    /*  If this times out, this is probably indicitive of a system failure
        (hardware or software) that needs to be debugged, as opposed to one
        of the expected failure mechanisms for NAND flash.
    */
    DclError();
    return FALSE;
}


/*-------------------------------------------------------------------
    Local: PageInHwEcc()

    Transfer and correct a Page Size+Spare Size byte page from the NAND chip.

    Transfer a Page Size-byte main page and (optionally) Spare Size-byte spare area from
    the NAND chip through the controller's RAM buffer to memory and
    perform error detection and correction.

    Note that if the page was written without ECC, the result of reading
    it with ECC is undefined.

    Parameters:
        pMain    - Page Size-byte buffer to receive main page data, aligned
                   on a 4-byte boundary.
        pTag     - Spare Size-byte buffer to receive spare area data, aligned
                   on a 4-byte boundary; may be NULL if spare data is
                   not desired.
        nTagSize - The tag size to use.
        pStatus  - FFXIOSTATUS indicating more detailed information
                   than the simple success or failure return code.
                   The caller must initialize the contents of this
                   structure.

                   On return, pStatus contents will be updated to include
                   an ffxStat value, as well as the flags updated with
                   PAGESTATUS_DATACORRECTED if necessary.  The ulCount
                   will NOT be updated with the number of pages read,
                   since this function cannot know whether it is
                   working with partial pages or full pages.

    Return Value:
        Indicates whether the function failed or succeeded.
-------------------------------------------------------------------*/
static D_BOOL PageInHwEcc(
    void               *pMain,
    void               *pTag,
    unsigned            nTagSize,
    FFXIOSTATUS        *pStatus,
    unsigned            nOffSet)
{
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);
    D_BOOL              uncorrectableError = FALSE;
    
    /*  The tag pointer and tag size are either always both set or both clear
    */
    
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));
    DclAssert(pMain);
    DclMemSet(spare.data, ERASED8, sizeof spare.data);
    
    /*  Transfer the raw page in from the NAND chip.
    */
    if (NFCPageIn(pMain, spare.data))
    {
         D_UINT32 ulEccStatus = DCLMEMGET32(ECC_STATUS_RESULT);
         
        /*  Check for an ECC error in the main page & spare area.
            Do a quick test for no error, this should be true most of the time.
        */
        if (ulEccStatus == ECC_STATUS_NO_ERROR)
        {
            pStatus->ffxStat = FFXSTAT_SUCCESS;
            if(pTag)
            {
                DclAssert(nTagSize == TAG_SIZE);
                DecodeTagHwEcc(pTag, nTagSize, spare.data,nOffSet);
            }
        }
        else
        {
            D_UINT8     i = 0;
            D_UINT32    eccError = 0;
            D_UINT32    ulNumBitErrors = 0;
            
            /*  According to ChipErrat ENGcm10356
                If the number of ECC bit error is equal to 4 (in 4-bit ECC
                mode) or 8 (in 8-bit ECC mode), it should also be treated
                as uncorrectable ECC error because NFC may report the number
                of ECC bit error as 4 (in 4-bit ECC mode) or 8 (in 8-bit
                ECC mode) when an actual uncorrectable ECC error occurs.
            */
            for (i = 0; i < SPARE_SEGMENTS; i++)
            {
                eccError = (ulEccStatus & NFC.ulEccBitMask);

                /*  Cast OK because the bit mask above limits checking
                    to low-order 4 bits only for the MX51 controller.
                */
                if ((eccError) > (D_UINT32)NFC.uEdcSetting)
                {
                    /*  This controller cannot discriminate between ECC
                        errors in the data area vs. spare area.
                    */
                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                        "ntmx51.PageInHwEcc(): Uncorrectable error in page area Or Spare Area.  ECC Status %x\n", DCLMEMGET32(ECC_STATUS_RESULT)));
                    pStatus->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                    NFCClearStatus();
                    uncorrectableError = TRUE;
                    break;
                }
                
                /*  The number of bit errors is always 4 bits/section.
                */
                ulEccStatus >>= NFC_ECC_SECTION_BITS;
                ulNumBitErrors += eccError;
            }
            if (!uncorrectableError)
            {
                /*  A correctable error occurred in the data/spare area.
                    The controller has already corrected the data.
                */
                FfxErrMgrEncodeCorrectedBits(ulNumBitErrors, pStatus);
                pStatus->ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
                pStatus->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                if(pTag)
                {
                    DclAssert(nTagSize == TAG_SIZE);
                    DecodeTagHwEcc(pTag, nTagSize, spare.data,nOffSet);
                }
            }
        }
    }
    else
    {
        pStatus->ffxStat = FFXSTAT_FIMTIMEOUT;
    }

    return ((pStatus->ffxStat == FFXSTAT_SUCCESS) || (pStatus->ffxStat == FFXSTAT_FIMCORRECTABLEDATA));
}


/*-------------------------------------------------------------------
    Local: PageOutHwEcc()

    Build spare area and transfer a Page Size+Spare Size byte page to the NAND chip.

    Transfer a Page Size-byte main page and (optionally) a tag from memory to
    the NAND chip through the controller's RAM buffer.

    Parameters:
        pMain - Page Size-byte buffer to receive main page data, aligned
                on a 4-byte boundary.
        pTag  - buffer containing tag data; may be NULL if tag is
                not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL PageOutHwEcc(
    const void         *pMain,
    const void         *pTag,
    unsigned            nTagSize,
    unsigned            nOffSet)
{
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    DclMemSet(spare.data, ERASED8, sizeof spare.data);

    if(nTagSize)
    {
        DclAssert(pTag);
        EncodeTagHwEcc(pTag, nTagSize, spare.data,nOffSet);
    }

    spare.data[SPARE_INDEX_ECCSTATUS] = ECCSTATUS_ECC;

    return NFCPageOut(pMain, spare.data);
}


/*-------------------------------------------------------------------
    Local: EncodeTagHwEcc()

    Accepts a NULL tag pointer and does nothing.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void EncodeTagHwEcc(
    const D_BUFFER *pTag,
    unsigned        nTagSize,
    D_BUFFER        pSpare[SPARE_BUFFER_SIZE],
    unsigned        nOffSet)
{
    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);

    DclMemCpy(&pSpare[nOffSet], pTag, nTagSize);

    pSpare[nOffSet + TAG_SIZE] = pTag[0] ^ ~pTag[1];

    
    /*  Write FBBSTATUS_FBB_INVALID to indicate ECC has been
        written (which invalidates the factory bad block marking)
        This value is used by GetBlockStatus to distinguish
        between a factory bad block mark and 512B page tags (or
        2KB/4KB page main data), since the factory bad block mark
        indication is overwritten with tag data due to the
        spare area layout.
    */
    pSpare[SPARE_INDEX_FBBSTATUS_XX] = FBBSTATUS_FBB_INVALID;
    
    return;
}


/*-------------------------------------------------------------------
    Local: DecodeTagHwEcc()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL DecodeTagHwEcc(
    D_BUFFER       *pTag,
    unsigned        nTagSize,
    const D_BUFFER  pSpare[SPARE_BUFFER_SIZE],
    unsigned        nOffSet)
{
    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);

    if ((pSpare[nOffSet] ^ (D_BUFFER) ~pSpare[nOffSet + 1])
        == pSpare[nOffSet + TAG_SIZE])
    {
        DclMemCpy(pTag, &pSpare[nOffSet], nTagSize);
        return TRUE;
    }
    else
    {
        static D_BUFFER abZero[TAG_SIZE] = {0};
        DclMemCpy(pTag, abZero, nTagSize);
        return FALSE;
    }
}





/*-------------------------------------------------------------------
    Local: NFCPageIn()

    Transfer a Page Size+Spare Size byte page from the NAND chip to memory

    Transfer a Page Size-byte main page and (optionally) Spare Size-byte spare area from
    the NAND chip through the controller's RAM buffer to memory.

    There is no checking of ECCs or interpretation of the data.

    Parameters:
        pMain  - Page Size-byte buffer to receive main page data.
        pSpare - Spare Size-byte buffer to receive spare area data; may be NULL
                 if spare data is not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCPageIn(void *pMain, void *pSpare)
{
    D_BOOL result = TRUE;
    D_UINT32 aulBounce[16];
    D_UINT32 aulBouncePage[4];
    
    /* We need read one page out.
       FSL NFC does not support partial write
       It alway send out 512+ecc+512+ecc ...
       for large page nand flash. But for small
       page nand flash, it did support SPARE
       ONLY operation. But to make driver
       simple. We take the same as large page,read
       whole page out and update. 
    */

    /*  Trigger the transfer from the NAND chip to the controller's RAM
        buffer.  Writing NAND_FLASH_CONFIG1 with the saved value sets or
        clears ECC_EN as appropriate for the operation.
        Writing LAUNCH_NFC starts the transfer.
    */
    DCLMEMPUT32(NFC_CONFIGURATION1, NFC_CONFIGURATION1_RBA_FIRST);
    NFCClearStatus();
    DCLMEMPUT32(LAUNCH_NFC,FDO_PAGE);

    /*  The controller waits for the NAND chip's Ready/Busy signal to indicate
        that it has data ready to transfer, then transfers it from the chip to
        its internal RAM buffer.  When the entire main page and spare area
        transfer is done, it sets INT in LAUNCH_NFC.
    */
    if (NFCWait())
    {
        D_UINT32 *pCtrlrRam = NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];
        unsigned int i;

        /*  The buffer for main page data may be unaligned.
        */
        if (DCLISALIGNED((D_UINTPTR)pMain, sizeof(D_UINT32)))
        {
            /*  Aligned properly, copy directly into it.
            */
            D_UINT32 *pBuffer = pMain;
            for (i = 0; i < (MAIN_BUFFER_SIZE/sizeof (D_UINT32)); i += 4)
            {
                pBuffer[i+0] = DCLMEMGET32(pCtrlrRam + (i+0));
                pBuffer[i+1] = DCLMEMGET32(pCtrlrRam + (i+1));
                pBuffer[i+2] = DCLMEMGET32(pCtrlrRam + (i+2));
                pBuffer[i+3] = DCLMEMGET32(pCtrlrRam + (i+3));
            }
        }
        else
        {
            /*  Unaligned, use a local bounce buffer to align it.
            */
            D_UCHAR *pBuffer = pMain;
            for (i = 0; i < (MAIN_BUFFER_SIZE/sizeof (D_UINT32)); i += 4)
            {
                aulBouncePage[0] = DCLMEMGET32(pCtrlrRam + (i+0));
                aulBouncePage[1] = DCLMEMGET32(pCtrlrRam + (i+1));
                aulBouncePage[2] = DCLMEMGET32(pCtrlrRam + (i+2));
                aulBouncePage[3] = DCLMEMGET32(pCtrlrRam + (i+3));
                DclMemCpy(pBuffer + i * sizeof (D_UINT32), aulBouncePage, sizeof aulBouncePage);
            }
        }

        if (pSpare)
        {
            D_UINT8 ii=0,jj=0;
            D_UINT8 sparebufIndex =0, rambufIndex = 0;

            pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];
            /*  The buffer for spare data may be unaligned.
            */
            if (DCLISALIGNED((D_UINTPTR)pSpare, sizeof(D_UINT32)))
            {
                /*  Aligned properly, copy directly into it.
                */
                D_UINT32 *pBuffer = pSpare;

                for(ii = 0; ii < (SPARE_BUFFER_SIZE/sizeof (D_UINT32)) ; ii += 4)
                {
                    for(jj = 0; jj < 4 ; jj++)
                    {
                        pBuffer[sparebufIndex] = DCLMEMGET32(pCtrlrRam + rambufIndex);
                        sparebufIndex = sparebufIndex + 1;
                        rambufIndex = rambufIndex + 1;
                    }
                    rambufIndex = rambufIndex + 12;
                }
            }
            else
            {
                /*  Unaligned, use a local bounce buffer to align it.
                */
                for(ii = 0; ii < (SPARE_BUFFER_SIZE/sizeof (D_UINT32)) ; ii += 4)
                {
                    for(jj = 0; jj < 4 ; jj++)
                    {
                        aulBounce[sparebufIndex] = DCLMEMGET32(pCtrlrRam + rambufIndex);
                        sparebufIndex = sparebufIndex + 1;
                        rambufIndex = rambufIndex + 1;
                    }
                    rambufIndex = rambufIndex + 12;
                }
                DclMemCpy(pSpare, aulBounce, sizeof aulBounce);
            }
        }

        result = TRUE;
    }
    else
    {
        FFXPRINTF(1, ("NFCPageIn: NFC timeout\n"));

        /*  Who knows what state of the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  If the NAND chip itself timed out on a read, it may need to be
            reset to return to a usable state.  If this fails, there's not
            much to be done for it here, so ignore the status of this
            operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }

    return result;
}


/*-------------------------------------------------------------------
    Local: NFCPageOut()
 
    Transfer a Page Size+Spare Size byte page from memory to the NAND chip

    Transfer a Page Size-byte main page and (optionally) Spare Size-byte spare area from
    memory to the NAND chip through the controller's RAM buffer.

    There is no processing of the data.  ECC generation may or may not
    occur, depending on how the transfer was set up outside this function.

    Parameters:
        pMain  - Page Size-byte buffer containing the main page data.
        pSpare - Spare Size-byte buffer containing the spare area data; may be
                 NULL if spare data is not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCPageOut(const void *pMain, const void *pSpare)
{
    D_UINT8 ii = 0,jj = 0;
    D_UINT8 sparebufIndex = 0, rambufIndex = 0;
    D_BOOL result = TRUE;
    D_UINT32 LoopCount = 0xFFFFFF;
    D_UINT32 *pCtrlrRam;
    D_UINT32 aulBounce[16];
    D_UINT32 aulBouncePage[4];
    unsigned int i;

    pCtrlrRam = NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];

    /*  The buffer for main page data may be unaligned.
    */
    if (DCLISALIGNED((D_UINTPTR)pMain, sizeof(D_UINT32)))
    {
        /*  Aligned properly, copy directly from it.
        */
        const D_UINT32 *pBuffer = pMain;
        for (i = 0; i < (MAIN_BUFFER_SIZE/sizeof (D_UINT32)); i += 4)
        {
            DCLMEMPUT32(pCtrlrRam + (i+0), pBuffer[i+0]);
            DCLMEMPUT32(pCtrlrRam + (i+1), pBuffer[i+1]);
            DCLMEMPUT32(pCtrlrRam + (i+2), pBuffer[i+2]);
            DCLMEMPUT32(pCtrlrRam + (i+3), pBuffer[i+3]);
        }
    }
    else
    {
        /*  Unaligned, use a local bounce buffer to align it.
        */
        const D_UCHAR *pBuffer = pMain;
        for (i = 0; i < (MAIN_BUFFER_SIZE/sizeof (D_UINT32)); i += 4)
        {
            DclMemCpy(aulBouncePage, pBuffer + i * sizeof (D_UINT32), sizeof aulBouncePage);
            DCLMEMPUT32(pCtrlrRam + (i+0), aulBouncePage[0]);
            DCLMEMPUT32(pCtrlrRam + (i+1), aulBouncePage[1]);
            DCLMEMPUT32(pCtrlrRam + (i+2), aulBouncePage[2]);
            DCLMEMPUT32(pCtrlrRam + (i+3), aulBouncePage[3]);
        }
    }

    if (!pSpare)
    {
        /*  The caller did not supply spare data, use our own spare buffer.
        */
        pSpare = &UnusedSpare.data;
    }
    pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];
    
    /* We need to mark this page so we'll know it never had a factory BBI.
    */
    ((D_BUFFER *)pSpare)[SPARE_INDEX_FBBSTATUS_XX] = FBBSTATUS_FBB_INVALID;
    
    /*  The buffer for spare data may be unaligned.
    */
    if (DCLISALIGNED((D_UINTPTR)pSpare, sizeof(D_UINT32)))
    {
        /*  Aligned properly, copy directly from it.
        */
        const D_UINT32 *pBuffer = pSpare;
        for(ii = 0; ii < (SPARE_BUFFER_SIZE/sizeof (D_UINT32)) ; ii += 4)
        {
            for(jj = 0; jj < 4 ; jj++)
            {
                DCLMEMPUT32(pCtrlrRam + rambufIndex,pBuffer[sparebufIndex]);
                sparebufIndex = sparebufIndex + 1;
                rambufIndex = rambufIndex + 1;
            }
            rambufIndex = rambufIndex + 12;
        }
    }
    else
    {
        /*  Unaligned, use a local bounce buffer to align it.
        */
        DclMemCpy(aulBounce, pSpare, sizeof aulBounce);

        for(ii = 0; ii < (SPARE_BUFFER_SIZE/sizeof (D_UINT32)) ; ii += 4)
        {
            for(jj = 0; jj < 4 ; jj++)
            {
                DCLMEMPUT32(pCtrlrRam + rambufIndex,aulBounce[sparebufIndex]);
                sparebufIndex = sparebufIndex + 1;
                rambufIndex = rambufIndex + 1;
            }
            rambufIndex = rambufIndex + 12;
        }
    }
    /*  Trigger the transfer from the controller's RAM buffer to the NAND
        chip.  Writing NAND_FLASH_CONFIG1 with the saved value sets or clears
        ECC_EN as appropriate for the operation.
        Writing LAUNCH_NFC starts the transfer.
    */
    DCLMEMPUT32(NFC_CONFIGURATION1, NFC_CONFIGURATION1_RBA_FIRST);
    NFCClearStatus();   
    DCLMEMPUT32(LAUNCH_NFC,FDI);
    if (!NFCWait())
    {
        FFXPRINTF(1, ("NFCPageOut: NFC timeout\n"));

        /*  Who knows what state the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  The NAND chip is in the middle of some sort of write command
            sequence, and needs to be reset to be reset to return to a usable
            state.  If this fails, there's not much to be done for it here, so
            ignore the status of this operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }
    /* Wait till NAND is idle 
    */
    while(LoopCount)
    {
        LoopCount--;
        if(DCLMEMGET32(NFC_IPC) & (NFC_IPC_RB_B))
            break;
    }
    if (!LoopCount)
    { 
        DclError();
        DclPrintf("            Write is UnSucssesFull.. The Device is Really Busy..\n");
        result = FALSE;
    }
    /*while (!(DCLMEMGET32(NFC_IPC) & NFC_IPC_RB_B));*/

    return result;
}


/*-------------------------------------------------------------------
    Local: NFCSpareIn()

    Transfer a Spare Size-byte spare area from the NAND chip through the
    controller's RAM buffer to memory.

    There is no checking of ECCs or interpretation of the data.

    Parameters:
        pSpare - Spare Size-byte buffer to receive spare area data, aligned
                 on a 4-byte boundary; may be NULL if spare data is
                 not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCSpareIn(void *pSpare)
{
    D_BOOL result = TRUE;  

    /*  Trigger the transfer from the NAND chip to the controller's RAM
        buffer.  Writing NAND_FLASH_CONFIG1 with the saved value sets or
        clears ECC_EN as appropriate for the operation.
        Writing LAUNCH_NFC starts the transfer.
    */

    DCLMEMPUT32(NFC_CONFIGURATION1, NFC_CONFIGURATION1_RBA_FIRST);
    NFCClearStatus();
    DCLMEMPUT32(LAUNCH_NFC,FDO_PAGE);
    
    /*  The controller waits for the NAND chip's Ready/Busy signal to indicate
        that it has data ready to transfer, then transfers it from the chip to
        its internal RAM buffer.  When the entire main page and spare area
        transfer is done, it sets INT in LAUNCH_NFC.
    */

    if (NFCWait())
    {
        
        D_UINT8 ii = 0,jj = 0;
        D_UINT8 sparebufIndex =0, rambufIndex = 0;
        D_UINT32 *pBuffer = pSpare;
        D_UINT32 *pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];

        for(ii = 0; ii < (SPARE_BUFFER_SIZE/sizeof (D_UINT32)) ; ii += 4)
        {
            for(jj = 0; jj < 4 ; jj++)
            {
                pBuffer[sparebufIndex] = DCLMEMGET32(pCtrlrRam + rambufIndex);
                sparebufIndex = sparebufIndex + 1;
                rambufIndex = rambufIndex + 1;
            }
            rambufIndex = rambufIndex + 12;
        }
        result = TRUE;
    }
    else
    {
        FFXPRINTF(1, ("NFCSpareIn: NFC timeout\n"));

        /*  Who knows what state the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  If the NAND chip itself timed out on a read, it may need to be
            reset to return to a usable state.  If this fails, there's not
            much to be done for it here, so ignore the status of this
            operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }

    return result;
}


/*-------------------------------------------------------------------
    Local: NFCSpareOut()

    Transfer a Spare Size-byte spare area from memory to the NAND chip through the
    controller's RAM buffer.

    There is no processing of the data.  ECC generation may or may not
    occur, depending on how the transfer was set up outside this function.

    Parameters:
        pSpare - Spare Size-byte buffer to receive spare area data, aligned
                 on a 4-byte boundary.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCSpareOut(const void *pSpare)
{
    D_BOOL result = TRUE;
    const D_UINT32 *pBuffer;
    D_UINT32 *pCtrlrRam;
    D_UINT8 ii = 0,jj = 0;
    D_UINT8 sparebufIndex =0, rambufIndex = 0;    
    D_UINT32 LoopCount = 0xFFFFFF;
    
    pBuffer = pSpare;
    pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];
    /* Mark this page so we'll know it never had a FBBI. 
    */
    ((D_BUFFER *)pSpare)[SPARE_INDEX_FBBSTATUS_XX] = FBBSTATUS_FBB_INVALID;
    
    for(ii = 0; ii < (SPARE_BUFFER_SIZE/sizeof (D_UINT32)) ; ii += 4)
    {
        for(jj = 0; jj < 4 ; jj++)
        {
            DCLMEMPUT32(pCtrlrRam + rambufIndex,pBuffer[sparebufIndex]);
            sparebufIndex = sparebufIndex + 1;
            rambufIndex = rambufIndex + 1;
        }
        rambufIndex = rambufIndex + 12;
    }

    /*  Trigger the transfer from the controller's RAM buffer to the NAND
        chip.  Writing NAND_FLASH_CONFIG1 with the saved value sets or clears
        ECC_EN as appropriate for the operation. 
        Writing LAUNCH_NFC starts the transfer.
    */
    DCLMEMPUT32(NFC_CONFIGURATION1, NFC_CONFIGURATION1_RBA_FIRST);
    NFCClearStatus();
    DCLMEMPUT32(LAUNCH_NFC,FDI);

    if (!NFCWait())
    {
        FFXPRINTF(1, ("NFCSpareOut: NFC timeout\n"));

        /*  Who knows what state the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  The NAND chip is in the middle of some sort of write command
            sequence, and needs to be reset  to return to a usable
            state.  If this fails, there's not much to be done for it here, so
            ignore the status of this operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }
    
    /* Wait till NAND is idle 
    */
    while(LoopCount)
    {
        LoopCount--;
        if(DCLMEMGET32(NFC_IPC) & (NFC_IPC_RB_B))
            break;
    }
    if (!LoopCount)
    { 
        /*DclAssert(FALSE);*/
        DclError();
        DclPrintf("            Write is UnSucssesFull.. The Device is Busy..\n");
        result = FALSE;
    }
    /*while (!(DCLMEMGET32(NFC_IPC) & NFC_IPC_RB_B));*/
    return result;
    
}


/*-------------------------------------------------------------------
    Local: NFCCommandOut()

    Send a command byte to the NAND chip through the controller,
    and wait for it to be accepted.

    Parameters:
        uCommand - the command byte to send

    Return Value:
        TRUE if the controller accepted the command; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCCommandOut(D_UCHAR uCommand)
{

    DCLMEMPUT32(NFC_FLASH_CMD, uCommand);
    NFCClearStatus();
    DCLMEMPUT32(LAUNCH_NFC,FCMD);

    return NFCWait();
}


/*-------------------------------------------------------------------
    Local: NFCAddressOut()

    Split an address into individual bytes and send them to the NAND chip.

    Parameters:
        ulAddress - the address to send
        uCount    - the number of address cycles to generate, must not
                    be zero

    Return Value:
        TRUE if the controller accepted the address; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCAddressOut(D_UINT32 ulAddress, unsigned uCount)
{
    DclAssert(uCount != 0);
    DclAssert(uCount <= sizeof(ulAddress));

    do
    {
        DCLMEMPUT32(NAND_ADD_LOW, (D_UINT16) ulAddress);       
        NFCClearStatus();
        DCLMEMPUT32(LAUNCH_NFC,FADD);
        ulAddress >>= 8;
        if (!NFCWait())
            break;
    }
    while (--uCount);

    return (uCount == 0);
}

/*-------------------------------------------------------------------
    Local: NFCRowOut()

    Send a row address to the NAND chip.

    Parameters:
        ulRow  - the ulRow address to send
        uCount - the number of row address cycles to generate,
                 must not be zero

    Return Value:
        TRUE if the controller accepted the address; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCRowOut(D_UINT32 ulRow, unsigned uCount)
{
    DclAssert(uCount != 0);
    return NFCAddressOut(ulRow, uCount);
}

/*-------------------------------------------------------------------
    Local: NFCColumnOut()

    Send a column address to the NAND chip.

    If necessary, shift the column address to account for the
    NAND chip width, then send the address out.

    Parameters:
        ulColumn - the column address to send
        uCount   - the number of column address cycles to generate,
                   must not be zero

    Return Value:
        TRUE if the controller accepted the address; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCColumnOut(D_UINT32 ulColumn, unsigned uCount)
{
    D_UINT32 ulAddress;
    DclAssert(uCount != 0);

    /*  Convert to width-specific column address
        (column addresses are either byte index or word index,
        depending on the bus width)
    */
    ulAddress = ulColumn >> (NFC.ucWidthBytes - 1);

    return NFCAddressOut(ulAddress, uCount);
}

/*-------------------------------------------------------------------
    Local: NFCPageAddressOut()

    Split a page address into individual bytes and send them to the NAND chip.

    Parameters:
        ulPage  - the page address to send
        uCount  - the number of address cycles to generate, must not be zero

    Return Value:
        TRUE if the controller accepted the address;
        FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCPageAddressOut(D_UINT32 ulPage, unsigned uCount)
{
    DclAssert(uCount != 0);
    DclAssert(uCount <= sizeof(ulPage));

    /* We know the column address is zero, except for ERASE*/
    NFCColumnOut(MX51_PAGE_COLUMN, 2);  /* Page address of Page Size bytes of native page area */
#if FFXCONF_NAND_4KBNATIVE
    /* TPP: For our Micron NAND device the upper 13 bits of the page should be zero.*/
    ulPage &= 0x0007FFFF;
#endif
    do
    {
        DCLMEMPUT32(NAND_ADD_LOW, (D_UINT16) ulPage);
        NFCClearStatus();
        DCLMEMPUT32(LAUNCH_NFC, FADD);
        ulPage >>= 8;

        if (!NFCWait())
            break;
    }
    while (--uCount);

    return (uCount == 0);
}


/*-------------------------------------------------------------------
    Local: NFCBlockAddressOut()

    Split a page address into individual bytes and send them to the NAND chip.

    Parameters:
        ulPage  - the page address to send
        uCount  - the number of address cycles to generate, must not be zero

    Return Value:
        TRUE if the controller accepted the address;
        FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCBlockAddressOut(D_UINT32 ulPage, unsigned uCount)
{
    DclAssert(uCount != 0);
    DclAssert(uCount <= sizeof(ulPage));
#if FFXCONF_NAND_4KBNATIVE
    /* TPP: For our Micron NAND device the upper 13 bits of the page should be zero.*/
    ulPage &= 0x0007FFFF;
#endif
    do
    {
        DCLMEMPUT32(NAND_ADD_LOW, (D_UINT16) ulPage);
        NFCClearStatus();
        DCLMEMPUT32(LAUNCH_NFC, FADD);
        ulPage >>= 8;

        if (!NFCWait())
            break;
    }
    while (--uCount);

    return (uCount == 0);
}

/*-------------------------------------------------------------------
    Local: NFCReadyWait()

    Wait for the controller to indicate completion of an operation that
    requires waiting for the NAND chip to indicate is is ready.  These
    operations typically take long enough (tens to thousands of
    microseconds) for it to be worth setting a timer.

    Parameters:
        None

    Return Value:
        TRUE if the chip operation finished; FALSE if this timed out.
          
-------------------------------------------------------------------*/
static D_BOOL NFCReadyWait(void)
{
    DCLTIMER t;
    D_UINT32 ulReadyWaitLoops = 0;
    D_UINT32 ulFlashConfig2Val;
    D_UINT32 sulReadyWaitLoopsMax = 0;
    D_UINT32 sulReadyWaitEntries = 0;

    sulReadyWaitEntries++;

    DclTimerSet(&t, NAND_TIMEOUT);
    do
    {
        ulFlashConfig2Val = DCLMEMGET32(NFC_IPC);        
        ulFlashConfig2Val = (ulFlashConfig2Val & NFC_OPS_STAT_INT);
        if ((ulFlashConfig2Val & NFC_OPS_STAT_INT) != 0)
        {
            /*  Per the specification, the operation bits should be cleared
                when INT is set.  Sanity check that here.
            */
            DclAssert(ulFlashConfig2Val == NFC_OPS_STAT_INT);
            return TRUE;
        }
        ulReadyWaitLoops++;
        if (ulReadyWaitLoops > sulReadyWaitLoopsMax)
        {
            sulReadyWaitLoopsMax = ulReadyWaitLoops;
        }


    }    
    while (!DclTimerExpired(&t));
    ulFlashConfig2Val = DCLMEMGET32(NFC_IPC);    
    ulFlashConfig2Val = (ulFlashConfig2Val & NFC_OPS_STAT_INT);
    if ((ulFlashConfig2Val & NFC_OPS_STAT_INT) != 0)
    {
        /*  Per the specification, the operation bits should be cleared
            when INT is set.  Sanity check that here.
        */
        DclAssert(ulFlashConfig2Val == NFC_OPS_STAT_INT);
        return TRUE;
    }

    FFXPRINTF(1, ("NFCReadyWait: timeout\n"));
    /*  Generally speaking, if NAND flash operations are timing out,
        something is wrong with the code or the system (rather than one
        of the expected failure cases for NAND) and needs to be debugged.
    */
    /*DclAssert(FALSE);*/
    DclError();
    return FALSE;
}



/*-------------------------------------------------------------------

    Local: NFCWait()

    Wait for the controller to indicate completion of an operation.

    Note that depending on circumstances and OS overhead, this function
    may exit immediately or could possibly wait up to several milliseconds.

    We use a three-stage approach to reduce latency and overhead,
    but the three stages are based on assumptions that must be
    tuned or validated in any particular system in order to be of
    the most benefit.  Note this function will behave correctly
    regardless of any tuning, and the difference between the
    worst case and best case is comparitively small, but
    relevant (10% for writes on the system I tested it on.)

    Parameters:
        None.

    Return Value:
        TRUE if the controller operation finished; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCWait(void)
{
    D_UINT32 ulFlashConfig2Val;
    D_UINT32 sulNFCWaitLoopsMax = 0;
    D_UINT32 sulNFCWaitEntries = 0;
    D_UINT32 sulNFCWaitShorts = 0;
    
    sulNFCWaitEntries++;

    {
        /*  Stage one:  Just check it.  If it's already set, don't bother
            with the loops.
        */
        ulFlashConfig2Val = DCLMEMGET32(NFC_IPC);        
        ulFlashConfig2Val = (ulFlashConfig2Val & NFC_OPS_STAT_INT);
        if ((ulFlashConfig2Val & NFC_OPS_STAT_INT) != 0)       
        {
            /*  Per the specification, the operation bits should be cleared
                when INT is set.  Sanity check that here.
            */
            sulNFCWaitShorts++;
            DclAssert(ulFlashConfig2Val == NFC_OPS_STAT_INT);            
            NFCIPCEnable();
            /* Clear the NANDFC interrupt
            */
            DCLMEMPUT32(NFC_IPC,DCLMEMGET32(NFC_IPC)&~(NFC_OPS_STAT_INT));
            NFCIPCDisable();
            return TRUE;
        }
        
    }

    {
        /*  Stage two: Check it in a counted loop.  If it gets set quickly,
            don't bother with the loop that calls into the OS for timer info
        */

        /*  Note this implementation auto-tunes the wait loop for best
            performance, entering the timed loop only until the count is
            calibrated, and only thereafter on a worst-case or catestrophic
            failure.
        */
        long timeout = sulNFCWaitLoopsMax;
        D_UINT32 ulNFCWaitLoops = 0;

        do
        {
            ulFlashConfig2Val = DCLMEMGET32(NFC_IPC);           
            ulFlashConfig2Val = (ulFlashConfig2Val & NFC_OPS_STAT_INT);            
            if ((ulFlashConfig2Val & NFC_OPS_STAT_INT) != 0)
            {
                /*  Per the specification, the operation bits should be cleared
                    when NFC_OPS_STAT_INT  is set.  Sanity check that here.
                */
                DclAssert(ulFlashConfig2Val == NFC_OPS_STAT_INT);
                NFCIPCEnable();
                /* Clear the NANDFC interrupt
                */
                DCLMEMPUT32(NFC_IPC,DCLMEMGET32(NFC_IPC)&~(NFC_OPS_STAT_INT));
                NFCIPCDisable();
                return TRUE;
            }
            ulNFCWaitLoops++;
            if (ulNFCWaitLoops > sulNFCWaitLoopsMax)
            {
                sulNFCWaitLoopsMax = ulNFCWaitLoops;
            }
        }
        while (--timeout);
        
    }

    {
        /*  Stage three: it hasn't gotten set for 'a while' now,
            assume it's not going to be set for 'a longer while',
            and use a timed loop to get a valid, CPU clock-independant
            timed wait.
        */
        if (NFCReadyWait())
        {
            NFCIPCEnable();            
            /* Clear the NANDFC interrupt
            */
            DCLMEMPUT32(NFC_IPC,DCLMEMGET32(NFC_IPC)&~(NFC_OPS_STAT_INT));
            NFCIPCDisable();
            return TRUE;
        }
    }
    FFXPRINTF(1, ("NFCWait: timeout\n"));

    /*  Generally speaking, if operations time out, there is something
        wrong with the system (as opposed to the specific expected failures
        of operations on NAND flash) that needs to be debugged.
    */
    /*DclAssert(FALSE);*/
    DclError();
    return FALSE;
}


#if NFC_HAS_RESET
/*-------------------------------------------------------------------
    Local: NFCReset()

    Reset the NFC itself (as opposed to the NAND chip).

    Parameters:
        None

    Return Value:
        None.
-------------------------------------------------------------------*/
static void NFCReset(void)
{
    DCLMEMPUT32(NFC_FLASH_CMD, CMD_RESET);    
    NFCClearStatus();
    DCLMEMPUT32(LAUNCH_NFC,FCMD);
    NFCWait();
}
#endif


/*-------------------------------------------------------------------
    Local: SetEccMode()

    To Enable or Disable Harware ECC

    Parameters:

        fUseEcc

    Return Value:

        None.
-------------------------------------------------------------------*/
static void SetEccMode(D_BOOL fUseEcc)
{
    NFCIPCEnable();
    if (fUseEcc)
    {  
        NFC_IP.uConfig2 |= NFC_CONFIGURATION2_ECC_EN;
        DCLMEMPUT32(NFC_CONFIGURATION2,NFC_IP.uConfig2);
        
    }
    else
    {
        NFC_IP.uConfig2 &= ~NFC_CONFIGURATION2_ECC_EN;
        DCLMEMPUT32(NFC_CONFIGURATION2,NFC_IP.uConfig2);        
    }
    NFCIPCDisable();
}


/*-------------------------------------------------------------------
    Local: NFCIPCEnable()

    Parameters:
        None

    Return Value:
        None.
-------------------------------------------------------------------*/
static void NFCIPCEnable(void)
{   
    DCLREG32BF(NFC_IPC, NFC_NFC_IPC_CREQ, NFC_NFC_IPC_CREQ_REQUEST);
    while (!(DCLMEMGET32(NFC_IPC) & DLBITFMASK(NFC_NFC_IPC_CACK)));
    return;
}


/*-------------------------------------------------------------------
    Local: NFCIPCDisable()

    Parameters:
        None

    Return Value:
        None.
-------------------------------------------------------------------*/
static void NFCIPCDisable(void)
{
    DCLREG32BF(NFC_IPC, NFC_NFC_IPC_CREQ, NFC_NFC_IPC_CREQ_NO_REQUEST);
    return;
}


/*-------------------------------------------------------------------
    Local: NFCClearStatus()

    Parameters:
        None

    Return Value:
        None.
-------------------------------------------------------------------*/
static void NFCClearStatus(void)
{
    DCLMEMPUT32(NFC_IPC,0x0);
    DCLMEMPUT32(ECC_STATUS_SUM,0x0);

    return;
}
/*-----------------------------------------------------------------------------

  Function: SetCs

  This function sets NFC to certain cs

  Parameters:
      dwCs:   which cs is needed to set.

  Returns:
      None
-----------------------------------------------------------------------------*/
static void SetCs(unsigned int dwCs)
{
    static unsigned int m_cs = 0;
    
    if(m_cs != dwCs && dwCs < 8)
    {
        DCLREG32BF(NFC_CONFIGURATION1,
            NFC_NFC_CONFIGURATION1_CS, (NFC_NFC_CONFIGURATION1_CS_ACTIVE_CS0 + dwCs));
        m_cs = dwCs;
    }
}

/*-------------------------------------------------------------------
    Local: NFCAutoLaunch()

    Parameters:
        None

    Return Value:
        None.
-------------------------------------------------------------------*/

#if D_DEBUG
static void NFCAutoLaunch(unsigned oPeration)
{
    DCLMEMPUT32(LAUNCH_NFC,oPeration);

    return;
}

#endif

#endif /* FFXCONF_NANDSUPPORT */
