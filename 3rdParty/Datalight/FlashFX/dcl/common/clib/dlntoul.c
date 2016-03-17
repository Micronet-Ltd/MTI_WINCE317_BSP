/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

    The functions in this module are used internally by Datalight products.
    Typically these functions are used rather than the similarly named,
    general Standard C library functions.  Typically this is done for
    compatibility, portability, and code size reasons.

    These functions are NOT intended to be complete, 100% ANSI C compatible
    implementations, but rather are designed address specific needs.

    See the specific function headers for more information about departures
    from the ANSI standard, and missing or extended functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlntoul.c $
    Revision 1.6  2012/03/01 19:23:56Z  garyp
    Documentation fixes -- no functional changes.
    Revision 1.5  2011/12/07 18:44:53Z  daniel.lewis
    Moved the hex digit macros into dlmacros.h.
    Revision 1.4  2010/09/14 21:30:59Z  garyp
    Updated DclNtoUL() to support negative numbers.
    Revision 1.3  2007/12/18 19:22:14Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.1  2006/11/29 23:18:04Z  Garyp
    Initial revision
 ---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclNtoUL()

    Convert the ASCII number pointed to by pachNum to a D_UINT32 
    value.  The number may be hex or decimal.  Hex numbers must be
    prefixed by '0x', and they may be upper or lower case.  The 
    conversion process will stop with the first non hex or decimal
    digit.

    If the number is negative (the first character is a '-' sign),
    the value will be range checked and returned as the equivalent
    unsigned value.

    Parameters:
        pachNum - A pointer to the ASCII number to convert
        pulNum  - A pointer to the D_UINT32 location to store the
                  result.  This value will be modified on return only
                  if the function succeeds and the returned pointer
                  is valid (not NULL).

    Return Value:
        Returns a pointer to the byte following the converted number,
        or NULL to indicate failure.
-------------------------------------------------------------------*/
const char * DclNtoUL(
    const char *pachNum,
    D_UINT32   *pulNum)
{
    D_BOOL      fNegative = FALSE;

    DclAssertReadPtr(pachNum, 0);
    DclAssertWritePtr(pulNum, sizeof(*pulNum));

    if(pachNum[0] == '-')
    {
        fNegative = TRUE;
        pachNum++;
    }

    /*  Hex numbers must be prefixed with '0x'.
    */
    if(pachNum[0] == '0' && DclToUpper(pachNum[1]) == 'X')
    {
        pachNum += 2;

        if(!DclIsDigit(*pachNum) && !DCLISHEXDIGIT(*pachNum))
            return NULL;

        pachNum = DclHtoUL(pachNum, pulNum);
    }
    else if(DclIsDigit(*pachNum))
    {
        D_UINT32    ulTemp;

        ulTemp = DclAtoL(pachNum);

        while(DclIsDigit(*pachNum))
            pachNum++;

        if(fNegative)
        {
            /*  Fail if the number is out of range.
            */
            if(ulTemp > D_INT32_MAX)
                return NULL;

            ulTemp = -((D_INT32)ulTemp);
        }

        *pulNum = ulTemp;
    }
    else
    {
        /*  Return an error if there is not at least one hex or
            decimal digit.
        */
        return NULL;
    }

    return pachNum;
}
