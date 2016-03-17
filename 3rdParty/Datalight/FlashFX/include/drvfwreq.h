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
    $Log: drvfwreq.h $
    Revision 1.13  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.12  2009/07/22 17:45:23Z  garyp
    Merged from the v4.0 branch.  Modified the shutdown processes to take
    a mode parameter.
    Revision 1.11  2009/02/09 03:33:12Z  garyp
    Added REQ_DRVRFW_OPTIONGET.
    Revision 1.10  2008/03/26 01:08:46Z  Garyp
    Updated to support more Driver Framework functions.
    Revision 1.9  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/04/12 22:05:48Z  jimmb
    Added const to match the new prototype for FfxDriverDiskParameterSet
    Revision 1.7  2007/04/07 03:40:41Z  Garyp
    Added support for "ParameterSet".
    Revision 1.6  2007/01/06 05:19:43Z  Garyp
    Removed an unnecessary header.
    Revision 1.5  2006/11/10 20:27:03Z  Garyp
    Added "ParameterGet" and "Handle" support to the external API and
    removed "Stats" support.
    Revision 1.4  2006/05/08 00:30:59Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.3  2006/02/20 22:22:16Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.2  2006/02/13 09:13:12Z  Garyp
    Updated to new external API interface.
    Revision 1.1  2005/11/06 09:05:24Z  Pauli
    Initial revision
    Revision 1.2  2005/11/06 09:05:23Z  Garyp
    Updated to support the new statistics interface.
    Revision 1.1  2005/10/01 17:30:16Z  Garyp
    Initial revision
    Revision 1.2  2004/12/30 17:31:13Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/11/29 18:45:06Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DRVFWREQ_H_INCLUDED
#define DRVFWREQ_H_INCLUDED

#include <fxiosys.h>
#include <fxconfig.h>


/*-------------------------------------------------------------------
    NOTE! All the REQ_... structures <MUST> have a FFXIOREQUEST
          structure as the first field!.
-------------------------------------------------------------------*/
typedef struct
{
    FFXIOREQUEST        req;
    FFXCONFIGINFO      *pCI;
    D_BOOL              fVerbose;
    D_BOOL              fReset;
    FFXSTATUS           ffxStat;
} REQ_DRIVERGETCONFIGINFO;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDRIVERINFO      *pFDI;               /* IN */
    FFXDEVCONFIG       *pConf;              /* IN, buffer contents OUT */
    FFXDEVHOOK         *pHook;              /* IN */
    FFXDEVHANDLE        hDev;               /* OUT */
} REQ_DRVRFW_DEVCREATEPARAM;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDEVHANDLE        hDev;               /* IN */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DEVDESTROY;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDEVHANDLE        hDev;               /* IN */
    FFXDEVGEOMETRY     *pGeometry;          /* IN, buffer contents OUT */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DEVGEOMETRY;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDRIVERINFO      *pFDI;               /* IN */
    unsigned            nDevNum;            /* IN */
    FFXDEVHANDLE        hDev;               /* OUT */
} REQ_DRVRFW_DEVHANDLE;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDRIVERINFO      *pFDI;               /* IN */
    FFXDEVHANDLE        hDev;               /* IN-OUT */
} REQ_DRVRFW_DEVENUMERATE;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDEVCONFIG       *pConf;              /* IN, buffer contents OUT  */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DEVSETTINGS;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDEVCONFIG       *pConf;              /* IN, buffer contents OUT  */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DEVBBMSETTINGS;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDRIVERINFO      *pFDI;               /* IN */
    FFXDISKCONFIG      *pConf;              /* IN, buffer contents OUT  */
    FFXDISKHOOK        *pHook;              /* IN */
    FFXDISKHANDLE       hDisk;              /* OUT */
} REQ_DRVRFW_DISKCREATEPARAM;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDISKHANDLE       hDisk;              /* IN */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DISKDESTROY;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDISKHANDLE       hDisk;              /* IN */
    FFXDISKGEOMETRY    *pGeometry;          /* IN, buffer contents OUT */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DISKGEOMETRY;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDRIVERINFO      *pFDI;               /* IN */
    unsigned            nDiskNum;           /* IN */
    FFXDISKHANDLE       hDisk;              /* OUT */
} REQ_DRVRFW_DISKHANDLE;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDRIVERINFO      *pFDI;               /* IN */
    FFXDISKHANDLE       hDisk;              /* IN-OUT */
} REQ_DRVRFW_DISKENUMERATE;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDISKHANDLE       hDisk;              /* IN */
    FFXPARAM            id;                 /* IN */
    void               *pBuffer;            /* IN, buffer contents OUT */
    D_UINT32            ulBuffLen;          /* IN */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DISKPARAMGET;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDISKHANDLE       hDisk;              /* IN */
    FFXPARAM            id;                 /* IN */
    const void         *pBuffer;            /* IN */
    D_UINT32            ulBuffLen;          /* IN */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_DISKPARAMSET;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDISKHANDLE       hDisk;              /* IN */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_ALLOCATORCREATE;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXDISKHANDLE       hDisk;              /* IN */
    D_UINT32            ulFlags;            /* IN */
    FFXSTATUS           ffxStat;            /* OUT */
} REQ_DRVRFW_ALLOCATORDESTROY;

typedef struct
{
    FFXIOREQUEST        req;                /* IN */
    FFXOPTION           option;             /* IN */
    void               *handle;             /* IN */
    void               *pBuffer;            /* IN, buffer contents OUT */
    D_UINT32            ulBuffLen;          /* IN */
    D_BOOL              fResult;            /* OUT */
} REQ_DRVRFW_OPTIONGET;



#endif /* DRVFWREQ_H_INCLUDED */



