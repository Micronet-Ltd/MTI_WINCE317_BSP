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

    This header defines the private interface between a given FIM instance
    and the Device Manager layer implemented in devmgr.c.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fimdev.h $
    Revision 1.25  2011/05/24 21:22:48Z  billr
    Change to dual license.
    Revision 1.24  2010/06/19 04:21:00Z  garyp
    Updated to track the bit correction capabilities on a per segment basis.
    Revision 1.23  2009/07/29 19:48:48Z  garyp
    Merged from the v4.0 branch.  Added locking and OTP support.
    Revision 1.22  2009/01/23 22:32:10Z  glenns
    Modified FIMINFO structure variable names to meet Datalight coding standards.
    Revision 1.21  2009/01/19 04:24:18Z  keithg
    Removed the now unused OLDNANDDEVICE structure.
    Revision 1.20  2008/10/09 00:59:26Z  keithg
    Changed FFXCONF_BBM_OBSOLESCENT to xxx_USE_v5
    Revision 1.19  2008/09/02 05:59:47Z  keithg
    Added condition for obsolescent BBM code.
    Revision 1.18  2008/07/25 16:50:43Z  billr
    Consistent use of const on input buffers of write functions.
    Revision 1.17  2008/06/24 21:30:43Z  thomd
    Fields to bring info up from ntminfo to fiminfo
    Revision 1.16  2008/03/26 02:40:46Z  Garyp
    Prototype updates.
    Revision 1.15  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.14  2007/09/26 22:12:55Z  pauli
    Updated comments.
    Revision 1.13  2007/03/06 20:54:21Z  Garyp
    Fixed the FIMINFO structure so its members are aligned when packed.
    Revision 1.12  2007/03/01 19:59:58Z  timothyj
    Modified FfxDevApplyArrayBounds() to use take and return block index in
    lieu of byte offset.  Moved uPagesPerBlock into the FimInfo structure,
    where it can be used by other modules to which FimInfo is public, rather
    than computing or storing an additional copy of the same value.
    Revision 1.11  2007/02/13 19:48:47Z  timothyj
    Modified OLDNANDDEVICE interface to use page and block indexes instead of
    linear byte offsets as parameters.
    Revision 1.10  2007/02/12 23:44:10Z  timothyj
    Removed deprecated SpareWrite and ValidatedRead functions from
    OLDNANDDEVICE interface.
    Revision 1.9  2007/01/31 20:24:49Z  Garyp
    Updated to allow erase-suspend support to be compiled out of the product.
    Revision 1.8  2006/11/08 17:46:43Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.7  2006/09/07 00:41:41Z  Garyp
    Function prototype updated, and documentation fixes.
    Revision 1.6  2006/08/31 17:28:00Z  pauli
    Removed the ulWindowSize member of the EXTMEDIAINFO structure.  It is no
    longer used.
    Revision 1.5  2006/05/19 00:13:05Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.4  2006/03/10 03:21:11Z  Garyp
    Conditionally include a structure only if BBM is being used.
    Revision 1.3  2006/02/26 01:36:23Z  Garyp
    Modified so that knowledge of the NAND spare area format is only known
    to the NTM.
    Revision 1.2  2006/02/24 04:29:17Z  Garyp
    Added some stuff from a refactored header.
    Revision 1.1  2006/02/14 00:38:52Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FIMDEV_H_INCLUDED
#define FIMDEV_H_INCLUDED

#include <fxdevice.h>
#include <fxiosys.h>

