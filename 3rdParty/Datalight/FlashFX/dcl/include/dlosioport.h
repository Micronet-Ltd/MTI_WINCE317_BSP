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

    Interface to osioport.c.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlosioport.h $
    Revision 1.2  2010/02/18 00:22:24Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.1  2010/02/11 19:46:30Z  billr
    Initial revision
    ---------------------
    Bill Roman 2010-02-04
---------------------------------------------------------------------------*/

/*  Any file that includes this header intends to use the I/O port
    access functions.  Make sure they are configured in.
*/
#if !(defined(DCLCONF_IOPORTS) && DCLCONF_IOPORTS)
#error Set DCLCONF_IOPORTS to TRUE in dclconf.h to enable support for I/O ports.
#endif

#ifndef DLOSIOPORT_H_INCLUDED
#define DLOSIOPORT_H_INCLUDED

typedef D_UINT32 D_IOPORT;

#ifndef DclOsIoIn8
#define DclOsIoIn8               DCLFUNC(DclOsIoIn8)
D_UINT8 DclOsIoIn8(D_IOPORT ioPort);
#endif

#ifndef DclOsIoIn16
#define DclOsIoIn16              DCLFUNC(DclOsIoIn16)
D_UINT16 DclOsIoIn16(D_IOPORT ioPort);
#endif

#ifndef DclOsIoIn32
#define DclOsIoIn32              DCLFUNC(DclOsIoIn32)
D_UINT32 DclOsIoIn32(D_IOPORT ioPort);
#endif

#ifndef DclOsIoInStr8
#define DclOsIoInStr8            DCLFUNC(DclOsIoInStr8)
void DclOsIoInStr8(D_IOPORT ioPort, D_UINT8 *pBuffer, size_t count);
#endif

#ifndef DclOsIoInStr16
#define DclOsIoInStr16           DCLFUNC(DclOsIoInStr16)
void DclOsIoInStr16(D_IOPORT ioPort, D_UINT16 *pBuffer, size_t count);
#endif

#ifndef DclOsIoOut8
#define DclOsIoOut8              DCLFUNC(DclOsIoOut8)
void DclOsIoOut8(D_IOPORT ioPort, D_UINT8 val);
#endif

#ifndef DclOsIoOut16
#define DclOsIoOut16             DCLFUNC(DclOsIoOut16)
void DclOsIoOut16(D_IOPORT ioPort, D_UINT16 val);
#endif

#ifndef DclOsIoOut32
#define DclOsIoOut32             DCLFUNC(DclOsIoOut32)
void DclOsIoOut32(D_IOPORT ioPort, D_UINT32 val);
#endif

#ifndef DclOsIoOutStr8
#define DclOsIoOutStr8           DCLFUNC(DclOsIoOutStr8)
void DclOsIoOutStr8(D_IOPORT ioPort, const D_UINT8 *pBuffer, size_t count);
#endif

#ifndef DclOsIoOutStr16
#define DclOsIoOutStr16          DCLFUNC(DclOsIoOutStr16)
void DclOsIoOutStr16(D_IOPORT ioPort, const D_UINT16 *pBuffer, size_t count);
#endif

#ifndef DclOsIoOutStr32
#define DclOsIoOutStr32          DCLFUNC(DclOsIoOutStr32)
void DclOsIoOutStr32(D_IOPORT ioPort, const D_UINT32 *pBuffer, size_t count);
#endif


#endif /* DLOSIOPORT_H_INCLUDED */
