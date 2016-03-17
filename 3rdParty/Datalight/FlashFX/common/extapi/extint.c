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
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

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

    This module contains the top level logic required to use the FlashFX
    External API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: extint.c $
    Revision 1.21  2010/07/10 16:12:03Z  garyp
    Updated to use the DCLREQ_VERSIONCHECK structure defined in DCL.
    Revision 1.20  2010/06/17 19:21:49Z  garyp
    Minor error code update.
    Revision 1.19  2010/01/23 21:36:32Z  garyp
    Updated FlashFX_Open() to provide a more useful status code in the
    event that there is a version mismatch.  Fixed some error messages to
    not be TRACE messages.
    Revision 1.18  2009/09/15 22:23:47Z  garyp
    Updated FlashFX_Close() to accommodate the case where the DCL instance
    destruction only reduces the usage count and does not actually destroy the
    instance.  Resolved Bug 2843: fxinfo fails on assertion at extint.c:412.
    Revision 1.17  2009/07/24 01:58:16Z  garyp
    Finished the merge from the v4.0 branch, and dynamically create a DCL
    instance if necessary.
    Revision 1.16  2009/05/19 21:32:18Z  garyp
    Extended "VERSIONVAL" to use the full four bytes for the version number
    and no longer tack the "Alpha" or "Beta" onto the end.
    Revision 1.15  2009/04/03 19:32:00Z  davidh
    Function header errors corrected for AutoDoc.
    Revision 1.14  2009/03/31 19:30:35Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.13  2009/03/24 21:45:50Z  keithg
    Added unconditional initialization to placate picky compilers.
    Revision 1.12  2009/02/09 06:59:16Z  garyp
    Merged from the v4.0 branch.  Major update to the external requestor API
    to disassociate the requestor handle from a specific Disk.  Most requests
    already include a handle inside the request packet itself.  Added the
    FlashFX_Open() and FlashFX_Close() functions.
    Revision 1.11  2008/05/04 03:01:45Z  garyp
    Renamed a symbol.
    Revision 1.10  2008/01/17 03:19:08Z  Garyp
    Fixed FlashFX_VersionCheck() to return the documented error codes.  Added
    FfxApiOpenAnyReqHandle()
    Revision 1.9  2008/01/13 07:26:41Z  keithg
    Function header updates to support autodoc.
    Revision 1.8  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/08/18 21:23:04Z  garyp
    Updated to use the new three part, segmented version number.
    Revision 1.6  2006/09/16 19:26:55Z  Garyp
    Modified to handle build numbers which are now strings rather than numbers.
    Revision 1.5  2006/02/20 19:41:59Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.4  2006/02/13 02:47:33Z  Garyp
    Updated to new external API interface.
    Revision 1.3  2006/02/07 07:32:16Z  Garyp
    Renamed FFXDEVHANDLE to FFXREQHANDLE to avoid conflicts with the
    new meaning of FFXDEVHANDLE.
    Revision 1.2  2005/12/31 19:49:51Z  Garyp
    Modified so DriveNum is not returned if the pointer is NULL.
    Revision 1.1  2005/10/02 01:43:36Z  Pauli
    Initial revision
    Revision 1.6  2005/04/20 01:09:29Z  GaryP
    Code formatting cleanup.
    Revision 1.5  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2004/11/30 18:33:47Z  GaryP
    Documentation fixes.
    Revision 1.3  2004/05/26 18:57:06Z  garyp
    Added a typecast so we build cleanly.
    Revision 1.2  2004/03/15 22:45:13Z  garys
    Added FfxApiFindAnyReqHandle().
    Revision 1.1  2003/11/26 20:48:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxver.h>
#include <fxapi.h>
#include <fxapireq.h>

#define DEFAULT_DCL_INSTANCE_NUMBER     (0)
#define BAD_REQUESTOR_HANDLE            (0)

typedef struct
{
    FFXREQHANDLE        hReq;
    DCLINSTANCEHANDLE   hDclInst;
    unsigned            nUsageCount;
    D_BOOL              fWeCreatedDclInstance;
} FFXREQINFO;

static FFXREQINFO   aReqInfo[DCL_MAX_INSTANCES];


