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

    Public interface to VBF.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbf.h $
    Revision 1.27  2010/11/20 00:11:48Z  glenns
    Added fUsesQuickMount to VBF DiskInfo structure.
    Revision 1.26  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.25  2009/12/18 03:46:56Z  garyp
    Resolved Bug 2955: Write interruption test failure.
    Revision 1.24  2009/08/04 03:21:36Z  garyp
    Merged from the v4.0 branch.  Added the function FfxVbfRegionMetrics()
    and moved the region specific information from FfxVbfDiskMetrics() to it.
    Updated the compaction functions to take a compaction level.  Modified
    the shutdown processes to take a mode parameter.  Added support for
    compaction suspend/resume.  Updated to honor a VBFCONF_DEFAULT_CUSHION
    value which may be defined in vbfconf.h.  Minor datatype changes from
    D_UINT16 to unsigned.  Updated for new compaction functions which now
    return an FFXIOSTATUS value rather than a D_BOOL.
    Revision 1.23  2009/04/15 22:02:59Z  thomd
    Added fEvaluation flag to VBF structures
    Revision 1.22  2009/03/27 23:53:14Z  keithg
    Added volatile keyword to FfxVbfTestWriteInterruptions to match definition.
    Revision 1.21  2008/12/09 22:25:23Z  keithg
    No longer pass a serial number to VbfFormat() - it is handled internally.
    Revision 1.20  2008/06/04 19:02:00Z  thomd
    Use VBF_DEFAULT_CUSHION from vbfconf if defined
    Revision 1.19  2008/05/23 17:25:39Z  thomd
    Added fFormatNoErase
    Revision 1.18  2008/03/23 02:39:35Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.17  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.16  2007/08/31 23:30:43Z  pauli
    Updated the VBFDISKMETRICS structure to have additional and more
    descriptive fields.
    Revision 1.15  2007/08/31 21:12:06Z  pauli
    Resolved Bug 1402: Changed byte counts in VBFUNITMETRICS to page counts.
    Revision 1.14  2007/08/30 22:08:12Z  pauli
    Resolved Bugs 458: The max cushion is now in 10ths of percent.  Renamed
    MIN_CUSHION to VBF_MIN_CUSHION.  Renamed MAX_CUSHION to VBF_MAX_CUSHION.
    Added definitions for min, max and default spare.
    Revision 1.13  2007/06/07 22:40:59Z  rickc
    Removed deprecated vbfread(), vbfwrite(), vbfdiscard(), and vbfclientsize()
    Revision 1.12  2007/01/19 04:32:03Z  Garyp
    Added VBF-level support for enabling and disabling idle-time compaction.
    Revision 1.11  2006/12/16 02:02:28Z  Garyp
    Modified FfxVbfTestRegionMountPerf() to return the performance result
    rather than displaying it, which is not very useful in non-monolithic
    environments.
    Revision 1.10  2006/11/10 18:11:47Z  Garyp
    Prototypes updated.
    Revision 1.9  2006/10/18 17:57:14Z  billr
    Revert changes made in revision 1.8.
    Revision 1.7  2006/05/08 10:38:09Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.6  2006/02/21 02:53:12Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.5  2006/02/08 00:22:29Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.4  2006/01/05 03:36:28Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.3  2005/12/12 19:27:09Z  garyp
    Added compaction tracking logic.
    Revision 1.2  2005/12/09 22:01:53Z  Garyp
    Removed some dead code.
    Revision 1.1  2005/12/04 21:07:04Z  Pauli
    Initial revision
    Revision 1.4  2005/12/04 21:07:04Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.3  2005/11/13 02:33:29Z  Garyp
    Prototype updates.
    Revision 1.2  2005/11/06 09:02:06Z  Garyp
    Updated to support the new statistics interface.
    Revision 1.1  2005/10/02 03:04:22Z  Garyp
    Initial revision
    Revision 1.2  2005/09/18 07:52:40Z  garyp
    Renamed vbfcompact() to FfxVbfCompact() and added FfxVbfCompactIfIdle().
    Revision 1.1  2005/07/28 16:07:50Z  pauli
    Initial revision
    Revision 1.32  2004/12/30 17:31:15Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.31  2004/11/20 00:54:36Z  GaryP
    Added prototypes for compaction suspend and restore functions.
    Revision 1.30  2004/09/23 08:13:46Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.29  2004/09/15 23:29:04Z  garys
    fAggressive parameter to FfxVbfCompact() to support background G.C.
    Revision 1.28  2004/07/20 01:25:35  GaryP
    Moved the prototypes for the All-in-one-framework to fxaiof.h.
    Revision 1.27  2004/07/03 02:47:23Z  GaryP
    Moved the USE_DEFAULT definition from vbf.h to fxmacros.h.
    Revision 1.26  2003/12/30 23:20:10Z  garys
    Merge from FlashFXMT
    Revision 1.23.1.5  2003/12/30 23:20:10  billr
    Comment changes only.
    Revision 1.23.1.4  2003/12/29 20:38:19Z  billr
    Eliminate vbfstatus().
    Revision 1.23.1.3  2003/11/26 20:58:55Z  billr
    Changes for thread safety. Compiles (Borland/DOS), not yet tested.
    Revision 1.23.1.2  2003/11/03 04:39:42Z  garyp
    Re-checked into variant sandbox.
    Revision 1.24  2003/11/03 04:39:42Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.23  2003/09/22 21:04:44Z  garys
    Removed debug asserts that FFX_RESERVED_SPACE is <= some constant, because
    of a compiler warning.
    Revision 1.22  2003/09/19 00:04:47  billr
    Change test for reserved space to eliminate a warning from some
    compilers when FFX_RESERVED_SPACE was zero.
    Revision 1.21  2003/09/11 20:55:34Z  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.20  2003/06/13 11:34:00  garyp
    Added some settings from _VBF.H which are used by the DOS FORMAT utility.
    Revision 1.19  2003/05/27 22:49:49Z  billr
    Define manifest constant VBF_MAX_TRANSFER_SIZE: the largest
    permissible length for vbfread() or vbfwrite().
    Revision 1.18  2003/05/22 17:10:18Z  garyp
    Added prototypes for test functions.
    Revision 1.17  2003/05/13 21:26:11Z  billr
    Add the region number to the UnitInformation structure returned by
    vbfgetunitinfo(). Minor comment editing.
    Revision 1.16  2003/05/02 18:06:18Z  garyp
    Fixed the vbfformat() function prototype to use FX standard pointers.
    Re-added the VbfAllInOneFramework() prototype.
    Revision 1.15  2003/05/01 18:55:52Z  billr
    Reorganize headers: include/vbf.h is now nothing but the public interface.
    VBF internal information has moved to core/vbf/_vbf.h.  core/vbf/_vbflowl.h
    is obsolete.  A new header include/vbfconf.h contains definitions the OEM
    may want to change.  There were a few changes to types, and to names of
    manifest constants to accommodate this.
    Revision 1.14  2003/04/30 21:29:30Z  billr
    Region size is now configured dynamically at format time, so get rid of
    REGION_SIZE. Remove some obsolete stuff.
    Revision 1.13  2003/04/29 18:55:51Z  billr
    Clean up compiler warnings in WinCE build.
    Revision 1.12  2003/04/24 22:52:39Z  garys
    merged partition info changes with write interruption test changes
    Revision 1.11  2003/04/24 19:27:28  dennis
    Final version of write interruption tests.  Fail all calls when the
    region cannot be mounted in vbfxxx().
    Revision 1.10  2003/04/24 01:01:08  billr
    Fix FXINFO for VBF4. Change the UnitInformation structure and add
    vbfgetpartitioninfo() to support this.
    Revision 1.9  2003/04/17 15:47:55Z  billr
    Merge from VBF4 branch.
    Revision 1.6.1.14  2003/04/10 15:58:44Z  billr
    On the way to resolving the ReadEUH() and FormatUnit() mess.
    Revision 1.6.1.13  2003/04/10 01:38:36Z  dennis
    Removed most of the conditional code left over from the MM merge.
    Revision 1.6.1.12  2003/04/05 00:00:45  billr
    Add the new Media Manager code.
    Revision 1.6.1.11  2003/04/03 01:41:02Z  dennis
    Incorporated changes suggested byf vbfwrite walkthrough
    Revision 1.6.1.10  2003/03/27 01:56:15  dennis
    All unit default unit tests pass with metadata working on NXMS,
    comments, functions and data cleaned up.
    Revision 1.8  2003/03/21 19:10:40Z  garyp
    Added a prototype for FfxVBFAllInOneFramework().
    Revision 1.6.1.9  2003/03/20 21:42:09  dennis
    Factored the wear leveling code into its own policy function
    Revision 1.6.1.8  2003/03/19 04:28:19  dennis
    With the exception of the Structure Tests, the default TESTVBF tests pass.
    Revision 1.6.1.6  2003/03/03 20:56:17  dennis
    Refactored _vbfwrite to write blocks into a region.   WriteNewBlock
    now transparently handles retries of partial writes due to I/O errors.
    WriteConsecutiveBlocks discards data that failed to be fully written
    to the flash.  WriteConsecutiveBlocks attempts to explicitly discard
    any valid data that is being implicitly discarded (up to the MM what
    to do about that.)   All write routines now return a IOReturn_t struct.
    Revision 1.6.1.5  2003/02/26 00:46:39  dennis
    Testvbf discard tests pass with these changes
    Revision 1.6.1.4  2003/02/24 21:08:13  dennis
    These changes maked the TESTVBF small blk tests pass.
    Revision 1.6.1.3  2003/02/24 18:45:23  dennis
    Changed the in-memory Allocation struct definition and changes to the MM
    interface.
    Revision 1.6.1.2  2003/01/23 00:48:54  dennis
    Established temporal order to units within a region.  Expanded the
    representation of unit counts to 32-bits.
    Revision 1.7  2003/01/10 22:56:22Z  dennis
    No changes.
    Revision 1.6  2003/01/10 22:56:22  dennis
    Eliminated the SORT_TYPE option from VBF.   The only sort type
    now is "Pigeon Hole".
    Revision 1.5  2002/12/02 19:38:34  billr
    Fix warning about '\D' thrown by Platform Builder 4.1.
    Revision 1.4  2002/11/30 22:52:16Z  dennis
    Changed IsValidEUH() to compare the entire VBF signature in the EUH
    being read off the disk.  Moved the VBF signature definition to VBF.H
    and use it directly throughout the project.  Renamed SIGNATURE_SIZE to
    VBF_SIGNATURE_SIZE to match the name of the new VBF_SIGNATURE string
    constant.
    Revision 1.3  2002/11/13 22:56:08  garyp
    Added the vbfgetunitinfo() prototype from _VBF.H so the function is
    usable by TESTVBF.C.
    10/16/02 gp  Fixed the #ifdef ALLOC_UNIT_OFFSET_CACHE logic to use #if
    09/27/02 de  Added a new D_BOOL field, inDownwardResize, to the
                 regionCache.  This is set by ResolveDupKeys() prior
                 to calling DoDiscard and cleared on return.  It avoids
                 creating duplicate allocation entries.
    07/30/02 pkg Added ALLOC_UNIT_OFFSET_CACHE into the region cache
    07/22/02 de  Added a ulSequenceNumber field to both the EUH and the
                 RegionInfo data structures so that we can identify which
                 erase unit to discard when a GC is interrupted.
                 Recommend reformating any existing NAND disks.
    01/15/02 gjs Added the UNITHEADER_RESERVED_BYTES define and used
                 the SIGNATURE_SIZE define in the UnitHeader struct.
    10/01/01 mjm Added VBF patent info.
    08/07/01 de  Added the _PADDED_FIELD_(n,t) macro to generalize the
                 alignment of structures with member fields that arrays
                 of configurable dimenstion.
    07/31/01 de  Modified declaration of VBFpartition.rgfRegionsMounted
                 to make the structure a dword multiple in length so that
                 arrays of the structs have the fields aligned.
    06/11/01 PKG Further documented to GARBAGE_COLLECT_THRESHOLD and
                 moved it toward the start of the source file.
    02/14/01 DE  Redefined MAX_OEM_REGIONS to use long constants (vs ints)
                 to avoid overflow in max boundary case.
    01/26/01 DE  Added rgfRegionsMounted bit array to the VBFParition struct to
                 indicate when a region is being mounted for the first time.
                 We now do a pigeon hole "sort" on each region the firt time it
                 is mounted in order to recover from interrupted downward resize
                 operations.
    01/26/01 DE  Added new constants to allow for selecting the pigeon
                 hole sort.
    06/28/00 HDS Changed the polarity of allocation start block MSBit (from
                 client block MSBit) to maintain backward compatibility.
    06/26/00 HDS Corrected generation of indicator that is used to determine
                 256KB unit start block address.
    05/02/00 HDS Changed macros to use | instead of +.
    01/11/00 HDS Added constants and macros to utilize the MSB of the allocation
                 client block for expanding the erase zone size to 256 KB.
    02/16/99 HDS Changed MAX_UNIT_SIZE to a constant 128K.
    01/25/99 HDS Changed the UNIT_TO_CACHE and MEDIA_SIZE_THRESHOLD constants
                 to use 32K max unit size for the smaller erase zone devices to
                 improve performance.
    01/04/99 HDS Added GARBAGE_COLLECT_THRESHOLD constant - used to guarantee
                 minimum number of free bytes before garbage collection required.
    12/16/98 PKG Changed THIS to thisPartition to avoid compiler warns
    10/21/98 HDS Removed conditional compile with Region_t related parameters.
    10/20/98 PKG Modified definitions of MAX_MEG_LIMIT and MAX_PARTITION to
                 support large disks.
    10/13/98 HDS Corrected regionList definition.
    10/13/98 PKG Corrected MAX_UNITS definition.
    10/02/98 HDS Changed Region_t and a few constants to support large disks
                 (> 128MB).
    08/27/98 HDS Added VBF_API_BLOCK_SIZE definition to allow VBF_BLOCK_SIZE to
                 be greater than 512 bytes.
    08/20/98 HDS Made definition of VBF_SCRATCH_BLOCK_SIZE dependent on
                 VBF_BLOCK_SIZE.
    08/20/98 PKG Updated all structures to keep elements aligned
    02/26/98 PKG Added the default cushion size for the VBF format
    12/01/97 PKG Made the VBF_xxx status states bit fields (OR'able)
    11/19/97 HDS Changed include file search to use predefined path.
    11/10/97 PKG Added type casts for the region number type
    10/15/97 PKG Updated the MAX_COUNT_DIFFERENCE for new parts
    08/07/97 PKG Added test for the optional unit info API call
    08/26/97 PKG Now uses new Datalight specific types
    08/14/97 PKG Added the block size into the unit header
    08/11/97 PKG Reorganized defines for ease of modifications
    08/07/97 PKG Re-organized defines that control the region size, block size,
                 and unit sizes in order to handle future variable sized
                 allocation blocks.  Removed obsolete defines
    05/16/97 PKG includes oemconf for VBF structures size
    04/22/97 PKG No longer include "flashfx.h", added proper type
       casts for ERASED states
    03/24/97 PKG Added signature size for the unit header
    01/28/97 PKG Original revision from v3.01
---------------------------------------------------------------------------*/

#if FFXCONF_VBFSUPPORT

#ifndef VBF_H_INCLUDED
#define VBF_H_INCLUDED

#include <vbfconf.h>


/*-------------------------------------------------------------------
    Maximum unit size in units of VBF blocks
-------------------------------------------------------------------*/
#define VBF_MAX_UNIT_BLOCKS (VBF_MAX_UNIT_SIZE / VBF_MIN_BLOCK_SIZE)


/*-------------------------------------------------------------------
    CUSHION

    Cushion is a portion of the flash which is set aside for use
    by FlashFX and is not available to the client.  The flash
    used for the cushion reduces the ratio of erases to writes
    when the flash is dirty.  This results in better performance
    at the cost of less available flash.

    Cushion is specified in 10ths of a percent.  The default
    cushion value is 2.5%, if it is not specified in vbfconf.h.
    When formatting the Disk, FlashFX will adjust the cushion
    percentage as necessary to accommodate the flash geometry.
-------------------------------------------------------------------*/
#define VBF_MIN_CUSHION     (1)
#define VBF_MAX_CUSHION     (250)
#if VBFCONF_DEFAULTCUSHION < VBF_MIN_CUSHION
    #error "FFX: VBF cushion is too small"
#endif
#if VBFCONF_DEFAULTCUSHION > VBF_MAX_CUSHION
    #error "FFX: VBF cushion is too large"
#endif
#define VBF_DEFAULT_CUSHION VBFCONF_DEFAULTCUSHION


/*-------------------------------------------------------------------
    These describe the number of spare units that may be used by VBF.
    By default, a single spare unit is allocated to each partition
    but the VBF format options allow more to be specified.  Having
    extra spare units in a partition may allow for some additional
    bad block protection on NOR flash.

    NOTE:
    VBF_MAX_SPARE is a duplicate of SPARE_UNITS_MAX located in
    vbfint.h.  It is here to allow for code external to VBF to have
    access to it.  This duplication can be eliminated when VBF is
    changed to no longer use SPARE_UNITS_MAX for determining sizes
    statically at compile time.
-------------------------------------------------------------------*/
#define VBF_MAX_SPARE       (5)
#define VBF_MIN_SPARE       (1)
#define VBF_DEFAULT_SPARE   (1)


/*-------------------------------------------------------------------
    Types and structures to support FfxVbfDiskInfo().
-------------------------------------------------------------------*/
typedef struct
{
    FFXFMLHANDLE    hFML;
    D_UINT32        ulSerialNumber;    /* serial number of the disk            */
    D_UINT32        ulTotalPages;      /* formatted client disk size in pages  */
    unsigned        nDiskNum;          /* Disk number                          */
    unsigned        fReadOnly:1;       /* disk was marked read-only            */
    unsigned        fEvaluation:1;     /* evaluation limit enabled             */
  #if FFXCONF_QUICKMOUNTSUPPORT
    unsigned        fUsesQuickMount:1; /* Disk is formatted for QuickMount     */
  #endif
    D_UINT16        uPageSize;         /* VBF allocation block size            */
} VBFDISKINFO;


/*-------------------------------------------------------------------
    Types and structures to support FfxVbfDiskMetrics().
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulTotalPages;       /* total disk size in pages                           */
    D_UINT32    ulClientPages;      /* formatted client disk size in pages                */
    D_UINT32    ulRegionTotalPages; /* total region size in pages                         */
    D_UINT32    ulRegionClientPages;/* client pages in a region                           */
    D_UINT32    ulTotalUnits;       /* number of erase units in the disk                  */
    D_UINT32    ulTotalRegions;     /* number of regions in the disk                      */
    D_UINT16    uPageSize;          /* VBF allocation block size                          */
    D_UINT16    uSpareUnits;        /* number of spare erase units                        */
    D_UINT16    uUnitTotalPages;    /* total size of a unit in pages (alloc blks)         */
    D_UINT16    uUnitDataPages;     /* space available for allocation                     */
    D_UINT16    uUnitClientPages;   /* client address space per unit                      */
    unsigned    nRegionsCached;     /* total number of regions cached             */
    unsigned    nQualNotClean;
    unsigned    nQualDirty;
    unsigned    nQualVeryDirty;
    unsigned    nQualSuperDirty;
} VBFDISKMETRICS;


