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

    This module registers a replacement output service which allow all
    all standard output to be redirected to the console or or a file,
    rather than the standard debug device.

    This is typically used with standalonce CE programs -- not device
    drivers.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlceoutput.c $
    Revision 1.4  2010/11/01 04:29:26Z  garyp
    Updated to use the DclWinLog* interface, rather than streams, as it
    allows much more flexibility.
    Revision 1.3  2009/06/27 23:10:40Z  garyp
    Updated to redirect I/O using services.
    Revision 1.2  2009/02/08 00:14:06Z  garyp
    Merged from the v4.0 branch.  Updated DclOutputString() to take an extra
    parameter.
    Revision 1.1  2007/11/28 22:45:08Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlceutil.h>
#include <dlservice.h>
#include <dlwinutil.h>

typedef struct 
{
    FILE               *hOutputFile;
    D_BOOL              fSerialOutput;
    D_BOOL              fShadow;
} OUTPUTCONTEXT;    

typedef struct sDCLCEREDIRECTIONINFO
{
    DCLSERVICEHEADER    ServOut;
    DCLOUTPUTINFO       oi;
    OUTPUTCONTEXT       Context;
} DCLCEREDIRECTIONINFO;


static void OutputString(void *pContext, const char *pszStr);


/*-------------------------------------------------------------------
    Public: DclCeOutputRedirectionBegin()

    This function initiates output redirection to either the standard
    CE console device, or to a file.  Redirection to a file may be
    specified with the standard ">" or ">>" notation.

    If the specified command-line does not contain any redirection
    directives, the output will be sent to the console.

    If the command-line contains redirection directives, the output
    will be sent to the specified file, and the directive will be
    removed from the command-line (and replaced with spaces).

    If this function returns a non-NULL handle, the function
    DclCeOutputRedirectionEnd() must be called to terminate the
    redirection, close the file (if any), and release resources.

    Parameters:
        hDclInst   - The DCL instance handle.  May be NULL to use
                     the default DCL instance.
        pszCmdLine - The command-line to process.

    Return Value:
        Returns a redirection handle if successful, or NULL
        otherwise.
-------------------------------------------------------------------*/
DCLREDIRECTIONHANDLE DclCeOutputRedirectionBegin(
    DCLINSTANCEHANDLE       hDclInst,
    char                   *pszCmdLine)
{
    char                    achBuffer[_MAX_PATH];
    D_UINT16                uArgNum;
    D_UINT16                uArgCount;
    DCLSTATUS               dclStat;
    DCLCEREDIRECTIONINFO   *pInfo;
    DCLSERVICEHEADER       *pSrv;

    DclAssert(pszCmdLine);

    pInfo = DclMemAllocZero(sizeof(*pInfo));
    if(!pInfo)
        return NULL;

    uArgCount = DclArgCount(pszCmdLine);

    for(uArgNum=1; uArgNum<=uArgCount; uArgNum++)
    {
        if(!DclArgRetrieve(pszCmdLine, uArgNum, sizeof(achBuffer), achBuffer))
        {
            DclProductionError();

            DclMemFree(pInfo);

            return NULL;
        }

        if(achBuffer[0] == '>')
        {
            D_BOOL          fAppend = FALSE;
            const char     *pStr = &achBuffer[1];
            D_WCHAR         wzName[256];

            if(*pStr == '>')
            {
                fAppend = TRUE;
                pStr++;
            }

            DclOsAnsiToWcs(wzName, DCLDIMENSIONOF(wzName), pStr, -1);

            pInfo->Context.hOutputFile = DclWinLogOpen(wzName, fAppend);
            if(!pInfo->Context.hOutputFile)
            {
                DclPrintf("Unable to open file \"%W\" for redirected output\n", wzName);

                DclMemFree(pInfo);

                return NULL;
            }

            DclWinLogPrintf(pInfo->Context.hOutputFile, "Log '%s' opened\n", pStr);

            pInfo->oi.pContext = &pInfo->Context;

            /*  Clear the argument so it is invisible to the guy that
                called us.
            */
            DclArgClear(pszCmdLine, uArgNum);

            break;
        }
    }

    /*  Initialize and create the output service
    */
    pInfo->oi.pfnOutputString = OutputString;

    pSrv = &pInfo->ServOut;
    dclStat = DclServiceHeaderInit(&pSrv, "DLCEOUTPUT", DCLSERVICE_OUTPUT, NULL, &pInfo->oi, 0);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        dclStat = DclServiceCreate(hDclInst, pSrv);
        if(dclStat == DCLSTAT_SUCCESS)
            return pInfo;
    }

    DclPrintf("Service creation failed with status code %lX\n", dclStat);

    DclMemFree(pInfo);

    return NULL;
}


/*-------------------------------------------------------------------
    Public: DclCeOutputRedirectionEnd()

    This function terminates output redirection which was set up
    with DclCeOutputRedirectionBegin(), and releases any allocated
    resources.

    Parameters:
        hRedirect - The redirection handle.

    Return Value:
        None
-------------------------------------------------------------------*/
void DclCeOutputRedirectionEnd(
    DCLREDIRECTIONHANDLE    hRedirect)
{
    DclAssert(hRedirect);

    /*  Restore the original output service
    */
    DclServiceDestroy(&hRedirect->ServOut);

    /*  Close the file, if any
    */
    if(hRedirect->Context.hOutputFile)
        DclWinLogClose(hRedirect->Context.hOutputFile);

    DclMemFree(hRedirect);

    return;
}


/*-------------------------------------------------------------------
    Local: OutputString()

    This is the workhorse display function used for both general
    messages as well as debugging messages.

    Parameters:
        pContext - A pointer to user data
        pszStr   - A pointer to the null-terminated string to
                   display.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OutputString(
    void           *pContext,
    const char     *pszStr)
{
    OUTPUTCONTEXT  *pOutputContext = pContext;

    if(pOutputContext && pOutputContext->hOutputFile)
        DclWinLogPrintf(pOutputContext->hOutputFile, "%s", pszStr);
    else
        printf("%s", pszStr);

    return;
}




