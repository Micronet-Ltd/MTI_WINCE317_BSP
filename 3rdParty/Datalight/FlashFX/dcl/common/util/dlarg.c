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

    This module contains functions for parsing an argument string.  These
    functions are typically used to make it easier to adapt code that was
    originally written to expect argc/argv style arguments to systems
    where the arguments are specified as a single string.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: dlarg.c $
    Revision 1.3  2007/12/18 04:51:51Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2005/10/02 03:58:36Z  Pauli
    Initial revision
    Revision 1.1  2005/10/02 04:58:36Z  Garyp
    Initial revision
    Revision 1.1  2005/09/21 07:19:00Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  Prototypes for internal functions
*/
static const char  *EatWhiteSpace(const char *pszString);
static D_UINT16     FindArgLen(const char *pszString);


/*-------------------------------------------------------------------
    Public: DclArgCount

    Get the number of arguments in a string.  This function returns
    the number of arguments in the specified string.

    Parameters:
        pszArgString    - A pointer to the null-terminated argument
                          string.

    Return:
        Returns the number of arguments.
-------------------------------------------------------------------*/
D_UINT16 DclArgCount(
    const char     *pszArgString)
{
    D_UINT16        uArgCount = 0;
    D_UINT16        uArgLen;

    if(!pszArgString)
        return 0;

    pszArgString = EatWhiteSpace(pszArgString);

    while(*pszArgString)
    {
        uArgLen = FindArgLen(pszArgString);
        DclAssert(uArgLen);

        pszArgString += uArgLen;

        uArgCount++;

        pszArgString = EatWhiteSpace(pszArgString);
    }

    return uArgCount;
}


/*-------------------------------------------------------------------
    Public: DclArgRetrieve

    Retrieve the specified argument.  This function returns the
    specified argument in the supplied buffer.

    Parameters:
        pszArgString    - A pointer to the null-terminated argument
                          string.
        uArgNum         - The argument number (relative to 1)
        uBuffLen        - The output buffer length
        pachBuffer      - A pointer to the output buffer

    Return:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL DclArgRetrieve(
    const char     *pszArgString,
    D_UINT16        uArgNum,
    D_UINT16        uBuffLen,
    char           *pachBuffer)
{
    D_UINT16        uArgLen;

    DclAssert(pszArgString);
    DclAssert(uBuffLen);
    DclAssert(pachBuffer);
    DclAssert(uArgNum);

    pszArgString = EatWhiteSpace(pszArgString);

    while(*pszArgString)
    {
        uArgLen = FindArgLen(pszArgString);
        DclAssert(uArgLen);

        uArgNum--;

        if(uArgNum == 0)
        {
            if(uArgLen + 1 > uBuffLen)
                return FALSE;

            DclMemCpy(pachBuffer, pszArgString, uArgLen);

            pachBuffer[uArgLen] = 0;

            return TRUE;
        }

        pszArgString += uArgLen;

        pszArgString = EatWhiteSpace(pszArgString);
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    Public: DclArgClear

    Clear the specified argument.  This function clears the specified
    argument to spaces.

    Parameters:
        pszArgString    - A pointer to the null-terminated argument
                          string.
        uArgNum         - The argument number (relative to 1)

    Return:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL DclArgClear(
    char       *pszArgString,
    D_UINT16    uArgNum)
{
    D_UINT16    uArgLen;

    DclAssert(pszArgString);
    DclAssert(uArgNum);

    pszArgString = (char *) EatWhiteSpace(pszArgString);

    while(*pszArgString)
    {
        uArgLen = FindArgLen(pszArgString);
        DclAssert(uArgLen);

        uArgNum--;

        if(uArgNum == 0)
        {
            DclMemSet(pszArgString, ' ', uArgLen);

            return TRUE;
        }

        pszArgString += uArgLen;

        pszArgString = (char *) EatWhiteSpace(pszArgString);
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    Local: EatWhiteSpace
-------------------------------------------------------------------*/
static const char *EatWhiteSpace(
    const char *pszString)
{
    while(*pszString)
    {
        if((*pszString == ' ') || (*pszString == '\t'))
            pszString++;
        else
            break;
    }

    return pszString;
}


/*-------------------------------------------------------------------
    Local: FindArgLen
-------------------------------------------------------------------*/
static D_UINT16 FindArgLen(
    const char *pszString)
{
    D_UINT16    uArgLen = 0;

    while(*pszString)
    {
        /*  If the string includes any single or double quoted
            information, skip past it.
        */
        if((*pszString == '"') || (*pszString == '\''))
        {
            char chr = *pszString;

            pszString++;
            uArgLen++;

            while(*pszString != chr)
            {
                if(!*pszString)
                    return uArgLen;

                pszString++;
                uArgLen++;
            }
        }

        if((*pszString == ' ') || (*pszString == '\t'))
            break;

        pszString++;
        uArgLen++;
    }

    return uArgLen;
}
