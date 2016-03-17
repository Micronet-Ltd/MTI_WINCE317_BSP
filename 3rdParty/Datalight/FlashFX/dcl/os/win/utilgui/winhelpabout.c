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

    This module implements a standard Help->About dialog box for use with
    general Datalight utilities.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.

    NOTE: This code is highly dependent on the structure of the standard
          product strings defined in the xxver.h file for each product.

    ToDo: - Replace strstr() uses with DclStrStr().
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winhelpabout.c $
    Revision 1.3  2009/07/01 21:57:40Z  garyp
    Reduced the volume of output when running on CE to better fit small screens.
    Revision 1.2  2009/06/28 02:46:24Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.4  2008/11/05 01:20:49Z  garyp
    Fixed some variable names to use correct Hungarian notation.  Updated
    documentation.  No functional changes.
    Revision 1.1.1.3  2008/09/06 23:04:23Z  garyp
    Minor tweak to the Datalight address.
    Revision 1.1.1.2  2008/09/01 00:58:35Z  garyp
    Updated to work better on CE when there is very little screen real-estate.
    Revision 1.1  2008/08/01 02:31:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>

#include <dcl.h>
#include <dlwinutil.h>


/*-------------------------------------------------------------------
    Public: DclWinHelpAbout()

    Display a Help->About dialog box using the specified text strings.

    Parameters:
        hWnd       - The window handle
        ptzPrefix  - A pointer to the string to use as the prefix,
                     which is typically the program name.  May be
                     NULL.
        ptzVersion - A pointer to the version information.  This
                     is typically the version string returned by
                     the "signon()" function for the product.
        ptzSuffix  - A pointer to the string to use as the suffix,
                     following the version information.  May be NULL.

    Return Value:
        Returns TRUE if everything worked, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclWinHelpAbout(
    HWND            hWnd,
    const WCHAR    *ptzPrefix,
    const char     *pszVersion,
    const WCHAR    *ptzSuffix)
{
    #define         MAILING_ADDRESS_LEN (512)
    WCHAR          *ptzBuff;
    unsigned        nSrcLen = 0;
    unsigned        nLen;
    int             iRet;
    unsigned        nBuffLen;
    const char     *pszPos = pszVersion;

    DclAssertReadPtr(pszVersion, 0);

    nBuffLen = strlen(pszVersion);

    if(ptzPrefix)
        nBuffLen += wcslen(ptzPrefix);
    if(ptzSuffix)
        nBuffLen += wcslen(ptzSuffix);

    /*  Add enough bytes for the hard-coded mailing address as used
        below.  Not very exact, but it's Windows, and it's a short
        term allocation...
    */
    nBuffLen += MAILING_ADDRESS_LEN;

    /*  Double for wide chars
    */
    nBuffLen *= 2;

    ptzBuff = DclMemAlloc(nBuffLen);
    if(!ptzBuff)
        return FALSE;

    if(ptzPrefix)
    {
        wcscpy(ptzBuff, ptzPrefix);
        wcscat(ptzBuff, TEXT("\n\n"));
    }
    else
    {
        *ptzBuff = 0;
    }
    nLen = wcslen(ptzBuff);

    /*  Find the end of the PRODUCTNAME string
    */
    while(pszPos[nSrcLen] && pszPos[nSrcLen] != '\n')
        nSrcLen++;

    if(pszPos[nSrcLen])
    {
        const char     *pszLegal;
        const char     *pszAllRights;

        nSrcLen++;

        /*  Put the PRODUCTNAME string into the output buffer
        */
        DclOsAnsiToWcs(ptzBuff+nLen, nBuffLen - nLen, pszPos, nSrcLen);
        nLen += nSrcLen;
        ptzBuff[nLen] = 0;
        pszPos += nSrcLen;

        /*  Find the end of the PRODUCTLEGAL string
        */
        nSrcLen = 0;
        while(pszPos[nSrcLen] && pszPos[nSrcLen] != '\n')
            nSrcLen++;

        /*  Get pointer to the start of the (c) in the copyright
            message, and the start of the "All Rights Reserved"
            message.
        */
        pszLegal = strstr(pszPos, "(c) ");
        pszAllRights = strstr(pszPos, "All ");

        if(pszPos[nSrcLen] && pszLegal && pszLegal < &pszPos[nSrcLen])
        {
          #ifndef _WIN32_WCE
            /*  For CE, eliminate the word "copyright" and just use
                the 'c' in a circle to accommodate displays which
                are short on real estate.
            */
            wcscat(ptzBuff, TEXT("Copyright "));
          #endif

            wcscat(ptzBuff, TEXT("\xA9"));
            nLen = wcslen(ptzBuff);

            /*  Adjust source pointer and remaining length to point
                to the space following "(c)".
            */
            nSrcLen -= (pszLegal - pszPos + 3);
            pszPos = pszLegal + 3;

            if(pszAllRights && pszAllRights < &pszPos[nSrcLen])
            {
                DclOsAnsiToWcs(ptzBuff+nLen, nBuffLen - nLen,  pszPos, pszAllRights - pszPos);
                nLen += pszAllRights - pszPos;
                ptzBuff[nLen] = 0;

              #ifndef _WIN32_WCE
                wcscat(ptzBuff, TEXT("\n"));
                nLen++;
              #endif

                nSrcLen -= pszAllRights - pszPos;
                pszPos = pszAllRights;

              #ifdef _WIN32_WCE
                {
                    static const char   szWW[] = " Worldwide";
                    const char         *pszWorldwide = strstr(pszPos, szWW);
                    const char         *pszCompiled;

                    /*  If using CE, skip the word "worldwide" for real estate
                        reasons.
                    */
                    if(pszWorldwide && pszWorldwide < &pszPos[nSrcLen])
                    {
                        DclOsAnsiToWcs(ptzBuff + nLen, nBuffLen - nLen,  pszPos, pszWorldwide - pszPos);
                        nLen += pszWorldwide - pszPos;

                        wcscpy(ptzBuff+nLen, TEXT(".\n"));
                        nLen += 2;

                        pszPos = pszWorldwide + sizeof(szWW) + 1;
                    }

                    pszCompiled = strstr(pszPos, " -- Compiled");

                    /*  If using CE, skip the compile timestamp info for
                        real estate reasons.
                    */
                    if(pszCompiled)
                    {
                        DclOsAnsiToWcs(ptzBuff + nLen, nBuffLen - nLen,  pszPos, pszCompiled - pszPos);
                        nLen += pszCompiled - pszPos;

                        pszPos = pszCompiled;

                        while(*pszPos && *pszPos != '\n')
                            pszPos++;
                    }
                }
              #endif

                /*  Write everything remaining exactly as is.
                */
                DclOsAnsiToWcs(ptzBuff + nLen, nBuffLen - nLen,  pszPos, -1);
            }
            else
            {
                /*  If the "All Rights Reserved" string was not found
                    just append everything remaining.
                */
                DclOsAnsiToWcs(ptzBuff + nLen, nBuffLen - nLen,  pszPos, -1);
            }
        }
        else
        {
            /*  If the "(c) " string was not found
                just append everything remaining.
            */
            DclOsAnsiToWcs(ptzBuff + nLen, nBuffLen - nLen,  pszPos, -1);
        }
    }
    else
    {
        /*  If a \n was not found just append everything remaining.
        */
        DclOsAnsiToWcs(ptzBuff + nLen, nBuffLen - nLen,  pszPos, -1);
    }

    /*  Note that this data needs to have no more characters than is
        defined in the MAILING_ADDRESS_LEN symbol.
    */
    wcscat(ptzBuff, TEXT("\nDatalight, Inc.\n"));
  #ifdef _WIN32_WCE
    wcscat(ptzBuff, TEXT("21520 30th Dr SE, Suite 110\n"));
  #else
    wcscat(ptzBuff, TEXT("21520 30th Drive SE, Suite 110\n"));
  #endif
    wcscat(ptzBuff, TEXT("Bothell, WA  98021  USA\n\n"));
    wcscat(ptzBuff, TEXT("+1.425.951.8086\n"));
    wcscat(ptzBuff, TEXT("http://www.datalight.com\n"));

    if(ptzSuffix)
    {
        wcscat(ptzBuff, TEXT("\n"));
        wcscat(ptzBuff, ptzSuffix);
    }

    iRet = MessageBox(hWnd, ptzBuff, TEXT("About..."), MB_OK|MB_ICONINFORMATION);

    DclMemFree(ptzBuff);

    if(iRet)
        return TRUE;
    else
        return FALSE;
}



