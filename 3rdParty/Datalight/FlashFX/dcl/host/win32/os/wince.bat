@echo off

REM ----------------------------------------------------------------------
REM                             Description
REM
REM This batch file initializes the build environment for any required OS
REM specific information.  It is called by MAKEALL.BAT after PRJINFO.BAT
REM has run, but before the TOOLINIT.BAT and the various build environment
REM variables are initialized.  It may also be called by the kit building
REM process with the "/info" option.
REM
REM This batch file is typically useful in two scenarios:
REM 1) In environments where the OS and the ToolSet are distinct, if the
REM    driver can be built with more than one ToolSet, often include paths
REM    must differ.
REM 2) This abstraction allows OS specific extensions to the build process
REM    to be implemented.
REM 3) It is used by the kit building process to obtain the proper name of
REM    the OS.
REM
REM The only possible parameter is /info, which if used will cause the batch
REM file to set the P_OSNAME variable to the proper name of the operating
REM system, and skip the remaining initialization.
REM
REM B_ERROR must be set to indicate an error, or clear if successful.
REM ----------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: wince.bat $
REM Revision 1.8  2011/01/27 17:12:30Z  johnb
REM Added Windws Embedded Compact 7 support
REM Revision 1.7  2009/12/14 20:43:38Z  keithg
REM Removed the B_ALLINONE variable - it is now set at the project level .
REM Revision 1.6  2009/06/25 00:19:29Z  garyp
REM Updated to accommodate WinCE build environments where the environment
REM variable names might not be all uppercase as case-sensitive make files
REM expect.
REM Revision 1.5  2009/05/16 00:19:33Z  garyp
REM Updated to support the "/info" command-line option.  General cleanup.
REM Revision 1.4  2007/11/24 18:02:50Z  Garyp
REM Added support for CE 5.02 -- AKA WinMobile 6.
REM Revision 1.3  2007/03/01 21:17:24Z  Garyp
REM Added CE 6.0 support.  Removed CE 3.0 support.
REM Revision 1.2  2006/03/08 05:50:30Z  Pauli
REM Turned on B_NEEDSOSHEADERS.
REM Removed include path from the general list that is only needed by
REM the crashtest code.
REM Revision 1.1  2005/12/02 16:14:26Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/02 16:14:26Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.1  2005/08/21 05:58:42Z  Garyp
REM Initial revision
REM Revision 1.2  2005/08/21 05:58:41Z  garyp
REM Updated to use new mechanism for specifying OS includes and defines.
REM Revision 1.1  2005/03/17 04:05:46Z  pauli
REM Initial revision
REM Revision 1.3  2005/03/17 03:05:46Z  GaryP
REM Updated to use P_OSROOT and P_OSVER.
REM Revision 1.2  2005/02/27 05:23:38Z  GaryP
REM Normalized settings to work better with CE 5.
REM Revision 1.1  2005/01/20 01:49:18Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set B_ERROR=
set P_OSVENDOR=Microsoft
set P_OSNAME=Windows CE

REM If obtaining info only, we're done
REM
if "%1" == "/info" goto Fini
if not "%1" == "" goto BadSyntax

if "%_WINCEOSVER%" == "" goto BadCEEnv
if "%_PROJECTROOT%" == "" goto BadPROJECTROOT

REM For CE, the OS root is the same as the tool root
set P_OSROOT=%P_TOOLROOT%

REM Win32 batch language is indifferent to the case of environment variable
REM names, however GNU Make cares very much.  Some CE environments may set
REM these variables as lowercase, which causes problems.  This logic ensures
REM that the variable names are all uppercase.
REM
set T_TMP=%_WINCEROOT%
set _WINCEROOT=
set _WINCEROOT=%T_TMP%

set T_TMP=%_WINCEOSVER%
set _WINCEOSVER=
set _WINCEOSVER=%T_TMP%

set T_TMP=%_PROJECTROOT%
set _PROJECTROOT=
set _PROJECTROOT=%T_TMP%

set P_OSVER=
if "%_WINCEOSVER%" == "400" set P_OSVER=4.00
if "%_WINCEOSVER%" == "410" set P_OSVER=4.10
if "%_WINCEOSVER%" == "420" set P_OSVER=4.20
if "%_WINCEOSVER%" == "500" set P_OSVER=5.00
if "%_WINCEOSVER%" == "502" set P_OSVER=5.02
if "%_WINCEOSVER%" == "600" set P_OSVER=6.00
if "%_WINCEOSVER%" == "700" set P_OSVER=7.00
if "%P_OSVER%" == "" goto BadCEVer

set B_NEEDSOSHEADERS=1
set B_OSINC=%P_OSROOT%\public\common\ddk\inc
set B_CLIBINC=%P_OSROOT%\public\common\oak\inc

:Success
REM success...
set B_ERROR=
goto Fini

:BadCEEnv
echo.
echo wince.bat: The environment variable "_WINCEOSVER" must be set to use
echo            this ToolSet.  This variable is typically set to "400",
echo            "410", "420", "500", "600", etc.
echo.
set B_ERROR=WINCE_BadCEEnv
%B_PAUSE%
goto Fini

:BadCEVer
echo.
echo wince.bat:  The CE version "%_WINCEOSVER%" is not supported at this time.
echo.
set B_ERROR=WINCE_BadCEVer
%B_PAUSE%
goto Fini

:BadPROJECTROOT
echo.
echo wince.bat: The _PROJECTROOT environment variable must be set to the root
echo            of the Windows CE project being built.
echo.
set B_ERROR=WINCE_PROJECTROOT
%B_PAUSE%
goto Fini

:BadSyntax
echo.
echo wince.bat:  Command-line syntax error.
echo.
set B_ERROR=WINCE_BadSyntax
%B_PAUSE%
goto Fini


:Fini
set T_TMP=

