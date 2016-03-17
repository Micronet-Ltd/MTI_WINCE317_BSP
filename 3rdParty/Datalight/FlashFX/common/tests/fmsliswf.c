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

    ISWF utility functions for FMSL tests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmsliswf.c $
    Revision 1.5  2009/03/31 21:20:39Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2008/01/13 07:26:49Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/02/27 20:57:23Z  Garyp
    Header updates.
    Revision 1.1  2006/05/18 23:21:06Z  Garyp
    Initial revision
    Revision 1.2  2006/01/31 23:03:04Z  Garyp
    Header updates.
    Revision 1.1  2006/01/11 22:50:10Z  billr
    Initial revision
    Revision 1.2  2005/08/30 16:04:25Z  billr
    Some compilers insist on a newline at the end of the file.
    Revision 1.1  2005/08/27 19:40:10Z  Cheryl
    Initial revision
-----------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include "fmsltst.h"


#if FFXCONF_ISWFSUPPORT

/*-------------------------------------------------------------------
    Public: FfxFmslISWFCalcControlAddress()

    Update the physical address based on the length and access
    type.  This is only used for control read/write operations.

    Parameters:
        ulAddress - physical address
        uPageSize - size of page

    Return Value:
        Address updated by uLength - take into account control pages
        for ISWF.  If crossing page boundary due to the addition of
        uLength, update page properly.
-------------------------------------------------------------------*/
D_UINT32 FfxFmslISWFCalcControlAddress(
    D_UINT32    ulAddress,
    D_UINT16    uPageSize)
{
    D_UINT32    ulRetAddress = ~(D_UINT32) 0;
    D_UINT32    ulPage = 0;
    D_UINT32    ulOffset = 0;
    D_UINT32    ulControlOffsetMask = 0;
    D_UINT32    ulPageMask = 0;

    /*  control mode offset mask
    */
    ulControlOffsetMask = (uPageSize>>1) - 1;
    ulPageMask = ~(D_UINT32)(uPageSize - 1);

    /*  find page
    */
    ulPage = ulAddress&ulPageMask;

    /*  find offset into page
    */
    ulOffset = ulAddress&ulControlOffsetMask;

    /* starting address +
       + page if sitting on page boundary +
       offset in range
    */
    ulRetAddress = (ulPage
            + ((ulOffset & ~ulControlOffsetMask) << 1)
            + (ulOffset & ulControlOffsetMask));

    return ulRetAddress;
}



#endif /* FFXCONF_ISWFSUPPORT */
