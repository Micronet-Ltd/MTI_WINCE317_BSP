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
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This module contains the default read routine used in NOR FIMs.  Since
    NOR flash reads just like RAM, this routine is generic and straight-
    forward.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: norread.c $
    Revision 1.6  2009/04/06 14:28:22Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2008/02/03 01:48:49Z  keithg
    comment updates to support autodoc.
    Revision 1.4  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/08/30 17:01:44Z  pauli
    Updated to use the new FfxFimNorWindowMap() function.
    Revision 1.2  2006/02/03 20:25:16Z  Garyp
    Modified to use new FfxHookWindowMap/Size() functions.  Updated debugging
    code.
    Revision 1.1  2005/10/06 05:47:20Z  Pauli
    Initial revision
    Revision 1.3  2005/04/23 06:30:41Z  garyp
    Minor header changes.
    Revision 1.2  2004/12/30 23:17:52Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2003/12/17 01:52:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fimdev.h>
#include "norread.h"
#include "nor.h"


/*-------------------------------------------------------------------
    Public: FfxFimNorRead()

    Performs transfers data from the flash into the client
    buffer.  This is performed by a simple call to memmove,
    usually provide by the C library.

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use
        ulStart - Starting offset in bytes to begin the access
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxFimNorRead(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    D_UCHAR        *pcBuffer = (D_UCHAR*)pBuffer;

    DclAssert(pEMI);
    DclAssert(ulStart < MAX_ARRAY);
    DclAssert(uLength);
    DclAssert(pBuffer);

    while(uLength)
    {
        void       *pMedia;
        D_UINT32    ulWindowSize;

        /*  Get the media pointer and the max size we can access with it
        */
        ulWindowSize = FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia);
        if(!ulWindowSize)
            break;

        while(ulWindowSize && uLength)
        {
            D_UINT32    ulThisLength = DCLMIN(uLength, ulWindowSize);

            /*  Move this chunk of data
            */
            DclMemCpy(pcBuffer, pMedia, ulThisLength);

            uLength         -= (D_UINT16)ulThisLength;
            ulWindowSize    -= ulThisLength;
            ulStart         += ulThisLength;
            pcBuffer        += ulThisLength;

            DclAssert(ulStart);
            DclAssert(pcBuffer);
        }
    }

    return (uLength == 0);
}


