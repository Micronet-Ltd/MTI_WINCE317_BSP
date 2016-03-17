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

    This module contains the default OS Services layer implementations for
    memory management.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: osmem.c $
    Revision 1.4  2009/04/10 18:08:12Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2009/02/08 00:06:18Z  garyp
    Merged from the v4.0 branch.  Modified to use the newly enhanced 
    Datalight heap manager code.
    Revision 1.2  2007/11/03 23:31:26Z  Garyp
    Added the standard module header.
    Revision 1.1  2006/02/09 18:30:16Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>
#include <dlheap.h>


/*-------------------------------------------------------------------
    Public: DclOsMemAlloc()

    Allocate a block of memory.  All buffers returned by this
    function are required to be aligned on DCL_ALIGNSIZE
    boundaries.

    Parameters:
        ulSize - number of bytes to allocate

    Return Value:
        Returns a pointer to allocated memory, or NULL if failure.
-------------------------------------------------------------------*/
void * DclOsMemAlloc(
    D_UINT32                ulSize)
{
    static D_BOOL           fInitialized = FALSE;
    static DCL_ALIGNTYPE    memPool[(DCL_HEAP_SIZE+(DCL_ALIGNSIZE-1))/DCL_ALIGNSIZE];

    DCLPRINTF(2, ("DclOsMemAlloc() size=%lX\n", ulSize));

    DclAssert(ulSize);

    if(!fInitialized)
    {
        DCLPRINTF(1, ("DclOsMemAlloc() Initializing the heap to contain %lU KB\n",
            sizeof(memPool) / 1024));

        if(DclHeapInit(&memPool[0], sizeof(memPool)) != DCLSTAT_SUCCESS)
        {
            DclError();
            return NULL;
        }

        fInitialized = TRUE;
    }

    /*  Note: This code uses our limited functionality heap manager.
        DclHeapInit() MUST have already been called to initialize
        the pool prior to this point.
    */

    return DclHeapAlloc(ulSize);
}


/*-------------------------------------------------------------------
    Public: DclOsMemFree()

    Release a block of memory that was allocated with DclOsMemAlloc().

    Parameters:
        pMem - A pointer to the allocated memory.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMemFree(
    void *      pMem)
{
    DCLSTATUS   dclStat;

    DCLPRINTF(2, ("DclOsMemFree() mem=%P\n", pMem));

    DclAssert(pMem);

    dclStat = DclHeapFree(pMem);

    return dclStat;
}



