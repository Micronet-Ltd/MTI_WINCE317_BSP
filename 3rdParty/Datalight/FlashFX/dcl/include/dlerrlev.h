/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This header contains logic to implement errorlevel codes for those
    environemnts which support this.

    This code is designed to allow mapping of a subset of the DCLSTAT_*
    values onto an integer errorlevel value in the range of 0 to 255.

    While the latest XP command-line extensions support a wider range of
    errorlevels, including negative numbers, we avoid this for now.  This
    is because a large number of automated processes expect that errorlevel
    0 means success, and a higher errorlevel, up to 255 means failure of
    some sort.

    Generally these should [ONLY] be used to return errorlevel values
    for DOS/XP/WinCE utilities which use errorlevels.  Therefore this
    header should/must be manually included in DOS/XP/CE specific code
    (or perhaps other code which makes explicit use of errorlevels).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlerrlev.h $
    Revision 1.5  2011/04/24 17:05:17Z  garyp
    Added DCLERRORLEVEL_INTERNALERROR.
    Revision 1.4  2010/12/08 19:16:14Z  billr
    Make dlapi.h and dlerrlev.h dual-license headers.
    Revision 1.3  2009/07/01 17:56:17Z  garyp
    Added an errorlevel value.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/08/15 19:40:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLERRLEV_H_INCLUDED
#define DLERRLEV_H_INCLUDED


/*-------------------------------------------------------------------
    This macro to converts a DCLSTAT_* value to an 8-bit unsigned
    integer errorlevel value.

    This macro <MUST> only be used on DCLSTAT_* values which have
    the status class of "Error" and the status locale of "General".
    If it is used on other types of codes, the results are not
    specified.
-------------------------------------------------------------------*/
#define MAKEERRLEV(stat)    ((stat) & 0xFF)


/*-------------------------------------------------------------------
    The following defines are for errorlevel values which are
    commonly used in a number of utilities.
-------------------------------------------------------------------*/
#define DCLERRORLEVEL_SUCCESS                   (MAKEERRLEV(DCLSTAT_SUCCESS))
#define DCLERRORLEVEL_HELPREQUEST               (MAKEERRLEV(DCLSTAT_HELPREQUEST))
#define DCLERRORLEVEL_BADREQUEST                (MAKEERRLEV(DCLSTAT_BADREQUEST))
#define DCLERRORLEVEL_BADSYNTAX                 (MAKEERRLEV(DCLSTAT_BADSYNTAX))
#define DCLERRORLEVEL_INVALIDDEVICE             (MAKEERRLEV(DCLSTAT_INVALIDDEVICE))
#define DCLERRORLEVEL_INVALIDDRIVE              (MAKEERRLEV(DCLSTAT_INVALIDDRIVE))
#define DCLERRORLEVEL_VERSIONMISMATCH           (MAKEERRLEV(DCLSTAT_VERSIONMISMATCH))
#define DCLERRORLEVEL_FEATUREDISABLED           (MAKEERRLEV(DCLSTAT_FEATUREDISABLED))
#define DCLERRORLEVEL_DRIVERLOCKFAILED          (MAKEERRLEV(DCLSTAT_DRIVERLOCKFAILED))
#define DCLERRORLEVEL_OUTOFMEMORY               (MAKEERRLEV(DCLSTAT_OUTOFMEMORY))
#define DCLERRORLEVEL_FAILURE                   (MAKEERRLEV(DCLSTAT_FAILURE))
#define DCLERRORLEVEL_INSTANCECREATIONFAILED    (MAKEERRLEV(DCLSTAT_INSTANCECREATIONFAILED))
#define DCLERRORLEVEL_INTERNALERROR             (MAKEERRLEV(DCLSTAT_INTERNAL_ERROR))


/*-------------------------------------------------------------------
    Mapping macros and function prototypes.
-------------------------------------------------------------------*/
#define DclStatusToErrorlevel   DCLFUNC(DclStatusToErrorlevel)

int DclStatusToErrorlevel(DCLSTATUS dclStat);



#endif  /* DLERRLEV_H_INCLUDED */


