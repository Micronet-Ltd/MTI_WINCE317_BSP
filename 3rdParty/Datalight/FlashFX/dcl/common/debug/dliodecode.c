/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions. 

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

    This module provides debugging functionality to decode status results
    from I/O requests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dliodecode.c $
    Revision 1.2  2010/12/22 00:03:29Z  garyp
    Documentation corrections -- no functional changes.
    Revision 1.1  2010/04/17 21:29:34Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dliosys.h>


#define MAXMSGLEN   (80)


/*-------------------------------------------------------------------
    Public: DclDecodeIOStatus()

    Decode an DCLIOSTATUS structure that was filled during an I/O
    request.

    This function returns a pointer to a static buffer, and
    therefore the contents must be used immediately.  This is
    not thread-safe, and this function should only be used
    for debugging purposes.

    Parameters:
        pIOStatus - A pointer to the DCLIOSTATUS structure to decode

    Return Value:
        Returns a pointer to a static string.
-------------------------------------------------------------------*/
const char *DclDecodeIOStatus(
    DCLIOSTATUS    *pIOStatus)
{
    static char     szMessageBuff[MAXMSGLEN+1];

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
    else
    {
        DclSNPrintf(szMessageBuff, MAXMSGLEN, "IOStatus={Stat=%lX Count=%2lU}",
            pIOStatus->dclStat, pIOStatus->ulCount);
    }

    return szMessageBuff;
}



