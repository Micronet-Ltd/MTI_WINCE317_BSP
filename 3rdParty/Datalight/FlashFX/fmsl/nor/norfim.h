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

    This header file contain macros for building flash commands NOR Flash
    parts.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: norfim.h $
    Revision 1.24  2011/10/04 13:54:56Z  johnb
    Updated comment.
    Revision 1.23  2010/05/19 20:29:09Z  garyp
    Fixed the MAKE_OFFSET macro to use FLASHCHIP_MASK rather than ~0,
    which causes problems in some environments.
    Revision 1.22  2010/04/29 00:04:24Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.21  2010/01/30 21:06:38Z  glenns
    Fix Bug 2880: major update to account for CFI rules regarding
    calculation of CFI, command and address latch offsets; fix Bug
    2099: provide a way for GetChipData to tell whether AMD- or 
    Intel-style commands are working; Minor fix to be sure chip
    width rather than bus width is used to gate declarations for
    16-bit wide ID codes; include "flashcmd.h", whose declarations
    are used in this file.
    Revision 1.20  2009/12/03 20:55:56Z  garyp
    Moved manufacturer ID codes into nor.h.  Renamed ID_STMICRO to
    ID_NUMONYX.
    Revision 1.19  2009/09/10 17:58:26Z  glenns
    Fixed Bug 2829: fix up commentary for better explanation of the issue.
    Revision 1.18  2009/04/03 05:40:40Z  keithg
    Fixed Bug 2521: removed obsolete READ_BACK_VERIFY.
    Revision 1.17  2008/02/05 20:20:01Z  Glenns
    Added macros and changed declaration of "DidWriteComplete" to support 
    changes to the NORFIM that more accurately represent the standard 
    flowchart for how AMD-style devices detect write completion.
    Revision 1.16  2008/01/30 21:48:40Z  Glenns
    Added support for AMD Advanced Sector Protection block locking
    mechanism and Spansion NS-P parts
    Revision 1.15  2007/11/07 17:26:46Z  pauli
    Added descriptive #error message.
    Revision 1.14  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.13  2007/09/19 22:55:01Z  pauli
    Resolved Bug 355: Eliminated PNORFIMDATA.  Removed inappropriate casts of
    pointers to PFLASHDATA.  Resolved Bug 1488.
    Revision 1.12  2007/07/30 22:56:50Z  pauli
    Reverted to revision 1.10.  1.11 should have been a branched revision,
    but it was not.
    Revision 1.11  2007/07/27 20:12:08Z  rickc
    Updated erase suspend flag to comply with this code base.
    Revision 1.10  2007/06/13 00:06:12Z  rickc
    Added 2nd and 3rd device IDs to the lookup table, factored out
    duplicate part listings
    Revision 1.9  2007/06/11 23:14:05Z  pauli
    Changed the definition of a bit field from a long type to unsigned.
    Revision 1.8  2007/02/06 19:49:12Z  rickc
    Added Samsung part, removed local constant INCLUDE_ERASE_SUSPEND.
    Revision 1.7  2007/02/01 03:31:13Z  Garyp
    Updated to allow erase-suspend support to be compiled out of the product.
    Revision 1.6  2007/01/30 19:19:31Z  rickc
    Added support for x4 interleave
    Revision 1.5  2007/01/24 20:09:11Z  rickc
    Added Samsung parts
    Revision 1.4  2007/01/24 19:00:14Z  rickc
    Added Samsung parts to table
    Revision 1.3  2007/01/09 20:11:45Z  rickc
    Fixed compile error
    Revision 1.2  2007/01/05 22:04:52Z  rickc
    Added JEDEC ID ability for parts not CFI-compliant, general
    rename from "CFI" to more-fitting "NOR" names
    Revision 1.1  2007/01/02 22:45:24Z  rickc
    Initial revision
    Revision 1.7  2006/12/28 23:25:29Z  rickc
    Removed primary table ver 0.0
    Revision 1.6  2006/12/28 00:27:23Z  rickc
    EraseResume returns error code if fails
    Revision 1.5  2006/12/13 17:24:43Z  rickc
    Fixed Hungarian notation
    Revision 1.4  2006/12/12 22:42:37Z  rickc
    Clean-up compile warnings for MSVC6 tools
    Revision 1.3  2006/12/07 20:22:56Z  rickc
    Added support for Intel M18 flash
    Revision 1.2  2006/11/20 21:14:19Z  rickc
    Removed unneeded comment
    Revision 1.1  2006/11/10 23:01:52Z  rickc
    Initial revision
---------------------------------------------------------------------------*/
#include "flashcmd.h"

#ifndef NORFIM_H_INCLUDED
#define NORFIM_H_INCLUDED


/*  The following two macros control how write completions are detected in
    a NOR device. Typically there are two methods- watching the toggle bits
    toggle or observing the data lines to see when they indicate valid data.
    The first macro chooses toggle polling, and the second chooses data
    polling. It is possible to use both, but you must at least use one or
    the other.
*/

#define USE_TOGGLE_POLLING TRUE
#define USE_DATA_POLLING   TRUE
#if !((USE_TOGGLE_POLLING) | (USE_DATA_POLLING))
#error "NORFIM- Must define write completion polling technique."
#endif


/*  Uncomment the following constant definition if MLC NOR is never to be used.
*/
/*
#define NORFIM_MLC_FLASH DEV_NOT_MLC
*/
#ifndef NORFIM_MLC_FLASH
#define NORFIM_MLC_FLASH 0
#endif


/*  The minimum amount of time (in microseconds) to let a suspended
    erase progress to ensure that some forward progress is made.
*/
#define MINIMUM_ERASE_INCREMENT (1000)

/*  Erase suspend timeout in milliseconds
*/
#define ERASE_SUSPEND_TIMEOUT   (1)

/*  Delay in microseconds before polling in buffer write
*/
#define WRITE_BUFFER_DELAY 10

/*  Default timeouts in milliseconds
*/
#define NORFIM_TIMEOUT_WRITE        50
#define NORFIM_TIMEOUT_WRITEBUFFFER 50
#define NORFIM_TIMEOUT_ERASE        20000

/*  ulEraseResult value; used internally to clarify intent.
*/
#define ERASE_IN_PROGRESS       (0)


#define FIM_MAX_ERASE_BLOCK_REGIONS     4


typedef struct _FimEraseBlkReg
{
    D_UINT32    ulNumEraseBlocks;
    D_UINT32    ulBlockSizeKB;
} FimEraseBlkReg;


typedef struct _FimChipCommands
{
    FLASHDATA   programWord;
    FLASHDATA   bufferWriteStart;
    FLASHDATA   bufferWriteCommit;
    FLASHDATA   eraseSuspend;
    FLASHDATA   statusMask;
} FimChipCommands;