static FFXSTATUS OpenAndValidateReqHandle(unsigned nDiskNum, FFXREQHANDLE *phReq);
static FFXSTATUS VersionValidate(FFXREQHANDLE hReq, FFXVERSION *pVer, D_BOOL fQuiet);



                /*---------------------------------*\
                 *                                 *
                 *        Public Interface         *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Public: FlashFX_Open()

    This function opens a requestor interface based on the specified
    Disk number if possible.

    This function validates that the version of the driver matches
    that of the calling application.

    Parameters:
        nDiskNum      - The Disk to open.  Specify UINT_MAX to open
                        any available Disk.
        phDclInst     - A pointer to a variable containing the DCL
                        Instance handle to use.  If the pointer or
                        the value pointed to are NULL, a DCL Instance
                        will be created on the fly.  The DCL Instance
                        handle will be returned in this field if it
                        is not NULL.
        phReq         - A pointer to the variable to receive the
                        requestor handle.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FlashFX_Open(
    unsigned            nDiskNum,
    DCLINSTANCEHANDLE  *phDclInst,
    FFXREQHANDLE       *phReq)
{
    FFXSTATUS           ffxStat = FFXSTAT_SUCCESS; /* Placate picky compilers */
    D_BOOL              fNewDclInst = FALSE;
    unsigned            nDclInstNum;
    DCLINSTANCEHANDLE   hDclInst = NULL;

    if(phDclInst)
        hDclInst = *phDclInst;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FlashFX_Open() DISK%d phDclInst=%P hDclInst=%P\n", nDiskNum, phDclInst, hDclInst));

    if(nDiskNum >= FFX_MAX_DISKS && nDiskNum != UINT_MAX)
        return FFXSTAT_DISKNUMBERINVALID;

    if(!phReq)
        return DCLSTAT_BADPOINTER;

    if(hDclInst)
    {
        /*  A DCL instance handle was supplied, so figure out which
            instance number it is, so we know what slot it goes in.
        */
        for(nDclInstNum=0; nDclInstNum<DCL_MAX_INSTANCES; nDclInstNum++)
        {
            if(DclInstanceHandle(nDclInstNum) == hDclInst)
                break;
        }

        if(nDclInstNum == DCL_MAX_INSTANCES)
        {
            FFXPRINTF(1, ("FlashFX_Open() Invalid DCL instance handle %P\n", hDclInst));

            return DCLSTAT_INST_BADHANDLE;
        }

        if(aReqInfo[nDclInstNum].hDclInst)
        {
            /*  If calling in with an explicit DCL instance handle, if the
                slot is taken, it had better be using a matching handle.
            */
            if(aReqInfo[nDclInstNum].hDclInst != hDclInst)
            {
                FFXPRINTF(1, ("FlashFX_Open() DCL handle mismatch for InstNum %u, Current=%P New=%P\n",
                    nDclInstNum, aReqInfo[nDclInstNum].hDclInst, hDclInst));

                return DCLSTAT_INST_HANDLEMISMATCH;
            }

            /*  Reality check, if the slot is already taken, these must be
                initialized as well.
            */
            DclAssert(aReqInfo[nDclInstNum].hReq);
            DclAssert(aReqInfo[nDclInstNum].nUsageCount);
        }
        else
        {
            /*  Reality check, if the slot is empty, these should be clear
            */
            DclAssert(!aReqInfo[nDclInstNum].hReq);
            DclAssert(!aReqInfo[nDclInstNum].nUsageCount);
            DclAssert(!aReqInfo[nDclInstNum].fWeCreatedDclInstance);

            aReqInfo[nDclInstNum].hDclInst = hDclInst;
        }
    }
    else
    {
        nDclInstNum = DEFAULT_DCL_INSTANCE_NUMBER;

        if(aReqInfo[nDclInstNum].hDclInst)
        {
            DclAssert(aReqInfo[nDclInstNum].hReq);
            DclAssert(aReqInfo[nDclInstNum].nUsageCount);
        }
        else
        {
            DclAssert(!aReqInfo[nDclInstNum].hReq);
            DclAssert(!aReqInfo[nDclInstNum].nUsageCount);
            DclAssert(!aReqInfo[nDclInstNum].fWeCreatedDclInstance);

            ffxStat = DclInstanceCreate(nDclInstNum, DCLFLAG_APP, &aReqInfo[nDclInstNum].hDclInst);
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;

            aReqInfo[nDclInstNum].fWeCreatedDclInstance = TRUE;

            fNewDclInst = TRUE;
        }

        hDclInst = aReqInfo[nDclInstNum].hDclInst;
    }

    if(aReqInfo[nDclInstNum].hReq)
    {
        aReqInfo[nDclInstNum].nUsageCount++;

        if(phDclInst && *phDclInst == NULL)
        {
            /*  Return the DCL instance handle in the caller's buffer
            */
            *phDclInst = hDclInst;
        }

        *phReq = aReqInfo[nDclInstNum].hReq;

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
            "FlashFX_Open() DclInst=%P InstNum=%u returning existing handle hReq=%lU UsageCount=%u\n",
            hDclInst, nDclInstNum, *phReq, aReqInfo[nDclInstNum].nUsageCount));

        return FFXSTAT_SUCCESS;
    }

    /*  If we get to this point, we have a DCL Instance, but there is no
        open requestor handle.  Depending on nDiskNum, either open a
        specific interface based on that number, or just try to find any
        Disk controlled by a FlashFX driver of the proper version.
    */
    if(nDiskNum != UINT_MAX)
    {
        ffxStat = OpenAndValidateReqHandle(nDiskNum, &aReqInfo[nDclInstNum].hReq);
    }
    else
    {
        D_BOOL  fVersionMismatch = FALSE;
        
        for(nDiskNum = 0; nDiskNum < FFX_MAX_DISKS; nDiskNum++)
        {
            ffxStat = OpenAndValidateReqHandle(nDiskNum, &aReqInfo[nDclInstNum].hReq);
            if(ffxStat == FFXSTAT_SUCCESS)
                break;

            /*  Make sure that any FFXSTAT_VERSIONMISMATCH error is preserved,
                since when we are scanning through devices, since that is
                probably more more useful.
            */
            if(ffxStat == FFXSTAT_VERSIONMISMATCH)
                fVersionMismatch = TRUE;
        }

        /*  Keep the more meaningful status code.
        */
        if(ffxStat != FFXSTAT_SUCCESS && fVersionMismatch)
            ffxStat = FFXSTAT_VERSIONMISMATCH;
    }

    if(ffxStat == FFXSTAT_SUCCESS)
    {
        aReqInfo[nDclInstNum].nUsageCount++;

        if(phDclInst && *phDclInst == NULL)
        {
            /*  Return the DCL instance handle in the caller's buffer
            */
            *phDclInst = hDclInst;
        }

        *phReq = aReqInfo[nDclInstNum].hReq;

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
            "FlashFX_Open() DclInst=%P InstNum=%u returning new handle hReq=%lU UsageCount=%u\n",
            hDclInst, nDclInstNum, *phReq, aReqInfo[nDclInstNum].nUsageCount));

        return ffxStat;
    }

    /*  Couldn't open the interface, so destroy the DCL Instance if
        we created it.
    */
    if(fNewDclInst)
    {
        if(DclInstanceDestroy(hDclInst) == FFXSTAT_SUCCESS)
            aReqInfo[nDclInstNum].fWeCreatedDclInstance = FALSE;
    }

    /*  Zero this in case we filled it in with a passed in handle, or
        a dynamically created instance.
    */
    aReqInfo[nDclInstNum].hDclInst = NULL;

    /*  If we have failed at this point, the slot should be empty.
    */
    DclAssert(!aReqInfo[nDclInstNum].hReq);
    DclAssert(!aReqInfo[nDclInstNum].nUsageCount);
    DclAssert(!aReqInfo[nDclInstNum].fWeCreatedDclInstance);

    FFXPRINTF(1, ("FlashFX_Open() hDclInst=%P InstNum=%u failed, Status=%lX\n",
        hDclInst, nDclInstNum, ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FlashFX_Close()

    This function closes the requestor interface which was opened
    with FlashFX_Open().

    If a DCL Instance was created when this requestor handle was
    opened, it will be destroyed during this call.

    Parameters:
        hReq - The requestor handle.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FlashFX_Close(
    FFXREQHANDLE    hReq)
{
    unsigned        nInstNum;
    FFXSTATUS       ffxStat = FFXSTAT_DEVNOTOPEN;

    DclAssert(hReq);

    /*  Scan until we find our handle
    */
    for(nInstNum = 0; nInstNum < DCLDIMENSIONOF(aReqInfo); nInstNum++)
    {
        if(aReqInfo[nInstNum].hReq == hReq)
        {
            /*  Reality check...
            */
            DclAssert(aReqInfo[nInstNum].hDclInst);
            DclAssert(aReqInfo[nInstNum].nUsageCount);

            /*  Reduce the usage count.  If it has not yet gone
                to zero, we just leave.
            */
            aReqInfo[nInstNum].nUsageCount--;
            if(aReqInfo[nInstNum].nUsageCount)
            {
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                    "FlashFX_Close() hReq=%lU hDclInst=%P InstNum=%u reduced usage count to %u\n",
                    hReq, aReqInfo[nInstNum].hDclInst, nInstNum, aReqInfo[nInstNum].nUsageCount));

                return FFXSTAT_SUCCESS;
            }

            /*  Usage count has gone to zero, so close the requestor handle
            */
            ffxStat = FlashFX_DevClose(hReq);
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                aReqInfo[nInstNum].hReq = BAD_REQUESTOR_HANDLE;

                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                    "FlashFX_Close() hReq=%lU hDclInst=%P InstNum=%u closed requestor handle\n",
                    hReq, aReqInfo[nInstNum].hDclInst, nInstNum));

                /*  If the DCL Instance was created at this level, destroy it
                */
                if(aReqInfo[nInstNum].fWeCreatedDclInstance)
                {
                    ffxStat = DclInstanceDestroy(aReqInfo[nInstNum].hDclInst);
                    if(ffxStat == DCLSTAT_INST_USAGECOUNTREDUCED)
                        ffxStat = FFXSTAT_SUCCESS;

                    DclAssert(ffxStat == FFXSTAT_SUCCESS);

                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                        "FlashFX_Close() hReq=%lU destroying hDclInst=%P returned status %lX\n",
                        hReq, aReqInfo[nInstNum].hDclInst, ffxStat));

                    aReqInfo[nInstNum].fWeCreatedDclInstance = FALSE;
                }

                aReqInfo[nInstNum].hDclInst = NULL;
            }

            break;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
        "FlashFX_Close() hReq=%lU returning %lX\n", hReq, ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FlashFX_VersionCheck()

    Ensure that the FlashFX device driver is installed, that the
    version number of the installed driver matches that of the 
    application, and optionally returns the version information.

    Parameters:
        pVer   - A pointer to the FFXVERSION structure to use.  May
                 be NULL if the version information does not need be
                 returned.
        fQuiet - Set to FALSE for normal operation or TRUE to
                 suppress messages.

    Return Value:
        FFXSTAT_SUCCESS             - Function successful
        FFXSTAT_VERSIONMISMATCH     - FlashFX driver version mismatch
        FFXSTAT_DRIVERNOTINSTALLED  - FlashFX driver not installed

        The FFXVERSION strucure will only be modified if the return
        value is FFXSTAT_SUCCESS or FFXSTAT_VERSIONMISMATCH.
