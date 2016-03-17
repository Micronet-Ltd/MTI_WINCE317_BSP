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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmemvalrequestor.c $
    Revision 1.1  2009/06/27 03:17:54Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlmemval.h>
#include <dlservice.h>


/*-------------------------------------------------------------------
    Protected: DclMemValErrorMode()

    Set the error mode for the Memory Validation service, and return
    the original mode.

    Parameters:
        nNewMode - The new error mode value, which must be
                   DCLMEM_ERROR_FAIL or DCLMEM_ERROR_ASSERT.

    Return Value:
        Returns an unsigned int containing the original error mode,
        or DCLMEM_ERROR_INVALID if the call failed.
-------------------------------------------------------------------*/
unsigned DclMemValErrorMode(
    DCLINSTANCEHANDLE           hDclInst,
    unsigned                    nNewMode)
{
    DCLDECLAREREQUESTPACKET     (MEMVAL, ERRORMODE, mode);  /* DCLREQ_MEMVAL_ERRORMODE */
    DCLSTATUS                   dclStat;

    DclAssert(nNewMode == DCLMEM_ERROR_FAIL || nNewMode == DCLMEM_ERROR_ASSERT);

    mode.nNewMode = nNewMode;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_MEMVALIDATION, &mode.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclMemValErrorMode() unexpected error %lX\n", dclStat));
        }

        return DCLMEM_ERROR_INVALID;
    }

    return mode.nOldMode;
}


/*-------------------------------------------------------------------
    Protected: DclMemValSummary()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemValSummary(
    DCLINSTANCEHANDLE       hDclInst,
    DCLMEMVALSUMMARY       *pVal,
    D_BOOL                  fQuiet,
    D_BOOL                  fReset)
{
    DCLDECLAREREQUESTPACKET (MEMVAL, SUMMARY, summary); /* DCLREQ_MEMVAL_SUMMARY */
    DCLSTATUS               dclStat;

    summary.fQuiet = fQuiet;
    summary.fReset = fReset;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_MEMVALIDATION, &summary.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclMemValSummary() unexpected error %lX\n", dclStat));
        }
    }
    else
    {
        if(pVal)
            *pVal = summary.MVS;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclMemValBufferAdd()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemValBufferAdd(
    DCLINSTANCEHANDLE       hDclInst,
    const void             *pBuffer,
    D_UINT32                ulBuffLen,
    unsigned                nFlags)
{
    DCLDECLAREREQUESTPACKET (MEMVAL, BUFFERADD, add); /* DCLREQ_MEMVAL_BUFFERADD */
    DCLSTATUS               dclStat;

    add.pBuffer = pBuffer;
    add.ulBuffLen = ulBuffLen;
    add.nFlags = nFlags;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_MEMVALIDATION, &add.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if( dclStat == DCLSTAT_SERVICE_RECURSEFAILED ||
            dclStat == DCLSTAT_SERVICE_NOTREGISTERED )
        {
            dclStat = DCLSTAT_SUCCESS;
        }
        else
        {
            DCLPRINTF(2, ("DclMemValBufferAdd() pBuff=%P Len=%lU Flags=%x unexpected error %lX\n",
                pBuffer, ulBuffLen, nFlags, dclStat));
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclMemValBufferRemove()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemValBufferRemove(
    DCLINSTANCEHANDLE       hDclInst,
    const void             *pBuffer)
{
    DCLDECLAREREQUESTPACKET (MEMVAL, BUFFERREMOVE, remove); /* DCLREQ_MEMVAL_BUFFERREMOVE */
    DCLSTATUS               dclStat;

    remove.pBuffer = pBuffer;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_MEMVALIDATION, &remove.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if( dclStat == DCLSTAT_SERVICE_RECURSEFAILED ||
            dclStat == DCLSTAT_SERVICE_NOTREGISTERED )
        {
            dclStat = DCLSTAT_SUCCESS;
        }
        else
        {
            DCLPRINTF(2, ("DclMemValBufferRemove() pBuff=%P unexpected error %lX\n", pBuffer, dclStat));
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclMemValBufferValidate()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemValBufferValidate(
    DCLINSTANCEHANDLE       hDclInst,
    const void             *pBuffer,
    D_UINT32                ulBuffLen,
    unsigned                nFlags)
{
    DCLDECLAREREQUESTPACKET (MEMVAL, BUFFERVALIDATE, validate); /* DCLREQ_MEMVAL_BUFFERVALIDATE */
    DCLSTATUS               dclStat;

    if(!pBuffer)
        return DCLSTAT_MEMVAL_INVALIDBUFFER;

    validate.pBuffer = pBuffer;
    validate.ulBuffLen = ulBuffLen;
    validate.nFlags = nFlags;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_MEMVALIDATION, &validate.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if( dclStat == DCLSTAT_SERVICE_RECURSEFAILED ||
            dclStat == DCLSTAT_SERVICE_NOTREGISTERED ||
            dclStat == DCLSTAT_INST_NOTINITIALIZED )
        {
            /*  We MUST be able to gracefully handle cases where the service
                is not installed or registered, or we are early in the startup
                process and these facilities are not available.  Not handling
                this properly will typically end up hanging the system.
            */
            dclStat = DCLSTAT_SUCCESS;
        }
        else
        {
            DCLPRINTF(1, ("DclMemValBufferValidate() unexpected error %lX\n", dclStat));
        }
    }

    return dclStat;
}


