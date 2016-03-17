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
    Revision 1.4  2009/04/10 20:52:20Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/10/04 02:50:26Z  Garyp
    Corrected some bogus sample code.
    Revision 1.1  2005/10/02 03:57:46Z  Pauli
    Initial revision
    Revision 1.3  2005/08/21 11:38:59Z  garyp
    Documentation update.
    Revision 1.2  2005/08/03 19:14:22Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/03 09:35:02Z  pauli
    Initial revision
    Revision 1.4  2004/12/31 00:11:02Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/01/14 20:04:58Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.3  2004/01/14 20:04:58  garyp
    Changed _sysmalloc() to take a D_UINT32 rather than a size_t.
    Revision 1.2.1.2  2003/12/30 03:38:06Z  garyp
    Added some commented out code for future reference.
    Revision 1.2  2003/04/21 05:33:38Z  garyp
    Modified _sysmalloc() to take a size_t.
    Revision 1.1  2003/04/14 00:34:44Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlapiprv.h>


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
    D_UINT32    ulSize)
{
    void *      pMem;

    DclAssert(ulSize);

#if 0

    /*  Some MS docs indicate that by doing the MEM_RESERVE and MEM_COMMIT
        separately, the memory will be allocated from outside process
        address space and allow larger allocations.  Tested this on CEPC
        under CE 4.0 and it does not seem to work (gp 12/29/03).
    */
    if(ulSize >= (2 * 1024L * 1024L))
    {
        pMem = VirtualAlloc(NULL, ulSize, MEM_RESERVE, PAGE_READWRITE);

        if(!pMem)
        {
            DCLPRINTF(1, ("VirtualAlloc() failed(a), error=%lu\n", GetLastError()));
            return NULL;
        }

        pMem = VirtualAlloc(pMem, ulSize, MEM_COMMIT, PAGE_READWRITE);

        if(!pMem)
        {
            DCLPRINTF(1, ("VirtualAlloc() failed(b), error=%lu\n", GetLastError()));
            return NULL;
        }
    }
    else
#endif
    {
        pMem = malloc(ulSize);
    }

    DclAssert(((D_UINT32)pMem & (DCL_ALIGNSIZE - 1)) == 0);

    return pMem;
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
    DclAssert(pMem);

    free(pMem);

    return DCLSTAT_SUCCESS;
}
