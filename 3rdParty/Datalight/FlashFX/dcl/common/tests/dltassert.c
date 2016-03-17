/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains code to test the assert functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltassert.c $
    Revision 1.4  2011/08/16 19:24:22Z  daniel.lewis
    Eliminated use of the non-standard __FUNCTION__ macro.
    Revision 1.3  2011/03/02 02:18:27Z  garyp
    Documentation corrections -- no functional changes.
    Revision 1.2  2009/11/18 01:41:44Z  garyp
    Added profiler instrumentation to allow verification that an assert can
    dump the call stack.
    Revision 1.1  2009/06/24 20:33:02Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlver.h>
#include <dlservice.h>
#include <dlinstance.h>
#include "dltests.h"

static DCLSTATUS TestAssertIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPriv);


/*-------------------------------------------------------------------
    Protected: DclTestAssert()

    Test assert functionality.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclTestAssert(void)
{
    DCLSTATUS       dclStat;
    DCLASSERTMODE   nOldMode;

    DclProfilerEnter("DclTestAssert", 0, 0);

    DclPrintf("Testing Assert Functionality...\n");

    DclPrintf("  Testing Assert in \"ignore\" mode...\n");

    nOldMode = DclAssertMode(0, DCLASSERTMODE_IGNORE);

    /*  Fire off a few of these while in "ignore" mode...
    */
    DclProductionError();
    DclProductionError();
    DclProductionError();

    DclPrintf("  Testing Assert in \"warn\" mode...\n");

    DclAssertMode(0, DCLASSERTMODE_WARN);

    /*  Fire off a few of these while in "warn" mode...
    */
    DclProductionError();
    DclProductionError();
    DclProductionError();

    /*  Restore the original mode
    */
    DclAssertMode(0, nOldMode);

    DclPrintf("  Testing using a private Assert function...\n");

    {
        unsigned            nFired = 0;
        DCLDECLARESERVICE   (srv, "TESTASSERT", DCLSERVICE_ASSERT, TestAssertIoctl, NULL, DCLSERVICEFLAG_THREADSAFE);

        srv.pPrivateData = &nFired;

        dclStat = DclServiceCreate(0, &srv);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;

        DclProductionError();
        DclProductionError();
        DclProductionError();

        if(nFired != 3)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }

        dclStat = DclServiceDestroy(&srv);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;
    }

    DclPrintf("  Testing Assert in \"ignore\" mode...\n");

    nOldMode = DclAssertMode(0, DCLASSERTMODE_IGNORE);

    /*  Fire off a few of these while in "ignore" mode...
    */
    DclProductionError();
    DclProductionError();
    DclProductionError();

    /*  Restore the original mode
    */
    DclAssertMode(0, nOldMode);

  Cleanup:

    DclProfilerLeave(0);

    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");
    else
        DclPrintf("    FAILED\n");

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestAssertIoctl()

    Test control of assert behavior via the IOCTL interface.

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data to pass

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS TestAssertIoctl(
    DCLSERVICEHANDLE        hService,
    DCLREQ_SERVICE         *pReq,
    void                   *pPriv)
{
    unsigned               *pnCounter = pPriv;

    DclAssert(hService);
    DclAssert(pReq);
    DclAssert(pnCounter);

    if(pReq->ior.ulReqLen != sizeof(*pReq))
        return DCLSTAT_BADSTRUCLEN;

    switch(pReq->ior.ioFunc)
    {
        case DCLIOFUNC_SERVICE_CREATE:
        case DCLIOFUNC_SERVICE_DESTROY:
        case DCLIOFUNC_SERVICE_REGISTER:
        case DCLIOFUNC_SERVICE_DEREGISTER:
            DclAssert(pReq->pSubRequest == NULL);

            /*  Nothing to do
            */
            return DCLSTAT_SUCCESS;

        case DCLIOFUNC_SERVICE_DISPATCH:
        {
            if(pReq->pSubRequest == NULL)
                return DCLSTAT_SERVICE_BADSUBREQUEST;

            if(pReq->pSubRequest->ulReqLen < sizeof(pReq->pSubRequest))
                return DCLSTAT_BADSTRUCLEN;

            switch(pReq->pSubRequest->ioFunc)
            {
                case DCLIOFUNC_ASSERT_FIRED:
                {
                    DCLREQ_ASSERT_FIRED *pSubReq = (DCLREQ_ASSERT_FIRED*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    DclPrintf("TestAssert fired in \"%s\" at line %u\n", pSubReq->pszModuleName, pSubReq->nLineNumber);

                    (*pnCounter)++;

                    return DCLSTAT_SUCCESS;
                }

                default:
                    DCLPRINTF(1, ("TestAssert:ServiceIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        default:
            DCLPRINTF(1, ("TestAssert:ServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}


