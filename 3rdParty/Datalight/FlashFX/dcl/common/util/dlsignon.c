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
  jurisdictions.  Patents may be pending.

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

    Display the Datalight Common Libraries signon message.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsignon.c $
    Revision 1.9  2009/06/28 00:50:04Z  garyp
    Updated to print the sign-on information as a single string, and to return
    a pointer to that string on exit.  Added the fQuiet option.
    Revision 1.8  2009/04/15 21:31:12Z  garyp
    Documentation updates -- no functional changes.
    Revision 1.7  2009/02/08 02:57:38Z  garyp
    Merged from the v4.0 code base.  Updated to print the sign-on information
    as a single string.
    Revision 1.6  2008/05/04 03:07:37Z  garyp
    Symbol renamed.
    Revision 1.5  2007/12/18 03:52:19Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.3  2006/10/08 20:58:30Z  Garyp
    Fixed up so we can properly scale the sign-on code out if we want.
    Revision 1.2  2006/10/06 22:34:11Z  Garyp
    Modified to declare the product strings as "const".
    Revision 1.1  2005/10/02 03:57:08Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 19:03:02Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/04/11 04:52:52Z  pauli
    Initial revision
    Revision 1.2  2005/04/11 04:52:51Z  PaulI
    XP Merge
    Revision 1.1.1.3  2005/04/11 04:52:51Z  garyp
    Removed an unnecessary header.
    Revision 1.1.1.2  2005/04/11 04:50:47Z  garyp
    Updated to the new form required for DCL functions.
    Revision 1.1  2005/01/10 05:48:11Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlver.h>


/*  Redefine this to the default value just in case it was defined
    in dlapiint.h.  If that is the case, this function will still
    compile as expected, it just won't be called.
*/
#undef  DclSignOn
#define DclSignOn   DCLFUNC(DclSignOn)


/*  Always embed the copyright into the data, even if output is not
    enabled.

    Note that the order and form of these strings is critical.  The
    definitions are done in dlver.h, and the order is done here, and
    it applies for the whole product.  Various other components may
    query and parse this data, so don't randomly "clean" it up.
*/
#if DCLCONF_OUTPUT_ENABLED
static const char szVersion[] = PRODUCTNAME "\n" PRODUCTLEGAL "\n" PRODUCTEDITION "\n";
#else
static const char szVersion[] = PRODUCTNAME "\n" PRODUCTLEGAL "\n";
#endif


/*  Note that this function is always compiled and linked in, even
    when DCLCONF_OUTPUT_ENABLED is FALSE -- to ensure that the
    copyright strings are embedded in the binary.
*/


/*-------------------------------------------------------------------
    Public: DclSignOn()

    This function displays the Datalight Common Libraries sign-on
    messages and returns a pointer to the sign-on information
    strings.

    If DCLCONF_OUTPUT_ENABLED is TRUE, the sign-on messages have
    the form:

        PRODUCTNAME <lf>
        PRODUCTLEGAL <lf>
        [PRODUCTPATENTS <lf>]       (if any)
        PRODUCTEDITION <lf> 0

    If DCLCONF_OUTPUT_ENABLED is FALSE, the sign-on messages have
    the form:

        PRODUCTNAME <lf>
        PRODUCTLEGAL <lf> 0

    The various PRODUCT* strings are defined in dlver.h.

    Parameters:
        fQuiet - If TRUE indicates that the act of signing on
                 should be ignored -- just return the sign-on
                 message string.

    Return Value:
        Returns a pointer to the sign-on message string.
-------------------------------------------------------------------*/
const char * DclSignOn(
    D_BOOL    fQuiet)
{
  #if DCLCONF_OUTPUT_ENABLED
    if(!fQuiet)
        DclPrintf("%s", szVersion);
  #endif

    return szVersion;
}
