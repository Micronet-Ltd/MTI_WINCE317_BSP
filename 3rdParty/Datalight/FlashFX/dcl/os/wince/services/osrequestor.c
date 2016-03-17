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

    This module provides the framework that allows a program/executable to
    issue generic IOCTL requests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osrequestor.c $
    Revision 1.7  2009/02/12 20:11:37Z  garyp
    Renamed a symbol.
    Revision 1.6  2009/02/10 02:25:04Z  garyp
    Fixed to trim leading and trailing white space off the device name.
    Revision 1.5  2009/02/07 23:49:00Z  garyp
    Merge from the v4.0 branch.  Updated the requestor interface so the device
    type is specified on the open call, rather than when functions are dispatched.
    Revision 1.4  2008/07/08 19:58:34Z  brandont
    Updated to support DCL DCLIODEVICE_FILESYSTEM IOCTLs.
    Revision 1.3  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/07/31 00:43:05Z  Garyp
    Moved a setting to dlosconf.h.
    Revision 1.1  2007/07/17 20:24:08Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <winioctl.h>
#include <diskio.h>
#include <pkfuncs.h>

#include <dcl.h>
#include <dlapireq.h>

static HANDLE       ahReqs[DCL_IOCTL_MAX_REQUESTS];
static DCLIODEVICE  anIODevs[DCL_IOCTL_MAX_REQUESTS];


/*-------------------------------------------------------------------
    Public: DclOsRequestorDispatch()

    This function dispatches IOCTL requests.

    Parameters:
        hReq      - The requestor handle
        pReq      - A pointer to the DCLIOREQUEST structure to use

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclOsRequestorDispatch(
    DCLOSREQHANDLE  hReq,
    DCLIOREQUEST   *pReq)
{
    DWORD           ulNumBytesReturned;
    D_UINT32        ulReqLen;
    DWORD           dwCtl;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_REQUESTOR, 1, 0),
        "DclOsRequestorDispatch() hReq=%lU pReq=%P\n", hReq, pReq));

    /*  Make relative to zero
    */
    hReq--;

    if(hReq >= DCL_IOCTL_MAX_REQUESTS)
        return DCLSTAT_REQUESTOR_BADHANDLE;

    if(ahReqs[hReq] == INVALID_HANDLE_VALUE)
        return DCLSTAT_REQUESTOR_NOTOPEN;

    switch(anIODevs[hReq])
    {
        case DCLIODEVICE_FILESYSTEM:
            dwCtl = DCL_IOCTL_FILESYS;
            break;

        case DCLIODEVICE_BLOCK:
            dwCtl = DCL_IOCTL_BLOCKDEV;
            break;

        default:
            DclError();
            return DCLSTAT_REQUESTOR_UNSUPPORTEDDEVICE;
    }

    ulReqLen = pReq->ulReqLen;

    if(DeviceIoControl(ahReqs[hReq], dwCtl, pReq, ulReqLen,
                       pReq, ulReqLen, &ulNumBytesReturned, NULL))
    {
        /*  Verify the correct size is returned
        */
        if(ulNumBytesReturned == ulReqLen)
            return DCLSTAT_SUCCESS;

        return DCLSTAT_REQUESTOR_REQUESTFAILED;
    }
    else
    {
        DCLPRINTF(1, ("DclOsRequestorDispatch() interface failed, Error=%lU\n", GetLastError()));

        return DCLSTAT_REQUESTOR_INTERFACEFAILED;
    }
}


