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

    Interface to osthread.c.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlosthread.h $
    Revision 1.3  2010/02/13 20:37:41Z  garyp
    Updated to use the correct license wording.
    Revision 1.2  2010/02/10 23:38:54Z  billr
    Changed the headers to reflect the shared or public license. [jimmb]
    Revision 1.1  2009/12/19 01:57:30Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLOSTHREAD_H_INCLUDED
#define DLOSTHREAD_H_INCLUDED

/*-------------------------------------------------------------------
    Threading API

    Type: DCLTHREADID

    The DCLTHREADID type is used to store a thread ID, as returned
    by DclOsThreadID().

    Type: DCLTHREADATTR

    The DCLTHREADATTR type is an opaque structure used to store
    thread attributes.

    Type: DCLTHREADHANDLE

    The DCLTHREADHANDLE type is an opaque pointer to a thread object
    which was craeted by DclOsThreadCreate().
-------------------------------------------------------------------*/
/*  DclOsThreadID() must exist even if threading features are
    disabled or nonexistant.
*/
#define     DclOsThreadID               DCLFUNC(DclOsThreadID)
DCLTHREADID DclOsThreadID(void);

#if DCL_OSFEATURE_THREADS
  enum
  {
      DCL_THREADPRIORITY_ENUMLOWLIMIT = -1,
      DCL_THREADPRIORITY_TIMECRITICAL,
      DCL_THREADPRIORITY_HIGH,
      DCL_THREADPRIORITY_ABOVENORMAL,
      DCL_THREADPRIORITY_NORMAL,
      DCL_THREADPRIORITY_BELOWNORMAL,
      DCL_THREADPRIORITY_LOW,
      DCL_THREADPRIORITY_ABOVEIDLE,
      DCL_THREADPRIORITY_IDLE,
      DCL_THREADPRIORITY_ENUMHIGHLIMIT
  };

  typedef struct DCLOSTHREADATTR    DCLTHREADATTR;
  typedef struct DCLOSTHREAD       *DCLTHREADHANDLE;
  typedef                   void *(*DCLTHREADFUNC) (void *);

  #define DclOsThreadAttrCreate         DCLFUNC(DclOsThreadAttrCreate)
  #define DclOsThreadAttrDestroy        DCLFUNC(DclOsThreadAttrDestroy)
  #define DclOsThreadAttrSetStackSize   DCLFUNC(DclOsThreadAttrSetStackSize)
  #define DclOsThreadAttrSetPriority    DCLFUNC(DclOsThreadAttrSetPriority)
  #define DclOsThreadCreate             DCLFUNC(DclOsThreadCreate)
  #define DclOsThreadDestroy            DCLFUNC(DclOsThreadDestroy)
  #define DclOsThreadPrioritySet        DCLFUNC(DclOsThreadPrioritySet)
  #define DclOsThreadTerminate          DCLFUNC(DclOsThreadTerminate)
  #define DclOsThreadWait               DCLFUNC(DclOsThreadWait)
  #define DclOsThreadSuspend            DCLFUNC(DclOsThreadSuspend)
  #define DclOsThreadResume             DCLFUNC(DclOsThreadResume)

  DCLTHREADATTR  *DclOsThreadAttrCreate(void);
  void            DclOsThreadAttrDestroy(       DCLTHREADATTR *pAttr);
  DCLSTATUS       DclOsThreadAttrSetStackSize(  DCLTHREADATTR *pAttr, D_UINT32 ulStackSize);
  DCLSTATUS       DclOsThreadAttrSetPriority(   DCLTHREADATTR *pAttr, D_UINT16 uPriority);
  DCLSTATUS       DclOsThreadCreate(            DCLTHREADHANDLE *phThread, char *pszName, DCLTHREADATTR *pAttr, DCLTHREADFUNC pFunc, void *pArg);
  DCLSTATUS       DclOsThreadDestroy(           DCLTHREADHANDLE hThread);
  DCLSTATUS       DclOsThreadPrioritySet(       DCLTHREADHANDLE hThread, unsigned nPriority);
  DCLSTATUS       DclOsThreadTerminate(         DCLTHREADHANDLE hThread);
  DCLSTATUS       DclOsThreadWait(              DCLTHREADHANDLE hThread, D_UINT32 ulTimeout);
  DCLSTATUS       DclOsThreadSuspend(DCLTHREADHANDLE hThread);
  DCLSTATUS       DclOsThreadResume(DCLTHREADHANDLE hThread);
#endif

#endif /* DLOSTHREAD_H_INCLUDED */
