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
    use the FML API functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxfmlapi.h $
    Revision 1.35  2010/07/07 18:21:08Z  garyp
    Added FMLEXPEND_PAGE().
    Revision 1.34  2010/06/19 03:59:53Z  garyp
    Updated to track the bit correction capabilities on a per segment basis.
    Revision 1.33  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.32  2010/01/10 20:56:29Z  garyp
    Comment updates -- no functional changes.
    Revision 1.31  2009/07/29 18:35:17Z  garyp
    Merged from the v4.0 branch.  Major update to support the concept of a 
    Disk which spans multiple Devices.  Added OTP support.  Updated so the
    Read/WriteUncorrectedPages functionality is visible even if NAND support
    is disabled, as this simplifies some higher level code.  A run-time
    determination is made to determine which low level interfaces to use.
    Added FMLREAD/WRITE_UNCORRECTEDPAGES() and deprecated FMLREAD_CORRECTED()
    pages.  Added support for block locking and unlocking.  Added support for
    power suspend and resume.
    Revision 1.30  2009/04/19 20:16:16Z  garyp
    Marked several macros as "Protected".
    Revision 1.29  2009/04/09 06:45:15Z  keithg
    Added descriptions of BBM related functionality.
    Revision 1.28  2009/04/09 02:58:32Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.27  2009/04/09 01:21:09Z  garyp
    Added AutoDoc compatible documentation.  Renamed macro parameters
    where needed so things document properly.  No other funtional changes.
    Revision 1.26  2009/03/10 00:14:22Z  glenns
    Fix Bug 2395: Added code to FFXFMLDEVICEINFO structure to make device EDC
    Capability and Requirement, BBM Reserved Block Rating and Erase Cycle 
    Rating visible.
    Revision 1.25  2009/01/18 08:52:45Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.24  2009/01/12 22:08:31Z  keithg
    Added placeholders for function headers.
    Revision 1.23  2008/12/24 05:51:26Z  keithg
    Added support new IOCTL support for raw erase operations.
    Revision 1.22  2008/12/18 17:54:03Z  keithg
    Replace the WriteNativePages functionality which as incorrectly
    removed in the previous check in.
    Revision 1.21  2008/12/18 00:10:14Z  keithg
    Corrected copmile time conditions and removed unused macros.
    Revision 1.20  2008/12/17 06:41:56Z  keithg
    Added info element to xxx_INFO macros to pass back useful information.
    made xxx_STATUS functions unconditionally included.
    Revision 1.19  2008/10/21 00:25:20Z  keithg
    Added block info macros and conditioned obsolescent code.
    Revision 1.18  2008/05/24 05:49:44Z  keithg
    Raw block status functionality is no longer dependant upon the
    inclusion of BBM functionality.
    Revision 1.17  2008/03/26 01:49:25Z  Garyp
    Updated to support variable width tags.  Expanded the various Get/Set
    Page and Block Status functions.
    Revision 1.16  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/09/28 00:44:11Z  pauli
    Resolved Bug 355: Added an alignment field to the FFXFMLDEVICEINFO
    structure.
    Revision 1.14  2007/09/12 20:17:24Z  Garyp
    Added FMLWRITE_SPARES().
    Revision 1.13  2007/06/29 21:11:08Z  rickc
    Added FML_GET_PHYSICAL_BLOCK_STATUS macro.
    Revision 1.12  2007/02/20 17:53:40Z  Garyp
    Moved the FMLREAD_SPARES() macro into the NAND specific section.
    Revision 1.11  2006/11/08 03:38:51Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.10  2006/10/10 01:48:51Z  Garyp
    Prototype updates.
    Revision 1.9  2006/10/06 01:04:43Z  Garyp
    Added the FMLREAD/WRITE_CONTROLDATA() macros.
    Revision 1.8  2006/06/06 01:40:14Z  DeanG
    Update for large flash array
    Revision 1.7  2006/05/19 00:04:28Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.6  2006/03/05 18:05:55Z  Garyp
    Added FfxFmlDeviceInfo() support,  Added Read/WriteRawPage support.
    Revision 1.5  2006/02/24 04:36:18Z  Garyp
    Updated to use refactored headers.
    Revision 1.4  2006/02/15 23:01:42Z  Garyp
    Re-added a commented out structure member.
    Revision 1.3  2006/02/15 20:45:54Z  Garyp
    Added several new FML macros.
    Revision 1.2  2006/02/15 09:23:01Z  Garyp
    Added FMLWRITE_TAGS().
    Revision 1.1  2006/02/10 10:35:48Z  Garyp
    Initial revision
    Revision 1.2  2006/01/11 19:36:14Z  billr
    Merge Sibley support from v2.01.
    Revision 1.1  2005/11/25 20:56:48Z  Pauli
    Initial revision
    Revision 1.4  2005/11/25 20:56:47Z  Garyp
    Updated to include fxiosys.h.
    Revision 1.3  2005/10/27 03:49:16Z  Garyp
    Added the uPageSize and uSpareSize to the MediaInfo structure.  Added
    the FFXNANDSPARE structure and related settings.
    Revision 1.2  2005/10/10 17:56:17Z  Garyp
    Removed singlethreaded FIM support.
    Revision 1.1  2005/10/02 03:03:32Z  Garyp
    Initial revision
    Revision 1.2  2005/08/05 17:40:56Z  Garyp
    Updated to use revamped reserved space options which now allow reserved
    space at the top of the array.
    Revision 1.1  2005/07/28 19:10:32Z  pauli
    Initial revision
    Revision 1.5  2004/12/30 23:08:29Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2004/07/23 00:13:36Z  GaryP
    Removed support for obsolete OEM status flags.
    Revision 1.3  2004/07/20 01:25:35Z  GaryP
    Moved the prototypes for the All-in-one-framework to fxaiof.h.
    Revision 1.2  2004/02/22 21:33:48Z  garys
    Revision 1.1.1.5  2004/02/22 21:33:48  garyp
    Eliminated the oemchanged() and oempowerdown() functionality.
    Revision 1.1.1.4  2004/01/25 06:46:18Z  garyp
    Eliminated the oeminterface() prototype.
    Revision 1.1.1.3  2003/12/11 22:12:56Z  garyp
    Eliminated oemgethandle() prototype.
    Revision 1.1.1.2  2003/12/04 23:13:38Z  garyp
    Added pDevExtra to oemmount().
    Revision 1.1  2003/11/03 04:28:08Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXFMLAPI_H_INCLUDED