/*-------------------------------------------------------------------
    Public: DclOsRequestorOpen()

    This function opens a requestor interface to the specified
    target device.  The semantics used for the target device
    name are unique to each operating system.

    Multiple opens for the same target may not be done unless
    the OS abstraction returns a unique handle for each.

    The requestor handle will always be a non-zero value.

    Software may detect whether the OS requestor interface is
    implemented/supported by passing a '0' nIODevType value (the
    other parameters will be ignored).  If the function returns
    DCLSTAT_REQUESTOR_BADDEVICE then the interface is supported.
    If it returns DCLSTAT_REQUESTOR_INTERFACEFAILED then the
    requestor interface is not implemented or not supported.

    Parameters:
        pszName     - The null terminated name of the disk to open.
        nIODevType  - The base IOCTL value (DCLIODEVICE_*) to use
        phRequestor - A pointer to the location in which the
                      requestor handle will be stored.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsRequestorOpen(
    const char     *pszName,
    DCLIODEVICE     nIODevType,
    DCLOSREQHANDLE *phRequestor)
{
    TCHAR           awcBuffer[MAX_PATH];
    HANDLE          hReq;
    static unsigned fInited = FALSE;
    unsigned        ii;
    unsigned        nSlot = UINT_MAX;
    unsigned        nLen;
    const char     *pszEnd;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_REQUESTOR, 1, 0),
        "DclOsRequestorOpen() Name=\"%s\" Type=%u phReq=%P\n", pszName, nIODevType, phRequestor));

    if(nIODevType <= DCLIODEVICE_RESERVED || nIODevType >= DCLIODEVICE_HIGHLIMIT)
        return DCLSTAT_REQUESTOR_BADDEVICE;

    /*  One time initialization
    */
    if(!fInited)
    {
        for(ii=0; ii<DCL_IOCTL_MAX_REQUESTS; ii++)
            ahReqs[ii] = INVALID_HANDLE_VALUE;

        fInited = TRUE;
    }

    DclAssert(pszName);
    DclAssert(phRequestor);

    /*  Eat any leading white space (spaces only)
    */
    while(*pszName && (*pszName == ' '))
        pszName++;

    /*  Better be something left
    */
    nLen = DclStrLen(pszName);
    if(!nLen)
        return DCLSTAT_REQUESTOR_BADNAME;

    /*  Reduce the length to eliminate any trailing space
    */
    pszEnd = pszName + nLen - 1;
    while(nLen && (*pszEnd == ' '))
    {
        nLen--;
        pszEnd--;
    }

    /*  Must be something left or we have a coding error
    */
    DclAssert(nLen);

    /*  The device name must be specified in Unicode TCHAR format
        so convert it now.
    */
    MultiByteToWideChar(CP_ACP, 0, pszName, nLen, awcBuffer, DCLDIMENSIONOF(awcBuffer)-1);

    /*  NULL-terminate the result
    */
    awcBuffer[nLen] = 0;

    if(nIODevType == DCLIODEVICE_FILESYSTEM)
    {
        if(awcBuffer[wcslen(awcBuffer) - 1] != L'\\')
            wcscat(awcBuffer, L"\\");

        /*  The file system handle is acquired by opening the "VOL:"
            file in the root of the volume.
        */
        wcscat(awcBuffer, L"VOL:");
    }

    /*  Attempt to open the disk device
    */
    hReq = CreateFile(awcBuffer,
                      GENERIC_READ | GENERIC_WRITE, 0, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hReq == INVALID_HANDLE_VALUE)
    {
        DCLPRINTF(1, ("DclOsRequestorOpen() failed for \"%W\", error=%lU\n",
                      awcBuffer, GetLastError()));
        return DCLSTAT_REQUESTOR_OPENFAILED;
    }

    for(ii=0; ii<DCL_IOCTL_MAX_REQUESTS; ii++)
    {
        /*  If it is already open, just fail for now because we have
            no logic to handle multiple instances of the same drive.
        */
        if(ahReqs[ii] == hReq)
        {
            CloseHandle(hReq);
            return DCLSTAT_REQUESTOR_ALREADYOPEN;
        }
        else if(nSlot == UINT_MAX && ahReqs[ii] == INVALID_HANDLE_VALUE)
        {
            nSlot = ii;
        }
    }

    if(nSlot == UINT_MAX)
    {
        /*  No slots left!
        */
        CloseHandle(hReq);
        return DCLSTAT_REQUESTOR_OUTOFHANDLES;
    }

    ahReqs[nSlot] = hReq;
    anIODevs[nSlot] = nIODevType;

    /*  A DCLOSREQHANDLE is an index number, however 0 is not
        legal, so add 1.
    */
    *phRequestor = (DCLOSREQHANDLE)(nSlot + 1);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsRequestorClose()

    This function closes a requestor handle which was opened by
    DclOsRequestorOpen().

    Parameters:
        hReq    - The requestor handle

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclOsRequestorClose(
    DCLOSREQHANDLE  hReq)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_REQUESTOR, 1, 0),
        "DclOsRequestorClose() hReq=%lU\n", hReq));

    /*  Make relative to zero
    */
    hReq--;

    if(hReq >= DCL_IOCTL_MAX_REQUESTS)
        return DCLSTAT_REQUESTOR_BADHANDLE;

    if(ahReqs[hReq] == INVALID_HANDLE_VALUE)
        return DCLSTAT_REQUESTOR_NOTOPEN;

    if(!CloseHandle(ahReqs[hReq]))
    {
        DCLPRINTF(1, ("DclOsRequestorClose() failed, error=%lU\n", GetLastError()));

        return DCLSTAT_REQUESTOR_CLOSEFAILED;
    }

    ahReqs[hReq] = INVALID_HANDLE_VALUE;

    return DCLSTAT_SUCCESS;
}


