@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to attempt to determine the ToolRoot value if
REM possible.  It is called by newproj.bat, and may be called by the OS
REM specific initialization batch file.
REM
REM One command-line parameter is supplied, which is the OS name.
REM
REM On exit, the P_TOOLROOT variable will be set if the ToolRoot location
REM could be determined, or the variable will be cleared otherwise.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: detect.bat $
REM Revision 1.7  2007/04/29 20:08:18Z  Garyp
REM Documentation updated.
REM Revision 1.6  2007/03/14 01:30:19Z  Garyp
REM Updated to handle the MS Visual Studio 2003 (MSVS7) tools.
REM Revision 1.5  2007/03/10 20:29:23Z  Garyp
REM Fixed syntax errors and improved error handling.
REM Revision 1.4  2006/11/04 21:17:27Z  Garyp
REM Added support for MSVS8.
REM Revision 1.3  2006/09/21 21:35:49Z  Garyp
REM Updated to preserve the echo state.
REM Revision 1.2  2006/05/14 05:07:00Z  Keithg
REM Added quotes to support paths wiith space(s)
REM Revision 1.1  2005/01/09 22:58:18Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=

REM Try MSVS6/7 first
REM
if "%MSVCDir%" == "" goto TryVS8
set TOOLNAME=MS Visual Studio 6
if "%VCINSTALLDIR%" == "" goto UseMSVCDir

REM MSVS7 has both of the MSVCDir and VCINSTALLDIR variables set,
REM but MSVCDir has the value we want to use.
REM
set TOOLNAME=MS Visual Studio 2003

:UseMSVCDir
set P_TOOLROOT=%MSVCDir%
goto CheckBin

REM Try MSVS8 next
REM
:TryVS8
if "%VCINSTALLDIR%" == "" goto Fini
set TOOLNAME=MS Visual Studio 2005
set P_TOOLROOT=%VCINSTALLDIR%

:CheckBin
if exist "%P_TOOLROOT%\bin\*.exe" goto DisplayResults
set P_TOOLROOT=
goto Fini

:DisplayResults
echo.
echo Detected %TOOLNAME% tools
echo.
goto Fini

:Failed
echo.
echo detect.bat: Unable to automatically determine the MS Visual Studio tool root.
echo.
goto Fini

:Fini
set TOOLNAME=

