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

    This module contains the Ioctl functions to dispatch generic DCL
    requests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dldisp.c $
    Revision 1.9  2010/11/23 16:39:16Z  garyp
    Made assert mode processing more flexible.
    Revision 1.8  2010/07/15 01:06:31Z  garyp
    Added functionality to reset the profiler data.
    Revision 1.7  2010/04/18 21:24:59Z  garyp
    Added support for DCLIOFUNC_SERVICE_REQUEST and DCLIOFUNC_SYSTEM_INFO.
    Revision 1.6  2009/11/08 16:21:06Z  garyp
    Updated to support the profiler toggle command.
    Revision 1.5  2009/10/19 22:44:21Z  garyp
    Properly conditioned some code to build cleanly with output disabled.
    Revision 1.4  2009/10/18 00:41:44Z  garyp
    Added a dispatcher for DCLIOFUNC_TRACE_TOGGLE.
    Revision 1.3  2009/06/28 01:01:44Z  garyp
    Added support for DCLIOFUNC_MEMTRACK_POOLDUMP.
    Revision 1.2  2009/02/11 02:06:52Z  garyp
    Updated to deal with the MKS branch first conundrum.
    Revision 1.1  2009/01/10 02:44:36Z  garyp
    Initial revision
 ---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlservice.h>


