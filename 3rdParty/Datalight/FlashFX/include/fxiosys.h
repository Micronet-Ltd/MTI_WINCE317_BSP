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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header defines the types and symbols used for the FlashFX I/O
    System.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxiosys.h $
    Revision 1.52  2011/03/03 23:48:46Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.51  2010/11/23 00:02:49Z  glenns
    Add IOCTL code to disable QuickMount state save upon the next disk dismount.
    Revision 1.50  2010/06/03 19:34:48Z  glenns
    Add clarifying comment about PAGESTATUS_NUMCORRECTEDBITS.  No functional 
    changes.
    Revision 1.49  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.48  2009/11/17 20:29:42Z  garyp
    Corrected the BLOCKSTATUS_MASKRESERVED value.
    Revision 1.47  2009/08/02 18:13:52Z  garyp
    Merged from the v4.0 branch.  Added FXIOFUNC_EXTDRVRFW_OPTIONGET.  Added
    support for FfxVbfRegionMetrics() to the external API.  Added support for
    compaction suspend/resume.  Added ulDeviceStatus to FFXIOSTATUS.  Added 
    support for OTP requests.  Updated the BlockStatus lock flags.  Updated
    so the Read/WriteUncorrectedPages functionality is visible even if NAND 
    support is disabled, as this simplifies some higher level code.  A runtime 
    determination is made to determine which low level interfaces to use.  
    Added support for the "Read/WriteUncorrectedPages" interfaces, and 
    deprecated the "ReadCorrectedPages" interface.   Added support for block
    locking and unlocking.  Added support for power suspend and resume.
    Revision 1.46  2009/04/08 19:17:57Z  garyp
    Updated for AutoDoc -- no functional changes.
    Revision 1.45  2009/03/18 05:40:16Z  glenns
    Fix Bugzilla #2370: Remove definitions of obsolete block status values.
    Revision 1.44  2009/02/09 03:36:44Z  garyp
    Merged from the v4.0 branch.  Define FFXIOREQUEST to be the same as
    DCLIOREQUEST.  Miscellanous minor additions.
    Revision 1.43  2009/01/20 22:11:07Z  glenns
    Added field in FFXIOSTATUS for storing  the location of a correctable error
    when one is found (main page, spare area, main and spare, or can't tell).
    Revision 1.42  2009/01/19 17:45:57Z  keithg
    Removed dependancy between BLOCKINFO_xxx and BLOCKSTATUS_xxx
    Revision 1.41  2009/01/18 08:44:46Z  keithg
    Removed code and compile time condition for support of the old BBM which is
    now obsolete.
    Revision 1.40  2009/01/16 05:46:26Z  glenns
    Modified the PAGESTATUS bitfield macros to provide space for the error
    manager to store bit-correction numbers and policy recommendations in
    the FFXIOSTATUS structure.  Added a number of macros to enable easy
    initialization of FFXIOSTATUS structures without having to use literal
    initializers.
    Revision 1.39  2008/12/24 19:50:04Z  keithg
    Added support new IOCTL support for raw erase operations.
    Revision 1.38  2008/12/22 05:38:22Z  keithg
    Removed unused BBM macros.
    Revision 1.37  2008/12/18 17:25:40Z  keithg
    Replace the WriteNativePages functionality which as incorrectly removed in
    the previous check in.
    Revision 1.36  2008/12/17 23:23:44Z  keithg
    Removed unused values and macros and added RETIRE_RAW_BLOCK.
    Revision 1.35  2008/12/17 02:30:43Z  keithg
    Made status functions compile unconditionally (NANDSUPPORT) only.  Added
    RETIRE_BLOCK support structure and IOCTL.
    Revision 1.34  2008/12/17 01:04:22Z  keithg
    Added xxx_RETIRE_RAW_BLOCK structure and enums.  Made the xxx_BLOCK_STATUS
    interfaces unconditional.
    Revision 1.33  2008/12/15 17:07:25Z  keithg
    Placed original block and status functions into IOCTL lists and structures.
    Revision 1.32  2008/12/12 07:53:03Z  keithg
    Added definitions for reading and writing native pages.  Enabled FIM level
    functions and types for setting block status;  Clarified some naming.
    Revision 1.31  2008/10/24 04:10:01Z  keithg
    Added block info enums and structures and conditioned BBM v5 definitions
    and obsolescent defines.
    Revision 1.30  2008/05/27 16:36:15Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.29  2008/03/27 01:35:17Z  Garyp
    Added a number of new IORequest packets.  Added macros to manage setting
    and retrieving the tag width information from the ulPageStatus value.
    Revision 1.28  2007/12/01 01:47:18Z  Garyp
    Added the INITIAL_BLOCKIO_STATUS() and INITIAL_PAGEIO_STATUS() macros.
    Revision 1.27  2007/11/20 22:34:23Z  jimmb
    Removed the #ifdef from the enum types.
    FXIOFUNC and FXIOSUBFUNC. this will make these types constant.
    Revision 1.26  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.25  2007/11/01 18:40:29Z  billr
    Fix compiler warnings.
    Revision 1.24  2007/11/01 02:08:57Z  Garyp
    Added default FFXIOSTATUS initializer values.
    Revision 1.23  2007/08/30 20:53:50Z  billr
    Remove unused structure member.
    Revision 1.22  2007/07/16 20:28:31Z  Garyp
    Updated to use the generic external requestor logic now implemented in DCL.
    Revision 1.21  2007/06/29 21:11:48Z  rickc
    Added GetPhysicalBlockStatus
    Revision 1.20  2007/06/07 22:42:15Z  rickc
    Removed deprecated vbfread(), vbfwrite(), vbfdiscard(), and vbfclientsize()
    Revision 1.19  2007/04/07 03:40:56Z  Garyp
    Added support for "ParameterSet".
    Revision 1.18  2007/03/09 21:11:52Z  billr
    Fix compiler warning when building without NAND support.
    Revision 1.17  2007/01/31 20:24:50Z  Garyp
    Updated to allow erase-suspend support to be compiled out of the product.
    Revision 1.16  2006/12/07 23:55:44Z  Garyp
    Added some typecasts to avoid problems with some tool chains which don't
    like mixing regular types with enumerated types.
    Revision 1.15  2006/11/10 20:34:45Z  Garyp
    Added "ParameterGet" and "Handle" support to the external API and removed
    "Stats" support.
    Revision 1.14  2006/05/17 17:32:31Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.13  2006/05/08 17:20:18Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.12  2006/04/28 19:52:49Z  billr
    Remove dead code.
    Revision 1.11  2006/03/14 02:40:57Z  garyp
    Added a new block status.
    Revision 1.10  2006/03/10 01:18:40Z  Garyp
    Updated to support a BBM specific block mark.
    Revision 1.9  2006/03/07 19:58:00Z  Garyp
    Added FfxFmlDeviceInfo() support.
    Revision 1.8  2006/03/05 21:10:52Z  billr
    Add a page status value for pages that have an unknown status.
    Revision 1.7  2006/03/02 20:30:43Z  Garyp
    Updated the block status values.
    Revision 1.6  2006/02/26 01:36:24Z  Garyp
    Added pagestatus values.
    Revision 1.5  2006/02/20 20:56:58Z  Garyp
    Updated to get the external API in working order.
    Revision 1.4  2006/02/13 02:55:50Z  Garyp
    Updated to new external API interface.
    Revision 1.3  2006/02/11 23:25:46Z  Garyp
    Tweaked to build cleanly.
    Revision 1.2  2006/02/10 10:16:12Z  Garyp
    Added IORequest support.
    Revision 1.1  2005/11/25 20:57:26Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXIOSYS_H_INCLUDED
#define FXIOSYS_H_INCLUDED

typedef enum
{
    FXIOFUNC_RESERVED               = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x000),

    /*  FML level functions
    */
    FXIOFUNC_FML_ERASE_BLOCKS       = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x100),
    FXIOFUNC_FML_ERASE_RAWBLOCKS,
    FXIOFUNC_FML_READ_PAGES,
    FXIOFUNC_FML_WRITE_PAGES,
    FXIOFUNC_FML_READ_CONTROLDATA,
    FXIOFUNC_FML_WRITE_CONTROLDATA,
    FXIOFUNC_FML_ERASE_BOOT_BLOCKS,
    FXIOFUNC_FML_READ_UNCORRECTEDPAGES,
    FXIOFUNC_FML_READ_TAGGEDPAGES,
    FXIOFUNC_FML_READ_RAWPAGES,
    FXIOFUNC_FML_READ_SPARES,
    FXIOFUNC_FML_READ_TAGS,
    FXIOFUNC_FML_WRITE_UNCORRECTEDPAGES,
    FXIOFUNC_FML_WRITE_TAGGEDPAGES,
    FXIOFUNC_FML_WRITE_RAWPAGES,
    FXIOFUNC_FML_WRITE_SPARES,
    FXIOFUNC_FML_WRITE_TAGS,
    FXIOFUNC_FML_READ_NATIVEPAGES,
    FXIOFUNC_FML_WRITE_NATIVEPAGES,
    FXIOFUNC_FML_GET_BLOCK_STATUS,
    FXIOFUNC_FML_GET_RAW_BLOCK_STATUS,
    FXIOFUNC_FML_SET_RAW_BLOCK_STATUS,
    FXIOFUNC_FML_GET_RAW_PAGE_STATUS,
    FXIOFUNC_FML_GET_BLOCK_INFO,
    FXIOFUNC_FML_GET_RAW_BLOCK_INFO,
    FXIOFUNC_FML_RETIRE_RAW_BLOCK,
    FXIOFUNC_FML_LOCK_FREEZE,
    FXIOFUNC_FML_LOCK_BLOCKS,
    FXIOFUNC_FML_UNLOCK_BLOCKS,
    FXIOFUNC_FML_OTP_READPAGES,
    FXIOFUNC_FML_OTP_WRITEPAGES,
    FXIOFUNC_FML_OTP_LOCK,

    /*  DevMgr level functions which are identical between DEV and FIM
    */
    FXIOFUNC_DEV_READ_GENERIC       = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x200),
    FXIOFUNC_DEV_WRITE_GENERIC,
    FXIOFUNC_DEV_ERASE_RAWBLOCKS,
    FXIOFUNC_DEV_READ_CONTROLDATA,
    FXIOFUNC_DEV_WRITE_CONTROLDATA,
    FXIOFUNC_DEV_GET_BLOCK_STATUS,
    FXIOFUNC_DEV_GET_RAW_BLOCK_STATUS,
    FXIOFUNC_DEV_SET_RAW_BLOCK_STATUS,
    FXIOFUNC_DEV_GET_RAW_PAGE_STATUS,
    FXIOFUNC_DEV_GET_BLOCK_INFO,
    FXIOFUNC_DEV_GET_RAW_BLOCK_INFO,
    FXIOFUNC_DEV_RETIRE_RAW_BLOCK,
    FXIOFUNC_DEV_LOCK_FREEZE,
    FXIOFUNC_DEV_LOCK_BLOCKS,
    FXIOFUNC_DEV_UNLOCK_BLOCKS,
    FXIOFUNC_DEV_POWER_SUSPEND,
    FXIOFUNC_DEV_POWER_RESUME,
    FXIOFUNC_DEV_OTP_READPAGES,
    FXIOFUNC_DEV_OTP_WRITEPAGES,
    FXIOFUNC_DEV_OTP_LOCK,

    /*  DevMgr level functions which are NOT identical between DEV and FIM
    */
    FXIOFUNC_DEV_ERASE_BLOCKS       = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x280),
    FXIOFUNC_DEV_ERASE_BOOT_BLOCKS,

    /*  FIM level functions which are identical between DEV and FIM
    */
    FXIOFUNC_FIM_READ_GENERIC       = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x300),
    FXIOFUNC_FIM_WRITE_GENERIC,
    FXIOFUNC_FIM_READ_CONTROLDATA,
    FXIOFUNC_FIM_WRITE_CONTROLDATA,
    FXIOFUNC_FIM_LOCK_FREEZE,
    FXIOFUNC_FIM_LOCK_BLOCKS,
    FXIOFUNC_FIM_UNLOCK_BLOCKS,
    FXIOFUNC_FIM_POWER_SUSPEND,
    FXIOFUNC_FIM_POWER_RESUME,
    FXIOFUNC_FIM_OTP_READPAGES,
    FXIOFUNC_FIM_OTP_WRITEPAGES,
    FXIOFUNC_FIM_OTP_LOCK,

    /*  FIM level functions which are NOT identical between DEV and FIM
    */
    FXIOFUNC_FIM_ERASE_START        = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x380),
    FXIOFUNC_FIM_ERASE_POLL,
    FXIOFUNC_FIM_ERASE_SUSPEND,
    FXIOFUNC_FIM_ERASE_RESUME,
    FXIOFUNC_FIM_ERASE_BOOT_BLOCK_START,

    FXIOFUNC_FIM_GET_PAGE_STATUS,
    FXIOFUNC_FIM_GET_BLOCK_STATUS,
    FXIOFUNC_FIM_SET_BLOCK_STATUS,
    FXIOFUNC_EXTFML_MIN             = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x500),
    FXIOFUNC_EXTFML_VERSIONCHECK,
    FXIOFUNC_EXTFML_CREATE,
    FXIOFUNC_EXTFML_DESTROY,
    FXIOFUNC_EXTFML_HANDLE,
    FXIOFUNC_EXTFML_DEVICERANGE,
    FXIOFUNC_EXTFML_DEVICEINFO,
    FXIOFUNC_EXTFML_DISKINFO,
    FXIOFUNC_EXTFML_PARAMETERGET,
    FXIOFUNC_EXTFML_PARAMETERSET,
    FXIOFUNC_EXTFML_IOREQUEST,
    FXIOFUNC_EXTFML_MAX,

    FXIOFUNC_EXTVBF_MIN             = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x600),
    FXIOFUNC_EXTVBF_CREATE,
    FXIOFUNC_EXTVBF_DESTROY,
    FXIOFUNC_EXTVBF_HANDLE,
    FXIOFUNC_EXTVBF_DISKINFO,
    FXIOFUNC_EXTVBF_DISKMETRICS,
    FXIOFUNC_EXTVBF_REGIONMETRICS,
    FXIOFUNC_EXTVBF_UNITMETRICS,
    FXIOFUNC_EXTVBF_READPAGES,
    FXIOFUNC_EXTVBF_WRITEPAGES,
    FXIOFUNC_EXTVBF_DISCARDPAGES,
    FXIOFUNC_EXTVBF_COMPACT,
    FXIOFUNC_EXTVBF_COMPACTSUSPEND,
    FXIOFUNC_EXTVBF_COMPACTRESUME,
    FXIOFUNC_EXTVBF_FORMAT,
    FXIOFUNC_EXTVBF_TESTRGNMOUNTPERF,
    FXIOFUNC_EXTVBF_TESTWRITEINTS,
    FXIOFUNC_EXTVBF_DISABLESTATSAVE,
    FXIOFUNC_EXTVBF_MAX,

    FXIOFUNC_EXTDRVRFW_MIN          = DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x800),
    FXIOFUNC_EXTDRVRFW_GETCONFIGINFO,
    FXIOFUNC_EXTDRVRFW_DEVCREATEPARAM,
    FXIOFUNC_EXTDRVRFW_DEVDESTROY,
    FXIOFUNC_EXTDRVRFW_DEVGEOMETRY,
    FXIOFUNC_EXTDRVRFW_DEVHANDLE,
    FXIOFUNC_EXTDRVRFW_DEVENUMERATE,
    FXIOFUNC_EXTDRVRFW_DEVSETTINGS,
    FXIOFUNC_EXTDRVRFW_DEVBBMSETTINGS,
    FXIOFUNC_EXTDRVRFW_DISKCREATEPARAM,
    FXIOFUNC_EXTDRVRFW_DISKDESTROY,
    FXIOFUNC_EXTDRVRFW_DISKGEOMETRY,
    FXIOFUNC_EXTDRVRFW_DISKHANDLE,
    FXIOFUNC_EXTDRVRFW_DISKENUMERATE,
    FXIOFUNC_EXTDRVRFW_DISKGETPARAM,
    FXIOFUNC_EXTDRVRFW_DISKSETPARAM,
    FXIOFUNC_EXTDRVRFW_ALLOCATORCREATE,
    FXIOFUNC_EXTDRVRFW_ALLOCATORDESTROY,
    FXIOFUNC_EXTDRVRFW_OPTIONGET,
    FXIOFUNC_EXTDRVRFW_MAX,

    FXIOFUNC_HIGHLIMIT
} FXIOFUNC;


