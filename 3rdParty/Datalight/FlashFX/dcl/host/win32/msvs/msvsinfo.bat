@echo off
REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to determine specific information about the
REM Microsoft Visual Studio development tools.
REM
REM B_MSVS_VER - Major Minor version of the compiler (60, 70, ...)
REM B_MSVS_DSC - Verbosed description of the compiler
REM B_MSVS_DIR - Install directory of the tools
REM B_MSVS_TGT - Target CPU of the compiler
REM
REM The toolset must be fully setup and in the path.
REM
REM On exit B_MSVS_VER will be the version of the compiler or null on
REM failure.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: msvsinfo.bat $
REM Revision 1.4  2011/01/12 01:47:36Z  garyp
REM Cleaned up the error messsages.
REM Revision 1.3  2010/12/24 00:58:19Z  qa
REM Updated to support Visual Studio 2010.
REM Revision 1.2  2008/07/23 21:33:30Z  keithg
REM Added support for VS 9.0 and broadened CPU support (x64)
REM Revision 1.1  2008/07/22 23:18:36Z  keithg
REM Initial revision
REM Revision 1.1  2008/07/22 23:18:36Z  keithg
REM Initial revision
REM -------------------------------------------------------------------------
@ECHO %ECHO%

REM Assume failure and clear the variables we are responsible for
REM
set B_MSVS_VER=
set B_MSVS_TGT=
set B_MSVS_DIR=
set B_MSVS_DSC=

set B_TEMP=cl_%RANDOM%.tmp
if exist %B_TEMP% del %B_TEMP%
cl 1>nul 2>%B_TEMP%

REM No file means no compiler
REM
if not exist %B_TEMP% goto BadTools
find "Microsoft (R)" %B_TEMP% >nul
if not "%ERRORLEVEL%" == "0" goto BadTools

REM -------------------------------------------------------------------------
REM Compiler version information, look for the X.YY.ZZZ.... in the signon
REM -------------------------------------------------------------------------

find "Compiler Version 12.00." %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_VER=60) && goto FoundCompiler

REM Check for MSVS 7.0 (Visual Studio .NET)
find "Compiler Version 13.00." %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_VER=70) && goto FoundCompiler

REM Check for MSVS 7.0 (Visual Studio .NET 2003)
find "Compiler Version 13.10." %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_VER=71) && goto FoundCompiler

REM Check for MSVS 8.0 (Visual C++ 2005)
find "Compiler Version 14.00." %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_VER=80) && goto FoundCompiler

REM Check for MSVS 9.0 (Visual Studio 2008)
find "Compiler Version 15.00." %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_VER=90) && goto FoundCompiler

REM Check for MSVS 10.0 (Visual Studio 2010)
find "Compiler Version 16.00." %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_VER=2010) && goto FoundCompiler

REM Did not find a supported version
goto BadVersion

:FoundCompiler


REM -------------------------------------------------------------------------
REM Compiler target CPU, look for "for XXX CPU"... in the signon
REM -------------------------------------------------------------------------

REM Check for the target build
find "for 80x86" %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_TGT=386) && goto FoundTarget

find "for x64" %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_TGT=x64) && goto FoundTarget

find "for ARM" %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_TGT=ARM) && goto FoundTarget

find "for MIPS R-Series" %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_TGT=MIPS) && goto FoundTarget

find "for Renesas SH" %B_TEMP% >nul
if "%ERRORLEVEL%" == "0" (set B_MSVS_TGT=SH) && goto FoundTarget

REM Did not find a suitable target CPU
goto BadCpu

:FoundTarget


REM -------------------------------------------------------------------------
REM Install directory and product name initialization...
REM -------------------------------------------------------------------------

if "%B_MSVS_VER%" == "60" (set B_MSVS_DSC=Microsoft Visual Studio 6.0) && set B_MSVS_DIR=%MSVCDIR%

if "%B_MSVS_VER%" == "70" (set B_MSVS_DSC=Microsoft Visual Studio .NET) && set B_MSVS_DIR=%VCINSTALLDIR%

if "%B_MSVS_VER%" == "71" (set B_MSVS_DSC=Microsoft Visual Studio .NET 2003) && set B_MSVS_DIR=%VCINSTALLDIR%

if "%B_MSVS_VER%" == "80" (set B_MSVS_DSC=Microsoft Visual Studio 2005) && set B_MSVS_DIR=%VCINSTALLDIR%

if "%B_MSVS_VER%" == "90" (set B_MSVS_DSC=Microsoft Visual Studio 2008) && set B_MSVS_DIR=%VCINSTALLDIR%

if "%B_MSVS_VER%" == "2010" (set B_MSVS_DSC=Microsoft Visual Studio 2010) && set B_MSVS_DIR=%VCINSTALLDIR%

set B_ERROR=
goto Fini

:BadVersion
set B_ERROR=MSVSINFO_BadVersion
goto Failed

:BadCpu
set B_ERROR=MSVSINFO_BadCpu
goto Failed

:BadTools
set B_ERROR=MSVSINFO_ToolsNotFound
goto Failed

:Failed
set B_MSVS_VER=
set B_MSVS_TGT=
set B_MSVS_DIR=
goto Fini


:Fini
@if exist %B_TEMP% del %B_TEMP%
set B_TEMP=


