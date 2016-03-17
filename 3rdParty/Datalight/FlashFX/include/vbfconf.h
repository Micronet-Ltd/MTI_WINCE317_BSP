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

    Configuration parameters for VBF.  If these configuration options need
    to be customized, this file is typically copied into the Project
    Directory, and modified in that location.  Include path ordering will
    ensure that the proper copy of the file is used.

    Note that if a private copy of this file is used, dependencies will NOT
    be automatically detected.  Therefore, after changing a private copy of
    this file, clean the project before re-building it.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbfconf.h $
    Revision 1.12  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.11  2009/12/04 01:41:52Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.10  2009/07/19 04:36:14Z  garyp
    Merged from the v4.0 branch.  Renamed VBF_REGION_CACHE_SIZE to
    VBFCONF_REGIONCACHESIZE.  Added the VBFCONF_DEFAULTCUSHION,
    VBFCONF_DEFERREDDISCARD, and VBFCONF_COMPACTIONTHRESHOLD settings.
    Revision 1.9  2009/04/09 02:32:18Z  garyp
    Updated for AutoDoc -- no functional changes.
    Revision 1.8  2008/01/30 23:49:12Z  Garyp
    Modified VBF_MAX_BLOCK_SIZE to be the same as FFX_MAX_PAGESIZE.
    Revision 1.7  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/01/02 21:11:47Z  Garyp
    Changed the default VBF_MAX_UNIT_SIZE value from 256 to 512KB.
    Revision 1.5  2006/08/28 22:32:02Z  Garyp
    Documentation cleanup -- no functional changes.
    Revision 1.4  2006/06/21 22:40:30Z  Garyp
    Change so the region tracking settings are now specified in ffxconf.h.
    Revision 1.3  2006/01/05 03:36:32Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.2  2005/12/25 05:32:44Z  Garyp
    Added the VBF_COMPACTION_BUFFER_SIZE setting.
    Revision 1.1  2005/11/14 20:35:44Z  Pauli
    Initial revision
    Revision 1.3  2005/11/14 20:35:44Z  Garyp
    Eliminated VBF_BLOCK_OVERHEAD vestiges.
    Revision 1.2  2005/11/06 06:41:09Z  Garyp
    Updated to support the new statistics interface.
    Revision 1.1  2005/07/10 02:37:56Z  Garyp
    Initial revision
    Revision 1.8  2005/02/13 20:31:32Z  GaryP
    Eliminated VBF_REGION_CACHE_PER_DRIVE.  Added the VBFCONF_REGIONTRACKING
    and VBFCONF_STATICREGION settings.  Changed the number of region cache
    entries from 2 to 3.
    Revision 1.7  2004/12/30 17:31:15Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.6  2004/11/29 19:14:20Z  GaryP
    Documentation fixes.
    Revision 1.5  2003/12/29 17:20:51Z  garys
    Merge from FlashFXMT
    Revision 1.3.1.4  2003/12/29 17:20:51  billr
    VBF_API_BLOCK_SIZE is no longer supported.
    Revision 1.3.1.3  2003/12/17 18:48:52Z  garyp
    Changed VBF_REGION_CACHE_PER_DRIVE to TRUE and set the region cache size
    to 2.
    Revision 1.3.1.2  2003/11/18 23:19:14Z  billr
    Merged from trunk build 744.
    Revision 1.4  2003/11/17 20:43:53Z  billr
    Implement multiple region caches.
    Revision 1.3  2003/06/10 20:21:38Z  garyp
    Modified so we don't try to redefine VBF_WRITEINT_ENABLED if it is already
    defined.
    Revision 1.2  2003/05/21 01:56:48Z  garyp
    Added the VBF_WRITEINT_ENABLED setting.
    Revision 1.1  2003/05/01 22:54:14Z  billr
    Initial revision
    ---------------------
    Bill Roman 2003-04-30
---------------------------------------------------------------------------*/

