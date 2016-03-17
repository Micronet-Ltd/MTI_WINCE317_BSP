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

    This module implements the NAND Simulator RAM Storage Mechanism.  This
    mechanism will store simulated NAND data in RAM.

    The RAM Storage Mechanism can be configured to dynamically allocate
    memory or a memory address can be specified.  To specify a memory
    address, you must service the FFXOPT_NANDSIM_RAM option request for
    your project.

    When a memory address is give via the options interface, care must be
    taken to ensure that enough memory is available at that address.

    If a memory address is given, the RAM Storage Mechanism indicates that
    a new storage areas was not created.  If the memory is dynamically
    allocated, it indicates that a new storage area was created.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nssram.c $
    Revision 1.9  2009/02/06 08:36:12Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.8  2008/02/02 20:32:51  keithg
    comment updates to support autodoc
    Revision 1.7  2007/12/27 00:16:40Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator
    code is now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.6  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/08/30 18:09:50Z  thomd
    Fix to use 64bit Macros where necessary
    Revision 1.4  2007/08/22 19:35:46Z  pauli
    Added a name field to the storage layer interface.
    Revision 1.3  2007/06/26 19:10:48Z  timothyj
    Added Erase method, for more accurate simulation purposes and
    to enable storage mechanism optimizations.
    Revision 1.2  2006/08/26 00:30:03Z  Pauli
    Added compile time setting to allow enabling/disabling the NAND simulator.
    Revision 1.1  2006/07/26 17:38:42Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <nandsim.h>


#if FFXCONF_NANDSUPPORT

static NsStorageCreate     Create;
static NsStorageDestroy    Destroy;
static NsStorageRead       Read;
static NsStorageWrite      Write;
static NsStorageErase      Erase;


NsStorageInterfaces NANDSIM_RamStorage =
{
    Create,
    Destroy,
    Read,
    Write,
    Erase,
    "RAM"
};


struct tagNsStorageData
{
    D_BUFFER   *pMemory;
    D_BOOL      fAllocated;
};


/*-------------------------------------------------------------------
    Public: Create

    This function performs whatever initialization the storage
    mechanism needs.  The storage mechanism must determine if a
    storage area already exists.

    Parameters:
        hDev      - An opaque pointer to a device on which this
                    module is to begin operating.  This module uses
                    this handle to acquire per-device configuration.
        ullSize   - Total size of the storage area to create, in bytes.
                    This is used only if the storage area does not already
                    exist.
        pfCreate  - Return parameter indicating if the storage area was
                    created.  How this is determined is specific to each
                    storage mechanism implementation.

                    For the RAM Storage Mechanism, a new area is created
                    when storage memory is dynamically created.  If a
                    memory address is given via the options interface,
                    this is not considered creating a new storage area.

    Return:
        Returns a handle to new instance of the storage mechanism that
        is used for subsequent storage mechanism calls.
-------------------------------------------------------------------*/
static NsStorageHandle Create(
    FFXDEVHANDLE    hDev,
    D_UINT64        ullSize,
    D_BOOL         *pfCreated)
{
    NsStorageData  *pStorageData;
    D_UINT32 ulSize;

    DclAssert(pfCreated);
    DclAssert(!DclUint64GreaterUint32(&ullSize,D_UINT32_MAX));

    if(!pfCreated)
    {
        return NULL;
    }

    /*  Allocate our private data.
    */
    pStorageData = DclMemAllocZero(sizeof(*pStorageData));
    if(!pStorageData)
    {
        return NULL;
    }

    /*  Get memory for the main area.
    */
    if(!FfxHookOptionGet(FFXOPT_NANDSIM_RAM, hDev, &pStorageData->pMemory,
                         sizeof(pStorageData->pMemory)))
    {
        /*  If the RAM disk memory for the main array has not been
            supplied for us, allocate it now.
        */
        ulSize = DclUint32CastUint64(&ullSize);
        pStorageData->pMemory = DclMemAlloc(ulSize);
        if(!pStorageData->pMemory)
        {
            DclMemFree(pStorageData);
            return NULL;
        }
        pStorageData->fAllocated = TRUE;
    }

    /*  Indicate if we created a new storage area.
    */
    *pfCreated = pStorageData->fAllocated;

    return pStorageData;
}