/*-------------------------------------------------------------------
    Types and structures to support FfxVbfRegionMetrics().
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulUsageCount;       /* Concurrent times this region is mounted */
    unsigned    nUnits;             /* How many units in this region */
    unsigned    nAllocsSpent;       /* Allocation blocks used */
    unsigned    nAllocsLeft;        /* Allocation blocks remaining */
    D_BOOL      fUnitsDirtyValid;   /* TRUE if the dirty unit data is valid */
    unsigned    nUnitsNotClean;     /* # of Units which have at least one discarded allocation */
    unsigned    nUnitsDirty;        /* # of Units which meet the threshold for background compaction */
    unsigned    nUnitsVeryDirty;    /* # of Units which are pretty darned dirty */
    unsigned    nUnitsSuperDirty;   /* # of Units which are supremely in need of compaction */
} VBFREGIONMETRICS;


/*-------------------------------------------------------------------
    Types and structures to support FfxVbfUnitMetrics().
-------------------------------------------------------------------*/
#define UNIT_BLOCK_SYSTEM        ((D_UCHAR) 0x10)
#define UNIT_BLOCK_FREE          ((D_UCHAR) 0x11)
#define UNIT_BLOCK_VALID         ((D_UCHAR) 0x12)
#define UNIT_BLOCK_DISCARDED     ((D_UCHAR) 0x13)