/*-------------------------------------------------------------------
    FXIOSUBFUNC

    Sub-functions for READ/WRITE_GENERIC.
-------------------------------------------------------------------*/
typedef enum
{
    /*  Device level functions.

        NOTE:  These subfunctions must be specified in the
               EXACT SAME ORDER for both the DEV and FIM
               levels.

        Since the same GENERIC READ/WRITE structures are used
        for both the DEV and FIM levels, we simply do a
        structure copy, and then use the FFX_TRANSFORM_DEV2FIM
        macro to convert the subfunction number.

        LIKEWISE, the READ and WRITE lists must be in the same
        order so that a WRITE function number can be converted
        to the equivalent READ function number by using the
        FFX_TRANSFORM_WRITE2READ.
    */
    FXIOSUBFUNC_DEV_BASE                        = 0x0100,
    FXIOSUBFUNC_DEVREAD_BASE                    = 0x0200,
    FXIOSUBFUNC_DEV_READ_PAGES,
    FXIOSUBFUNC_DEV_READ_UNCORRECTEDPAGES,
    FXIOSUBFUNC_DEV_READ_TAGGEDPAGES,
    FXIOSUBFUNC_DEV_READ_RAWPAGES,
    FXIOSUBFUNC_DEV_READ_SPARES,
    FXIOSUBFUNC_DEV_READ_TAGS,
    FXIOSUBFUNC_DEV_READ_NATIVEPAGES,
    FXIOSUBFUNC_DEVWRITE_BASE                   = 0x0300,
    FXIOSUBFUNC_DEV_WRITE_PAGES,
    FXIOSUBFUNC_DEV_WRITE_UNCORRECTEDPAGES,
    FXIOSUBFUNC_DEV_WRITE_TAGGEDPAGES,
    FXIOSUBFUNC_DEV_WRITE_RAWPAGES,
    FXIOSUBFUNC_DEV_WRITE_SPARES,
    FXIOSUBFUNC_DEV_WRITE_TAGS,
    FXIOSUBFUNC_DEV_WRITE_NATIVEPAGES,

    /*  FIM level functions

        PAY ATTENTION TO THE NOTES ABOVE BEFORE ADDING OR
        REMOVING ENTRIES FROM THESE LISTS!
    */
    FXIOSUBFUNC_FIM_BASE                        = 0x0400,
    FXIOSUBFUNC_FIMREAD_BASE                    = 0x0500,
    FXIOSUBFUNC_FIM_READ_PAGES,
    FXIOSUBFUNC_FIM_READ_UNCORRECTEDPAGES,
    FXIOSUBFUNC_FIM_READ_TAGGEDPAGES,
    FXIOSUBFUNC_FIM_READ_RAWPAGES,
    FXIOSUBFUNC_FIM_READ_SPARES,
    FXIOSUBFUNC_FIM_READ_TAGS,
    FXIOSUBFUNC_FIM_READ_NATIVEPAGES,
    FXIOSUBFUNC_FIMWRITE_BASE                   = 0x0600,
    FXIOSUBFUNC_FIM_WRITE_PAGES,
    FXIOSUBFUNC_FIM_WRITE_UNCORRECTEDPAGES,
    FXIOSUBFUNC_FIM_WRITE_TAGGEDPAGES,
    FXIOSUBFUNC_FIM_WRITE_RAWPAGES,
    FXIOSUBFUNC_FIM_WRITE_SPARES,
    FXIOSUBFUNC_FIM_WRITE_TAGS,
    FXIOSUBFUNC_FIM_WRITE_NATIVEPAGES,

    FXIOSUBFUNC_HIGHLIMIT
} FXIOSUBFUNC;

