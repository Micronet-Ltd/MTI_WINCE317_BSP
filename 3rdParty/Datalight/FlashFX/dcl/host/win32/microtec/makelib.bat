@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM !! This overrides the the default batch file located in ..\bin.        !!
REM
REM It is used internally by the product build process and is not run
REM directly.
REM
REM On entry, %1 will be the output library name, and params %2+ will
REM be the names of the files to add to the library (wildcards OK).
REM
REM Up to 9 modules may be specified in addition to the library name.
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: makelib.bat $
REM Revision 1.1  2006/12/07 23:37:58Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set LIBNAME=%1
shift
shift

REM Create the library if it does not already exist.  Unfortunately this
REM Microtec tool is stupid and a module must be added at the same time
REM the library is initially created.  If not, subsequent runs will generate
REM a "library does not exist" message -- even though create by itself will
REM in fact create a library file!
if not exist %LIBNAME% %B_LIB% -C -a %0 %LIBNAME% && shift

for %%a in (%0 %1 %2 %3 %4 %5 %6 %7 %8 %9) do call %B_LIB% -a %%a %LIBNAME%

shift
if "%9" == "" goto Success

echo.
echo makelib.bat: Parameter overflow!
echo.
set B_ERROR=MAKELIB_Error
%B_PAUSE%
goto Fini

:Success
REM Indicate success
set B_ERROR=
goto Fini

:Fini
set LIBNAME=
