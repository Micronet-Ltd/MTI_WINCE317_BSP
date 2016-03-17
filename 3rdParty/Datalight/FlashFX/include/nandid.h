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

    Data structures for NAND ID recognition

    The process of recognizing NAND chip ID information is now table driven.
    At the top level is an array of FFXNANDMFGs.  Each Manufacturer has its
    manufacturer ID value, a manufacturer name string, and a pointer to an
    array of FFXNANDCHIPs.

    Each Chip similarly has a device ID value and a device name string, and
    a pointer to a FFXNANDCHIPCLASS that describes the chip.

    The ChipClass represents a class of chips that have the same total size,
    page size, and command set.  It also has some special-case characteristics
    that differentiate otherwise very similar parts (for example, special
    handling for chip errata).

    Note that the arrays of FFXNANDMFGs and FFXNANDCHIPs are searched linearly
    (they are small, and this is infrequent).  There is no need to have the
    entries in any particular order, but they are generally sorted so that
    it's convenient for people to look for an ID and avoid duplicating an
    entry.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandid.h $
    Revision 1.9  2011/11/22 18:14:01Z  glenns
    Add a chip class to support 16-bit wide devices with 8KB pages.
    Revision 1.8  2010/09/28 21:32:09Z  glenns
    Change revision comment in previous checkin pending product
    release.
    Revision 1.7  2010/06/09 02:36:56Z  glenns
    Add chip ID for [TRADE NAME TBA].
    Revision 1.6  2009/12/03 00:38:39Z  garyp
    Changed STMicro references to Numonyx.
    Revision 1.5  2009/08/04 03:13:52Z  garyp
    Fixed a broken structure field name.
    Revision 1.4  2009/04/09 20:46:42Z  keithg
    Added nominal device configurations for use under simulation and test.
    Revision 1.3  2009/03/25 18:21:35Z  glenns
    Removed unnecessary cautionary comments about unused macros.
    Revision 1.2  2009/03/24 09:12:23Z  glenns
    Fix Bugzilla #2540. See report in Bugzilla for details.
    Revision 1.1  2009/02/06 02:13:30Z  keithg
    Initial revision
    Revision 1.19  2009/01/28 13:44:03  glenns
    Added a flag to indicate last-page-in-block factory bad-block marking, and
    updated macros to initialize the flag.
    Revision 1.18  2009/01/26 22:38:56Z  glenns
    Modified FFXNANDCHIPCLASS structure to use variable names that meet 
    Datalight coding standards.
    Revision 1.17  2009/01/24 00:54:27Z  billr
    Update copyright date.
    Revision 1.16  2009/01/24 00:21:57Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Add
    fProgramOnce to FFXNANDCHIPCLASS structure.
    Revision 1.15  2008/12/04 14:52:34Z  thomd
    Added useful define for LFA customers
    Revision 1.14  2008/06/16 13:20:55Z  thomd
    Renamed ChipClass field to match higher levels
    Revision 1.13  2008/06/11 13:12:10Z  thomd
    Extend nand definitions with default values for EDC,
    BBM, and erase cycle ratings
    Revision 1.12  2008/05/23 20:40:04Z  billr
    Resolve bug 1984: FXCONF_NANDSUPPORT_SAMSUNG and
    FFXCONF_NANDSUPPORT_SPANSION are swapped.
    Revision 1.11  2008/01/30 22:20:35Z  Glenns
    Added flag to chip special characteristics indicating support for a
    block locking mechanism.
    Revision 1.10  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/09/12 22:34:07Z  Garyp
    Added manufacturer specific conditions.  Added support for Micron dual-
    plane operations.  Removed dead-code.
    Revision 1.8  2007/06/22 06:23:44Z  timothyj
    Added column to table indicating whether chips support 0x31 command for
    no-seek partial page program operations (ORNAND).
    Added several ORNAND parts to the table.
    Revision 1.7  2007/02/28 20:47:46Z  timothyj
    Modified table and macros that load it to store the number of
    blocks in lieu of the device size, for LFA compatibility (Tables can
    now accomodate devices larger than 32Gb).
    Revision 1.6  2007/01/03 23:56:28Z  Timothyj
    IR #777, 778, 681: Modified FfxNandDecodeId() to allow for
    devices such as ST Micro NAND04GW3C2AN1 and
    Spansion OrNAND, which have characteristics that are
    not common among all devices.  Allows the use of additional
    bytes in the ID string.
    Added OneNAND decode function (moved from static
    implementation in OneNAND NTM), as it conforms to the
    above method and could therefore be used by other OneNAND
    NTMs in addition to nt1nand.c.
    IR #809: Renamed AMD to Spansion.
    Revision 1.5  2006/03/14 02:04:27Z  garyp
    Changed "1st2pages" terminology to "OffsetZero" for clarity.
    Revision 1.4  2006/03/07 20:45:11Z  Garyp
    Changed the CHIPFBB_* settings to be an enumeration rather than a
    bitmask.
    Revision 1.3  2006/03/07 01:58:03Z  timothyj
    Added Chip Flags for 1KB OneNAND parts
    Revision 1.2  2006/03/03 20:26:52Z  Garyp
    Updated the FFXNANDCHIP structure to include a status byte containing
    the interface width, 8 or 16 bit, and the style of factory bad block marking.
    Revision 1.1  2005/12/03 02:58:56Z  Pauli
    Initial revision
    Revision 1.3  2005/12/03 02:58:56Z  Garyp
    Modified the FFXNANDCHIPCLASS structure to contain a "spare-format" field.
    Revision 1.2  2005/10/30 04:21:37Z  Garyp
    Added spare size to the FFXNANDCHIPCLASS structure.
    Revision 1.1  2005/10/02 02:58:42Z  Garyp
    Initial revision
    Revision 1.3  2005/08/31 22:58:30Z  Garyp
    Added the DECLARE_NAND_CHIP and DECLARE_NAND_MFG macros as well as the
    VERBOSE_CHIP_INFO to reduce run-time image sizes.
    Revision 1.2  2005/08/21 04:52:09Z  garyp
    Eliminated // comments.
    Revision 1.1  2005/07/29 17:14:50Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef NANDID_H_INCLUDED