/*  The typecasts in the following macros exist only to mollify some
    compilers which complain about mixing enumerated types.
*/
#define FFX_TRANSFORM_DEV2FIM(f)    ((FXIOSUBFUNC)((f) + (FXIOSUBFUNC_FIM_BASE - FXIOSUBFUNC_DEV_BASE)))
#define FFX_TRANSFORM_WRITE2READ(f) ((FXIOSUBFUNC)((f) - (FXIOSUBFUNC_DEVWRITE_BASE - FXIOSUBFUNC_DEVREAD_BASE)))


/*-------------------------------------------------------------------
    Type: FFXIOREQUEST

    A structure containing header information for an I/O request
    operation.

    FFXIOREQUEST is defined to be the same thing as DCLIOREQUEST.

    All I/O request packets are defined as structures, with the
    FFXIOREQUEST structure being the first element.
-------------------------------------------------------------------*/
typedef DCLIOREQUEST FFXIOREQUEST;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    D_BUFFER       *pPageData;
}   FFXIOR_FML_READ_PAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    const D_BUFFER *pPageData;
}   FFXIOR_FML_WRITE_PAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    D_BUFFER       *pPageData;
    D_BUFFER       *pSpare;
} FFXIOR_FML_READ_UNCORRECTEDPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    const D_BUFFER *pPageData;
    const D_BUFFER *pSpare;
} FFXIOR_FML_WRITE_UNCORRECTEDPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartBlock;
    D_UINT32        ulBlockCount;
}   FFXIOR_FML_ERASE_BLOCKS,
    FFXIOR_DEV_ERASE_BLOCKS,
    FFXIOR_FML_ERASE_RAWBLOCKS,
    FFXIOR_DEV_ERASE_RAWBLOCKS;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartBlock;
    D_UINT32        ulBlockCount;
}   FFXIOR_FML_LOCK_FREEZE,
    FFXIOR_DEV_LOCK_FREEZE,
    FFXIOR_FIM_LOCK_FREEZE;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartBlock;
    D_UINT32        ulBlockCount;
}   FFXIOR_FML_LOCK_BLOCKS,
    FFXIOR_DEV_LOCK_BLOCKS,
    FFXIOR_FIM_LOCK_BLOCKS;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartBlock;
    D_UINT32        ulBlockCount;
    D_BOOL          fInvert;
}   FFXIOR_FML_UNLOCK_BLOCKS,
    FFXIOR_DEV_UNLOCK_BLOCKS,
    FFXIOR_FIM_UNLOCK_BLOCKS;

