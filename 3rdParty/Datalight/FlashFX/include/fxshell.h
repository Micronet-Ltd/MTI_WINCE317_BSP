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

    Interfaces for implementing (rather than just invoking) the FlashFX
    Shell.
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshell.h $
    Revision 1.1  2009/12/31 17:24:42Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXSHELL_H_INCLUDED
#define FXSHELL_H_INCLUDED

/*  FlashFX Shell stuff
*/
#if DCLCONF_COMMAND_SHELL

FFXSTATUS   FfxShellAddCommands(     DCLSHELLHANDLE hShell, const char *pszClass, const char *pszPrefix);
FFXSTATUS   FfxShellAddTraceCommands(DCLSHELLHANDLE hShell, const char *pszClass, const char *pszPrefix);
FFXSTATUS   FfxShellAddOsCommands(   DCLSHELLHANDLE hShell, const char *pszClass, const char *pszPrefix);
FFXSTATUS   FfxDebug(unsigned nDiskNum, DCLSHELLPARAMS *pSP);
#endif

#endif /* FXSHELL_H_INCLUDED */