/*  Used for ID codes
*/
typedef struct
{
    FLASHDATA   manufacture;
    FLASHDATA   device1;
    FLASHDATA   device2;
    FLASHDATA   device3;
} NORFLASHID;

/*  Used to identify flash block locking standard
*/

typedef enum _FimBlockLockSupport
{
	NoProtection = 0,
	IntelInstantBlockLock,
	AmdAdvancedSectorProtection,
	Unsupported
} FimLockSupport, *pFimLockSupport;

/*---------------------------------------------------------
    The FIM instance data structure.
---------------------------------------------------------*/
typedef struct tagNORFIMDATA
{
    FIMINFO         FimInfo;
    FFXDEVHANDLE    hDev;
    DCLTIMER        tErase;
    FimLockSupport	eFimBlockLockSupport;
    D_UINT32        ulEraseStart;
    D_UINT32        ulEraseResult;
    D_UINT32        ulTimeoutRemaining;
    D_BOOL          fEraseSuspendSupported;
    D_BOOL          fEraseInProgress;
    D_BOOL          fEraseSuspended;
    unsigned        fInBootBlocks       : 1;
    unsigned        fAmdFlash           : 1;
    unsigned        fIntelFlash         : 1;
    unsigned        fIntelControlMode   : 1;
    unsigned        fAmdUnlockBypass    : 1;
    unsigned        fCfiCompliant       : 1;
    unsigned        fJedecID            : 1;

    NORFLASHID         ID;

    /*  CFI info
    */
    D_UINT32        ulPrimaryCmdSet;
    D_UINT32        ulPrimaryTableAddress;
    D_UINT32        ulAlternateCmdSet;
    D_UINT32        ulAlternateTableAddress;
    D_UINT32        ulPrimaryTableMajorVersion;
    D_UINT32        ulPrimaryTableMinorVersion;
    D_UINT32        ulAlternateTableMajorVersion;
    D_UINT32        ulAlternateTableMinorVersion;

    FimChipCommands ChipCommand;

    /*  Chip timing
    */
    D_UINT32        ulTimeoutWriteWordMillisec;
    D_UINT32        ulTimeoutWriteBufferMillisec;
    D_UINT32        ulTimeoutEraseBlockMillisec;

    /*  Chip geometry
    */
    D_UINT32        ulNumEraseBlockRegions;
    D_UINT32        ulChipSize;             /* In case of interleave, this is (chip size * interleave) */
    FimEraseBlkReg  FimEraseBlks[FIM_MAX_ERASE_BLOCK_REGIONS];
    D_UINT16        uPageSize;
    D_UINT16        uDeviceType;
    D_UINT16        uDeviceFlags;
    D_UINT32        ulIntelControlValidSize;
    D_UINT32        ulIntelControlInvalidSize;

    D_UINT32        ulBufferWriteSize;
    PFLASHDATA      pMedia;

}NORFIMDATA;

struct tagFIMDATA
{
    NORFIMDATA      *pCFIFimData;
};

/*---------------------------------------------------------
    Function Prototypes
---------------------------------------------------------*/
static D_BOOL   Read(NORFIMDATA *pFim, D_UINT32 ulStart, D_UINT32 ulLength, D_BUFFER *pBuffer);
static D_BOOL   Write(NORFIMDATA *pFim, D_UINT32 ulStart, D_UINT32 ulLength, const D_BUFFER *pBuffer);
static D_BOOL   EraseStart(NORFIMDATA *pFim, D_UINT32 ulStart);
static D_UINT32 ErasePoll(NORFIMDATA *pFim);

#if FFXCONF_ERASESUSPENDSUPPORT
static D_BOOL   EraseSuspend(NORFIMDATA *pFim);
static D_BOOL   EraseResume(NORFIMDATA *pFim);
#endif /* FFXCONF_ERASESUSPENDSUPPORT */

static D_BOOL   WriteBufferedBytes(NORFIMDATA *pFim, D_UINT32 ulStart, const D_BUFFER *pData, D_UINT32 ulLength);
static D_BOOL   WriteBytes(NORFIMDATA *pFim, D_UINT32 ulStart, const D_BUFFER *pData, D_UINT32 ulLength);
static D_BOOL   DidWriteComplete(NORFIMDATA *pFim, PFLASHDATA pMedia, FLASHDATA lastData, DCLTIMER *ptimer);
static D_BOOL   HandleProgramFailure(NORFIMDATA *pFim, D_UINT32 ulStart, PFLASHDATA pProgramMedia, FLASHDATA expectedData);
static D_BOOL	FimUnlockBlocks(D_UINT32 ulLength, FFXFIMBOUNDS *pBounds, NORFIMDATA *pFim);
static D_BOOL   IntelUnlockBlocks(D_UINT32 ulLength, FFXFIMBOUNDS *pBounds, NORFIMDATA *pFim);
static D_BOOL   AmdUnlockBlocks(D_UINT32 ulLength, FFXFIMBOUNDS *pBounds, NORFIMDATA *pFim);

static D_BOOL   IdChip(NORFIMDATA *pFim, PFLASHDATA pMedia, D_UINT32 ulQueryAndGetData, D_UINT32 ulUseCFI, D_UINT32 ulUseJedec);
static D_BOOL   ReadDeviceCodes(PFLASHDATA pMedia, NORFLASHID *lpChipID);
static D_BOOL   DoIDsMatch(NORFLASHID id, NORFLASHID idTable);
static D_BOOL   GetChipData(NORFIMDATA *pFim, PFLASHDATA pMedia, D_BOOL fUseIntelReadCmd);
static void     ResetFlash(NORFIMDATA *pFim, D_UINT32 ulAddress);
static void     DisplayFimInfo(NORFIMDATA *pFim);
static D_BOOL   ChipInfoCompare(NORFIMDATA *pFim1, NORFIMDATA *pFim2);

static D_BOOL   ReadControl(NORFIMDATA *pFim, D_UINT32 ulStart, D_UINT32 ulLength, D_BUFFER *pBuffer);
static D_BOOL   WriteControl(NORFIMDATA *pFim, D_UINT32 ulStart, D_UINT32 ulLength, const D_BUFFER *pBuffer);


/*---------------------------------------------------------
    CFI Command Codes and Status
---------------------------------------------------------*/

/*  The following macro and the way it is used by other NORFIM macros is purely
    an artifice to allow developers to work with test platforms that have
    adapters that do funny shifting with the address lines when you configure
    them to use multi-byte bus widths. Nominally the macro is set to zero.
*/
#define MAGIC_ADDRESS_SHIFT         (0)

