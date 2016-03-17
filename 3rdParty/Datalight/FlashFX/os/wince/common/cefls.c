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

    This module contains code for locating and loading the Windows CE MBR
    and FLS (Flash Layout Sector).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: cefls.c $
    Revision 1.2  2009/02/09 03:47:12Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.8  2009/02/09 03:47:12Z  garyp
    Updated to correctly use #ifdef rather than #if to test DCL_OSTARGET_WINMOBILE.
    Revision 1.1.1.7  2009/02/05 02:37:37Z  garyp
    Made some includes conditional on CE 5.02 rather than 5.00.
    Revision 1.1.1.6  2009/02/04 21:31:47Z  garyp
    Use a better mechanism to calculate ulSectorsPerBlock.
    Revision 1.1.1.5  2009/01/22 02:50:57Z  garyp
    Conditioned some code on DCL_OSTARGET_WINMOBILE.
    Revision 1.1.1.4  2008/10/08 02:20:32Z  garyp
    Modified FfxCeGetFLSInfo() to optionally return detailed information about
    any compaction blocks.
    Revision 1.1.1.3  2008/08/19 02:21:31Z  garyp
    Updated to use FfxDriverFmlSectorRead().
    Revision 1.1.1.2  2008/07/11 21:18:55Z  garyp
    Modified FfxCeLoadMBRandFLS() to return the erase block number where
    the MBR and FLS were found.
    Revision 1.1  2008/06/18 01:16:08Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>

#if _WIN32_WCE >= 502
#include <fls.h>
#endif

#include <flashfx.h>




/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 1/0 instances containing 346 lines.
    Modified 0 lines.
---------------------------------------------------------*/