#define NANDID_H_INCLUDED


#define NAND_ID_SIZE (6)

/*  At some point these will probably be defined in ffxconf.h, but for now...
*/
#ifndef FFXCONF_NANDSUPPORT_HYNIX
  #define FFXCONF_NANDSUPPORT_HYNIX     TRUE
#endif
#ifndef FFXCONF_NANDSUPPORT_MICRON
  #define FFXCONF_NANDSUPPORT_MICRON    TRUE
#endif
#ifndef FFXCONF_NANDSUPPORT_ONENAND
  #define FFXCONF_NANDSUPPORT_ONENAND   TRUE
#endif
#ifndef FFXCONF_NANDSUPPORT_SAMSUNG
  #define FFXCONF_NANDSUPPORT_SAMSUNG   TRUE
#endif
#ifndef FFXCONF_NANDSUPPORT_SPANSION
  #define FFXCONF_NANDSUPPORT_SPANSION  TRUE
#endif
#ifndef FFXCONF_NANDSUPPORT_NUMONYX
  #define FFXCONF_NANDSUPPORT_NUMONYX   TRUE
#endif
#ifndef FFXCONF_NANDSUPPORT_TOSHIBA
  #define FFXCONF_NANDSUPPORT_TOSHIBA   TRUE
#endif
#ifndef FFXCONF_NANDSUPPORT_DLTEST
  #define FFXCONF_NANDSUPPORT_DLTEST    FALSE
#endif

#if D_DEBUG
  #define VERBOSE_CHIP_INFO  TRUE         /* TRUE for checked in code */
#else
  #define VERBOSE_CHIP_INFO  FALSE        /* FALSE for checked in code */
#endif

/*  Utility macros for specifying chip ID sequences.
*/
#define ID_DONT_CARE 0xFF
#define ID_1(id1) { (id1), (ID_DONT_CARE), (ID_DONT_CARE), (ID_DONT_CARE), (ID_DONT_CARE) }
#define ID_2(id1, id2) { (id1), (id2), (ID_DONT_CARE), (ID_DONT_CARE), (ID_DONT_CARE) }
#define ID_3(id1, id2, id3) { (id1), (id2), (id3), (ID_DONT_CARE), (ID_DONT_CARE) }
#define ID_4(id1, id2, id3, id4) { (id1), (id2), (id3), (id4), (ID_DONT_CARE) }
#define ID_5(id1, id2, id3, id4, id5) { (id1), (id2), (id3), (id4), (id5) }

