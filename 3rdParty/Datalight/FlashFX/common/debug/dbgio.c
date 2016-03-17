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

    This module provides debugging functionality to decode status results
    from I/O requests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dbgio.c $
    Revision 1.9  2010/12/12 06:51:47Z  garyp
    Added a missing const keyword.
    Revision 1.8  2009/11/18 02:41:44Z  garyp
    Corrected a merge problem with the previous checkin.
    Revision 1.7  2009/11/18 02:32:23Z  garyp
    Updated to accommodate the case where pIOStatus is NULL.
    Revision 1.6  2009/07/21 17:51:43Z  garyp
    Merged from the v4.0 branch.  Added support for DeviceIOStatus.
    Revision 1.5  2009/03/31 17:49:18Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2008/01/13 07:26:16Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/11/03 23:49:30Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/04/05 07:29:04Z  Garyp
    Fixed FfxDecodeIOStatus() to be thread-safe.
    Revision 1.1  2006/02/01 23:44:40Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxiosys.h>


#define MAXMSGLEN   (80)


/*-------------------------------------------------------------------
    Public: FfxDecodeIOStatus()

    Decode an FFXIOSTATUS structure and format it in human readable
    form.

    This function is typically only used by debug-level code to
    decode an FFXIOSTATUS structure that was filled during an
    I/O request.

    This function returns a pointer to a static buffer, and
    therefore the contents must be used immediately.  This is
    not thread-safe, and this function should only be used
    for debugging purposes.

    Parameters:
        pIOStatus - A pointer to the FFXIOSTATUS structure to decode

    Return Value:
        Returns a pointer to a static string.
-------------------------------------------------------------------*/
const char *FfxDecodeIOStatus(
    const FFXIOSTATUS  *pIOStatus)
{
    static char         szMessageBuff[MAXMSGLEN+1];

    /*  Because this is a static buffer, it is not thread-safe.  Since this
        is used for debug output only we don't care too much about making
        it perfectly thread-safe.  However, we must ensure that there is
        always a final null-terminator regardless of whatever else we might
        be outputting.
    */
    szMessageBuff[MAXMSGLEN] = 0;

    if(!pIOStatus)
    {
        DclStrNCpy(szMessageBuff, "NULL Pointer", MAXMSGLEN);
    }
    else if((pIOStatus->ulFlags & IOFLAGS_TYPEMASK) == IOFLAGS_BLOCK)
    {
        DclSNPrintf(szMessageBuff, MAXMSGLEN, "BlockIOStatus={Stat=%lX Count=%lU BlockStat=%lX}",
            pIOStatus->ffxStat, pIOStatus->ulCount, pIOStatus->op.ulBlockStatus);
    }
    else if((pIOStatus->ulFlags & IOFLAGS_TYPEMASK) == IOFLAGS_PAGE)
    {
        DclSNPrintf(szMessageBuff, MAXMSGLEN, "PageIOStatus={Stat=%lX Count=%lU PageStat=%lX}",
            pIOStatus->ffxStat, pIOStatus->ulCount, pIOStatus->op.ulPageStatus);
    }
    else if((pIOStatus->ulFlags & IOFLAGS_TYPEMASK) == IOFLAGS_BYTE)
    {
        DclSNPrintf(szMessageBuff, MAXMSGLEN, "ByteIOStatus={Stat=%lX Count=%lU ByteStat=%lX}",
            pIOStatus->ffxStat, pIOStatus->ulCount, pIOStatus->op.ulByteStatus);
    }
    else if((pIOStatus->ulFlags & IOFLAGS_TYPEMASK) == IOFLAGS_DEVICE)
    {
        DclSNPrintf(szMessageBuff, MAXMSGLEN, "DeviceIOStatus={Stat=%lX Count=%lU DeviceStat=%lX}",
            pIOStatus->ffxStat, pIOStatus->ulCount, pIOStatus->op.ulDeviceStatus);
    }
    else
    {
        DclSNPrintf(szMessageBuff, MAXMSGLEN, "IOStatus={Stat=%lX Count=%lU}",
            pIOStatus->ffxStat, pIOStatus->ulCount);
    }

    return szMessageBuff;
}