/*-------------------------------------------------------------------
    FIMINFO

    This structure defines FIM device characteristics, as presented
    to the Device Manager layer.

    ulPhyscialBlocks and ulTotalBlocks will always be the same for
    NOR, but will be different for NAND if BBM is being used.  The
    ulTotalBlocks value will be reduced so that it does not include
    the BBM area.  Additionally, for NOR ulPhysicalBlocks will
    include any reserved space (low and high), while for NAND, this
    field will not include any reserved areas because they are
    completely hidden by the NTM.

    Note that a FIM device may consist of multiple physical chips.
      - If the chips are interleaved, the requisite block and page
        sizes are adjusted accordingly, and this completely hidden
        from layers outside the FIM itself.
      - If the chips are organized end-to-end, the total number of
        chips can be calculated by dividing ulPhysicalBlocks by
        ulChipBlocks.  Typically higher layers only care about this
        arrangement if the chip has boot blocks, which must be
        specially handled.

    Boot Block Information (NOR only)
        Boot blocks are mini-erase zones which typically reside at
        the beginning or end of the flash, sometimes both.  They
        normally span one regular sized erase zone, though this
        interface allows more than that if need be.

        The counts are specified on a per-chip basis.  If an array
        of chips is used, boot blocks will appear in the middle of
        the array.

        If interleaved chips are used, this will already be factored
        into the "boot block size" value.

        For normal operation, FlashFX simply treats the boot-block
        areas as regular flash, automatically handling the extra
        logic to erase these areas.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT16    uDeviceType;        /* Device type code prefixed with DEVTYPE_ */
    D_UINT16    uDeviceFlags;
    D_UINT16    uLockFlags;         /* Locking capabilities: FFXLOCKFLAGS_*    */
    D_UINT16    uAlignSize;         /* The required alignment, in bytes        */
    D_UINT32    ulReservedBlocks;   /* Total reserved blocks at the start      */
    D_UINT32    ulPhysicalBlocks;   /* Total physical blocks                   */
    D_UINT32    ulTotalBlocks;      /* Total erase blocks in the array         */
    D_UINT32    ulChipBlocks;       /* Total blocks per each physical chip     */
    D_UINT32    ulBlockSize;        /* Bytes per erase block                   */
    D_UINT16    uPagesPerBlock;     /* Pages per erase block                   */
    D_UINT16    uPageSize;          /* Bytes per programmable page             */
    D_UINT16    uSpareSize;         /* Spare bytes per page (NAND)             */
    D_UINT16    uMetaSize;          /* Bytes available per spare area (NAND)   */
    D_UINT16    uEdcSegmentSize;    /* Segment size in bytes to which uEdcCapability applies */
    D_UINT16    uEdcCapability;     /* Max error correcting capability in bits per segment bytes */
    D_UINT16    uEdcRequirement;    /* Required level of error correction */
    D_UINT32    ulEraseCycleRating;  /* Factory spec erase cycle rating */
    D_UINT32    ulBBMReservedRating; /* Reserved blocks required for above */
  #if FFXCONF_OTPSUPPORT
    unsigned    nOTPPages;          /* # of OTP pages, or zero if unsupported  */
  #endif
  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    D_UINT32    ulBootBlockSize;    /* Boot block size                         */
    D_UINT16    uBootBlockCountLow; /* Low end boot block count (per chip)     */
    D_UINT16    uBootBlockCountHigh;/* High end boot block count (per chip)    */
  #endif
} FIMINFO;


/*-------------------------------------------------------------------
    FIMEXTRA

    FIMEXTRA is used in FIMs to store extra information accessible
    through the ExtndMediaInfo structure.
-------------------------------------------------------------------*/
typedef struct tagFIMEXTRA *PFIMEXTRA;


/*-------------------------------------------------------------------
    EXTMEDIAINFO

    Note that this structure is deprecated.  It is only used by old
    style NOR FIMs which use the wrapper interface in norwrap.c.
-------------------------------------------------------------------*/
typedef struct tagEXTMEDIAINFO
{
    D_UINT16        uDeviceType;        /* Device type code prefixed with DEV_  */
    D_UINT16        uDriveNum;          /* The drive number (DISKn)             */
    D_UINT32        ulTotalSize;        /* Total size of the entire array       */
    D_UINT32        ulDeviceSize;       /* Size of one physical device          */
    D_UINT32        ulEraseZoneSize;    /* Smallest physical eraseable sector   */
    D_UINT32        ulBootBlockSize;    /* Size of each boot block, if any      */
    D_UINT16        uPageSize;          /* Size of a programmable page          */
    D_UINT16        uSpareSize;         /* Spare bytes per page                 */
    FFXDEVHANDLE    hDev;               /* A pointer to optional extra device info */
    PFIMEXTRA       pFimExtra;          /* A pointer to optional extra FIM info */
    D_UINT16        uBootBlockCountLow; /* Boot blocks at the bottom of each chip */
    D_UINT16        uBootBlockCountHigh;/* Boot blocks at the top of each chip  */
} EXTMEDIAINFO, *PEXTMEDIAINFO;


