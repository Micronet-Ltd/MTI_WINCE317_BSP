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

    This module contains general functions for displaying output.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dloutput.c $
    Revision 1.12  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.11  2009/11/11 02:39:33Z  garyp
    Conditioned some debug output.
    Revision 1.10  2009/11/06 01:29:38Z  garyp
    Updated to allow output to happen even if the service has not yet
    been initialized.
    Revision 1.9  2009/06/28 00:46:59Z  garyp
    Updated to use this feature as a service.
    Revision 1.8  2009/02/17 06:04:01Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.7  2009/02/08 00:14:05Z  garyp
    Merged from the v4.0 branch.  Updated DclOutputString() to take an extra
    parameter.
    Revision 1.6  2008/05/03 19:54:02Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.5  2007/12/18 03:56:15Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/02/21 07:01:15Z  Garyp
    Modified to allow the thread ID and timestamp to be prepended onto
    messages.
    Revision 1.2  2006/10/03 22:49:20Z  Garyp
    Updated to allow output to be entirely disabled (and the code not pulled
    into the image).
    Revision 1.1  2005/10/02 03:57:06Z  Pauli
    Initial revision
    Revision 1.3  2005/09/30 01:44:49Z  Garyp
    Modified the interface to allow a user data pointer to be supplied along
    with the output function pointer.
    Revision 1.2  2005/08/03 19:13:58Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/04/10 18:58:14Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlservice.h>
#include <dlinstance.h>
#include <dlapiprv.h>


/*-------------------------------------------------------------------
    Public: DclOutputServiceInit()

    Initialize the output service.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOutputServiceInit(
    DCLINSTANCEHANDLE       hDclInst)
{
  #if DCLCONF_OUTPUT_ENABLED
    static DCLSERVICEHEADER SrvOutput[DCL_MAX_INSTANCES];
    static DCLOUTPUTINFO    oi[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER       *pSrv;
    DCLSTATUS               dclStat;
    unsigned                nInstNum;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    if(!hDclInst)
        return DCLSTAT_SERVICE_BADHANDLE;

    nInstNum = DclInstanceNumber(hDclInst);
    oi[nInstNum].pfnOutputString = DclOsOutputString;
    oi[nInstNum].pContext = NULL;

    pSrv = &SrvOutput[nInstNum];
    dclStat = DclServiceHeaderInit(&pSrv, "DLOUTPUT", DCLSERVICE_OUTPUT, NULL, &oi[nInstNum], 0);
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = DclServiceCreate(hDclInst, pSrv);

    return dclStat;

  #else

    /*  Even if a feature is disabled at compile-time, a given project may
        still be using a default set of services which tries to initialize
        it.  Therefore, to avoid link errors, this "Init" function must
        exist.  Simply politely decline to initialize if this is the case.
        The DCLSTAT_FEATUREDISABLED status code is explicitly accommodated
        in the project creation process, and will not abort the process.
    */
    (void)hDclInst;
    return DCLSTAT_FEATUREDISABLED;

  #endif
}


#if DCLCONF_OUTPUT_ENABLED

/*-------------------------------------------------------------------
    Public: DclOutputString()

    Output a string.

    This is the workhorse display function used for both general
    messages as well as debugging messages.

    Parameters:
        hDclInst - The DCL instance handle.  May be NULL to use the
                   default DCL instance.
        pszStr   - A pointer to the null-terminated string.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOutputString(
    DCLINSTANCEHANDLE       hDclInst,
    const char             *pszStr)
{
    DCLSTATUS               dclStat;
    static DCLOUTPUTINFO    oi;
    DCLOUTPUTINFO          *pOI;
    D_BOOL                  fGotSysMutex = FALSE;

    /*  DclAssert(pszStr); */

    dclStat = DclServiceData(hDclInst, DCLSERVICE_OUTPUT, (void**)&pOI);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(oi.pfnOutputString == NULL)
        {
          #if D_DEBUG >= 2
            DclOsOutputString(NULL, "The OUTPUT service is not initialized -- using the default OS output service\n");
          #endif

            oi.pContext = NULL;
            oi.pfnOutputString = DclOsOutputString;
        }
        
        pOI = &oi;
    }    

    if(!pOI)
        return DCLSTAT_SERVICE_NOTINITIALIZED;

    if(!pOI->pfnOutputString)
        return DCLSTAT_OUTPUTDISABLED;

  #if DCLOUTPUT_TIMESTAMP || DCLOUTPUT_THREADID
    if(pszStr)
    {
        static D_BOOL fNewLine = TRUE;

        /*  The while() loop we are in exists only to specially process
            those strings with leading linefeeds.  This allows thread
            ID/timestamp stuff to be displayed for those lines with
            leading linefeeds.  Doesn't help situations where there are
            embedded linefeeds, or multiple linefeeds at the end of a
            line.
        */
        while(TRUE)
        {
            if(fNewLine)
            {
                #define     MAXBUFFLEN  (32)
                char        szBuffer[MAXBUFFLEN];

              #if DCLOUTPUT_THREADID
                DclSNPrintf(szBuffer, sizeof(szBuffer), "%08lX ", (D_UINT32)DclOsThreadID());
              #else
                szBuffer[0] = 0;
              #endif

              #if DCLOUTPUT_TIMESTAMP
                {
                    size_t      nLen = DclStrLen(szBuffer);
                    D_TIME      ullTime;
                    DCLDATETIME DateTime;

                    DclOsGetDateTime(&ullTime);
                    DclDateTimeDecode(&ullTime, &DateTime);

                    DclSNPrintf(&szBuffer[nLen], sizeof(szBuffer) - nLen, "%02u:%02U:%02U.%03u ",
                        DateTime.uHour, DateTime.uMinute, DateTime.uSecond, DateTime.uMillisecond);
                }
              #endif

                if(!fGotSysMutex)
                {
                    DclOsMutexAcquire(NULL);
                    fGotSysMutex = TRUE;
                }

                (*pOI->pfnOutputString)(pOI->pContext, szBuffer);
            }

            /*  If we a leading linefeed, and not the last character,
                output it now, and increment our source string pointer
                to handle the next string segment.
            */
            if(*pszStr == '\n' && *(pszStr+1))
            {
                /*  Output the linefee now
                */
                (*pOI->pfnOutputString)(pOI->pContext, "\n");

                /*  Increment past it
                */
                pszStr++;

                /*  We are definitely on a new line...
                */
                fNewLine = TRUE;

                /*  Keep looping...
                */
                continue;
            }

            /*  We have no (more) leading linefeeds, so adjust the
                newline flag appropriately based on how this string
                segment ends, and break out of this while() loop.
            */
            if(pszStr[DclStrLen(pszStr)-1] == '\n')
                fNewLine = TRUE;
            else
                fNewLine = FALSE;

            break;
        }
    }
  #endif

    /*  Output whatever is left
    */
    (*pOI->pfnOutputString)(pOI->pContext, pszStr);

    if(fGotSysMutex)
        DclOsMutexRelease(NULL);

    return DCLSTAT_SUCCESS;
}


#endif  /* DCLCONF_OUTPUT_ENABLED */