typedef struct
{
    FFXIOREQUEST    ior;
    FXIOSUBFUNC     ioSubFunc;
    D_UINT32        ulStartPage;
    D_UINT32        ulCount;
    D_BUFFER       *pPageData;
    D_BUFFER       *pAltData;
    unsigned        nAltDataSize;
}   FFXIOR_DEV_READ_GENERIC,
    FFXIOR_FIM_READ_GENERIC;

typedef struct
{
    FFXIOREQUEST    ior;
    FXIOSUBFUNC     ioSubFunc;
    D_UINT32        ulStartPage;
    D_UINT32        ulCount;
    const D_BUFFER *pPageData;
    const D_BUFFER *pAltData;
    unsigned        nAltDataSize;
}   FFXIOR_DEV_WRITE_GENERIC,
    FFXIOR_FIM_WRITE_GENERIC;

typedef struct
{
    FFXIOREQUEST    ior;
    unsigned        nPowerState;
} FFXIOR_DEV_POWER_SUSPEND, FFXIOR_FIM_POWER_SUSPEND;

typedef struct
{
    FFXIOREQUEST    ior;
    unsigned        nPowerState;
} FFXIOR_DEV_POWER_RESUME, FFXIOR_FIM_POWER_RESUME;



#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulOffset;
    D_UINT32        ulLength;
    D_BUFFER       *pData;
}   FFXIOR_FML_READ_CONTROLDATA,
    FFXIOR_DEV_READ_CONTROLDATA,
    FFXIOR_FIM_READ_CONTROLDATA;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulOffset;
    D_UINT32        ulLength;
    const D_BUFFER *pData;
}   FFXIOR_FML_WRITE_CONTROLDATA,
    FFXIOR_DEV_WRITE_CONTROLDATA,
    FFXIOR_FIM_WRITE_CONTROLDATA;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartBB;  /* numbered as if the entire array was boot blocks */
    D_UINT32        ulBBCount;
}   FFXIOR_FML_ERASE_BOOT_BLOCKS,
    FFXIOR_DEV_ERASE_BOOT_BLOCKS;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartBB;  /* numbered as if the entire array was boot blocks */
    D_UINT32        ulBBCount;
}   FFXIOR_FIM_ERASE_BOOT_BLOCK_START;

