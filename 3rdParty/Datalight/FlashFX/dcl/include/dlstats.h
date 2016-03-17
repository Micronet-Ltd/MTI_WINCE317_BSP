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

    This header defines the interface for querying and displaying statistical
    information.  It is typically only used in debug builds.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlstats.h $
    Revision 1.16  2010/02/28 17:33:51Z  garyp
    Did some refactoring of the DCL system info functions to make them
    more useful.
    Revision 1.15  2010/02/13 20:37:41Z  garyp
    Updated to use the correct license wording.
    Revision 1.14  2010/02/10 23:38:54Z  billr
    Changed the headers to reflect the shared or public license. [jimmb]
    Revision 1.13  2009/11/05 03:22:46Z  garyp
    Updated to track statistics in microseonds rather than millisedonds.
    Revision 1.12  2009/06/28 01:20:05Z  garyp
    Minor tweaks to the mutex and semaphore stats.
    Revision 1.11  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.10  2007/08/03 01:00:48Z  garyp
    Modified version numbers to be 32-bit values
    Revision 1.9  2007/03/31 19:08:49Z  Garyp
    Added support for mutex tracking.
    Revision 1.8  2007/03/29 23:44:04Z  Garyp
    Added structures and prototypes to support mutex and read/write semaphore
    statistics.
    Revision 1.7  2006/12/16 21:34:36Z  Garyp
    Added more compile time project information.
    Revision 1.6  2006/12/11 22:04:16Z  Garyp
    Updated to support DCL_OSFEATURE_CONSOLEINPUT.
    Revision 1.5  2006/12/08 00:05:18Z  Garyp
    Updated to work with the D_CPUTYPE value which is a string rather than
    a number.
    Revision 1.4  2006/10/01 18:13:28Z  Garyp
    Updated to track the maximum number of concurrent blocks allocated.
    Revision 1.3  2006/09/20 00:08:49Z  Garyp
    Modified to accommodate build numbers which are now strings rather than
    numbers.
    Revision 1.2  2006/08/18 20:19:23Z  Garyp
    Added support for the processor alignment boundary.
    Revision 1.1  2006/01/02 12:25:22Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLSTATS_H_INCLUDED
#define DLSTATS_H_INCLUDED


typedef struct DCLMEMSTATS
{
    D_UINT16    uStrucLen;
    D_UINT16    uOverhead;
    D_UINT32    ulBytesCurrentlyAllocated;
    D_UINT32    ulBlocksCurrentlyAllocated;
    D_UINT32    ulMaxConcurrentBytes;
    D_UINT32    ulMaxConcurrentBlocks;
    D_UINT32    ulTotalAllocCalls;
    D_UINT32    ulLargestAllocation;
} DCLMEMSTATS;

typedef struct
{
    char        szName[16];
    D_UINT64    ullTotalWaitUS;
    D_UINT32    ulMaxWaitUS;
    D_UINT32    ulAcquireCount;
    unsigned    nCurrentCount;
} DCLMUTEXINFO;

typedef struct DCLMUTEXSTATS
{
    unsigned        nStrucLen;
    unsigned        nCount;     /* Total mutex count    */
    unsigned        nSupplied;  /* number of DCLMUTEXINFO structures supplied */
    unsigned        nReturned;  /* number of DCLMUTEXINFO structures returned */
    DCLMUTEXINFO   *pDMI;
} DCLMUTEXSTATS;

typedef struct DCLSEMAPHOREINFO
{
    char        szName[16];
    D_UINT64    ullTotalWaitUS;
    D_UINT32    ulMaxWaitUS;
    D_UINT32    ulMaxCount;
    D_UINT32    ulAcquireCount;
    D_UINT32    ulCurrentCount;
    D_UINT32    ulHighWater;
} DCLSEMAPHOREINFO;

typedef struct DCLSEMAPHORESTATS
{
    unsigned            nStrucLen;
    unsigned            nCount;     /* Total semaphore count    */
    unsigned            nSupplied;  /* number of DCLSEMAPHOREINFO structures supplied */
    unsigned            nReturned;  /* number of DCLSEMAPHOREINFO structures returned */
    DCLSEMAPHOREINFO   *pDSI;
} DCLSEMAPHORESTATS;

typedef struct DCLRDWRSEMAPHOREINFO
{
    char        szName[16];
    D_UINT32    ulMaxCount;
    D_UINT32    ulCurrentCount;
    unsigned    fWriting : 1;
    D_UINT32    ulReadAcquireCount;
    D_UINT32    ulReadHighWater;
    D_UINT64    ullReadTotalWaitUS;
    D_UINT32    ulReadMaxWaitUS;
    D_UINT32    ulWriteAcquireCount;
    D_UINT64    ullWriteTotalWaitUS;
    D_UINT32    ulWriteMaxWaitUS;
} DCLRDWRSEMAPHOREINFO;

typedef struct DCLRDWRSEMAPHORESTATS
{
    unsigned            nStrucLen;
    unsigned            nCount;     /* Total semaphore count    */
    unsigned            nSupplied;  /* number of DCLRDWRSEMAPHOREINFO structures supplied */
    unsigned            nReturned;  /* number of DCLRDWRSEMAPHOREINFO structures returned */
    DCLRDWRSEMAPHOREINFO   *pRWI;
} DCLRDWRSEMAPHORESTATS;

typedef struct
{
    DCLIOREQUEST    ior;
    D_BOOL          fVerbose;
    D_BOOL          fReset;
    DCLMEMSTATS     dms;
} DCLREQ_MEMTRACK_STATS;


#define     DclMemStatsDisplay              DCLFUNC(DclMemStatsDisplay)
#define     DclMutexStatsQuery              DCLFUNC(DclMutexStatsQuery)
#define     DclMutexStatsDisplay            DCLFUNC(DclMutexStatsDisplay)
#define     DclSemaphoreStatsQuery          DCLFUNC(DclSemaphoreStatsQuery)
#define     DclSemaphoreStatsDisplay        DCLFUNC(DclSemaphoreStatsDisplay)
#define     DclSemaphoreRdWrStatsQuery      DCLFUNC(DclSemaphoreRdWrStatsQuery)
#define     DclSemaphoreRdWrStatsDisplay    DCLFUNC(DclSemaphoreRdWrStatsDisplay)

DCLSTATUS   DclMemStatsDisplay(             DCLMEMSTATS *pDMS);
DCLSTATUS   DclMutexStatsQuery(             DCLMUTEXSTATS *pDMS, D_BOOL fVerbose, D_BOOL fReset);
DCLSTATUS   DclMutexStatsDisplay(           DCLMUTEXSTATS *pDMS);
DCLSTATUS   DclSemaphoreStatsQuery(         DCLSEMAPHORESTATS *pDSS, D_BOOL fVerbose, D_BOOL fReset);
DCLSTATUS   DclSemaphoreStatsDisplay(       DCLSEMAPHORESTATS *pDSS);
DCLSTATUS   DclSemaphoreRdWrStatsQuery(     DCLRDWRSEMAPHORESTATS *pRWS, D_BOOL fVerbose, D_BOOL fReset);
DCLSTATUS   DclSemaphoreRdWrStatsDisplay(   DCLRDWRSEMAPHORESTATS *pRWS);


#endif  /* DLSTATS_H_INCLUDED */

