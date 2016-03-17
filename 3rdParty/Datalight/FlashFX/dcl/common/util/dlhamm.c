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

    A simple Hamming code implementation that can correct single-bit errors.

    This code is based on what I would call a "mechanical" understanding
    of how a Hamming code works.  It really deserves a good explanation,
    better than I have time for right now.  If you would like to know
    about as much as I know about Hamming codes, see:

    http://www.nuvisionmiami.com/books/asm/workbook/error_correcting.htm
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlhamm.c $
    Revision 1.4  2007/12/18 04:04:01Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/11/01 19:54:19Z  Garyp
    Eliminated the inclusion of limits.h.
    Revision 1.1  2006/02/26 20:20:16Z  Garyp
    Initial revision
    ---------------------
    Bill Roman 2003-03-17
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclHammingCalculate

    Calculate Hamming code check bits or error syndrome for a
    buffer of data.

    Parameters:
        pBuffer  - A pointer to data for which to calculate the
                   Hamming code check bits
        nLength  - The buffer length
        ulParity - Zero to calculate check bits, or previously
                   calculated check bits to use in calculating an
                   error syndrome.

    Return:
        Check bits for a future call to correct a single-bit error,
        or an error syndrome to correct an error now.
-------------------------------------------------------------------*/
unsigned long DclHammingCalculate(
    const void             *pBuffer,
    size_t                  nLength,
    unsigned long           ulParity)
{
    const unsigned char    *p = pBuffer;
    int                     bit = 1;

    while(nLength--)
    {
        unsigned char   mask;
        unsigned char   byte = *p++;

        for(mask = 1; mask; mask <<= 1)
        {
            while(DCLISPOWEROF2(bit))
                ++bit;

            if(byte & mask)
            {
                ulParity ^= bit;
            }
            ++bit;
        }
    }

    return ulParity;
}


/*-------------------------------------------------------------------
    Public: DclHammingCorrect

    Correct a single-bit error.

    Parameters:
        pBuffer - A pointer to data in which to correct a single-bit
                  error.
        nLength - The buffer length
        ulCheck - The non-zero error syndrome from a call to
                  DclHammingCalculate() with previously calculated
                  check bits.

    Return:
        None
-------------------------------------------------------------------*/
void DclHammingCorrect(
    void           *pBuffer,
    size_t          nLength,
    unsigned long   ulCheck)
{
    int             skip;
    unsigned long   mask;

    if(DCLISPOWEROF2(ulCheck))
        return;

    skip = 2;
    for(mask = 4; mask < ulCheck; mask <<= 1)
    {
        ++skip;
    }

    ulCheck -= skip + 1;

    /*  Note that in the case of multi-bit errors or complete garbage
        data, it is possible the ulCheck code will index outside of
        the intended range.
    */
    if(ulCheck / CHAR_BIT < nLength)
    {
        ((unsigned char *)pBuffer)[(size_t) (ulCheck / CHAR_BIT)] ^=
            (unsigned char)(1 << (int)(ulCheck % CHAR_BIT));
    }

}
