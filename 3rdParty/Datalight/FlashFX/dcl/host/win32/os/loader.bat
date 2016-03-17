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
REM $Log: loader.bat $
REM Revision 1.3  2009/12/14 20:43:34Z  keithg
REM Removed the B_ALLINONE variable - it is now set at the project level .
REM Revision 1.2  2009/05/16 00:21:22Z  garyp
REM Updated to support the "/info" command-line option.  General cleanup.
REM Revision 1.1  2007/10/10 02:42:30Z  brandont
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set B_ERROR=
set P_OSVENDOR=
set P_OSNAME=Loader

REM If obtaining info only, we're done
REM
if "%1" == "/info" goto Fini
if not "%1" == "" goto BadSyntax

REM This is not relevant for building the loader
REM
REM if "%P_OSROOT%" == "" goto BadRoot
REM if "%P_OSVER%" == "" goto BadVer


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
if exist %P_ROOT%\..\reliance\include\reliance_loader.h	set B_FSINC=%P_ROOT%\..\reliance\include
if exist %P_ROOT%\reliance\include\reliance_loader.h 	set B_FSINC=%P_ROOT%\reliance\include
if exist %P_ROOT%\include\reliance_loader.h		set B_FSINC=%P_ROOT%\include


REM success...
set B_ERROR=
goto Fini


:BadRoot
echo.
echo loader.bat:  The P_OSROOT environment variable must be set.  If there is no
echo              applicable OS root directory, use "N/A".
echo.
set B_ERROR=LOADER_BadOSROOT
%B_PAUSE%
goto Fini

:BadVer
echo.
echo loader.bat:  The P_OSVER environment variable must be set.  If there is no
echo              applicable version information, use "N/A".
echo.
set B_ERROR=LOADER_BadOSVER
%B_PAUSE%
goto Fini

:BadSyntax
echo.
echo loader.bat:  Command-line syntax error.
echo.
set B_ERROR=LOADER_BadSyntax
%B_PAUSE%
goto Fini



:Fini