#if VERBOSE_CHIP_INFO
  #define DECLARE_NAND_CHIP(id_len, idarray, clas, flg, nam)  { (id_len), idarray, (flg), (nam), (clas) }
  #define DECLARE_NAND_MFG(id, pfnCreate, nam)   { (id), (pfnCreate), (nam) }
#else
  #define DECLARE_NAND_CHIP(id_len, idarray, clas, flg, nam)  { (id_len), idarray, (flg), (clas) }
  #define DECLARE_NAND_MFG(id, pfnCreate, nam)   { (id), (pfnCreate) }
#endif




/*-------------------------------------------------------------------
    NAND manufacturer IDs -- in alphabetical order...
-------------------------------------------------------------------*/
#if FFXCONF_NANDSUPPORT_HYNIX
  #define NAND_MFG_HYNIX            (0xAD)
#endif
#if FFXCONF_NANDSUPPORT_MICRON
  #define NAND_MFG_MICRON           (0x2C)
#endif
#if FFXCONF_NANDSUPPORT_SAMSUNG
  #define NAND_MFG_SAMSUNG          (0xEC)
#endif
#if FFXCONF_NANDSUPPORT_SPANSION
  #define NAND_MFG_SPANSION         (0x01)
#endif
#if FFXCONF_NANDSUPPORT_NUMONYX
  #define NAND_MFG_NUMONYX          (0x20)
#endif
#if FFXCONF_NANDSUPPORT_TOSHIBA
  #define NAND_MFG_TOSHIBA          (0x98)
#endif
#if FFXCONF_NANDSUPPORT_DLTEST
  /*  This is used for test instrumentation within the NAND modules.
      It is an invalid JEDEC device ID (note the even parity)
  */
  #define NAND_MFG_DLTEST           (0x81)
#endif


/*-------------------------------------------------------------------
    NAND chip flags values.
-------------------------------------------------------------------*/
/*  NAND chip interface width
*/
#define CHIPINT_MASK        (0x03)
#define CHIPINT_UNKNOWN     (0x00)
#define CHIPINT_8BIT        (0x01)
#define CHIPINT_16BIT       (0x02)

/* NAND chip Factory Bad Block (FBB) mark style.
*/
#define CHIPFBB_MASK        (0xF0)
#define CHIPFBB_NONE        (0x00)  /* all good -- no BBM required */
#define CHIPFBB_ANYBITZERO  (0x10)  /* any bit in the whole block is zero */
#define CHIPFBB_SSFDC       (0x20)  /* a zero bit in byte offset 5 in 1st or 2nd page */
#define CHIPFBB_OFFSETZERO  (0x30)  /* a zero bit in byte/word offset 0 in 1st or 2nd page */

/*  Typical combinations
*/
#define CHIPFLAGS_TOSHIBA   (CHIPFBB_ANYBITZERO | CHIPINT_8BIT)     /* Old style behavior */
#define CHIPFLAGS_SSFDC     (CHIPFBB_SSFDC      | CHIPINT_8BIT)     /* SSFDC is always 8-bit */
#define CHIPFLAGS_512_16BIT (CHIPFBB_OFFSETZERO | CHIPINT_16BIT)
#define CHIPFLAGS_2KB_8BIT  (CHIPFBB_OFFSETZERO | CHIPINT_8BIT)
#define CHIPFLAGS_4KB_8BIT  (CHIPFBB_OFFSETZERO | CHIPINT_8BIT)
#define CHIPFLAGS_8KB_8BIT  (CHIPFBB_OFFSETZERO | CHIPINT_8BIT)
#define CHIPFLAGS_2KB_16BIT (CHIPFBB_OFFSETZERO | CHIPINT_16BIT)
#define CHIPFLAGS_1KB_16BIT (CHIPFBB_OFFSETZERO | CHIPINT_16BIT)
#define CHIPFLAGS_8KB_16BIT (CHIPFBB_OFFSETZERO | CHIPINT_16BIT)


