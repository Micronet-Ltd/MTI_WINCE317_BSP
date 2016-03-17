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
REM Revision 1.2  2011/01/27 17:24:48Z  johnb
REM Added Windws Embedded Compact 7 support
REM Added check for ARM7 CPU
REM Revision 1.1  2008/06/20 03:03:24Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_CPUFAMILY=
if "%1" == "x86" set P_CPUFAMILY=x86
if "%1" == "386" set P_CPUFAMILY=x86
if "%1" == "sh3" set P_CPUFAMILY=sh
if "%1" == "sh4" set P_CPUFAMILY=sh
if "%1" == "arm" set P_CPUFAMILY=arm
if "%1" == "armv4" set P_CPUFAMILY=arm
if "%1" == "armv4i" set P_CPUFAMILY=arm
if "%1" == "xscale" set P_CPUFAMILY=arm
if "%1" == "mipsii" set P_CPUFAMILY=mips
if "%1" == "armv7"   set P_CPUFAMILY=arm

if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo cpucheck.bat:  This ToolSet does not recognize the CPU type "%1"
echo.
echo The following CPU types are supported:  "x86"
echo                                         "386"
echo                                         "sh3"
echo                                         "sh4"
echo                                         "arm"
echo                                         "armv4"
echo                                         "armv4i"
echo                                         "xscale"
echo                                         "mipsii"
echo.


:Fini
