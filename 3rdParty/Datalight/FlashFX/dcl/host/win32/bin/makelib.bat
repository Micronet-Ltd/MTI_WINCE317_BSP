@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This is the default batch file to build a library for x86/DOS hosted
REM systems.  It is used internally by the product build process and is
REM not run directly.
REM
REM Note that this batch file will NOT be used if the ToolSet in use has
REM a batch file with this name.
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
REM Revision 1.1  2005/01/09 23:35:54Z  Pauli
REM Initial revision
REM Revision 1.1  2005/01/09 23:35:54Z  Garyp
REM Initial revision
REM Revision 1.1  2005/01/09 22:35:54Z  pauli
REM Initial revision
REM Revision 1.1  2005/01/09 21:35:53Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %echo%

shift

if "%1" == "" goto Cleanup
for %%a in (%1) do call %B_LIB% %0 -+%%a

if "%2" == "" goto Cleanup
for %%a in (%2) do call %B_LIB% %0 -+%%a

if "%3" == "" goto Cleanup
for %%a in (%3) do call %B_LIB% %0 -+%%a

if "%4" == "" goto Cleanup
for %%a in (%4) do call %B_LIB% %0 -+%%a

if "%5" == "" goto Cleanup
for %%a in (%5) do call %B_LIB% %0 -+%%a

if "%6" == "" goto Cleanup
for %%a in (%6) do call %B_LIB% %0 -+%%a

if "%7" == "" goto Cleanup
for %%a in (%7) do call %B_LIB% %0 -+%%a

if "%8" == "" goto Cleanup
for %%a in (%8) do call %B_LIB% %0 -+%%a

if "%9" == "" goto Cleanup
for %%a in (%9) do call %B_LIB% %0 -+%%a

:Cleanup

shift
if "%9" == "" goto Success

echo.
echo MAKELIB: Parameter overflow!
echo.
set B_ERROR=MAKELIB_Error
goto Fini

:Success
REM Indicate success
set B_ERROR=
goto Fini

:Fini