-------------------------------------------------------------------*/
FFXSTATUS FlashFX_VersionCheck(
    FFXVERSION         *pVer,
    D_BOOL              fQuiet)
{
    FFXSTATUS           ffxStat;
    FFXREQHANDLE        hReq;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
        "FlashFX_VersionCheck() Quiet=%u\n", fQuiet));

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        /*  In many cases the FlashFX_Open() call will validate the version,
            however, this could be for a Disk which was already open, or the
            use may want a verbose version message, so do it again.
        */
        ffxStat = VersionValidate(hReq, pVer, fQuiet);

        /*  Close the requestor handle only if we were the ones
            that opened it.
        */
        FlashFX_Close(hReq);

        return ffxStat;
    }

    if(!fQuiet)
        DclPrintf("\nThe FlashFX device driver is not installed.\n");

    return ffxStat;
}


                /*---------------------------------*\
                 *                                 *
                 *         Local Functions         *
                 *                                 *
                \*---------------------------------*/


/*-------------------------------------------------------------------
    Local: OpenAndValidateReqHandle()

    This function attempts to find an already open requestor handle,
    and if one cannot be found, will attempt to open one.

    Parameters:
        nDiskNum  - The Disk number to use
        phReq     - A pointer to a location in which the requestor
                    handle will be stored.

    Return Value:
        Returns FFXSTAT_DEVWONTOPEN if the interface cannot be opened
        or a general FFXSTATUS value if some other error occurred.
-------------------------------------------------------------------*/
static FFXSTATUS OpenAndValidateReqHandle(
    unsigned        nDiskNum,
    FFXREQHANDLE   *phReq)
{
    FFXREQHANDLE    hReq;

    DclAssert(phReq);

    hReq = FlashFX_DevOpen(nDiskNum);
    if(hReq)
    {
        FFXSTATUS   ffxStat;

        ffxStat = VersionValidate(hReq, NULL, TRUE);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, ("Ffx:OpenAndValidateReqHandle() device %u bad driver version or not a FlashFX Disk, Status=%lX\n",
                nDiskNum, ffxStat));

            FlashFX_DevClose(hReq);

            return ffxStat;
        }

        *phReq = hReq;

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
            "Ffx:OpenAndValidateReqHandle() returning hReq=%lU\n", hReq));

        return ffxStat;
    }

    return FFXSTAT_DEVWONTOPEN;
}


