/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    This program forces a save of the entire WinCE registry by calling
    RegFlushKey.  This is useful for testing the FXWriteRegistryToOEM
    and the FXReadRegistryFromOEM functions of FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: savereg.c $
    Revision 1.2  2007/11/03 23:50:15Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2004/12/31 02:43:16Z  Pauli
    Initial revision
    Revision 1.1  2004/12/31 02:43:16Z  Garyp
    Initial revision
    Revision 1.1  2004/12/31 01:43:16Z  pauli
    Initial revision
    Revision 1.1  2004/12/31 00:43:16Z  pauli
    Initial revision
    Revision 1.2  2004/12/30 23:43:16Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2003/05/29 06:25:54Z  garyp
    Initial revision
    02/27/02 gp  Changed to use REG_READ_BYTES_START as the condition to use to
                 determine if CE 2.0 is in use or not.  Changed to flush both
                 HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER for hive registry
                 compatibility (should have no detrimental affect on earlier
                 versions of CE).
    02/18/02 gjs ifdef for CE 2.0, which doesn't have RegFlushKey.
    02/07/02 DE  Updated copyright notice for 2002.
    02/06/02 gp  Modified to dump out the amount of time required to save
                 the registry.
    07/15/01 DE  Created.
---------------------------------------------------------------------------*/
#include <windows.h>


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine,
    int nCmdShow)
{
#ifdef REG_READ_BYTES_START
    DWORD           dwTime;

    dwTime = GetTickCount();

    RegFlushKey(HKEY_LOCAL_MACHINE);
    RegFlushKey(HKEY_CURRENT_USER);

    dwTime = GetTickCount() - dwTime;

    RETAILMSG(1, (TEXT("\r\nSaveReg required %dms\r\n"), dwTime));

#else
    RETAILMSG(1, (TEXT("\r\nSaveReg requires CE 2.10 or later!\r\n")));
#endif

    return 0;
}