#ifndef VBFCONF_H_INCLUDED
#define VBFCONF_H_INCLUDED

#include <fxdevice.h>


/*-------------------------------------------------------------------
    Setting: VBFCONF_REGIONCACHESIZE

    Specifies the VBF region cache size.  The region cache size
    represents the maximum number of regions which can be mounted
    at any given time.  This directly affects the number of
    concurrent threads which may be accessing different areas of
    the flash.

    Each region cache entry requires a substantial amount of memory,
    typically in the 8KB vicinity, depending on other configuration
    options.

    This value may be specified as a positive absolute number of
    regions, or as a negative percentage of regions to cache.

    If the value is positive, the minimum value is 1, however if a
    static region is defined with VBFCONF_STATICREGION, this value
    must be at least 2.  If the value is larger than the number of
    regions in the Disk, then it will be automatically reduced when
    VBF is initialized for the Disk (memory will not be wasted).

    If the value is negative, it must range from -1 to -100, which
    indicates the percentage of the total regions for which to create
    cache entries.  The number will be truncated when it is rounded
    during the calculation, but regardless a safe minimum number of
    region cache entries will be created.

    The default value is -25, to cache 25% of the regions.
-------------------------------------------------------------------*/
#define VBFCONF_REGIONCACHESIZE         (-25)


/*-------------------------------------------------------------------
    Setting: VBFCONF_STATICREGION

    Specifies a VBF region number to keep statically mounted all the
    time.  If there is a small number of region cache entries, for
    some file systems it is optimal to keep a specific region mounted
    at all times, because file systems often store metadata in fixed
    locations on the Disk.

    When there is a reasonably large number of region cache entries,
    this is not as valuable because the LRU algorithm is going to
    ensure that the more recently accessed regions are always in
    the cache.

    The static region feature is disabled by default.  Uncomment the
    line below to enable the feature and specify the region to cache.
-------------------------------------------------------------------*/
/* #define VBFCONF_STATICREGION            (0)  */


/*-------------------------------------------------------------------
    Setting: VBFCONF_DEFAULTCUSHION

    Specifies the default VBF cushion percentage to use.

    Cushion is a portion of the flash which is set aside for use
    by FlashFX and is not available to the client.  The flash used
    for the cushion reduces the ratio of erases to writes when the
    flash is dirty.  This results in better performance at the cost
    of less available flash.

    Cushion is specified in 10ths of a percent.  The default cushion
    value is 6.2%.  When formatting the Disk, FlashFX will adjust
    the cushion percentage as necessary to accommodate the flash
    geometry.
-------------------------------------------------------------------*/
#define VBFCONF_DEFAULTCUSHION          (62)


/*-------------------------------------------------------------------
    Setting: VBFCONF_DEFERREDDISCARD

    Specifies whether the Deferred NAND Discard feature is enabled
    or not.

    The Deferred NAND Discard feature allows discard requests to be
    put off until the last possible moment.  This feature is only
    meaningful on NAND, and is most beneficial when there is a
    reasonably high percentage of regions cached, and there is a
    significant amount of random writes to previously written areas.

    Defaults to being enabled if NAND support is enabled.
-------------------------------------------------------------------*/
#define VBFCONF_DEFERREDDISCARD         FFXCONF_NANDSUPPORT


/*-------------------------------------------------------------------
    Setting: VBFCONF_COMPACTIONTHRESHOLD

    Specifies the percentage of dirtiness that a given VBF Erase Unit
    must have before it becomes a qualified candidate for background
    compaction.

    This is the percentage of allocations which have been discarded,
    relative to the total number of allocations in the erase unit.

    Set this too high and performance may suffer, set it too low and
    the flash will wear out sooner.

    This value must either be -1, or a positive percentage value
    from 0 to 100.

    The value defaults to -1, which tells FlashFX to automatically
    choose the threshold.  Currently the default threshold for NAND
    is 65% and for NOR is 50%.
-------------------------------------------------------------------*/
#define VBFCONF_COMPACTIONTHRESHOLD     (-1)


