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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlutf8stricmp.c $
    Revision 1.2  2009/05/27 22:11:54Z  garyp
    Marked the function protected.
    Revision 1.1  2009/05/20 19:26:08Z  keithg
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Protected: DclUtf8StrICmp()

    This function implements a case-insensitive comparison of two
    null terminated strings.

    Parameters:
        pacBuffer1     - A pointer to a char buffer containing UTF-8
                         string 1.
        pacBuffer2     - A pointer to a char buffer containing UTF-8
                         string 2.

    Return Value:
        Returns 0 if the strings are equal.  If the strings do not
        match, non-zero is returned.  If the strings are within the
        same language set, 1 will indicate that string 1 is greater
        than string 2 and -1 if string 2 is greater than string 1.
-------------------------------------------------------------------*/
int DclUtf8StrICmp(
    const char     *pacBuffer1,
    const char     *pacBuffer2)
{
    D_UINT32        ulCodePoint1;
    D_UINT32        ulCodePoint2;
    int             iResult = 0;
    size_t          nBytes1;
    size_t          nBytes2;

    /*  The size listed here is the maximum size that a valid UTF-8
        byte sequence can be.  It is necessary for the UTF-8 helper
        functions and because this is an unsafe function which does
        not know the size of the strings.
    */
    const size_t    cnMaxBytes = 4;

    DclAssert(pacBuffer1);
    DclAssert(pacBuffer2);

    /*  This is an unsafe operation, we do not know how many bytes
        are actually available.
    */
    while(TRUE)
    {

        ulCodePoint1 = DclUtf8ToScalar(pacBuffer1, cnMaxBytes, &nBytes1);
        ulCodePoint1 = DclUtfScalarToLower(ulCodePoint1);

        ulCodePoint2 = DclUtf8ToScalar(pacBuffer2, cnMaxBytes, &nBytes2);
        ulCodePoint2 = DclUtfScalarToLower(ulCodePoint2);

        /*  Check for mismatch - At least one character must be valid
            and the characters must be different from each other. This
            will handle the cases where one string has an invalid
            UTF-8 sequences.
        */
        if( (ulCodePoint1 != ulCodePoint2) && (nBytes1 || nBytes2) )
        {
            if(ulCodePoint1 > ulCodePoint2)
            {
                iResult = 1;
                break;
            }
            else
            {
                iResult = -1;
                break;
            }
        }

        /* Terminating string */
        if(ulCodePoint1 == 0)
        {
            iResult = 0;
            break;
        }

        pacBuffer1      += nBytes1;
        pacBuffer2      += nBytes2;
    }

    return iResult;
}


