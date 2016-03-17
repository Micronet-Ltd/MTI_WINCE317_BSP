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
REM Revision 1.1  2006/11/02 19:47:08Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %echo%

shift

for %%a in (%1 %2 %3 %4 %5 %6 %7 %8 %9) do call %B_LIB% -rcem %0 %%a

shift
if "%9" == "" goto success

echo.
echo makelib.bat: Parameter overflow!
echo.
set B_ERROR=MAKELIB_BAT_Error
%B_PAUSE%
goto Fini

:success
REM Indicate success
set B_ERROR=
goto Fini

:Fini
