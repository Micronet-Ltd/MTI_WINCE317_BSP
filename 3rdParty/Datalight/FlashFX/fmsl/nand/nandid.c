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

    The process of recognizing NAND chip ID information is table driven.  At
    the top level is an array of struct FFXNANDMFG.  Each FFXNANDMFG has its
    manufacturer ID value, a manufacturer name string, and a pointer to an
    array of struct FFXNANDCHIP.

    Each FFXNANDCHIP similarly has a device ID value and a device name string,
    and a pointer to a FFXNANDCHIPCLASS structure that describes the chip.

    The FFXNANDCHIPCLASS represents a class of chips that have the same total
    size, page size, and command set.  It also has some special-case char-
    acteristics that differentiate otherwise very similar parts (for example,
    special handling for chip errata).

    Note that the arrays of FFXNANDMFG and FFXNANDCHIP are searched linearly
    (they are small, and this is infrequent).  There is no need to have the
    entries in any particular order, but they are generally sorted so that
    it's convenient for people to look for an ID and avoid duplicating an
    entry.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandid.c $
    Revision 1.76  2011/04/06 20:08:28Z  jimmb
    Updated for support of the Tegra II NTM and Nand controller.
    Revision 1.75  2011/02/07 21:19:48Z  jimmb
    Added the nVidia Tegra 2
    Revision 1.74  2010/09/28 21:28:28Z  glenns
    Back out changes in Rev. 1.72 pending product release.
    Revision 1.73  2010/09/21 22:15:17Z  garyp
    Added more "DatalightSIM" test parts.
    Revision 1.72  2010/08/05 21:22:33Z  glenns
    Add [TRADE NAME TBA] chip class and chip.
    Revision 1.71  2010/06/19 17:01:02Z  garyp
    Updated for a relocated ntmicron.h.
    Revision 1.70  2010/06/19 06:59:09Z  garyp
    Fixed a cut-and-paste error from the previous rev.
    Revision 1.69  2010/06/18 00:03:46Z  garyp
    Added support for some Toshiba parts with chip IDs 0xBC and 0xB3.
    Revision 1.68  2010/06/09 18:02:44Z  billr
    Temporarily disable chip classes that were defined but not used.
    Revision 1.67  2010/06/09 00:05:35Z  glenns
    Add support for Micron M60 and [TRADE NAME TBA].
    Revision 1.66  2010/04/29 00:04:23Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.65  2009/12/04 00:46:17Z  garyp
    Renamed the STMicro references to Numonyx.  General code formatting
    cleanup.  Fixed documentation tags.  No functional changes.
    Revision 1.64  2009/09/28 23:24:17Z  glenns
    Fix Bugzilla #2582- commentary repair. No functional changes.
    Revision 1.63  2009/08/27 17:55:05Z  jimmb
    Added support for Hynix HY27UH088G(5/D)M Series
    Revision 1.62  2009/08/01 01:13:49Z  garyp
    Fixed a syntax error from the previous checkin.
    Revision 1.61  2009/07/24 22:33:26Z  garyp
    Merged from the v4.0 branch.  Added support for a variety of OneNAND
    and Micron parts.
    Revision 1.60  2009/04/24 17:04:16Z  glenns
    - Fix Bugzilla #2666: Corrected entry in Samsung chip table
    Revision 1.59  2009/04/17 21:18:53Z  keithg
    Fixed bug 2620 to properly handle MLC spare area sizes.
    Revision 1.58  2009/04/15 21:47:24Z  keithg
    Corrected chip declaration and added a 128Mbit device to test code.
    Revision 1.57  2009/04/15 17:13:28Z  glenns
    - Fix Bugzilla #2614: Added valid EDC requirement and capability
      values for Spansion ORNAND.
    Revision 1.56  2009/04/13 22:34:38Z  keithg
    Removed unused static.
    Revision 1.55  2009/04/13 20:23:45Z  keithg
    Added small MLC device declaration for test validation.
    Revision 1.54  2009/04/13 17:54:05Z  glenns
    - Fix Bugzilla #2595: Add clarifying comment that the current release
      of FlashFX will not format AnyBitZero devices.
    Revision 1.53  2009/04/10 18:44:01Z  keithg
    Added nominal device configurations for use under simulation and test.
    Revision 1.52  2009/04/02 18:15:36Z  glenns
    - Resolve Bugzilla #1203: Fixed up large-block STMicro chip classes
      to be sure the Random Data Input operator is used.
    Revision 1.51  2009/04/02 14:08:37Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.50  2009/04/01 23:40:18Z  glenns
    - Add support for various Micron, Toshiba and Samsung MLC/LFA
      parts.
    - Fix up Micron device search algorithm to account for possible
      "don't care" bytes in the middle of the ID sequence.
    - Reorganize the samsung tables to be sure order of declaration
      requirement is met yet also be sure to classify the entries.
    - Add commentary about how the tables work.
    Revision 1.49  2009/03/24 19:22:19Z  glenns
    - Fix up a Samsung chip declaration neglected in the last checkin.
    Revision 1.48  2009/03/24 09:18:22Z  glenns
    - Fix Bugzilla #2540. See report in Bugzilla for details.
    Revision 1.47  2009/03/06 23:20:22Z  glenns
    - Fixed FlexOneNAND flags to indicate fProgramOnce is true.
    - Fixed a Micron MLC chip declaration which contained a test
      change.
    - Added commentary to explain how FlexOneNAND devices should
      be declared in the NAND ID tables.
    Revision 1.46  2009/03/03 17:09:52Z  glenns
    - Fix Bug #2477: Standardize method for defining FlexOneNAND
      devices in the NAND ID tables.
    Revision 1.45  2009/03/01 01:56:56Z  glenns
    - Fix up OneNAND chip parameters
    - Re-enable MLC device for FlexOneNAND
    Revision 1.44  2009/02/12 20:23:45Z  glenns
    - Fix device flag for lockable STMicro MLC devices-
      "ResetBeforeProgram" should be turned off.
    Revision 1.43  2009/02/10 21:12:47Z  garyp
    Added a OneNAND part ID.
    Revision 1.42  2009/02/09 22:18:13Z  glenns
    - Added a "flags" initializer for STMicro MLC devices that support
      block locking.
    - Removed 4Gb Numonyx chip class. No such devices.
    - Added a 16GB Micron MLC chip class.
    - Updated Micron chip flags to indicate DEV_PGM_ONCE true.
    Revision 1.41  2009/02/08 21:13:31Z  garyp
    Use correct header inclusion notation.
    Revision 1.40  2009/02/03 00:30:26Z  glenns
    - Added macros to support MLC functionality.
    - Added a flag to indicate last-page-in-block factory bad-block
      marking, and updated macros to initialize the flag.
    - Minor macro name corrections and commentary clarifications.
    - Revised STMicro device tables and algorithms to accommodate
      longer identification strings.
    - Added Numonyx MLC devices.
    - Added Micron MLC device.
    - Added Samsung MLC devices.
    Revision 1.39  2009/01/24 01:01:19Z  billr
    Remove debug code.
    Revision 1.38  2009/01/23 23:00:19Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Add
    fProgramOnce to FFXNANDCHIPCLASS structure.
    Revision 1.37  2009/01/16 23:55:37Z  glenns
    - Updated STMicro chip classes to reflect appropriate EDC
      capability and requirements.
    Revision 1.36  2008/12/10 06:18:09Z  keithg
    Changed RESERVED_UNKNOWN to intended D_UINT32_MAX
    Revision 1.35  2008/12/09 07:10:15Z  keithg
    Changed RESERVED_INVALID to RESERVED_UNKNOWN and set
    sentinel value to D_UINT32_MAX.
    Revision 1.34  2008/12/04 14:51:55Z  thomd
    Added some LFA parts and useful defines
    Revision 1.33  2008/10/24 03:52:40Z  keithg
    Clarified default max cycle counts and EDC requirements defines;
    Defaulted all chip classes to either invalid or correct values.
    Revision 1.32  2008/06/11 13:19:54Z  thomd
    Extend nand definitions with default values for EDC,
    BBM, and erase cycle ratings
    Revision 1.31  2008/05/20 22:00:07Z  billr
    Resolve bug 1888: FlashFX 3.3.0, add Samsung NAND chips
    Revision 1.30  2008/05/07 22:43:48Z  garyp
    Commented out an unused static variable.  Minor code formatting cleanup.
    Revision 1.29  2008/05/06 17:08:30Z  Glenns
    Fixed up some comments. No functional changes.
    Revision 1.28  2008/05/03 03:58:50Z  garyp
    Added support for Micron chip types 0xAA and 0xBA.
    Revision 1.27  2008/05/01 20:18:47Z  Glenns
    Add support for FlexOneNAND devices
    Revision 1.26  2008/02/03 01:46:05Z  keithg
    comment updates to support autodoc.
    Revision 1.25  2008/02/03 00:42:06Z  keithg
    Minor comment clarifications.
    Revision 1.24  2008/01/30 22:23:18Z  Glenns
    Added entries into chip special characteristics table for STMicro and
    OneNAND parts that have block locking.
    Added support for STMicro NAND04GW3B and NAND08GW3B parts.  Added support
    for Samsung FlexOneNAND.
    Revision 1.23  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.22  2007/09/14 21:16:52Z  pauli
    Removed old and unneeded comments.
    Revision 1.21  2007/09/12 20:56:56Z  Garyp
    Placed each different manufacturer's ID tables in conditioned code.  Moved
    non-generic chip class definitions into the scope where they are used.
    Added support for getting extended IDs for Micron chips and added ID
    entries to support Micron dual-plane operations.
    Revision 1.20  2007/08/28 18:40:08Z  pauli
    Added more descriptive comments regarding the use of DEVSIZE_PARAMS.
    Revision 1.19  2007/07/24 17:53:04Z  timothyj
    Removed obsolete macro.
    Revision 1.18  2007/06/27 19:18:27Z  timothyj
    Changed text for ORNAND part IDs that apply to multiple series parts.
    Revision 1.17  2007/06/15 00:14:28Z  timothyj
    Added column to table indicating whether chips support 0x31 command for
    no-seek partial page program operations (ORNAND).
    Revision 1.16  2007/02/28 22:40:45Z  timothyj
    Modified table and macros that load it to store the number of blocks in
    lieu of the device size, for LFA compatibility (Tables can now accommodate
    devices larger than 32Gb).
    Revision 1.15  2007/01/19 23:29:06Z  peterb
    Corrected ST Micro chip lookup conditions to correctly find parts for
    which we do not care about the third ID byte.
    Revision 1.14  2007/01/04 02:11:02Z  Timothyj
    Removed extra ','s that were causing a compile warning on release builds.
    Revision 1.13  2007/01/03 23:58:39Z  Timothyj
    IR #777, 778, 681: Modified FfxNandDecodeId() to allow for devices such as
    ST Micro NAND04GW3C2AN1 and Spansion OrNAND, which have characteristics
    that are not common among all devices.  Allows the use of additional bytes
    in the ID string.  Added OneNAND decode function (moved from static
    implementation in OneNAND NTM), as it conforms to the above method and
    could therefore be used by other OneNAND NTMs in addition to nt1nand.c.
    IR #809: Renamed AMD to Spansion.
    Revision 1.12  2006/12/21 23:15:02Z  timothyj
    Added support for 16Gb Hynix part
    Revision 1.11  2006/12/20 21:28:07Z  timothyj
    Added support for Micron 16Gb part (related 8Gb part defacto supported by
    4Gb - it's 2 4Gb die in the same package).
    Revision 1.10  2006/10/07 03:35:40Z  Garyp
    Corrected to eliminate warnings generated by the RealView tools.
    Revision 1.9  2006/07/10 22:38:45Z  Garyp
    Corrected an apparently wrong chip class.
    Revision 1.8  2006/05/23 02:37:23Z  Garyp
    Re-ordered the ST parts for clarity.
    Revision 1.7  2006/05/04 20:40:38Z  Garyp
    Fixed some misclassified Toshiba 2K page chips.
    Revision 1.6  2006/04/12 21:57:18Z  rickc
    Added part info for Spansion ORNAND
    Revision 1.5  2006/03/16 04:04:44  Garyp
    Fixed syntax errors with the previous checkin.
    Revision 1.4  2006/03/16 02:41:04Z  Garyp
    Added missing parts stemming from the v2.0 merge.
    Revision 1.3  2006/03/02 21:22:24Z  Garyp
    Updated the FFXNANDCHIP structure to include a status byte containing the
    interface width, 8 or 16 bit, and the style of factory bad block marking.
    Revision 1.2  2006/01/11 02:13:01Z  Garyp
    Documentation and debug code changes only -- nothing functional.
    Revision 1.1  2005/12/03 20:45:28Z  Pauli
    Initial revision
    Revision 1.3  2005/12/03 20:45:27Z  Garyp
    Modified the FFXNANDCHIPCLASS structure to contain a "spare-format" field.
    Revision 1.2  2005/10/25 22:11:00Z  Garyp
    Updated to add the spare size to the FFXNANDCHIPCLASS structure.
    Revision 1.1  2005/05/03 18:29:12Z  Garyp
    Initial revision
    Revision 1.4  2005/09/07 01:19:24Z  garyp
    Documentation fix.
    Revision 1.3  2005/08/25 04:39:58Z  Garyp
    Added the DECLARE_NAND_CHIP and DECLARE_NAND_MFG macros as well as the
    VERBOSE_CHIP_INFO to reduce run-time image sizes.
    Revision 1.2  2005/08/17 06:36:13Z  garyp
    Updated a NAND part family name (code unchanged).
    Revision 1.1  2005/07/29 16:40:42Z  pauli
    Initial revision
    Revision 1.2  2005/07/29 16:40:42Z  Garyp
    Change back to using the real device type.
    Revision 1.1  2005/07/27 03:56:54Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <nandid.h>

#if FFXCONF_NANDSUPPORT_MICRON
#include <ntmicron.h>
#endif

/*  Initializers for the bit counts and sizes for 512-byte to 4096-byte
    page chips with standard spare area sizes.  Each of the parameters
    below are used to initialize the FFXNANDCHIPCLASS members:
    uPageSize, uSpareSize, uLinearPageAddrMSB, uChipPageIndexLSB,
    and uLinearBlockAddrLSB.
*/
#define PAGESIZE_512         512,  16,    9,  8,  9
#define PAGESIZE_1024       1024,  32,   10, 16, 10
#define PAGESIZE_2048       2048,  64,   11, 16, 11
#define PAGESIZE_4096       4096,  128,  12, 16, 12

/*  The following interleaved macros have only be tested under simulation.
*/
#define PAGESIZE_2048X2   4096,    128,  11, 16, 11
#define PAGESIZE_2048X4   8192,    256,  11, 16, 11
#define PAGESIZE_2048X8   16384,   512,  11, 16, 11

/*  Initializers for large-page MLC devices requiring high levels of
    ECC protoection:
*/
#define MLCPAGESIZE_4096    4096,  218,  12, 16, 12
#define MLCPAGESIZE_8192    8192,  436,  13, 16, 13


/*  The following interleaved macros have only be tested under simulation.
*/
#define MLCPAGESIZE_4096X2   8192,  436,  12, 16, 12
#define MLCPAGESIZE_4096X4  16384,  872,  12, 16, 12
#define MLCPAGESIZE_4096X8  32768, 1724,  12, 16, 12

/*---------------------------------------------------------
    Macro for specifying the number of blocks and block
    size using human-readable number of megabits and 
    kilobytes.

    m - number of megabits specified in units of KB
    n - erase block size in KB

    For example, to specify a 128 megabit part with 16
    kilobyte erase blocks:
    
        DEVSIZE_PARAMS(128, 16)
---------------------------------------------------------*/
#define DEVSIZE_PARAMS(chpMb, blkKB) (((chpMb) * (1024UL / 8)) / (blkKB)), ((blkKB) * 1024UL)

/*  Number of address bits for device size in Mbits
*/
#define ADDR_LINES_8        20
#define ADDR_LINES_16       21
#define ADDR_LINES_32       22
#define ADDR_LINES_64       23
#define ADDR_LINES_128      24
#define ADDR_LINES_256      25
#define ADDR_LINES_512      26
#define ADDR_LINES_1024     27
#define ADDR_LINES_2048     28
#define ADDR_LINES_4096     29
#define ADDR_LINES_8192     30
#define ADDR_LINES_16384    31
#define ADDR_LINES_32768    32
#define ADDR_LINES_65536    33

/*  Erase cycle endurance macros */
#define  MAX_CYCLE_100K     100000
#define  MAX_CYCLE_50K      50000
#define  MAX_CYCLE_20K      20000
#define  MAX_CYCLE_10K      10000
#define  MAX_CYCLE_5K       5000
#define  MAX_CYCLE_1K       1000
#define  MAX_CYCLE_INVALID  9

/* EDC Capabilities and Requirements macros */
#define EDC_NONE            0
#define EDC_INVALID         255
#define EDC_1_BIT           1
#define EDC_2_BIT           2
#define EDC_4_BIT           4
#define EDC_6_BIT           6
#define EDC_8_BIT           8
#define EDC_10_BIT          10
#define EDC_12_BIT          12

/*  The following is used by NANDID to indicate when the reserved
    rating of a part has not been set or is otherwise unknown.
    A default rating
*/
#define RESERVED_UNKNOWN            D_UINT32_MAX

/*  Initializers for chip special characteristics flags.  These
    are all booleans, and are 1 and 0 instead of TRUE and FALSE
    for compactness and readability.

                         +---------------------- ResetBeforeProgram
                         |  +------------------- ReadConfirm
                         |  |  +---------------- Samsung2Kb
                         |  |  |  +------------- ORNANDPartialPageReads
                         |  |  |  |  +---------- MicronDualPlaneOps
                         |  |  |  |  |  +------- BlockLockSupport
                         |  |  |  |  |  |  +---- fProgramOnce
                         |  |  |  |  |  |  |  +- fLastPageFBB
                         |  |  |  |  |  |  |  |
*/
#define GENERIC          0, 0, 0, 0, 0, 0, 0, 0  /* For most older parts */
#define READ_CONFIRM     0, 1, 0, 0, 0, 0, 0, 0  /* For non-ST 1Gb and up */
#define ST_1G_RESET      1, 1, 1, 0, 0, 0, 0, 0  /* For Numonyx 1Gb parts */
#define SAMSUNG1KOPS     0, 1, 0, 0, 0, 0, 0, 0  /* For Samsung 1K ops */
#define SAMSUNG2KOPS     0, 1, 1, 0, 0, 0, 0, 0  /* For non-ST 1Gb and up with Samsung 2K ops */
#define SAMSUNGMLC2KOPS  0, 1, 1, 0, 0, 0, 1, 0  /* For Samsung MLC chips */
#define ORNAND2KOPS      0, 1, 1, 1, 0, 0, 0, 0  /* For Spansion ORNAND 2K ops w/ 0x31 command */
#define MICRONDPOPS      0, 1, 1, 0, 1, 0, 0, 0  /* For Micron chips with dual-plane ops */
#define MICRONLOCK       0, 1, 1, 0, 0, 1, 0, 0  /* For Micron chips with lock support ops */
#define MICRONDPLOCK     0, 1, 1, 0, 1, 1, 0, 0  /* For Micron chips with dual-plane ops and locking */
#define MICRONM60LOCK    0, 1, 1, 0, 1, 1, 1, 0  /* For Micron M60 1.8v lockable devices */
#define MICRONM60        0, 1, 1, 0, 1, 0, 1, 0  /* For Micron M60 3.3v non-lockable devices */
#define ST_LOCKABLE      1, 1, 1, 0, 0, 1, 0, 0  /* For Numonyx large parts with locking*/
#define FLEX1NAND        0, 0, 0, 0, 0, 1, 1, 0  /* Samsung FlexOneNAND parts */
#define MOSTMLC          0, 1, 1, 0, 0, 0, 1, 0  /* Typical MLC chips */
#define MICRONMLC        0, 1, 1, 0, 1, 0, 1, 0  /* Micron MLC chips */
#define NUMONYXMLC       0, 1, 1, 0, 0, 0, 1, 1  /* Numonyx MLC chips (FBB in last page)*/
#define STMLC_LOCKABLE   0, 1, 1, 0, 0, 1, 1, 1  /* Numonyx MLC chips (FBB in last page)*/

/*  Placeholder for ID positions that are not checked:
*/
#define ID_DONT_CARE 0xFF

/*  Chip classes.  The names are based on sizes in Mbits (not MBytes).

    The only those chip classes which should appear in this table are
    those which are shared by multiple chip manufacturers (such as the
    Samsung classes below).  Chip classes which are exclusive to a
    particular manufacturer should be declared within the scope of
    that manufacturer's FindChip() function.

    Note: The GeneralXXX chip classes assume 24 / 1000 blocks are set
    recommened for replacement blocks.  This should be sufficient for
    most parts, however check the manufacturer specifications to obtain
    accurate requirements.
*/
static const FFXNANDCHIPCLASS Generic32 =        {PAGESIZE_512,  ADDR_LINES_32,      DEVSIZE_PARAMS(  32,    8), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  12, GENERIC     };
static const FFXNANDCHIPCLASS Generic64 =        {PAGESIZE_512,  ADDR_LINES_64,      DEVSIZE_PARAMS(  64,    8), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  24, GENERIC     };
static const FFXNANDCHIPCLASS Generic128 =       {PAGESIZE_512,  ADDR_LINES_128,     DEVSIZE_PARAMS( 128,   16), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  24, GENERIC     };
static const FFXNANDCHIPCLASS Generic256 =       {PAGESIZE_512,  ADDR_LINES_256,     DEVSIZE_PARAMS( 256,   16), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  48, GENERIC     };
static const FFXNANDCHIPCLASS Generic512 =       {PAGESIZE_512,  ADDR_LINES_512,     DEVSIZE_PARAMS( 512,   16), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  96, GENERIC     };
static const FFXNANDCHIPCLASS Generic1G =        {PAGESIZE_512,  ADDR_LINES_1024,    DEVSIZE_PARAMS(1024,   16), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, 192, GENERIC     };
static const FFXNANDCHIPCLASS Generic512_2K =    {PAGESIZE_2048, ADDR_LINES_512,     DEVSIZE_PARAMS( 512,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  12, READ_CONFIRM};
static const FFXNANDCHIPCLASS Generic1G_2K =     {PAGESIZE_2048, ADDR_LINES_1024,    DEVSIZE_PARAMS(1024,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  24, READ_CONFIRM};
static const FFXNANDCHIPCLASS Generic2G_2K =     {PAGESIZE_2048, ADDR_LINES_2048,    DEVSIZE_PARAMS(2048,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  48, READ_CONFIRM};
static const FFXNANDCHIPCLASS Generic4G_2K =     {PAGESIZE_2048, ADDR_LINES_4096,    DEVSIZE_PARAMS(4096,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  96, READ_CONFIRM}; 
static const FFXNANDCHIPCLASS Generic8G_2K =     {PAGESIZE_2048, ADDR_LINES_8192,    DEVSIZE_PARAMS(8192,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, 192, READ_CONFIRM}; 
static const FFXNANDCHIPCLASS GenericMLC8G_4K =  {MLCPAGESIZE_4096, ADDR_LINES_8192, DEVSIZE_PARAMS(8192,  512), EDC_8_BIT, EDC_NONE, MAX_CYCLE_10K,   80, MOSTMLC};
static const FFXNANDCHIPCLASS GenericMLC16G_4K = {MLCPAGESIZE_4096, ADDR_LINES_16384,DEVSIZE_PARAMS(16384, 512), EDC_8_BIT, EDC_NONE, MAX_CYCLE_10K,  160, MOSTMLC};
static const FFXNANDCHIPCLASS Samsung1G_2K =     {PAGESIZE_2048, ADDR_LINES_1024,    DEVSIZE_PARAMS(1024,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  20, SAMSUNG2KOPS};
static const FFXNANDCHIPCLASS Samsung2G_2K =     {PAGESIZE_2048, ADDR_LINES_2048,    DEVSIZE_PARAMS(2048,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  40, SAMSUNG2KOPS};
static const FFXNANDCHIPCLASS Samsung4G_2K =     {PAGESIZE_2048, ADDR_LINES_4096,    DEVSIZE_PARAMS(4096,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K,  80, SAMSUNG2KOPS};
static const FFXNANDCHIPCLASS Hynix4G_2K =     {PAGESIZE_2048, ADDR_LINES_4096,    DEVSIZE_PARAMS(4096,  128), EDC_4_BIT, EDC_NONE, MAX_CYCLE_100K,  80, SAMSUNG2KOPS}; /* GL NVidia Tegra*/
static const FFXNANDCHIPCLASS Samsung8G_2K =     {PAGESIZE_2048, ADDR_LINES_8192,    DEVSIZE_PARAMS(8192,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, 160, SAMSUNG2KOPS};
static const FFXNANDCHIPCLASS Samsung16G_4K =    {PAGESIZE_4096, ADDR_LINES_16384,   DEVSIZE_PARAMS(16384, 256), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, 320, SAMSUNG2KOPS};

/*  Notes about the following chip classes: Samsung's evolution of MLC NAND
    devices has led to two basic classifications; Generation 1 and Generation
    5.  Devices of Generation 1 have a part number "K9xxxxxxxM-xxxxxxx" and
    for Generation 5, the number is "K9xxxxxxxD-xxxxxxx".  There are two
    important differences between Generation 1-4 and Generation 5; they encode
    Device ID sequences according to a different standard, and Generation 5
    devices have a larger number of bytes in the spare area per 512 bytes
    of page storage to accommodate stronger EDC protection.

    Samsung offers some devices in multiple generations.  Although these devices
    have otherwise similar operating characteristics and geometries, they must
    have distinct chip classes due to the differences outlined above.
*/
static const FFXNANDCHIPCLASS SamsungMLC4G_2K       = {PAGESIZE_2048,    ADDR_LINES_4096,  DEVSIZE_PARAMS(4096, 256),  EDC_4_BIT, EDC_NONE, MAX_CYCLE_5K, RESERVED_UNKNOWN, SAMSUNGMLC2KOPS};
static const FFXNANDCHIPCLASS SamsungMLC8G_2K       = {PAGESIZE_2048,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192, 256),  EDC_4_BIT, EDC_NONE, MAX_CYCLE_5K, RESERVED_UNKNOWN, SAMSUNGMLC2KOPS};
/* static const FFXNANDCHIPCLASS SamsungMLC16G_4K   = {PAGESIZE_4096,    ADDR_LINES_16384, DEVSIZE_PARAMS(16384, 512), EDC_4_BIT, EDC_NONE, MAX_CYCLE_5K, RESERVED_UNKNOWN, SAMSUNGMLC2KOPS}; */
static const FFXNANDCHIPCLASS SamsungMLC32G_4K      = {PAGESIZE_4096,    ADDR_LINES_32768, DEVSIZE_PARAMS(32768, 512), EDC_4_BIT, EDC_NONE, MAX_CYCLE_5K, RESERVED_UNKNOWN, SAMSUNGMLC2KOPS};
static const FFXNANDCHIPCLASS SamsungMLC16G_4K_GEN5 = {MLCPAGESIZE_4096, ADDR_LINES_16384, DEVSIZE_PARAMS(16384, 512), EDC_8_BIT, EDC_NONE, MAX_CYCLE_5K, RESERVED_UNKNOWN, SAMSUNGMLC2KOPS};
static const FFXNANDCHIPCLASS SamsungMLC32G_4K_GEN5 = {MLCPAGESIZE_4096, ADDR_LINES_32768, DEVSIZE_PARAMS(32768, 512), EDC_8_BIT, EDC_NONE, MAX_CYCLE_5K, RESERVED_UNKNOWN, SAMSUNGMLC2KOPS};

/*  Implementations of the FindChip function of the FFXNANDMFG table.
*/
#if FFXCONF_NANDSUPPORT_SPANSION
    static const FFXNANDCHIP *SpansionFindChip(const D_UCHAR *aucID);
#endif
#if FFXCONF_NANDSUPPORT_HYNIX
    static const FFXNANDCHIP *HynixFindChip(   const D_UCHAR *aucID);
#endif
#if FFXCONF_NANDSUPPORT_MICRON
    static const FFXNANDCHIP *MicronFindChip(  const D_UCHAR *aucID);
#endif
#if FFXCONF_NANDSUPPORT_SAMSUNG
    static const FFXNANDCHIP *SamsungFindChip( const D_UCHAR *aucID);
#endif
#if FFXCONF_NANDSUPPORT_NUMONYX
    static const FFXNANDCHIP *NumonyxFindChip( const D_UCHAR *aucID);
#endif
#if FFXCONF_NANDSUPPORT_TOSHIBA
    static const FFXNANDCHIP *ToshibaFindChip( const D_UCHAR *aucID);
#endif
#if FFXCONF_NANDSUPPORT_DLTEST
    static const FFXNANDCHIP *DlTestFindChip( const D_UCHAR *aucID);
#endif


/*  Note that OneNAND is not in the table below - FfxNandDecodeID() can't
    be used to search for OneNAND parts, as they share DevIDs with other
    Samsung parts.  Instead, the OneNAND NTM(s) must call OneNANDFindChip()
    directly.
*/

/*  List of chip manufacturers.
*/
static const FFXNANDMFG aChipMfr[] =
{
  #if FFXCONF_NANDSUPPORT_HYNIX
    DECLARE_NAND_MFG(NAND_MFG_HYNIX,    HynixFindChip,    "Hynix"   ),
  #endif
  #if FFXCONF_NANDSUPPORT_MICRON
    DECLARE_NAND_MFG(NAND_MFG_MICRON,   MicronFindChip,   "Micron"  ),
  #endif
  #if FFXCONF_NANDSUPPORT_SAMSUNG
    DECLARE_NAND_MFG(NAND_MFG_SAMSUNG,  SamsungFindChip,  "Samsung" ),
  #endif
  #if FFXCONF_NANDSUPPORT_SPANSION
    DECLARE_NAND_MFG(NAND_MFG_SPANSION, SpansionFindChip, "Spansion"),
  #endif
  #if FFXCONF_NANDSUPPORT_NUMONYX
    DECLARE_NAND_MFG(NAND_MFG_NUMONYX,  NumonyxFindChip,  "Numonyx"),
  #endif
  #if FFXCONF_NANDSUPPORT_TOSHIBA
    DECLARE_NAND_MFG(NAND_MFG_TOSHIBA,  ToshibaFindChip,  "Toshiba" ),
  #endif
  #if FFXCONF_NANDSUPPORT_DLTEST
    DECLARE_NAND_MFG(NAND_MFG_DLTEST,   DlTestFindChip,   "DatalightSIM" ),
  #endif
    DECLARE_NAND_MFG(0,                 NULL,             NULL      )
};



/*-------------------------------------------------------------------
    Private: FfxNandDecodeID()

    Look up the manufacturer and device IDs read from a NAND
    chip, and fill in the information needed at run time to
    operate the chip.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a filled-in FFXNANDCHIP structure if
        successful, NULL otherwise
-------------------------------------------------------------------*/
const FFXNANDCHIP *FfxNandDecodeID(
    const D_UCHAR      *aucID)
{
    const FFXNANDMFG   *pMfr = NULL;
    const FFXNANDCHIP  *pChip = NULL;
    unsigned int        i;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, 0), "FfxNandDecodeID()\n"));

    /*  The first ID byte is the manufacturer.
    */
    for(i = 0; i < DCLDIMENSIONOF(aChipMfr); ++i)
    {
        /*  Get out if the end of the list was found
        */
        if(aChipMfr[i].FindChip == NULL)
            break;

        if(aChipMfr[i].mfrID == aucID[0])
        {
            pMfr = &aChipMfr[i];
            break;
        }
    }

    if(pMfr)
    {
        /*  We now have a manufacturer (in the figurative sense as well as
            in many cases the literal sense, a chip factory)
        */
        DclAssert(pMfr->FindChip);

        pChip = pMfr->FindChip(aucID);

    }

  #if VERBOSE_CHIP_INFO
    FFXPRINTF(1, ("NAND chip manufacturer: %s (%02x) chip %s (%02x)\n",
                  pMfr ? pMfr->pMfrName : "unknown", aucID[0],
                  pChip ? pChip->pPartNumber : "unknown", aucID[1]));
  #else
    FFXPRINTF(1, ("NAND chip manufacturer ID: %02x Chip ID %02x\n",
                  aucID[0], aucID[1]));
  #endif

    return pChip;
}



#if FFXCONF_NANDSUPPORT_NUMONYX
/*---------------------------------------------------------------------*/
/*----------------------------- Numonyx Table -------------------------*/
/*-----------------------------     Begin     -------------------------*/
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Local: NumonyxFindChip()

    This is the Numonyx-specific implementation of the FindChip()
    function.  This function searches the static tables for a
    chip data structure that matches the ID passed in as an
    argument, and returns the address of that structure.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a constant FFXNANDCHIP structure if
        successful, NULL otherwise.  Note that the pointer is to a
        static structure, and as such is const (and the caller
        must not modify).  Also, the pointer value can be used
        in comparison operations to determine if two chips are
        of identicial configurations.
-------------------------------------------------------------------*/
static const FFXNANDCHIP *NumonyxFindChip(
    const D_UCHAR      *aucID)
{
    const FFXNANDCHIP  *pChip = NULL;
    unsigned int        i,j;
    D_BOOL              bMatch;

    static const FFXNANDCHIPCLASS Numonyx1G_2K =     {PAGESIZE_2048, ADDR_LINES_1024,  DEVSIZE_PARAMS(1024,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, RESERVED_UNKNOWN, ST_1G_RESET };
    static const FFXNANDCHIPCLASS Numonyx2G_2K =     {PAGESIZE_2048, ADDR_LINES_2048,  DEVSIZE_PARAMS(2048,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
    static const FFXNANDCHIPCLASS Numonyx4G_2K =     {PAGESIZE_2048, ADDR_LINES_4096,  DEVSIZE_PARAMS(4096,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, RESERVED_UNKNOWN, ST_LOCKABLE};
    static const FFXNANDCHIPCLASS Numonyx8G_2K =     {PAGESIZE_2048, ADDR_LINES_8192,  DEVSIZE_PARAMS(8192,  128), EDC_1_BIT, EDC_NONE, MAX_CYCLE_100K, RESERVED_UNKNOWN, ST_LOCKABLE};
    static const FFXNANDCHIPCLASS NumonyxMLC4G_2K =  {PAGESIZE_2048, ADDR_LINES_4096,  DEVSIZE_PARAMS(4096,  256), EDC_4_BIT, EDC_NONE, MAX_CYCLE_10K,  RESERVED_UNKNOWN, STMLC_LOCKABLE};
    static const FFXNANDCHIPCLASS NumonyxMLC8G_2K =  {PAGESIZE_2048, ADDR_LINES_8192,  DEVSIZE_PARAMS(8192,  256), EDC_4_BIT, EDC_NONE, MAX_CYCLE_10K,  RESERVED_UNKNOWN, NUMONYXMLC};
    static const FFXNANDCHIPCLASS NumonyxMLC8G_4K =  {PAGESIZE_4096, ADDR_LINES_8192,  DEVSIZE_PARAMS(8192,  512), EDC_4_BIT, EDC_NONE, MAX_CYCLE_10K,  RESERVED_UNKNOWN, NUMONYXMLC};
    static const FFXNANDCHIPCLASS NumonyxMLC16G_4K = {PAGESIZE_4096, ADDR_LINES_16384, DEVSIZE_PARAMS(16384, 512), EDC_4_BIT, EDC_NONE, MAX_CYCLE_10K,  RESERVED_UNKNOWN, NUMONYXMLC};

    /*  List of all Numonyx chips supported.
    */
    static const FFXNANDCHIP Numonyx[] =
    {
        /*  The algorithm used to match devices in this table is a strict
            exact byte-for-byte match of not only the individual id bytes,
            but also for the length of the sequence. Device listings in this
            table will only be matched up to the listed length of the sequence,
            which is the first entry in the DECLARE_NAND_CHIP macro.

            Because of this, the order in which entries is important for
            devices that have duplicate bytes in their entries up to some point
            in the sequence. Devices with the most bytes in their sequences
            *must* be listed first in the table. Otherwise, such a device will
            get compared first against the table entry that has one or more
            duplicate bytes early in its sequence, but the full sequence will
            not get examined and a false match will occur.
        */
        DECLARE_NAND_CHIP(3, ID_3(0xD3, 0x14, 0xA5), &NumonyxMLC8G_2K,   CHIPFLAGS_2KB_8BIT,  "NAND08GW3C"),
        DECLARE_NAND_CHIP(3, ID_3(0xD3, 0x14, 0xB6), &NumonyxMLC8G_4K,   CHIPFLAGS_2KB_8BIT,  "NAND08GW3D"),
        DECLARE_NAND_CHIP(3, ID_3(0xD5, 0x14, 0xB6), &NumonyxMLC16G_4K,  CHIPFLAGS_2KB_8BIT,  "NAND16GW3D"),
        DECLARE_NAND_CHIP(2, ID_2(0xDC, 0x10), &Samsung4G_2K,            CHIPFLAGS_2KB_8BIT,  "NAND04GW3B2D"),
        DECLARE_NAND_CHIP(2, ID_2(0xDC, 0x80), &Numonyx4G_2K,            CHIPFLAGS_2KB_8BIT,  "NAND04GW3B2B"),
        DECLARE_NAND_CHIP(2, ID_2(0xDC, 0x84), &NumonyxMLC4G_2K,         CHIPFLAGS_2KB_8BIT,  "NAND04GW3C"),
        DECLARE_NAND_CHIP(2, ID_2(0xD3, 0x51), &Samsung8G_2K,            CHIPFLAGS_2KB_8BIT,  "NAND08GW3B2C"),
        DECLARE_NAND_CHIP(2, ID_2(0xD3, 0x81), &Numonyx8G_2K,            CHIPFLAGS_2KB_8BIT,  "NAND08GW3B2A"),
        DECLARE_NAND_CHIP(2, ID_2(0xD3, 0xC1), &Numonyx8G_2K,            CHIPFLAGS_2KB_8BIT,  "NAND08GW3B"),
        DECLARE_NAND_CHIP(1, ID_1(0x33), &Generic128,       CHIPFLAGS_SSFDC,     "NAND128R3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x35), &Generic256,       CHIPFLAGS_SSFDC,     "NAND256R3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x36), &Generic512,       CHIPFLAGS_SSFDC,     "NAND512R3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x39), &Generic1G,        CHIPFLAGS_SSFDC,     "NAND01GR3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x43), &Generic128,       CHIPFLAGS_512_16BIT, "NAND128R4A"),
        DECLARE_NAND_CHIP(1, ID_1(0x45), &Generic256,       CHIPFLAGS_512_16BIT, "NAND256R4A"),
        DECLARE_NAND_CHIP(1, ID_1(0x46), &Generic512,       CHIPFLAGS_512_16BIT, "NAND512R4A"),
        DECLARE_NAND_CHIP(1, ID_1(0x49), &Generic1G,        CHIPFLAGS_512_16BIT, "NAND01GR4A"),
        DECLARE_NAND_CHIP(1, ID_1(0x73), &Generic128,       CHIPFLAGS_SSFDC,     "NAND128W3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x75), &Generic256,       CHIPFLAGS_SSFDC,     "NAND256W3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x76), &Generic512,       CHIPFLAGS_SSFDC,     "NAND512W3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x79), &Generic1G,        CHIPFLAGS_SSFDC,     "NAND01GW3A"),
        DECLARE_NAND_CHIP(1, ID_1(0x53), &Generic128,       CHIPFLAGS_512_16BIT, "NAND128W4A"),
        DECLARE_NAND_CHIP(1, ID_1(0x55), &Generic256,       CHIPFLAGS_512_16BIT, "NAND256W4A"),
        DECLARE_NAND_CHIP(1, ID_1(0x56), &Generic512,       CHIPFLAGS_512_16BIT, "NAND512W4A"),
        DECLARE_NAND_CHIP(1, ID_1(0x59), &Generic1G,        CHIPFLAGS_512_16BIT, "NAND01GW4A"),
        DECLARE_NAND_CHIP(1, ID_1(0xA2), &Generic512_2K,    CHIPFLAGS_2KB_8BIT,  "NAND512R3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xA1), &Numonyx1G_2K,     CHIPFLAGS_2KB_8BIT,  "NAND01GR3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xAA), &Numonyx2G_2K,     CHIPFLAGS_2KB_8BIT,  "NAND02GR3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xAC), &Numonyx4G_2K,     CHIPFLAGS_2KB_8BIT,  "NAND04GR3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xA3), &Numonyx8G_2K,     CHIPFLAGS_2KB_8BIT,  "NAND08GR3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xB2), &Generic512_2K,    CHIPFLAGS_2KB_16BIT, "NAND512R4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xB1), &Numonyx1G_2K,     CHIPFLAGS_2KB_16BIT, "NAND01GR4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xBA), &Numonyx2G_2K,     CHIPFLAGS_2KB_16BIT, "NAND02GR4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xBC), &Numonyx4G_2K,     CHIPFLAGS_2KB_16BIT, "NAND04GR4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xB3), &Numonyx8G_2K,     CHIPFLAGS_2KB_16BIT, "NAND08GR4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xF2), &Generic512_2K,    CHIPFLAGS_2KB_8BIT,  "NAND512W3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xF1), &Numonyx1G_2K,     CHIPFLAGS_2KB_8BIT,  "NAND01GW3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xDA), &Numonyx2G_2K,     CHIPFLAGS_2KB_8BIT,  "NAND02GW3B"),
        DECLARE_NAND_CHIP(1, ID_1(0xC2), &Generic512_2K,    CHIPFLAGS_2KB_16BIT, "NAND512W4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xC1), &Numonyx1G_2K,     CHIPFLAGS_2KB_16BIT, "NAND01GW4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xCA), &Numonyx2G_2K,     CHIPFLAGS_2KB_16BIT, "NAND02GW4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xCC), &Numonyx4G_2K,     CHIPFLAGS_2KB_16BIT, "NAND04GW4B"),
        DECLARE_NAND_CHIP(1, ID_1(0xC3), &Numonyx8G_2K,     CHIPFLAGS_2KB_16BIT, "NAND08GW4B"),

    };


    for(i = 0; i < DCLDIMENSIONOF(Numonyx); ++i)
    {
        bMatch = TRUE;
        for (j=0; j<Numonyx[i].uDevIdLength; j++)
        {
            if (Numonyx[i].aucDevID[j] != aucID[1+j])
                bMatch = FALSE;
        }
        
        if (bMatch)
        {
            pChip = &Numonyx[i];
            break;
        }
     }

    return pChip;
}


/*---------------------------------------------------------------------*/
/*----------------------------- Numonyx Table -------------------------*/
/*-----------------------------       End     -------------------------*/
/*---------------------------------------------------------------------*/
#endif


#if FFXCONF_NANDSUPPORT_SAMSUNG
/*---------------------------------------------------------------------*/
/*----------------------------- Samsung Table -------------------------*/
/*-----------------------------     Begin      ------------------------*/
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Local: SamsungFindChip()

    This is the Samsung-specific implementation of the FindChip()
    function.  This function searches the static tables for a
    chip data structure that matches the ID passed in as an
    argument, and returns the address of that structure.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a constant FFXNANDCHIP structure if
        successful, NULL otherwise.  Note that the pointer is to a
        static structure, and as such is const (and the caller
        must not modify).  Also, the pointer value can be used
        in comparison operations to determine if two chips are
        of identicial configurations.
-------------------------------------------------------------------*/
static const FFXNANDCHIP *SamsungFindChip(
    const D_UCHAR              *aucID)
{
    const FFXNANDCHIP          *pChip = NULL;
    unsigned int                i,j;
    D_BOOL                      bMatch;
    static const FFXNANDCHIP    Samsung[] =
    {
        /*  The algorithm used to match devices in this table is a strict
            exact byte-for-byte match of not only the individual id bytes,
            but also for the length of the sequence. Device listings in this
            table will only be matched up to the listed length of the sequence,
            which is the first entry in the DECLARE_NAND_CHIP macro.

            Because of this, the order in which entries is important for
            devices that have duplicate bytes in their entries up to some point
            in the sequence. Devices with the most bytes in their sequences
            *must* be listed first in the table. Otherwise, such a device will
            get compared first against the table entry that has one or more
            duplicate bytes early in its sequence, but the full sequence will
            not get examined and a false match will occur.

            Note that many Samsung devices support multi-chip-enable versions of
            the devices in these tables. None of the part numbers listed in
            these tables represent multi-CE configurations. Since such devices
            do not encode the fact that a multi-CE package is being used and
            only give information about the individual die configurations, it
            is up to your project hooks to be able to sort out the chip-enable
            configuration and deduce the appropriate FlashFX device array size.

            Note also that older Samsung devices used the ID codes to distinguish
            Vcc and bus width configurations. This table may not cover all
            combinations of those. Consult your datasheet.

            The following notes apply to the Samsung ID table:

            Note 1: These are devices that have been tested and/or were historically
            recognized.

            Note 2: These devices have not been tested, but are very likely to
            work, as they appear on the same datasheet as parts that are already
            supported (referenced in comment).

            Note 3: These devices have not been specifically tested by Datalight,
            but have been reported as working by various customers.

            Note 4: These devices have not been tested, but exist within device
            families that have been tested.
        */
        DECLARE_NAND_CHIP(2, ID_2(0xDC, 0x14), &SamsungMLC4G_2K,       CHIPFLAGS_2KB_8BIT,  "K9G4G08U0A/B"),            /* Note 4 */
        DECLARE_NAND_CHIP(2, ID_2(0xD3, 0x14), &SamsungMLC8G_2K,       CHIPFLAGS_2KB_8BIT,  "K9G8G08U0A/B"),            /* Note 4 */
        DECLARE_NAND_CHIP(2, ID_2(0xD5, 0x84), &SamsungMLC16G_4K_GEN5, CHIPFLAGS_4KB_8BIT,  "K9GAG08U0D"),              /* Note 4 */
        DECLARE_NAND_CHIP(2, ID_2(0xD7, 0x55), &SamsungMLC32G_4K,      CHIPFLAGS_4KB_8BIT,  "K9LBG08U0M"),              /* Note 1 */
        DECLARE_NAND_CHIP(2, ID_2(0xD7, 0xD5), &SamsungMLC32G_4K_GEN5, CHIPFLAGS_4KB_8BIT,  "K9LBG08U0D"),              /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0xE6),       &Generic64,             CHIPFLAGS_SSFDC,     "K9F6408U0B/C"),            /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0x73),       &Generic128,            CHIPFLAGS_SSFDC,     "K9F2808U0B/C/KM29U128T/IT"), /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0x75),       &Generic256,            CHIPFLAGS_SSFDC,     "K9F5608D0C/U0A/B/C"),      /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0x46),       &Generic512,            CHIPFLAGS_512_16BIT, "K9F1216Q0A/B/K9K1216Q0A/C"), /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0x76),       &Generic512,            CHIPFLAGS_SSFDC,     "K9F1208D0A/B/U0A/B/M/K9K1208D0C/U0C"), /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0x79),       &Generic1G,             CHIPFLAGS_SSFDC,     "K9T1G08UOM/K9K1G08U0A/M"), /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0xA1),       &Samsung1G_2K,          CHIPFLAGS_2KB_8BIT,  "K9F1G08Q0M"),              /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0xB1),       &Samsung1G_2K,          CHIPFLAGS_2KB_16BIT, "K9F1G16Q0M"),              /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0xC1),       &Samsung1G_2K,          CHIPFLAGS_2KB_16BIT, "K9F1G16D0M/U0M"),          /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0xF1),       &Samsung1G_2K,          CHIPFLAGS_2KB_8BIT,  "K9F1G08D0M/U0A/M"),        /* Note 1 */
        DECLARE_NAND_CHIP(1, ID_1(0x39),       &Generic64 ,            CHIPFLAGS_SSFDC,     "K9F6408Q0C"),              /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x43),       &Generic128,            CHIPFLAGS_512_16BIT, "K9F2816Q0C"),              /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x53),       &Generic128,            CHIPFLAGS_512_16BIT, "K9F2816U0C"),              /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x33),       &Generic128,            CHIPFLAGS_SSFDC,     "K9F2808Q0B/C"),            /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x35),       &Generic256,            CHIPFLAGS_SSFDC,     "K9F5608Q0B/C"),            /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x36),       &Generic512,            CHIPFLAGS_SSFDC,     "K9F1208Q0A/B/K9K1208Q0B/C"), /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x45),       &Generic256,            CHIPFLAGS_512_16BIT, "K9F5616Q0B/C"),            /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x56),       &Generic512,            CHIPFLAGS_512_16BIT, "K9F1216D0A/B/U0A/B/K9K1216D0C/U0A/C"), /* Note 2 */
        DECLARE_NAND_CHIP(1, ID_1(0x55),       &Generic256,            CHIPFLAGS_512_16BIT, "K9F5616D0C/U0B/C"),        /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0x72),       &Generic1G,             CHIPFLAGS_512_16BIT, "K9K1G16Q0A"),              /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0x74),       &Generic1G,             CHIPFLAGS_512_16BIT, "K9K1G16U0A"),              /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0x78),       &Generic1G,             CHIPFLAGS_SSFDC,     "K9K1G08Q0A"),              /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xAA),       &Samsung2G_2K,          CHIPFLAGS_2KB_8BIT,  "K9F2G08Q0M/K9K2G08Q0M"),   /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xAC),       &Samsung4G_2K,          CHIPFLAGS_2KB_8BIT,  "K9K4G08Q0M"),              /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xBA),       &Samsung2G_2K,          CHIPFLAGS_2KB_16BIT, "K9F2G16Q0M/K9K2G16Q0M"),   /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xBC),       &Samsung4G_2K,          CHIPFLAGS_2KB_16BIT, "K9K4G16Q0M"),              /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xCA),       &Samsung2G_2K,          CHIPFLAGS_2KB_16BIT, "K9F2G16U0M/K9K2G16U0M"),   /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xCC),       &Samsung4G_2K,          CHIPFLAGS_2KB_16BIT, "K9K4G16U0M"),              /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xDA),       &Samsung2G_2K,          CHIPFLAGS_2KB_8BIT,  "K9F2G08U0M/K9K2G08U0A/M"), /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xDC),       &Samsung4G_2K,          CHIPFLAGS_2KB_8BIT,  "K9K4G08U0M"),              /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xD3),       &Samsung8G_2K,          CHIPFLAGS_2KB_8BIT,  "K9K8G08U0A/K9WAG08U1A/K9NBG08U5A"), /* Note 3 */
        DECLARE_NAND_CHIP(1, ID_1(0xD5),       &Samsung16G_4K,         CHIPFLAGS_4KB_8BIT, "K9KAG08U0M")                /* Note 3 */
    };

    for(i = 0; i < DCLDIMENSIONOF(Samsung); ++i)
    {
        bMatch = TRUE;
        for (j=0; j<Samsung[i].uDevIdLength; j++)
        {
            if (Samsung[i].aucDevID[j] != aucID[1+j])
                bMatch = FALSE;
        }
        
        if (bMatch)
        {
            pChip = &Samsung[i];
            break;
        }
     }

    return pChip;
}

