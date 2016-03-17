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
REM Revision 1.2  2007/04/29 20:08:21Z  Garyp
REM Documentation updated.
REM Revision 1.1  2005/04/24 16:19:52Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=

REM If the DIAB tools are being used with something other than
REM VxWorks, we don't know how to find the Tool Root.
if not "%1" == "vxworks" goto Fini

REM Fail if WIND_BASE is not initialized
if "%WIND_BASE%" == "" goto Fini

set P_TOOLROOT=%WIND_BASE%

REM See if the DIAB tools are where we think they should be
if exist %P_TOOLROOT%\host\diab\win32\bin\dcc.exe goto Fini
set P_TOOLROOT=

:Fini

