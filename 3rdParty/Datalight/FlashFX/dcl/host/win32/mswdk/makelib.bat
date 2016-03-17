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
REM Revision 1.1  2006/11/15 22:04:30Z  jeremys
REM Initial revision
REM Revision 1.2  2006/11/15 22:04:30Z  Garyp
REM Removed remnants of some obsolete code to make environment space.
REM Revision 1.1  2005/12/01 02:19:08Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 02:19:07Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.2  2005/11/09 17:04:09Z  Pauli
REM Updated to use B_PAUSE so as to work with the "nopause" option.
REM Revision 1.1  2005/03/31 22:04:34Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

shift

REM Allow new files to be appended to any previously existing library

set OLDLIB=%0
if not exist %0 set OLDLIB=

if "%1" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %1

REM There may not have been an existing library on the first
REM iteration, but there will be on subsequent.
set OLDLIB=%0

if "%2" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %2

if "%3" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %3

if "%4" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %4

if "%5" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %5

if "%6" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %6

if "%7" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %7

if "%8" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %8

if "%9" == "" goto Cleanup
%B_LIB% /out:%0 %OLDLIB% %9


:Cleanup

shift
if "%9" == "" goto success

echo.
echo MAKELIB: Parameter overflow!
echo.
pause
set B_ERROR=MAKELIB_Error
%B_PAUSE%
goto fini

:success
REM Indicate success
set B_ERROR=
goto fini

:fini
set OLDLIB=
