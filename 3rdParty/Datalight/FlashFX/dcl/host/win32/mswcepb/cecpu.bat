@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch is used to translate the CE specific CPU names into CPU
REM family and CPU type values that are recognized by the generalized
REM build process (returned in the CPUFAM and CPUTYP environment
REM variables).
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cecpu.bat $
REM Revision 1.3  2009/02/08 00:21:33Z  garyp
REM Merged from the v4.0 branch.  Fixed the "ARMV4I" CPU type to <not> be
REM converted to "xscale".  Removed CE3 support.
REM Revision 1.2  2008/07/23 05:59:43Z  keithg
REM Clarified comments with professional customer facing terms.
REM Revision 1.1  2005/05/17 22:35:00Z  Pauli
REM Initial revision
REM Revision 1.2  2005/05/17 23:35:00Z  garyp
REM Added the missing MIPSII case translation, and removed the PPC entry.
REM Revision 1.1  2005/01/09 20:58:22Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set CPUFAM=
set CPUTYP=

set CPUFAM=%_TGTCPUFAMILY%
set CPUTYP=%_TGTCPU%

if "%CPUFAM%" == "x86"    set CPUFAM=x86
if "%CPUFAM%" == "SH"     set CPUFAM=sh
if "%CPUFAM%" == "ARM"    set CPUFAM=arm
if "%CPUFAM%" == "MIPS"   set CPUFAM=mips

if "%CPUTYP%" == "SA1100" set CPUTYP=armv4
if "%CPUTYP%" == "ARMV4"  set CPUTYP=armv4
if "%CPUTYP%" == "ARMV4I" set CPUTYP=armv4i
if "%CPUTYP%" == "MIPSII" set CPUTYP=mipsii
if "%CPUTYP%" == "SH3"    set CPUTYP=sh3
if "%CPUTYP%" == "SH4"    set CPUTYP=sh4


REM Any CE x86 build can use our 386 CPU tree
if "%CPUTYP%" == "X86"    set CPUTYP=386
if "%CPUTYP%" == "x86"    set CPUTYP=386
if "%CPUTYP%" == "i486"   set CPUTYP=386


:Fini