#define FXFMLAPI_H_INCLUDED

#include <fxdevice.h>
#include <fxiosys.h>


/*-------------------------------------------------------------------
    Type: FFXFMLDEVINFO

    Contains information about a Device, and is filled in by
    FfxFmlDeviceInfo().
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT16        uDeviceType;        /* Device type code prefixed with DEVTYPE_  */
    D_UINT16        uDeviceFlags;       /* Device flags DEV_*                       */
    unsigned        nDeviceNum;         /* DEVn                                     */
    unsigned        nDisksMapped;       /* # of active DISKn mappings to this DEVn  */
    unsigned        nLockFlags;         /* Locking capabilities: FFXLOCKFLAGS_*     */
    unsigned        nOTPPages;          /* The number of OTP pages, if any          */
    D_UINT32        ulReservedBlocks;   /* Low reserved blocks (inaccessible)       */
    D_UINT32        ulRawBlocks;        /* Raw blocks in the device                 */
    D_UINT32        ulTotalBlocks;      /* Total usable blocks in the device        */
    D_UINT32        ulChipBlocks;       /* Blocks per physical chip                 */
    D_UINT32        ulBlockSize;        /* Physical erase block size                */
    D_UINT16        uPageSize;          /* Device page size                         */
    D_UINT16        uSpareSize;         /* Spare area per page (NAND only)          */
    D_UINT16        uMetaSize;          /* Meta bytes per spare area (NAND only)    */
    D_UINT16        uAlignSize;         /* Alignment required by this device, in bytes */
    D_UINT32        ulBootBlockSize;    /* Size of each boot block (NOR only)       */
    D_UINT16        uLowBootBlockCount; /* Boot blocks at the bottom of each chip   */
    D_UINT16        uHighBootBlockCount;/* Boot blocks at the top of each chip      */
    D_UINT16        uEdcSegmentSize;    /* Segment size in bytes to which uEdcCapability applies */
    D_UINT16        uEdcCapability;     /* Max error correcting capability in bits per segment bytes */
    D_UINT16        uEdcRequirement;    /* Required level of error correction       */
    D_UINT32        ulEraseCycleRating; /* Factory spec erase cycle rating          */
    D_UINT32        ulBBMReservedRating;/* Reserved blocks required for above       */
    FFXFIMDEVHANDLE hFimDev;
} FFXFMLDEVINFO;