/*  The MAKE_OFFSET macro provides a quick way to adjust the "magic offsets"
    that NOR FLASH devices use to issue commands, as well as the "magic offset"
    used to gain access to the CFI table. These offsets are dependent on a
    number of factors including how the device is connected to the address and
    data busses, and the width of the data bus.
*/
#define MAKE_INTERIM_OFFSET(x)      (((x) << (MAGIC_ADDRESS_SHIFT)) >> ((FLASH_CHIP_WIDTH/8) - 1))
#define MAKE_OFFSET(x)              MAKE_INTERIM_OFFSET((x) & ((FLASHCHIP_MASK) << ((FLASH_CHIP_WIDTH/8) - 1)))

/*  CFI stuff.
*/

#define CFI_CMD_QUERY   0x98


#define CFI_COMMAND_QUERY   MAKEFLASHDUPCMD(CFI_CMD_QUERY)

#define CMDSET_NULL                  0x0000
#define CMDSET_INTEL_EXTENDED        0x0001
#define CMDSET_AMD_STANDARD          0x0002
#define CMDSET_INTEL_STANDARD        0x0003
#define CMDSET_AMD_EXTENDED          0x0004
#define CMDSET_MITSUBISHI_STANDARD   0x0101
#define CMDSET_MITSUBISHI_EXTENDED   0x0102
#define CMDSET_INTEL_EXTENDED_M18    0x0200


#define CFI_DEVICE_INTERFACE_X8             0   /* x8 only */
#define CFI_DEVICE_INTERFACE_16             1   /* x16 only */
#define CFI_DEVICE_INTERFACE_x8_AND_x16     2   /* x8 and x16 */
#define CFI_DEVICE_INTERFACE_x32            3   /* x32 only */
#define CFI_DEVICE_INTERFACE_x16_AND_x32    5   /* x16 and x32 */

/* CFI Query ID
*/
#define CFI_OFFSET_QUERY_ADDRESS     MAKE_OFFSET(0xAA)

/*  CFI Device-mode compensation

    The JEDEC CFI defines how CFI tables are accessed for various types
    of devices. There are basically two classes of devices; those whose
    native bus width is fixed to a specific value and those that can be
    configured to operate as x8 devices even though they support an x16
    or x32 bus width. This is important because configurable devices that 
    are operating in x8 mode must access CFI differently than native x8
    devices.

    The following three macros are designed to offer support for these
    options.

    CFI_DEVICE_TYPE:

        This macro basically represents the largest data bus width the
        device can support. For configurable devices, this does not
        necessarily represent how the device is connected to bus, but
        basically how many data bus pins are on the device package.
        The default value of this macro is 0x10, which indicates a
        device with 16 data bus pins, which is what most NOR devices
        feature. However, not all, and if you are using a NOR device
        with a different number of data bus connections, this value
        must be adjusted accordingly.

    CFI_FIELD_INTERVAL:

        This macro basically identifies the minimum distance between
        fields in the CFI table, in units of FLASHDATA. This value 
        will be 1 when CFI_DEVICE_TYPE and FLASH_CHIP_WIDTH are the
        same, but for configurable devices set up such that the
        device type and chip width are different, this value will
        *not* be 1, and this *must* be accounted for when accessing
        the CFI tables.
        
    CFI_DEV_MODE_SHIFT:

        This macro is a value computed from the CFI_DEVICE_TYPE and the
        FLASH_CHIP_WIDTH macro (itself derived from FFXCONF_NORCONFIG and
        the interleaving structure represented by the FLASH_INTERLEAVE 
        macro) that adjusts the value of the offsets into the CFI table
        used to access CFI information.
    
    For non-configurable devices, CFI_DEVICE_TYPE and FFXCONF_NORCONFIG
    must represent equal values; for example, CFI_DEVICE_TYPE = 0x10 would
    stand for a device that has 16 data bus pins and for a non-configurable
    device, all 16 would have to be connected and therefor FFXCONF_NORCONFIG
    would have to be set to NORFLASH_x16. 

    For configurable devices, these macros would represent equal values only
    if the device is connected to the bus using its maximum supportable
    bus width. If, on the other hand, a configurable device that could be
    configured to support a data bus 16 bits wide but was attached in a design
    that used only 8 bits, CFI_DEVICE_TYPE would be 0x10 but FFXCONF_NORCONFIG
    would be set to NORFLASH_x8.

    Refer to the JEDEC specification for more details.
*/

#define CFI_DEVICE_TYPE             0x10
#define CFI_FIELD_INTERVAL          (CFI_DEVICE_TYPE / FLASH_CHIP_WIDTH)
#define CFI_DEV_MODE_SHIFT          (CFI_FIELD_INTERVAL - 1)

#define CFI_OFFSET_QUERY_ASCII_STRING_QRY           (0x10 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_PRIMARY_COMMAND_SET              (0x13 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_PRIMARY_EXTENDED_TABLE_ADDRESS   (0x15 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_ALTERNATE_COMMAND_SET            (0x17 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_ALTERNATE_EXTENDED_TABLE_ADDRESS (0x19 << CFI_DEV_MODE_SHIFT)

/* CFI Timings
*/
#define CFI_OFFSET_TYPICAL_TIMEOUT_WRITE_WORD   (0x1f << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_TYPICAL_TIMEOUT_WRITE_BUFFER (0x20 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_TYPICAL_TIMEOUT_ERASE_BLOCK  (0x21 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_TYPICAL_TIMEOUT_ERASE_CHIP   (0x22 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_MAX_TIMEOUT_WRITE_WORD       (0x23 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_MAX_TIMEOUT_WRITE_BUFFER     (0x24 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_MAX_TIMEOUT_ERASE_BLOCK      (0x25 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_MAX_TIMEOUT_ERASE_CHIP       (0x26 << CFI_DEV_MODE_SHIFT)


/* CFI Device geometry
*/
#define CFI_OFFSET_DEVICE_SIZE                      (0x27 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_DEVICE_INTERFACE_DESCRIPTION     (0x28 << CFI_DEV_MODE_SHIFT)

#define CFI_OFFSET_BUFFER_WRITE_SIZE                (0x2a << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_ERASE_BLOCK_REGIONS              (0x2c << CFI_DEV_MODE_SHIFT)

#define CFI_OFFSET_ERASE_BLOCK_REGION_INFO_BASE     (0x2d << CFI_DEV_MODE_SHIFT)
#define CFI_SIZE_ERASE_BLOCK_REGION_IN_BUSWIDTH     (4*CFI_FIELD_INTERVAL)


/*  Primary Extended Query Table offsets from beginning of  table.  
    Base address of table specified in 0x15.
*/
#define CFI_OFFSET_QUERY_ASCII_STRING_PRI           (0x00 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_MAJOR_VERSION                    (0x03 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_MINOR_VERSION                    (0x04 << CFI_DEV_MODE_SHIFT)


