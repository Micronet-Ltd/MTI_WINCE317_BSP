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
REM $Log: rtos.bat $
REM Revision 1.10  2009/12/14 20:43:36Z  keithg
REM Removed the B_ALLINONE variable - it is now set at the project level .
REM Revision 1.9  2009/05/16 00:20:39Z  garyp
REM Updated to support the "/info" command-line option.  General cleanup.
REM Revision 1.8  2007/02/09 19:48:12Z  brandont
REM Reversed the directory search order looking for reliance_rtos.h.
REM Revision 1.7  2006/09/27 23:16:44Z  garyp
REM Fixed a broken error message.
REM Revision 1.6  2006/08/28 21:28:02Z  pauli
REM Added validation for P_OSROOT and P_OSVER and appropriate
REM error messages if they are not set.
REM Revision 1.5  2006/08/25 18:33:19Z  Pauli
REM Updated to not set P_OSROOT, P_OSVER or B_OSINC to allow
REM these to be set per project in prjinfo.bat.
REM Revision 1.4  2006/03/02 19:43:41Z  Pauli
REM Created a env var for a file system path.  Set this to the Reliance include
REM directory.
REM Revision 1.3  2006/03/01 20:01:18Z  Pauli
REM Added the main Reliance include directory to the OS include paths.
REM Revision 1.2  2006/02/09 18:17:39Z  Pauli
REM Disabled "All-In-One".
REM Revision 1.1  2005/12/02 16:13:18Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/02 16:13:17Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.4  2005/11/09 17:04:09Z  Pauli
REM Updated to use B_PAUSE so as to work with the "nopause" option.
REM Revision 1.3  2005/03/17 03:05:42Z  GaryP
REM Updated to use P_OSROOT and P_OSVER.
REM Revision 1.2  2005/02/22 00:10:10Z  GaryP
REM Changed to force ALL-IN-ONE behavior.
REM Revision 1.1  2005/01/09 21:35:50Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set B_ERROR=
set P_OSVENDOR=
set P_OSNAME=RTOS

REM If obtaining info only, we're done
REM
if "%1" == "/info" goto Fini
if not "%1" == "" goto BadSyntax

if "%P_OSROOT%" == "" goto BadRoot
if "%P_OSVER%" == "" goto BadVer


REM By not setting B_OSINC, we assume that any necessary includes will
REM be taken care of by the ToolSet.  If this value is set, and the
REM B_NEEDSOSHEADERS value is set to '1' in any given makefile, the product
REM build process will ensure that the path specified here is a part of
REM the standard include path.
REM set B_OSINC=


REM In OS environments where a file system interface is available, DCL is
REM configured to use that interface.  However, for the RTOS kit we cannot
REM assume there is such an interface.  Therefore the RTOS kits for FlashFX
REM and Reliance are configured to use the Reliance file system API.  This
REM breaks the rule that DCL should not have any knowledge of any other
REM products.  To handle this, we setup a file system include path for
REM Reliance, if we can find it.
REM
REM We check the following cases:
REM	Reliance is the master product.
REM	Reliance is a subproduct.
REM	Reliance is installed in the same directory as the master product.
REM
set B_FSINC=
if exist %P_ROOT%\..\reliance\include\reliance_rtos.h	set B_FSINC=%P_ROOT%\..\reliance\include
if exist %P_ROOT%\reliance\include\reliance_rtos.h 	set B_FSINC=%P_ROOT%\reliance\include
if exist %P_ROOT%\include\reliance_rtos.h		set B_FSINC=%P_ROOT%\include


REM success...
set B_ERROR=
goto Fini


:BadRoot
echo.
echo rtos.bat:  The P_OSROOT environment variable must be set.  If there is no
echo            applicable OS root directory, use "N/A".
echo.
set B_ERROR=RTOS_BadOSROOT
%B_PAUSE%
goto Fini

:BadVer
echo.
echo rtos.bat:  The P_OSVER environment variable must be set.  If there is no
echo            applicable version information, use "N/A".
echo.
set B_ERROR=RTOS_BadOSVER
%B_PAUSE%
goto Fini

:BadSyntax
echo.
echo rtos.bat:  Command-line syntax error.
echo.
set B_ERROR=RTOS_BadSyntax
%B_PAUSE%
goto Fini


:Fini

