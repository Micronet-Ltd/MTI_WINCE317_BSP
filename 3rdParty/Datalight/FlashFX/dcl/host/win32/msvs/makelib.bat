@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM !! This overrides the the default batch file located in ..\bin.        !!
REM
REM It is used internally by the product build process and is not run
REM directly.
REM
REM On entry, %1 will be the output library name, and parms %2+ will
REM be the names of the files to add to the library (wildcards OK).
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: makelib.bat $
REM Revision 1.3  2010/06/06 19:45:58Z  garyp
REM Removed a pause on general command failure.
REM Revision 1.2  2009/06/17 19:04:02Z  garyp
REM Updated to allow more than 9 modules to be specified.
REM Revision 1.1  2007/03/20 20:32:32Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%2" == "" goto Syntax

REM Allow new files to be appended to any previously existing library

set T_OUTFILE=%1
set T_OLDLIB=%1
if not exist %1 set T_OLDLIB=

shift && shift

:NextBatch
%B_LIB% /out:%T_OUTFILE% %T_OLDLIB% %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 goto Failure

REM There may not have been an existing library on the first
REM iteration, but there will be on subsequent.
set T_OLDLIB=%T_OUTFILE%

shift && shift && shift && shift && shift && shift && shift && shift && shift && shift
if not "%0" == "" goto NextBatch

REM Indicate success
set B_ERROR=
set T_OLDLIB=
set T_OUTFILE=
goto Fini

:Syntax
echo.
echo MAKELIB: Bad syntax!
echo.
set B_ERROR=MAKELIB_Syntax
%B_PAUSE%
goto Fini

:Failure
echo.
echo MAKELIB: LIB command failed!
echo.
set B_ERROR=MAKELIB_LibFailed

REM Make sure no partial results are laying around, especially because the
REM makefiles may not check for errors after this command.
REM
if exist %T_OUTFILE% del %T_OUTFILE%
goto Fini

:Fini
