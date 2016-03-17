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

    This module contains the default OS Services functions for Ansi to wide-
    character translation.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oswcstr.c $
    Revision 1.3  2009/04/10 20:52:04Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.2  2007/11/03 23:31:26Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/08/01 00:20:44Z  brandont
    Initial revision
    Revision 1.3  2007/08/01 00:20:44Z  Garyp
    Documentation fixes.
    Revision 1.2  2006/04/06 22:00:52Z  brandont
    Changed to be a stubbed implementation.
    Revision 1.1  2006/04/03 21:11:06Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#if DCL_OSFEATURE_UNICODE

/*-------------------------------------------------------------------
    Public: DclOsWcsToAnsi()

    Convert the specified Unicode string to an ANSI string.  It can
    also be used to determine the amount of buffer space required to
    hold a converted string.

    Should the specified output buffer size be too small this
    function will return zero, however the output buffer may
    be modified.

    The resulting output string will only be null-terminated if
    the source string and designated length was null-terminated.

    Parameters:
        pOutBuff    - A pointer to the output buffer (should be NULL
                      if nOutBuffLen is zero).
        nOutBuffLen - The length of the output buffer.  Set this
                      value to zero to query for the required output
                      buffer size.
        pUnicodeStr - A pointer to Unicode source string to convert.
        iLen        - The length to convert, or -1 if pUnicodeStr is
                      null-terminated.

    Return Value:
      - If nOutBuffLen is zero, the return value will be the size of
        buffer required to hold the fully converted string.
      - If nOutBuffLen is not zero, this function returns the number
        of converted characters.
      - Returns zero on failure.
-------------------------------------------------------------------*/
unsigned DclOsWcsToAnsi(
    char           *pOutBuff,
    unsigned        nOutBuffLen,
    const D_WCHAR  *pUnicodeStr,
    int             iLen)
{
    DclAssert(pUnicodeStr);

    DclProductionError();

    return 0;
}


/*-------------------------------------------------------------------
    Public: DclOsAnsiToWcs()

    convert the specified ANSI string to a Unicode string.  It can
    also be used to determine the amount of buffer space required
    to hold a converted string.

    Should the specified output buffer size be too small this
    function will return zero, however the output buffer may
    be modified.

    The resulting output string will only be null-terminated if
    the source string and designated length was null-terminated.

    Parameters:
        pOutBuff    - A pointer to the output buffer (should be NULL
                      if nOutBuffLen is zero).
        nOutBuffLen - The length of the output buffer.  Set this
                      value to zero to query for the required output
                      buffer size.
        pAnsiStr    - A pointer to Ansi source string to convert.
        iLen        - The length to convert, or -1 if pAnsiStr is
                      null-terminated.

    Return Value:
      - If nOutBuffLen is zero, the return value will be the size of
        buffer required to hold the fully converted string.
      - If nOutBuffLen is not zero, this function returns the number
        of converted characters.
      - Returns zero on failure.
-------------------------------------------------------------------*/
unsigned DclOsAnsiToWcs(
    D_WCHAR        *pOutBuff,
    unsigned        nOutBuffLen,
    const char     *pAnsiStr,
    int             iLen)
{
    DclAssert(pAnsiStr);

    DclProductionError();

    return 0;
}



#endif
