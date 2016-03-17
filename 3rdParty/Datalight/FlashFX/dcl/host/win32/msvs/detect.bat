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
REM Revision 1.6  2009/05/27 23:51:40Z  garyp
REM Fixed a syntax error which would abort the batch file if no prior MSVS
REM environment vars had been set up.  Removed use of XP command
REM extensions.  
REM Revision 1.5  2009/01/09 01:42:32Z  brandont
REM Added quotes for a P_TOOLROOT reference in case it contains a
REM long file name with spaces.
REM Revision 1.4  2008/07/22 20:58:05Z  keithg
REM Added support for VS 9.0 and broadened CPU support (x64)
REM Revision 1.3  2007/07/09 01:33:28Z  Garyp
REM Minor fix to display the failure message -- no functional changes.
REM Revision 1.2  2007/04/29 20:08:18Z  Garyp
REM Documentation updated.
REM Revision 1.1  2007/03/20 20:32:32Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set B_TEMP=detect.tmp

if not "%MSVCDIR%" == "" set P_TOOLROOT=%MSVCDIR%
if not "%VCINSTALLDIR%" == "" set P_TOOLROOT=%VCINSTALLDIR%
if "%P_TOOLROOT%" == "" goto Failed

if exist %B_TEMP% del %B_TEMP%

REM Look for the MSVC run time library, it is named MSVCR??.DLL
REM

dir /s/b "%P_TOOLROOT%\msvcr*.dll" >%B_TEMP%
find /I "msvcr" %B_TEMP% >nul
if errorlevel 1 goto Failed


echo.
echo Detected Microsoft tools at "%P_TOOLROOT%"
echo.
goto Fini

:Failed
echo.
echo detect.bat: Unable to automatically determine the MS Visual Studio tool root.
echo             Be sure the ToolSet is properly initialized.
echo.
goto Fini

:Fini
if exist %B_TEMP% del %B_TEMP%
set B_TEMP=

