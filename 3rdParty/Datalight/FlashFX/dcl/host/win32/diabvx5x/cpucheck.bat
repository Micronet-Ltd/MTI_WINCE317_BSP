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
REM Revision 1.2  2007/05/02 21:10:43Z  Garyp
REM Minor documentation and message cleanup.
REM Revision 1.1  2005/04/24 16:25:18Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_CPUFAMILY=
if "%1" == "armarch4"   set P_CPUFAMILY=arm
if "%1" == "armarch4_t" set P_CPUFAMILY=arm
if "%1" == "armarch5"   set P_CPUFAMILY=arm
if "%1" == "armarch5_t" set P_CPUFAMILY=arm
if "%1" == "strongarm"  set P_CPUFAMILY=arm
if "%1" == "xscale"     set P_CPUFAMILY=arm
if "%1" == "mcf5200"    set P_CPUFAMILY=coldfire
if "%1" == "mcf5400"    set P_CPUFAMILY=coldfire
if "%1" == "sh7600"     set P_CPUFAMILY=sh
if "%1" == "sh7700"     set P_CPUFAMILY=sh
if "%1" == "sh7750"     set P_CPUFAMILY=sh
if "%1" == "mips32"     set P_CPUFAMILY=mips
if "%1" == "mips64"     set P_CPUFAMILY=mips
if "%1" == "pentium"    set P_CPUFAMILY=x86
if "%1" == "pentium2"   set P_CPUFAMILY=x86
if "%1" == "pentium3"   set P_CPUFAMILY=x86
if "%1" == "pentium4"   set P_CPUFAMILY=x86
if "%1" == "ppc403"     set P_CPUFAMILY=ppc
if "%1" == "ppc405"     set P_CPUFAMILY=ppc
if "%1" == "ppc440"     set P_CPUFAMILY=ppc
if "%1" == "ppc603"     set P_CPUFAMILY=ppc
if "%1" == "ppc604"     set P_CPUFAMILY=ppc
if "%1" == "ppc85xx"    set P_CPUFAMILY=ppc
if "%1" == "ppc860"     set P_CPUFAMILY=ppc
if "%1" == "simnt"      set P_CPUFAMILY=simulator
if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo cpucheck.bat:  This ToolSet does not recognize the CPU type "%1"
echo.
echo The following CPU types are supported:
echo.
echo ARM:                     armarch4 armarch4_t armarch5 armarch5_t
echo.
echo StrongARM, XScale:       strongarm xscale
echo.
echo Coldfire:                mcf5200 mcf5400
echo.
echo SuperH:                  sh7600 sh7700 sh7750
echo.
echo MIPS:                    mips32 mips64
echo.
echo Pentium:                 pentium pentium2 pentium3 pentium4
echo.
echo PowerPC:                 ppc403 ppc405 ppc440 ppc603 ppc604
echo                          ppc85xx ppc860
echo.
echo Simulator:               simnt
echo.
echo Note that the CPU type must be specified in all lower case.
echo.



:Fini
