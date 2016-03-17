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
  jurisdictions.  Patents may be pending.

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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header defines the interface to the DCL tools for Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcetools.h $
    Revision 1.4  2010/12/10 02:53:57Z  garyp
    Condition the volume stuff on CE 5.0 or later.
    Revision 1.3  2010/12/01 23:40:17Z  garyp
    Added prototypes.
    Revision 1.2  2009/06/27 23:15:16Z  garyp
    Prototype update.
    Revision 1.1  2007/12/01 18:17:58Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLCETOOLS_H_INCLUDED
#define DLCETOOLS_H_INCLUDED

#include <dltools.h>


#if DCLCONF_COMMAND_SHELL
#define     DclCeShell                  DCLFUNC(DclCeShell)
DCLSTATUS   DclCeShell(DCLTOOLPARAMS *pTP);
#endif

#define     DclCeDriverLoad             DCLFUNC(DclCeDriverLoad)
#define     DclCePartition              DCLFUNC(DclCePartition)

DCLSTATUS   DclCeDriverLoad(const char *pszCmdName, char *pszCmdLine);
DCLSTATUS   DclCePartition(const char *pszCmdName, char *pszCmdLine);

#if _WIN32_WCE >= 500 
#define     DclCeVolume                 DCLFUNC(DclCeVolume)
#define     DclCeVolumeDisplayInfo      DCLFUNC(DclCeVolumeDisplayInfo)

DCLSTATUS   DclCeVolume(const char *pszCmdName, char *pszCmdLine);
DWORD       DclCeVolumeDisplayInfo(const D_WCHAR *pwzVolumeName);
#endif


#endif  /* DLCETOOLS_H_INCLUDED */


