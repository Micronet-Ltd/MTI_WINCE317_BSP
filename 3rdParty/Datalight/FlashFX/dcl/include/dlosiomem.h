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

    Interface to osiomem.c.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlosiomem.h $
    Revision 1.2  2010/02/18 00:22:24Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.1  2010/02/11 19:46:30Z  billr
    Initial revision
    ---------------------
    Bill Roman 2010-02-04
---------------------------------------------------------------------------*/

#ifndef DLOSIOMEM_H_INCLUDED
#define DLOSIOMEM_H_INCLUDED

/*  Any file that includes this header intends to use the I/O memory
    access functions.  Make sure they are configured in.
*/
#if !(defined(DCLCONF_IOMEM) && DCLCONF_IOMEM)
#error Set DCLCONF_IOMEM to TRUE in dclconf.h to enable support for I/O memory access.
#endif

#ifndef DclOsMemIn8
#define DclOsMemIn8              DCLFUNC(DclOsMemIn8)
D_UINT8 DclOsMemIn8(void *ioAddr);
#endif

#ifndef DclOsMemIn16
#define DclOsMemIn16             DCLFUNC(DclOsMemIn16)
D_UINT16 DclOsMemIn16(void *ioAddr);
#endif

#ifndef DclOsMemIn32
#define DclOsMemIn32             DCLFUNC(DclOsMemIn32)
D_UINT32 DclOsMemIn32(void *ioAddr);
#endif

#ifndef DclOsMemInStr8
#define DclOsMemInStr8           DCLFUNC(DclOsMemInStr8)
void DclOsMemInStr8(void *ioAddr, D_UINT8 *pBuffer, size_t count);
#endif

#ifndef DclOsMemInStr16
#define DclOsMemInStr16          DCLFUNC(DclOsMemInStr16)
void DclOsMemInStr16(void *ioAddr, D_UINT16 *pBuffer, size_t count);
#endif

#ifndef DclOsMemInStr32
#define DclOsMemInStr32          DCLFUNC(DclOsMemInStr32)
void DclOsMemInStr32(void *ioAddr, D_UINT32 *pBuffer, size_t count);
#endif

#ifndef DclOsMemInCopy8
#define DclOsMemInCopy8          DCLFUNC(DclOsMemInCopy8)
void DclOsMemInCopy8(void *ioAddr, D_UINT8 *pBuffer, size_t count);
#endif

#ifndef DclOsMemInCopy16
#define DclOsMemInCopy16         DCLFUNC(DclOsMemInCopy16)
void DclOsMemInCopy16(void *ioAddr, D_UINT16 *pBuffer, size_t count);
#endif

#ifndef DclOsMemInCopy32
#define DclOsMemInCopy32         DCLFUNC(DclOsMemInCopy32)
void DclOsMemInCopy32(void *ioAddr, D_UINT32 *pBuffer, size_t count);
#endif

#ifndef DclOsMemOut8
#define DclOsMemOut8             DCLFUNC(DclOsMemOut8)
void DclOsMemOut8(void *ioAddr, D_UINT8 val);
#endif

#ifndef DclOsMemOut16
#define DclOsMemOut16            DCLFUNC(DclOsMemOut16)
void DclOsMemOut16(void *ioAddr, D_UINT16 val);
#endif

#ifndef DclOsMemOut32
#define DclOsMemOut32            DCLFUNC(DclOsMemOut32)
void DclOsMemOut32(void *ioAddr, D_UINT32 val);
#endif

#ifndef DclOsMemOutStr8
#define DclOsMemOutStr8          DCLFUNC(DclOsMemOutStr8)
void DclOsMemOutStr8(void *ioAddr, const D_UINT8 *pBuffer, size_t count);
#endif

#ifndef DclOsMemOutStr16
#define DclOsMemOutStr16         DCLFUNC(DclOsMemOutStr16)
void DclOsMemOutStr16(void *ioAddr, const D_UINT16 *pBuffer, size_t count);
#endif

#ifndef DclOsMemOutStr32
#define DclOsMemOutStr32         DCLFUNC(DclOsMemOutStr32)
void DclOsMemOutStr32(void *ioAddr, const D_UINT32 *pBuffer, size_t count);
#endif

#ifndef DclOsMemOutCopy8
#define DclOsMemOutCopy8         DCLFUNC(DclOsMemOutCopy8)
void DclOsMemOutCopy8(void *ioAddr, const D_UINT8 *pBuffer, size_t count);
#endif

#ifndef DclOsMemOutCopy16
#define DclOsMemOutCopy16        DCLFUNC(DclOsMemOutCopy16)
void DclOsMemOutCopy16(void *ioAddr, const D_UINT16 *pBuffer, size_t count);
#endif

#ifndef DclOsMemOutCopy32
#define DclOsMemOutCopy32        DCLFUNC(DclOsMemOutCopy32)
void DclOsMemOutCopy32(void *ioAddr, const D_UINT32 *pBuffer, size_t count);
#endif

#endif /* DLOSIOMEM_H_INCLUDED */