#if 0
/* Primary AMD-Specific Extended Query Table offsets from beginning of table
    Major version: 0x31
    Minor version: 0x33
*/
#define CFI_OFFSET_AMD_PRI_ADD_SENSITIVE_UNLOCK             0x05
#define CFI_OFFSET_AMD_PRI_ERASE_SUSPEND                    0x06
#define CFI_OFFSET_AMD_PRI_SECTOR_PROTECT                   0x07
#define CFI_OFFSET_AMD_PRI_SECTOR_TEMPORARY_UNPROTECT       0x07
#define CFI_OFFSET_AMD_PRI_SECTOR_PROTECT_UNPROTECT_SCHEME  0x09
#define CFI_OFFSET_AMD_PRI_SIMULTANEOUS_OPERATION           0x0a
#define CFI_OFFSET_AMD_PRI_BURST_MODE_TYPE                  0x0b
#define CFI_OFFSET_AMD_PRI_PAGE_MODE_TYPE                   0x0c
#define CFI_OFFSET_AMD_PRI_ACC_MIN_SUPPLY                   0x0d
#define CFI_OFFSET_AMD_PRI_ACC_MAX_SUPPLY                   0x0e
#define CFI_OFFSET_AMD_PRI_3133_BOOT_BLOCK_FLAG             0x0f
#define CFI_OFFSET_AMD_PRI_PROGRAM_SUSPEND                  0x10
#endif

/*  Primary AMD-Specific Extended Query Table offsets from beginning of table.
    Numbers in constant specify major and minor version numbers of the
    primary query table.  Details of various versions specified by
    http://www.amd.com/us-en/assets/content_type/DownloadableAssets/cfi_r20.pdf
*/
#define CFI_OFFSET_AMD_PRI_ERASE_SUSPEND                    (0x06 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_M18_INTEL_PRI_PROGRAMMING_REGION_SIZE    (0x33 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_M18_INTEL_PRI_CONTROL_MODE_VALID_SIZE    (0x35 << CFI_DEV_MODE_SHIFT)
#define CFI_OFFSET_M18_INTEL_PRI_CONTROL_MODE_INVALID_SIZE  (0x37 << CFI_DEV_MODE_SHIFT)

/*
Version 1.1
0x02 = Bottom
0x03 = Top
0x04 = Uniform/Bottom WP
0x05 = Uniform/Top WP
If number of erase regions > 1 and boot block flag = 0x00, refer to device ID code

Version 1.2
0x00 = No WP
0x01 = Top/Bottom WP
0x02 = Bottom
0x03 = Top
0x04 = Uniform/Bottom WP
0x05 = Uniform/Top WP

Version 1.3
0x00 = No WP
0x01 = Top/Bottom WP
0x02 = Bottom
0x03 = Top
0x04 = Uniform/Bottom WP
0x05 = Uniform/Top WP

Version 1.4
s29NSxxxN
0x01 = Top/Middle Boot Device
0x02 = Bottom Boot Device
0x03 = Top Boot Device
*/
#define CFI_OFFSET_AMD_PRI_BOOT_BLOCK_FLAG      (0x0F << CFI_DEV_MODE_SHIFT)

/*  AMD CFI Boot block values
*/
#define CFI_BOOT_BLOCK_UNIFORM                      0x00
#define CFI_BOOT_BLOCK_TOP_AND_BOTTOM_WP            0x01
#define CFI_BOOT_BLOCK_BOTTOM                       0x02
#define CFI_BOOT_BLOCK_TOP                          0x03
#define CFI_BOOT_BLOCK_TOP_AND_BOTTOM               0x04

/*  AMD CFI Boot block values if only one erase region (so actually no boot blocks)
*/
#define CFI_BOOT_BLOCK_UNIFORM_BOTTOM_WP            0x04
#define CFI_BOOT_BLOCK_UNIFORM_TOP_WP               0x05

/*  Primary Intel-Specific Extended Query Table offsets from beginning of table.
    Based on the following documents, Erase suspend support is bit 1 and individual
    block locking support is bit 5

    v1.1 - http://www.intel.com/design/flcomp/datashts/29073709.pdf
    v1.3 - http://download.intel.com/design/flcomp/datashts/25190210.pdf
    v1.4 - http://download.intel.com/design/flcomp/datashts/31474902.pdf
*/
#define CFI_OFFSET_INTEL_PRI_ERASE_SUSPEND                  (0x05 << CFI_DEV_MODE_SHIFT)
#define CFI_INTEL_PRI_ERASE_SUSPEND_MASK                    0x02

#define CFI_OFFSET_INTEL_PRI_INSTANT_BLOCK_LOCKING          (0x05 << CFI_DEV_MODE_SHIFT)
#define CFI_INTEL_PRI_INSTANT_BLOCK_LOCKING_MASK            0x20

/*  Primary AMD-Specific Primary Extended Query Table offsets from the
    beginning of the table. These offsets can be used to find whether and
    what sector-protection mechanism is being used.
*/

#define CFI_OFFSET_AMD_SECTOR_PROTECT_SCHEME		(0x09 << CFI_DEV_MODE_SHIFT)
#define CFI_AMD_ADVANCED_SECTOR_PROTECTION_VAL		0x08

/*  AMD Advanced Sector Protection bit masks:
*/

#define AMD_ASP_PASSWORD_PROTECTION_ENABLE_MASK		0x04
#define AMD_ASP_PERSISTENT_PROTECTION_ENABLE_MASK	0x02
#define AMD_ASP_PPB_ERASECOMMAND_ENABLE_MASK		0x08
#define AMD_ASP_GLOBAL_PPB_PROTECTION_MASK			0x01
#define AMD_ASP_PPB_BIT_MASK						0x01
#define AMD_ASP_DYB_BIT_MASK						0x01

#if FLASH_INTERLEAVE == 1
    #if FLASH_BUS_WIDTH == 8
        #define CHIP0   0xFF
        #define CHIP1   0xFF

    #elif FLASH_BUS_WIDTH == 16
        #define CHIP0   0xFFFF
        #define CHIP1   0xFFFF
    #else
        #error "Invalid FLASH_BUS_WIDTH"
    #endif
#elif FLASH_INTERLEAVE == 2
    #if FLASH_BUS_WIDTH == 16
        #define CHIP0   0x00FF
        #define CHIP1   0xFF00
    #elif FLASH_BUS_WIDTH == 32
        #define CHIP0   0x0000FFFF
        #define CHIP1   0xFFFF0000
    #else
        #error "Invalid FLASH_BUS_WIDTH"
    #endif
#elif FLASH_INTERLEAVE == 4
    #if FLASH_BUS_WIDTH == 32
        #define CHIP0   0x000000FF
        #define CHIP1   0x0000FF00
        #define CHIP2   0x00FF0000
        #define CHIP3   0xFF000000
    #else
        #error "Invalid FLASH_BUS_WIDTH"
    #endif
