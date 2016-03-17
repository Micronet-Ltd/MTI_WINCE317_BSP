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

    This header defines the interfact to the logging API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dllog.h $
    Revision 1.4  2010/11/01 02:56:53Z  garyp
    Modified DclLogOpen() to take a flags parameter rather than a whole
    bunch of bools.  Updated to support binary logs.  Renamed DclLogWrite()
    to DclLogWriteString() to avoid confusion with the new logging capability
    of handling binary logs.
    Revision 1.3  2010/02/13 20:37:40Z  garyp
    Updated to use the correct license wording.
    Revision 1.2  2010/02/10 23:38:54Z  billr
    Changed the headers to reflect the shared or public license. [jimmb]
    Revision 1.1  2009/06/22 17:05:06Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLLOG_H_INCLUDED
#define DLLOG_H_INCLUDED


/*-------------------------------------------------------------------
    Logging Functions

    Type: DCLLOGHANDLE

    A DCLLOGHANDLE is used to reference a logging object which is
    created with DclLogOpen().
-------------------------------------------------------------------*/
typedef struct sLOGINFO *DCLLOGHANDLE;


typedef enum
{
    DCLLOGMODE_LOWLIMIT     = 0x00,
    DCLLOGMODE_QUERY        = 0x01,
    DCLLOGMODE_FAILED       = 0x02,
    DCLLOGMODE_SHADOW       = 0x04,
    DCLLOGMODE_NOSHADOW     = 0x08,
    DCLLOGMODE_HIGHLIMIT
} DCLLOGMODE;

/*  Log creation flags
*/
#define DCLLOGFLAGS_APPEND      0x0001
#define DCLLOGFLAGS_SHADOW      0x0002
#define DCLLOGFLAGS_PRIVATE     0x0004
#define DCLLOGFLAGS_BINARY      0x0008


typedef struct
{
    DCLIOREQUEST    ior;
    const char     *pszFile;
    unsigned        nBufferKB;
    D_UINT32        ulFlags;
    DCLLOGHANDLE    hLog;
} DCLREQ_LOG_OPEN;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLLOGHANDLE    hLog;
    const D_BUFFER *pData;
    size_t          nDataLen;
    DCLSTATUS       dclStat;
} DCLREQ_LOG_WRITE;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLLOGHANDLE    hLog;
    const char     *pszString;
    DCLSTATUS       dclStat;
} DCLREQ_LOG_WRITESTRING;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLLOGHANDLE    hLog;
    DCLSTATUS       dclStat;
} DCLREQ_LOG_FLUSH;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLLOGHANDLE    hLog;
    DCLSTATUS       dclStat;
} DCLREQ_LOG_CLOSE;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLLOGHANDLE    hLog;
    DCLLOGMODE      nMode;
    DCLLOGMODE      nOldMode;
} DCLREQ_LOG_MODE;


#define         DclLogOpen              DCLFUNC(DclLogOpen)
#define         DclLogFlush             DCLFUNC(DclLogFlush)
#define         DclLogClose             DCLFUNC(DclLogClose)
#define         DclLogMode              DCLFUNC(DclLogMode)
#define         DclLogPrintf            DCLFUNC(DclLogPrintf)
#define         DclLogWrite             DCLFUNC(DclLogWrite)
#define         DclLogWriteString       DCLFUNC(DclLogWriteString)

DCLLOGHANDLE    DclLogOpen(       DCLINSTANCEHANDLE hDclInst, const char *pszFile, unsigned nBufferKB, D_UINT32 ulFlags);
DCLSTATUS       DclLogFlush(      DCLINSTANCEHANDLE hDclInst, DCLLOGHANDLE hLog);
DCLSTATUS       DclLogClose(      DCLINSTANCEHANDLE hDclInst, DCLLOGHANDLE hLog);
DCLLOGMODE      DclLogMode(       DCLINSTANCEHANDLE hDclInst, DCLLOGHANDLE hLog, DCLLOGMODE nMode);
int             DclLogPrintf(     DCLINSTANCEHANDLE hDclInst, DCLLOGHANDLE hLog, const char *pszFmt, ...);
DCLSTATUS       DclLogWrite(      DCLINSTANCEHANDLE hDclInst, DCLLOGHANDLE hLog, const D_BUFFER *pData, size_t nDataLen);
DCLSTATUS       DclLogWriteString(DCLINSTANCEHANDLE hDclInst, DCLLOGHANDLE hLog, const char *pszData);


#endif  /* DLLOG_H_INCLUDED */

