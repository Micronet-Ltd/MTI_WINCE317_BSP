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

    This module contains the Windows CE Bootloader OS Services Layer default
    implementations of memory mapping services.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osmapmem.c $
    Revision 1.4  2010/04/22 23:29:21Z  garyp
    Added a typecast to get things to build cleanly.
    Revision 1.3  2010/02/12 21:21:14Z  garyp
    Slightly revamped the interface so all the OS specific implementations
    of these services are the same.
    Revision 1.2  2009/12/03 02:58:35Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.1  2008/11/14 20:51:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlosmapmem.h>

typedef DWORD   PHYSMEM_OFFSET;
#define         PHYSMEM_MAX 0xFFFFFFFF


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
    PHYSMEM_OFFSET  physOffset = (PHYSMEM_OFFSET)ullPhysOffset;

    DCLPRINTF(1, ("DclOsVirtualPointerMap() PhysicalOffset=%lX Length=%u Cacheable=%u (STUBBED!)\n",
        physOffset, nLength, fCacheable));

    /*  Don't assert pPhysical, as '0' may be a legitimate area to map...
    */
    DclAssert(ppVirtual);
    DclAssert(nLength);
    DclAssert(physOffset == ullPhysOffset);
    DclAssert(nLength <= PHYSMEM_MAX - physOffset);

    (void)nLength;
    (void)fCacheable;

    /*  OALPAtoVA(), at least in some BSPs, appears to function only when
        the MMU is on, which in some environments such as WinMobile, it is
        on and off at different points in the boot process.  Until we have
        a standard way of detecting at run-time whether it is on or off (or
        until we know that OALPAtoVA() will simply return the physical
        address if the MMU is off) we do nothing in this code and require
        that the issue be handled in project code.
    */        
/*
    *ppVirtual = OALPAtoVA(pPhysical, fCacheable);
*/

    *ppVirtual = (void*)physOffset;

    DCLPRINTF(2, ("DclOsVirtualPointerMap() assigned virtual address %P for physical offset %lX\n",
        *ppVirtual, physOffset));

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsVirtualPointerUnmap()

    Unmap a virtual pointer which was previously mapped with 
    DclOsVirtualPointerMap() and release the physical address
    range.
						
    *Note* -- The ullPhysOffset value is not used in the Windows CE
    Bootloader abstraction of this service, and will be ignored.

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





