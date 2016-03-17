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

    This module contains the DCL path utility functions:

       DclAbsolutePath()

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                 Revision History
    $Log: dlpath.c $
    Revision 1.3  2007/12/18 03:55:51Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2006/03/16 02:47:26Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclAbsolutePath

    Resolve a relative path string to an absolute path.

    Parameters:
        pszRelPath      - The relative path to resolve.
        pszCWD          - An optional current working directory to use if the
                          relative path does not contain the root.  If this is
                          NULL the root is implied.
        pszSeparators   - A NULL terminated string of path separators.
                          The first character in the list is used when
                          creating the absolute path.
        pszAbsPath      - A buffer to receive the absolute path.

        Note: The CWD must be an absolute path and should not have
              a trailing path separator.

    Return:
        Returns TRUE if the path was converted successfully,
        otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL DclAbsolutePath(
    const char     *pszRelPath,
    const char     *pszCWD,
    const char     *pszSeparators,
    char           *pszAbsPath)
{
    const char     *pCurrRel = pszRelPath;
    char           *pCurrAbs = pszAbsPath;

    /*  validate parameters
    */
    DclAssert(pszRelPath);
    DclAssert(pszAbsPath);
    DclAssert(pszSeparators);
    if(!pszRelPath || !pszAbsPath || !pszSeparators)
    {
        return FALSE;
    }

    /*  If a CWD is specified that contains more than just the root and
        the raw path does not include the root, then add the current
        working directory.  To avoid duplicate path separators and to
        simplify the code below, do not add the CWD if it is just the root.
    */
    if(pszCWD && pszCWD[0] && pszCWD[1] &&
       !DclStrChr(pszSeparators, pszRelPath[0]))
    {
        DclStrCpy(pCurrAbs, pszCWD);
        pCurrAbs = &pCurrAbs[DclStrLen(pCurrAbs)];
    }

    /*  Copy all the path segments to the absolute path, stripping off the
        relative segments as we go.
    */
    while(pCurrRel[0])
    {
        /*  Skip path separators.
        */
        while(pCurrRel[0] && DclStrChr(pszSeparators, pCurrRel[0]))
        {
            pCurrRel++;
        }
        if(!pCurrRel[0])
        {
            break;
        }

        /*  Check for Dot and DotDot
        */
        if(pCurrRel[0] == '.')
        {
            /*  If this is a "." or "./" then ignore it.
            */
            if(!pCurrRel[1] || DclStrChr(pszSeparators, pCurrRel[1]))
            {
                pCurrRel++;
                continue;
            }

            /*  If this is a ".." or "../" then remove the previous directory
                from the absolute path.
            */
            if((pCurrRel[1] == '.') && (!pCurrRel[2] ||
                DclStrChr(pszSeparators, pCurrRel[2])))
            {
                /*  Can't back-up past the root.
                */
                if((pCurrAbs == pszAbsPath) || (pCurrAbs == &pszAbsPath[1]))
                {
                    return FALSE;
                }

                pCurrRel += 2;
                pCurrAbs--;
                while((pCurrAbs >= pszAbsPath) &&
                      !DclStrChr(pszSeparators, pCurrAbs[0]))
                {
                    pCurrAbs--;
                }
                if(pCurrAbs < pszAbsPath)
                {
                    /*  This should never happen.
                    */
                    DclError();
                    return FALSE;
                }
                continue;
            }
        }

        /*  Must be a real name.
            Add a path separator and copy the name.
        */
        pCurrAbs[0] = pszSeparators[0];
        pCurrAbs++;
        while(pCurrRel[0] && !DclStrChr(pszSeparators, pCurrRel[0]))
        {
            pCurrAbs[0] = pCurrRel[0];
            pCurrAbs++;
            pCurrRel++;
        }
    }

    /*  If the raw path didn't resolve to anything but the root, add it now.
    */
    if(pCurrAbs == pszAbsPath)
    {
        pCurrAbs[0] = pszSeparators[0];
        pCurrAbs++;
    }

    /*  Terminate the absolute path and return success.
    */
    pCurrAbs[0] = 0;
    return TRUE;
}
