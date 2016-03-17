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

    This module provides the framework that allows a standalone Windows CE
    application to call into FlashFX.  Typically the application will simply
    use standard FlashFX API function calls, as documented, however it will
    link with the FXOEAPI library rather than the main FlashFX libraries.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: extreq.c $
    Revision 1.10  2009/03/31 20:43:21Z  thomd
    Added a missing header.
    Revision 1.9  2009/02/09 07:09:22Z  garyp
    Merged from the v4.0 branch.  Modified to use the updated requestor
    interface.
    Revision 1.8  2008/03/26 00:26:52Z  Garyp
    Documentation updated.
    Revision 1.7  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/06/22 15:25:22Z  Garyp
    Updated to use the generic external requestor logic now implemented in DCL.
    Revision 1.5  2007/04/21 03:26:20Z  timothyj
    Fixed asserts to use one-based index instead of zero-based.
    Revision 1.4  2006/02/20 16:36:45Z  Garyp
    Updated debugging code.
    Revision 1.3  2006/02/13 10:38:55Z  Garyp
    Updated to new external API requestor interface.
    Revision 1.2  2006/02/07 07:04:46Z  Garyp
    Started the process of factoring things into Devices and Disks.
    Work-in-progress, not completed yet.
    Revision 1.1  2005/10/02 01:29:14Z  Pauli
    Initial revision
    Revision 1.2  2004/12/30 23:03:10Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/01/19 03:53:08Z  garyp
    Initial revision
    Revision 1.1.1.2  2004/01/03 02:08:34Z  garyp
    Modified to use DRIVERPREFIX rather than hardcoding "DSK".
    Revision 1.1  2003/11/22 03:21:30Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>

#include <flashfx.h>
#include <fxdriver.h>
#include <fxapireq.h>
#include <ffxwce.h>
#include <dlapireq.h>


/*-------------------------------------------------------------------
    Protected: FlashFX_DevRequest()

    Dispatch an IOCTL request.

    Parameters:
        hReq     - The requestor handle
        pReq     - A pointer to the DCLIOREQUEST structure to use

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FlashFX_DevRequest(
    FFXREQHANDLE    hReq,
    FFXIOREQUEST   *pReq)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "DevRequest hReq=%lU pReq=%P\n", hReq, pReq));

    DclAssert(hReq);
    DclAssert(pReq);

    return DclOsRequestorDispatch(hReq, (DCLIOREQUEST*)pReq);
}


/*-------------------------------------------------------------------
    Protected: FlashFX_DevOpen()

    Open a requestor interface to the specified CE store.

    Multiple opens for the same store may not be done unless
    the operating system returns a unique handle for each.

    Parameters:
        nDiskNum - The zero-relative CE disk store number to use.

    Return Value:
        Returns the handle to use on subsequent calls to
        FlashFX_DevRequest(), or zero on failure.
-------------------------------------------------------------------*/
#define BUFFLEN 16
FFXREQHANDLE FlashFX_DevOpen(
    unsigned        nDiskNum)
{
    char            szDiskName[] = DRIVERPREFIX "n:";
    DCLOSREQHANDLE  hReq;
    DCLSTATUS       dclStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
        "FlashFX_DevOpen() DiskNum=%u\n", nDiskNum));

    DclAssert(DclStrLen(DRIVERPREFIX) == 3);

    szDiskName[3] = (D_UCHAR) nDiskNum + '1';

    dclStat = DclOsRequestorOpen(szDiskName, DCLIODEVICE_BLOCK, &hReq);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        return hReq;
    }
    else
    {
        FFXPRINTF(1, ("FlashFX_DevOpen() failed to open \"%s\", Status=%lX\n", szDiskName, dclStat));
        return 0;
    }
}


/*-------------------------------------------------------------------
    Protected: FlashFX_DevClose()

    Close a requestor handle which was opened by FlashFX_DevOpen().

    Parameters:
        hReq    - The requestor handle

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FlashFX_DevClose(
    FFXREQHANDLE    hReq)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
        "FlashFX_DevClose() hReq=%lU\n", hReq));

    DclAssert(hReq);

    return DclOsRequestorClose(hReq);
}