#else
#error "Invalid FLASH_INTERLEAVE"
#endif

/*  Chip ID offsets
*/
#define NORFIM_DEVICE_CODE1_OFFSET  0x1
#define NORFIM_DEVICE_CODE2_OFFSET  0xe
#define NORFIM_DEVICE_CODE3_OFFSET  0xf

/*  AMD Single-Chip Command Codes and Status
*/
#define ACMD_ID                 (0x90)
#define ACMD_PROG               (0xA0)
#define ACMD_READMODE           (0xF0)
#define ACMD_ERASE1             (0x80)
#define ACMD_ERASE2             (0x30)
#define ACMD_ERASESUSPEND       (0xB0)
#define ACMD_UNLOCK1            (0xAA)
#define ACMD_UNLOCK2            (0x55)
#define ACMD_UNLOCK_BYPASS      (0x20)
#define ACMD_BYPASS_RESET       (0x00)
#define ACMD_WRITETOBUFFER      (0x25)
#define ACMD_BUFFERTOFLASH      (0x29)

#define ACMD_LKREG_CMDSET_ENTRY (0x40)
#define ACMD_LKREG_BREAD		(0x00)
#define ACMD_NV_SECTPROT_CMDSET_ENTRY		(0xC0)
#define ACMD_GBL_PROTFREEZE_CMDSET_ENTRY	(0x50)
#define ACMD_GBL_PPB_ERASE1		(0x80)
#define ACMD_GBL_PPB_ERASE2		(0x30)
#define ACMD_VOL_SECTPROT_CMDSET_ENTRY		(0xE0)
#define ACMD_VOL_SECTPROT_CMD_ADJUST		(0xA0)
#define ACMD_VOL_SECTPROT_SET 	(0x00)
#define ACMD_VOL_SECTPROT_CLEAR (0x01)
#define ACMD_PROT_CMDSET_EXIT1	(0x90)
#define ACMD_PROT_CMDSET_EXIT2	(0x00)

#define ASTAT_OK                (0x80)  /* DQ7 Data# polling                */
#define ASTAT_TOGGLEDONE1       (0x40)  /* DQ6 Toggle bit 1                 */
#define ASTAT_FAIL              (0x20)  /* DQ5 exceeded timing limits       */
#define ASTAT_ERASEBEGUN        (0x08)  /* DQ3 sector erase timer           */
#define ASTAT_TOGGLEDONE2       (0x04)  /* DQ2 Toggle bit 2                 */
#define ASTAT_BUFFERABORT       (0x02)  /* DQ1 Write-to-Buffer Abort (high) */
#define ASTAT_MASK              (0x88)

/*  AMD Multi-Chip Command Codes and Status
*/
#define AMDCMD_IDENTIFY                 MAKEFLASHDUPCMD(ACMD_ID)
#define AMDCMD_PROGRAM                  MAKEFLASHDUPCMD(ACMD_PROG)
#define AMDCMD_READ_MODE                MAKEFLASHDUPCMD(ACMD_READMODE)
#define AMDCMD_ERASE1                   MAKEFLASHDUPCMD(ACMD_ERASE1)
#define AMDCMD_ERASE2                   MAKEFLASHDUPCMD(ACMD_ERASE2)
#define AMDCMD_ERASE_SUSPEND            MAKEFLASHDUPCMD(ACMD_ERASESUSPEND)
#define AMDCMD_UNLOCK1                  MAKEFLASHDUPCMD(ACMD_UNLOCK1)
#define AMDCMD_UNLOCK2                  MAKEFLASHDUPCMD(ACMD_UNLOCK2)
#define AMDCMD_UNLOCK_BYPASS            MAKEFLASHDUPCMD(ACMD_UNLOCK_BYPASS)
#define AMDCMD_BYPASS_RESET             MAKEFLASHDUPCMD(ACMD_BYPASS_RESET)
#define AMDCMD_WRITETOBUFFER            MAKEFLASHDUPCMD(ACMD_WRITETOBUFFER)
#define AMDCMD_BUFFERTOFLASH            MAKEFLASHDUPCMD(ACMD_BUFFERTOFLASH)

#define AMDSTAT_DONE                    MAKEFLASHDUPCMD(ASTAT_OK)
#define AMDSTAT_TOGGLEDONE              MAKEFLASHDUPCMD(ASTAT_TOGGLEDONE1)
#define AMDSTAT_ERASEBEGUN              MAKEFLASHDUPCMD(ASTAT_ERASEBEGUN)
#define AMDSTAT_MASK                    MAKEFLASHDUPCMD(ASTAT_MASK)
#define AMDSTAT_BUFFERABORT             MAKEFLASHDUPCMD(ASTAT_BUFFERABORT)

#define AMDCMD_LKREG_CMDSET_ENTRY		MAKEFLASHDUPCMD(ACMD_LKREG_CMDSET_ENTRY)
#define AMDCMD_LKREG_BREAD				MAKEFLASHDUPCMD(ACMD_LKREG_BREAD)
#define AMDCMD_NV_SECTPROT_CMDSET_ENTRY MAKEFLASHDUPCMD(ACMD_NV_SECTPROT_CMDSET_ENTRY)
#define AMDCMD_GBL_PROTFREEZE_CMDSET_ENTRY MAKEFLASHDUPCMD(ACMD_GBL_PROTFREEZE_CMDSET_ENTRY)
#define AMDCMD_GBL_PPB_ERASE1			MAKEFLASHDUPCMD(ACMD_GBL_PPB_ERASE1)
#define AMDCMD_GBL_PPB_ERASE2			MAKEFLASHDUPCMD(ACMD_GBL_PPB_ERASE2)
#define AMDCMD_VOL_SECTPROT_CMDSET_ENTRY MAKEFLASHDUPCMD(ACMD_VOL_SECTPROT_CMDSET_ENTRY)
#define AMDCMD_VOL_SECTPROT_CMD_ADJUST	MAKEFLASHDUPCMD(ACMD_VOL_SECTPROT_CMD_ADJUST)
#define AMDCMD_VOL_SECTPROT_SET			MAKEFLASHDUPCMD(ACMD_VOL_SECTPROT_SET)
#define AMDCMD_VOL_SECTPROT_CLEAR		MAKEFLASHDUPCMD(ACMD_VOL_SECTPROT_CLEAR)
#define AMDCMD_PROT_CMDSET_EXIT1		MAKEFLASHDUPCMD(ACMD_PROT_CMDSET_EXIT1)
#define AMDCMD_PROT_CMDSET_EXIT2		MAKEFLASHDUPCMD(ACMD_PROT_CMDSET_EXIT2)

