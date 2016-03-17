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
REM $Log: wincebl.bat $
REM Revision 1.2  2009/05/16 00:19:25Z  garyp
REM Updated to support the "/info" command-line option.  General cleanup.
REM Revision 1.1  2005/03/17 05:05:42Z  Pauli
REM Initial revision
REM Revision 1.2  2005/03/17 03:05:42Z  GaryP
REM Updated to simply call the WinCE initialization batch file.
REM Revision 1.1  2005/01/09 21:35:46Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set B_ERROR=
set P_OSVENDOR=Microsoft
set P_OSNAME=Windows CE Bootloader

REM If obtaining info only, we're done
REM
if "%1" == "/info" goto Fini

REM For the time being, the WinCE BootLoader initialization process is
REM identical to the regular CE initialization, so just call that batch
REM file.

shift
call %P_DCLROOT%\host\win32\os\wince.bat %1 %2 %3 %4 %5 %6 %7 %8 %9

:Fini