typedef struct tagFIMDATA FIMDATA;
typedef struct tagFIMDATA *FIMHANDLE;


/*-------------------------------------------------------------------
    FIMDEVICE

    This defines the function call interface into the FIMs.

    Note that the first 10 functions are deprecated, and only used
    by the old-style NOR FIMs, which all use the wrapper interface
    defined in norwrap.c.

    All the new style FIMs, both NAND and NOR use only the last
    5 functions, Create(), Delete(), IORequest(), ParameterGet(),
    and ParameterSet().
-------------------------------------------------------------------*/
typedef struct tagFIMDEVICE
{
    /* TBD: Can we remove most of these? */
    D_BOOL      (*Mount)         (PEXTMEDIAINFO pEMI);
    void        (*Unmount)       (PEXTMEDIAINFO pEMI);
    D_BOOL      (*Read)          (PEXTMEDIAINFO pEMI, D_UINT32 ulStart, D_UINT16 uLength, void * pBuffer);
    D_BOOL      (*Write)         (PEXTMEDIAINFO pEMI, D_UINT32 ulStart, D_UINT16 uLength, void * pBuffer);
    D_BOOL      (*EraseStart)    (PEXTMEDIAINFO pEMI, D_UINT32 ulStartBlock, D_UINT32 ulCount);
    D_UINT32    (*ErasePoll)     (PEXTMEDIAINFO pEMI);
    D_BOOL      (*EraseSuspend)  (PEXTMEDIAINFO pEMI);
    void        (*EraseResume)   (PEXTMEDIAINFO pEMI);
    D_BOOL      (*ReadControl)   (PEXTMEDIAINFO pEMI, D_UINT32 ulStart, D_UINT16 uLength, void *pBuffer);
    D_BOOL      (*WriteControl)  (PEXTMEDIAINFO pEMI, D_UINT32 ulStart, D_UINT16 uLength, void *pBuffer);
    FIMHANDLE   (*Create)        (FFXDEVHANDLE hDev, const FIMINFO **ppFimInfo);
    void        (*Destroy)       (FIMHANDLE hFim);
    FFXIOSTATUS (*IORequest)     (FIMHANDLE hFim, FFXIOREQUEST *pIOR);
    FFXSTATUS   (*ParameterGet)  (FIMHANDLE hFim, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
    FFXSTATUS   (*ParameterSet)  (FIMHANDLE hFim, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
    struct tagFIMDEVICE *pOldFimDevice;
} FIMDEVICE;


typedef struct
{
    FFXIOREQUEST    ior;
    D_UINT32        ulStartBlock;
    D_UINT32        ulCount;
} FFXIOR_FIM_ERASE_START;

typedef struct
{
    FFXIOREQUEST    ior;
} FFXIOR_FIM_ERASE_POLL;

#if FFXCONF_ERASESUSPENDSUPPORT
typedef struct
{
    FFXIOREQUEST    ior;
} FFXIOR_FIM_ERASE_SUSPEND;

typedef struct
{
    FFXIOREQUEST    ior;
} FFXIOR_FIM_ERASE_RESUME;
#endif


void        FfxDevGetArrayBounds(FFXDEVHANDLE hDev, FFXFIMBOUNDS *pBounds);
D_UINT32    FfxDevApplyArrayBounds(D_UINT32 ulTotalBlocks, D_UINT32 ulBlockSize, const FFXFIMBOUNDS *pBounds);


/*  Error return value from ErasePoll()
*/
#define ERASE_FAILED ((D_UINT32)-1)



#endif /* FIMDEV_H_INCLUDED */


