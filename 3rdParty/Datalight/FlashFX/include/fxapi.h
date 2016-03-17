/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This is the top-level header required by code using the portion of the
    FlashFX externally visible API which is the same across all operating
    systems.  Code which uses this header MAY not be using the Datalight
    standard build process, and therefore this header may not include any
    sub-headers which are not in the same directory as this header.  This
    header is typically included by the flashfx_osname.h headers.

    In addition to this header, there are two other available top-level
    headers, which are:

    flashfx.h
        This header is used for all internal FlashFX code.  It includes
        pretty much everything, and the code MUST always be built by the
        Datalight standard build process.  If multiple top-level headers
        are included, this one MUST be first.

    flashfx_osname.h
        This header is used by OS-specific code which calls FlashFX
        functionality, but is NOT being built by the Datalight standard
        build process.  Therefore this header may not include any sub-
        headers which are not in the same directory as this header.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxapi.h $
    Revision 1.10  2009/10/09 01:19:11Z  garyp
    Updated to include fxtools.h and removed a duplicated prototype.
    Revision 1.9  2009/08/07 23:22:25Z  garyp
    Added some default project settings to be used only in the event that the
    code is being built outside the standard DL build process, with no access
    to the project settings.
    Revision 1.8  2009/08/04 17:26:28Z  garyp
    Merged from the v4.0 branch.  Updated the prototype for FlashFX_Open().
    Revision 1.7  2009/03/27 02:08:50Z  keithg
    Moved FFXREQHANDLE into fxtypes.h module.
    Revision 1.6  2009/03/26 16:57:23Z  thomd
    Removed an unnecessary header.
    Revision 1.5  2009/02/09 02:29:17Z  garyp
    Merged from the v4.0 branch.  Moved the FlashFX_VersionCheck() here
    from ffxapi.h.  Added headers necessary to support applications using
    the external API.  Added the FlashFX_Open() and FlashFX_Close() APIs.
    Revision 1.4  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/02/04 00:21:27Z  Garyp
    Added prototypes.
    Revision 1.2  2007/01/20 01:11:32Z  pauli
    Minor documentation correction.
    Revision 1.1  2007/01/12 23:00:58Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXAPI_H_INCLUDED
#define FXAPI_H_INCLUDED


/*-------------------------------------------------------------------
    These checks ONLY exist to resolve the problem that externally
    built programs will not have the project definitions of these
    settings.  Define these all to be TRUE, and let any issues be
    caught at run-time.

    Conversely, internally built Datalight code will have the project
    definitions, which is highly preferable, and allows more accurate
    compile-time scaling of the product.
-------------------------------------------------------------------*/
#ifndef FFXCONF_NANDSUPPORT
#define FFXCONF_NANDSUPPORT         TRUE
#endif
#ifndef FFXCONF_NORSUPPORT
#define FFXCONF_NORSUPPORT          TRUE
#endif
#ifndef FFXCONF_ISWFSUPPORT
#define FFXCONF_ISWFSUPPORT         TRUE
#endif
#ifndef FFXCONF_BBMSUPPORT
#define FFXCONF_BBMSUPPORT          FFXCONF_NANDSUPPORT
#endif
#ifndef FFXCONF_POWERSUSPENDRESUME
#define FFXCONF_POWERSUSPENDRESUME  TRUE
#endif


/*-------------------------------------------------------------------
    Include FlashFX headers which provide access to the portions of
    the publicly exposed FlashFX API which are not OS-specific.

    Note the use of quotes rather than angle brackets.  Because this
    code may be built outside the Datalight build process, this
    header may only include Datalight headers which can be located
    relative to the current directory in which this header resides.
-------------------------------------------------------------------*/
#include "fxstatus.h"
#include "fxtypes.h"
#include "fxparam.h"
#include "fxtools.h"


/*-------------------------------------------------------------------
    Prototypes and structures for externally visible functions.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulVersion;
    char        szBuildNum[DCL_MAX_BUILDNUM_LENGTH+1];
} FFXVERSION;


FFXSTATUS   FlashFX_VersionCheck(FFXVERSION *pVer, D_BOOL fQuiet);
FFXSTATUS   FlashFX_Open(unsigned nDiskNum, DCLINSTANCEHANDLE *phDclInst, FFXREQHANDLE *phReq);
FFXSTATUS   FlashFX_Close(FFXREQHANDLE hReq);

FFXSTATUS   FlashFXShellAddCommands(DCLSHELLHANDLE hShell);


#endif  /* FXAPI_H_INCLUDED */

