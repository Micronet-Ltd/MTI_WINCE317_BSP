/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains an insertion sort algorithm for an array of D_UINT32
    elements.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsortinsert32.c $
    Revision 1.1  2011/03/23 23:19:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclSortInsertion32()

    Perform an insertion sort on an array of D_UINT32 items.

    Parameters:
        pulList   - The list to sort.
        nElements - The number of elements in pulList.

    Return Value:
        Returns a count from 0 to nElements, indicating the level
        of unsortedness.  Returns 0 if the list was already sorted.
-------------------------------------------------------------------*/
size_t DclSortInsertion32(
    D_UINT32   *pulList,
    size_t      nElements)
{
    size_t      nMoves = 0;
    size_t      ii;

    DclAssertWritePtr(pulList, sizeof(*pulList) * nElements);

    for(ii = 1; ii < nElements; ii++)
    {
        size_t      nStart;
        size_t      nLen;
        D_UINT32    ulVal = pulList[ii];

        /*  If this element is greater than or equal to the previous
            element, it's already sorted, so go onto the next.
        */            
        if(ulVal >= pulList[ii - 1])
            continue;

        /*  We have a value which must be inserted into the already
            sorted part of the list.  Binary search it to find the 
            right location.
        */            
        nStart = 0;     /* Start of the sub-list */
        nLen = ii;      /* Length of the sub-list */

        while(nLen > 1)
        {
            size_t  nPos = nLen / 2;

            /*  See if the value should be in the low half of the
                sub-list, or the high half.
            */                
            if(ulVal < pulList[nStart + nPos])
            {
                /*  However, if the value is >= the last item in
                    the low half of the sub-list, we're done.
                */    
                if(ulVal >= pulList[nStart + nPos - 1])
                {
                    nStart += nPos;
                    break;
                }
                
                /*  The item belongs in the low half, so reduce
                    the sub-list size.
                */    
                nLen = nPos;    
            }
            else
            {
                /*  The item belongs in the high half, so adjust
                    the sub-list start and length, accounting for
                    list lengths which are not even.
                */    
                nStart += nPos;
                nLen = nPos + (nLen & 1);
            }
        };

        DclMemMove(&pulList[nStart+1], &pulList[nStart], (ii - nStart) * sizeof(*pulList));

        pulList[nStart] = ulVal;

        nMoves++;
    }    

    return nMoves;
}