/*-------------------------------------------------------------------
    The VBF compaction buffer is used for intermediate storage while
    performing compaction operations.  The larger the buffer, the
    more data can be read from, and written to the FMSL at one time.
    The minimum size for this value is VBF_MAX_BLOCK_SIZE.
-------------------------------------------------------------------*/
#define VBF_COMPACTION_BUFFER_SIZE      (8 * 1024UL)


/*-------------------------------------------------------------------
    How often to force a static move.  The reciprocal is the
    percentage of time a static move will occur.  A value of
    50 would create an average 2% more compactions than
    normal.  This number must be prime to break deadlocks.
-------------------------------------------------------------------*/
#define VBF_FORCE_STATIC_CLEAN          (59)


/*-------------------------------------------------------------------
    INTEL specifications say that units encountering an erase count
    of greater than 100,000 should be moved to another unit.  INTEL
    also only guarantees 100,000 as a mimimum lifetime.  Here we
    should move static data in the 10,000 level to ensure even wear.
    VBF will attempt to keep units within this range.

    NOTE: The new Intel Strata Flash (as of 10/01/97) states that it
    is only guaranteed for 10,000 erase cycles.  Thus the difference
    here has been reduced to 700 before determining that an area is
    static and must be moved.

    ToDo: This should be defined in the FIM.
-------------------------------------------------------------------*/
#define VBF_MAX_COUNT_DIFFERENCE        (700)


/*-------------------------------------------------------------------
    Number of attempts at writing client data before going to read
    only.
-------------------------------------------------------------------*/
#define VBF_RETRY                       (5)


/*-------------------------------------------------------------------
    Unwritten allocations are read as this value.
-------------------------------------------------------------------*/
#define VBF_FILL_VALUE                  (0xFE)


/*-------------------------------------------------------------------
    This is the maximum number of erase units within a VBF Disk.  It
    governs the size of the regionList[] in the VBFDATA structure.
-------------------------------------------------------------------*/
#define VBF_PARTITION_MAX_UNITS         (1024L)


/*-------------------------------------------------------------------
    These values define the range of VBF allocation block sizes which
    will be supported.  The actual allocation size is determined at
    run-time based on the geometry of the flash.  When a Disk is 
    (re)formatted, the allocation block size is determined either
    by the page size of the media (if it has a native page size), 
    by a value supplied by a configuration hook, or by a reasonable
    default.
-------------------------------------------------------------------*/
#define VBF_MIN_BLOCK_SIZE              (512)
#define VBF_MAX_BLOCK_SIZE              (FFX_MAX_PAGESIZE)


/*-------------------------------------------------------------------
    Setting: VBF_MAX_UNIT_SIZE

    The maximum size of an erase unit.  This must be a power of two
    equal to at least 32 VBF allocation blocks and not more than 2048
    allocation blocks.  The default is 512K, or 1024 allocations of
    512 bytes each.

    This directly affects run-time memory needed by FlashFX.
    Decreasing this value to the size of the flash hardware erase
    zone can be useful in memory-constrained applications.
-------------------------------------------------------------------*/
#define VBF_MAX_UNIT_SIZE               (512 * 1024L)


/*-------------------------------------------------------------------
    This value determines whether VBF is built with write
    interruption testing enabled.  This value is TRUE by default
    in DEBUG builds, and FALSE by default in RELEASE builds.
    However, if the value is already defined, leave it as is.
-------------------------------------------------------------------*/
#ifndef VBF_WRITEINT_ENABLED
#if D_DEBUG
#define VBF_WRITEINT_ENABLED            TRUE
#else
#define VBF_WRITEINT_ENABLED            FALSE
#endif
#endif




#endif /* VBFCONF_H_INCLUDED */
