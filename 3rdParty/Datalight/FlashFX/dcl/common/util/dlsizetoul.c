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

    This module contains general ASCII to binary conversion function which
    are used by various tools.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsizetoul.c $
    Revision 1.1  2009/03/05 07:37:06Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclSizeToUL()

    Convert an ASCII hex or decimal number, which may may have a
    "B", "KB", or "MB" suffix (case insensitive), and converts it
    to a binary value.  Hex numbers must be prefixed with "0x".

    *If there is no postfix, KB is assumed!*

    May fail due to bad formatting or overflow.

    Parameters:
        pachNum   - A pointer to the ASCII number to convert.
        pulResult - A pointer to a D_UINT32 in which to place the
                    result.

    Return Value:
        Returns a pointer to the byte following the string, or NULL
        to indicate an error.  In the event of an error, *pulResult
        will not be modified.
-------------------------------------------------------------------*/
const char * DclSizeToUL(
    const char *pachNum,
    D_UINT32   *pulResult)
{
    D_UINT32    ulResult;

    DclAssert(pachNum);
    DclAssert(pulResult);

    /*  Do the basic hex/decimal conversion
    */
    pachNum = DclNtoUL(pachNum, &ulResult);
    if(pachNum)
    {
        if(DclToUpper(*pachNum) == 'B')
        {
            pachNum++;
        }
        else if((DclToUpper(*pachNum) == 'M') && (DclToUpper(*(pachNum+1)) == 'B'))
        {
            pachNum += 2;

            if(ulResult > D_UINT32_MAX / (1024UL * 1024UL))
                return NULL;

            ulResult *= (1024UL * 1024UL);
        }
        else
        {
            /*  The number is either postfixed with "KB" or something
                else (we don't care), but we must increment the pointer
                if it is something recognize.
            */
            if((DclToUpper(*pachNum) == 'K') && (DclToUpper(*(pachNum+1)) == 'B'))
                pachNum += 2;

            /*  "B" or "MB" were not specified, so it must be "KB"
            */
            if(ulResult > D_UINT32_MAX / 1024UL)
                return NULL;

            ulResult *= 1024UL;
        }

        *pulResult = ulResult;
    }

    return (char*)pachNum;
}

