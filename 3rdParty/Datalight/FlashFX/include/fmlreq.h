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

    This header defines the interface used to build FML requests to be
    processed through the external API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlreq.h $
    Revision 1.9  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.8  2009/07/22 00:21:17Z  garyp
    Merged from the v4.0 branch.  Updated the FfxFmlCreate() function to take
    a flags parameter.  Adjusted for a number of fields which are now unsigned
    rather than D_UINT16.  Added REQ_FMLDEVICERANGE.
    Revision 1.7  2009/04/09 03:52:32Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.6  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2006/11/10 20:46:01Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.4  2006/03/05 03:16:55Z  Garyp
    Added FfxFmlDeviceInfo() support.
    Revision 1.3  2006/02/13 02:58:07Z  Garyp
    Updated to new external API interface.
    Revision 1.2  2006/02/07 07:32:16Z  Garyp
    Temporarily disable the FML external interface while it is being revamped.
    Revision 1.1  2005/05/02 17:38:08Z  Pauli
    Initial revision
    Revision 1.2  2005/05/02 18:38:07Z  Garyp
    Added external API support for FfxFmlWritePages.
    Revision 1.1  2005/10/02 03:03:34Z  Garyp
    Initial revision
    Revision 1.3  2004/12/30 23:08:28Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/02/22 19:15:14Z  garys
    Revision 1.1.1.3  2004/02/22 19:15:14  garyp
    Eliminated support for oemchanged().
    Revision 1.1.1.2  2003/12/04 23:20:58Z  garyp
    Added pDevExtra to oemmount().
    Revision 1.1  2003/11/22 03:30:52Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FMLREQ_H_INCLUDED
#define FMLREQ_H_INCLUDED

/*-------------------------------------------------------------------
    NOTE! All the REQ_... structures <MUST> have a FFXIOREQUEST
          structure as the first field!.
-------------------------------------------------------------------*/

typedef struct
{
    FFXIOREQUEST    req;
    FFXDISKHANDLE   hDisk;
    FFXFIMDEVHANDLE hFimDev;
    unsigned        nDiskNum;
    D_UINT32        ulStartBlock;
    D_UINT32        ulBlockCount;
    FFXFMLHANDLE    hFML;
    unsigned        nFlags;
} REQ_FMLCREATE;

typedef struct
{
    FFXIOREQUEST    req;
    FFXFMLHANDLE    hFML;
    FFXSTATUS       ffxStat;
} REQ_FMLDESTROY;

typedef struct
{
    FFXIOREQUEST    req;
    unsigned        nDiskNum;
    FFXFMLHANDLE    hFML;
} REQ_FMLHANDLE;

typedef struct
{
    FFXIOREQUEST        req;
    unsigned            nDeviceNum;
    D_UINT32            ulBlockNum;
    FFXFMLHANDLE       *phFML;
    D_UINT32           *pulBlockCount;
    FFXSTATUS           ffxStat;
} REQ_FMLDEVICERANGE;

typedef struct
{
    FFXIOREQUEST        req;
    unsigned            nDeviceNum;
    FFXFMLDEVINFO      *pFmlDeviceInfo;
    FFXSTATUS           ffxStat;
} REQ_FMLDEVICEINFO;

typedef struct
{
    FFXIOREQUEST    req;
    FFXFMLHANDLE    hFML;
    FFXFMLINFO     *pFmlInfo;
    FFXSTATUS       ffxStat;
} REQ_FMLINFO;

typedef struct
{
    FFXIOREQUEST    req;
    FFXFMLHANDLE    hFML;
    FFXPARAM        id;
    void           *pBuffer;
    D_UINT32        ulBuffLen;
    FFXSTATUS       ffxStat;
} REQ_FMLPARAMETERGET;

typedef struct
{
    FFXIOREQUEST    req;
    FFXFMLHANDLE    hFML;
    FFXPARAM        id;
    const void     *pBuffer;
    D_UINT32        ulBuffLen;
    FFXSTATUS       ffxStat;
} REQ_FMLPARAMETERSET;

typedef struct
{
    FFXIOREQUEST    req;
    FFXFMLHANDLE    hFML;
    FFXIOREQUEST   *pIOReq;
    FFXIOSTATUS     ioStat;
} REQ_FMLIOREQUEST;



#endif /* FMLREQ_H_INCLUDED */
