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

    This header is used to define standard project-wide macros.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxmacros.h $
    Revision 1.49  2010/12/18 22:20:46Z  garyp
    Fixed to use ASCII quote characters.
    Revision 1.48  2010/11/19 23:54:59Z  glenns
    Added new macros for allocator option flags.
    Revision 1.47  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.46  2009/11/11 00:53:13Z  garyp
    Updated to initialize default values for FFXCONF_SHELL_TESTS and
    FFXCONF_SHELL_TOOLS (should be done in ffxconf.h).
    Revision 1.45  2009/10/13 16:05:01Z  billr
    Set default for FFXCONF_FATFORMATSUPPORT before testing it.
    Revision 1.44  2009/07/31 19:26:10Z  garyp
    Merged from the v4.0 branch.  Added the compaction and shutdown flags
    values.  Added the FFX_CHIP_REMAINING and FFX_DEVCHIP() macros, to allow
    Device size and reserved space to be specified at chip boundaries.  Added
    FFX_NEXT.  Moved the default Disk and Device settings into this file from
    fxtypes.h.  Added OTP support.
    Revision 1.43  2009/04/15 17:15:16Z  thomd
    Set default for FFXCONF_FATFORMATSUPPORT before testing it.
    Revision 1.42  2009/04/10 01:21:06Z  keithg
    Removed explicit initializer for the reserved field of the BBMSETTINGS.
    Revision 1.41  2009/04/09 06:53:07Z  keithg
    Updated ulSparePercent description.
    Revision 1.40  2009/04/08 19:17:59Z  garyp
    Updated for AutoDoc -- no functional changes.
    Revision 1.39  2009/04/08 01:01:47Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.38  2009/04/02 20:34:31Z  keithg
    Now errors if FFXCONF_READAHEADENABLED is turned on.
    Revision 1.37  2009/04/01 06:56:29Z  keithg
    Default FFXCONF_READAHEADENABLED to FALSE.  This is a workaround for 
    bug 2561.
    Revision 1.36  2009/03/23 00:38:43Z  keithg
    Corrected bug reference in v1.35
    Revision 1.35  2009/03/10 17:04:41Z  glenns
    Fixed bug 2238: Set default state of FFXCONF_MIGRATE_LEGACY_FLASH to FALSE.
    Revision 1.34  2009/01/19 04:34:44Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.33  2008/12/23 06:06:28Z  keithg
    Removed obsolete macro FFX_BBMFORMAT_REUSE_DISCOVEREDBADBLOCKS
    Revision 1.32  2008/12/22 07:37:37Z  keithg
    Removed unused BBM macros.
    Revision 1.31  2008/12/15 21:15:38Z  keithg
    Added default definition for FFXCONF_BBMSUPPORT.
    Revision 1.30  2008/12/15 18:05:37Z  keithg
    Changed default BBM_USE_V5 to be enabled.
    Revision 1.29  2008/11/19 23:21:40Z  keithg
    Added default macro definitions from fxtypes.h
    Revision 1.28  2008/10/08 20:33:21Z  keithg
    Added BBM format types and conditioned obsolescent BBM defines.
    Revision 1.27  2008/09/02 05:59:47Z  keithg
    Added condition for obsolescent BBM code.
    Revision 1.26  2008/06/03 20:49:51Z  thomd
    Removed ffxconf LFA conditions
    Revision 1.25  2008/05/27 16:36:15Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.24  2008/03/27 01:39:54Z  Garyp
    Added FFX_COMPACT_NONE as a synonym for FFX_COMPACT_SYNCHRONOUS.  
    Added default initialization values for FFXCONF_ALLOCATORSUPPORT 
    and FFXCONF_VBFSUPPORT.
    Revision 1.23  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.22  2007/10/01 19:23:01Z  pauli
    Corrected the default definition for FFXCONF_HRFSSUPPORT.
    Revision 1.21  2007/08/29 22:26:43Z  pauli
    Renamed USE_DEFAULT to FFX_USE_DEFAULT.
    Revision 1.20  2007/07/12 19:01:04Z  pauli
    BZ 603: Added new FFXCONF_BBM_FAST_FACTORY_FORMAT conditional used to
    determine whether FFX_FORMAT_ONCE-style BBM formats result in a format
    that erases all of the good blocks.
    Revision 1.19  2007/05/23 00:29:13Z  Garyp
    Updated to support hrFs.
    Revision 1.18  2007/04/22 20:15:57Z  Garyp
    Added BBM stats support.
    Revision 1.17  2007/03/30 23:32:59Z  timothyj
    Added default FFXCONF_LARGE_FLASH for configuring large flash array support
    (BZ/IR #953).
    Revision 1.16  2007/02/05 22:35:12Z  Garyp
    Updated to define the file system types in terms of the symbols provided
    by DCL.
    Revision 1.15  2007/01/31 20:24:48Z  Garyp
    Added latency related symbol validation.
    Revision 1.14  2006/11/10 18:07:47Z  Garyp
    Standardized on a naming convention for the various "stats" interfaces.
    Modified the stats query to use the generic ParameterGet() ability now
    implemented through most all layers of the code.
    Revision 1.13  2006/10/16 20:58:26Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.12  2006/10/11 22:20:21Z  Garyp
    Updated to work with new formatting features which can now be scaled
    out of the code base.
    Revision 1.11  2006/10/06 00:32:42Z  Garyp
    Eliminated the DBGPRINTF() macro and the byte-order conversion macros
    (now in DCL).
    Revision 1.10  2006/07/21 22:03:35Z  Garyp
    Corrected some preprocessor logic.
    Revision 1.9  2006/06/21 22:53:51Z  Garyp
    Updated to handle new region tracking options.
    Revision 1.8  2006/05/28 23:08:36Z  Garyp
    Improved FAT settings validation.
    Revision 1.7  2006/04/05 09:07:01Z  Garyp
    Removed some invalid and unused reserved space settings.
    Revision 1.6  2006/03/16 19:00:33Z  Garyp
    Added the FFXCONF_ACCEPT_OLD_BBM setting.
    Revision 1.5  2006/03/14 02:32:05Z  garyp
    Added the FFXCONF_MIGRATE_LEGACY_FLASH setting.
    Revision 1.4  2006/01/25 03:44:52Z  Garyp
    Added Sibley support.
    Revision 1.3  2005/12/31 03:06:57Z  Garyp
    Updated to use a renamed type.
    Revision 1.2  2005/12/17 21:11:09Z  garyp
    Updated to default to compaction tracking off, if not already set.
    Revision 1.1  2005/12/05 15:54:38Z  Pauli
    Initial revision
    Revision 1.3  2005/12/05 15:54:38Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.2  2005/11/23 21:20:46Z  Garyp
    Added support for new BBM configuration settings.
    Revision 1.1  2005/08/21 10:42:06Z  Garyp
    Initial revision
    Revision 1.3  2005/08/05 17:40:55Z  Garyp
    New reserved space options.
    Revision 1.2  2005/08/03 19:40:28Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/28 16:07:54Z  pauli
    Initial revision
    Revision 1.17  2005/03/29 07:09:18Z  GaryP
    Added support for the BBM format type, and spare area percentage. both
    of which can be configured through the standard options interface.
    Revision 1.16  2004/12/30 23:14:03Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.15  2004/09/21 04:42:28Z  GaryP
    Changed to use "compaction" rather than "garbage collection".
    Revision 1.14  2004/08/22 20:40:02Z  GaryP
    Added compile time configuration validation code.
    Revision 1.13  2004/08/11 19:26:53Z  GaryP
    Added logic to validate the default file system choice.
    Revision 1.12  2004/07/23 23:43:31Z  GaryP
    Added support for specifying a default file system.
    Revision 1.11  2004/07/20 01:36:24Z  GaryP
    Moved the format state definitions to fxmacros.h from fxdriver.h.
    Revision 1.10  2004/07/03 02:47:22Z  GaryP
    Moved the USE_DEFAULT definition from vbf.h to fxmacros.h.
    Revision 1.9  2003/11/13 01:03:50Z  garys
    Merge from FlashFXMT
    Revision 1.8.1.2  2003/11/13 01:03:50  garyp
    Added mutex and semaphore naming reality checks.
    Revision 1.8  2003/09/22 21:25:13Z  garys
    Removed debug asserts that FFX_RESERVED_SPACE is <= some constant, because
    of a compiler warning.
    Revision 1.7  2003/09/18 19:34:30  billr
    Change test for reserved space to eliminate a warning from some compilers
    when FFX_RESERVED_SPACE was zero.
    Revision 1.6  2003/09/18 16:25:10Z  billr
    Guard against multiple inclusion.
    Revision 1.5  2003/05/27 23:27:48Z  billr
    Make sure D_UINT16_MAX is consistently unsigned on both 16-bit and 32-bit
    architectures, and D_UINT32_MAX is consistently unsigned on both
    32-bit and 64-bit architectures.
    Revision 1.4  2003/04/16 19:52:23Z  billr
    Merge from VBF4 branch.
    Revision 1.2.1.3  2003/04/10 22:57:36Z  dennis
    Combined some portability mods to MAKE_XXX_ENDIAN with code from main
    branch to reverse and move bytes simultaneously.
    Revision 1.2.1.2  2003/04/04 23:59:50  billr
    Add the new Media Manager code.
    Revision 1.3  2003/03/16 21:01:58Z  garyp
    Added the COPYLITTLE2NATIVE() and COPYNATIVE2LITTLE() macros.
    Revision 1.2  2002/11/25 03:28:30Z  garyp
    Changed LITTLE_ENDIAN to FFX_LITTLE_ENDIAN and eliminated the use of #ifdef
    to examine it.
---------------------------------------------------------------------------*/

#ifndef FXMACROS_H_INCLUDED
#define FXMACROS_H_INCLUDED


/*-------------------------------------------------------------------
    Used by the format code to initialize data structures
-------------------------------------------------------------------*/
#define FFX_USE_DEFAULT                         (0x7FFFFFFFL)


/*-------------------------------------------------------------------
    Symbol: FFX_FORMAT_NEVER

    Never format.

    Symbol: FFX_FORMAT_ONCE

    Format only if it is determined that a format is needed.

    Symbol: FFX_FORMAT_ALWAYS

    Always reformat whether it is needed or not.
-------------------------------------------------------------------*/
#define FFX_FORMAT_NEVER                        (0)
#define FFX_FORMAT_ONCE                         (1)
#define FFX_FORMAT_ALWAYS                       (2)
#define FFX_FORMAT_UNFORMAT                     (3) /* special case for BBM */


#define FFX_NEXT                        (D_UINT32_MAX)      /* Use next sequential location */
#define FFX_REMAINING                   (D_UINT32_MAX)      /* Use all remaining space */
/* #define FFX_AVAILABLE                (D_UINT32_MAX-1)       Use all available space, up to the start of another Disk, (FUTURE..) */
#define FFX_CHIP_REMAINING              (D_UINT32_MAX-2)    /* Use all remaining area in this chip */
#define FFX_SPECIAL_CASE_MINIMUM        (FFX_REMAINING)     /* Track the lowest numbered special case value */

#define FFX_BADADDRESS                  ((void*)D_UINT32_MAX)


/*-------------------------------------------------------------------
    These settings and macros are used to allow a Device's
    "LowReservedSpace" to be specified as a chip length.

    The FFX_DEVCHIP() macro may be used to specify a zero-based
    chip number, from 0 to 127.  This allows a single compile-
    time parameter to work in a device which may get built with
    a variety of chips of different sizes.

    NOTE: The FFX_DEVCHIP() macro may <only> be used on NAND, and
          may only be used with NTMs which specifically support the
          feature.  See the NTM documentation.
-------------------------------------------------------------------*/
#define FFX_MAXDEVCHIPS                 (128)           /* Somewhat arbitrary */
#define FFX_DEVCHIPFLAG                 (0x10000000)

#define FFX_DEVCHIP(num)                (((num)+1) | FFX_DEVCHIPFLAG)
#define FFX_GETDEVCHIP(siz)             (((siz) & ~FFX_DEVCHIPFLAG) - 1)
#define FFX_ISDEVCHIP(siz)              (((siz) & FFX_DEVCHIPFLAG) && (FFX_GETDEVCHIP(siz) < FFX_MAXDEVCHIPS))


/*---------------------------------------------------------------------------
	Setting: FFX_DEV0_SETTINGS

	This defines the basic configuration information for each Device.  This
	setting has the form:

	{UncachedAddress, ReservedLoKB, ReservedHiKB, MaxDevSizeKB}

	- *UnchachedAddress* is the base address of the flash array. For flash
	   types such as NAND, or the NOR "ram" or "file" FIMS, where virtual
	   address mapping is not required, this value is not used and is
	   typically set to FFX_BADADDRESS.
	- *ReservedLoKB* specifies the amount of flash at the beginning of the
	   flash array in KB, which will not be touched by FlashFX.
	- *ReservedHiKB* specifies the amount of flash at the end of the flash
	   array in KB, which will not be touched by FlashFX.
	- *MaxDevSizeKB* is the maximum amount of flash in KB, to use for the
	   device, *AFTER removing any reserved space*. For NAND, the actual
	   usable amount of flash will be less than that specified here, due
	   to BBM overhead.

	Note that when working with NOR flash, setting MaxDevSizeKB to a size
	greater than the available physical flash will cause FlashFX to
	access the address space outside of the address space of the flash
	array, which is not allowed on many systems. Please use FFX_REMAINING
	only if you know that issueing the flash identify command to the
	address space outside of your flash array will not cause problems on
	your system.

	Notes:
	- If the total of ReservedLoKB, ReservedHiKB, and MaxDevSizeKB is less
	  than the total amount of flash, the effective ReservedHiKB value will
	  be increased.
	- If the total of ReservedLoKB, ReservedHiKB, and MaxDevSizeKB is greater
	  than the total amount of flash, the effective MaxDevSizeKB value will be
	  automatically reduced.
	- If, due to any of these factors, MaxDevSizeKB becomes zero, the Device
	  will fail to initialize.
	- The boundaries between the various reserved areas and the usable flash
	must fall on erase block boundaries.

	A new FlashFX project uses default Device settings of
	{FFX_DEFAULT_DEV_SETTINGS},	which resolves to:

	{ FFX_BADADDRESS, 0, 0, FFX_REMAINING }
---------------------------------------------------------------------------*/
#define FFX_DEFAULT_DEV_SETTINGS        FFX_BADADDRESS,0,0,FFX_REMAINING


/*---------------------------------------------------------------------------
	Setting: FFX_DEV0_FIMS

	This setting defines the list of FIMs which will be associated with the
	Device. FlashFX will attempt to initialize each FIM listed, in the
	order specified, until one is loaded successfully. The list is
	specified in curly braces, with each FIM specified as an address, and
	the entire list must be terminated with a NULL entry. See the FlashFX
	Supported Hardware document from the Resource section of the Datalight
	Website, or the source file devflash.c for a complete list of FIMs and
	their device names.
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
	Setting: FFX_DEV0_NTMS

	This setting defines the list of NTMs which will be associated with the
	Device, if the NAND FIM is used. FlashFX will attempt to initialize
	each NTM listed, in the order specified, until one is loaded
	successfully. The list is specified in curly braces, with each NTM
	specified as an address, and the entire list must be terminated with a
	NULL entry. See the FlashFX Supported Hardware document from the
	Resource section of the Datalight Website, or the source file nand.c
	for a complete list of NTMs and their device names.

	NTMs are only used if the NAND FIM is used. If NAND is not being used,
	the FFX_DEV0_NTMS setting should be set to {NULL}.
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
	Setting: FFX_DEV0_LATENCY

	This is the desired latency characteristics for the device.  This is
	only used if FFXCONF_LATENCYREDUCTIONENABLED is TRUE.  This feature is
	only accurate if the flash device timing characteristics are accurate.
	According to the flash timing, large operations will be broken up as
	necessary to reduce overall system latency.  This setting has the
	following form:

	{ReadTime, WriteTime, EraseTime, ErasePollInterval}

	*ReadTime*, *WriteTime*, and *EraseTime* are the desired maximum number
	of microseconds to spend in any given read, write, or erase operation.
	Set	these values to 0 to cause the "minimum latency" settings to be used
	where exactly one page or block will be processed at a time.  Set these
	values to -1 to get the "maximum-throughput" configuration, (but higher
	latency).  This is the default setting.

	*ErasePollInterval* is the number of microseconds to sleep between
	polling for the completion of an erase operation.  Specify 0 to disable
	sleeping while polling, or -1 to use default values based on the flash
	performance.

	By default, FlashFX uses latency settings of
	{FFX_DEFAULT_LATENCY_SETTINGS},	which resolves to:

	{ -1, -1, -1, -1 }
---------------------------------------------------------------------------*/
#define FFX_DEFAULT_LATENCY_SETTINGS    -1, -1, -1, -1


/*---------------------------------------------------------------------------
	Setting: FFX_DEV0_TIMING

	This defines the timing characteristics for the device.  This is only
	used when FFXCONF_LATENCYREDUCTIONENABLED is TRUE.  This setting has
	the following form:

	{PageReadTime, PageWriteTime, BlockEraseTime}

	These values are the number of microseconds required to read a page,
	write a page, and erase a block.  If these values are set to zero, and
	the feature FFXCONF_LATENCYAUTOTUNE is enabled, the values will be
	calculated at run-time.

	By default, FlashFX uses timing settings of {FFX_DEFAULT_TIMING_SETTINGS},
	which resolves to:

	{ 0, 0, 0 }
---------------------------------------------------------------------------*/
#define FFX_DEFAULT_TIMING_SETTINGS     0, 0, 0


/*---------------------------------------------------------------------------
    Symbol:	FFX_BBMFORMAT_PRESERVE_BADBLOCKS

    Preserve any previously found bad blocks on the Device.

	Symbol: FFX_BBMFORMAT_REUSE_FACTORYBADBLOCKS

	Reformat (re-use) any factory marked bad blocks (which implies re-using the
	"discovered" bad blocks as well). This option is strongly discouraged as it
	violates the specifications from most NAND manufacturer’s data sheets. This
	may render the flash unusable.
---------------------------------------------------------------------------*/
#define FFX_BBMFORMAT_PRESERVE_BADBLOCKS        (0)
#define FFX_BBMFORMAT_REUSE_FACTORYBADBLOCKS    (2) /* WARNING! -- SEE DOCS! */


/*---------------------------------------------------------------------------
	Setting: FFX_DEV0_BBM

	This setting defines several characteristics of the BBM format
	functionality. These settings are only used if NAND support is enabled, and
	the specific Device type is NAND. These are specified as a structure
	initialization, in curly braces. The following fields must be initialized:

	{fEnable, uFormatState, uFormatType, ulSparePercent, nReserved}

    - *fEnable* indicates whether BBM should be enabled for the Device or not.

	- *uFormatState* defines the format state to use for the device, which must
      be one of the following values:

	FFX_FORMAT_NEVER    - Never BBM format the Device. If there is not already
	                      a valid BBM format on the Device, the Device
	                      initialization will fail.
	FFX_FORMAT_ONCE     - Perform a BBM format only if BBM fails to initialize
	                      on the Device.
	FFX_FORMAT_ALWAYS   - Perform a BBM format every time the Device loads.
	FFX_FORMAT_UNFORMAT - Erase any previous BBM (and FlashFX) information
	                      on the Device.  Once the "unformat" is complete, the
	                      Device will fail to initialize.

	- *uFormatType* defines the type of BBM format to perform, which must
	  be one of the following values:

	FFX_BBMFORMAT_PRESERVE_BADBLOCKS        - Preserve any previously found bad
	                                          blocks on the Device.
	FFX_BBMFORMAT_REUSE_DISCOVEREDBADBLOCKS - Reformat (re-use) any aftermarket
	                                          discovered bad blocks, but preserve
	                                          any Factory Bad Blocks.
	FFX_BBMFORMAT_REUSE_FACTORYBADBLOCKS    - Reformat (re-use) any factory marked
	                                          bad blocks (which implies re-using
	                                          the "discovered" bad blocks as well).
	                                          This option is strongly discouraged
	                                          as it violates the specifications
	                                          from most NAND manufacturer’s data
	                                          sheets. This may render the flash
	                                          unusable.

	- *ulSparePercent* defines the percentage of the Device's manufacturer
	   recommended number of reserved blocks.  Leave this value set to 100
	   to use the default size.  Generally, the default is approximately 2.4%
	   or 24 blocks out of every 1024.

	- *nReserved* is a reserved field which must always be 0.

	By default, FlashFX uses BBM settings of {FFX_DEFAULT_BBM_SETTINGS},
	which resolves to:

	{ TRUE, FFX_FORMAT_ONCE, FFX_BBMFORMAT_PRESERVE_BADBLOCKS, 100, 0 }
---------------------------------------------------------------------------*/
/* The following initialization implicitly sets nReserved to zero. */
#define FFX_DEFAULT_BBM_SETTINGS        TRUE,FFX_FORMAT_ONCE,FFX_BBMFORMAT_PRESERVE_BADBLOCKS,100
/*---------------------------------------------------------------------------
    Symbol: FFX_DISK_RAW

    A flag indicating that a Disk may include the BBM area.

    Symbol: FFX_DISK_SPANDEVICES

    A flag indicating that the Disk may span multiple Devices with the same
    basic characteristics.
---------------------------------------------------------------------------*/
#define FFX_DISK_VALIDMASK              (0x0003)
#define FFX_DISK_RAW                    (0x0001)
#define FFX_DISK_SPANDEVICES            (0x0002)


/*---------------------------------------------------------------------------
	Setting: FFX_DISK0_SETTINGS

	This defines the basic configuration information for each Disk.  This
	setting has the form:

	{DeviceNum, OffsetKB, LengthKB, Flags}

	- *DeviceNum* is the Device number, from 0 to FFX_MAX_DEVICES-1, on which
	   the Disk is to reside.
	- *OffsetKB* is the offset within the Device, in kilobytes, where the Disk
	   is to start. This offset must be on an erase block boundary. If it is
	   not, the offset will be rounded up to the next boundary.
	- *LengthKB* is the size of the Disk, in kilobytes. This length must be
       evenly divisible by the size of an erase block if it is not, it will
       be reduced as necessary. A disk may not overlap any other disks which
       are mapped onto the disk. Set this value to FFX_REMAINING to use the
       remaining space on the device (starting at OffsetKB).
    - *Flags* may be zero, or include any combination of the following values:

       FFX_DISK_SPANDEVICES - Allows the Disk to span multiple Devices with
                              the same basic characteristics, starting with
                              the Device specified by DeviceNum.
       FFX_DISK_RAW         - The Disk may include the BBM area.

    *OffsetKB* and *LengthKB* values which are not evenly divisible by the
    Device's erase block size, will be adjusted up for offsets and down for
    the length.  A Disk may not overlap any other Disks which are mapped
    onto the Device.

	A new FlashFX project uses default Disk settings of
	{FFX_DEFAULT_DISK_SETTINGS}, which resolves to:

	{ 0, 0, FFX_REMAINING, 0 }
---------------------------------------------------------------------------*/
#define FFX_DEFAULT_DISK_SETTINGS       0, 0, FFX_REMAINING, 0


/*---------------------------------------------------------------------------
    Symbol: FFX_FILESYS_UNKNOWN

    Specifies that the file system is not known.

    Symbol: FFX_FILESYS_FAT

    Specifies the FAT file system.

    Symbol: FFX_FILESYS_RELIANCE

    Specifies the Datalight Reliance file system.

    Symbol: FFX_FILESYS_HRFS

    Specifies the Wind River HRFS file system.
---------------------------------------------------------------------------*/
#define FFX_FILESYS_UNKNOWN                     DCL_FILESYS_UNKNOWN
#define FFX_FILESYS_FAT                         DCL_FILESYS_FAT
#define FFX_FILESYS_RELIANCE                    DCL_FILESYS_RELIANCE
#define FFX_FILESYS_HRFS                        DCL_FILESYS_HRFS


/*---------------------------------------------------------------------------
	Setting: FFX_DISK0_FORMAT

	This defines the format settings for each Disk.  This setting has
	the form:

	{FormatState, FileSystem, UseMBR, Reserved}

	- *FormatState* is one of FFX_FORMAT_NEVER, FFX_FORMAT_ONCE, or
	   FFX_FORMAT_ALWAYS.
	- *FileSystem* specifies the type of format to perform, which is one
	   of the following:

  	   FFX_FILESYS_UNKNOWN - Don't write any file system
	   FFX_FILESYS_FAT - Write a FAT file system
	   FFX_FILESYS_RELIANCE - Write a Reliance file system

	- *UseMBR* is a TRUE or FALSE value which indicates whether an MBR should
	   be written to the Disk.  This value is only used if the FileSystem is
	   "FAT" or "Reliance".
    - *Reserved* is a reserved field which should always be zero.

	By default, FlashFX uses format settings of
	{FFX_DEFAULT_FORMAT_SETTINGS}, which resolves to:

	{ FFX_FORMAT_ONCE, FFX_FILESYS_UNKNOWN, FALSE, 0 }
---------------------------------------------------------------------------*/
#define FFX_DEFAULT_FORMAT_SETTINGS     FFX_FORMAT_ONCE, FFX_FILESYS_UNKNOWN, FALSE, 0

/*---------------------------------------------------------------------------
	Setting: FFX_VBFOPT flags

	This defines the VBF option flags for each Disk.  The following flag
	settings are currently supported:

	FFX_VBFOPT_NONE:       No VBF options are used for this disk.
	FFX_VBFOPT_QUICKMOUNT: Use the QuickMount feature for this disk.
---------------------------------------------------------------------------*/
#define FFX_VBFOPT_NONE (0)
#define FFX_VBFOPT_QUICKMOUNT (1)


/*---------------------------------------------------------------------------
	Setting: FFX_DISK0_ALLOCATOR

	This defines the allocator settings for each Disk.  This setting has
	the form:

	{Allocator}

	- *Allocator* is one of FFX_ALLOC_NONE or FFX_ALLOC_VBF.

	By default, FlashFX uses allocator settings of
	{FFX_DEFAULT_ALLOCATOR_SETTINGS}, which resolves to:

	{ FFX_ALLOC_VBF }
---------------------------------------------------------------------------*/
#define FFX_DEFAULT_ALLOCATOR_SETTINGS  FFX_ALLOC_VBF, FFX_VBFOPT_NONE
#define FFX_QUICKMOUNT_ALLOCATOR_SETTINGS FFX_ALLOC_VBF, FFX_VBFOPT_QUICKMOUNT


/*---------------------------------------------------------------------------
	Setting: FFX_COMPACTIONMODEL

    Defines the style of background compaction operations to support.  FlashFX
    includes three different compaction models.  The compaction model is
    typically set to a default value in the oesl.h file for the specific
    FlashFX port.  The compaction model applies to the entire FlashFX driver,
    not an individual Disk.  It may default to different values for different
    operating systems.

    A FlashFX Project may override the default compaction model by specifying
    a different compaction model in the ffxconf.h file.

	The three compaction
	models are:

    	- FFX_COMPACT_NONE
	    - FFX_COMPACT_BACKGROUNDIDLE
	    - FFX_COMPACT_BACKGROUNDTHREAD

    Symbol: FFX_COMPACT_NONE

    This option only performs compaction as needed during the course of writes,
    making it synchronous in nature.  This is the simplest compaction model to
    use, but results in the slowest overall performance.  Regardless of the
    compaction model selected, synchronous compaction operations will very
    likely always happen to one degree or another.

	Symbol: FFX_COMPACT_BACKGROUNDIDLE

	This option performs background compaction operations in system idle time.
	Typically this option is used in those operating systems where threading is
	either not supported, or is not practical.  To implement this, an idle loop
	somewhere in the system is modified to call FfxDriverCompactIfIdle().

	Symbol: FFX_COMPACT_BACKGROUNDTHREAD

	This option performs background compaction operations on a separate, low
	priority thread.  This is the default setting for those operating systems
	which support threads.
---------------------------------------------------------------------------*/
#define FFX_COMPACT_NONE                        (0)
#define FFX_COMPACT_SYNCHRONOUS                 (0) /* deprecated synonym for FFX_COMPACT_NONE */
#define FFX_COMPACT_BACKGROUNDIDLE              (1)
#define FFX_COMPACT_BACKGROUNDTHREAD            (2)


/*---------------------------------------------------------------------------
	Setting: FFX_DISK0_COMPACTION

	Define the compaction settings for the Disk.  This setting is used to
	configure compaction if either of the FFX_COMPACT_BACKGROUNDIDLE or
	FFX_COMPACT_BACKGROUNDTHREAD models are used.  FFX_COMPACT_NONE
	requires no configuration and this setting is not used.

	The configuration settings are specified as a structure initialization,
	in curly braces.  The following fields must be initialized:

	{ulReadIdleMS, ulWriteIdleMS, ulSleepMS, uThreadPriority}

	- *ulReadIdleMS* is the minimum number of milliseconds which must elapse
	   after the last read operation before the background compaction attempt
	   will begin examining the flash for erase zones to compact.
	- *ulWriteIdleMS* is the minimum number of milliseconds which must elapse
	   after the last write/discard operation before the background compaction
	   attempt will begin examining the flash for erase zones to compact.
	- *ulSleepMS* is the minimum number of milliseconds which the compaction
	   thread will sleep if it finds that there is nothing to compact.
	- *uThreadPriority* is the thread priority (DCL_THREADPRIORITY_*) at which
	   the compaction thread will run.

	Note that the *ulSleepMS* and *uThreadPriority* settings are only
	meaningful if the compaction model is set to FFX_COMPACT_BACKGROUNDTHREAD.

	By default, FlashFX uses compaction settings of
	{FFX_DEFAULT_COMPACTION_SETTINGS}, which resolves to:

	{ 1000UL, 3000UL, 1000UL, DCL_THREADPRIORITY_IDLE }
---------------------------------------------------------------------------*/
#define FFX_DEFAULT_COMPACTION_SETTINGS 1000UL, 3000UL, 1000UL, DCL_THREADPRIORITY_IDLE


/*-------------------------------------------------------------------
    NOTE: These levels are defined in this order to maintain legacy
          compatibility with TRUE (1) being aggressive, and FALSE (0)
          being normal.
-------------------------------------------------------------------*/
#define FFX_COMPACTFLAGS_RESERVEDMASK  (0xFFFFFFFC) /* Mask containing the reserved bits */
#define FFX_COMPACTFLAGS_LEVELMASK     (0x00000003) /* Mask containing the "level" bits */
#define FFX_COMPACTFLAGS_NORMAL        (0x00000000) /* Compact only "Qualified" or better units */
#define FFX_COMPACTFLAGS_EVERYTHING    (0x00000001) /* Compact aggressively until nothing can be gained */
#define FFX_COMPACTFLAGS_GOOD          (0x00000002) /* Compact only "WellQualified" or better units */
#define FFX_COMPACTFLAGS_MILD          (0x00000003) /* Compact only "SuperQualified" or better units */


/*-------------------------------------------------------------------
    Shutdown flags values.
-------------------------------------------------------------------*/
#define FFX_SHUTDOWNFLAGS_RESERVEDMASK (0xFFFFFFFD)
#define FFX_SHUTDOWNFLAGS_MODEMASK     (0x00000002)
#define FFX_SHUTDOWNFLAGS_NORMAL       (0x00000000) /* Normal shutdown    */
#define FFX_SHUTDOWNFLAGS_FAST         (0x00000001) /* Expedited shutdown */
#define FFX_SHUTDOWNFLAGS_TIMECRITICAL (0x00000002) /* Shutdown ASAP      */


/*-------------------------------------------------------------------
    Unlike FAT and Reliance support which must always be enabled
    or disabled in ffxconf.h, hrFs will only be defined in the
    VxWorks versions of those files.  Ensure it is turned off in
    all other cases where it is not defined.
-------------------------------------------------------------------*/
#ifndef FFXCONF_HRFSSUPPORT
  #define FFXCONF_HRFSSUPPORT                   FALSE
#endif


/*-------------------------------------------------------------------
    Flash Allocator Support
-------------------------------------------------------------------*/
#ifndef FFXCONF_ALLOCATORSUPPORT
#define FFXCONF_ALLOCATORSUPPORT                TRUE
#endif
#ifndef FFXCONF_VBFSUPPORT
#define FFXCONF_VBFSUPPORT                      FFXCONF_ALLOCATORSUPPORT
#endif

#if !FFXCONF_ALLOCATORSUPPORT
  #if FFXCONF_VBFSUPPORT
    #error "FFX: FFXCONF_ALLOCATORSUPPORT is FALSE, but FFXCONF_VBFSUPPORT is TRUE"
  #endif
#endif

#ifndef FFXCONF_QUICKMOUNTSUPPORT
  #define FFXCONF_QUICKMOUNTSUPPORT             FALSE
#endif
#if FFXCONF_QUICKMOUNTSUPPORT
  #if !FFXCONF_VBFSUPPORT
    #error "FFX: FFXCONF_QUICKMOUNTSUPPORT is TRUE, but FFXCONF_VBFSUPPORT is FALSE."
  #endif
#endif



/*-------------------------------------------------------------------
    Initialize default feature support settings for those items which
    were not already configured in ffxconf.h or oesl.h.
-------------------------------------------------------------------*/
#ifndef FFXCONF_DRIVERAUTOFORMAT
  #define FFXCONF_DRIVERAUTOFORMAT              FALSE
#endif
#ifndef FFXCONF_MBRFORMAT
  #define FFXCONF_MBRFORMAT                     FALSE
#endif
#ifndef FFXCONF_FATFORMATSUPPORT
  #define FFXCONF_FATFORMATSUPPORT              FALSE
#endif
#ifndef FFXCONF_BBMFORMAT
  #define FFXCONF_BBMFORMAT                     FALSE
#endif
#ifndef FFXCONF_SOFTLOCKSUPPORT
  /*  Soft lock support is NAND specific for the time being
  */
  #define FFXCONF_SOFTLOCKSUPPORT               FFXCONF_NANDSUPPORT
#endif
#ifndef FFXCONF_OTPSUPPORT
  /*  OTP support is NAND specific for the time being
  */
  #define FFXCONF_OTPSUPPORT                    FFXCONF_NANDSUPPORT
#endif


/*-------------------------------------------------------------------
    Validity checks to ensure that valid file system support is
    specified (if any).
-------------------------------------------------------------------*/
#if !FFXCONF_FATSUPPORT
  #if FFXCONF_FATFORMATSUPPORT
    #error "FFX: FFXCONF_FATSUPPORT is FALSE, but FFXCONF_FATFORMATSUPPORT is TRUE"
  #endif
  #if FFXCONF_FATMONITORSUPPORT
    #error "FFX: FFXCONF_FATSUPPORT is FALSE, but FFXCONF_FATMONITORSUPPORT is TRUE"
  #endif
#endif


/*-------------------------------------------------------------------
    Ensure that the MBR support and format settings are valid.
-------------------------------------------------------------------*/
#if !FFXCONF_MBRSUPPORT
  #if FFXCONF_MBRFORMAT
    #error "FFX: FFXCONF_MBRSUPPORT is FALSE, but FFXCONF_MBRFORMAT is TRUE"
  #endif
#endif


/*-------------------------------------------------------------------
    Validity checks to ensure that formatting features are properly
    configured.
-------------------------------------------------------------------*/
#if !FFXCONF_FORMATSUPPORT
  #if FFXCONF_MBRFORMAT
    #error "FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_MBRFORMAT is TRUE"
  #endif
  #if FFXCONF_DRIVERAUTOFORMAT
    #error "FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_DRIVERAUTOFORMAT is TRUE"
  #endif
  #if FFXCONF_FATFORMATSUPPORT
    #error "FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_FATFORMATSUPPORT is TRUE"
  #endif
  #if FFXCONF_BBMFORMAT
    #error "FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_BBMFORMAT is TRUE"
  #endif
#endif


/*-------------------------------------------------------------------
    Validity checks to ensure that the compaction model is properly
    configured.
-------------------------------------------------------------------*/
#if !FFXCONF_ALLOCATORSUPPORT
  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    #error "FFX: FFXCONF_ALLOCATORSUPPORT is FALSE, but FFX_COMPACTIONMODEL is not 'FFX_COMPACT_NONE'"
  #endif
#endif

#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
  #if DCL_OSFEATURE_THREADS == FALSE
    #error "FFX: FFX_COMPACTIONMODEL is FFX_COMPACT_BACKGROUNDTHREAD, but DCL_OSFEATURE_THREADS is FALSE"
  #endif
#else
  #if FFX_COMPACTIONMODEL != FFX_COMPACT_BACKGROUNDIDLE
    #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
      #error "FFX: FFX_COMPACTIONMODEL is not valid"
    #endif
  #endif
#endif


/*-------------------------------------------------------------------
    At least one of FFXCONF_NANDSUPPORT, FFXCONF_NORSUPPORT, or
    FFXCONF_ISWFSUPPORT must be set to TRUE.
-------------------------------------------------------------------*/
#ifndef FFXCONF_NANDSUPPORT
  #define FFXCONF_NANDSUPPORT                   TRUE
#endif
#ifndef FFXCONF_NORSUPPORT
  #define FFXCONF_NORSUPPORT                    TRUE
#endif
#ifndef FFXCONF_ISWFSUPPORT
  #define FFXCONF_ISWFSUPPORT                   FALSE
#endif
#if ((!FFXCONF_NANDSUPPORT) && (!FFXCONF_NORSUPPORT) && (!FFXCONF_ISWFSUPPORT))
  #error "FFX: FFXCONF_NANDSUPPORT, FFXCONF_NORSUPPORT, and FFXCONF_ISWFSUPPORT are all FALSE"
#endif


/*-------------------------------------------------------------------
    Validate the NAND and BBM settings.
-------------------------------------------------------------------*/
#ifndef FFXCONF_BBMSUPPORT
  #define FFXCONF_BBMSUPPORT FFXCONF_NANDSUPPORT
#endif

#if !FFXCONF_NANDSUPPORT
  #if FFXCONF_BBMSUPPORT
    #error "FFX: FFXCONF_BBMSUPPORT is TRUE but FFXCONF_NANDSUPPORT is FALSE"
  #endif
#endif

#if !FFXCONF_BBMSUPPORT
  #if FFXCONF_BBMFORMAT
    #error "FFX: FFXCONF_BBMSUPPORT is FALSE but FFXCONF_BBMFORMAT is TRUE"
  #endif
#endif


/*-------------------------------------------------------------------
    Validity checks to ensure that the statistics interface is
    properly configured.  Eventually these configuration options
    will probably be documented and moved into ffxconf.h.  For
    now we keep them here until the interfaces are stabilized.
-------------------------------------------------------------------*/
#ifndef FFXCONF_STATS_BBM
  #define FFXCONF_STATS_BBM FFXCONF_BBMSUPPORT  /* Default to TRUE for now to maintain existing behavior */
#endif

#ifndef FFXCONF_STATS_FML
  #define FFXCONF_STATS_FML                     FALSE   /* FALSE for checkin */
#endif

#ifndef FFXCONF_STATS_DRIVERIO
  #define FFXCONF_STATS_DRIVERIO                FALSE   /* FALSE for checkin */
#endif

#ifndef FFXCONF_STATS_COMPACTION
  #define FFXCONF_STATS_COMPACTION              FALSE   /* FALSE for checkin */
#endif

#ifndef FFXCONF_STATS_VBFREGION
  #define FFXCONF_STATS_VBFREGION               FALSE   /* FALSE for checkin */
#endif


/*-------------------------------------------------------------------
    Validate the latency configuration settings.
-------------------------------------------------------------------*/
#if !FFXCONF_LATENCYREDUCTIONENABLED
  #define FFXCONF_ERASESUSPENDSUPPORT           FALSE
  #define FFXCONF_LATENCYAUTOTUNE               FALSE
#endif


/*-------------------------------------------------------------------
    Deal with NAND flash migration to FlashFX 3.0.
-------------------------------------------------------------------*/
#if FFXCONF_NANDSUPPORT
  #ifndef FFXCONF_MIGRATE_LEGACY_FLASH
    #define FFXCONF_MIGRATE_LEGACY_FLASH        FALSE
  #endif

#endif


/*-------------------------------------------------------------------
    Initialize/validate shell TESTS and TOOLS configuration.

    ToDo: The default FlashFX TESTS and TOOLS values should be 
          propagated to the ffxconf.h files, rather than hidden
          in here.
-------------------------------------------------------------------*/
#if DCLCONF_COMMAND_SHELL

  /*  If the shell is enabled, but the FlashFX specific TESTS and TOOLS
      defines are not set, default to using the same settings as DCL.
  */  
  #ifndef FFXCONF_SHELL_TESTS
    #define FFXCONF_SHELL_TESTS     DCLCONF_SHELL_TESTS
  #endif
  #ifndef FFXCONF_SHELL_TOOLS
    #define FFXCONF_SHELL_TOOLS     DCLCONF_SHELL_TOOLS
  #endif
  
#else

  /*  If the shell is not enabled, the FlashFX specific TESTS and TOOLS
      defines should never be enabled.
  */      
  #if defined(FFXCONF_SHELL_TESTS) && FFXCONF_SHELL_TESTS
    #error "FFX: FFXCONF_SHELL_TESTS is defined, but DCLCONF_COMMAND_SHELL is FALSE"
  #endif
  #if defined(FFXCONF_SHELL_TOOLS) && FFXCONF_SHELL_TOOLS
    #error "FFX: FFXCONF_SHELL_TOOLS is defined, but DCLCONF_COMMAND_SHELL is FALSE"
  #endif
  
#endif





#endif /* FXMACROS_H_INCLUDED */


