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

    ToDo: The IOCTL_FMD_RAW_WRITE_BLOCKS functionality must be updated to
          recognize SectorInfo structures which have the FFXWIN_TAG_SIGNATURE
          value, and treat them appropriately.  See fxwmpostproc.c and
          ffxfmdwm.c.

          Remove commented out debug code.

          Decide if any real FMD_* interface is needed and finish/delete
          that code as necessary.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxfmd.c $
    Revision 1.6  2009/08/04 20:15:58Z  garyp
    Merged from the v4.0 branch.  Added FfxCeEmulatedFmdIoControl() and
    eliminated all the original code.  This module is WinMobile specific.
    Revision 1.5  2009/03/09 20:04:46Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.4  2009/02/25 18:25:57Z  michaelm
    Initialized bTagBuff in FMD_ReadSector() to avoid compile warning.
    Revision 1.3  2009/02/09 02:15:31Z  garyp
    Merged from the v4.0 branch.  Updated to use some renamed variables
    -- no functional changes.
    Revision 1.2  2008/03/31 02:07:26Z  garyp
    Updated to build cleanly with CE 4.2.
    Revision 1.1  2008/03/13 20:48:32Z  Garyp
    Initial revision
    Revision 1.4  2007/12/26 02:51:34Z  Garyp
    Cleaned up the NAND handling logic.  Laid the foundation for supporting
    NOR (not finished), and added an IOCTL handler.
    Revision 1.3  2007/12/01 01:17:22Z  Garyp
    Updated to build cleanly with the new LFA way of doing things.
    Revision 1.2  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/07/31 02:36:04Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#if _WIN32_WCE >= 502

#include <fmd.h>

#include <flashfx.h>

#endif  /* _WIN32_WCE >= 500 */




/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 1/0 instances containing 421 lines.
    Modified 0 lines.
---------------------------------------------------------*/