/*-------------------------------------------------------------------
    Public: DclIoctlDispatch()

    Dispatch generic IOCTL requests which came from outside DCL.

    Parameters:
        pReqHdr - A pointer to the request block for the function.

    Return Value:
        Returns an DCLSTATUS value indicating the status of the
        request.  Note that this status value indicates whether the
        IOCTL request and interface are working properly, NOT whether
        the actual function call was successful.  If this function
        returns DCLSTAT_SUCCESS, then the returned request structure
        must be examined to see if the actual API function was
        successful.
-------------------------------------------------------------------*/
DCLSTATUS DclIoctlDispatch(
    DCLIOREQUEST   *pReqHdr)
{
    if(!pReqHdr)
    {
        DclError();
        return DCLSTAT_BADPARAMETER;
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 3, 0),
        "DclIoctlDispatch() Function 0x%x\n", pReqHdr->ioFunc));

    switch(pReqHdr->ioFunc)
    {
                    /*--------------------------------*\
                     *                                *
                     *            General             *
                     *                                *
                    \*--------------------------------*/

        case DCLIOFUNC_SYSTEM_INFO:
        {
            DCLREQ_SYSTEM_INFO  *pReq = (DCLREQ_SYSTEM_INFO*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_SYSTEM_INFO\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            pReq->dclStat = DclSystemInfoQuery(&pReq->SysInfo);
 
            return DCLSTAT_SUCCESS;
        }

                    /*--------------------------------*\
                     *                                *
                     *            Stats               *
                     *                                *
                    \*--------------------------------*/

        case DCLIOFUNC_STATS_RESET:
        {
            DCLREQ_STATSRESET  *pReq = (DCLREQ_STATSRESET*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_STATS_RESET\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            /*  Nothing to do at this time...
            */

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_STATS_REQUEST:
        {
          #if DCLCONF_OUTPUT_ENABLED
            DCLREQ_STATS       *pReq = (DCLREQ_STATS*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_STATS_REQUEST\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            /*  Nothing to do at this time...
            */

            return DCLSTAT_FEATUREDISABLED;
          #else
            return DCLSTAT_FEATUREDISABLED;
          #endif
        }

                    /*--------------------------------*\
                     *                                *
                     *            Profiler            *
                     *                                *
                    \*--------------------------------*/

        case DCLIOFUNC_PROFILER_ENABLE:
        {
            DCLREQ_PROFILER_ENABLE  *pReq = (DCLREQ_PROFILER_ENABLE*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_PROFILER_ENABLE\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            pReq->dclStat = DclProfEnable();

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_PROFILER_DISABLE:
        {
            DCLREQ_PROFILER_DISABLE *pReq = (DCLREQ_PROFILER_DISABLE*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_PROFILER_DISABLE\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            pReq->dclStat = DclProfDisable();

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_PROFILER_RESET:
        {
            DCLREQ_PROFILER_RESET *pReq = (DCLREQ_PROFILER_RESET*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_PROFILER_RESET\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            pReq->dclStat = DclProfReset();

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_PROFILER_SUMMARY:
        {
            DCLREQ_PROFILER_SUMMARY *pReq = (DCLREQ_PROFILER_SUMMARY*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_PROFILER_SUMMARY\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            pReq->dclStat = DclProfSummary(pReq->fReset, pReq->fShort, pReq->fAdjustOverhead);

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_PROFILER_TOGGLE:
        {
            DCLREQ_PROFILER_TOGGLE  *pReq = (DCLREQ_PROFILER_TOGGLE*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_PROFILER_TOGGLE\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            pReq->dclStat = DclProfToggle(pReq->fSummaryReset);

            return DCLSTAT_SUCCESS;
        }

                    /*--------------------------------*\
                     *                                *
                     *           Debug Tracing        *
                     *                                *
                    \*--------------------------------*/

        case DCLIOFUNC_TRACE_ENABLE:
        {
            DCLREQ_TRACEENABLE *pReq = (DCLREQ_TRACEENABLE*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_TRACE_ENABLE\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, pReq->nLevel, TRACEFORCEON), "Global Trace Started\n"));

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_TRACE_DISABLE:
        {
            DCLREQ_TRACEDISABLE *pReq = (DCLREQ_TRACEDISABLE*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_TRACE_DISABLE\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEOFF), "Global Trace Stopped\n"));

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_TRACE_TOGGLE:
        {
            DCLREQ_TRACETOGGLE *pReq = (DCLREQ_TRACETOGGLE*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_TRACE_TOGGLE\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

          #if D_DEBUG && DCLCONF_OUTPUT_ENABLED
            DclTraceToggle();
          #endif

            return DCLSTAT_SUCCESS;
        }

                    /*--------------------------------*\
                     *                                *
                     *          Miscellaneous         *
                     *                                *
                    \*--------------------------------*/

        case DCLIOFUNC_SERVICE_REQUEST:
        {
            DCLREQ_SERVICE_REQUEST *pReq = (DCLREQ_SERVICE_REQUEST*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_SERVICE_REQUEST\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            pReq->dclStat = DclServiceIoctl(NULL, pReq->nServiceType, pReq->pSubReq);

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_MEMTRACK_POOLDUMP:
        {
            DCLREQ_MEMTRACK_POOLDUMP   *pReq = (DCLREQ_MEMTRACK_POOLDUMP*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_MEMTRACK_POOLDUMP\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            /*  Pass the request right on down the stack...
            */
            return DclServiceIoctl(NULL, DCLSERVICE_MEMTRACK, &pReq->ior);
        }

        case DCLIOFUNC_ASSERT_MODE:
        {
            DCLREQ_ASSERT_MODE   *pReq = (DCLREQ_ASSERT_MODE*)pReqHdr;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() DCLIOFUNC_ASSERT_MODE\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            /*  Pass the request right on down the stack...
            */
            return DclServiceIoctl(NULL, DCLSERVICE_ASSERT, &pReq->ior);
        }

        default:
        {
            /*  If this is an unhandled DCL request use "TRACEALWAYS", 
                otherwise use DCLTRACE_EXTAPI since it is just another
                product's requests which are passing through.
            */                
            DCLTRACEPRINTF((
                MAKETRACEFLAGS((DCLIO_GETPRODUCT(pReqHdr->ioFunc) == PRODUCTNUM_DCL) ? TRACEALWAYS : DCLTRACE_EXTAPI, 1, 0),
                "DclIoctlDispatch() Function 0x%x unknown\n", pReqHdr->ioFunc));

            return DCLSTAT_UNSUPPORTEDFUNCTION;
        }
    }
}




