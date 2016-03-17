/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains functions for converting ECCs between the Datalight
    canonical format and that used by the OMAP 3530 hardware ECC calculator.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dleccomap35x.c $
    Revision 1.2  2010/01/09 17:58:43Z  garyp
    Documentation updates -- no functional changes.
    Revision 1.1  2009/10/06 17:44:34Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#define ECC_BYTES   (3)


/*-------------------------------------------------------------------
    Public: DclEccOmap35xToCanonical()

    Convert an ECC in the format provided by the OMAP 3530 hardware,
    into the Datalight canonical 32-bit format.  See dlecc.c and the
    3530 specification for details.

    Note that the hardware ECC for data which is all zeros or all
    ones is all zeros, which is opposite of what the Datalight 
    canonical format uses.

    Parameters:
        pabECC     - A pointer to the three ECC bytes provided by
                     the hardware.

    Return Value:
        Returns the ECC in the Datalight canonical format.
-------------------------------------------------------------------*/
D_UINT32 DclEccOmap35xToCanonical(
    const D_BUFFER *pabECC)
{
    D_UINT32        ulEven;
    D_UINT32        ulOdd;
    D_UINT32        ulECC = 0;
    unsigned        nIndex = 0;

    DclAssertReadPtr(pabECC, ECC_BYTES);

    /*  Get the even and odd parity bits out of the 3 bytes
    */
    ulEven = pabECC[0] | (((D_UINT32)(pabECC[1] & 0xF)) << 8);
    ulOdd = ((D_UINT32)pabECC[1] >> 4) | (((D_UINT32)(pabECC[2])) << 4);

    /*  The bits are all inverted...
    */
    ulEven = ~ulEven;
    ulOdd = ~ulOdd;

    while(nIndex < 24)
    {
        ulECC |= (ulEven & 1) << nIndex;
        ulECC |= ((ulOdd & 1) << (nIndex+1));
        ulEven >>= 1;
        ulOdd >>= 1;
        nIndex += 2;
    }

    return ulECC;
}


/*-------------------------------------------------------------------
    Public: DclEccCanonicalToOmap35x()

    Convert an ECC in the Datalight canonical format to the format
    used by the OMAP 3530 hardware.  See dlecc.c and the 3530
    specification for details.

    Parameters:
        ulECC       - The ECC in Datalight canonical format.
        pabECC      - A pointer to a three byte buffer to receive
                      the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclEccCanonicalToOmap35x(
    D_UINT32        ulECC,
    D_BUFFER       *pabECC)
{
    D_UINT32        ulEven = 0;
    D_UINT32        ulOdd = 0;
    unsigned        nIndex = 0;

    DclAssertWritePtr(pabECC, ECC_BYTES);

    /*  Get the even and odd parity bits out of our interleaved format
        and into separate variables.
    */                
    while(nIndex < 12)
    {
        ulEven |= (ulECC & 1) << nIndex;
        ulECC >>= 1;
        ulOdd |= (ulECC & 1) << nIndex;
        ulECC >>= 1;
        nIndex++;
    }

    /*  The bits are all inverted...
    */
    ulEven = ~ulEven;
    ulOdd = ~ulOdd;

    /*  Get the right bits into the correct bytes.
    */
    pabECC[0] = (D_BYTE)ulEven;
    pabECC[1] = (D_BYTE)((ulEven >> 8) & 0xF) | ((ulOdd & 0xF) << 4);
    pabECC[2] = (D_BYTE)(ulOdd >> 4);

    return;
}