#if FLASH_INTERLEAVE == 4
#define AMD_DONE_CHIP0                  MAKEFLASHCMD(0x0, 0x0, 0x0, ASTAT_OK)
#define AMD_DONE_CHIP1                  MAKEFLASHCMD(0x0, 0x0, ASTAT_OK, 0x0)
#define AMD_DONE_CHIP2                  MAKEFLASHCMD(0x0, ASTAT_OK, 0x0, 0x0)
#define AMD_DONE_CHIP3                  MAKEFLASHCMD(ASTAT_OK, 0x0, 0x0, 0x0)
#define AMD_FAIL_CHIP0                  MAKEFLASHCMD(0x0, 0x0, 0x0, ASTAT_FAIL)
#define AMD_FAIL_CHIP1                  MAKEFLASHCMD(0x0, 0x0, ASTAT_FAIL, 0x0)
#define AMD_FAIL_CHIP2                  MAKEFLASHCMD(0x0, ASTAT_FAIL, 0x0, 0x0)
#define AMD_FAIL_CHIP3                  MAKEFLASHCMD(ASTAT_FAIL, 0x0, 0x0, 0x0)
#define AMD_ERASE_BEGUN_CHIP0           MAKEFLASHCMD(0x0, 0x0, 0x0, ASTAT_ERASEBEGUN) /* DQ3 */
#define AMD_ERASE_BEGUN_CHIP1           MAKEFLASHCMD(0x0, 0x0, ASTAT_ERASEBEGUN, 0x0) /* DQ3 */
#define AMD_ERASE_BEGUN_CHIP2           MAKEFLASHCMD(0x0, ASTAT_ERASEBEGUN, 0x0, 0x0) /* DQ3 */
#define AMD_ERASE_BEGUN_CHIP3           MAKEFLASHCMD(ASTAT_ERASEBEGUN, 0x0, 0x0, 0x0) /* DQ3 */
#define AMD_BUFF_ABORT_CHIP0            MAKEFLASHCMD(0x0, 0x0, 0x0, ASTAT_BUFFERABORT) /* DQ1 */
#define AMD_BUFF_ABORT_CHIP1            MAKEFLASHCMD(0x0, 0x0, ASTAT_BUFFERABORT, 0x0) /* DQ1 */
#define AMD_BUFF_ABORT_CHIP2            MAKEFLASHCMD(0x0, ASTAT_BUFFERABORT, 0x0, 0x0) /* DQ1 */
#define AMD_BUFF_ABORT_CHIP3            MAKEFLASHCMD(ASTAT_BUFFERABORT, 0x0, 0x0, 0x0) /* DQ1 */
#define AMD_AMD_ERASE_CMD2_CHIP0        MAKEFLASHCMD(0x0, 0x0, 0x0, ACMD_ERASE2)
#define AMD_AMD_ERASE_CMD2_CHIP1        MAKEFLASHCMD(0x0, 0x0, ACMD_ERASE2, 0x0)
#define AMD_AMD_ERASE_CMD2_CHIP2        MAKEFLASHCMD(0x0, ACMD_ERASE2, 0x0, 0x0)
#define AMD_AMD_ERASE_CMD2_CHIP3        MAKEFLASHCMD(ACMD_ERASE2, 0x0, 0x0, 0x0)
#elif FLASH_INTERLEAVE == 2
#define AMD_DONE_CHIP0                  MAKEFLASHCMD(0x0, ASTAT_OK)             /* DQ7 */
#define AMD_DONE_CHIP1                  MAKEFLASHCMD(ASTAT_OK, 0x0)             /* DQ7 */
#define AMD_FAIL_CHIP0                  MAKEFLASHCMD(0x0, ASTAT_FAIL)           /* DQ5 */
#define AMD_FAIL_CHIP1                  MAKEFLASHCMD(ASTAT_FAIL, 0x0)           /* DQ5 */
#define AMD_ERASE_BEGUN_CHIP0           MAKEFLASHCMD(0x0, ASTAT_ERASEBEGUN)     /* DQ3 */
#define AMD_ERASE_BEGUN_CHIP1           MAKEFLASHCMD(ASTAT_ERASEBEGUN, 0x0)     /* DQ3 */
#define AMD_BUFF_ABORT_CHIP0            MAKEFLASHCMD(0x0, ASTAT_BUFFERABORT)    /* DQ1 */
#define AMD_BUFF_ABORT_CHIP1            MAKEFLASHCMD(ASTAT_BUFFERABORT, 0x0)    /* DQ1 */
#define AMD_ERASE_CMD2_CHIP0            MAKEFLASHCMD(0x0, ACMD_ERASE2)
#define AMD_ERASE_CMD2_CHIP1            MAKEFLASHCMD(ACMD_ERASE2, 0x0)
#elif FLASH_INTERLEAVE == 1
#define AMD_DONE_CHIP0                  MAKEFLASHCMD(ASTAT_OK)             /* DQ7 */
#define AMD_FAIL_CHIP0                  MAKEFLASHCMD(ASTAT_FAIL)           /* DQ5 */
#define AMD_ERASE_BEGUN_CHIP0           MAKEFLASHCMD(ASTAT_ERASEBEGUN)     /* DQ3 */
#define AMD_BUFF_ABORT_CHIP0            MAKEFLASHCMD(ASTAT_BUFFERABORT)    /* DQ1 */
#endif


/*  Latch address offsets. Note that using the MAKE_OFFSET macro is necessary
    because the NOR window mapping functions do all their calculations as if
    addresses are byte addresses and offsets are byte offsets, but the NOR FIM
    uses the pointers returned by the window mapping functions as pointers to
    the FLASHDATA type, which can be a 16-bit word. In this case, compiler
    pointer arithmetic would introduce an unwanted 1-bit left-shift of the
    address offsets used to access the command and address latches.
*/

#define AMD_LATCH_OFFSET1     MAKE_OFFSET(0xAAAU)
#define AMD_LATCH_OFFSET2     MAKE_OFFSET(0x555U)


/*  Intel Single-Chip Command Codes and Status
*/
#define ICMD_ID                         (0x90)

#define ICMD_PROG                       (0x40)
#define ICMD_PROG_M18                   (0x41)

#define ICMD_CLEARSTAT                  (0x50)
#define ICMD_READSTAT                   (0x70)

#define ICMD_WRITEBUFFER                (0xE8)
#define ICMD_WRITEBUFFER_M18            (0xE9)

#define ICMD_READMODE                   (0xFF)
#define ICMD_ERASESTART                 (0x20)
#define ICMD_ERASESUSPEND               (0xB0)
#define ICMD_ERASERESUME                (0xD0)
#define ICMD_CONFIRM                    (0xD0)
#define ICMD_LOCKUNLOCK                 (0x60)
#define ICMD_LOCKCONFIRM                (0x01)

#define ISTAT_WRITEFAIL                 (0x10)
#define ISTAT_ERASEFAIL                 (0x20)
#define ISTAT_OK                        (0x80)
#define ISTAT_ERASESUSPENDED            (0xC0)

