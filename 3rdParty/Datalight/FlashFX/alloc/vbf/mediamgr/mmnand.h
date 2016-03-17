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

    Interface to mmnand.c.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: mmnand.h $
    Revision 1.10  2009/03/12 20:40:44Z  billr
    Resolve bug 2124: Page buffers are allocated on the stack.
    Revision 1.9  2009/01/27 03:11:18Z  billr
    Resolve bug 1610: VBF write protects the disk after an
    uncorrectable ECC error.  Implement new Media Manager function
    FfxMediaMgrCopyBadBlock().  Allow per-instance data for
    media-specific functions.
    
    --- Added comments ---  billr [2009/01/29 20:00:22Z]
    Make that bug 1601.
    Revision 1.8  2009/01/07 01:18:23Z  billr
    FfxMMNandReadHeader() now returns a full FFXIOSTATUS, and
    propagates information about correctable errors.
    Revision 1.7  2008/06/03 20:57:54Z  thomd
    Added fFormatNoErase
    Revision 1.6  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/11/01 00:56:23Z  Garyp
    Major update to eliminate the inconsistent use of D_UINT16 types.  All block
    indexes and counts which were D_UINT16 are now consistently unsigned.
    Replaced the use of Return_t and IOReturn_t with FFXSTATUS and
    FFXIOSTATUS.  Improved error handling.
    Revision 1.4  2007/04/19 23:58:37Z  timothyj
    Added fLargeAddressing parameter, to indicate whether the on-media format
    should use large flash array compatible addressing.
    Revision 1.3  2006/02/08 20:20:26Z  Garyp
    Modified to use the updated FML interface.
    Revision 1.2  2006/01/09 19:38:10Z  billr
    Merge Sibley support from v2.01.
    Revision 1.1  2005/10/02 00:33:40Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

FFXSTATUS   FfxMMNandDeviceInit(MEDIAMGRINSTANCE *pMM);
FFXIOSTATUS FfxMMNandReadHeader(FFXFMLHANDLE hFML, D_UINT32 ulStart, EUH *peuh, void *pPage);
FFXSTATUS   FfxMMNandFormatEraseUnit(FFXFMLHANDLE hFML, D_UINT32 ulUnitStart, const EUH *peuh, D_BOOL fLargeAddressing, D_BOOL fFormatNoErase, void *pPage);
unsigned    FfxMMNandCalcEUHDataUnitBlocks(const EUH *peuh);

