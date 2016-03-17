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
REM Revision 1.3  2011/03/01 21:33:52Z  garyp
REM Changed the armv5tej CPU "family" name to ARM.
REM Revision 1.2  2010/08/11 20:11:51Z  johnb
REM Added ARM9 support
REM Revision 1.1  2010/04/27 15:07:44Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_CPUFAMILY=
if "%1" == "c6748"               set P_CPUFAMILY=c6000
if "%1" == "arm926ejs"           set P_CPUFAMILY=arm

if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo CPUCHECK.BAT:  This ToolSet does not recognize the CPU type "%1"
echo.
echo The following CPU types are supported:
echo.
echo c6000:                   c6748
echo.
echo arm:                     arm926ejs
echo.
echo Note that the CPU type must be specified in all lower case.
echo.

:Fini

