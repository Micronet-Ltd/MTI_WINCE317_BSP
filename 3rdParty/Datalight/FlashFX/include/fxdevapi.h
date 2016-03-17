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

    This header contains structures, symbols, and prototypes necessary to
    use the DevMgr API functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxdevapi.h $
    Revision 1.14  2010/12/14 20:02:23Z  glenns
    Add element to FFXFIMDEVINFO structure so the number of
    BBM-managed chips is available. Needed for FML disk spanning
    of devices, as spanning doesn't currently work with multi-chip
    devices. See bug 3280.
    Revision 1.13  2010/06/19 03:58:45Z  garyp
    Updated to track the bit correction capabilities on a per segment basis.
    Revision 1.12  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.11  2009/11/25 23:29:45Z  garyp
    Added the FFXLOCKFLAGS_MASKRESERVED symbol.  Docs updated.
    Revision 1.10  2009/07/29 18:26:12Z  garyp
    Merged from the v4.0 branch.  Added support for locking and OTP.
    Revision 1.9  2009/03/10 00:47:46Z  glenns
    Fixed Bug 2395: Added elements to make device EDC Requirement and 
    Capability, BBM Reserved Block rating and Erase Cycle Rating visible
    in the FFXFIMDEVINFO structure.
    Revision 1.8  2008/03/27 02:08:18Z  Garyp
    Eliminated the uTagSize field.
    Revision 1.7  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/09/28 00:44:11Z  pauli
    Resolved Bug 355: Added an alignment field to the FFXFIMDEVINFO structure.
    Revision 1.5  2006/11/08 03:38:51Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.4  2006/05/18 23:47:29Z  Garyp
    Added support for NOR boot blocks, as well as reserved space and the
    number of blocks per chip.
    Revision 1.3  2006/03/04 21:36:55Z  Garyp
    Added prototypes.
    Revision 1.2  2006/02/15 19:23:26Z  Garyp
    Added support for a raw block count.
    Revision 1.1  2006/02/06 18:48:50Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXDEVAPI_H_INCLUDED
#define FXDEVAPI_H_INCLUDED

#include <fxiosys.h>
#include <fxdevice.h>

typedef struct
{
    D_UINT16    uDeviceType;        /* Device type code prefixed with DEVTYPE_*     */
    D_UINT16    uDeviceFlags;       /* Device flags DEV_*                           */
    D_UINT16    uLockFlags;         /* Locking capabilities: FFXLOCKFLAGS_*         */
    unsigned    nDeviceNum;         /* DEVn                                         */
    D_UINT32    ulReservedBlocks;   /* Blocks reserved (inaccessible)               */
    D_UINT32    ulRawBlocks;        /* Blocks in the entire array (including BBM)   */
    D_UINT32    ulTotalBlocks;      /* Blocks in the entire array (excluding BBM)   */
    D_UINT32    ulChipBlocks;       /* Blocks per chip                              */
    D_UINT32    ulBlockSize;        /* Physical erase block size                    */
    D_UINT16    uPageSize;          /* Device page size                             */
    D_UINT16    uSpareSize;         /* Spare area per page (NAND only)              */
    D_UINT16    uMetaSize;          /* Meta bytes per spare area (NAND only)        */
    D_UINT16    uAlignSize;         /* Alignment required by this device, in bytes  */
  #if FFXCONF_OTPSUPPORT
    unsigned    nOTPPages;          /* # of OTP pages, or zero if unsupported       */
  #endif
    D_UINT16    uEdcSegmentSize;    /* Segment size in bytes to which uEdcCapability applies */
    D_UINT16    uEdcCapability;     /* Max error correcting capability in bits per segment bytes */
    D_UINT16    uEdcRequirement;    /* Required level of error correction */
    D_UINT32    ulEraseCycleRating;  /* Factory spec erase cycle rating */
    D_UINT32    ulBBMReservedRating; /* Reserved blocks required for above */
  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    D_UINT32    ulBootBlockSize;    /* Size of each boot block                      */
    D_UINT16    uLowBootBlockCount; /* Full sized blocks at the low end (per chip)  */
    D_UINT16    uHighBootBlockCount;/* Start boot block the high end (per chip)     */
  #endif
  #if FFXCONF_BBMSUPPORT
    D_UINT32    ulBbmChips;         /* Number of BBM-managed chips in device */
  #endif
} FFXFIMDEVINFO;