/*---------------------------------------------------------------------*/
/*----------------------------- Samsung Table ------------------------*/
/*-----------------------------       End      ------------------------*/
/*---------------------------------------------------------------------*/
#endif


#if FFXCONF_NANDSUPPORT_SPANSION
/*---------------------------------------------------------------------*/
/*--------------------------   Spansion Table    ----------------------*/
/*-----------------------------     Begin      ------------------------*/
/*---------------------------------------------------------------------*/
typedef struct FFXSPANSIONNANDCHIP
{
    /*
     * Common, module-wide FFXNANDCHIP structure, visible to the caller
     */
    FFXNANDCHIP Chip;

    /* Third byte of ID, for comparison (indicates whether ECC or ECC-free) */
    D_UINT8     bDevID3;

} FFXSPANSIONNANDCHIP;

#if VERBOSE_CHIP_INFO
#define DECLARE_SPANSIONNAND_CHIP(id1, id3, clas, flg, nam)  { {1, ID_1(id1),(flg),(nam),(clas) }, (id3) }
#else
#define DECLARE_SPANSIONNAND_CHIP(id1, id3, clas, flg, nam)  { {1, ID_1(id1), (flg),(clas) }, (id3) }
#endif

/*-------------------------------------------------------------------
    Local: SpansionFindChip()

    This is the Spansion-specific implementation of the FindChip()
    function.  This function searches the static tables for a
    chip data structure that matches the ID passed in as an
    argument, and returns the address of that structure.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a constant FFXNANDCHIP structure if
        successful, NULL otherwise.  Note that the pointer is to a
        static structure, and as such is const (and the caller
        must not modify).  Also, the pointer value can be used
        in comparison operations to determine if two chips are
        of identicial configurations.
-------------------------------------------------------------------*/
static const FFXNANDCHIP *SpansionFindChip(
    const D_UCHAR      *aucID)
{
    const FFXNANDCHIP  *pChip = NULL;
    unsigned int        i;

    static const FFXNANDCHIPCLASS SpnML_128    =  {PAGESIZE_512,  ADDR_LINES_128,  DEVSIZE_PARAMS(128, 512),  EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, GENERIC     };
    static const FFXNANDCHIPCLASS SpnML_256    =  {PAGESIZE_512,  ADDR_LINES_256,  DEVSIZE_PARAMS(256, 512),  EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, GENERIC     };
    static const FFXNANDCHIPCLASS SpnML_512    =  {PAGESIZE_512,  ADDR_LINES_512,  DEVSIZE_PARAMS(512, 512),  EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, GENERIC     };
    static const FFXNANDCHIPCLASS SpnMS_512_2K =  {PAGESIZE_2048, ADDR_LINES_512,  DEVSIZE_PARAMS(512, 128),  EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, ORNAND2KOPS };
 /* static const FFXNANDCHIPCLASS SpnMS_1G_2K  =  {PAGESIZE_2048, ADDR_LINES_1024, DEVSIZE_PARAMS(1024, 128), EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, ORNAND2KOPS }; */
    static const FFXNANDCHIPCLASS SpnMS_2G_2K  =  {PAGESIZE_2048, ADDR_LINES_2048, DEVSIZE_PARAMS(2048, 128), EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, ORNAND2KOPS };
 /* static const FFXNANDCHIPCLASS SpnMS_4G_2K  =  {PAGESIZE_2048, ADDR_LINES_4096, DEVSIZE_PARAMS(4096, 128), EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, ORNAND2KOPS }; */
    static const FFXNANDCHIPCLASS SpnML_1G_2K  =  {PAGESIZE_2048, ADDR_LINES_1024, DEVSIZE_PARAMS(1024, 512), EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
    static const FFXNANDCHIPCLASS SpnML_2G_2K  =  {PAGESIZE_2048, ADDR_LINES_2048, DEVSIZE_PARAMS(2048, 512), EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
    static const FFXNANDCHIPCLASS SpnML_4G_2K  =  {PAGESIZE_2048, ADDR_LINES_4096, DEVSIZE_PARAMS(4096, 512), EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
 /* static const FFXNANDCHIPCLASS SpnML_8G_2K  =  {PAGESIZE_2048, ADDR_LINES_8192, DEVSIZE_PARAMS(8192, 512), EDC_1_BIT, 0, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS}; */

    /*  List of all AMD UltraNAND (Am30LV) and Spansion ORNAND (S30MS and S30ML) chips.
        S30MS is 1.8V and S30ML is 3.3V.  In both families, the 2nd device code shows
        whether ECC-required (up to 2% bad blocks) or ECC-free (100% good blocks).

        The first generation of ORNAND (90nm) is designated with P after the density,
        i.e. S30MS01GP or S30MS512P.  The second generation (65nm) is designated with
        R, i.e. S30MS01GR or S30MS512R, but the device codes do not change between
        different generations (for a given family and density).
    */
    static const FFXSPANSIONNANDCHIP Spansion[] =
    {
        DECLARE_SPANSIONNAND_CHIP(0xE6, 0x00, &Generic64,    CHIPINT_8BIT,                     "Am30LV0064D"),          /* UltraNAND */
        DECLARE_SPANSIONNAND_CHIP(0xE6, 0x01, &Generic64,    CHIPINT_8BIT|CHIPFBB_NONE,        "Am30LV0064D-noECC"),    /* UltraNAND */
        DECLARE_SPANSIONNAND_CHIP(0x73, 0x00, &SpnML_128,    CHIPINT_8BIT|CHIPFBB_SSFDC,       "S30ML128xxxxxx50x"),
        DECLARE_SPANSIONNAND_CHIP(0x73, 0x01, &SpnML_128,    CHIPINT_8BIT|CHIPFBB_NONE,        "S30ML128xxxxxx50x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0x53, 0x00, &SpnML_128,    CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30ML128xxxxxx51x"),
        DECLARE_SPANSIONNAND_CHIP(0x53, 0x01, &SpnML_128,    CHIPINT_16BIT|CHIPFBB_NONE,       "S30ML128xxxxxx51x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0x75, 0x00, &SpnML_256,    CHIPINT_8BIT|CHIPFBB_SSFDC,       "S30ML256xxxxxx50x"),
        DECLARE_SPANSIONNAND_CHIP(0x75, 0x01, &SpnML_256,    CHIPINT_8BIT|CHIPFBB_NONE,        "S30ML256xxxxxx50x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0x55, 0x00, &SpnML_256,    CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30ML256xxxxxx51x"),
        DECLARE_SPANSIONNAND_CHIP(0x55, 0x01, &SpnML_256,    CHIPINT_16BIT|CHIPFBB_NONE,       "S30ML256xxxxxx51x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0x76, 0x00, &SpnML_512,    CHIPINT_8BIT|CHIPFBB_SSFDC,       "S30ML512xxxxxx50x"),
        DECLARE_SPANSIONNAND_CHIP(0x76, 0x01, &SpnML_512,    CHIPINT_8BIT|CHIPFBB_NONE,        "S30ML512xxxxxx50x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0x56, 0x00, &SpnML_512,    CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30ML512xxxxxx51x"),
        DECLARE_SPANSIONNAND_CHIP(0x56, 0x01, &SpnML_512,    CHIPINT_16BIT|CHIPFBB_NONE,       "S30ML512xxxxxx51x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0x81, 0x00, &SpnMS_512_2K, CHIPINT_8BIT|CHIPFBB_OFFSETZERO,  "S30MS512xxxxxx50x"),
        DECLARE_SPANSIONNAND_CHIP(0x81, 0x01, &SpnMS_512_2K, CHIPINT_8BIT|CHIPFBB_NONE,        "S30MS512xxxxxx50x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0x91, 0x00, &SpnMS_512_2K, CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30MS512xxxxxx51x"),
        DECLARE_SPANSIONNAND_CHIP(0x91, 0x01, &SpnMS_512_2K, CHIPINT_16BIT|CHIPFBB_NONE,       "S30MS512xxxxxx51x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xA1, 0x00, &Samsung1G_2K, CHIPINT_8BIT|CHIPFBB_OFFSETZERO,  "S30MS01Gxxxxxx50x"),
        DECLARE_SPANSIONNAND_CHIP(0xA1, 0x01, &Samsung1G_2K, CHIPINT_8BIT|CHIPFBB_NONE,        "S30MS01Gxxxxxx50x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xB1, 0x00, &Samsung1G_2K, CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30MS01Gxxxxxx51x"),
        DECLARE_SPANSIONNAND_CHIP(0xB1, 0x01, &Samsung1G_2K, CHIPINT_16BIT|CHIPFBB_NONE,       "S30MS01Gxxxxxx51x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xF1, 0x00, &SpnML_1G_2K,  CHIPINT_8BIT|CHIPFBB_OFFSETZERO,  "S30ML01Gxxxx00x"),
        DECLARE_SPANSIONNAND_CHIP(0xF1, 0x01, &SpnML_1G_2K,  CHIPINT_8BIT|CHIPFBB_NONE,        "S30ML01Gxxxx00x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xC1, 0x00, &SpnML_1G_2K,  CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30ML01Gxxxx01x"),
        DECLARE_SPANSIONNAND_CHIP(0xC1, 0x01, &SpnML_1G_2K,  CHIPINT_16BIT|CHIPFBB_NONE,       "S30ML01Gxxxx01x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xDA, 0x00, &SpnML_2G_2K,  CHIPINT_8BIT|CHIPFBB_OFFSETZERO,  "S30ML02Gxxxx00x"),
        DECLARE_SPANSIONNAND_CHIP(0xDA, 0x01, &SpnML_2G_2K,  CHIPINT_8BIT|CHIPFBB_NONE,        "S30ML02Gxxxx00x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xCA, 0x00, &SpnML_2G_2K,  CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30ML02Gxxxx01x"),
        DECLARE_SPANSIONNAND_CHIP(0xCA, 0x01, &SpnML_2G_2K,  CHIPINT_16BIT|CHIPFBB_NONE,       "S30ML02Gxxxx01x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xDC, 0x00, &SpnML_4G_2K,  CHIPINT_8BIT|CHIPFBB_OFFSETZERO,  "S30ML04Gxxxx00x"),
        DECLARE_SPANSIONNAND_CHIP(0xDC, 0x01, &SpnML_4G_2K,  CHIPINT_8BIT|CHIPFBB_NONE,        "S30ML04Gxxxx00x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xCC, 0x00, &SpnML_4G_2K,  CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30ML04Gxxxx01x"),
        DECLARE_SPANSIONNAND_CHIP(0xCC, 0x01, &SpnML_4G_2K,  CHIPINT_16BIT|CHIPFBB_NONE,       "S30ML04Gxxxx01x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xAA, 0x00, &SpnMS_2G_2K,  CHIPINT_8BIT|CHIPFBB_OFFSETZERO,  "S30MS02Gxxxxxx50x"),
        DECLARE_SPANSIONNAND_CHIP(0xAA, 0x01, &SpnMS_2G_2K,  CHIPINT_8BIT|CHIPFBB_NONE,        "S30MS02Gxxxxxx50x-noECC"),
        DECLARE_SPANSIONNAND_CHIP(0xBA, 0x00, &SpnMS_2G_2K,  CHIPINT_16BIT|CHIPFBB_OFFSETZERO, "S30MS02Gxxxxxx51x"),
        DECLARE_SPANSIONNAND_CHIP(0xBA, 0x01, &SpnMS_2G_2K,  CHIPINT_16BIT|CHIPFBB_NONE,       "S30MS02Gxxxxxx51x-noECC"),
    };


    for(i = 0; i < DCLDIMENSIONOF(Spansion); ++i)
    {
        if(Spansion[i].Chip.aucDevID[0] == aucID[1])
        {
            if (Spansion[i].bDevID3 == aucID[2])
            {
                /*  Return a pointer to the chip structure.
                    Note that while a pointer to the Spansion-specific
                    structure is equivelant to a pointer to the FFXNANDCHIP
                    structure, we use the more formal return here and
                    whereever else possible, for additional safety,
                    by avoiding typecasts that could otherwise shield a
                    real problem from the compiler.
                 */
                pChip = &Spansion[i].Chip;
                break;
            }
        }
    }

    return pChip;
}

/*---------------------------------------------------------------------*/
/*----------------------------- Spansion Table ------------------------*/
/*-----------------------------       End      ------------------------*/
/*---------------------------------------------------------------------*/
#endif


#if FFXCONF_NANDSUPPORT_HYNIX
/*---------------------------------------------------------------------*/
/*----------------------------   Hynix Table   ------------------------*/
/*-----------------------------     Begin      ------------------------*/
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Local: HynixFindChip()

    This is the Hynix-specific implementation of the FindChip()
    function.  This function searches the static tables for a
    chip data structure that matches the ID passed in as an
    argument, and returns the address of that structure.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a constant FFXNANDCHIP structure if
        successful, NULL otherwise.  Note that the pointer is to a
        static structure, and as such is const (and the caller
        must not modify).  Also, the pointer value can be used
        in comparison operations to determine if two chips are
        of identicial configurations.
-------------------------------------------------------------------*/
static const FFXNANDCHIP *HynixFindChip(
    const D_UCHAR              *aucID)
{
    const FFXNANDCHIP          *pChip = NULL;
    unsigned int                i,j;
    D_BOOL                      bMatch;
    static const FFXNANDCHIP    Hynix[] =
    {
        /*  The algorithm used to match devices in this table is a strict
            exact byte-for-byte match of not only the individual id bytes,
            but also for the length of the sequence. Device listings in this
            table will only be matched up to the listed length of the sequence,
            which is the first entry in the DECLARE_NAND_CHIP macro.

            Because of this, the order in which entries is important for
            devices that have duplicate bytes in their entries up to some point
            in the sequence. Devices with the most bytes in their sequences
            *must* be listed first in the table. Otherwise, such a device will
            get compared first against the table entry that has one or more
            duplicate bytes early in its sequence, but the full sequence will
            not get examined and a false match will occur.
        */
        DECLARE_NAND_CHIP(1, ID_1(0x35), &Generic256,   CHIPFLAGS_SSFDC,     "HY27SS08561M"),
        DECLARE_NAND_CHIP(1, ID_1(0x36), &Generic512,   CHIPFLAGS_SSFDC,     "HY27SS08121M/A"),
        DECLARE_NAND_CHIP(1, ID_1(0x45), &Generic256,   CHIPFLAGS_512_16BIT, "HY27SS16561M"),
        DECLARE_NAND_CHIP(1, ID_1(0x46), &Generic512,   CHIPFLAGS_512_16BIT, "HY27SS16121M/A"),
        DECLARE_NAND_CHIP(1, ID_1(0x55), &Generic256,   CHIPFLAGS_512_16BIT, "HY27US16561M"),
        DECLARE_NAND_CHIP(1, ID_1(0x56), &Generic512,   CHIPFLAGS_512_16BIT, "HY27US16121M/A"),
        DECLARE_NAND_CHIP(1, ID_1(0x75), &Generic256,   CHIPFLAGS_SSFDC,     "HY27US08561M"),
        DECLARE_NAND_CHIP(1, ID_1(0x76), &Generic512,   CHIPFLAGS_SSFDC,     "HY27US08121M/A"),
        DECLARE_NAND_CHIP(1, ID_1(0xAA), &Samsung2G_2K, CHIPFLAGS_2KB_16BIT, "HY27UG162G2M"),
        DECLARE_NAND_CHIP(1, ID_1(0xBA), &Samsung2G_2K, CHIPFLAGS_2KB_16BIT, "HY27SG162G2M"),
        DECLARE_NAND_CHIP(1, ID_1(0xCA), &Samsung2G_2K, CHIPFLAGS_2KB_8BIT,  "HY27SG082G2M"),
        DECLARE_NAND_CHIP(1, ID_1(0xDA), &Samsung2G_2K, CHIPFLAGS_2KB_8BIT,  "HY27UG082G2M"),
        DECLARE_NAND_CHIP(1, ID_1(0xDC), &Hynix4G_2K, CHIPFLAGS_2KB_8BIT,  "HY27UF084G2B"), /* Nvidia Gracelabs GL*/

        DECLARE_NAND_CHIP(1, ID_1(0xD3), &Samsung8G_2K, CHIPFLAGS_2KB_8BIT,  "HY27UH08AG(5/D)M Series")

        /*  HY27UH08AG(5/D)M Series parts contains two independent 8Gbit dies.
            FfxProjNandSetChipSelect() is responsible for selecting the right
            chip, and FfxProjNandReadyWait() is responsible for checking the
            correct Ready/Busy signal (if used).
        */
    };

    for(i = 0; i < DCLDIMENSIONOF(Hynix); ++i)
    {
        bMatch = TRUE;
        for (j=0; j<Hynix[i].uDevIdLength; j++)
        {
            if (Hynix[i].aucDevID[j] != aucID[1+j])
                bMatch = FALSE;
        }
        
        if (bMatch)
        {
            pChip = &Hynix[i];
            break;
        }
     }

    return pChip;
}

/*---------------------------------------------------------------------*/
/*-----------------------------    Hynix Table   ------------------------*/
/*-----------------------------       End      ------------------------*/
/*---------------------------------------------------------------------*/
#endif


#if FFXCONF_NANDSUPPORT_MICRON
/*---------------------------------------------------------------------*/
/*----------------------------   Micron Table  ------------------------*/
/*-----------------------------     Begin      ------------------------*/
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Local: MicronFindChip()

    This is the Micron-specific implementation of the FindChip()
    function.  This function searches the static tables for a
    chip data structure that matches the ID passed in as an
    argument, and returns the address of that structure.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a constant FFXNANDCHIP structure if
        successful, NULL otherwise.  Note that the pointer is to a
        static structure, and as such is const (and the caller
        must not modify).  Also, the pointer value can be used
        in comparison operations to determine if two chips are
        of identicial configurations.
-------------------------------------------------------------------*/
static const FFXNANDCHIP *MicronFindChip(
    const D_UCHAR      *aucID)
{
    const FFXNANDCHIP  *pChip = NULL;
    unsigned int        i,j;
    D_BOOL              bMatch;

    static const FFXNANDCHIPCLASS Micron2G_2KLock     = {PAGESIZE_2048,    ADDR_LINES_2048,  DEVSIZE_PARAMS(2048, 128),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONLOCK};
    static const FFXNANDCHIPCLASS Micron4G_2KLock     = {PAGESIZE_2048,    ADDR_LINES_4096,  DEVSIZE_PARAMS(4096, 128),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONLOCK};
    static const FFXNANDCHIPCLASS Micron8G_2KLock     = {PAGESIZE_2048,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192, 128),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONLOCK};
    static const FFXNANDCHIPCLASS Micron4G_2KDP       = {PAGESIZE_2048,    ADDR_LINES_4096,  DEVSIZE_PARAMS(4096, 128),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONDPOPS};
    static const FFXNANDCHIPCLASS Micron8G_2KDP       = {PAGESIZE_2048,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192, 128),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONDPOPS};
#if 0 /* for future, presently unused. */
    static const FFXNANDCHIPCLASS Micron4G_2KDPLock   = {PAGESIZE_2048,    ADDR_LINES_4096,  DEVSIZE_PARAMS(4096, 128),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONDPLOCK};
    static const FFXNANDCHIPCLASS Micron8G_2KDPLock   = {PAGESIZE_2048,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192, 128),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONDPLOCK};
#endif 
    static const FFXNANDCHIPCLASS Micron4G_2KM60Lock  = {PAGESIZE_2048,    ADDR_LINES_4096,  DEVSIZE_PARAMS(4096, 128),  EDC_4_BIT,  EDC_4_BIT, MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONM60LOCK};
    static const FFXNANDCHIPCLASS Micron8G_2KM60Lock  = {PAGESIZE_2048,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192, 128),  EDC_4_BIT,  EDC_4_BIT, MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONM60LOCK}; 
    static const FFXNANDCHIPCLASS Micron4G_2KM60      = {PAGESIZE_2048,    ADDR_LINES_4096,  DEVSIZE_PARAMS(4096, 128),  EDC_4_BIT,  EDC_4_BIT, MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONM60};
    static const FFXNANDCHIPCLASS Micron8G_2KM60      = {PAGESIZE_2048,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192, 128),  EDC_4_BIT,  EDC_4_BIT, MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONM60}; 

    /*  Micron MLC chip classes:
    */
    static const FFXNANDCHIPCLASS MicronMLC64G_4KDP   = {MLCPAGESIZE_4096, ADDR_LINES_65536, DEVSIZE_PARAMS(65536, 512), EDC_12_BIT, EDC_NONE,  MAX_CYCLE_10K,     RESERVED_UNKNOWN, MICRONMLC};
    static const FFXNANDCHIPCLASS MicronMLC32G_4KDP   = {MLCPAGESIZE_4096, ADDR_LINES_32768, DEVSIZE_PARAMS(32768, 512), EDC_12_BIT, EDC_NONE,  MAX_CYCLE_10K,     RESERVED_UNKNOWN, MICRONMLC};
    static const FFXNANDCHIPCLASS MicronMLC16G_4KDP   = {MLCPAGESIZE_4096, ADDR_LINES_16384, DEVSIZE_PARAMS(16384, 512), EDC_12_BIT, EDC_NONE,  MAX_CYCLE_10K,     RESERVED_UNKNOWN, MICRONMLC};
    static const FFXNANDCHIPCLASS MicronMLC16G_2KDP   = {PAGESIZE_2048,    ADDR_LINES_16384, DEVSIZE_PARAMS(16384, 256), EDC_4_BIT,  EDC_NONE,  MAX_CYCLE_10K,     RESERVED_UNKNOWN, MICRONMLC};
    static const FFXNANDCHIPCLASS MicronMLC08G_2KDP   = {PAGESIZE_2048,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192,  256), EDC_4_BIT,  EDC_NONE,  MAX_CYCLE_10K,     RESERVED_UNKNOWN, MICRONMLC};

    /*  The following are a peculiar Micron configuration that is SLC technology,
        but has MLC-sized spare area. FlashFX will ignore the extra spare area
        bytes.
    */
    static const FFXNANDCHIPCLASS Micron8G_4KDP       = {PAGESIZE_4096,    ADDR_LINES_8192,  DEVSIZE_PARAMS(8192, 256),  EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONDPOPS};
    static const FFXNANDCHIPCLASS Micron16G_4KDP      = {PAGESIZE_4096,    ADDR_LINES_16384, DEVSIZE_PARAMS(16384, 256), EDC_1_BIT,  EDC_NONE,  MAX_CYCLE_INVALID, RESERVED_UNKNOWN, MICRONDPOPS};

    /*  List of all Micron chips.

        Note that Micron chips have the same command set as Samsung chips
        (as far as what this FIM uses), so can use the same operations.
    */
    static const FFXNANDCHIP Micron[] =
    {
        /*  The algorithm used to match devices in this table is a strict
            exact byte-for-byte match of not only the individual id bytes,
            but also for the length of the sequence. Device listings in this
            table will only be matched up to the listed length of the sequence,
            which is the first entry in the DECLARE_NAND_CHIP macro.

            Because of this, the order in which entries is important for
            devices that have duplicate bytes in their entries up to some point
            in the sequence. Devices with the most bytes in their sequences
            *must* be listed first in the table. Otherwise, such a device will
            get compared first against the table entry that has one or more
            duplicate bytes early in its sequence, but the full sequence will
            not get examined and a false match will occur.

            Note that most Micron devices support multi-chip-enable versions of
            the devices in these tables. None of the part numbers listed in
            these tables represent multi-CE configurations. Since such devices
            do not encode the fact that a multi-CE package is being used and
            only give information about the individual die configurations, it
            is up to your project hooks to be able to sort out the chip-enable
            configuration and deduce the appropriate FlashFX device array size.

            Note also that older Micron devices used the ID codes to distinguish
            Vcc and bus width configurations. This table does not attempt to
            cover all combinations of those. Consult your datasheet.
        */
        DECLARE_NAND_CHIP(4, ID_4(0xDC, 0x90, 0x95, 0x54),   &Micron4G_2KDP,      CHIPFLAGS_2KB_8BIT,  "MT29F4G08AAA/C;MT29F8G08DAA"),  /* 3.3v m40a/m50a   */
        DECLARE_NAND_CHIP(4, ID_4(0xD3, 0xD1, 0x95, 0x58),   &Micron8G_2KDP,      CHIPFLAGS_2KB_8BIT,  "MT29F8G08BAA/MT29F16G08FAA"),
        DECLARE_NAND_CHIP(4, ID_4(0xD3, 0x90, 0x2E, 0x64),   &Micron8G_4KDP,      CHIPFLAGS_2KB_8BIT,  "MT29F8G08AAA/MT29F16G08DAA"),
        DECLARE_NAND_CHIP(4, ID_4(0xD5, 0xD1, 0x2E, 0x68),   &Micron16G_4KDP,     CHIPFLAGS_2KB_8BIT,  "MT29F32G08FAA"),

        /*  Declarations below are Micron MLC.
        */
        DECLARE_NAND_CHIP(4, ID_4(0xD3, 0x94, 0xA5, 0x64),   &MicronMLC08G_2KDP,  CHIPFLAGS_2KB_8BIT,  "MT29F8G08MAA/MT29F16G08QAA"),
        DECLARE_NAND_CHIP(4, ID_4(0xD5, 0xD5, 0xA5, 0x68),   &MicronMLC16G_2KDP,  CHIPFLAGS_2KB_8BIT,  "MT29F32G08TAA"),
        DECLARE_NAND_CHIP(4, ID_4(0xD3, 0x94, 0x2D, 0x64),   &MicronMLC08G_2KDP,  CHIPFLAGS_2KB_8BIT,  "MT29F8G08MAD"),
        DECLARE_NAND_CHIP(4, ID_4(0xD5, 0x94, 0x3E, 0x74),   &MicronMLC16G_4KDP,  CHIPFLAGS_4KB_8BIT,  "MT29F16G08MAA"),
        DECLARE_NAND_CHIP(4, ID_4(0xD7, 0xD5, 0x3E, 0x78),   &MicronMLC32G_4KDP,  CHIPFLAGS_4KB_8BIT,  "MT29F32G08QAA/MT29F64G08TAA"),
        DECLARE_NAND_CHIP(4, ID_4(0xD7, 0x94, 0x3E, 0x74),   &MicronMLC32G_4KDP,  CHIPFLAGS_4KB_8BIT,  "MT29F32G08MAA/MT29F64G08QAA"),
        DECLARE_NAND_CHIP(4, ID_4(0xD9, 0xD5, 0x3E, 0x78),   &MicronMLC64G_4KDP,  CHIPFLAGS_4KB_8BIT,  "MT29F128G08TAA"),

        /*  Declarations in this section are Micron M60 family.
        */
        DECLARE_NAND_CHIP(4, ID_4(0xDC, 0x90, 0x95, 0x56),   &Micron4G_2KM60,     CHIPFLAGS_2KB_8BIT,  "MT29F4G08ABADA"),  /* 3.3v 4G m60a x8  */
        DECLARE_NAND_CHIP(4, ID_4(0xCC, 0x90, 0xD5, 0x56),   &Micron4G_2KM60,     CHIPFLAGS_2KB_16BIT, "MT29F4G16ABADA"),  /* 3.3v 4G m60a x16 */
        DECLARE_NAND_CHIP(4, ID_4(0xAC, 0x90, 0x15, 0x56),   &Micron4G_2KM60Lock, CHIPFLAGS_2KB_8BIT,  "MT29F4G08ABBDA"),  /* 1.8v 4G m60a x8  */
        DECLARE_NAND_CHIP(4, ID_4(0xBC, 0x90, 0x55, 0x56),   &Micron4G_2KM60Lock, CHIPFLAGS_2KB_16BIT, "MT29F4G16ABBDA"),  /* 1.8v 4G m60a x16 */
        DECLARE_NAND_CHIP(4, ID_4(0xA3, 0xD1, 0x15, 0x5A),   &Micron8G_2KM60Lock, CHIPFLAGS_2KB_8BIT,  "MT29F8G08ADBDA"),  /* 1.8v 8G m60a x8  */
        DECLARE_NAND_CHIP(4, ID_4(0xB3, 0xD1, 0x55, 0x5A),   &Micron8G_2KM60Lock, CHIPFLAGS_2KB_16BIT, "MT29F8G16ADBDA"),  /* 1.8v 8G m60a x16 */
        DECLARE_NAND_CHIP(4, ID_4(0xD3, 0xD1, 0x95, 0x5A),   &Micron8G_2KM60,     CHIPFLAGS_2KB_8BIT,  "MT29F8G08ADADA"),  /* 3.3v 8G m60a x8  */
        DECLARE_NAND_CHIP(4, ID_4(0xC3, 0xD1, 0xD5, 0x5A),   &Micron8G_2KM60,     CHIPFLAGS_2KB_16BIT, "MT29F8G16ADADA"),  /* 3.3v 8G m60a x16 */

        /*  Older supported Micron devices
        */
        DECLARE_NAND_CHIP(3, ID_3(0xDC, ID_DONT_CARE, 0x15), &Samsung4G_2K,       CHIPFLAGS_2KB_8BIT,  "MT29F4G08BAB/MT298G08FAB"),  /* 3.3v m29b x8      */
        DECLARE_NAND_CHIP(3, ID_3(0xDC, ID_DONT_CARE, 0x55), &Samsung4G_2K,       CHIPFLAGS_2KB_16BIT, "MT29F4G16BAB"),              /* 3.3v m29b x16     */
        DECLARE_NAND_CHIP(1, ID_1(0xDA),                     &Samsung2G_2K,       CHIPFLAGS_2KB_8BIT,  "MT29F2G08AAB/D"),            /* 3.3v m29b/m59a x8 */
        DECLARE_NAND_CHIP(1, ID_1(0xCA),                     &Samsung2G_2K,       CHIPFLAGS_2KB_16BIT, "MT29F2G16AAB/D"),            /* 3.3v m29b/m59a    */
        DECLARE_NAND_CHIP(1, ID_1(0xAA),                     &Micron2G_2KLock,    CHIPFLAGS_2KB_8BIT,  "MT29F2G08ABD"),              /* 1.8v m59a         */
        DECLARE_NAND_CHIP(1, ID_1(0xBA),                     &Micron2G_2KLock,    CHIPFLAGS_2KB_16BIT, "MT29F2G16ABD"),              /* 1.8v m59a         */
        DECLARE_NAND_CHIP(1, ID_1(0xAC),                     &Micron4G_2KLock,    CHIPFLAGS_2KB_8BIT,  "MT29F4G08ABC"),              /* 1.8v m50a         */
        DECLARE_NAND_CHIP(1, ID_1(0xBC),                     &Micron4G_2KLock,    CHIPFLAGS_2KB_16BIT, "MT29F4G16ABC"),              /* 1.8v m50a         */
        DECLARE_NAND_CHIP(1, ID_1(0xCC),                     &Samsung4G_2K,       CHIPFLAGS_2KB_16BIT, "MT29F4G16BAB/MT29F8G08FAB"), /* 3.3v m29b         */
        DECLARE_NAND_CHIP(1, ID_1(0xA1),                     &Samsung1G_2K,       CHIPFLAGS_2KB_8BIT,  "MT29F1G08ABA"),              /* 1.8v m28a         */
        DECLARE_NAND_CHIP(1, ID_1(0xB1),                     &Samsung1G_2K,       CHIPFLAGS_2KB_16BIT, "MT29F1G16ABA"),              /* 1.8v m28a         */
        DECLARE_NAND_CHIP(1, ID_1(0xA3),                     &Micron8G_2KLock,    CHIPFLAGS_2KB_8BIT,  "MT29F8G08???"),              /* 1.8v m50a ?       */
        DECLARE_NAND_CHIP(1, ID_1(0xB3),                     &Micron8G_2KLock,    CHIPFLAGS_2KB_16BIT, "MT29F8G16???")               /* 1.8v m50a ?       */
    };

    for(i = 0; i < DCLDIMENSIONOF(Micron); ++i)
    {
        bMatch = TRUE;
        for (j=0; j<Micron[i].uDevIdLength; j++)
        {
            if (Micron[i].aucDevID[j] != ID_DONT_CARE)
            {
                if (j == 3)
                {
                    /*  Ignore the "factory ECC enabled" bit in the fourth ID byte.
                    */
                    if (Micron[i].aucDevID[j] != (aucID[1+j] & (~(MICRONID4_ECC_ENABLED))))
                        bMatch = FALSE;
                }
                else if (Micron[i].aucDevID[j] != aucID[1+j])
                {
                    bMatch = FALSE;
                }
            }
        }
        
        if (bMatch)
        {
            pChip = &Micron[i];
            break;
        }
     }

    return pChip;
}

/*---------------------------------------------------------------------*/
/*-----------------------------  Micron Table  ------------------------*/
/*-----------------------------       End      ------------------------*/
/*---------------------------------------------------------------------*/
#endif


#if FFXCONF_NANDSUPPORT_TOSHIBA
/*---------------------------------------------------------------------*/
/*----------------------------  Toshiba Table  ------------------------*/
/*-----------------------------     Begin      ------------------------*/
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Local: ToshibaFindChip()

    This is the Toshiba-specific implementation of the FindChip()
    function.  This function searches the static tables for a
    chip data structure that matches the ID passed in as an
    argument, and returns the address of that structure.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a constant FFXNANDCHIP structure if
        successful, NULL otherwise.  Note that the pointer is to a
        static structure, and as such is const (and the caller
        must not modify).  Also, the pointer value can be used
        in comparison operations to determine if two chips are
        of identicial configurations.
-------------------------------------------------------------------*/
static const FFXNANDCHIP *ToshibaFindChip(
    const D_UCHAR              *aucID)
{
    const FFXNANDCHIP          *pChip = NULL;
    unsigned                    i, j;
    D_BOOL                      bMatch;
    static const FFXNANDCHIP    Toshiba[] =
    {
        /*  The algorithm used to match devices in this table is a strict
            exact byte-for-byte match of not only the individual ID bytes,
            but also for the length of the sequence.  Device listings in this
            table will only be matched up to the listed length of the sequence,
            which is the first entry in the DECLARE_NAND_CHIP macro.

            Because of this, the order in which entries is important for
            devices that have duplicate bytes in their entries up to some point
            in the sequence. Devices with the most bytes in their sequences
            *must* be listed first in the table.  Otherwise, such a device will
            get compared first against the table entry that has one or more
            duplicate bytes early in its sequence, but the full sequence will
            not get examined and a false match will occur.

            These are devices that have been tested and/or were historically
            recognized.

            Important note on the use of old-style Toshiba chips: Some of the
            chips in this table use the legacy "AnyBitZero" factory-bad-block
            marking method.  This release of FlashFX won't format such devices.
            Consult your datasheet.  If you wish to use such a device, please
            contact Datalight Customer Service.
        */
        DECLARE_NAND_CHIP(1, ID_1(0xE5), &Generic32,        CHIPFLAGS_TOSHIBA,   "TC58V32AFT/ADC/DC"),
        DECLARE_NAND_CHIP(1, ID_1(0xE6), &Generic64,        CHIPFLAGS_TOSHIBA,   "TC58V64AFT/BFT/2BXB"),
        DECLARE_NAND_CHIP(1, ID_1(0x73), &Generic128,       CHIPFLAGS_TOSHIBA,   "TC58128/FT/2AXB/AFT,TC58D(A,V)M72(A,F)1FT00"),
        DECLARE_NAND_CHIP(1, ID_1(0x75), &Generic256,       CHIPFLAGS_TOSHIBA,   "TC58256/FT/2AXB/AFT/DC,TC58D(A,V)M82(A,F)1FT00,TC58DVM82A1XBJ1"),
        DECLARE_NAND_CHIP(1, ID_1(0x76), &Generic512,       CHIPFLAGS_TOSHIBA,   "TC58512FT,TC58DVM92A1FT00"),
        DECLARE_NAND_CHIP(1, ID_1(0x79), &Generic1G,        CHIPFLAGS_TOSHIBA,   "TC58DVG02A1FT00,TH58100FT,TY9000A410AMBF"),
        DECLARE_NAND_CHIP(1, ID_1(0xF1), &Generic1G_2K,     CHIPFLAGS_2KB_8BIT,  "TC58NVG0S3AFT00/5"),
        DECLARE_NAND_CHIP(1, ID_1(0xDA), &Generic2G_2K,     CHIPFLAGS_2KB_8BIT,  "TH58NVG1S3AFT00/5"),
        DECLARE_NAND_CHIP(1, ID_1(0xBC), &Generic4G_2K,     CHIPFLAGS_2KB_16BIT, "TY58NYG2S8E"),
        DECLARE_NAND_CHIP(1, ID_1(0xB3), &Generic8G_2K,     CHIPFLAGS_2KB_16BIT, "TY58NYG3S8E"),
        DECLARE_NAND_CHIP(1, ID_1(0xD3), &GenericMLC8G_4K,  CHIPFLAGS_4KB_8BIT,  "TC58NVG3D1DG00"),
        DECLARE_NAND_CHIP(1, ID_1(0xD5), &GenericMLC16G_4K, CHIPFLAGS_4KB_8BIT,  "TC58NVG4D1DG00"),
    };

    for(i = 0; i < DCLDIMENSIONOF(Toshiba); ++i)
    {
        bMatch = TRUE;
        for (j=0; j<Toshiba[i].uDevIdLength; j++)
        {
            if (Toshiba[i].aucDevID[j] != aucID[1+j])
                bMatch = FALSE;
        }

        if (bMatch)
        {
            pChip = &Toshiba[i];
            break;
        }
     }

    return pChip;
}

/*---------------------------------------------------------------------*/
/*-----------------------------  Toshiba Table  ------------------------*/
/*-----------------------------       End      ------------------------*/
/*---------------------------------------------------------------------*/
#endif


#if FFXCONF_NANDSUPPORT_ONENAND
/*---------------------------------------------------------------------*/
/*----------------------------  OneNAND Table  ------------------------*/
/*-----------------------------     Begin      ------------------------*/
/*---------------------------------------------------------------------*/

/*  Declare the OneNAND chip structure.
*/

typedef struct FFXONENANDCHIP
{
	FFXNANDCHIP				chip;
    D_UINT8                 ucDevID2;
} FFXONENANDCHIP;

#if VERBOSE_CHIP_INFO
  #define DECLARE_ONENAND_CHIP(id1, id2, clas, flg, nam)  { {1, ID_1(id1),(flg),(nam),(clas) }, (id2) }
#else
  #define DECLARE_ONENAND_CHIP(id1, id2, clas, flg, nam)  { {1, ID_1(id1), (flg),(clas) }, (id2) }
#endif

/*-------------------------------------------------------------------
    Private: OneNANDFindChip()

    Look up the device ID read from a OneNAND chip, and fill in
    the information needed at run time to operate the chip.

    Parameters:
        pucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns the pChip structure describing the OneNAND
        device, or NULL if the device was not found in the table
-------------------------------------------------------------------*/
const FFXNANDCHIP *OneNANDFindChip(
    const D_UCHAR                  *aucID)
{
    FFXNANDCHIP const              *pChip;
    static const FFXNANDCHIPCLASS   OneNAND4G_2K  = { PAGESIZE_2048, ADDR_LINES_4096, DEVSIZE_PARAMS(4096, 128), EDC_1_BIT, EDC_1_BIT, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
    static const FFXNANDCHIPCLASS   OneNAND2G_2K  = { PAGESIZE_2048, ADDR_LINES_2048, DEVSIZE_PARAMS(2048, 128), EDC_1_BIT, EDC_1_BIT, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
    static const FFXNANDCHIPCLASS   OneNAND1G_2K  = { PAGESIZE_2048, ADDR_LINES_1024, DEVSIZE_PARAMS(1024, 128), EDC_1_BIT, EDC_1_BIT, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
    static const FFXNANDCHIPCLASS   OneNAND512_2K = { PAGESIZE_2048, ADDR_LINES_512,  DEVSIZE_PARAMS( 512, 128), EDC_1_BIT, EDC_1_BIT, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG2KOPS};
    static const FFXNANDCHIPCLASS   OneNAND256_1K = { PAGESIZE_1024, ADDR_LINES_256,  DEVSIZE_PARAMS( 256,  64), EDC_1_BIT, EDC_1_BIT, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG1KOPS};
    static const FFXNANDCHIPCLASS   OneNAND128_1K = { PAGESIZE_1024, ADDR_LINES_128,  DEVSIZE_PARAMS( 128,  64), EDC_1_BIT, EDC_1_BIT, MAX_CYCLE_100K, RESERVED_UNKNOWN, SAMSUNG1KOPS};

    /*  For FlexOneNAND devices, we just declare the device as if it were
        configured entirely as SLC. The OneNAND NTM will handle any necessary
        block size adjustments for Flex partitions if they are in use.
    */
    static const FFXNANDCHIPCLASS OneNAND2G_4K_FLEX = { PAGESIZE_4096, ADDR_LINES_4096, DEVSIZE_PARAMS(2048, 256), EDC_4_BIT, EDC_4_BIT, MAX_CYCLE_50K,  RESERVED_UNKNOWN, FLEX1NAND};

    /*  List of all Samsung OneNAND chips.
    */
    static const FFXONENANDCHIP OneNAND[] =
    {
        DECLARE_ONENAND_CHIP(0x35, 0x00, &OneNAND1G_2K,      CHIPFLAGS_2KB_16BIT, "KFG1G16{D|U}2M"),
        DECLARE_ONENAND_CHIP(0x30, 0x00, &OneNAND1G_2K,      CHIPFLAGS_2KB_16BIT, "KFM1G16Q2M-DEB5/G2B-DE{B|D}{6|8}"),
        DECLARE_ONENAND_CHIP(0x48, 0x00, &OneNAND2G_2K,      CHIPFLAGS_2KB_16BIT, "KFN2G16Q2M-DEB5"),
        DECLARE_ONENAND_CHIP(0x34, 0x00, &OneNAND1G_2K,      CHIPFLAGS_2KB_16BIT, "KFG1G16Q2{M|A}"),
        DECLARE_ONENAND_CHIP(0x40, 0x00, &OneNAND2G_2K,      CHIPFLAGS_2KB_16BIT, "KFM2G16Q2{M|A}"),
        DECLARE_ONENAND_CHIP(0x44, 0x00, &OneNAND2G_2K,      CHIPFLAGS_2KB_16BIT, "KFG2G16Q2{M|A}"),
        DECLARE_ONENAND_CHIP(0x4C, 0x00, &OneNAND2G_2K,      CHIPFLAGS_2KB_16BIT, "KF{H2|W4}G16Q2{M|A}"),
        DECLARE_ONENAND_CHIP(0x5C, 0x00, &OneNAND4G_2K,      CHIPFLAGS_2KB_16BIT, "KFH4G16Q2{M|A}"),

        DECLARE_ONENAND_CHIP(0x24, 0x00, &OneNAND512_2K,     CHIPFLAGS_2KB_16BIT, "KF{G|H}1216Q2{M|A}"),
        DECLARE_ONENAND_CHIP(0x25, 0x00, &OneNAND512_2K,     CHIPFLAGS_2KB_16BIT, "KF{G|H}1216{D|U}2{M|A}"),
        DECLARE_ONENAND_CHIP(0x14, 0x00, &OneNAND256_1K,     CHIPFLAGS_1KB_16BIT, "KFG5616Q1A"),
        DECLARE_ONENAND_CHIP(0x15, 0x00, &OneNAND256_1K,     CHIPFLAGS_1KB_16BIT, "KFG5616{D|U}1A"),
        DECLARE_ONENAND_CHIP(0x04, 0x00, &OneNAND128_1K,     CHIPFLAGS_1KB_16BIT, "KFG2816Q1M"),
        DECLARE_ONENAND_CHIP(0x05, 0x00, &OneNAND128_1K,     CHIPFLAGS_1KB_16BIT, "KFG2816{D|U}1M"),

        /*  FlexOneNAND devices are declared using SLC geometry only. The
            OneNAND NTM will make any adjustments necessary as a result of
            MLC partitions.
        */
        DECLARE_ONENAND_CHIP(0x50, 0x02, &OneNAND2G_4K_FLEX, CHIPFLAGS_1KB_16BIT, "KF{G|H|W|M}H6{Q|U}4G{D|U}4M"),
        DECLARE_ONENAND_CHIP(0x51, 0x02, &OneNAND2G_4K_FLEX, CHIPFLAGS_1KB_16BIT, "KF{G|H|W|M}H6{Q|U}4G{D|U}4M"),
        DECLARE_ONENAND_CHIP(0x54, 0x02, &OneNAND2G_4K_FLEX, CHIPFLAGS_1KB_16BIT, "KF{G|H|W|M}H6{Q|U}4G{D|U}4M"),
        DECLARE_ONENAND_CHIP(0x55, 0x02, &OneNAND2G_4K_FLEX, CHIPFLAGS_1KB_16BIT, "KF{G|H|W|M}H6{Q|U}4G{D|U}4M"),
        DECLARE_ONENAND_CHIP(0x6C, 0x02, &OneNAND2G_4K_FLEX, CHIPFLAGS_1KB_16BIT, "KF{G|H|W|M}H6{Q|U}4G{D|U}4M"),
        DECLARE_ONENAND_CHIP(0x6D, 0x02, &OneNAND2G_4K_FLEX, CHIPFLAGS_1KB_16BIT, "KF{G|H|W|M}H6{Q|U}4G{D|U}4M"),

    };

    D_UINT16 uChips = sizeof (OneNAND) / sizeof (FFXNANDCHIP);
    D_UINT16 uCurrentChip;
    D_UINT16 u1NANDChipID = 0;
    D_UINT16 uTableChipID = 0;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, 0), "OneNANDFindChip(0x%02x, 0x%02x)\n", aucID[0], aucID[1]));

    pChip = NULL;

    /*  Assemble the OneNAND Chip ID to compare against the table values.
        In order to preserve our compile and link structure, OneNANDFindChip
        had to be defined using an array of bytes as the input parameter,
        even though OneNAND parts typically do not do this; they use a
        16-bit quantity as the DeviceID.  Unfortunately this means that
        OneNANDReadChipID (in nt1nand.c) has to split up the device ID into
        bytes and put them into aucID, and here we have to reassemble them
        into 16-bit words.

        Note also that we can get away with all this for now because as of
        now there are no OneNAND chips that are NOT 16-bit parts, and thus
        between OneNANDReadChipID and this procedure is OK assuming that
        this is so.  However, if we ever get a OneNAND part that is not
        16-bit, both this procedure and OneNANDReadChipID will have to be
        revisited.
    */
    u1NANDChipID = (((D_UINT16)(aucID[2]<<8)) | ((D_UINT16)(aucID[1])));

    /*  Find the chip:
    */
    for(uCurrentChip = 0; uCurrentChip < uChips; ++uCurrentChip)
    {

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, 0),
            "OneNANDFindChip() comparing to device ID 0x%02x\n", OneNAND[uCurrentChip].chip.aucDevID[0]));

        uTableChipID = (((D_UINT16)((OneNAND[uCurrentChip].ucDevID2)<<8)) |
            ((D_UINT16)(OneNAND[uCurrentChip].chip.aucDevID[0])));
        if(uTableChipID == u1NANDChipID)
        {

            pChip = &(OneNAND[uCurrentChip].chip);
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, 0),
                "OneNANDFindChip() matched device ID 0x%02x\n", OneNAND[uCurrentChip].chip.aucDevID[0]));

            break;
        }
    }

    return pChip;
}

