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

    This header defines internal interfaces used by the NAND FIM and NTMs.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nand.h $
    Revision 1.26  2010/06/19 04:21:00Z  garyp
    Updated to track the bit correction capabilities on a per segment basis.
    Revision 1.25  2009/12/11 20:38:47Z  garyp
    Minor type changes.  Moved some stuff to fxnandapi.h.
    Revision 1.24  2009/07/24 18:48:44Z  garyp
    Merged from the v4.0 branch.  Updated to support OTP and locking.
    Revision 1.23  2009/03/09 18:33:17Z  glenns
    - Fix Bugzilla #2384: Add an element to the NTMINFO structure
      so the parameterized EDC mechanism and clients of the FIM
      can distinguish between how big the spare area physically is and
      how much is actually used.
    Revision 1.22  2009/03/04 18:04:53Z  glenns
    - Removed "ulReservedBlock" element from NTMINFO, as it is
      no longer used.
    Revision 1.21  2009/02/26 02:04:00Z  glenns
    - Removed "MAX_ADDR_LINES" macro that is no longer used.
    - Moved remaining LFA-related macros to "nandctl.h".
    Revision 1.20  2009/02/24 02:04:51Z  glenns
    - Added some macros to assist in making LFA changes in the
      NTMs more clear.
    Revision 1.19  2009/02/02 18:03:54Z  billr
    Implement FXIOSUBFUNC_FIM_READ_NATIVEPAGES by calling a FIM
    NativePageRead() method if it is available, with a fallback to
    RawPageRead() if it is not.
    --- Added comments ---  billr [2009/02/02 21:24:15Z]
    Make that "calling an NTM NativePageRead() method."
    Revision 1.18  2009/01/23 22:43:38Z  glenns
    - Updated NTMINFO structure to use variable names that meet
      Datalight coding standards.
    - Clarified commentary.
    Revision 1.17  2009/01/16 23:55:04Z  glenns
    - Added new elements to NTMINFO to support parameterized
      EDC and custom spare area formats.
    Revision 1.16  2008/06/16 13:19:47Z  thomd
    Added chip capability fields to NTMINFO structure
    Revision 1.15  2008/03/23 20:38:17Z  Garyp
    Structure and prototype updates to support variable length tags.
    Revision 1.14  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.13  2007/09/26 22:12:55Z  pauli
    Resolved Bug 355: Added an alignment field to the NTMINFO structure.
    Revision 1.12  2007/02/22 19:15:31Z  timothyj
    Moved uPagesPerBlock into the FimInfo structure, where it can be used by
    other modules to which FimInfo is public, rather than computing or storing
    an additional copy of the same value.  Added reference to PXA320 NTM.
    Revision 1.11  2007/02/13 22:18:57Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.
    Revision 1.10  2007/02/05 15:30:15Z  timothyj
    Updated function calls into NTMs to use blocks and pages instead of linear
    byte offsets.
    Revision 1.9  2006/11/08 03:38:50Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.8  2006/05/17 22:02:50Z  Garyp
    Added the ulReservedBlocks and ulChipBlocks fields to the NtmInfo structure.
    Revision 1.7  2006/03/04 19:24:38Z  Garyp
    Added RawPageRead/Write() support.
    Revision 1.6  2006/03/03 20:14:02Z  Garyp
    Moved a misplaced definition.
    Revision 1.5  2006/02/26 02:30:55Z  Garyp
    Prototype updates.
    Revision 1.4  2006/02/23 20:44:35Z  Garyp
    Added some types.
    Revision 1.3  2006/02/08 18:16:54Z  Garyp
    Updated to no longer use the EXTMEDIAINFO structure.
    Revision 1.2  2006/01/11 02:12:34Z  Garyp
    Eliminated support for IsWriteProtected() and GetChipInfo().
    Revision 1.1  2005/12/03 19:55:02Z  Pauli
    Initial revision
    Revision 1.3  2005/12/03 19:55:01Z  Garyp
    Prototypes updated,  Removed dead code.
    Revision 1.2  2005/10/31 00:10:16Z  Garyp
    Major update to support run-time page size determination.
    Revision 1.1  2005/10/10 04:44:14Z  Garyp
    Initial revision
    Revision 1.3  2005/07/30 20:25:25Z  Garyp
    Updated to support read-ahead capability.
    Revision 1.2  2005/07/29 17:17:05Z  Garyp
    Updated to compile cleanly.
    Revision 1.1  2005/07/28 17:38:34Z  pauli
    Initial revision
    Revision 1.15  2005/04/01 18:24:43Z  billr
    Add ID for Samsung K9K1216Q0C (512 Mbit, 16-bit I/O).
    Revision 1.14  2005/03/27 19:26:34Z  GaryP
    Standardized the function headers.  General code formatting cleanup.
    Added some debug code.  Changed functions to static where possible.
    Renamed public but non-published functions to reduce name-space pollution.
    Revision 1.13  2004/12/30 23:17:53Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.12  2004/07/21 20:24:30Z  jaredw
    added ST mfg code (0x20)
    Revision 1.11  2003/10/09 21:01:29Z  garys
    Added support for the Toshiba TH58NVG1S3AFT
    Revision 1.10  2003/09/11 21:14:55  billr
    Merged from [Integration_2K_NAND] branch. Major rewrite to support
    NAND parts with 2KB page size.
    Revision 1.1  2003/07/31 01:56:34Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#include <fxiosys.h>

