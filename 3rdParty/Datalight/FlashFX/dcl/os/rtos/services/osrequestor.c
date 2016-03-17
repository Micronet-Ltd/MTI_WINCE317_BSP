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

    This module provides the framework that allows a program/executable to
    issue generic IOCTL requests.

    This is a stubbed implementation.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osrequestor.c $
    Revision 1.4  2011/02/09 00:42:04Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2009/02/07 23:43:15Z  garyp
    Merge from the v4.0 branch.  Updated the requestor interface so the device
    type is specified on the open call, rather than when functions are dispatched.
    Revision 1.2  2007/11/03 23:31:30Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/07/17 20:25:08Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapireq.h>

/*-------------------------------------------------------------------
    Public: DclOsRequestorDispatch()

    Dispatch IOCTL requests.

    Parameters:
        hReq      - The requestor handle
        pReq      - A pointer to the DCLIOREQUEST structure to use

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsRequestorDispatch(
    DCLOSREQHANDLE  hReq,
    DCLIOREQUEST   *pReq)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_REQUESTOR, 2, 0),
        "DclOsRequestorDispatch() hReq=%lU pReq=%P\n", hReq, pReq));

    (void)hReq;
    (void)pReq;

    DclProductionError();

    return DCLSTAT_REQUESTOR_NOTOPEN;
}


/*-------------------------------------------------------------------
    Public: DclOsRequestorOpen()

    Open a requestor interface to the specified target file system or
    block device.  The semantics used for the target device name are
    unique to each operating system.

    Multiple opens for the same target may not be done unless the OS
    abstraction returns a unique handle for each.

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
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsRequestorOpen(
    const char     *pszName,
    DCLIODEVICE     nIODevType,
    DCLOSREQHANDLE *phRequestor)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_REQUESTOR, 1, 0),
        "DclOsRequestorOpen() Name=\"%s\" Type=%u phReq=%P\n", pszName, nIODevType, phRequestor));

    (void)pszName;
    (void)nIODevType;
    (void)phRequestor;

    return DCLSTAT_REQUESTOR_INTERFACEFAILED;
}


/*-------------------------------------------------------------------
    Public: DclOsRequestorClose()

    Close a requestor handle which was opened by DclOsRequestorOpen().

    Parameters:
        hReq    - The requestor handle

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsRequestorClose(
    DCLOSREQHANDLE  hReq)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_REQUESTOR, 1, 0),
        "DclOsRequestorClose() hReq=%lU\n", hReq));

    (void)hReq;

    DclProductionError();

    return DCLSTAT_REQUESTOR_NOTOPEN;
}



