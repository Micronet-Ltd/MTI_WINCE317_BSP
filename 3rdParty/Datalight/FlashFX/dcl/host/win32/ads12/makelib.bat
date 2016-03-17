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
REM Revision 1.1  2005/12/01 01:48:46Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 01:48:46Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.3  2005/11/09 17:04:10Z  Pauli
REM Changed PAUSE to B_PAUSE for consistency with the build process.
REM Revision 1.2  2005/11/09 16:39:28Z  Pauli
REM Updated to correctly adhere to the "/nopause" option.
REM Revision 1.1  2005/01/09 21:35:53Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %echo%

shift

for %%a in (%1 %2 %3 %4 %5 %6 %7 %8 %9) do call %B_LIB% -c -r %0 %%a

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