/*---------------------------------------------------------------------*/
/*----------------------------  OneNAND Table  ------------------------*/
/*-----------------------------       End      ------------------------*/
/*---------------------------------------------------------------------*/
#endif

#if FFXCONF_NANDSUPPORT_DLTEST
/*---------------------------------------------------------------------*/
/*---------------------------  Datalight Table  -----------------------*/
/*-----------------------------     Begin      ------------------------*/
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Protected: DlTestFindChip()

    This is Datalight test instrumentation used to identify and setup
    chip configurations for verification purposes.  The chip IDs used
    here are ficticous and designed only for testing specific features
    or characteristics of the product.

    Parameters:
        aucID - A pointer to a NAND_ID_SIZE-byte buffer containing
                the ID read from the chip.

    Return Value:
        Returns a pointer to a constant FFXNANDCHIP structure if
        successful, NULL otherwise.  Note that the pointer is to a
        static structure, and as such is const (and the caller
        must not modify).  Also, the pointer value can be used
        in comparison operations to determine if two chips are
        of identicial configurations.
-------------------------------------------------------------------*/
static const FFXNANDCHIP *DlTestFindChip(
    const D_UCHAR      *aucID)
{
    const FFXNANDCHIP  *pChip = NULL;
    unsigned            ii, jj;
    D_BOOL              fMatch;

    /*  The following chips are all set up to describe specific geometries 
        for various tests.  Do not change any of these without careful
        attention to any test and validation code that makes use of the
        related MFG and device ID codes.
    */
    static const FFXNANDCHIPCLASS ChipClass1M64K1024B =           {PAGESIZE_1024,    ADDR_LINES_8,   DEVSIZE_PARAMS(  8,  64), EDC_1_BIT,  EDC_NONE, MAX_CYCLE_100K,  4, GENERIC};
    static const FFXNANDCHIPCLASS ChipClass2M128K1024B =          {PAGESIZE_1024,    ADDR_LINES_16,  DEVSIZE_PARAMS( 16, 128), EDC_1_BIT,  EDC_NONE, MAX_CYCLE_100K,  4, GENERIC};
    static const FFXNANDCHIPCLASS ChipClass512M128K512B =         {PAGESIZE_512,     ADDR_LINES_512, DEVSIZE_PARAMS(512, 128), EDC_1_BIT,  EDC_NONE, MAX_CYCLE_100K, 24, GENERIC};
    static const FFXNANDCHIPCLASS ChipClass512M128K2K =           {PAGESIZE_512,     ADDR_LINES_512, DEVSIZE_PARAMS(512, 128), EDC_1_BIT,  EDC_NONE, MAX_CYCLE_100K, 24, GENERIC};
    static const FFXNANDCHIPCLASS ChipClass512M128K2KOdd =        {MLCPAGESIZE_4096, ADDR_LINES_512, DEVSIZE_PARAMS(512, 128), EDC_1_BIT,  EDC_NONE, MAX_CYCLE_100K, 24, GENERIC};
    static const FFXNANDCHIPCLASS ChipClass512M128K2K2bitEdc =    {PAGESIZE_2048,    ADDR_LINES_512, DEVSIZE_PARAMS(512, 128), EDC_2_BIT,  EDC_NONE, MAX_CYCLE_100K, 24, MOSTMLC};
    static const FFXNANDCHIPCLASS ChipClass512M128K4K4bitEdc =    {MLCPAGESIZE_4096, ADDR_LINES_512, DEVSIZE_PARAMS(512, 128), EDC_4_BIT,  EDC_NONE, MAX_CYCLE_100K, 24, MOSTMLC};
    static const FFXNANDCHIPCLASS ChipClass512M128K4K4bitEdcMlc = {MLCPAGESIZE_4096, ADDR_LINES_512, DEVSIZE_PARAMS(512, 256), EDC_4_BIT,  EDC_NONE, MAX_CYCLE_5K,   12, MOSTMLC};
    static const FFXNANDCHIPCLASS ChipClass128M128K4K4bitEdcMlc = {MLCPAGESIZE_4096, ADDR_LINES_128, DEVSIZE_PARAMS(128, 256), EDC_4_BIT,  EDC_NONE, MAX_CYCLE_5K,   12, MOSTMLC};
    static const FFXNANDCHIPCLASS ChipClassRequires12bitEdc =     {MLCPAGESIZE_4096, ADDR_LINES_512, DEVSIZE_PARAMS(512, 128), EDC_12_BIT, EDC_NONE, MAX_CYCLE_5K,   24, MOSTMLC};

    /* Page size deviations 
    */
    static const FFXNANDCHIPCLASS aChipClassPageSize[] =
    {
        {PAGESIZE_1024,      ADDR_LINES_512,  DEVSIZE_PARAMS( 512, 128), 1, EDC_NONE, MAX_CYCLE_100K, 48, GENERIC},
        {PAGESIZE_2048,      ADDR_LINES_512,  DEVSIZE_PARAMS( 512, 128), 1, EDC_NONE, MAX_CYCLE_100K, 48, GENERIC},
        {MLCPAGESIZE_4096,   ADDR_LINES_512,  DEVSIZE_PARAMS( 512, 128), 1, EDC_NONE, MAX_CYCLE_100K, 48, GENERIC},
        {PAGESIZE_2048X2,    ADDR_LINES_512,  DEVSIZE_PARAMS( 512, 128), 1, EDC_NONE, MAX_CYCLE_100K, 48, GENERIC},
        {PAGESIZE_2048X4,    ADDR_LINES_512,  DEVSIZE_PARAMS( 512, 256), 1, EDC_NONE, MAX_CYCLE_100K, 48, GENERIC},
        {PAGESIZE_2048X8,    ADDR_LINES_1024, DEVSIZE_PARAMS( 512, 512), 1, EDC_NONE, MAX_CYCLE_100K, 48, GENERIC},
        {MLCPAGESIZE_4096X2, ADDR_LINES_1024, DEVSIZE_PARAMS(1024, 512), 1, EDC_NONE, MAX_CYCLE_100K, 24, GENERIC},
        {MLCPAGESIZE_4096X4, ADDR_LINES_1024, DEVSIZE_PARAMS(1024, 512), 1, EDC_NONE, MAX_CYCLE_100K, 24, GENERIC},
        {MLCPAGESIZE_4096X8, ADDR_LINES_1024, DEVSIZE_PARAMS(1024, 512), 1, EDC_NONE, MAX_CYCLE_100K, 24, GENERIC},
    };

    /*  List of chip test configurations.
    */
    static const FFXNANDCHIP DatalightTestChips[] =
    {
        /*  Page size variations
        */
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x00), &aChipClassPageSize[0],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x01), &aChipClassPageSize[1],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x02), &aChipClassPageSize[2],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x03), &aChipClassPageSize[3],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x04), &aChipClassPageSize[4],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x05), &aChipClassPageSize[5],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x06), &aChipClassPageSize[6],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x07), &aChipClassPageSize[7],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x08), &aChipClassPageSize[8],  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),
        DECLARE_NAND_CHIP(2, ID_2(0x01, 0x40), &ChipClass512M128K2KOdd, (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-PageSizeN"),

        /*  Various spare area formats
        */
        DECLARE_NAND_CHIP(2, ID_2(0x02, 0x00), &ChipClass512M128K2K,    (CHIPFBB_SSFDC      | CHIPINT_8BIT),  "DL512M128K-2K-SSFDC"),
        DECLARE_NAND_CHIP(2, ID_2(0x02, 0x01), &ChipClass512M128K2K,    (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-2K-OFFZ"),
        DECLARE_NAND_CHIP(2, ID_2(0x02, 0x02), &ChipClass512M128K512B,  (CHIPFBB_SSFDC      | CHIPINT_8BIT),  "DL512M128K-512B-SSFDC"),
        DECLARE_NAND_CHIP(2, ID_2(0x02, 0x03), &ChipClass512M128K512B,  (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-512B-OFFZ"),
        DECLARE_NAND_CHIP(2, ID_2(0x02, 0x04), &ChipClass512M128K512B,  (CHIPFBB_NONE       | CHIPINT_8BIT),  "DL512M128K-512B-OFFZ"),

        /*  MLC variations
        */
        DECLARE_NAND_CHIP(2, ID_2(0x03, 0x00), &ChipClass512M128K4K4bitEdc,    (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K2K4bitEdcMlc"),
        DECLARE_NAND_CHIP(2, ID_2(0x03, 0x01), &ChipClass512M128K4K4bitEdcMlc, (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K4K4bitEdcMlc"),
        DECLARE_NAND_CHIP(2, ID_2(0x03, 0x02), &ChipClass128M128K4K4bitEdcMlc, (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL128M128K4K4bitEdcMlc"),
        DECLARE_NAND_CHIP(2, ID_2(0x03, 0x03), &ChipClass512M128K2K2bitEdc,    (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K-2bitEdc"),

        /*  Miscellaneous variations

            0) A very small 1MB chip which is useful for walking through 
               multi-chip problem test cases.
            1) A very small 2MB chip which is useful for walking through 
               multi-chip problem test cases.
        */
        DECLARE_NAND_CHIP(2, ID_2(0x04, 0x00), &ChipClass1M64K1024B,           (CHIPFBB_OFFSETZERO | CHIPINT_8BIT),  "DL1M64K-1K-SuperSmall"),
        DECLARE_NAND_CHIP(2, ID_2(0x04, 0x01), &ChipClass2M128K1024B,          (CHIPFBB_OFFSETZERO | CHIPINT_8BIT),  "DL2M128K-1K-SuperSmall"),

        /*  Expected failure cases

            0) Any bit zero is no longer supported
            1) 12 bit EDC are not supported as of build 1845
            2) SSFDC format is not supported with >1 bit correction
        */
        DECLARE_NAND_CHIP(2, ID_2(0xF0, 0x00), &ChipClass512M128K2K,           (CHIPFBB_ANYBITZERO | CHIPINT_16BIT), "DL512M128K2K-AnyBitZero"),
        DECLARE_NAND_CHIP(2, ID_2(0xF0, 0x01), &ChipClassRequires12bitEdc,     (CHIPFBB_OFFSETZERO | CHIPINT_16BIT), "DL512M128K2K-Req12bitEdc"),
        DECLARE_NAND_CHIP(2, ID_2(0xF0, 0x02), &ChipClass512M128K4K4bitEdc,    (CHIPFBB_SSFDC | CHIPINT_16BIT),      "DL512M128K-SSFDC-4bitEdc"),
    };

    for(ii = 0; ii < DCLDIMENSIONOF(DatalightTestChips); ii++)
    {
        fMatch = TRUE;
        for(jj = 0; jj < DatalightTestChips[ii].uDevIdLength; jj++)
        {
            if(DatalightTestChips[ii].aucDevID[jj] != aucID[jj + 1])
                fMatch = FALSE;
        }
           
        if(fMatch)
        {
            pChip = &DatalightTestChips[ii];
            break;
        }
     }

    return pChip;
}

/*---------------------------------------------------------------------*/
/*---------------------------  Datalight Table   ----------------------*/
/*-----------------------------       End      ------------------------*/
/*---------------------------------------------------------------------*/
#endif