/*-------------------------------------------------------------------
    Local: VersionValidate()

    Ensure that the FlashFX device driver is installed, that the
    version number of the installed driver matches that of the 
    application, and optionally returns the version information.

    Parameters:
        hReq   - The requestor handle.
        pVer   - A pointer to the FFXVERSION structure to use.  May
                 be NULL if the version information does not need be
                 returned.
        fQuiet - Set to FALSE for normal operation or TRUE to
                 suppress messages.

    Return Value:
        FFXSTAT_SUCCESS             - Function successful
        FFXSTAT_VERSIONMISMATCH     - FlashFX driver version mismatch
        FFXSTAT_DRIVERNOTINSTALLED  - FlashFX driver not installed

        The FFXVERSION strucure will only be modified if the return
        value is FFXSTAT_SUCCESS or FFXSTAT_VERSIONMISMATCH.

        May be some other status code if the requestor interface
        fails.
-------------------------------------------------------------------*/
static FFXSTATUS VersionValidate(
    FFXREQHANDLE        hReq,
    FFXVERSION         *pVer,
    D_BOOL              fQuiet)
{
    FFXSTATUS           ffxStat;
    FFXVERSION          fv;
    DCLREQ_VERSIONCHECK version;

    DclAssert(hReq);

    /*  If we aren't returning the version information use
        our own local structure.
    */
    if(!pVer)
        pVer = &fv;

    DclMemSet(&version, 0, sizeof(version));

    version.ior.ioFunc = FXIOFUNC_EXTFML_VERSIONCHECK;
    version.ior.ulReqLen = sizeof(version);
    version.ulSignature = FFXAPI_REQUESTSIGNATURE;

    ffxStat = FlashFX_DevRequest(hReq, &version.ior);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        if(version.ulSignature == FFXAPI_REPLYSIGNATURE)
        {
            pVer->ulVersion = version.ulVersion;

            DclAssert(DclStrLen(version.szBuildNum) < sizeof(pVer->szBuildNum));
            DclStrNCpy(pVer->szBuildNum, version.szBuildNum, sizeof(pVer->szBuildNum));

            /*  Fail this call if the version number is NOT an exact match
            */
            if(pVer->ulVersion != VERSIONVAL)
            {
                if(!fQuiet)
                {
                    char    achVersionBuff[DCL_MAX_VERSION_LENGTH+1];

                    DclPrintf("\nThis utility is designed for FlashFX " PRODUCTVERSION "\n");
                    DclPrintf("The FlashFX device driver in use is version %s\n",
                        DclVersionFormat(achVersionBuff, sizeof(achVersionBuff),
                        version.ulVersion, version.szBuildNum));
                }

                return FFXSTAT_VERSIONMISMATCH;
            }

            return FFXSTAT_SUCCESS;
        }

        ffxStat = FFXSTAT_DRIVERNOTINSTALLED;
    }

    return ffxStat;
}


