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
REM Revision 1.3  2009/02/25 03:13:36Z  brandont
REM Removed for loop that was causing builds to take more than 5 minutes.
REM Revision 1.2  2006/11/15 22:04:29Z  Garyp
REM Removed remnants from some obsolete code to make environment space.
REM Revision 1.1  2005/12/01 02:07:44Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 02:07:44Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.2  2005/11/09 17:04:09Z  Pauli
REM Updated to use B_PAUSE so as to work with the "nopause" option.
REM Revision 1.1  2005/04/24 17:25:16Z  GaryP
REM Initial revision
REM Revision 1.3  2004/12/17 21:40:32Z  PaulI
REM Changed to use a for loop to expand wildcards.  The VxWorks 5.4
REM link tool does not support wildcards.
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
call %B_LIB% crus %1 %2 %3 %4 %5 %6 %7 %8 %9

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

