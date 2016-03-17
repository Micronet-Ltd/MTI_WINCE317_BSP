@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file implements the default BLDLAYER functionality which is
REM used for any ToolSet that does not have its own bldlayer.bat.  It is
REM used internally by the product build process and is not run directly.
REM
REM On entry, the first parameter must be the name of the product layer,
REM ie, Core, Common, etc.  Note that the given product layer must contain
REM a .MAK file with the exact same name.
REM
REM The second parameter may be an optional target to build.
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: bldlayer.bat $
REM Revision 1.3  2006/12/08 01:33:55Z  Garyp
REM Fixed logic to ensure that the .err files are always processed properly
REM Broke in the previous rev).
REM Revision 1.2  2006/11/17 17:34:27Z  Garyp
REM Eliminated the obsolete "autobld" functionality.  Changed the way logging
REM is done to avoid sharing violations on XP with GNU Make.
REM Revision 1.1  2005/01/11 21:20:36Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "" goto Syntax
if not exist %1.mak goto BadLayer

echo                           %1

set B_ERROR=

if exist *.err del *.err
if exist %1.log del %1.log

REM Windows XP and/or GNU Make sometimes have sharing issues if we try to
REM append the output directly onto the log file (%B_LOG%).  Therefore jump
REM through a couple levels of indirection to avoid this.

%B_MAKE% %B_BLDOPTS% -f%1.mak %2>%1.log
if errorlevel 1 set B_ERROR=MakeFailed

REM Ensure that a log.tmp file exists...
echo. >log.tmp
if exist %1.log copy %1.log log.tmp >nul
if exist *.err copy /a log.tmp + *.err log.tmp >nul
copy /a %B_LOG% + log.tmp %B_LOG% >nul
del log.tmp

if "%B_ERROR%" == "" goto Fini

echo Error, MAKE failed for %1
echo Error, MAKE failed for %1>>%B_LOG%
goto Fini

:BadLayer
echo bldlayer.bat: Error, cannot build the layer "%1"
echo bldlayer.bat: Error, cannot build the layer "%1">>%B_LOG%
goto Fini

:Syntax
echo bldlayer.bat: Syntax Error!
set B_ERROR=BLDLAYER_Syntax
goto Fini

:Fini
