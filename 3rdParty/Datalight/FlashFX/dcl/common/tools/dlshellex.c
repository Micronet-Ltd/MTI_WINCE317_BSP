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

    This module implements the Datalight command shell and adds the basic,
    built in commands.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlshellex.c $
    Revision 1.3  2010/08/04 01:00:06Z  garyp
    Updated to use the default system and prefix names for FlashFX and
    Reliance.
    Revision 1.2  2009/11/08 03:51:12Z  garyp
    Eliiminated explicit profiler creation and destruction, since it now
    initializes as a service.
    Revision 1.1  2009/03/03 02:56:52Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlshell.h>


/*-------------------------------------------------------------------
    DclShellExecute()

    Description
        This function initializes and runs the 4GR shell.

    Parameters
        pszTitle  - The shell instance title, or NULL if none.
                    This value will be used as the shell prompt.
        pszScript - A pointer to a script to play, or NULL if none.
        hEnv      - The environment handle to use, or NULL to use a
                    default environment.
        fQuiet    - Initialize without signing on.
        fEcho     - Indicates whether input should be echoed.
        pUserData - An optional pointer to any arbitrary data
                    which the caller needs to have available to
                    the shell commands.

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclShellExecute(
    const char     *pszTitle,
    const char     *pszScript,
    DCLENVHANDLE    hEnv,
    unsigned        fQuiet,
    unsigned        fEcho,
    void           *pUserData)
{
  #if DCLCONF_COMMAND_SHELL
    DCLSTATUS       dclStat = DCLSTAT_SHELLCREATEFAILED;
    DCLSHELLHANDLE  hShell;

    hShell = DclShellCreate(NULL, pszTitle, pszScript, hEnv, fQuiet, fEcho, pUserData);
    if(hShell)
    {
      #if (D_PRODUCTNUM == PRODUCTNUM_4GR) || (D_PRODUCTNUM == PRODUCTNUM_RELIANCE) || (D_PRODUCTNUM == PRODUCTNUM_DCL)
        dclStat = DclShellAddFileSystemCommands(hShell);
      #endif
        if(dclStat == DCLSTAT_SUCCESS)
        {
          #if (D_PRODUCTNUM == PRODUCTNUM_4GR) || (D_PRODUCTNUM == PRODUCTNUM_FLASHFX)
            dclStat = FfxShellAddCommands(hShell, NULL, NULL);
          #endif
            if(dclStat == DCLSTAT_SUCCESS)
            {
              #if (D_PRODUCTNUM == PRODUCTNUM_4GR) || (D_PRODUCTNUM == PRODUCTNUM_RELIANCE)
                dclStat = RelianceShellAddCommands(hShell, "NULL", "NULL");
              #endif
                if(dclStat == DCLSTAT_SUCCESS)
                {
                  #if (D_PRODUCTNUM == PRODUCTNUM_4GR)
                    dclStat = DL4GRShellAddCommands(hShell, "4GR", "4GR");
                  #endif
                    if(dclStat == DCLSTAT_SUCCESS)
                        dclStat = DclShellRun(hShell);
                }
            }
        }

        DclShellDestroy(hShell);
    }

    return dclStat;

  #else

    (void)pszTitle;
    (void)pszScript;
    (void)hEnv;
    (void)fQuiet;
    (void)fEcho;
    (void)pUserData;

    DclPrintf("The Shell functionality is not enabled\n");

    return DCLSTAT_SHELLDISABLED;

  #endif
}

