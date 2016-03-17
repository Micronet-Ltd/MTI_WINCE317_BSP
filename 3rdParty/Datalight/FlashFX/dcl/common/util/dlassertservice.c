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

    This module holds the functions that implement assert and error handling.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlassertservice.c $
    Revision 1.5  2011/10/03 15:56:06Z  garyp
    Modified the assert handler to flush the log to ensure that everything
    is on the disk before the assert action is taken.
    Revision 1.4  2010/12/01 23:35:32Z  garyp
    Made assert mode processing more flexible.
    Revision 1.3  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.2  2009/11/09 23:31:17Z  garyp
    Updated to exclude more stuff when output is disabled.
    Revision 1.1  2009/06/27 20:05:42Z  garyp
    Initial revision
    Revision 1.12.1.5  2009/03/07 16:19:39Z  garyp
    Properly condition the call to dump the BackTrace data.
    Revision 1.12.1.4  2009/03/05 15:32:02Z  garyp
    Added protection against being re-entered -- either recursively or by a
    different thread.  Updated to dump the BackTrace Log, if any.
    Revision 1.12.1.3  2009/01/10 02:41:04Z  garyp
    Updated to destroy all services before halting.  Added a "return"
    instruction outside the normal code path to more easily allow someone
    in a debugger to continue execution past the assert.
    Revision 1.12.1.2  2008/08/25 20:50:20Z  garyp
    Modified to attemp a regular debug command query if the "auto-query" fails.
    Revision 1.12  2007/12/18 04:21:34Z  brandont
    Updated function headers.
    Revision 1.11  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.10  2007/10/06 01:38:16Z  brandont
    Switched to using a function pointer for DclLogClose to prevent
    a link dependency with the logging module.
    Revision 1.9  2007/08/21 22:00:55Z  Garyp
    Added a "v" to the version number.
    Revision 1.8  2007/08/19 16:47:50Z  garyp
    Updated to use a segmented, three-part version number.
    Revision 1.7  2007/04/03 03:02:02Z  Garyp
    Modified to use the new DclOsDebugBreak() functionality.
    Revision 1.6  2007/01/19 04:22:06Z  Garyp
    Modified to display the thread ID rather than the DCL build number.
    Revision 1.5  2006/10/06 00:07:42Z  Garyp
    Updated to allow output to be entirely disabled (and the code not pulled
    into the image).
    Revision 1.4  2006/09/30 01:46:34Z  brandont
    Corrected warning message for unreachable return statement.
    Revision 1.3  2006/09/16 20:44:16Z  Garyp
    Modified to accommodate build numbers which are now strings rather than
    numbers.
    Revision 1.2  2006/02/16 02:30:10Z  Garyp
    Added a 1 second sleep before halting the system.
    Revision 1.1  2005/12/07 22:33:32Z  Pauli
    Initial revision
    Revision 1.4  2005/12/07 22:33:32Z  Garyp
    Fixed some bogus comments.
    Revision 1.3  2005/12/01 01:40:32Z  Pauli
    Merge with 2.0 product line, build 173.
    Revision 1.2  2005/10/07 04:02:59Z  Garyp
    Removed some dead code.
    Revision 1.1  2005/10/03 13:44:44Z  Garyp
    Initial revision
    Revision 1.3  2005/08/15 07:19:39Z  garyp
    Updated to display the product version number.
    Revision 1.2  2005/08/03 19:17:46Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/19 17:56:36Z  pauli
    Initial revision
    Revision 1.3  2005/06/16 14:44:24Z  PaulI
    Updated assert message.
    Revision 1.2  2005/06/13 19:44:31Z  PaulI
    Modified to pass the module name to DCL_OsHalt().
    Revision 1.1  2005/04/10 01:51:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlver.h>
#include <dlservice.h>
#include <dlinstance.h>
#include <dllog.h>
#include <dltrace.h>


typedef struct
{
    DCLINSTANCEHANDLE   hDclInst;
    D_ATOMIC32          ulEntryCount;
    DCLASSERTMODE       nAssertMode;
} ASSERTSERVICEDATA;

static DCLSTATUS    ServiceIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPriv);
static void         ProcessAssert(ASSERTSERVICEDATA *pASD, const char *pszModuleName, unsigned nLineNumber);


