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

    Display the FlashFX signon message.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: signon.c $
    Revision 1.10  2009/07/19 02:32:35Z  garyp
    Merged from the v4.0 branch.  Updated to print the sign-on information as a
    single string, and to return a pointer to that string on exit.  Added the fQuiet
    option.
    Revision 1.9  2009/04/01 15:44:27Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.8  2009/02/09 00:42:44Z  garyp
    Documentation updated.
    Revision 1.7  2008/05/04 03:01:43Z  garyp
    Renamed a symbol.
    Revision 1.6  2008/01/13 07:27:17Z  keithg
    Function header updates to support autodoc.
    Revision 1.5  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/10/03 02:21:13Z  Garyp
    Modified to display the signon only if FlashFX is NOT acting as a
    subproduct.
    Revision 1.3  2006/10/06 22:34:09Z  Garyp
    Updated to allow the signon functionality to be trimmed out.
    Revision 1.2  2006/10/03 23:58:25Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.1  2005/10/02 02:11:50Z  Pauli
    Initial revision
    Revision 1.6  2004/12/30 23:43:16Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.5  2003/04/15 20:53:58Z  garyp
    Updated to include FXVER.H.
    Revision 1.4  2003/03/22 08:17:44Z  garyp
    Prefixed the functions with "Ffx" and modified to use the updated
    product and version strings.
    Revision 1.3  2002/12/02 21:44:18Z  garyp
    Modified to use FfxPrintf().
    11/19/97 HDS Changed include file search to use predefined path.
    08/26/97 PKG Now uses new Datalight specific types
    03/24/97 PKG Removed the extra new line at start of signon
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxver.h>

/*  Always embed the copyright into the data, even if output is not
    enabled.

    Note that the order and form of these strings is critical.  The
    definitions are done in fxver.h, and the order is done here, and
    it applies for the whole product.  Various other components may
    query and parse this data, so don't randomly "clean" it up.
*/
#if DCLCONF_OUTPUT_ENABLED
static const char szVersion[] = PRODUCTNAME "\n" PRODUCTLEGAL "\n" PRODUCTPATENT "\n" PRODUCTEDITION "\n";
#else
static const char szVersion[] = PRODUCTNAME "\n" PRODUCTLEGAL "\n";
#endif


/*  Note that this function is always compiled and linked in, even
    when DCLCONF_OUTPUT_ENABLED is FALSE -- to ensure that the
    copyright strings are embedded in the binary.
*/


/*-------------------------------------------------------------------
    Public: FfxSignOn()

    Display the FlashFX sign-on messages and returns a pointer to the
    sign-on information strings.

    If DCLCONF_OUTPUT_ENABLED is TRUE, the sign-on messages have
    the form:

        PRODUCTNAME <lf>
        PRODUCTLEGAL <lf>
        [PRODUCTPATENT <lf>]          (if any)
        PRODUCTEDITION <lf> \0

    If DCLCONF_OUTPUT_ENABLED is FALSE, the sign-on messages have
    the form:

        PRODUCTNAME <lf>
        PRODUCTLEGAL <lf> \0

    The various PRODUCT* strings are defined in fxver.h.

    Parameters:
        fQuiet - If TRUE indicates that the act of signing on
                 should be ignored -- just return the sign-on
                 message string.

    Return Value:
        Returns a pointer to the sign-on message string.
-------------------------------------------------------------------*/
const char * FfxSignOn(
    D_BOOL      fQuiet)
{
  #if DCLCONF_OUTPUT_ENABLED

    /*  Display the signon ONLY if FlashFX is NOT acting as a sub-product.
    */
  #ifndef D_SUBPRODUCTNUM
    if(!fQuiet)
        DclPrintf("%s", szVersion);
  #endif
  #endif

    return szVersion;
}


