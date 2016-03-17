//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Abstract: Code specific to the network CPL
**/

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <commctrl.h>
#include <cpl.h>
#include <sipapi.h>
#include <owner.h>
#include <winsock.h>
#include <aygshell.h>

#include "regcpl.h"
#include "cplmacro.h"
#include "cplglobl.h"
#include "cplres.h"

#if defined(CPL_PORT)
#include "..\..\..\private\apps\inc\richink.h"
#include <Commdlg.h>
#include <ossvcs.h>
#include <wpcpriv.h>
#include <tchar.h>
#include "wpcpriv.h"
#include "path.h"
#include "eventsnd.h"
#include "phone.h"

#include <rcml.h>

#include "cfgmgr.h"         // For ConfigManager interfaces
#include <ras.h>

#include "tapi.h"
#include "tapihelp.h"
#include "unimodem.h"
#include "dbt.h"

#include "path.h"
#include "raserror.h"

//Dialer messages and notifications
#include "commctrl_priv.h" 

#ifndef CPL_IDNAME
#define CPL_IDNAME    100
/* this message gives the id (WCHAR) name of a cpl that can be used
** to find it's registry data hive (HKLM\ControlPanel\{name})
** lParam2 == pointer to string of buffer length 32.  the name is NOT
** required to be the display name
** lParam1 == which applet of the cpl
*/

#endif
#endif	// defined(CPL_PORT)