#endif


#if FFXCONF_NANDSUPPORT

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    D_BUFFER       *pPageData;
    D_BUFFER       *pSpare;
}   FFXIOR_FML_READ_RAWPAGES,
    FFXIOR_FML_READ_NATIVEPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    const D_BUFFER *pPageData;
    const D_BUFFER *pSpare;
}   FFXIOR_FML_WRITE_RAWPAGES,
    FFXIOR_FML_WRITE_NATIVEPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    D_BUFFER       *pPageData;
    D_BUFFER       *pTag;
    unsigned        nTagSize;
}   FFXIOR_FML_READ_TAGGEDPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    const D_BUFFER *pPageData;
    const D_BUFFER *pTag;
    unsigned        nTagSize;
}   FFXIOR_FML_WRITE_TAGGEDPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulSpareCount;
    D_BUFFER       *pSpare;
}   FFXIOR_FML_READ_SPARES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulSpareCount;
    const D_BUFFER *pSpare;
}   FFXIOR_FML_WRITE_SPARES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulTagCount;
    D_BUFFER       *pTag;
    unsigned        nTagSize;
}   FFXIOR_FML_READ_TAGS;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulTagCount;
    const D_BUFFER *pTag;
    unsigned        nTagSize;
}   FFXIOR_FML_WRITE_TAGS;


