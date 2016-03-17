/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains the Windows CE BootLoader OS Services default
    implementations for:

        DclOsOutputString()

    Should these routines need customization for your project, copy this
    module into your project directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: osoutput.c $
    Revision 1.2  2010/07/06 01:32:52Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.1  2009/11/06 01:57:12Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <nkintr.h>

#include <dcl.h>

/*  Serial output function for the MS Ethernet boot loader...
*/



/*-------------------------------------------------------------------
    Public: DclOsOutputString()

    Display a string on the output device.

    Parameters:
        pUserData - Must be NULL at this time
        pszStr    - A pointer to the null-terminated string to display

    Return Value:
        None.
-------------------------------------------------------------------*/
#define BUFFLEN 256
void DclOsOutputString(
    void           *pUserData,
    const char     *pszStr)
{
    (void)pUserData;

    DclAssert(pszStr);

/*    NKDbgPrintfW((LPCWSTR)pszStr);*/

/*    RETAILMSG(1,(TEXT("%hs\r\n"),pszStr)); */

    while(*pszStr)
    {
        if(*pszStr == '\n')
            OEMWriteDebugByte('\r');
        
        OEMWriteDebugByte(*pszStr++);
    }

#if 0

    /*  This logic may be used in an OAL implementation that
        has a functional RETAILMSG() macro.
    */
    TCHAR           awcBuffer[BUFFLEN];
    int             i;

    /*  This code converts an ANSI string into a Unicode string.
        It is necessary because MultiByteToWideChar() is not available
        at this stage in the CE boot process, and there is apparently
        not any other kernel function available to do this.
    */
    for(i = 0; i < BUFFLEN; i++)
    {
        awcBuffer[i] = (TCHAR) pszStr[i];

        if(!pszStr[i])
            break;
    }

    RETAILMSG(1, (awcBuffer));
#endif

    return;
}
