@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM  !! This overrides the the default batch file located in ..\bin. !!
REM
REM This batch file is used to add the contents of one library to another.
REM It is used internally by the product build process and is not run
REM directly.
REM
REM On entry, %1 will be the output library name, and params %2+ will
REM be the names of the other libraries whose contents must be added to
REM the output library.  Both the output and source libraries MUST be in
REM the current directory -- the specifications may not include a path.
REM
REM Up to 8 source libraries may be specified in addition to the output
REM library name.
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: addlibs.bat $
REM Revision 1.1  2010/04/28 18:20:06Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set LIBNAME=%1

:NextLib
shift
if "%1" == "" goto Success

REM Extract all the modules
REM
%B_LIB% x %1

REM Add the modules to the output library
REM
call %B_BUILDLIB% %LIBNAME% *.%B_OBJEXT%
if not "%B_ERROR%" == "" goto MakeLibErr

del *.%B_OBJEXT%

goto NextLib


:MakeLibErr
echo.
echo addlibs.bat: Error running MAKELIB "%B_ERROR%"
echo.
set B_ERROR=ADDLIBS_%B_ERROR%
%B_PAUSE%
goto Fini

:Success
REM Indicate success
set B_ERROR=
goto Fini

:Fini
set LIBNAME=


