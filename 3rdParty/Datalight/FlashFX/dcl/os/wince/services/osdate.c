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
  jurisdictions.  Patents may be pending.

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

    This module contains the OS Services default implementation for
    manipulating date and time.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osdate.c $
    Revision 1.7  2009/06/25 01:57:19Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2009/04/10 02:01:31Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.5  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/08/18 19:39:36Z  garyp
    Minor header and documentation updates.
    Revision 1.3  2007/03/15 22:30:39Z  Garyp
    Factored out a helper function for use by the file services.
    Revision 1.2  2006/02/12 20:21:30Z  Garyp
    Minor fixes to make it work.
    Revision 1.1  2006/02/08 01:09:40Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlwindows.h>
#include "osdate.h"


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
    SYSTEMTIME      sSysTime;
    FILETIME        sFileTime;

    if(!pTime)
    {
        DclError();
        return FALSE;
    }

    /*  Get the system time.  Convert it to file time.
    */
    GetSystemTime(&sSysTime);
    SystemTimeToFileTime(&sSysTime, &sFileTime);

    DclOsFileTimeToDclTime(pTime, &sFileTime);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclOsFileTimeToDclTime()

    Convert the OS FILETIME to DCL time.

    Parameters:
        pTime     - A buffer to receive the date/time
        pFileTime - A pointer to the FILETIME structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclOsFileTimeToDclTime(
    D_TIME         *pTime,
    FILETIME       *pFileTime)
{
    ULARGE_INTEGER  ullFileTime;

    DclAssert(pTime);
    DclAssert(pFileTime);

    /*  Place the file time in a union so that we can access it as one
        64-bit value.
    */
    ullFileTime.LowPart = pFileTime->dwLowDateTime;
    ullFileTime.HighPart = pFileTime->dwHighDateTime;

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

    return;
}


