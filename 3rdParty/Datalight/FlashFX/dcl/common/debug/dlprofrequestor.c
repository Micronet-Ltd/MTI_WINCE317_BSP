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

    This module contains the requestor interface to the profiler service.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlprofrequestor.c $
    Revision 1.3  2010/07/15 01:03:05Z  garyp
    Added functionality to reset the profiler data.
    Revision 1.2  2010/04/17 22:29:53Z  garyp
    Updated the profiler summary command to support an "AdjustOverhead"
    flag.
    Revision 1.1  2009/11/10 20:57:24Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlservice.h>


/*-------------------------------------------------------------------
    Protected: DclProfSummary()

    Parameters:
        fReset - TRUE to reset the profiler data
        fShort - TRUE if a short summary is desired

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProfSummary(
    D_BOOL                  fReset,
    D_BOOL                  fShort,
    D_BOOL                  fAdjustOverhead)
{
    DCLDECLAREREQUESTPACKET (PROFILER, SUMMARY, summary);  /* DCLREQ_PROFILER_SUMMARY */
    DCLSTATUS               dclStat;

    summary.fReset = fReset;
    summary.fShort = fShort;
    summary.fAdjustOverhead = fAdjustOverhead;
    
    dclStat = DclServiceIoctl(NULL, DCLSERVICE_PROFILER, &summary.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclProfSummary() unexpected error %lX\n", dclStat));
        }
    }
    else
    {
        dclStat = summary.dclStat;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclProfEnable()

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProfEnable(void)
{
    DCLDECLAREREQUESTPACKET (PROFILER, ENABLE, enable);  /* DCLREQ_PROFILER_ENABLE */
    DCLSTATUS               dclStat;

    dclStat = DclServiceIoctl(NULL, DCLSERVICE_PROFILER, &enable.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclProfEnable() unexpected error %lX\n", dclStat));
        }
    }
    else
    {
        dclStat = enable.dclStat;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclProfDisable()

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProfDisable(void)
{
    DCLDECLAREREQUESTPACKET (PROFILER, DISABLE, disable);  /* DCLREQ_PROFILER_DISABLE */
    DCLSTATUS               dclStat;

    dclStat = DclServiceIoctl(NULL, DCLSERVICE_PROFILER, &disable.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclProfDisable() unexpected error %lX\n", dclStat));
        }
    }
    else
    {
        dclStat = disable.dclStat;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclProfReset()

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProfReset(void)
{
    DCLDECLAREREQUESTPACKET (PROFILER, RESET, reset);  /* DCLREQ_PROFILER_RESET */
    DCLSTATUS               dclStat;

    dclStat = DclServiceIoctl(NULL, DCLSERVICE_PROFILER, &reset.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclProfReset() unexpected error %lX\n", dclStat));
        }
    }
    else
    {
        dclStat = reset.dclStat;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclProfToggle()

    Toggle the enable/disabled state of the profiler.

    Parameters:
        fSummaryReset - TRUE to display the profiler summary and
                        reset the counters.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProfToggle(
    D_BOOL                  fSummaryReset)
{
    DCLDECLAREREQUESTPACKET (PROFILER, TOGGLE, toggle);  /* DCLREQ_PROFILER_TOGGLE */
    DCLSTATUS               dclStat;

    toggle.fSummaryReset = fSummaryReset;
    dclStat = DclServiceIoctl(NULL, DCLSERVICE_PROFILER, &toggle.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclProfToggle() unexpected error %lX\n", dclStat));
        }
    }
    else
    {
        dclStat = toggle.dclStat;
    }

    return dclStat;
}


