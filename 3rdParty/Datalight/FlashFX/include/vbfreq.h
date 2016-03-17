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

    This header defines the interface used to build VBF requests to be
    processed through the external API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbfreq.h $
    Revision 1.14  2010/11/23 19:32:42Z  glenns
    Add new request for disabling QuickMount state save upon the
    next disk dismount.
    Revision 1.13  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.12  2009/12/18 17:19:26Z  garyp
    Resolved Bug 2955: Write interruption test failure.
    Revision 1.11  2009/08/04 03:26:39Z  garyp
    Merged from the v4.0 branch.  Added support for FfxVbfRegionMetrics() to
    the external API.  Updated the compaction functions to take a compaction
    level.  Modified the shutdown processes to take a mode parameter.  Added
    support for compaction suspend/resume.  Minor datatype changes from 
    D_UINT16 to unsigned.  Updated for new compaction functions which now
    return an FFXIOSTATUS value rather than a D_BOOL.
    Revision 1.10  2008/12/09 21:32:10Z  keithg
    No longer pass a serial number to VbfFormat() - it is handled internally.
    Revision 1.9  2008/06/03 20:57:59Z  thomd
    Added fFormatNoErase
    Revision 1.8  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/06/07 22:43:36Z  rickc
    Removed structs for deprecated vbfread(), vbfwrite(), vbfdiscard(),
    and vbfclientsize()
    Revision 1.6  2006/12/16 20:36:27Z  Garyp
    Modified FfxVbfTestRegionMountPerf() to return the performance result
    rather than displaying it, which is not very useful in non-monolithic
    environments.
    Revision 1.5  2006/05/08 17:21:34Z  Garyp
    Updated to use the new write interruption test interfaces.
    Revision 1.4  2006/02/20 08:05:21Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.3  2006/02/13 03:08:16Z  Garyp
    Temporarily disabled -- work-in-progress.
    Revision 1.2  2006/02/08 00:26:20Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.1  2005/11/06 09:02:08Z  Pauli
    Initial revision
    Revision 1.2  2005/11/06 09:02:07Z  Garyp
    Updated to support the new statistics interface.
    Revision 1.1  2005/10/02 03:04:22Z  Garyp
    Initial revision
    Revision 1.5  2004/12/30 23:48:10Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2004/11/20 00:29:42Z  GaryP
    Added external API support for compaction suspend/restore.
    Revision 1.3  2004/09/23 08:37:33Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.2  2003/12/29 22:48:35Z  garys
    Revision 1.1.1.2  2003/12/29 22:48:35  billr
    Fix interface to vbfgetpartitioninfo(), it now returns a status.
    Revision 1.1  2003/11/22 03:30:54Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef VBFREQ_H_INCLUDED
#define VBFREQ_H_INCLUDED


/*---------------------------------------------------------
    NOTE! All the REQ_... structures <MUST> have a REQHDR
          structure as the first field!.
---------------------------------------------------------*/
typedef struct
{
    FFXIOREQUEST    req;
    FFXFMLHANDLE    hFML;
    VBFHANDLE       hVBF;
} REQ_VBFCREATE;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulFlags;
    FFXSTATUS       ffxStat;
} REQ_VBFDESTROY;

typedef struct
{
    FFXIOREQUEST    req;
    unsigned        nDiskNum;
    VBFHANDLE       hVBF;
} REQ_VBFHANDLE;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    VBFDISKINFO    *pDiskInfo;
    FFXSTATUS       ffxStat;
} REQ_VBFDISKINFO;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    VBFDISKMETRICS *pDiskMets;
    FFXSTATUS       ffxStat;
} REQ_VBFDISKMETRICS;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32            ulRegionNum;
    VBFREGIONMETRICS   *pRegionMets;
    FFXSTATUS           ffxStat;
} REQ_VBFREGIONMETRICS;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    VBFUNITMETRICS *pUnitMets;
    FFXSTATUS       ffxStat;
} REQ_VBFUNITMETRICS;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    D_BUFFER       *pBuffer;
    FFXIOSTATUS     ioStat;
} REQ_VBFREAD;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    const D_BUFFER *pBuffer;
    FFXIOSTATUS     ioStat;
} REQ_VBFWRITE;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    FFXIOSTATUS     ioStat;
} REQ_VBFDISCARD;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    FFXIOSTATUS     ioStat;
    D_UINT32        ulFlags;
} REQ_VBFCOMPACT;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulCount;
 } REQ_VBFCOMPSUSPEND;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulCount;
 } REQ_VBFCOMPRESUME;

typedef struct
{
    FFXIOREQUEST    req;
    FFXFMLHANDLE    hFML;
    D_UINT32        ulCushion;
    D_UINT32        ulSpareUnits;
    FFXSTATUS       ffxStat;
    D_BOOL          fFormatNoErase;
} REQ_VBFFORMAT;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulResult;
} REQ_VBFTESTRGNMOUNTPERF;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    D_UINT32        ulSeed;
    unsigned        nVerbosity;
    D_INT16         nReturn;
} REQ_VBFTESTWRITEINTS;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    FFXSTATUS       ffxStat;
} REQ_VBFDISABLESAVESTATE;

/*
typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    FFXSTATUS       ffxStat;
} REQ_VBFCOMPACTIONSUSPEND, * PREQ_VBFCOMPACTIONSUSPEND;

typedef struct
{
    FFXIOREQUEST    req;
    VBFHANDLE       hVBF;
    FFXSTATUS       ffxStat;
} REQ_VBFCOMPACTIONRESTORE, * PREQ_VBFCOMPACTIONRESTORE;
*/

#endif /* VBFREQ_H_INCLUDED */