typedef struct
{
    D_UINT32    ulEraseCount;       /* Number of erases of this unit */
    D_UINT32    ulFreePages;        /* Free pages in this unit */
    D_UINT32    ulValidPages;       /* Used data pages in this unit */
    D_UINT32    ulDiscardedPages;   /* Recoverable pages in this unit */
    D_UINT32    ulSystemPages;      /* Number of pages used by VBF */
    D_UINT32    ulNumOfBlocks;      /* # of entries in acBlockMap */
    D_UINT32    lnu;                /* The physical unit number */
    D_UINT32    region;             /* Region the unit is assigned to */
    D_BOOL      bValidVbfUnit;      /* Is this unit valid or not */
    D_BOOL      bSpareUnit;         /* Is this a spare unit? */

    /*  Each UCHAR in this array represents a physical block in the
        unit represented by this structure.  Each will have one of
        the UNIT_BLOCK_ types to indicate if the block is free, used
        discarded, or belongs to the system.
    */
    D_UCHAR     acBlockMap[VBF_MAX_UNIT_BLOCKS];
}VBFUNITMETRICS;

#if FFXCONF_QUICKMOUNTSUPPORT

FFXSTATUS FfxVbfCalcDiskStateSize(FFXFMLHANDLE hFML, D_UINT32 ulUnitSize, D_UINT32 *pulStateInfoSize, D_UINT32 *pulUnitsSetAside);
void FfxVbfDisableStateSave(VBFHANDLE hVBF);

