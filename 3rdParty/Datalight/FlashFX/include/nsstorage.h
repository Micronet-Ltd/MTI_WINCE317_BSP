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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header defines the interface to the NAND Simulator Storage
    subsystem.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nsstorage.h $
    Revision 1.1  2010/02/25 02:23:16Z  billr
    Initial revision
    Based on include/nandsim.h revision 1.4
---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Storage layer.  All errors are catastrophic (ASSERTs).

    NsStorageHandle is an internal reference to private data
    associated with an instance of an storage mechanism.  It
    is returned by the storage mechanism Create call.  It
    contains any internal variables that layer desires.
-------------------------------------------------------------------*/

#ifndef NSSTORAGE_H_INCLUDED
#define NSSTORAGE_H_INCLUDED

typedef struct tagNsStorageData *NsStorageHandle;
typedef struct tagNsStorageData  NsStorageData;

typedef NsStorageHandle NsStorageCreate(FFXDEVHANDLE hDev, D_UINT64 ullSize, D_BOOL *pfCreated);
typedef void            NsStorageDestroy(NsStorageHandle hStorage);
typedef void            NsStorageRead(   NsStorageHandle hStorage, D_UINT64 ullOffset, D_BUFFER       *pBuffer, D_UINT32 ulLength);
typedef void            NsStorageWrite(  NsStorageHandle hStorage, D_UINT64 ullOffset, D_BUFFER const *pBuffer, D_UINT32 ulLength);
typedef void            NsStorageErase(  NsStorageHandle hStorage, D_UINT64 ullOffset, D_UINT32 ulLength);

/*  This structure contains the "V-table" of all the storage mechanism
    functions.
*/
typedef struct
{
    NsStorageCreate     * const Create;
    NsStorageDestroy    * const Destroy;
    NsStorageRead       * const Read;
    NsStorageWrite      * const Write;
    NsStorageErase      * const Erase;
    char                * const szName;
} NsStorageInterfaces;

/*  Storage Mechanism Interfaces
*/
extern NsStorageInterfaces NANDSIM_FileStorage;
extern NsStorageInterfaces NANDSIM_RamStorage;


#endif /* NSSTORAGE_H_INCLUDED */
