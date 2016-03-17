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

    This module implements functions for managing One-Time Programming (OTP).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlotp.c $
    Revision 1.2  2009/02/18 04:26:50Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.3  2009/02/18 04:26:50Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.2  2008/12/17 02:31:29Z  garyp
    Major update to support the concept of a Disk which spans multiple Devices.
    Revision 1.1  2008/12/13 19:34:18Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fxfmlapi.h>
#include "fml.h"

#if FFXCONF_OTPSUPPORT

/*-------------------------------------------------------------------
    Private: FfxFmlOtpReadPages()

    Parameters:
        hFML    - The FML handle
        pFmlReq - A pointer to the request structure to use

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlOtpReadPages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_READ_OTPPAGES       *pFmlReq)
{
    FFXIOSTATUS                     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlOtpReadPages() hFML=%P\n", hFML));

    DclProfilerEnter("FfxFmlOtpReadPages", 0, 0);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->nOTPPages) ||
        (pFmlReq->ulPageCount > hFML->nOTPPages - pFmlReq->ulStartPage))
    {
        ioStat.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOR_DEV_READ_OTPPAGES    DevReq;

        /*  These structures are the same for now.
        */
        DevReq = *pFmlReq;
        DevReq.ior.ioFunc = FXIOFUNC_DEV_OTP_READPAGES;

        ioStat = FfxDevIORequest(hFML->pLayout->hFimDev, &DevReq.ior);
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlOtpReadPages() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlOtpWritePages()

    Parameters:
        hFML    - The FML handle
        pFmlReq - A pointer to the request structure to use

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlOtpWritePages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_WRITE_OTPPAGES      *pFmlReq)
{
    FFXIOSTATUS                     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlOtpWritePages() hFML=%P\n", hFML));

    DclProfilerEnter("FfxFmlOtpWritePages", 0, 0);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->nOTPPages) ||
        (pFmlReq->ulPageCount > hFML->nOTPPages - pFmlReq->ulStartPage))
    {
        ioStat.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOR_DEV_WRITE_OTPPAGES    DevReq;

        /*  These structures are the same for now.
        */
        DevReq = *pFmlReq;
        DevReq.ior.ioFunc = FXIOFUNC_DEV_OTP_WRITEPAGES;

        ioStat = FfxDevIORequest(hFML->pLayout->hFimDev, &DevReq.ior);
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlOtpWritePages() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlOtpLock()

    Parameters:
        hFML    - The FML handle
        pFmlReq - A pointer to the request structure to use

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlOtpLock(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_LOCK_OTP            *pFmlReq)
{
    FFXIOR_DEV_LOCK_OTP             DevReq;
    FFXIOSTATUS                     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlOtpLock() hFML=%P\n", hFML));

    DclProfilerEnter("FfxFmlOtpLock", 0, 0);

    /*  These structures are the same for now.
    */
    DevReq = *pFmlReq;
    DevReq.ior.ioFunc = FXIOFUNC_DEV_OTP_LOCK;

    ioStat = FfxDevIORequest(hFML->pLayout->hFimDev, &DevReq.ior);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlOtpLock() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}

#endif

