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

    This module contains functions to manage loading and unloading controls
    in commctrl.dll.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: wincommoncontrols.c $
    Revision 1.3  2010/11/01 04:16:05Z  garyp
    Documentation and debug messages updated -- no functional changes.
    Revision 1.2  2009/06/28 02:46:23Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2008/11/05 01:50:39Z  garyp
    Documentation updated.
    Revision 1.1  2008/09/01 06:18:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <commctrl.h>

#include <dcl.h>
#include <dlwinutil.h>

typedef BOOL (*PFNINITCOMMONCONTRLSEX)(INITCOMMONCONTROLSEX*);


/*-------------------------------------------------------------------
    DclWinCommonControlsLoad()

    Load the common controls DLL using LoadLibrary(), and initialize
    the specified control classes.  See the Windows function 
    InitCommonControlsEx() for information on the various control
    class values.

    This function should be evenly paired with a call to
    DclWinCommonControlsUnload().

    Parameters:
        dwICC   - The control classes to initialize.

    Return Value:
        The module handle for commctrl.dll if successful, or NULL
        otherwise.
-------------------------------------------------------------------*/
HMODULE DclWinCommonControlsLoad(
    DWORD                   dwICC)
{
    HMODULE                 hModCommCtrl;
    PFNINITCOMMONCONTRLSEX  pfnInitCommCtrl;
    INITCOMMONCONTROLSEX    icc = {0};

    /*  Load the common controls DLL.
    */
    hModCommCtrl = LoadLibrary(TEXT("commctrl.dll"));
    if(!hModCommCtrl)
    {
        DCLPRINTF(1, ("DclWinCommonControlsLoad() failed to load the DLL\n"));
        return NULL;
    }

    /*  Get a pointer to the InitCommonControls function in commctrl.dll and
        call it to initialize common controls used by the GUI.
    */
  #ifdef _WIN32_WCE
    pfnInitCommCtrl = (PFNINITCOMMONCONTRLSEX)GetProcAddress(hModCommCtrl, TEXT("InitCommonControlsEx"));
  #else
    pfnInitCommCtrl = (PFNINITCOMMONCONTRLSEX)GetProcAddress(hModCommCtrl, "InitCommonControlsEx");
  #endif
    if(!pfnInitCommCtrl)
    {
        DCLPRINTF(1, ("DclWinCommonControlsLoad() failed to find proc address for InitCommonControlsEx()\n"));

        FreeLibrary(hModCommCtrl);
        return NULL;
    }

    icc.dwSize = sizeof(icc);
    icc.dwICC = dwICC;

    if(!pfnInitCommCtrl(&icc))
    {
        DCLPRINTF(1, ("DclWinCommonControlsLoad() failed to initialize common control classes %lX\n", dwICC));

        FreeLibrary(hModCommCtrl);
        return NULL;
    }

    return hModCommCtrl;
}


/*-------------------------------------------------------------------
    DclWinCommonControlsUnload()

    Unload the common controls DLL which was loaded using the API
    DclWinCommonControlsLoad().  Technically FreeLibrary() will
    decrement the usage count, and unload the DLL only if it has
    gone to zero.

    Parameters:
        hModCommCtrl - The handle returned by DclWinCommonControlsLoad().

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclWinCommonControlsUnload(
    HMODULE                 hModCommCtrl)
{
    DclAssert(hModCommCtrl);

    return FreeLibrary(hModCommCtrl) ? TRUE : FALSE;
}

