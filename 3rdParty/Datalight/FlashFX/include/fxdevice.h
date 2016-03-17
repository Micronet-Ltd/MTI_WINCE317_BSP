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

    This header contains generic Device specific symbols and structures
    which are used across multiple layers.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxdevice.h $
    Revision 1.28  2011/03/03 23:43:43Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.27  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.26  2009/07/29 18:28:16Z  garyp
    Merged from the v4.0 branch.  Factored the Device settings structures out
    of here.  Added the DEV_USING_BBM device flag.  Added support for locking.
    Revision 1.25  2009/04/08 19:17:59Z  garyp
    Updated for AutoDoc -- no functional changes.
    Revision 1.24  2009/03/31 16:50:42Z  keithg
    Added an fAlwaysMount flag for BBM initialization to allow media
    access in the event of a BBM failed format or mount.
    Revision 1.23  2009/01/24 00:52:59Z  billr
    Update copyright date.
    Revision 1.22  2009/01/23 18:33:11Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Comment
    changes only.
    Revision 1.21  2008/03/27 02:07:01Z  Garyp
    Added the DEV_VARIABLE_LENGTH_TAGS flag.  Eliminated the obsolete MEM_*
    settings.  Enhanced the logic for validating the max page sizes and
    simplified the logic for specifying the max spare size.  Renamed the
    FFXBBMFORMATNIFO structure to the more accurate FFXBBMSETTINGS, and added
    a field to specify whether BBM should be used at all.
    Revision 1.20  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.19  2007/09/12 20:16:48Z  Garyp
    Added 4KB NAND page size support.
    Revision 1.18  2007/08/02 17:45:40Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.17  2007/08/01 21:56:52Z  timothyj
    Removed obsolete FFX_FLASHOFFSET.
    Revision 1.16  2007/04/17 22:20:27Z  rickc
    Added default NOR flash config.
    Revision 1.15  2007/03/01 21:17:17Z  timothyj
    Interim changed FXDEVSETTINGS to use new width-nonspecific
    FFX_FLASHOFFSET_MAX in lieu of 32-bit specific D_UINT32_MAX,
    currently only D_UINT32 is supported.
    Revision 1.14  2007/02/20 17:53:39Z  Garyp
    Removed obsolete and/or dead code.
    Revision 1.13  2007/02/01 02:42:10Z  Garyp
    Added support for handling device timing and latency settings.
    Revision 1.12  2007/01/30 18:31:35Z  rickc
    Added NOR interleave configuration constants.
    Revision 1.11  2007/01/02 21:11:17Z  Garyp
    Added the DEV_OLDSTYLE_NORFIM so we can clearly distinguish old and new
    style NOR FIMs at the DevMgr level.
    Revision 1.10  2006/08/21 22:34:30Z  Garyp
    Modified to automatically configure FFX_NAND_MAXSPARESIZE if it can be
    derived from FFX_NAND_MAXPAGESIZE.
    Revision 1.9  2006/06/13 23:00:52Z  Pauli
    Rearranged some preprocessor statements to work around a bug in the
    obfuscator.
    Revision 1.8  2006/06/08 18:25:26Z  jeremys
    Added some checks to fix problems with GCC.
    Revision 1.7  2006/04/28 20:29:43Z  billr
    Remove dead code.
    Revision 1.6  2006/03/04 22:00:07Z  Garyp
    Added better compile-time validation checks.  Changed the base flash
    address pointer to be void *.
    Revision 1.5  2006/03/03 20:04:54Z  Garyp
    Updated preprocessor checking.
    Revision 1.4  2006/02/26 02:30:54Z  Garyp
    Modified so that knowledge of the NAND spare area format is only known
    to the NTM.
    Revision 1.3  2006/02/24 04:36:16Z  Garyp
    Updated to use refactored headers.
    Revision 1.2  2006/02/22 19:04:00Z  Garyp
    Added logic to calculate a system-wide max page size.
    Revision 1.1  2006/02/08 18:45:52Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXDEVICE_H_INCLUDED
#define FXDEVICE_H_INCLUDED


/*-------------------------------------------------------------------
    Erased flash is all 1's
-------------------------------------------------------------------*/
#define ERASED8               0xFFU
#define ERASED16            0xFFFFU
#define ERASED32        0xFFFFFFFFUL