#endif

typedef enum
{
    VBF_ACCESS_LOWLIMIT = 0,
    VBF_ACCESS_READ,
    VBF_ACCESS_WRITE,
    VBF_ACCESS_DISCARD,
    VBF_ACCESS_COMPACT,
    VBF_ACCESS_HIGHLIMIT
} VBFACCESSTYPE;

#if 0
/* See fxtypes.h.  This should be declared here, not there. */

/*----------------------------------------------------------
 * VBFHANDLE
 *
 * For the time being, a VBFHANDLE is the same thing as
 * as a pointer to a VBFPartition structure.
 */
typedef struct sVBFDATA **VBFHANDLE;
#endif

/*  VBF API function prototypes
*/
VBFHANDLE   FfxVbfHandle(       unsigned nDiskNum);
FFXSTATUS   FfxVbfFormat(       FFXFMLHANDLE hFML, D_UINT32 ulCushion, D_UINT32 ulNumSpares, D_BOOL fFormatNoErase);
VBFHANDLE   FfxVbfCreate(       FFXFMLHANDLE hFML);
FFXSTATUS   FfxVbfDestroy(      VBFHANDLE hVBF, D_UINT32 ulFlags);
FFXSTATUS   FfxVbfReformat(     VBFHANDLE hVBF);
FFXIOSTATUS FfxVbfReadPages(    VBFHANDLE hVBF, D_UINT32 ulStartPage, D_UINT32 ulPageCount, D_BUFFER *pBuffer);
FFXIOSTATUS FfxVbfWritePages(   VBFHANDLE hVBF, D_UINT32 ulStartPage, D_UINT32 ulPageCount, const D_BUFFER *pBuffer);
FFXIOSTATUS FfxVbfDiscardPages( VBFHANDLE hVBF, D_UINT32 ulStartPage, D_UINT32 ulPageCount);
FFXSTATUS   FfxVbfDiskInfo(     VBFHANDLE hVBF, VBFDISKINFO *pDiskInfo);
FFXSTATUS   FfxVbfDiskMetrics(  VBFHANDLE hVBF, VBFDISKMETRICS *pDiskMets);
FFXSTATUS   FfxVbfRegionMetrics(VBFHANDLE hVBF, D_UINT32 ulRegionNum, VBFREGIONMETRICS *pRegionMets);
FFXSTATUS   FfxVbfUnitMetrics(  VBFHANDLE hVBF, VBFUNITMETRICS *pUnitMets);
FFXIOSTATUS FfxVbfCompact(      VBFHANDLE hVBF, D_UINT32 ulFlags);
FFXSTATUS   FfxVbfParameterGet(VBFHANDLE hVBF, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
FFXSTATUS   FfxVbfParameterSet(VBFHANDLE hVBF, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
D_INT16     FfxVbfTestWriteInterruptions(volatile VBFHANDLE hVBF, D_UINT32 ulSeed, unsigned nVerbosity);
D_UINT32    FfxVbfTestRegionMountPerf(   VBFHANDLE hVBF);

#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
  D_UINT32  FfxVbfCompactIfIdle(VBFHANDLE hVBF, D_UINT32 ulFlags);
  D_UINT32  FfxVbfCompactIdleSuspend(VBFHANDLE hVBF);
  D_UINT32  FfxVbfCompactIdleResume(VBFHANDLE hVBF);
#endif

#if FFXCONF_POWERSUSPENDRESUME
  FFXSTATUS FfxVbfPowerSuspend(VBFHANDLE hVBF, unsigned nPowerState);
  FFXSTATUS FfxVbfPowerResume( VBFHANDLE hVBF, unsigned nPowerState);
#endif

#endif /* VBF_H_INCLUDED */

#endif  /* FFXCONF_VBFSUPPORT */


