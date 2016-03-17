/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains the RTOS OS Services Layer default implementations
    of memory mapping services.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osmapmem.c $
    Revision 1.4  2010/02/14 21:10:50Z  garyp
    Try that last "fix" again.
    Revision 1.3  2010/02/14 19:11:59Z  garyp
    Added an extra typecast to avoid warnings in some environments.
    Revision 1.2  2010/02/12 21:21:14Z  garyp
    Slightly revamped the interface so all the OS specific implementations
    of these services are the same.
    Revision 1.1  2009/12/03 02:56:28Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlosmapmem.h>


/*-------------------------------------------------------------------
    Public: DclOsVirtualPointerMap()

    Map a virtual pointer to a particular range of physical memory.
    The virtual pointer must be unmapped using the function 
    DclOsVirtualPointerUnmap().

    *Note* -- The sum of ullPhysOffset and nLength may not exceed the
    operating system's maximum amount of mappable memory.

    Parameters:
        ppVirtual     - A pointer to a buffer to receive the virtual
                        address.
        ullPhysOffset - The byte offset of the physical memory offset
                        to map.  Does not need to be aligned.
        nLength       - The length of the area to map.  Does not need
                        to be aligned.
        fCacheable    - A flag indicating whether the virtual mapping
                        may be cached.

    Return Value:
        A DCLSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
DCLSTATUS DclOsVirtualPointerMap(
    void          **ppVirtual,
    D_UINT64        ullPhysOffset,
    size_t          nLength,
    D_BOOL          fCacheable)
{
    DCLPRINTF(1, ("DclOsVirtualPointerMap() PhysicalOffset=%llX Length=%u Cacheable=%u (STUBBED!)\n",
        ullPhysOffset, nLength, fCacheable));

    /*  Don't assert pPhysical, as '0' may be a legitimate area to map...
    */
    DclAssert(ppVirtual);
    DclAssert(nLength);

    (void)nLength;
    (void)fCacheable;

    /*  Do nothing by default for the RTOS kit.  If the RTOS being used
        requires memory mapping, this service must be implemented,
        otherwise just return the physical offset as a pointer.

        Double-cast this to placate some compilers which think that a
        single cast is not enough...
    */       
    
/*  When we have this... 
    DclAssert(ullPhysOffset <= D_UINTPTR_MAX);  
*/
    
    *ppVirtual = (void*)(D_UINTPTR)ullPhysOffset;

    DCLPRINTF(2, ("DclOsVirtualPointerMap() assigned virtual address %P for physical offset %llX\n",
        *ppVirtual, ullPhysOffset));

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsVirtualPointerUnmap()

    Unmap a virtual pointer which was previously mapped with 
    DclOsVirtualPointerMap() and release the physical address
    range.

    Parameters:
        pVirtual      - The virtual address to unmap.
        ullPhysOffset - The physical memory offset to unmap.  This 
                        must match the original physical offset used
                        when the memory was mapped.
        nLength       - The length of the area to unmap.  This must
                        match the original length used when the 
                        memory was mapped.

    Return Value:
        A DCLSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
DCLSTATUS DclOsVirtualPointerUnmap(
    void           *pVirtual,
    D_UINT64        ullPhysOffset,
    size_t          nLength)
{
    DclAssert(nLength);

    (void)pVirtual;
    (void)nLength;

    return DCLSTAT_SUCCESS;
}





