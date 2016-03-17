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
REM Revision 1.2  2010/06/28 20:15:10Z  jimmb
REM Fixed syntax to correctly add the Pentium and Pentium4 CPU to
REM the Intel abstraction.
REM Revision 1.1  2010/06/16 20:57:32Z  jimmb
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_CPUFAMILY=
if "%1" == "simnt"           set P_CPUFAMILY=simulator
if "%1" == "pentium"         set P_CPUFAMILY=x86
if "%1" == "pentium4"        set P_CPUFAMILY=x86

if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo CPUCHECK.BAT:  This ToolSet does not recognize the CPU type "%1"
echo.
echo The following CPU types are supported:
echo.
echo Simulator:               simnt
echo.
echo Note that the CPU type must be specified in all lower case.
echo.


:Fini
