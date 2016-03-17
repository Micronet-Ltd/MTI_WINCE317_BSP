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

    This module contains the Windows CE FlashFX Hooks layer default
    implementations for:

       FfxHookBbmFormatBlock()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    These functions provide a default implementation that uses software ECCs,
    which serves as a template for supporting hardware ECC generation.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhbbm.c $
    Revision 1.10  2009/08/02 19:35:14Z  garyp
    Merged from the v4.0 branch.  Updated to examine the properly masked
    ulBlockStatus value.  Eliminated references to the obsolete "Legacy" style
    Factory Bad Block markings.  Documentation updated accordingly.
    Revision 1.9  2009/04/08 20:21:32Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.8  2009/03/18 18:06:50Z  glenns
    Fix Bugzilla #2370: Remove commentary about obsolete block status values.
    Revision 1.7  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/03/01 22:01:04Z  timothyj
    Changed interface to take a block index in lieu of a linear byte offset.
    Revision 1.5  2006/12/20 21:35:01Z  timothyj
    Removed conditional FFXCONF_BBMFORMAT, so that FfxHookBbmFormatBlock() is
    included even when FFXCONF_BBMFORMAT is FALSE.  FfxHookBbmFormatBlock()
    is called by FfxBbmFormatChunk(), which is called by FfxBbmNextBlock() and
    is included and used regardless of whether BBM formatting is enabled.
    Revision 1.4  2006/03/13 02:38:28Z  Garyp
    Updated the debug output to better describe "possible" bad blocks.
    Revision 1.3  2006/02/10 23:54:56Z  Garyp
    Header updates.
    Revision 1.2  2006/02/10 00:33:17Z  Garyp
    Updated to use device/disk handles rather than the EXTMEDIAINFO structure.
    Revision 1.1  2005/05/02 16:18:10Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>

#if FFXCONF_BBMSUPPORT



/*-------------------------------------------------------------------
    Public: FfxHookBbmFormatBlock()

    Hook the BBM format functionality.

    This function is called during BBM format operations when
    a block is about to be formatted.  The ulBlockStatus value
    indicates the state of the block.  BBM's default behavior
    with regard to the specified block may be changed by
    returning a different block status value than what was
    passed in.

    The return value indicates the desired action to perform.
    Simply return the original ulBlockStatus value to cause BBM
    to treat the block in its normal fashion.  Return a different
    block status value to change the BBM behavior.  Return
    BLOCKSTATUS_NOTBAD to treat the block as a good block even
    if it was originally bad.  Good blocks can be treated as
    factory-bad (and therefore ignored) as a means for mapping
    out blocks in the middle of the flash array that may be
    required for other purposes (some NAND controllers do this).

    Note that there may be other bits, such as lock flags, set in
    the ulBlockStatus value.  These should generally be left
    unchanged.  See fxiosys.h for the definitions of the various
    block status bits.

    Parameters:
        hDev          - The Device handle
        ulBlock       - The block
        ulBlockStatus - The type of bad block, one of the following
                        values will be set:

            BLOCKSTATUS_NOTBAD       - The block is good
            BLOCKSTATUS_FACTORYBAD   - Bad from the factory

    Return Value:
        The potentially updated block status value.
-------------------------------------------------------------------*/
D_UINT32 FfxHookBbmFormatBlock(
    FFXDEVHANDLE    hDev,
    D_UINT32        ulBlock,
    D_UINT32        ulBlockStatus)
{
    DclAssert(hDev);

    (void)hDev;

    if((ulBlockStatus & BLOCKSTATUS_MASKTYPE) != BLOCKSTATUS_NOTBAD)
    {
        FFXPRINTF(1, ("BBM: Bad Block found at block %lX, style=%lX\n",
            ulBlock, ulBlockStatus));
    }

    /*  Default to simply treating the block according to the
        standard BBM behavior.
    */
    return ulBlockStatus;
}



#endif  /* FFXCONF_BBMSUPPORT */

