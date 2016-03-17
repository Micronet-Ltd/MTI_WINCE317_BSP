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

    This module contains the Windows CE OS Services Layer default
    implementations of memory mapping services.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osmapmem.c $
    Revision 1.3  2010/02/12 21:21:14Z  garyp
    Slightly revamped the interface so all the OS specific implementations
    of these services are the same.
    Revision 1.2  2009/03/23 01:07:52Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.3  2009/03/23 01:07:52Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.2  2008/11/30 04:25:18Z  garyp
    Updated debug code -- no functional changes.
    Revision 1.1  2008/10/23 18:56:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <ceddk.h>      /* needed for PAGE_SIZE */

#include <dcl.h>
#include <dlosmapmem.h>

#define PHYSMEM_MAX UINT64SUFFIX(0x0000010000000000)


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
    void           *pVirt;
    D_UINT32        ulFlags;
    D_UINT64        ullPhysAlignedBase;
    unsigned        nOffset;
    size_t          nAlignedLength;

    DCLPRINTF(1, ("DclOsVirtualPointerMap() PhysicalOffset=%llX Length=%u Cacheable=%u\n",
        ullPhysOffset, nLength, fCacheable));

    /*  Don't assert pPhysical, as '0' may be a legitimate area to map...
    */
    DclAssert(ppVirtual);
    DclAssert(nLength);
    DclAssert(ullPhysOffset < PHYSMEM_MAX);
    DclAssert(nLength <= PHYSMEM_MAX - ullPhysOffset);

    ullPhysAlignedBase = ullPhysOffset & ~(PAGE_SIZE - 1);
    nOffset = (unsigned)(ullPhysOffset & (PAGE_SIZE - 1));

    /*  Add an extra page if necessary
    */
    if(nOffset)
        nLength += PAGE_SIZE;

    nAlignedLength = nLength & ~(PAGE_SIZE - 1);

    /*  The flash address and length were both specified, map it and
        record the virtual address.
    */
    pVirt = VirtualAlloc(0, nAlignedLength, MEM_RESERVE, PAGE_NOACCESS);
    if(!pVirt)
    {
        DCLPRINTF(1, ("DclOsVirtualPointerMap() VirtualAlloc() failed, LastError=%lX\n", GetLastError()));

        return DCLSTAT_MEM_VIRTUALMAPFAILED1;
    }

    ulFlags = PAGE_READWRITE | PAGE_PHYSICAL;
    if(!fCacheable)
         ulFlags |= PAGE_NOCACHE;

    /*  NOTE: The following cast and shift assume the pointer of specific size
    */
    if(!VirtualCopy(pVirt, (LPVOID)(ullPhysAlignedBase >> 8), nAlignedLength, ulFlags))
    {
        DCLPRINTF(1, ("DclOsVirtualPointerMap() VirtualCopy() failed, LastError=%lX\n", GetLastError()));

        VirtualFree(pVirt, 0, MEM_RELEASE);

        return DCLSTAT_MEM_VIRTUALMAPFAILED2;
    }

    (char*)pVirt += nOffset;

    *ppVirtual = pVirt;

    DCLPRINTF(2, ("DclOsVirtualPointerMap() assigned virtual address %P for physical offset %llX\n",
        *ppVirtual, ullPhysOffset));

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsVirtualPointerUnmap()

    Unmap a virtual pointer which was previously mapped with 
    DclOsVirtualPointerMap() and release the physical address
    range.
						
    *Note* -- The ullPhysOffset value is not used in the Windows CE
    abstraction of this service, and will be ignored.
  
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
    DclAssert(pVirtual);
    DclAssert(nLength);   

    (void)ullPhysOffset;

    /*  Note that the assert for pVirtual above is appropriate for CE since
        it will never return a NULL virtual pointer.  However for other
        environments this <may> not be the case.
    */

    if(!VirtualFree(pVirtual, 0, MEM_RELEASE))
    {
        DCLPRINTF(1, ("DclOsVirtualPointerUnmap() VirtualFree() for %P, PhysOffset=%llX, length %u failed, LastError=%lX\n",
            pVirtual, ullPhysOffset, nLength, GetLastError()));

        return DCLSTAT_MEM_VIRTUALUNMAPFAILED;
    }

    return DCLSTAT_SUCCESS;
}