#if FFXCONF_BBMSUPPORT

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulBlock;
    D_UINT32        ulRawMapping;
    D_UINT32        ulBlockInfo;
    D_UINT32        ulBlockStatus;
}   FFXIOR_FML_GET_BLOCK_INFO,
    FFXIOR_DEV_GET_BLOCK_INFO,
    FFXIOR_FML_GET_RAW_BLOCK_INFO,
    FFXIOR_DEV_GET_RAW_BLOCK_INFO;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulRawBlock;
    D_UINT32        ulPageCount;
    D_UINT32        ulReason;
}   FFXIOR_FML_RETIRE_RAW_BLOCK,
    FFXIOR_DEV_RETIRE_RAW_BLOCK;

#endif


typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulPage;
}   FFXIOR_FML_GET_RAW_PAGE_STATUS,
    FFXIOR_DEV_GET_RAW_PAGE_STATUS,
    FFXIOR_FIM_GET_PAGE_STATUS;


typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulBlock;
}   FFXIOR_FML_GET_BLOCK_STATUS,
    FFXIOR_DEV_GET_BLOCK_STATUS,
    FFXIOR_FIM_GET_BLOCK_STATUS,
    FFXIOR_FML_GET_RAW_BLOCK_STATUS,
    FFXIOR_DEV_GET_RAW_BLOCK_STATUS;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulBlock;
    D_UINT32        ulBlockStatus;
}   FFXIOR_FML_SET_RAW_BLOCK_STATUS,
    FFXIOR_DEV_SET_RAW_BLOCK_STATUS,
    FFXIOR_FIM_SET_BLOCK_STATUS;
typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    D_BUFFER       *pPageData;
}   FFXIOR_FML_READ_OTPPAGES,
    FFXIOR_DEV_READ_OTPPAGES,
    FFXIOR_FIM_READ_OTPPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    const D_BUFFER *pPageData;
}   FFXIOR_FML_WRITE_OTPPAGES,
    FFXIOR_DEV_WRITE_OTPPAGES,
    FFXIOR_FIM_WRITE_OTPPAGES;

typedef struct
{
    FFXIOREQUEST    ior;
}   FFXIOR_FML_LOCK_OTP,
    FFXIOR_DEV_LOCK_OTP,
    FFXIOR_FIM_LOCK_OTP;

#endif  /* FFXCONF_NANDSUPPORT */


