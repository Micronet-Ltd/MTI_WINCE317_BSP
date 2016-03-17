@echo %echo%

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to validate the CPU type and set the P_CPUFAMILY
REM value for each ToolSet.  The first parameter will be the CPU type.  On
REM exit, the P_CPUFAMILY value must be set to the CPU family name, or NULL
REM if the CPU type is not valid for the ToolSet.
REM
REM If the CPU type is not valid, the batch file should display a brief list
REM of valid CPU types.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cpucheck.bat $
REM Revision 1.1  2007/06/15 00:09:46Z  pauli
REM Initial revision
REM -------------------------------------------------------------------------

set P_CPUFAMILY=
if "%1" == "arm9" set P_CPUFAMILY=arm
if "%1" == "arm11" set P_CPUFAMILY=arm
if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo cpucheck.bat:  This ToolSet does not recognize the CPU type "%1"
echo.
echo The following CPU types are supported:  "arm9" "arm11"
echo.
%B_PAUSE%


:Fini
