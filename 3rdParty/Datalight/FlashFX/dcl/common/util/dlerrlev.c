/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    This module contains logic to map DCLSTATUS values onto errorlevel codes
    for environemnts which support this.

    This code is designed to allow mapping of a subset of the DCLSTAT_*
    values onto an integer errorlevel value in the range of 0 to 255.

    While the latest XP command-line extensions support a wider range of
    errorlevels, including negative numbers, we avoid this for now.  This
    is because a large number of automated processes expect that errorlevel
    0 means success, and a higher errorlevel, up to 255 means failure of
    some sort.

    Generally these should [ONLY] be used to return errorlevel values
    for DOS/XP/WinCE utilities which use errorlevels.  Therefore this
    header should/must be manually included in DOS/XP/CE specific code
    (or perhaps other code which makes explicit use of errorlevels).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlerrlev.c $
    Revision 1.3  2007/12/18 04:04:21Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/08/15 19:40:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlerrlev.h>


/*-------------------------------------------------------------------
    Public: DclStatusToErrorlevel

    This function converts a DCLSTATUS value to an errorlevel
    value in the range of 0 to 255.

    This function should only be used for DCLSTATUS values
    where the product number is DCL, the status class is
    "Error", and the status locale is "General".  Additionally
    the status value must be 255 or less.  If any of these
    conditions are NOT met, an errorlevel value of 255 will
    be returned.

    (One exception is that the *STAT_SUCCESS and *STAT_FAILURE
    codes are identical regardless of the product code, and will
    properly return DCLERRORLEVEL_SUCCESS/FAILURE respectively.)

    Parameters:
        dclStat - The DCLSTAT_* value to convert

    Return:
        Returns an errorlevel value in the range of 0 to 255.
-------------------------------------------------------------------*/
int DclStatusToErrorlevel(
    DCLSTATUS   dclStat)
{
    /*  Reality checks that our constants are correct
    */
    DclAssert(DCLERRORLEVEL_SUCCESS == 0);
    DclAssert(DCLERRORLEVEL_FAILURE == 255);

    switch(dclStat)
    {
        case DCLSTAT_SUCCESS:
            /*  (Includes *STAT_SUCCESS as well)
            */
            return DCLERRORLEVEL_SUCCESS;

        case DCLSTAT_FAILURE:
            /*  (Includes *STAT_FAILURE as well)
            */
            return DCLERRORLEVEL_FAILURE;

        case DCLSTAT_MEMALLOCFAILED:
            return DCLERRORLEVEL_OUTOFMEMORY;

        default:
            if((DCLSTATGETCODEBASE(dclStat) == PRODUCTNUM_DCL) &&
               (DCLSTATGETCLASS(dclStat) == DCLSTATUSCLASS_ERROR) &&
               (DCLSTATGETLOC(dclStat) == DCLLOCALE_GENERAL) &&
               (DCLSTATGETVAL(dclStat) <= 255))
            {
                return (int)(DCLSTATGETVAL(dclStat) & 255);
            }
            else
            {
                return DCLERRORLEVEL_FAILURE;
            }
     }
}