/*-------------------------------------------------------------------
    Type: FFXIOSTATUS

    Contains the results of an I/O request.  The FFXIOSTATUS structure
    has the following layout:

	<pre>
    {{+-----------------------------------------+}}
    {{|                                         |}}
    {{|  typedef struct                         |}}
    {{|  {                                      |}}
    {{|      D_UINT32    ulCount;               |}}
    {{|      FFXSTATUS   ffxStat;               |}}
    {{|      D_UINT32    ulFlags;               |}}
    {{|      union                              |}}
    {{|      {                                  |}}
    {{|          D_UINT32    ulDeviceStatus;    |}}
    {{|          D_UINT32    ulBlockStatus;     |}}
    {{|          D_UINT32    ulPageStatus;      |}}
    {{|          D_UINT32    ulByteStatus;      |}}
    {{|      } op;                              |}}
    {{|  } FFXIOSTATUS;                         |}}
    {{|                                         |}}
    {{+-----------------------------------------+}}
	</pre>

    *ulcount* - The count of elements which were successfully
                transferred.
    *ffxStat* - The standard FFXSTATUS value.
    *ulFlags* - Flags pertaining to the results, including the
                operation type, which will be one of the following
                (all other bits are reserved):
              + IOFLAGS_UNKNOWN - 0x00000000
              + IOFLAGS_BLOCK   - 0x10000000
              + IOFLAGS_PAGE    - 0x20000000
              + IOFLAGS_BYTE    - 0x30000000
              + IOFLAGS_DEVICE  - 0x40000000

    *op.ulxxxStatus* - Status information specific to each operation type.


    For IOFLAGS_BLOCK operations, op.ulBlockStatus will be one of
    the following values:
        - BLOCKSTATUS_NOTBAD
        - BLOCKSTATUS_IOERROR
        - BLOCKSTATUS_FACTORYBAD
        - BLOCKSTATUS_MARKEDBAD
        - BLOCKSTATUS_LEGACYNOTBAD
        - BLOCKSTATUS_BBMBLOCK
        - BLOCKSTATUS_LEGACYFACTORYBAD
        - BLOCKSTATUS_BBMREMAPPED
        - BLOCKSTATUS_SOFT_LOCKED
        - BLOCKSTATUS_DEV_LOCKED
        - BLOCKSTATUS_DEV_LOCKFROZEN

    For IOFLAGS_PAGE operations, op.ulPageStatus will be one of
    the following values:
        - PAGESTATUS_UNKNOWN
        - PAGESTATUS_DATACORRECTED
        - PAGESTATUS_WRITTENWITHECC
        - PAGESTATUS_UNWRITTEN

   The following methods are used to determine the state of
   an operation:

   1) If ffxStat == FFXSTAT_SUCCESS, then the operation was entirely
      successful.  ulCount will indicate the count of elements
      transferred, however examining this value is not necessary.

   2) If ffxStat == FFXSTAT_FIMIOERROR, then the operation <may> have
      been partially successful.  ulCount will indicate the number of
      elements that were transferred successfully, if any.  The
      element indicated by ulCount+1 will be the element where the
      I/O error occurred.  For write operations, it should be assumed
      that the element where the I/O error occurred was modified.

   3) If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, then the operation
      <may> have been partially successful.  ulCount will indicate
      the number of elements that were transferred successfully, if
      any.  The element indicated by ulCount+1 will be the element
      where the uncorrectable error occurred.  This ffxStat value
      is only legitimate for read operations.

    In all cases, for read operations, where one or more elements are
    successfully returned, if the PAGESTATUS_DATACORRECTED flag is
    set within the op.ulPageStatus field, then a correctable error
    was encountered and fixed.
-------------------------------------------------------------------*/
struct tagFFXIOSTATUS
{
    D_UINT32    ulCount;            /* Number of elements transferred   */
    FFXSTATUS   ffxStat;            /* Standard values from fxstatus.h  */
    D_UINT32    ulFlags;
    union
    {
        D_UINT32    ulDeviceStatus; /* (should change to "chip" once our FIM abstraction changes) */
        D_UINT32    ulBlockStatus;
        D_UINT32    ulPageStatus;
        D_UINT32    ulByteStatus;
    } op;                           /* operation specific information */
};

/*  High 4-bits indicate the operation type
*/
#define IOFLAGS_TYPEMASK                0xF0000000
#define IOFLAGS_UNKNOWN                 0x00000000
#define IOFLAGS_BLOCK                   0x10000000
#define IOFLAGS_PAGE                    0x20000000
#define IOFLAGS_BYTE                    0x30000000
#define IOFLAGS_DEVICE                  0x40000000  /* (should change to "chip" once our FIM abstraction changes) */

/*---------------------------------------------------------
   Status values for op.ulBlockStatus
---------------------------------------------------------*/
#define BLOCKSTATUS_MASKRESERVED        (~(BLOCKSTATUS_MASKTYPE | BLOCKSTATUS_MASKLOCK))

#define BLOCKSTATUS_MASKTYPE            0x0000000F  /* Low 4 bits for Bad Block type */
#define BLOCKSTATUS_UNKNOWN             0x00000000
#define BLOCKSTATUS_NOTBAD              0x00000001
#define BLOCKSTATUS_FACTORYBAD          0x00000003
#define BLOCKSTATUS_LEGACYNOTBAD        0x00000005

#define BLOCKSTATUS_MASKLOCK            0x00000070  /* 3 bits for block lock status */
#define BLOCKSTATUS_SOFT_LOCKED         0x00000010  /* Locked via software */
#define BLOCKSTATUS_DEV_LOCKED          0x00000020  /* Locked via hardware */
#define BLOCKSTATUS_DEV_LOCKFROZEN      0x00000040  /* (Really a DEVICE-wide setting!  The DEVICE can be frozen, */
                                                    /* but a given block may be frozen in the unlocked state.)   */


/*---------------------------------------------------------
    Status values for ulBlockInfo
---------------------------------------------------------*/
#define BLOCKINFO_UNKNOWN               0x00000080
#define BLOCKINFO_GOODBLOCK             0x00000081
#define BLOCKINFO_FACTORYBAD            0x00000082
#define BLOCKINFO_RETIRED               0x00000083
#define BLOCKINFO_RESERVED              0x00000084
#define BLOCKINFO_BBMSYSTEM             0x00000085
#define BLOCKINFO_TEMPORARY             0x00000086


