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
REM Revision 1.1  2010/04/27 22:18:32Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM Allow new files to be appended to any previously existing library

rem  If the commented out statment below is found to be necessary, the
rem  change must be conditioned for the specific case where it is an
rem  issue.  Otherwise, the final driver library creation will take
rem  a very long time.
rem
rem  for %%a in (%2 %3 %4 %5 %6 %7 %8 %9) do call %B_LIB% crus %1 %%a
rem
call %B_LIB% a %1 %2 %3 %4 %5 %6 %7 %8 %9

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

:fini