/*-------------------------------------------------------------------
    Max Page Size Overview

    Maximum page sizes are specified or calculated at compile-time
    for each of the NAND, NOR, and ISWF flash types.  A driver-wide
    maximum page size -- FFX_MAX_PAGESIZE -- is used as well.  These
    values are managed separately, and are important in ensuring
    efficient memory usage by the driver.

    In all cases, any sizes which are defined in the ffxconf.h file
    take precedence over those calculated here.  These calculations
    are only used in the event that the ffxconf.h file does not
    contain all the necessary settings.  Regardless how the values
    are calculated, they must pass some basic validity checks,
    mainly, that FFX_MAX_PAGESIZE must be at least as large as the
    largest page size for each of the flash types.  Also, at run-
    time, the hardware page sizes must not exceed the compile-time
    values.

    The initialization process works as follows:

    For each of NAND, NOR, and ISWF, if the max page size is not
    already defined in ffxconf.h, but FFX_MAX_PAGESIZE is defined,
    that value is used.  If FFX_MAX_PAGESIZE is not defined either,
    then a default page size is chosen for each different flash
    type.

    After the max page sizes for each flash type are determined, if
    FFX_MAX_PAGESIZE is not already specified, it is set to the
    largest page size of the different flash types.

    For NAND, the max spare size is automatically calculated as
    1/32nd of the NAND max page size.

    The page sizes are then validated to ensure that the assumptions
    above are true.
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Define the max sizes of the NAND pages we support.
-------------------------------------------------------------------*/
#if FFXCONF_NANDSUPPORT
  #define NAND512_PAGE              (512)
  #define NAND512_SPARE             (16)

  #define NAND2K_PAGE               (2048)
  #define NAND2K_SPARE              (64)

  #define NAND4K_PAGE               (4096)
  #define NAND4K_SPARE              (128)

  #if !defined(FFX_NAND_MAXPAGESIZE)
    #if defined(FFX_MAX_PAGESIZE)
      #define FFX_NAND_MAXPAGESIZE  (FFX_MAX_PAGESIZE)
    #else
      #define FFX_NAND_MAXPAGESIZE  (NAND2K_PAGE)
    #endif
  #endif

  #if !defined(FFX_NAND_MAXSPARESIZE)
    /*  Define the spare size in terms of the specified page size.
    */
    #define FFX_NAND_MAXSPARESIZE   (FFX_NAND_MAXPAGESIZE / 32)
  #endif

  #define FFX_NAND_LEGACYTAGSIZE    (2)
  #define FFX_NAND_MAXTAGSIZE       (FFX_NAND_LEGACYTAGSIZE)

#else

  #ifdef FFX_NAND_MAXPAGESIZE
    #undef FFX_NAND_MAXPAGESIZE
    #undef FFX_NAND_MAXSPARESIZE
  #endif

  /*  Temporarily define this to zero, so the checks below work.
  */
  #define FFX_NAND_MAXPAGESIZE      (0)

#endif

/*-------------------------------------------------------------------
    Define the max sizes of the ISWF pages we support.
-------------------------------------------------------------------*/
#if FFXCONF_ISWFSUPPORT
  #if !defined(FFX_ISWF_MAXPAGESIZE)
    #if defined(FFX_MAX_PAGESIZE)
      #define FFX_ISWF_MAXPAGESIZE  (FFX_MAX_PAGESIZE)
    #else
      #define FFX_ISWF_MAXPAGESIZE  (1024)
    #endif
  #endif

#else

  #ifdef FFX_ISWF_MAXPAGESIZE
    #undef FFX_ISWF_MAXPAGESIZE
  #endif

  /*  Temporarily define this to zero, so the checks below work.
  */
  #define FFX_ISWF_MAXPAGESIZE      (0)
#endif

/*-------------------------------------------------------------------
    Define the max sizes of the NOR pages we support.
-------------------------------------------------------------------*/
#if FFXCONF_NORSUPPORT
  #if !defined(FFX_NOR_MAXPAGESIZE)
    #if defined(FFX_MAX_PAGESIZE)
      #define FFX_NOR_MAXPAGESIZE   (FFX_MAX_PAGESIZE)
    #else
      #define FFX_NOR_MAXPAGESIZE   (512)
    #endif
  #endif

#else

  #ifdef FFX_NOR_MAXPAGESIZE
    #undef FFX_NOR_MAXPAGESIZE
  #endif

  /*  Temporarily define this to zero, so the checks below work.
  */
  #define FFX_NOR_MAXPAGESIZE       (0)
#endif

/*-------------------------------------------------------------------
    Based on the above settings, determine driver-wide maximum page
    sizes -- if it has not already been specified.  Only use values
    for the flash types that are enabled.
-------------------------------------------------------------------*/
#ifndef FFX_MAX_PAGESIZE
  #if FFXCONF_NANDSUPPORT && \
        (FFX_NAND_MAXPAGESIZE >= FFX_ISWF_MAXPAGESIZE) && \
        (FFX_NAND_MAXPAGESIZE >= FFX_NOR_MAXPAGESIZE)
    #define FFX_MAX_PAGESIZE        FFX_NAND_MAXPAGESIZE
  #elif FFXCONF_ISWFSUPPORT && \
        (FFX_ISWF_MAXPAGESIZE >= FFX_NAND_MAXPAGESIZE) && \
        (FFX_ISWF_MAXPAGESIZE >= FFX_NOR_MAXPAGESIZE)
    #define FFX_MAX_PAGESIZE        FFX_ISWF_MAXPAGESIZE
  #else
    #define FFX_MAX_PAGESIZE        FFX_NOR_MAXPAGESIZE
  #endif
#endif

