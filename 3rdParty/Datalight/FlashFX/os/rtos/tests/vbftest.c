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

    This module is the interface to the VBF test.

    See the comments included in the respective tests/tools code for
    the syntax for this utility.
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                              Revision History
    $Log: vbftest.c $
    Revision 1.6  2009/02/09 18:53:26Z  thomd
    Corrected affected members of FFXTOOLPARAMS
    Revision 1.5  2008/05/06 05:30:47Z  keithg
    Function header comment updates and additions.
    Revision 1.4  2008/01/13 07:28:44Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/11/03 23:50:02Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/05/06 00:32:26Z  Pauli
    Added "const" to input parameter.
    Revision 1.1  2006/01/25 17:25:00Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/


#include <flashfx.h>
#include <toolfw.h>
#include <oecommon.h>

static D_CHAR   szCmdName[] = "FlashFXTestVBF";

/*-------------------------------------------------------------------
    Public: FlashFXTestVBF

    Parameters:
        pszCmdLine  - Pointer to an ASCISZ terminated command
                      line parameters.

    Return:
        Zero upon successful completion of the unit tests.
        Non-zero upon any error, see FfxTestVBF implementation
        for more details.
-------------------------------------------------------------------*/
int FlashFXTestVBF(
    const char     *pszCmdLine)
{
    FFXTOOLPARAMS   tp;

    tp.dtp.pszCmdName = szCmdName;
    tp.dtp.pszCmdLine = pszCmdLine;
    tp.dtp.pszExtraHelp = 0;

    return FfxToolFramework(&tp, FfxTestVBF);
}

