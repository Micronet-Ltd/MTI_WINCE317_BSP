/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    Interface to the Media Manager layer.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: mediamgr.h $
    Revision 1.16  2009/01/27 23:31:38Z  billr
    Resolve bug 1610: VBF write protects the disk after an
    uncorrectable ECC error.  Implement new Media Manager function
    FfxMediaMgrCopyBadBlock().  Allow per-instance data for
    media-specific functions.
    
    --- Added comments ---  billr [2009/01/29 19:59:06Z]
    Make that bug 1601.
    Revision 1.15  2009/01/10 00:31:24Z  billr
    FfxMediaMgrReadLnuHeader() now returns a full FFXIOSTATUS.
    Revision 1.14  2009/01/07 01:18:23Z  billr
    FfxMediaMgrReadHeader() now returns a full FFXIOSTATUS.
    Revision 1.13  2008/05/23 17:16:40Z  thomd
    Added fFormatNoErase
    Revision 1.12  2008/03/23 02:39:23Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.11  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/11/01 00:43:38Z  Garyp
    Major update to eliminate the inconsistent use of D_UINT16 types.  All
    block indexes and counts which were D_UINT16 are now consistently unsigned.
    Replaced the use of Return_t and IOReturn_t with FFXSTATUS and FFXIOSTATUS.
    Improved error handling.
    Revision 1.9  2007/10/10 22:58:26Z  billr
    Resolve bug 412.
    Revision 1.8  2007/10/04 23:25:48Z  billr
    Resolve bug 970.
    Revision 1.7  2007/04/19 23:55:21Z  timothyj
    Added fLargeAddressing parameter, to indicate whether the on-media format
    should use large flash array compatible addressing.
    Revision 1.6  2006/11/08 03:38:53Z  Garyp
    Added ParameterGet/Set() functionality.
    Revision 1.5  2006/10/18 17:34:00Z  billr
    Revert changes made in revision 1.4.
    Revision 1.3  2006/02/01 00:16:31Z  Garyp
    Modified to use the updated FML interface.
    Revision 1.2  2006/01/05 18:49:46Z  billr
    Merge Sibley support from v2.01.
    Revision 1.1  2005/12/08 02:51:14Z  Garyp
    Initial revision
    Revision 1.13  2005/08/14 19:18:40Z  Cheryl
    prototypes for new functions with FfxMediaMgr prefix.
    Revision 1.12  2004/12/30 23:17:54Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.11  2003/11/27 00:26:07Z  garys
    Merge from FlashFXMT
    Revision 1.10.1.2  2003/11/27 00:26:07  billr
    Changes for thread safety. Compiles (Borland/DOS), not yet tested.
    Revision 1.10  2003/06/11 20:43:23Z  billr
    No functional changes. Remove dead conditional code.
    Revision 1.9  2003/06/09 23:39:31Z  billr
    Implement FIM "merge writes" characteristic for writing and formatting (was
    previously only done for discards).
    Revision 1.8  2003/05/23 00:04:50Z  billr
    Remove isNAND flag from the EUH. The Media Manager now gets this
    information via oemmediainfo() during vbfmount().
    Revision 1.7  2003/05/16 23:58:06Z  billr
    Media Manager now modifies its behavior to accommodate MLC flash
    and flash parts that don't allow writing a '1' where a '0' was previously
    programmed. A FIM controls this by setting bits in uDeviceType in
    the ExtndMediaInfo structure.
    Revision 1.6  2003/05/15 00:35:02Z  billr
    WriteToLnu() and ReadFromLnu() buffers are now void * rather than D_UCHAR *.
    Revision 1.5  2003/05/15 00:30:47Z  billr
    No functional changes. Remove some obsolete code.
    Revision 1.4  2003/05/14 16:38:16Z  billr
    Add new Media Manager function ExpendLnuAllocations().
    Revision 1.3  2003/04/29 16:48:33Z  billr
    Clean up compiler warnings in WinCE build.
    Revision 1.2  2003/04/09 18:59:26Z  billr
    Merge from VBF4 branch.
    Revision 1.1.1.2  2003/04/09 18:59:26Z  billr
    On the way to resolving the ReadEUH() and FormatUnit() mess.
    Revision 1.1  2003/04/04 22:30:16Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#if FFXCONF_VBFSUPPORT

/*  NOTE: These three MediaMgr functions are called before VBF is mounted
*/
FFXIOSTATUS FfxMediaMgrReadHeader(              FFXFMLHANDLE hFML, D_UINT32 ulStart, EUH *pEUH);
FFXSTATUS   FfxMediaMgrFormatUnit(              FFXFMLHANDLE hFML, D_UINT32 ulStart, const EUH *pEUH, D_BOOL fLargeAddressing, D_BOOL fFormatNoErase);
unsigned    FfxMediaMgrCalcEUHDataUnitBlocks(   FFXFMLHANDLE hFML, const EUH *pEUH);

HMEDIAMGR   FfxMediaMgrInstanceCreate(          FFXFMLHANDLE hFML);
void        FfxMediaMgrInstanceDestroy(         HMEDIAMGR hMM);
FFXSTATUS   FfxMediaMgrParameterGet(            HMEDIAMGR hMM, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
FFXSTATUS   FfxMediaMgrParameterSet(            HMEDIAMGR hMM, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
FFXSTATUS   FfxMediaMgrFormatLnu(            const VBFDATA *pVBF, LinearUnit_t lnu, const EUH *pEUH);
FFXIOSTATUS FfxMediaMgrWriteToLnu(           const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nBlockIndex, unsigned nBlockCount, unsigned nBlockKey, const void *pBuffer);
FFXIOSTATUS FfxMediaMgrReadFromLnu(          const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nBlockIndex, unsigned nBlockCount, void *pBuffer);
FFXIOSTATUS FfxMediaMgrReadLnuHeader(        const VBFDATA *pVBF, LinearUnit_t lnu, EUH *pEUH);
FFXIOSTATUS FfxMediaMgrReadLnuAllocations(   const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nStartAlloc, unsigned nMaxAllocs , Allocation[]);
FFXSTATUS   FfxMediaMgrDiscardLnuAllocations(const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nStartAlloc, unsigned nMaxAllocs);
FFXIOSTATUS FfxMediaMgrExpendLnuAllocations( const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nStartAlloc, unsigned nMaxAllocs);
FFXSTATUS   FfxMediaMgrCopyBadBlock(         const VBFDATA *pVBF, LinearUnit_t lnuFrom, unsigned nBlockFrom, LinearUnit_t lnuTo, unsigned nBlockTo);

#if FFXCONF_NANDSUPPORT
FFXSTATUS   FfxMediaMgrBlankCheck(           const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nBlockIndex);
FFXSTATUS   FfxMediaMgrVerify(               const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nBlockIndex);
D_BOOL      FfxMediaMgrIsNand(               const VBFDATA *pVBF);
#endif

#endif
