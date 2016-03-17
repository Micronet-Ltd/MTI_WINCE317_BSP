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

    This module contains code for testing the memory validation subsystem.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltmemval.c $
    Revision 1.1  2009/06/27 19:53:34Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include <dlservice.h>
#include "dltests.h"

#if DCLCONF_MEMORYVALIDATION

typedef struct
{
    DCLSTATUS   dclStat;
} TESTMEMVALDATA;

static DCLSTATUS MemValIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPriv);
static DCLSTATUS VerifyAutomaticSummary(DCLMEMVALSUMMARY *pOriginal, unsigned nExtraBlocks, D_UINT32 ulExtraBytes);
static DCLSTATUS VerifyExplicitSummary( DCLMEMVALSUMMARY *pOriginal, unsigned nExtraBlocks, D_UINT32 ulExtraBytes);

#endif


/*-------------------------------------------------------------------
    DclTestMemoryValidation()

    Parameters:
        None

    Returns:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestMemoryValidation(void)
{
  #if DCLCONF_MEMORYVALIDATION
    DCLSTATUS           dclStat;
    static D_BUFFER     abTestBuff1[32];
    TESTMEMVALDATA      TestData;
    DCLDECLARESERVICE   (memval, "DLTESTMEMVAL", DCLSERVICE_MEMVALIDATION, MemValIoctl, NULL, 0);
    DCLMEMVALSUMMARY    BaselineSummary;

    DclPrintf("Testing Memory Validation...\n");

    memval.pPrivateData = &TestData;

    dclStat = DclServiceCreate(NULL, &memval);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValSummary;

    /*  The following functions emulate what would be done by
        DclAssertReadPtr() to add automatic buffers.  We do this
        explicitly since the assert would not generate any code
        in release mode.
    */
    DclPrintf("  Testing Writing to an Explicit ReadOnly Buffer...\n");

    /*  Gather baseline summary information
    */
    dclStat = DclMemValSummary(NULL, &BaselineSummary, TRUE, FALSE);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add a buffer marked as read-only
    */
    dclStat = DclMemValBufferAdd(NULL, &abTestBuff1[0], sizeof(abTestBuff1), DCL_MVFLAG_READONLY|DCL_MVFLAG_TYPEEXPLICIT);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyExplicitSummary(&BaselineSummary, 1, sizeof(abTestBuff1));
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Try to validate write access to it
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[0], sizeof(abTestBuff1), 0);
    if(dclStat != DCLSTAT_MEMVAL_READONLY)
        goto MemValCleanup;

    /*  Ensure that our local handler saw the request and recorded the result
    */
    if(TestData.dclStat != dclStat)
    {
        dclStat = DCLSTAT_CURRENTLINE;
        goto MemValCleanup;
    }

    dclStat = DclMemValBufferRemove(NULL, &abTestBuff1[0]);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyExplicitSummary(&BaselineSummary, 0, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;



    DclPrintf("  Testing Automatic Buffers...\n");

    /*  Gather baseline summary information
    */
    dclStat = DclMemValSummary(NULL, &BaselineSummary, TRUE, FALSE);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic towards the end of abTestBuff1[]
            01234567890123456789012345678901
            ----------------------------xx--
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[28], 2, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 1, 2);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic for the 3 bytes preceding the entry just added,
        which should cause them to combin.
            01234567890123456789012345678901
            -------------------------xxxxx--
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[25], 3, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 1, 5);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic which partially overlaps the beginning of
        the next buffer.
            01234567890123456789012345678901
            ------------------------xxxxxx--
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[24], 3, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 1, 6);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic which completely overlaps the beginning of
        the next buffer.
            01234567890123456789012345678901
            ----------------------xxxxxxxxxx
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[22], 10, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 1, 10);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic for the 10 bytes at the beginning of abTestBuff1[]
            01234567890123456789012345678901
            xxxxxxxxxx------------xxxxxxxxxx
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[0], 10, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 2, 20);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic for the 5 bytes following that
            01234567890123456789012345678901
            xxxxxxxxxxxxxxx-------xxxxxxxxxx
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[10], 5, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 2, 25);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic which overlaps the tail end of the previous
        entry.
            01234567890123456789012345678901
            xxxxxxxxxxxxxxxxx-----xxxxxxxxxx
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[13], 4, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 2, 27);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  Add an automatic which causes everything to coalesce back into
        one group.
            01234567890123456789012345678901
            xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    */
    dclStat = DclMemValBufferValidate(NULL, &abTestBuff1[17], 5, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyAutomaticSummary(&BaselineSummary, 1, 32);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    /*  In normal operation, automatics are never removed, however we do
        so here so we can run this test a second time.
    */
/*  dclStat = DclMemValBufferRemove(NULL, &abTestBuff1[0]);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;

    dclStat = VerifyExplicitSummary(&BaselineSummary, 0, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MemValCleanup;
*/
    dclStat = DCLSTAT_SUCCESS;

  MemValCleanup:

    /*  Clean up -- making sure we preserve any original status code
    */
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = DclServiceDestroy(&memval);
    else
        DclServiceDestroy(&memval);

  MemValSummary:

    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");
    else
        DclPrintf("    FAILED\n");

    return dclStat;

  #else

    DclPrintf("Memory Validation Service is disabled\n");

    return DCLSTAT_SUCCESS;

  #endif
}


