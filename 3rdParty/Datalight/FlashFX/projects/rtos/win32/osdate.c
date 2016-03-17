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

    This module contains a project specific implementation for the date and
    time OS Services.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osdate.c $
    Revision 1.4  2009/04/10 02:21:00Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.3  2007/11/03 23:50:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/08/18 19:39:35Z  garyp
    Minor header and documentation updates.
    Revision 1.1  2007/02/18 17:31:38Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <time.h>

#include <dcl.h>
#include <dlwindows.h>


/*-------------------------------------------------------------------
    Public: DclOsGetDateTime()

    Get the current GMT date and time encoded as the number of
    microseconds since 1-1-1970.

    Parameters:
        pTime - A pointer to a D_TIME variable to receive the result.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclOsGetDateTime(
    D_TIME         *pTime)
{
    FILETIME        sFileTime;
    ULARGE_INTEGER  ullFileTime;

    if(!pTime)
    {
        DclError();
        return FALSE;
    }

    /*  Get the system time.
    */
    GetSystemTimeAsFileTime(&sFileTime);

    /*  Place the file time in a union so that we can access it as one
        64-bit value.
    */
    ullFileTime.LowPart = sFileTime.dwLowDateTime;
    ullFileTime.HighPart = sFileTime.dwHighDateTime;

    /*  Account for the difference in base-points for the timestamps.
        Windows file time starts at January 1, 1601.
        DCL time starts at January 1, 1970.
    */
    ullFileTime.QuadPart -= DCLTIME_1601_TO_1970_DAYS * DCLTIME_100NANOS_PER_DAY;

    /*  Translate from 100/nanoseconds to microseconds.
    */
    ullFileTime.QuadPart /= 10;

    /*  All done.
    */
    DclUint64AssignUint64(pTime, (D_TIME*)&ullFileTime);

    return TRUE;
}