/*-------------------------------------------------------------------
    Type: FFXFMLINFO

    Contains information about a Disk, as returned by FfxFmlDiskInfo().
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT16        uDeviceType;    /* Device type code prefixed with DEVTYPE_  */
    D_UINT16        uDiskFlags;     /* Disk flags DISK_*                        */
    unsigned        nDeviceCount;   /* Disk could span multiple Devices         */
    unsigned        nDeviceNum;     /* DEVn on which DISKn starts               */
    unsigned        nDiskNum;       /* DISKn                                    */
    D_UINT32        ulStartBlock;   /* Starting block in the Device             */
    D_UINT32        ulTotalBlocks;  /* Total blocks in the window (DISKn)       */
    D_UINT32        ulBlockSize;    /* Physical erase block size                */
    D_UINT16        uPageSize;      /* Device page size                         */
    D_UINT16        uSpareSize;     /* Spare area per page (NAND only)          */
    FFXDISKHANDLE   hDisk;
} FFXFMLINFO;

/*  Bitmapped values for the uDiskFlags field
*/
#define DISK_RAWACCESSONLY      (0x0001)
#define DISK_HASBOOTBLOCKS      (0x0002)
#define DISK_NAND               (0x0004)


/*-------------------------------------------------------------------
    FML API Function Prototypes
-------------------------------------------------------------------*/
FFXSTATUS    FfxFmlDeviceInfo(          unsigned nDevNum, FFXFMLDEVINFO *pFDI);
FFXSTATUS    FfxFmlDeviceRangeEnumerate(unsigned nDevNum, D_UINT32 ulBlockNum, FFXFMLHANDLE *phFML, D_UINT32 *pulBlockCount);
FFXFMLHANDLE FfxFmlHandle(              unsigned nDiskNum);
FFXFMLHANDLE FfxFmlCreate(              FFXDISKHANDLE hDisk, FFXFIMDEVHANDLE hFimDev, unsigned nDiskNum, D_UINT32 ulStartBlock, D_UINT32 ulBlockCount, unsigned nFlags);
FFXSTATUS    FfxFmlDestroy(     FFXFMLHANDLE hFML);
FFXSTATUS    FfxFmlDiskInfo(    FFXFMLHANDLE hFML, FFXFMLINFO *pFI);
FFXSTATUS    FfxFmlParameterGet(FFXFMLHANDLE hFML, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
FFXSTATUS    FfxFmlParameterSet(FFXFMLHANDLE hFML, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
FFXIOSTATUS  FfxFmlIORequest(   FFXFMLHANDLE hFML, FFXIOREQUEST *pIOR);
FFXSTATUS    FfxFmlFormat(      FFXFMLHANDLE hFML);

#if FFXCONF_POWERSUSPENDRESUME
FFXSTATUS    FfxFmlPowerSuspend(        FFXFMLHANDLE hFML, unsigned nPowerState);
FFXSTATUS    FfxFmlPowerResume(         FFXFMLHANDLE hFML, unsigned nPowerState);
#endif

/*-------------------------------------------------------------------
    Macros so we don't have to muck with building IO Request
    structures manually.
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: FMLERASE_BLOCKS()

    Erase one or more blocks in an FML Disk.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting block number.
        ulCount     - The count of blocks to process.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLERASE_BLOCKS(hFML, ulStart, ulCount, ioStat)             \
{                                                                   \
    FFXIOR_FML_ERASE_BLOCKS         FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_ERASE_BLOCKS;                \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartBlock = (ulStart);                                \
    FmlReq.ulBlockCount = (ulCount);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLLOCK_FREEZE()

    Lock the flash such that it cannot be unlocked short of
    restarting.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting block number.
        ulCount     - The count of blocks to process.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLLOCK_FREEZE(hFML, ulStart, ulCount, ioStat)              \
{                                                                   \
    FFXIOR_FML_LOCK_FREEZE          FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_LOCK_FREEZE;                 \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartBlock = (ulStart);                                \
    FmlReq.ulBlockCount = (ulCount);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLLOCK_BLOCKS()

    Lock one or more blocks in an FML Disk.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting block number.
        ulCount     - The count of blocks to process.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLLOCK_BLOCKS(hFML, ulStart, ulCount, ioStat)              \
{                                                                   \
    FFXIOR_FML_LOCK_BLOCKS          FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_LOCK_BLOCKS;                 \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartBlock = (ulStart);                                \
    FmlReq.ulBlockCount = (ulCount);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLUNLOCK_BLOCKS()

    Unlock one or more blocks in an FML Disk.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting block number.
        ulCount     - The count of blocks to process.
        fInverted   - A flag meaning that the range should be inverted.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLUNLOCK_BLOCKS(hFML, ulStart, ulCount, fInverted, ioStat) \
{                                                                   \
    FFXIOR_FML_UNLOCK_BLOCKS        FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_UNLOCK_BLOCKS;               \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartBlock = (ulStart);                                \
    FmlReq.ulBlockCount = (ulCount);                                \
    FmlReq.fInvert      = (fInverted);                              \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_PAGES()

    Read one or more pages from an FML Disk.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_PAGES(hFML, ulStart, ulCount, pBuffer, ioStat)      \
{                                                                   \
    FFXIOR_FML_READ_PAGES           FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_PAGES;                  \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_PAGES()

    Write one or more pages to an FML Disk.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_PAGES(hFML, ulStart, ulCount, pBuffer, ioStat)     \
{                                                                   \
    FFXIOR_FML_WRITE_PAGES          FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_PAGES;                 \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

#if !FFXCONF_NANDSUPPORT
/*  If NAND support <is not> included, just map the "uncorrected"
    macros directly onto the general page read/write macros,
    ignoring the "spare" parameter.

    If NAND support <is> included, the macros in the following
    NAND specific section will be used.  If execution is on a
    NOR Device, the FML code at run-time will redirect the
    functions to the standard page read/write interface.
*/
#define FMLREAD_UNCORRECTEDPAGES(handle, start, count, buff, spare, stat)   \
    FMLREAD_PAGES(handle, start, count, buff, stat)

#define FMLWRITE_UNCORRECTEDPAGES(handle, start, count, buff, spare, stat)  \
    FMLWRITE_PAGES(handle, start, count, buff, stat)
#endif


#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT

                /*----------------------------------*\
                 *                                  *
                 *    NOR/Sibley Specific Macros    *
                 *                                  *
                \*----------------------------------*/

/*-------------------------------------------------------------------
    Public: FMLREAD_CONTROLDATA()

    Read control bytes from an FML Disk.

    This macro is to be used on NOR or ISWF style flash only.

    Parameters:
        hFML         - The FML handle.
        ulByteOffset - The byte offset.
        ulCount      - The count of bytes to process.
        pBuffer      - A pointer to the data buffer to use.
        ioStat       - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_CONTROLDATA(hFML, ulByteOffset, ulCount, pBuffer, ioStat)  \
{                                                                   \
    FFXIOR_FML_READ_CONTROLDATA     FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_CONTROLDATA;            \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulOffset     = (ulByteOffset);                           \
    FmlReq.ulLength     = (ulCount);                                \
    FmlReq.pData        = (pBuffer);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_CONTROLDATA()

    Write control bytes to an FML Disk.

    This macro is to be used on NOR or ISWF style flash only.

    Parameters:
        hFML         - The FML handle.
        ulByteOffset - The byte offset.
        ulCount      - The count of bytes to process.
        pBuffer      - A pointer to the data buffer to use.
        ioStat       - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_CONTROLDATA(hFML, ulByteOffset, ulCount, pBuffer, ioStat)  \
{                                                                   \
    FFXIOR_FML_WRITE_CONTROLDATA     FmlReq;                        \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_CONTROLDATA;           \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulOffset     = (ulByteOffset);                           \
    FmlReq.ulLength     = (ulCount);                                \
    FmlReq.pData        = (pBuffer);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLERASE_BOOT_BLOCKS()

    Erase one or more boot blocks in an FML Disk.

    This macro is to be used on NOR or ISWF style flash only.

    Parameters:
        hFML    - The FML handle.
        ulStart - The starting block number.
        ulCount - The count of blocks to process.
        ioStat  - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLERASE_BOOT_BLOCKS(hFML, ulStart, ulCount, ioStat)        \
{                                                                   \
    FFXIOR_FML_ERASE_BOOT_BLOCKS    FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_ERASE_BOOT_BLOCKS;           \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartBB    = (ulStart);                                \
    FmlReq.ulBBCount    = (ulCount);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

#endif


#if FFXCONF_NANDSUPPORT

                /*----------------------------------*\
                 *                                  *
                 *       NAND Specific Macros       *
                 *                                  *
                \*----------------------------------*/

/*  Deprecated -- use FMLREAD_PAGES() instead.
*/
#define FMLREAD_CORRECTEDPAGES FMLREAD_PAGES

/*-------------------------------------------------------------------
    Public: FMLEXPEND_PAGE()

    Expend a NAND page.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulPage      - The page number.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLEXPEND_PAGE(hFML, ulPage, ioStat)                        \
{                                                                   \
    FFXIOR_FML_WRITE_TAGS       FmlReq;                             \
    D_BUFFER                    abTag[FFX_NAND_TAGSIZE] = {0};      \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_TAGS;                  \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulPage);                                 \
    FmlReq.ulTagCount   = 1;                                        \
    FmlReq.pTag         = abTag;                                    \
    FmlReq.nTagSize     = FFX_NAND_TAGSIZE;                         \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_TAGS()

    Read tag data from one or more pages of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of tags to process.
        pTagBuff    - A pointer to the tag buffer to use.
        nTagLen     - The tag length
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_TAGS(hFML, ulStart, ulCount, pTagBuff, nTagLen, ioStat)  \
{                                                                   \
    FFXIOR_FML_READ_TAGS            FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_TAGS;                   \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulTagCount   = (ulCount);                                \
    FmlReq.pTag         = (pTagBuff);                               \
    FmlReq.nTagSize     = (nTagLen);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_TAGS()

    Write tag data to one or more pages of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of tags to process.
        pTagBuff    - A pointer to the tag buffer to use.
        nTagLen     - The tag length
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_TAGS(hFML, ulStart, ulCount, pTagBuff, nTagLen, ioStat)  \
{                                                                   \
    FFXIOR_FML_WRITE_TAGS           FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_TAGS;                  \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulTagCount   = (ulCount);                                \
    FmlReq.pTag         = (pTagBuff);                               \
    FmlReq.nTagSize     = (nTagLen);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_TAGGEDPAGES()

    Read main page and tag data from one or more pages of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        pTagBuff    - A pointer to the tag buffer to use.
        nTagLen     - The tag length
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_TAGGEDPAGES(hFML, ulStart, ulCount, pBuffer, pTagBuff, nTagLen, ioStat)  \
{                                                                   \
    FFXIOR_FML_READ_TAGGEDPAGES     FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_TAGGEDPAGES;            \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    FmlReq.pTag         = (pTagBuff);                               \
    FmlReq.nTagSize     = (nTagLen);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_TAGGEDPAGES()

    Write main page and tag data to one or more pages of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        pTagBuff    - A pointer to the tag buffer to use.
        nTagLen     - The tag length
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_TAGGEDPAGES(hFML, ulStart, ulCount, pBuffer, pTagBuff, nTagLen, ioStat) \
{                                                                   \
    FFXIOR_FML_WRITE_TAGGEDPAGES    FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_TAGGEDPAGES;           \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    FmlReq.pTag         = (pTagBuff);                               \
    FmlReq.nTagSize     = (nTagLen);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_UNCORRECTEDPAGES()

    Read uncorrected main page and spare data from one or more pages
    of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        pSpareBuff  - A pointer to the spare buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_UNCORRECTEDPAGES(hFML, ulStart, ulCount, pBuffer, pSpareBuff, ioStat)  \
{                                                                   \
    FFXIOR_FML_READ_UNCORRECTEDPAGES  FmlReq;                       \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_UNCORRECTEDPAGES;       \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    FmlReq.pSpare       = (pSpareBuff);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_UNCORRECTEDPAGES()

    Write uncorrected main page and spare data from one or more pages
    of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        pSpareBuff  - A pointer to the spare buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_UNCORRECTEDPAGES(hFML, ulStart, ulCount, pBuffer, pSpareBuff, ioStat)  \
{                                                                   \
    FFXIOR_FML_WRITE_UNCORRECTEDPAGES  FmlReq;                      \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_UNCORRECTEDPAGES;      \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    FmlReq.pSpare       = (pSpareBuff);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLERASE_RAWBLOCKS()

    Erase one or more raw blocks (not remapped by BBM).

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting block number.
        ulCount     - The count of blocks to process.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLERASE_RAWBLOCKS(hFML, ulStart, ulCount, ioStat)          \
{                                                                   \
    FFXIOR_FML_ERASE_RAWBLOCKS         FmlReq;                      \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_ERASE_RAWBLOCKS;             \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartBlock = (ulStart);                                \
    FmlReq.ulBlockCount = (ulCount);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_RAWPAGES()

    Read uncorrected main page and spare data from one or more pages
    of a raw FML Disk (not remapped by BBM).

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        pSpareBuff  - A pointer to the spare buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_RAWPAGES(hFML, ulStart, ulCount, pBuffer, pSpareBuff, ioStat)  \
{                                                                   \
    FFXIOR_FML_READ_RAWPAGES        FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_RAWPAGES;               \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    FmlReq.pSpare       = (pSpareBuff);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_RAWPAGES()

    Write uncorrected main page and spare data to one or more pages
    of a raw FML Disk (not remapped by BBM).

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process.
        pBuffer     - A pointer to the data buffer to use.
        pSpareBuff  - A pointer to the spare buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_RAWPAGES(hFML, ulStart, ulCount, pBuffer, pSpareBuff, ioStat)  \
{                                                                   \
    FFXIOR_FML_WRITE_RAWPAGES       FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_RAWPAGES;              \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    FmlReq.pSpare       = (pSpareBuff);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_SPARES()

    Read spare area data from one or more pages of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of spares to process.
        pSpareBuff  - A pointer to the buffer to receive the data.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_SPARES(hFML, ulStart, ulCount, pSpareBuff, ioStat)     \
{                                                                   \
    FFXIOR_FML_READ_SPARES          FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_SPARES;                 \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulSpareCount = (ulCount);                                \
    FmlReq.pSpare       = (pSpareBuff);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_SPARES()

    Write spare area data from one or more pages of an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of spares to process.
        pSpareBuff  - A pointer to the buffer to receive the data.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_SPARES(hFML, ulStart, ulCount, pSpareBuff, ioStat)    \
{                                                                   \
    FFXIOR_FML_WRITE_SPARES         FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_SPARES;                \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulSpareCount = (ulCount);                                \
    FmlReq.pSpare       = (pSpareBuff);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_NATIVEPAGES()

    Reads corrected main page and correct spare data from one or
    more pages of an FML Disk.  The given address may be remapped
    by the BBM if necessary to fulfill the request.  Reads will be
    aborted early in the event of any error and the ioStatus updated
    to describe the transfer.

    ioStat.ffxStat may be set to FFXSTAT_FIMUNCORRECTABLEDATA in the
    event that a reliable read is not possible, or this interface is
    not fully supported.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of spares to process.
        pPageBuff   - A pointer to the buffer to receive the data.
        pSpareBuff  - A pointer to the buffer to receive the data.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_NATIVEPAGES(hFML, ulStart, ulCount, pPageBuff, pSpareBuff, ioStat)\
{                                                                   \
    FFXIOR_FML_READ_NATIVEPAGES     FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_READ_NATIVEPAGES;            \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pPageBuff);                              \
    FmlReq.pSpare       = (pSpareBuff);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_NATIVEPAGES()

    Writes a main page and given spare data to one or more pages of
    an FML Disk.  The given address may be remapped by the BBM if
    necessary to complete the request.  Write failures may cause a
    block replacement based on the error policy implementation and
    the ioStatus updated to describe the transfer.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of spares to process.
        pPageBuff   - A pointer to the buffer to receive the data.
        pSpareBuff  - A pointer to the buffer to receive the data.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_NATIVEPAGES(hFML, ulStart, ulCount, pPageBuff, pSpareBuff, ioStat)\
{                                                                   \
    FFXIOR_FML_WRITE_NATIVEPAGES    FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_WRITE_NATIVEPAGES;           \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pPageBuff);                              \
    FmlReq.pSpare       = (pSpareBuff);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

#if FFXCONF_BBMSUPPORT

/*-------------------------------------------------------------------
    Public: FML_GET_BLOCK_INFO()

    Queries the FIM and BBM to describe the requested erase block.
    The given block number is subject to mapping by the BBM.  The
    returned info will include the raw block number within the FIM
    device.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulBlockNum  - The block number.
        ioStat      - An FFXIOSTATUS structure to receive the results.
        info        - An FFXIOR_FML_GET_BLOCK_INFO structure to receive
                      results of the query.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FML_GET_BLOCK_INFO(hFML, ulBlockNum, ioStat, info)          \
{                                                                   \
    FFXIOR_FML_GET_BLOCK_INFO FmlReq;                               \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_GET_BLOCK_INFO;              \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulBlock      = (ulBlockNum);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
    (info) = FmlReq;                                                \
}

/*-------------------------------------------------------------------
    Public: FML_GET_RAW_BLOCK_INFO()

    Queries the FIM and BBM to describe the requested erase block.
    The given block number is a raw block number from within the
    FIM Device.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulBlockNum  - The raw block number.
        ioStat      - An FFXIOSTATUS structure to receive the results.
        info        - An FFXIOR_FML_GET_BLOCK_INFO structure to receive
                      results of the query.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FML_GET_RAW_BLOCK_INFO(hFML, ulBlockNum, ioStat, info)      \
{                                                                   \
    FFXIOR_FML_GET_RAW_BLOCK_INFO FmlReq;                           \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_GET_RAW_BLOCK_INFO;          \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulBlock      = (ulBlockNum);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
    (info) = FmlReq;                                                \
}

/*-------------------------------------------------------------------
    Protected: FML_RETIRE_RAW_BLOCK()

    Copies the given number of pages from the raw block number into
    a replacement block.  Subsequent accesses to the data block
    which is mapped to the raw block will be redirected to the
    replacement block.  The block replacement is an atomic operation
    and is primarily available for development and test purposes.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulBlockNum  - The raw block number.
        nPages      - Number of pages within the given raw block
                      to transfer to the new replacement block.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FML_RETIRE_RAW_BLOCK(hFML, ulBlockNum, nPages, ioStat)      \
{                                                                   \
    FFXIOR_FML_RETIRE_RAW_BLOCK FmlReq;                             \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_RETIRE_RAW_BLOCK;            \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulRawBlock      = (ulBlockNum);                          \
    FmlReq.ulPageCount     = (nPages);                              \
    FmlReq.ulReason        = BLOCKINFO_RETIRED;                     \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}
#endif

/*-------------------------------------------------------------------
    Public: FML_GET_BLOCK_STATUS()

    Get the block status value for an erase block in an FML Disk.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulBlockNum  - The block number.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FML_GET_BLOCK_STATUS(hFML, ulBlockNum, ioStat)              \
{                                                                   \
    FFXIOR_FML_GET_BLOCK_STATUS FmlReq;                             \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_GET_BLOCK_STATUS;            \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulBlock      = (ulBlockNum);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Protected: FML_GET_RAW_BLOCK_STATUS()

    Get the block status value for an erase block in a raw FML Disk
    (not remapped by BBM).

    This macro may be used on NAND media only.

    *Warning* -- This function does little or no range checking,
                 USE WITH CAUTION!

    Parameters:
        hFML        - The FML handle.
        ulBlockNum  - The block number.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FML_GET_RAW_BLOCK_STATUS(hFML, ulBlockNum, ioStat)          \
{                                                                   \
    FFXIOR_FML_GET_RAW_BLOCK_STATUS FmlReq;                         \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_GET_RAW_BLOCK_STATUS;        \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulBlock      = (ulBlockNum);                             \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Protected: FML_SET_RAW_BLOCK_STATUS()

    Set the block status value for an erase block in a raw FML Disk
    (not remapped by BBM).

    This macro may be used on NAND media only.

    *Warning* -- This function does little or no range checking,
                 USE WITH CAUTION!

    Parameters:
        hFML        - The FML handle.
        ulBlockNum  - The block number.
        ulStatus    - The block status value to set.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FML_SET_RAW_BLOCK_STATUS(hFML, ulBlockNum, ulStatus, ioStat)   \
{                                                                   \
    FFXIOR_FML_SET_RAW_BLOCK_STATUS FmlReq;                         \
    FmlReq.ior.ioFunc    = FXIOFUNC_FML_SET_RAW_BLOCK_STATUS;       \
    FmlReq.ior.ulReqLen  = sizeof FmlReq;                           \
    FmlReq.ulBlock       = (ulBlockNum);                            \
    FmlReq.ulBlockStatus = (ulStatus);                              \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Protected: FML_GET_RAW_PAGE_STATUS()

    Get the page status value for an pages in a raw FML Disk
    (not remapped by BBM).

    This macro may be used on NAND media only.

    *Warning* -- This function does little or no range checking,
                 USE WITH CAUTION!

    Parameters:
        hFML        - The FML handle.
        ulPageNum   - The page number.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FML_GET_RAW_PAGE_STATUS(hFML, ulPageNum, ioStat)            \
{                                                                   \
    FFXIOR_FML_GET_RAW_PAGE_STATUS FmlReq;                          \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_GET_RAW_PAGE_STATUS;         \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulPage       = (ulPageNum);                              \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLREAD_OTPPAGES()

    Read main page data from an OTP page.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process
        pBuffer     - A pointer to the data buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLREAD_OTPPAGES(hFML, ulStart, ulCount, pBuffer, ioStat)   \
{                                                                   \
    FFXIOR_FML_READ_OTPPAGES FmlReq;                                \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_OTP_READPAGES;               \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLWRITE_OTPPAGES()

    Write main page data to an OTP page.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process
        pBuffer     - A pointer to the data buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLWRITE_OTPPAGES(hFML, ulStart, ulCount, pBuffer, ioStat)  \
{                                                                   \
    FFXIOR_FML_WRITE_OTPPAGES FmlReq;                               \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_OTP_WRITEPAGES;              \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    FmlReq.ulStartPage  = (ulStart);                                \
    FmlReq.ulPageCount  = (ulCount);                                \
    FmlReq.pPageData    = (pBuffer);                                \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}

/*-------------------------------------------------------------------
    Public: FMLLOCK_OTP()

    Lock OTP pages.

    This macro may be used on NAND media only.

    Parameters:
        hFML        - The FML handle.
        ulStart     - The starting page number.
        ulCount     - The count of pages to process
        pBuffer     - A pointer to the data buffer to use.
        ioStat      - An FFXIOSTATUS structure to receive the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define FMLLOCK_OTP(hFML, ulStart, ulCount, pBuffer, ioStat)        \
{                                                                   \
    FFXIOR_FML_LOCK_OTP FmlReq;                                     \
    FmlReq.ior.ioFunc   = FXIOFUNC_FML_OTP_LOCK;                    \
    FmlReq.ior.ulReqLen = sizeof FmlReq;                            \
    ioStat = FfxFmlIORequest((hFML), &FmlReq.ior);                  \
}


#endif /* FFXCONF_NANDSUPPORT */
#endif /* FXFMLAPI_H_INCLUDED */


