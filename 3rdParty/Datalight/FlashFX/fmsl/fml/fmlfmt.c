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

    This module contains FML functions for formatting the flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlfmt.c $
    Revision 1.5  2009/04/03 20:27:59Z  davidh
    Function header error corrections for AutoDoc.
    Revision 1.4  2009/04/01 21:06:54Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2008/01/13 07:36:41Z  keithg
    Function header updates to support autodoc.
    Revision 1.2  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/10/10 01:48:52Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fxfmlapi.h>
#include "fml.h"


/*-------------------------------------------------------------------
    Protected: FfxFmlFormat()

    This function formats the disk from an FML perspective --
    which is erasing the flash.  This function erases only the
    DISKn window which is mapped onto the flash (DEVn).  It
    does not erase the BBM area (if any), unless a raw flash
    mapping was done.

    Parameters:
        hFML - The handle for the FML instance to format.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlFormat(
    FFXFMLHANDLE    hFML)
{
    FFXFMLINFO      FmlInfo;
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEINDENT),
        "FfxFmlFormat() hFML=%P\n", hFML));

    ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        FFXIOSTATUS ioStat;

        FMLERASE_BLOCKS(hFML, 0, FmlInfo.ulTotalBlocks, ioStat);

        ffxStat = ioStat.ffxStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlFormat() returning Status=%lX\n", ffxStat));

    return ffxStat;
}