/*-------------------------------------------------------------------
    Protected: DclAssertServiceInit()

    Initialize the assert service.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclAssertServiceInit(
    DCLINSTANCEHANDLE           hDclInst)
{
    static DCLSERVICEHEADER     SrvAssert[DCL_MAX_INSTANCES];
    static ASSERTSERVICEDATA    ASD[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER           *pSrv;
    DCLSTATUS                   dclStat;
    unsigned                    nInstNum;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    if(!hDclInst)
        return DCLSTAT_SERVICE_BADHANDLE;

    nInstNum = DclInstanceNumber(hDclInst);
    DclMemSet(&ASD[nInstNum], 0, sizeof(ASD[0]));

    ASD[nInstNum].hDclInst = hDclInst;
    ASD[nInstNum].nAssertMode = DCLASSERTMODE_AUTODEBUGGERFAIL;

    pSrv = &SrvAssert[nInstNum];
    dclStat = DclServiceHeaderInit(&pSrv, "DLASSERT", DCLSERVICE_ASSERT, ServiceIoctl, &ASD[nInstNum], DCLSERVICEFLAG_THREADSAFE);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        dclStat = DclServiceCreate(hDclInst, pSrv);
    }

    return dclStat;
}




                /*---------------------------------*\
                 *                                 *
                 *     Static Local Functions      *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Local: ServiceIoctl()

    Process an assert service IOCTL request.

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data to pass

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS ServiceIoctl(
    DCLSERVICEHANDLE    hService,
    DCLREQ_SERVICE     *pReq,
    void               *pPriv)
{
    ASSERTSERVICEDATA  *pASD = pPriv;

    DclAssert(hService);
    DclAssert(pReq);
    DclAssert(pASD);

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

                    ProcessAssert(pASD, pSubReq->pszModuleName, pSubReq->nLineNumber);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_ASSERT_MODE:
                {
                    DCLREQ_ASSERT_MODE *pSubReq = (DCLREQ_ASSERT_MODE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    if(pSubReq->nNewMode == DCLASSERTMODE_QUERY)
                    {
                        pSubReq->nOldMode = pASD->nAssertMode;
                    }
                    else
                    {
                        if(pSubReq->nNewMode <= DCLASSERTMODE_LOWLIMIT || pSubReq->nNewMode >= DCLASSERTMODE_HIGHLIMIT)
                        {
                            DCLPRINTF(1, ("Assert:ServiceIoctl() Invalid assert mode %u\n", pSubReq->nNewMode));
                            pSubReq->nOldMode = DCLASSERTMODE_INVALID;
                        }
                        else
                        {
                            pSubReq->nOldMode = pASD->nAssertMode;
                            pASD->nAssertMode = pSubReq->nNewMode;
                        }
                    }
                    
                    return DCLSTAT_SUCCESS;
                }

                default:
                    DCLPRINTF(1, ("Assert:ServiceIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        default:
            DCLPRINTF(1, ("Assert:ServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}


/*-------------------------------------------------------------------
    Local: ProcessAssert()

    Process an assert.  This is the workhorse routine for DclAssert(),
    DclProductionAssert(), DclError(), and DclProductionError().

    Parameters:
        pASD          - The ASSERTSERVICEDATA structure to use.
        pszModuleName - A pointer to the null-terminated module name.
        nLineNumber   - Line number in the module that the assert
                        fired.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ProcessAssert(
    ASSERTSERVICEDATA  *pASD,
    const char         *pszModuleName,
    unsigned            nLineNumber)
{
    DCLSTATUS           dclStat;
  #if DCLCONF_OUTPUT_ENABLED
    char                achBuff[DCL_MAX_VERSION_LENGTH+1];
    DCLLOGMODE          nOldLogMode;
  #endif

    if(pASD->nAssertMode == DCLASSERTMODE_IGNORE)
        return;

  #if DCLCONF_OUTPUT_ENABLED
    /*  Ensure that log shadowing is turned on to allow the assert output
        to go to the screen (hopefully) rather than be buried exclusively
        in some log file.
    */
    nOldLogMode = DclLogMode(pASD->hDclInst, NULL, DCLLOGMODE_SHADOW);

    DclPrintf(D_PRODUCTNAME" v%s -- Thread ID %lX\n",
        DclVersionFormat(achBuff, sizeof(achBuff), D_PRODUCTVER, DCLSTRINGIZE(D_PRODUCTBUILD)),
        DclOsThreadID());

    DclPrintf("Assertion failure in module %s line %u\n", pszModuleName, nLineNumber);

  #if DCLCONF_PROFILERENABLED
    /*  If the profiler is enabled, we may have detailed call-tree
        information.  Display it if so.
    */
    DclProfDumpCallTree();
  #endif

    DclLogFlush(pASD->hDclInst, NULL);
  
  #endif

    if(pASD->nAssertMode == DCLASSERTMODE_WARN)
    {
      #if DCLCONF_OUTPUT_ENABLED
        /*  Restore the logging mode if necessary
        */
        if(nOldLogMode == DCLLOGMODE_NOSHADOW)
            DclLogMode(pASD->hDclInst, NULL, nOldLogMode);
      #endif

        /*  Note that if output is disabled the "WARN" assert
            mode is the same as the "IGNORE" assert mode.
        */            
        return;
    }

    /*  Attempt to atomically enter the assert code.  Note that we defined
        this service as "reentrant" so we need to handle any recursion or
        reentrancy explicitly.
    */
    if(!DclOsAtomic32SectionEnter(&pASD->ulEntryCount))
    {
        DclPrintf("RECURSIVE ASSERTS OR AN ASSERTION BY ANOTHER THREAD -- HALTING!\n");

      #if D_DEBUG && DCLTRACE_BACKTRACEBUFFERSIZE && DCLCONF_OUTPUT_ENABLED
        /*  If the BackTrace feature is enabled, dump the log.
        */
        DclTraceDumpBackTrace(UINT_MAX);
      #endif

        /*  Debatable as to whether we want to issue another halt or
            not at this point, since the first assert may have caused
            something like entering the debugger to take place, and
            we generally may not want that to happen again while we're
            still in the first instance.

        DclOsHalt(pszModuleName, nLineNumber);

            Skip it for now and sit in a sleep loop.
        */

        /*  The DclOsHalt() function should not return, but in case it does...
        */
        while(TRUE)
        {
            DclOsSleep(1);
        }
    }

  #if D_DEBUG && DCLTRACE_BACKTRACEBUFFERSIZE && DCLCONF_OUTPUT_ENABLED
    /*  If the BackTrace feature is enabled, dump the log.
    */
    DclTraceDumpBackTrace(UINT_MAX);
  #endif

    /*  Process the assert according to the nAssertMode setting,
        and the capabilities of the underlying OS.

        Some or none of these options may be viable in many RTOS
        implementations, in which case we will just fall through
        and do whatever DclOsHalt() does.
    */
    switch(pASD->nAssertMode)
    {
        case DCLASSERTMODE_AUTODEBUGGERFAIL:
            if(DclOsDebugBreak(DCLDBGCMD_CAPABILITIES|DCLDBGCMD_AUTOQUERY) == DCLSTAT_SUCCESS)
            {
                dclStat = DclOsDebugBreak(DCLDBGCMD_AUTOQUERY);
                break;
            }

            /* ... Fall through ...
            */

        case DCLASSERTMODE_QUERYDEBUGGERFAIL:
            if(DclOsDebugBreak(DCLDBGCMD_CAPABILITIES|DCLDBGCMD_QUERY) == DCLSTAT_SUCCESS)
            {
                dclStat = DclOsDebugBreak(DCLDBGCMD_QUERY);
                break;
            }

            /* ... Fall through ...
            */

        case DCLASSERTMODE_DEBUGGERFAIL:
            if(DclOsDebugBreak(DCLDBGCMD_CAPABILITIES|DCLDBGCMD_BREAK) == DCLSTAT_SUCCESS)
            {
                dclStat = DclOsDebugBreak(DCLDBGCMD_BREAK);
                break;
            }

            dclStat = DCLSTAT_FAILURE;
            break;

        default:
            if(pASD->nAssertMode != DCLASSERTMODE_FAIL)
            {
                /*  Ain't recursion-safe stuff wonderfull...
                */
                DclProductionError();
            }

            dclStat = DCLSTAT_FAILURE;
    }

        /*------------------------------------------------------*\
         *    DCLSTAT_CONTINUE is not normally ever returned    *
         *    by the functions above, however if you are in a   *
         *    debugger and want to continue executing, even     *
         *    past the assert condition, set the instruction    *
         *    pointer to the "goto AssertCleanup" statement.    *
        \*------------------------------------------------------*/

    if(dclStat == DCLSTAT_CONTINUE)
        goto AssertCleanup;

    /*  On some systems, the output messages can't keep up with reality, and
        we end up halting the system before the assert message has displayed.

        Sleep for a short while to give it a chance...
    */
    DclOsSleep(1000);

    /*  Destroy all services in reverse order of creation (that includes
        <this> service).
    */
    DclServiceDestroyAll(pASD->hDclInst, FALSE);

    /*  Tell the OS we want to die...

        Some OS environments such as REX and Windows XP will not be able to
        display the information processed above in all cases, but do have
        low level interfaces that can display information.  Therefore we
        pass this information on down in the event that the lower level
        code can do something useful with it.
    */
    DclOsHalt(pszModuleName, nLineNumber);

    /*  The DclOsHalt() function should not return, but in case it does...
    */
    while(TRUE)
    {
        DclOsSleep(1);
    }

    /*  Normally the only way we will ever get to this code is if someone
        interactively in a debugger reset the IP to allow the system to
        continue.
    */
  AssertCleanup:
    DclOsAtomic32SectionLeave(&pASD->ulEntryCount);

  #if DCLCONF_OUTPUT_ENABLED
    /*  Restore the logging mode if necessary
    */
    if(nOldLogMode == DCLLOGMODE_NOSHADOW)
        DclLogMode(pASD->hDclInst, NULL, nOldLogMode);
  #endif

    return;
}