/*  All valid status bits
*/
#define ISTAT_MASK                      (0xFE)
#define ISTAT_MASK_M18                  (0x02FE)

#define INTLSTAT_BLOCK_LOCKED           (0x01)
#define INTLSTAT_BLOCK_LOCKED_DOWN      (0x02)

/*  Error - attempted write with Object data to Control Mode region
    bit 9 set, bit 8 unset and bit 4 (write fail) set
*/
#define ISTAT_OBJ_TO_CTL                (0x0210)

/*  Error - attempted rewrite to object region
    bit 9 unset, bit 8 set and bit 4 (write fail) set
*/
#define ISTAT_REWRITE_OBJ               (0x0110)

/*  Error - write using illegal command
    bit 9 set, bit 8 set, and bit 4 (write fail) set
*/
#define ISTAT_ILLEGAL_CMD               (0x0310)

/*---------------------------------------------------------
    Intel Multi-Chip Command Codes and Status
---------------------------------------------------------*/
#define INTLCMD_IDENTIFY                MAKEFLASHDUPCMD(ICMD_ID)
#define INTLCMD_PROGRAM                 MAKEFLASHDUPCMD(ICMD_PROG)
#define INTLCMD_PROGRAM_M18             MAKEFLASHDUPCMD(ICMD_PROG_M18)
#define INTLCMD_CLEAR_STATUS            MAKEFLASHDUPCMD(ICMD_CLEARSTAT)
#define INTLCMD_READ_STATUS             MAKEFLASHDUPCMD(ICMD_READSTAT)
#define INTLCMD_BUFFERED_WRITE          MAKEFLASHDUPCMD(ICMD_WRITEBUFFER)
#define INTLCMD_BUFFERED_WRITE_M18      MAKEFLASHDUPCMD(ICMD_WRITEBUFFER_M18)
#define INTLCMD_READ_MODE               MAKEFLASHDUPCMD(ICMD_READMODE)
#define INTLCMD_ERASE_START             MAKEFLASHDUPCMD(ICMD_ERASESTART)
#define INTLCMD_ERASE_SUSPEND           MAKEFLASHDUPCMD(ICMD_ERASESUSPEND)
#define INTLCMD_ERASE_RESUME            MAKEFLASHDUPCMD(ICMD_ERASERESUME)
#define INTLCMD_CONFIRM                 MAKEFLASHDUPCMD(ICMD_CONFIRM)
#define INTLCMD_LOCKUNLOCK              MAKEFLASHDUPCMD(ICMD_LOCKUNLOCK)
#define INTLCMD_LOCKCONFIRM             MAKEFLASHDUPCMD(ICMD_LOCKCONFIRM)

#define INTLSTAT_WRITE_FAIL             MAKEFLASHDUPCMD(ISTAT_WRITEFAIL)
#define INTLSTAT_ERASE_FAIL             MAKEFLASHDUPCMD(ISTAT_ERASEFAIL)
#define INTLSTAT_DONE                   MAKEFLASHDUPCMD(ISTAT_OK)
#define INTLSTAT_ERASE_SUSPENDED        MAKEFLASHDUPCMD(ISTAT_ERASESUSPENDED)
#define INTLSTAT_STATUS_MASK            MAKEFLASHDUPCMD(ISTAT_MASK)
#define INTLSTAT_STATUS_MASK_M18        MAKEFLASHDUPCMD(ISTAT_MASK_M18)

#define BLOCK_LOCKED                    MAKEFLASHDUPCMD(INTLSTAT_BLOCK_LOCKED)
#define BLOCK_LOCKED_DOWN               MAKEFLASHDUPCMD(INTLSTAT_BLOCK_LOCKED_DOWN)

#define INTLSTAT_OBJ_TO_CTL             MAKEFLASHDUPCMD(ISTAT_OBJ_TO_CTL)
#define INTLSTAT_REWRITE_OBJ            MAKEFLASHDUPCMD(ISTAT_REWRITE_OBJ)
#define INTLSTAT_ILLEGAL_CMD            MAKEFLASHDUPCMD(ISTAT_ILLEGAL_CMD)


/*  Offset into flash to check block status after sending the identify command
*/
#define INTEL_BLOCK_LOCK_STATUS_INDEX   2


#define M18_PAGE_MASK       0xFFFFFC00 /* 22 bits for page address */
#define M18_OFFSET_MASK     0x000003FF /* 10 bits for offset (bits 0-9, bit 9 is always 0 for 1x16) */

/*  Bits in bytes to make table easier to maintain
*/
#define NORFIM_4Mb       0x00080000UL
#define NORFIM_8Mb       0x00100000UL
#define NORFIM_16Mb      0x00200000UL

#define NORFIM_32Mb      0x00400000UL
#define NORFIM_64Mb      0x00800000UL
#define NORFIM_128Mb     0x01000000UL
#define NORFIM_256Mb     0x02000000UL
#define NORFIM_512Mb     0x04000000UL
#define NORFIM_1024Mb    0x08000000UL

/*  Bytes to make table easier to maintain
*/
#define NORFIM_4KB    0x00001000UL
#define NORFIM_8KB    0x00002000UL
#define NORFIM_16KB   0x00004000UL
#define NORFIM_32KB   0x00008000UL
#define NORFIM_64KB   0x00010000UL
#define NORFIM_128KB  0x00020000UL
#define NORFIM_256KB  0x00040000UL

/*  Kwords in Bytes to make table easier to maintain
*/
#define NORFIM_4Kword    0x00002000UL
#define NORFIM_8Kword    0x00004000UL
#define NORFIM_16Kword   0x00008000UL
#define NORFIM_32Kword   0x00010000UL
#define NORFIM_64Kword   0x00020000UL
#define NORFIM_128Kword  0x00040000UL
#define NORFIM_256Kword  0x00080000UL


/*  Flag if erase suspend is NOT supported by the flash part.  Not supported
    is flagged because most modern parts will support this and therefore this
    flag may never be used.
*/
#define NORFIM_IDFLAGS_NOERASESUSPEND   0x01


/*  Flag if Intel or clone supports instant block lock
*/
#define NORFIM_IDFLAGS_INTELBLOCKLOCK   0x02

/*  Flag if unlock bypass is NOT supported by the AMD flash part.  Not supported
    is flagged because most modern parts will support this and therefore this
    flag may never be used.
*/
#define NORFIM_IDFLAGS_NOUNLOCKPYPASS   0x04