/*-------------------------------------------------------------------
    Structure describing a class of NAND chips having similar
    characteristics, typically compatible parts from different
    manufacturers.
-------------------------------------------------------------------*/
typedef struct FFXNANDCHIPCLASS
{
    D_UINT16        uPageSize;
    D_UINT16        uSpareSize;
    unsigned char   uLinearPageAddrMSB;  /* how much to >> linear address to get chip page index  */
    unsigned char   uChipPageIndexLSB;   /* how much to << chip page index before sending to chip */
    unsigned char   uLinearBlockAddrLSB; /* how much to >> linear address to get block index */
    unsigned char   uLinearChipAddrMSB;  /* how much to >> linear address to get chip select */
    D_UINT32        ulChipBlocks;        /* Number of erase blocks per chip                  */
    D_UINT32        ulBlockSize;         /* Size of an erase block */
    D_UINT16        uEdcRequirement;     /* Required level of error correction */
    D_UINT16        uEdcCapability;      /* Max error correcting capability */
    D_UINT32        ulEraseCycleRating;  /* Factory spec erase cycle rating */
    D_UINT32        ulBBMReservedRating; /* Reserved blocks required for above */

    /*  Miscellaneous chip special characteristics */

    /*  There is a known issue with the glue logic on 1Gbit ST nand chips (they
        are actually 2 512Mbit chips in series) that causes a program failure in
        certain situations.  From ST Errata (known issues list REV.0.1 5/21/2004
        for NAND01GW4AXAXX6X parts) "Program Failure Jumping from block 0:4095
        to block 4096:8191 Data are not properly written inside the flash" the
        proposed work around from the same document "Insert a RESET command
        before any Page Program Command"
    */
    unsigned ResetBeforeProgram : 1;

    /*  Some newer, larger chips need a Read Start command.
    */
    unsigned ReadConfirm : 1;

    /*  Newer Samsung (and compatibles) have extended operations
    */
    unsigned Samsung2kOps : 1;

    /*  Newer ORNAND has 512-byte emulation support whereby
        partial page reads return a 512B main area and corresponding
        16B spare area without a column address change (seek)
    */
    unsigned ORNANDPartialPageReads : 1;

    /*  Newer Micron chips support dual-plane operations
    */
    unsigned fMicronDualPlaneOps : 1;

    /*  Newer Numonyx parts support block locking mechanism
    */
    unsigned fBlockLockSupport : 1;

    /*  Indicates the chip can only support a single program operation
        per page, generally true for MLC NAND.
    */
    unsigned fProgramOnce : 1;

    /*  Indicates that the factory bad block marker may exist
        in the last page of a block.
    */
    unsigned fLastPageFBB : 1;

} FFXNANDCHIPCLASS;


/*-------------------------------------------------------------------
    Structure containing information for a particular type of NAND
    chip.
-------------------------------------------------------------------*/
typedef struct FFXNANDCHIP
{
    D_UINT8                 uDevIdLength;
    D_UINT8                 aucDevID[NAND_ID_SIZE -1]; /* "-1" because mfr code handled separately */
    D_UINT8                 bFlags;
  #if VERBOSE_CHIP_INFO
    const char *            pPartNumber;
  #endif
    const FFXNANDCHIPCLASS *pChipClass;
} FFXNANDCHIP;


/*-------------------------------------------------------------------
    Structure containing information for a particular chip
    manufacturer.
-------------------------------------------------------------------*/
typedef struct FFXNANDMFG
{
    unsigned int        mfrID;
    const FFXNANDCHIP * (*FindChip)  (const D_UCHAR *aucID);

  #if VERBOSE_CHIP_INFO
    const char *        pMfrName;
  #endif
} FFXNANDMFG;


const FFXNANDCHIP *FfxNandDecodeID(const D_UCHAR *aucID);


#if FFXCONF_NANDSUPPORT_ONENAND
/*  Note that OneNAND devices are not returned by FfxNandDecodeID(), which can't
    be used to search for OneNAND parts, as they share DevIDs with other Samsung
    parts.  Instead, the OneNAND NTM(s) must call OneNANDFindChip() directly.
*/
const FFXNANDCHIP *OneNANDFindChip(const D_UCHAR *aucID);
#endif

#endif


