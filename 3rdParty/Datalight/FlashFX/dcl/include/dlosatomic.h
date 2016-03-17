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

    Interface to osatomic.c.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlosatomic.h $
    Revision 1.3  2010/12/09 21:30:04Z  billr
    Correct license agreement in dual-licensed headers.
    Revision 1.2  2010/01/23 21:28:51Z  garyp
    Added prototypes.
    Revision 1.1  2009/12/19 01:57:30Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLOSATOMIC_H_INCLUDED
#define DLOSATOMIC_H_INCLUDED

#define         DclOsAtomic32CompareExchange    DCLFUNC(DclOsAtomic32CompareExchange)
#define         DclOsAtomic32Exchange           DCLFUNC(DclOsAtomic32Exchange)
#define         DclOsAtomic32ExchangeAdd        DCLFUNC(DclOsAtomic32ExchangeAdd)
#define         DclOsAtomic32Decrement          DCLFUNC(DclOsAtomic32Decrement)
#define         DclOsAtomic32Increment          DCLFUNC(DclOsAtomic32Increment)
#define         DclOsAtomicPtrCompareExchange   DCLFUNC(DclOsAtomicPtrCompareExchange)
#define         DclOsAtomicPtrExchange          DCLFUNC(DclOsAtomicPtrExchange)

typedef D_UINT32 volatile   D_ATOMIC32;
typedef void * volatile     D_ATOMICPTR;

D_UINT32        DclOsAtomic32CompareExchange(   D_ATOMIC32 *pulAtomic, D_UINT32 ulCompare, D_UINT32 ulExchange);
D_UINT32        DclOsAtomic32Exchange(          D_ATOMIC32 *pulAtomic, D_UINT32 ulExchange);
D_UINT32        DclOsAtomic32ExchangeAdd(       D_ATOMIC32 *pulAtomic, D_UINT32 ulAdd);
D_UINT32        DclOsAtomic32Decrement(         D_ATOMIC32 *pulAtomic);
D_UINT32        DclOsAtomic32Increment(         D_ATOMIC32 *pulAtomic);
void *          DclOsAtomicPtrCompareExchange(  D_ATOMICPTR *pDestination, void *pCompare, void *pExchange);
void *          DclOsAtomicPtrExchange(         D_ATOMICPTR *pDestination, void *pExchange);

/*  The following functions are implemented as macros since they are
    basically just a shorthand way of using the standard APIs.
*/
#define         DclOsAtomic32SectionEnter(pVal)(DclOsAtomic32CompareExchange(pVal, 0, 1) == 0)
#define         DclOsAtomic32SectionLeave(pVal)(DclOsAtomic32Decrement(pVal))

/*  The two "retrieve" macros are conditionally defined if they are not
    already defined elsewhere, such as dlosconf.h or dclconf.h.  Those
    two macros default to using an implementation which assumes that
    aligned pointers and 32-bit values will be atomically read when
    accessed.  If this is not the case, alternate macros are provided
    which will read the values in safe fashion.  Datalight is unaware
    of any current target environments where this is an issue.
*/
#ifndef DclOsAtomic32Retrieve
  #define       DclOsAtomic32Retrieve(pVal)     (*(pVal))
  /*  Alternate definition for systems which won't atomically read
      an aligned 32-bit value.
      #define   DclOsAtomic32Retrieve(pVal)     DclOsAtomic32CompareExchange(pVal, 0, 0)
  */    
#endif
#ifndef DclOsAtomicPtrRetrieve
  #define       DclOsAtomicPtrRetrieve(pPtr)    (*(pPtr))
  /*  Alternate definition for systems which won't atomically read
      an aligned pointer value.
      #define   DclOsAtomicPtrRetrieve(pPtr)    DclOsAtomicPtrCompareExchange(pPtr, NULL, NULL)
  */
#endif


#endif /* DLOSATOMIC_H_INCLUDED */
