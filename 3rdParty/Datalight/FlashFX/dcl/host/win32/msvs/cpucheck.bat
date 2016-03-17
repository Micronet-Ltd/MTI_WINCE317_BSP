@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to validate the CPU type and set the P_CPUFAMILY
REM value for a ToolSet.  The first and only parameter is the CPU type.  On
REM exit, the P_CPUFAMILY value will be set to the CPU family name, or NULL
REM if the CPU type is not valid for the ToolSet.
REM
REM If the CPU type is not valid, the batch file should display a brief list
REM of valid CPU types.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cpucheck.bat $
REM Revision 1.3  2008/07/22 23:21:48Z  keithg
REM Added support for VS 9.0 and broadened CPU support (x64)
REM Revision 1.2  2007/05/02 21:10:36Z  Garyp
REM Minor documentation and message cleanup.
REM Revision 1.1  2007/03/20 20:32:32Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_CPUFAMILY=
if "%1"=="386"   set P_CPUFAMILY=x86
if "%1"=="x64"   set P_CPUFAMILY=x86
if "%1"=="sh"    set P_CPUFAMILY=sh
if "%1"=="arm"   set P_CPUFAMILY=arm
if "%1"=="mips"  set P_CPUFAMILY=mips

if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo cpucheck.bat:  This ToolSet does not recognize the CPU type "%1"
echo.

echo The following CPU types are supported:  "386"
echo                                         "x64"
echo                                         "arm"
echo                                         "mips"
echo                                         "sh"

:Fini

