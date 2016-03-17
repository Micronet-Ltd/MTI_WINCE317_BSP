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

    This module contains the Windows CE OS Services default implementations
    for:

        DclOsOutputString()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osoutput.c $
    Revision 1.7  2009/04/10 18:46:14Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2009/03/30 03:27:53Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.5  2009/03/27 21:22:23Z  billr
    Update copyright date.
    Revision 1.4  2009/03/26 21:39:44Z  billr
    Resolve bug 2102: Setting DCLCONF_OUTPUT_ENABLED to FALSE breaks builds.
    Revision 1.3  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/10/15 18:13:04Z  Garyp
    Fixed to wrap long lines rather than truncate them.  Fixed to not try to
    printf the output line as the format string.  Now use DCLOUTPUT_BUFFERLEN
    rather than an arbitrary work buffer length.  Reduced stack usage.
    Revision 1.1  2005/10/22 05:55:48Z  Pauli
    Initial revision
    Revision 1.2  2005/10/22 06:55:47Z  garyp
    Updated DclOsOutputString() to take a new pUserData parameter.
    Revision 1.1  2005/05/04 23:41:38Z  Garyp
    Initial revision
    Revision 1.3  2005/08/21 11:39:15Z  garyp
    Documentation update.
    Revision 1.2  2005/08/03 19:14:28Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/06/12 21:19:54Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclOsOutputString()

    Display a string on the output device.

    Parameters:
        pUserData - Must be NULL at this time
        pszStr    - A pointer to the null-terminated string to display

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclOsOutputString(
    void           *pUserData,
    const char     *pszStr)
{
#if DCLCONF_OUTPUT_ENABLED
    DclAssert(pUserData == NULL);
    DclAssert(pszStr);

    (void)pUserData;

    while(*pszStr)
    {
        TCHAR       awcBuffer[DCLOUTPUT_BUFFERLEN];
        unsigned    nLen = 0;

        /*  The RETAILMSG() stuff automatically includes an implicit
            CR-LF for every line output, so no need to include one in
            the data streams we send to it.
        */
        while(*(pszStr+nLen) && nLen < DCLOUTPUT_BUFFERLEN && *(pszStr+nLen) != '\n')
            nLen++;

        MultiByteToWideChar(CP_ACP, 0, pszStr, nLen, awcBuffer, DCLOUTPUT_BUFFERLEN);

        /*  Null terminate the wide-char string
        */
        awcBuffer[nLen] = 0;

        /*  (FYI, already tried using an uppercase %S to cause the CE stuff
            to do the ANSI to wide-char conversion -- doesn't work.)
        */
        RETAILMSG(1, (TEXT("%s\r\n"), awcBuffer));

        /*  Point past the segment we just processed
        */
        pszStr += nLen;

        /*  If that segment was terminated by a newline, just skip past it
        */
        if(*pszStr == '\n')
            pszStr++;
    }
#else
    (void) pUserData;
    (void) pszStr;
#endif
}