struct sNTMINFO
{
    D_UINT16    uDeviceFlags;
    D_UINT16    uAlignSize;         /* The required alignment, in bytes         */
    unsigned    nLockFlags;         /* Locking capabilities: FFXLOCKFLAGS_*     */
  #if FFXCONF_OTPSUPPORT
    unsigned    nOTPPages;          /* # of OTP pages, or zero if unsupported   */
  #endif
    D_UINT32    ulTotalBlocks;      /* Total blocks in array, not including reserved */
    D_UINT32    ulChipBlocks;       /* The number of blocks per chip            */
    D_UINT32    ulBlockSize;        /* Smallest physical eraseable sector       */
    D_UINT16    uPagesPerBlock;     /* Pages per block                          */
    D_UINT16    uPageSize;          /* Device page size                         */
    D_UINT16    uSpareSize;         /* Spare area per page                      */
    D_UINT16    uSpareAreaUsed;     /* How much of the spare area will be used  */
    D_UINT16    uMetaSize;          /* Meta bytes per spare area                */
    D_UINT16    uEdcSegmentSize;    /* Segment size in bytes to which uEdcCapability applies */
    D_UINT16    uEdcCapability;     /* Max error correcting capability in bits per segment */
    D_UINT16    uEdcRequirement;    /* Required level of error correction */
    D_BOOL      fEdcParameterized;  /* Indicates whether to use parameterized ECC */
    D_BOOL      fEdcProtectsMetadata; /* Whether primary EDC protection is needed for tags */
    D_UINT16    uCustomSpareFormat;  /* Indicates custom spare area format handling */
    D_UINT32    ulEraseCycleRating;  /* Factory spec erase cycle rating */
    D_UINT32    ulBBMReservedRating; /* Reserved blocks required for above */
    
    /*  REFACTOR: the following elements of this structure are used by the
        parameterized ECC mechanism, and by some of the new algorithms being
        put into Tera for protecting tags and page status fields with ECCs 
        and CRCs. THEY ARE ALLOCATED AT CREATE TIME by NTMs that wish to use
        parameterized EDC.

        This isn't really the  right way to do this; there should be a memory
        pool server from which temporary buffers can be secured and released. 
        However, that isn't yet developed, and therefore we are doing this here
        for now.

        Note that not all NTMs use these elements and do not allocate them, so
        it is very important that no module attempts to use them outside the
        NTM or the nthelp mechanisms.
    */
    D_BUFFER    *pScratchSpareBuffer;
    D_BUFFER    *pScratchEccBuffer;
    D_BUFFER    *pScratchPageBuffer;
};

/*  Table of entry points for the NTM's internal operations.
*/
typedef struct NANDTECHNOLOGYMODULE
{
    NTMHANDLE   (*Create)          (FFXDEVHANDLE hDev, const NTMINFO **ppNtmInfo);
    void        (*Destroy)         (NTMHANDLE hNTM);
    FFXIOSTATUS (*PageRead)        (NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pBuffer, D_BUFFER *pTags, D_UINT32 ulCount, unsigned nTagWidth, D_BOOL fUseEcc);
    FFXIOSTATUS (*PageWrite)       (NTMHANDLE hNTM, D_UINT32 ulStartPage, const D_BUFFER *pBuffer, const D_BUFFER *pTags, D_UINT32 ulCount, unsigned nTagWidth, D_BOOL fUseEcc);
    FFXIOSTATUS (*HiddenRead)      (NTMHANDLE hNTM, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pcHidden, unsigned nTagWidth);
    FFXIOSTATUS (*HiddenWrite)     (NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pTags, unsigned nTagWidth);
    FFXIOSTATUS (*SpareRead)       (NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
    FFXIOSTATUS (*SpareWrite)      (NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
    FFXIOSTATUS (*BlockErase)      (NTMHANDLE hNTM, D_UINT32 ulBlock);
    FFXIOSTATUS (*GetPageStatus)   (NTMHANDLE hNTM, D_UINT32 ulPage);
    FFXIOSTATUS (*GetBlockStatus)  (NTMHANDLE hNTM, D_UINT32 ulBlock);
    FFXIOSTATUS (*SetBlockStatus)  (NTMHANDLE hNTM, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
    FFXSTATUS   (*ParameterGet)    (NTMHANDLE hNTM, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
    FFXSTATUS   (*ParameterSet)    (NTMHANDLE hNTM, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
    /*  The following functions are optional.  Their pointers may be
        NULL if the NTM does not support them.
    */
    FFXIOSTATUS (*RawPageRead)     (NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pPages, D_BUFFER *pSpares, D_UINT32 ulCount);
    FFXIOSTATUS (*RawPageWrite)    (NTMHANDLE hNTM, D_UINT32 ulStartPage, const D_BUFFER *pPages, const D_BUFFER *pSpares, D_UINT32 ulCount);
    FFXIOSTATUS (*IORequest)       (NTMHANDLE hNTM, FFXIOREQUEST *pIOR);
    FFXIOSTATUS (*NativePageRead)  (NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pPages, D_BUFFER *pSpares, D_UINT32 ulCount);
} NANDTECHNOLOGYMODULE, *PNANDTECHNOLOGYMODULE;
