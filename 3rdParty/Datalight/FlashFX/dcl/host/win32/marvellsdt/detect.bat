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
REM Revision 1.1  2008/07/26 19:14:52Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=

if "%MARVELL_SDT_HOME%" == "" goto Failed

if not exist "%MARVELL_SDT_HOME%\marvellpxa\bin\ccxsc.exe" goto Failed

set P_TOOLROOT=%MARVELL_SDT_HOME%\marvellpxa

:DisplayResults
echo.
echo Detected Marvell SDT installation root
echo.
goto Fini

:Failed
echo.
echo detect.bat: Unable to automatically determine the Marvell SDT root.
echo.
goto Fini

:Fini