/*---------------------------------------------------------
    Status values for op.ulPageStatus

    Note on the PAGESTATUS_NUMCORRECTEDBITS field:
    This field contains 8 bits for the hardware interface
    modules to encode the number of corrected bits during
    a read.  Some hardware modules may not be able to do
    this, and may instead simply need to recommend a
    scrubbing operation.  A value of 0xFF encoded in this 
    field is reserved for this purpose.
---------------------------------------------------------*/
#define PAGESTATUS_MASKRESERVED         0xF0000008
#define PAGESTATUS_MASKTAGWIDTH         0x0FF00000  /* 8 bits for the tag width */
#define PAGESTATUS_NUMCORRECTEDBITS     0x000FF000  /* 8 bits for corrected bit count */
#define PAGESTATUS_CORRECTIONLOCALE     0x00000C00  /* 2 bits for correction locale */
#define PAGESTATUS_ERRPOLICY            0x000003F0  /* 6 bits for err policy recommendation */
#define PAGESTATUS_MASKFLAGS            0x00000007
#define PAGESTATUS_UNKNOWN              0x00000000  /* invalid value in page status    */
#define PAGESTATUS_DATACORRECTED        0x00000001  /* correctable error encountered */
#define PAGESTATUS_WRITTENWITHECC       0x00000002  /* page was written with ECC     */
#define PAGESTATUS_UNWRITTEN            0x00000004  /* page has not been written     */


/*---------------------------------------------------------
    Macro to get/set the tag width value in the
    op.ulPageStatus field.
---------------------------------------------------------*/
#define PAGESTATUS_SET_TAG_WIDTH(pstat) (((D_UINT32)(pstat)) << 20)
#define PAGESTATUS_GET_TAG_WIDTH(pstat) (((pstat) & PAGESTATUS_MASKTAGWIDTH) >> 20)


/*---------------------------------------------------------
    Handy-dandy macro to check if everything
    was successful in an I/O operation.
---------------------------------------------------------*/
#define IOSUCCESS(io, count) (io.ulCount == (count) && io.ffxStat == FFXSTAT_SUCCESS)


/*---------------------------------------------------------
    Default FFXIOSTATUS initializers
---------------------------------------------------------*/
#define NULL_IO_STATUS                    {0, FFXSTAT_SUCCESS, IOFLAGS_UNKNOWN, {0}}

#define INITIAL_DEVICEIO_STATUS(stat)     {0, (stat), IOFLAGS_DEVICE, {0}}
#define INITIAL_BLOCKIO_STATUS(stat)      {0, (stat), IOFLAGS_BLOCK, {0}}
#define INITIAL_PAGEIO_STATUS(stat)       {0, (stat), IOFLAGS_PAGE, {0}}
#define INITIAL_BYTEIO_STATUS(stat)       {0, (stat), IOFLAGS_BYTE, {0}}
#define INITIAL_UNKNOWNFLAGS_STATUS(stat) {0, (stat), IOFLAGS_UNKNOWN, {0}}

#define INITIAL_BLOCKIO_STATUS_WTIH_COUNT(count, stat)      {(count), (stat), IOFLAGS_BLOCK, {0}}
#define INITIAL_PAGEIO_STATUS_WITH_COUNT(count, stat)       {(count), (stat), IOFLAGS_PAGE, {0}}
#define INITIAL_BYTEIO_STATUS_WITH_COUNT(count, stat)       {(count), (stat), IOFLAGS_BYTE, {0}}
#define INITIAL_UNKNOWNFLAGS_STATUS_WITH_COUNT(count, stat) {(count), (stat), IOFLAGS_UNKNOWN, {0}}

#define DEFAULT_DEVICEIO_STATUS             INITIAL_DEVICEIO_STATUS(FFXSTAT_BADPARAMETER)
#define DEFAULT_BLOCKIO_STATUS              INITIAL_BLOCKIO_STATUS(FFXSTAT_BADPARAMETER)
#define DEFAULT_PAGEIO_STATUS               INITIAL_PAGEIO_STATUS(FFXSTAT_BADPARAMETER)
#define DEFAULT_BYTEIO_STATUS               INITIAL_BYTEIO_STATUS(FFXSTAT_BADPARAMETER)

#define DEFAULT_UNKNOWNFLAGS_STATUS         INITIAL_UNKNOWNFLAGS_STATUS(FFXSTAT_BADPARAMETER)
#define DEFAULT_GOOD_BLOCKIO_STATUS         INITIAL_BLOCKIO_STATUS(FFXSTAT_SUCCESS)
#define DEFAULT_GOOD_PAGEIO_STATUS          INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS)
#define DEFAULT_UNSUPPORTEDFUNCTION_PAGEIO_STATUS INITIAL_PAGEIO_STATUS(FFXSTAT_UNSUPPORTEDFUNCTION)
#define DEFAULT_BADHANDLE_PAGEIO_STATUS     INITIAL_PAGEIO_STATUS(FFXSTAT_BADHANDLE)
#define DEFAULT_FAILURE_BLOCKIO_STATUS      INITIAL_BLOCKIO_STATUS(FFXSTAT_FAILURE)
#define DEFAULT_BADSTRUCLEN_BYTEIO_STATUS   INITIAL_BYTEIO_STATUS(FFXSTAT_BADSTRUCLEN)
#define DEFAULT_BADSTRUCLEN_PAGEIO_STATUS   INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN)
#define DEFAULT_BADSTRUCLEN_BLOCKIO_STATUS  INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN)

#endif /* FXIOSYS_H_INCLUDED */


