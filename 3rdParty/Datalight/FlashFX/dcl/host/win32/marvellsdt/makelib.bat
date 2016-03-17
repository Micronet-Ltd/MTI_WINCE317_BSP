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
REM Up to 9 modules may be specified in addition to the library name.
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: makelib.bat $
REM Revision 1.1  2008/07/18 01:58:30Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set LIBNAME=%1

:NextGroup
if "%2" == "" goto Fini
%B_LIB% -add %LIBNAME% %2
shift
goto NextGroup


shift

REM Allow new files to be appended to any previously existing library

set OLDLIB=%0
REM if not exist %0 set OLDLIB=

if "%1" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %1

REM There may not have been an existing library on the first
REM iteration, but there will be on subsequent.
set OLDLIB=%0

if "%2" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %2

if "%3" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %3

if "%4" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %4

if "%5" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %5

if "%6" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %6

if "%7" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %7

if "%8" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %8

if "%9" == "" goto Cleanup
%B_LIB% -add %OLDLIB% %9


:Cleanup

shift
if "%9" == "" goto success

echo.
echo MAKELIB: Parameter overflow!
echo.
set B_ERROR=MAKELIB_Error
%B_PAUSE%
goto fini

:success
REM Indicate success
set B_ERROR=
goto fini

:Fini
set LIBNAME=