/*-------------------------------------------------------------------
    Public: Destroy

    This function is called to destroy an instance of the storage
    mechanism.  It should perform whatever cleanup is required for
    this storage mechanism implementation.

    Parameters:
        hStorage  - a handle to the storage mechanism.

    Return:
        (none)
-------------------------------------------------------------------*/
static void Destroy(
    NsStorageHandle hStorage)
{
    DclAssert(hStorage);
    if(hStorage)
    {
        /*  Free the data storage memory if we allocated it.
        */
        if(hStorage->fAllocated)
        {
            DclMemFree(hStorage->pMemory);
        }

        DclMemFree(hStorage);
    }
}


/*-------------------------------------------------------------------
    Public: Read

    This function is called to read from the storage mechanism.

    Parameters:
        hStorage  - a handle to the storage mechanism.
        ullOffset - the byte offset to read from.
        pBuffer   - the buffer to populate with data from the storage mechanism
        ullLength - the number of bytes to read

    Return:
        None, failure is catastrophic.
-------------------------------------------------------------------*/
static void Read(
    NsStorageHandle hStorage,
    D_UINT64        ullOffset,
    D_BUFFER       *pBuffer,
    D_UINT32        ulLength )
{
    D_BUFFER       *pFrom;
    D_UINT32        ulTempOffset;

    DclProductionAssert(hStorage);
    DclProductionAssert(pBuffer);

    /*  Create limits ullOffset to D_UINT32_MAX, so we will cast it here
    */
    ulTempOffset = DclUint32CastUint64(&ullOffset);
    pFrom = hStorage->pMemory + ulTempOffset;
    DclMemCpy(pBuffer, pFrom, ulLength);
}


/*-------------------------------------------------------------------
    Public: Write

    This function is called to write data to the storage mechanism.

    Parameters:
        hStorage  - a handle to the storage mechanism.
        ullOffset - the byte offset to write to.
        pBuffer   - the buffer containing data to write to the storage mechanism
        ullLength - the number of bytes to write

    Return:
        None, failure is catastrophic.
-------------------------------------------------------------------*/
static void Write(
    NsStorageHandle hStorage,
    D_UINT64        ullOffset,
    D_BUFFER const *pBuffer,
    D_UINT32        ulLength )
{
    D_BUFFER       *pTo;
    D_UINT32        ulTempOffset;

    DclProductionAssert(hStorage);
    DclProductionAssert(pBuffer);

    /*  Create limits ullOffset to D_UINT32_MAX, so we will cast it here
    */
    ulTempOffset = DclUint32CastUint64(&ullOffset);
    pTo = hStorage->pMemory + ulTempOffset;
    DclMemCpy(pTo, pBuffer, ulLength);
}

/*-------------------------------------------------------------------
    Public: Erase

    This function is called to erase data in the storage mechanism.

    Parameters:
        hStorage  - a handle to the storage mechanism.
        ullOffset - the byte offset to erase.
        ullLength - the number of bytes to erase

    Return:
        None, failure is catastrophic.
-------------------------------------------------------------------*/
static void Erase(
    NsStorageHandle hStorage,
    D_UINT64        ullOffset,
    D_UINT32        ulLength )
{
    D_BUFFER       *pTo;
    D_UINT32        ulTempOffset;

    DclProductionAssert(hStorage);

    /*  Create limits ullOffset to D_UINT32_MAX, so we will cast it here
    */
    ulTempOffset = DclUint32CastUint64(&ullOffset);
    pTo = hStorage->pMemory + ulTempOffset;
    DclMemSet(pTo, -1, ulLength);
}

#endif  /* FFXCONF_NANDSUPPORT */