typedef struct NORCHIPPARAMS_TAG
{
    NORFLASHID  ID;
    D_UINT32    ulPrimaryCmdSet;
    D_UINT32    ulChipSize;
    D_UINT32    ulBlockSize;
    D_UINT32    ulBootBlockSize;
    D_UINT16    uLowBootBlocks;
    D_UINT16    uHighBootBlocks;
    D_UINT32    ulBufferWriteSize;

    D_UINT32    ulIDFlags;

    /*  Will only be used for Intel M18 cloned parts, if needed at all
    */
    D_UINT16    uPageSize;
    D_UINT32    ulIntelControlValidSize;
    D_UINT32    ulIntelControlInvalidSize;
} NORCHIPPARAMS;


/*---------------------------------------------------------
    Device IDs

    For devices in the below table that do not specify 
    three device IDs, use ID_NA for the IDs not used. 
    Devices that specify 3 device IDs must be listed
    before parts that use ID_NA if the common device
    IDs match.
---------------------------------------------------------*/

/*                                                    PrimaryCmdSet,     ChipSize,        BlkSize,    BootBlkSize, L, H,   BufferSize, flags*/
#define ID_28F016S5     {ID_INTEL89,0xAA,ID_NA,ID_NA}, CMDSET_INTEL_STANDARD,  NORFIM_16Mb,    NORFIM_64KB,              0, 0, 0, 0x00000000UL
#define ID_28F008SAL    {ID_INTEL89,0xA2,ID_NA,ID_NA}, CMDSET_INTEL_STANDARD,   NORFIM_8Mb,    NORFIM_64KB,              0, 0, 0, 0x00000000UL

#define ID_AM29F080         {ID_AMD,0xD5,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, 0x00100000UL,    NORFIM_64KB,              0, 0, 0, 0x00000000UL, NORFIM_IDFLAGS_NOUNLOCKPYPASS

/*  If the flash bus width is only 8 bits, leave out macro definitions for 
    devices we know to be 16 bits wide. this is important because these devices
    16-bit ID codes, and if we try to include them with a flash bus width of
    only 8 bits, compilers will complain about integer truncation.
*/

#if !(FLASH_CHIP_WIDTH == 8)

/*  Spansion WS-N
*/
#define ID_S29WS128N    {ID_AMD,0x227e,0x2231,0x2200},   CMDSET_AMD_STANDARD, NORFIM_128Mb, NORFIM_64Kword, NORFIM_16Kword, 4, 4, 0x00000040UL
#define ID_S29WS256N    {ID_AMD,0x227e,0x2230,0x2200},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_64Kword, NORFIM_16Kword, 4, 4, 0x00000040UL

/*  Samsung K8A
*/
#define ID_K8A3215EBA   {ID_SAMSUNG,0x2271,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_32Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8A3215ETA   {ID_SAMSUNG,0x2270,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_32Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8A6415EBA   {ID_SAMSUNG,0x2253,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_64Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8A6415ETA   {ID_SAMSUNG,0x2252,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_64Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8A2815EBA   {ID_SAMSUNG,0x2249,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_128Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8A2815ETA   {ID_SAMSUNG,0x2248,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_128Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8A5615EBA   {ID_SAMSUNG,0x22FD,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8A5615ETA   {ID_SAMSUNG,0x22FC,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL

/*  Samsung K8C
*/
#define ID_K8C5x15EBM   {ID_SAMSUNG,0x2207,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_64Kword, NORFIM_16Kword, 4, 0, 0x00000040UL
#define ID_K8C5x15ETM   {ID_SAMSUNG,0x2206,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_64Kword, NORFIM_16Kword, 0, 4, 0x00000040UL

#define ID_K8C1x15EBM   {ID_SAMSUNG,0x220B,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_512Mb, NORFIM_64Kword, NORFIM_16Kword, 4, 0, 0x00000040UL
#define ID_K8C1x15ETM   {ID_SAMSUNG,0x220A,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_512Mb, NORFIM_64Kword, NORFIM_16Kword, 0, 4, 0x00000040UL

/*  Samsung K8D
*/
#define ID_K8D1716UBC   {ID_SAMSUNG,0x2277,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_16Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8D1716UTC   {ID_SAMSUNG,0x2275,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_16Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8D3216UBC   {ID_SAMSUNG,0x22A2,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_32Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8D3216UTC   {ID_SAMSUNG,0x22A0,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_32Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8D6316UBM   {ID_SAMSUNG,0x22E2,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_64Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8D6316UTM   {ID_SAMSUNG,0x22E0,ID_NA,ID_NA},   CMDSET_AMD_STANDARD,  NORFIM_64Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL

/*  Samsung K8F
*/
#define ID_K8F5x15EBM   {ID_SAMSUNG,0x2209,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_64Kword, NORFIM_16Kword, 4, 0, 0x00000040UL
#define ID_K8F5x15ETM   {ID_SAMSUNG,0x2208,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_64Kword, NORFIM_16Kword, 0, 4, 0x00000040UL
#define ID_K8F1x15EBM   {ID_SAMSUNG,0x220D,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_512Mb, NORFIM_64Kword, NORFIM_16Kword, 4, 0, 0x00000040UL
#define ID_K8F1x15ETM   {ID_SAMSUNG,0x220C,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_512Mb, NORFIM_64Kword, NORFIM_16Kword, 0, 4, 0x00000040UL

/*  Samsung K8P
*/
#define ID_K8P2x15UQB   {ID_SAMSUNG,0x257E,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_128Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 8, 0x00000000UL
#define ID_K8P5615UQB   {ID_SAMSUNG,0x227E,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb,NORFIM_128Kword, NORFIM_32Kword, 4, 4, 0x00000040UL

/*  Samsung K8S
*/
#define ID_K8S3215EBE   {ID_SAMSUNG,0x2269,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_32Mb,  NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8S3215ETE   {ID_SAMSUNG,0x2268,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_32Mb,  NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8S6415EBE   {ID_SAMSUNG,0x2251,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_64Mb,  NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8S6415ETE   {ID_SAMSUNG,0x2250,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_64Mb,  NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8S2815EBE   {ID_SAMSUNG,0x22E9,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_128Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8S2815ETE   {ID_SAMSUNG,0x22E8,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_128Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL
#define ID_K8S5615EBE   {ID_SAMSUNG,0x22FF,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_32Kword,  NORFIM_4Kword, 8, 0, 0x00000000UL
#define ID_K8S5615ETE   {ID_SAMSUNG,0x22FE,ID_NA,ID_NA},   CMDSET_AMD_STANDARD, NORFIM_256Mb, NORFIM_32Kword,  NORFIM_4Kword, 0, 8, 0x00000000UL

#endif /* !(FLASH_BUS_WIDTH == 8) */

/*                                                    PrimaryCmdSet,     ChipSize,        BlkSize,    BootBlkSize, L, H,   BufferSize, flags*/
#define ID_ENDOFLIST          {0, 0, 0, 0},                 0x0,            0x0,            0x0,            0x0, 0, 0,          0x0, D_UINT32_MAX


#endif  /* NORFIM_H_INCLUDED */