#if DCLCONF_MEMORYVALIDATION

/*-------------------------------------------------------------------
    Local: MemValIoctl()

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data to pass

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS MemValIoctl(
    DCLSERVICEHANDLE    hService,
    DCLREQ_SERVICE     *pReq,
    void               *pPriv)
{
    TESTMEMVALDATA     *pMVD = pPriv;
    DCLSTATUS           dclStat;

    DclAssert(hService);
    DclAssert(pReq);

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
                case DCLIOFUNC_MEMVAL_BUFFERVALIDATE:
                {
                    dclStat = DclServiceIoctlPrevious(hService, pReq);

                    /*  Stuff only the last non-success status code into
                        the private data structure.
                    */
                    if(dclStat != DCLSTAT_SUCCESS)
                        pMVD->dclStat = dclStat;

                    return dclStat;
                }

                default:
                    /*  Everything else, besides the cases explicitly handled above,
                        falls out and is processed here by the previous handler.
                    */
                    return DclServiceIoctlPrevious(hService, pReq);
            }
        }

        default:
            DCLPRINTF(1, ("TestMemVal:ServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}


/*-------------------------------------------------------------------
    Local: VerifyExplicitSummary()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS VerifyExplicitSummary(
    DCLMEMVALSUMMARY   *pOriginal,
    unsigned            nExtraBlocks,
    D_UINT32            ulExtraBytes)
{
    DCLMEMVALSUMMARY    Summary;
    DCLSTATUS           dclStat;

    DclAssertWritePtr(pOriginal, sizeof(*pOriginal));

    dclStat = DclMemValSummary(NULL, &Summary, TRUE, FALSE);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Verify we have the expected explicit block count
    */
    if(Summary.nExplicitBlocks != pOriginal->nExplicitBlocks + nExtraBlocks)
    {
        DclPrintf("VerifyExplicitSummary(A) failed, OldBlocks=%u NewBlocks=%u BlockAdjust=%u, OldBytes=%lU NewBytes=%lU ByteAdjust=%lU\n",
            pOriginal->nExplicitBlocks, Summary.nExplicitBlocks, nExtraBlocks,
            pOriginal->ulExplicitBytes, Summary.ulExplicitBytes, ulExtraBytes);

        return DCLSTAT_CURRENTLINE;
    }

    /*  Verify we have the expected explicit byte count
    */
    if(Summary.ulExplicitBytes != pOriginal->ulExplicitBytes + ulExtraBytes)
    {
        DclPrintf("VerifyExplicitSummary(B) failed, OldBlocks=%u NewBlocks=%u BlockAdjust=%u, OldBytes=%lU NewBytes=%lU ByteAdjust=%lU\n",
            pOriginal->nExplicitBlocks, Summary.nExplicitBlocks, nExtraBlocks,
            pOriginal->ulExplicitBytes, Summary.ulExplicitBytes, ulExtraBytes);

        return DCLSTAT_CURRENTLINE;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: VerifyAutomaticSummary()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS VerifyAutomaticSummary(
    DCLMEMVALSUMMARY   *pOriginal,
    unsigned            nExtraBlocks,
    D_UINT32            ulExtraBytes)
{
    DCLMEMVALSUMMARY    Summary;
    DCLSTATUS           dclStat;

    DclAssertWritePtr(pOriginal, sizeof(*pOriginal));

    dclStat = DclMemValSummary(NULL, &Summary, TRUE, FALSE);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Verify we have the expected automatic block count
    */
    if(Summary.nAutomaticBlocks != pOriginal->nAutomaticBlocks + nExtraBlocks)
    {
        DclPrintf("VerifyAutomaticSummary(A) failed, OldBlocks=%u NewBlocks=%u BlockAdjust=%u, OldBytes=%lU NewBytes=%lU ByteAdjust=%lU\n",
            pOriginal->nAutomaticBlocks, Summary.nAutomaticBlocks, nExtraBlocks,
            pOriginal->ulAutomaticBytes, Summary.ulAutomaticBytes, ulExtraBytes);

        return DCLSTAT_CURRENTLINE;
    }

    /*  Verify we have the expected automatic byte count
    */
    if(Summary.ulAutomaticBytes != pOriginal->ulAutomaticBytes + ulExtraBytes)
    {
        DclPrintf("VerifyAutomaticSummary(B) failed, OldBlocks=%u NewBlocks=%u BlockAdjust=%u, OldBytes=%lU NewBytes=%lU ByteAdjust=%lU\n",
            pOriginal->nAutomaticBlocks, Summary.nAutomaticBlocks, nExtraBlocks,
            pOriginal->ulAutomaticBytes, Summary.ulAutomaticBytes, ulExtraBytes);

        return DCLSTAT_CURRENTLINE;
    }

    return dclStat;
}


#endif  /* DCLCONF_MEMORYVALIDATION */