FFXFIMDEVHANDLE FfxDevHandle(      D_UINT16 uDeviceNum);
FFXFIMDEVHANDLE FfxDevCreate(      FFXDEVHANDLE hDev, D_UINT16 uDeviceNum);
FFXSTATUS       FfxDevDestroy(     FFXFIMDEVHANDLE hFimDev);
D_BOOL          FfxDevInfo(        FFXFIMDEVHANDLE hFimDev, FFXFIMDEVINFO *pDevInfo);
FFXSTATUS       FfxDevParameterGet(FFXFIMDEVHANDLE hFimDev, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
FFXSTATUS       FfxDevParameterSet(FFXFIMDEVHANDLE hFimDev, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
FFXIOSTATUS     FfxDevIORequest(   FFXFIMDEVHANDLE hFimDev, FFXIOREQUEST *pIOR);

/*-------------------------------------------------------------------
    If the physical device supports locking, or if the soft-locking
    feature is enabled, the flag DEV_LOCKABLE must be set in the 
    uDeviceFlags fields, and the uLockFlags field must be initialized
    to properly define the locking capabilities.

    Use Caution!  Several methods of locking apply to all blocks in
                  a flash chip, implying that a lock command for a
                  given range of blocks in a Disk, will affect other
                  Disks which share that Device.  This also implies
                  that blocks in the reserved areas of flash, which
                  FlashFX is not managing, will also have their lock
                  status changed.

                  Secondly, a Device may span chips, however for
                  those lock commands which are chip specific, you
                  may NOT end up locking/unlocking all blocks when
                  you think you are.

                  Finally, BBM must be considered.  For those locking
                  schemes which only allow a single (or maybe two, if
                  inverted) range of blocks to be unlocked, it is
                  impossible to unlock a range of blocks where some
                  of them may be remapped by BBM, unless you are
                  unlocking the whole area that BBM occupies.

    FFXLOCKFLAGS_LOCKALLBLOCKS

    This flag indicates that a lock command locks all blocks in the
    chip.  If this flag is not set, then the implication is that
    individual blocks or ranges may be locked, without affecting
    the lock state of other blocks.

    FFXLOCKFLAGS_LOCKFREEZE

    This flag indicates that the "lock-freeze" capability is
    supported, which causes the locked/unlocked state for the
    blocks to be frozen.  If this flag is used in combination
    with FFXLOCKFLAGS_LOCKALLBLOCKS, then the Lock-Freeze command
    will apply to all blocks in the flash.  This "Freeze" can
    generally only be thawed by powering off.

    FFXLOCKFLAGS_UNLOCKSINGLERANGE

    This flag indicates that only a single range of flash may be
    unlocked at any given time -- blocks which were previously
    unlocked will become locked.  This flag should not be used in
    combination with FFXLOCKFLAGS_UNLOCKINVERTEDRANGE.

    FFXLOCKFLAGS_UNLOCKINVERTEDRANGE

    This flag indicates that an inverted range of flash may be
    unlocked (the ability to unlock everything except for a certain
    range of flash).  However, blocks which were previously unlocked
    will become locked.  This flag should not be used in combination
    with FFXLOCKFLAGS_UNLOCKSINGLERANGE, as that capability is implied.

    NOTE:  The presence of the FFXLOCKFLAGS_UNLOCKSINGLERANGE or
           FFXLOCKFLAGS_UNLOCKINVERTEDRANGE bits is an indicator
           that an unlocking operation is a chip-wide feature.
           If neither of those flags are set, the implication is
           that multiple individual blocks may be unlocked without
           affecting the lock state of other blocks.
-------------------------------------------------------------------*/
#define FFXLOCKFLAGS_MASKRESERVED           (~(FFXLOCKFLAGS_LOCKALLBLOCKS |     \
                                               FFXLOCKFLAGS_LOCKFREEZE |        \
                                               FFXLOCKFLAGS_UNLOCKSINGLERANGE | \
                                               FFXLOCKFLAGS_UNLOCKINVERTEDRANGE))
#define FFXLOCKFLAGS_LOCKALLBLOCKS          0x0001U
#define FFXLOCKFLAGS_LOCKFREEZE             0x0002U
#define FFXLOCKFLAGS_UNLOCKSINGLERANGE      0x0004U
#define FFXLOCKFLAGS_UNLOCKINVERTEDRANGE    0x0008U


#endif /* FXDEVAPI_H_INCLUDED */