/*-------------------------------------------------------------------
    Now validate the settings, since developer settings may override
    the stuff above, and it could be set wrong.  Finally, we will
    #undef things which are not relevant.
-------------------------------------------------------------------*/
#if FFXCONF_NANDSUPPORT
  #if FFX_NAND_MAXPAGESIZE > FFX_MAX_PAGESIZE
    #error "FFX: FFX_NAND_MAXPAGESIZE is greater than FFX_MAX_PAGESIZE"
  #endif
  /*  Reality check...
  */
  #if FFX_NAND_MAXPAGESIZE != (FFX_NAND_MAXSPARESIZE * 32)
    #error "FFX: NAND page/spare size ratio appears to be invalid"
  #endif

  #define FFX_MAX_SPARESIZE FFX_NAND_MAXSPARESIZE
#else
  #define FFX_MAX_SPARESIZE (0)
  #undef FFX_NAND_MAXPAGESIZE
#endif

#if FFXCONF_ISWFSUPPORT
  #if FFX_ISWF_MAXPAGESIZE > FFX_MAX_PAGESIZE
    #error "FFX: FFX_ISWF_MAXPAGESIZE is greater than FFX_MAX_PAGESIZE"
  #endif
#else
 #undef FFX_ISWF_MAXPAGESIZE
#endif

#if FFXCONF_NORSUPPORT
  #if FFX_NOR_MAXPAGESIZE > FFX_MAX_PAGESIZE
    #error "FFX: FFX_NOR_MAXPAGESIZE is greater than FFX_MAX_PAGESIZE"
  #endif
#else
 #undef FFX_NOR_MAXPAGESIZE
#endif



/*-------------------------------------------------------------------
    Device/FIM level reserved space and max array size information.
-------------------------------------------------------------------*/
typedef struct FFXFIMBOUNDS
{
    D_UINT32           ulReservedBottomKB;
    D_UINT32           ulReservedTopKB;
    D_UINT32           ulMaxArraySizeKB;         /* 0 denotes maximum */
    D_UINT32           ulMaxScanTotalKB;
} FFXFIMBOUNDS;


/*-------------------------------------------------------------------
    Possible device types returned in the NTMINFO, FFXDEVINFO, and
    FMLINFO structures.
-------------------------------------------------------------------*/
#define DEVTYPE_ROM       (0x0000)
#define DEVTYPE_RAM       (0x0001)
#define DEVTYPE_ATA       (0x0002)
#define DEVTYPE_NOR       (0x0003)
#define DEVTYPE_NAND      (0x0004)
#define DEVTYPE_AND       (0x0005)
#define DEVTYPE_ISWF      (0x0006)


/*-------------------------------------------------------------------
    Possible NOR flash configurations
-------------------------------------------------------------------*/
#define NORFLASH_DEFAULT    (0x00)
#define NORFLASH_x8         (0x01)
#define NORFLASH_2x8        (0x02)
#define NORFLASH_4x8        (0x03)
#define NORFLASH_x16        (0x04)
#define NORFLASH_2x16       (0x05)


/*-------------------------------------------------------------------
    Device special characteristics.  These flags represent different
    characteristics that higher-level software (typically the Media
    Manager) may need to accommodate.
-------------------------------------------------------------------*/

/*  Set this bit if the device requires the BBM
*/
#define DEV_REQUIRES_BBM            (1U << 15)

/*  Set this bit if the device is using BBM
*/
#define DEV_USING_BBM               (1U << 14)

/*  NOTE: This flag applies to MLC NOR only, it has nothing to
    do with MLC NAND.

    Indicates flash is not MLC (multi-level cell).  Allows an
    allocation to be discarded with a single write instead of
    requiring two writes.  The sense of this bit is chosen for
    backwards compatibility: an old FIM that doesn't set this
    bit is handled in safe but slow fashion.
*/
#define DEV_NOT_MLC                 (1U << 13)

/*  Flash will not accept writing '1' to a bit previously
    programmed to '0'.  Merge new data with old when writing
    to a previously programmed location.
*/
#define DEV_MERGE_WRITES            (1U << 12)

/*  NOTE: This flag has different meanings for NOR and NAND.

    Some preliminary spec sheets for NOR flash parts prohibit
    programming a bit to zero more than once, indicating that
    doing this can produce an unerasable bit.  Note that the
    DEV_PGM_ONCE flags is incompatible with DEV_MERGE_WRITES;
    only one of these bits should be set at a time.

    MLC NAND flash does not allow more than one program operation
    per page before erasing; on some parts it may actually be 
    physically impossible.
*/
#define DEV_PGM_ONCE                (1U << 11)

/*  Set this bit if the device requires ECC
*/
#define DEV_REQUIRES_ECC            (1U << 10)

/*  Set this bit for old-style NOR FIMS (handled by norwrap.c)
*/
#define DEV_OLDSTYLE_NORFIM         (1U << 9)

/*  Set this bit for NTMs which support variable length tag data.
*/
#define DEV_VARIABLE_LENGTH_TAGS    (1U << 8)

/*  This flag indicates that the device supports locking blocks
    or pages in some fashion.  If this bit is clear, then none
    of the flags in the FFXFIMDEVINFO.uLockFlags are used.
*/
#define DEV_LOCKABLE                (1U << 7)



#endif /* FXDEVICE_H_INCLUDED */
