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

    This module provides functionality for calculating Cylinder, Head, and
    SectorsPerTrack (CHS) values.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dskchs.c $
    Revision 1.9  2009/04/01 15:34:45Z  davidh
    Function Headers Updated for AutoDoc.
    Revision 1.8  2009/02/08 08:41:27Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.7  2008/03/22 17:17:37Z  Garyp
    Updated to allow CHS calculations to work for disks as small as two sectors.
    Revision 1.6  2008/01/13 07:27:09Z  keithg
    Function header updates to support autodoc.
    Revision 1.5  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/08/01 14:59:02Z  timothyj
    Modified to return the sentinel values with the cylinders, heads, and
    sectors-per-track exceeds that which can be stored in the MBR.  Callers
    should generally ignore these values and use the total sectors, or
    implementations will be limited to ~8GB.
    Revision 1.3  2006/10/16 20:32:10Z  Garyp
    Minor header changes.
    Revision 1.2  2006/01/08 21:32:58Z  Garyp
    Documentation and debug code changes only -- nothing functional.
    Revision 1.1  2005/10/02 01:45:30Z  Pauli
    Initial revision
    Revision 1.5  2004/12/30 23:03:12Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2004/09/25 04:10:32Z  GaryP
    Added error handling to FfxCalculateCHS().
    Revision 1.3  2004/08/05 20:33:32Z  GaryP
    Changed some production asserts to debug asserts.
    Revision 1.2  2003/04/13 02:27:06Z  garyp
    Header file changes.
    Revision 1.1  2003/03/26 00:42:32Z  garyp
    Initial revision
    04/04/01 gjs Removed mjm's fix in BuildCHS cuz GP also fixed this bug
                 but in a more appropriate place (fatfmt.c, BuildMBR).
                 Fixed the assert for total sectors to be ==, not <=.
    03/15/01 mjm Make Cylinder value zero-based (ETR3497).
    01/03/01 gp  Renamed MAX_SPC to MAX_SPT
    08/28/00 HDS Fixed the while loop to prevent a system hang-up.
    11/22/99 GJS Fixed sectors per track logic for new MAX_SPC (63).
    10/20/98 PKG Made cylinder count 32 bit to handle large disks
    01/22/98 PKG Removed invalid assert
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses new extended Datalight specific types.
    08/26/97 PKG Now uses new Datalight specific types
    07/09/97 DM  Fixed invalid assert
    06/06/97 DM  Original extracted from FATFMT.C
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <diskapi.h>


/*-------------------------------------------------------------------
    Public: FfxCalculateCHS()

    Calculates Cylinder-Heads-SectorsPerTrack values based on a
    total number of sectors.  On exit, the total number of sectors
    will be adjusted to reflect the calculated CHS values.

    If the disk is too large to be represented in CHS notation when
    considering the typical limits imposed by MBRs and BPBs, this
    function returns CHS values as the MBR tuple 'sentinal values',
    and returns *pulTotalSectors value as-is.

    Generally, callers should use the *pulTotalSectors value
    exclusively, or better, not call this function at all. Callers
    that use the CHS values are limited by the MBR data structures
    to ~8GB.

    Parameters:
        pulTotalSectors - Total number of sectors in the media.
        puSides         - Number of sides.
        puSpt           - Number of sectors per track (SPC).
        puCyls          - Number of cylinders.

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxCalculateCHS(
    D_UINT32   *pulTotalSectors,
    D_UINT16   *puSides,
    D_UINT16   *puSpt,
    D_UINT16   *puCyls)
{
    D_UINT16    uSides;
    D_UINT16    uSecPerTrack;
    D_UINT32    ulCylinders;

    FFXPRINTF(1, ("FfxCalculateCHS() TotalSectors=%lU\n", *pulTotalSectors));

    DclAssert(pulTotalSectors);
    DclAssert(puSides);
    DclAssert(puSpt);
    DclAssert(puCyls);

    /*  The calculations below require a disk to have at least 2 sectors
    */
    DclAssert(*pulTotalSectors >= 2);

    /*  Determine our default values
    */
    uSides = 2;
    uSecPerTrack = 16;

    /*  This doesn't happen much, but allow for <really> small disks
    */
    while(((D_UINT32)(uSides * uSecPerTrack) > *pulTotalSectors) && (uSecPerTrack >= 2))
        uSecPerTrack /= 2;

    ulCylinders = *pulTotalSectors / uSides / uSecPerTrack;

    if(!ulCylinders)
        return FALSE;

    /*  NOTE!  If the disk is too large to be represented in the MBR or
        BPB using the CHS fields, return CHS values as the MBR tuple
        'sentinal values', and use use the value *pulTotalSectors as-is.

        Generally, callers should use the *pulTotalSectors value exclusively,
        or better, not call this function at all.  Callers that use the
        CHS values are limited by the MBR data structures to ~8GB.
    */
    if (*pulTotalSectors > MAX_TRACKS * MAX_SIDES * MAX_SPT)
    {
        *puSpt      = MAX_SPT;
        *puSides    = MAX_SIDES;
        *puCyls     = MAX_TRACKS;
        return TRUE;
    }

    /*  Get the CHS values down to reasonable values

        The conditions below prevent a system hang-up
        when have a large number of sectors, e.g. 4MB.
    */
    while((ulCylinders > MAX_TRACKS)
          && !((uSides >= MAX_SIDES) && (uSecPerTrack >= MAX_SPT)))
    {
        /*  If we can, up the number of sides!
        */
        if(uSides < MAX_SIDES)
        {
            uSides += 2;
            ulCylinders = *pulTotalSectors / uSides / uSecPerTrack;

            /*  Quit if the number of cylinders is to a good point
            */
            if(ulCylinders < MAX_TRACKS)
                break;
        }

        /*  If we can, up the number of sectors per track!
        */
        if(uSecPerTrack < MAX_SPT)
        {
            uSecPerTrack += 16;

            /*  Changed MAX_SPT from 64 to 63 for > 190MB disks
            */
            if(uSecPerTrack > MAX_SPT)
                uSecPerTrack = MAX_SPT;

            ulCylinders = *pulTotalSectors / uSides / uSecPerTrack;

            /*  Quit if the number of cylinders is to a good point
            */
            if(ulCylinders < MAX_TRACKS)
                break;
        }
    }

    DclAssert(uSecPerTrack <= MAX_SPT);
    DclAssert(uSides <= MAX_SIDES);
    DclAssert(ulCylinders);
    DclAssert(ulCylinders <= MAX_TRACKS);

    /*  Save the new values
    */
    *puSpt = uSecPerTrack;
    *puSides = uSides;
    *puCyls = (D_UINT16) ulCylinders;     /* GJS removed -1 on 4/4/01 */

    /*  The number of cylinders is calculated from the above and
        the total number of sectors in the partition.
    */
    *pulTotalSectors = (D_UINT32)uSides * (D_UINT32)ulCylinders * (D_UINT32)uSecPerTrack;

    /*  We had better not have more sectors than we originally had!
    */

    /*  GJS changes assert from <= to == on 4/4/01
    */
    DclAssert(*pulTotalSectors ==
        ((D_UINT32)uSides * (D_UINT32)ulCylinders * (D_UINT32)uSecPerTrack));

    return TRUE;
}
