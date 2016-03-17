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
  jurisdictions.  Patents may be pending.

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

    This module holds the functions that implement assert and error handling.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlassertrequestor.c $
    Revision 1.3  2010/11/23 18:21:03Z  garyp
    Made assert mode processing more flexible.
    Revision 1.2  2010/04/28 23:31:29Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.1  2009/06/18 22:11:20Z  garyp
    Initial revision
 ---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlver.h>
#include <dlservice.h>


/*-------------------------------------------------------------------
    Protected: DclAssertFired()

    This is the workhorse routine for the DclAssert(),
    DclProductionAssert(), DclError(), and DclProductionError()
    macros.

    Parameters:
        hDclInst - The DCL instance handle.
        pszModuleName - A pointer to the null-terminated module name.
        nLineNumber   - Line number in the module that the assert
                        fired.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclAssertFired(
    DCLINSTANCEHANDLE       hDclInst,
    const char             *pszModuleName,
    unsigned                nLineNumber)
{
    DCLDECLAREREQUESTPACKET (ASSERT, FIRED, fired);  /* DCLREQ_ASSERT_FIRED */
    DCLSTATUS               dclStat;

    fired.pszModuleName = pszModuleName;
    fired.nLineNumber = nLineNumber;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_ASSERT, &fired.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclAssertFired() unexpected error %lX\n", dclStat));
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclAssertMode()

    Set the error mode for the Assert service, and return
    the original mode.

    Parameters:
        hDclInst - The DCL instance handle.
        nNewMode - The new DCLASSERTMODE_* mode value.

    Return Value:
        Returns an unsigned int containing the original error mode,
        or DCLASSERTMODE_INVALID if the call failed.
-------------------------------------------------------------------*/
DCLASSERTMODE DclAssertMode(
    DCLINSTANCEHANDLE       hDclInst,
    DCLASSERTMODE           nNewMode)
{
    DCLDECLAREREQUESTPACKET (ASSERT, MODE, mode);  /* DCLREQ_ASSERT_MODE */
    DCLSTATUS               dclStat;

    DclAssert((nNewMode > DCLASSERTMODE_LOWLIMIT && nNewMode < DCLASSERTMODE_HIGHLIMIT) ||
              (nNewMode == DCLASSERTMODE_QUERY));

    mode.nNewMode = nNewMode;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_ASSERT, &mode.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclAssertMode() unexpected error %lX\n", dclStat));
        }

        return DCLASSERTMODE_INVALID;
    }

    return mode.nOldMode;
}

