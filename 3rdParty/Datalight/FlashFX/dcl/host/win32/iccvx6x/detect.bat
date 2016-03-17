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
REM Revision 1.1  2010/06/16 21:49:44Z  jimmb
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=

REM If the Compiler tools are being used with something other than
REM VxWorks, we don't know how to find the Tool Root.
if not "%1" == "vxworks" goto Fini

REM Fail if WIND_HOME is not initialized
if "%WIND_HOME%" == "" goto Fini

set P_TOOLROOT=%WIND_HOME%

REM See if the Intel and GNU tools are where we think they should be
if exist %P_TOOLROOT%\intel\20100316-vxworks-6.9\x86-win32\bin\ia32\*.exe goto Next
set P_TOOLROOT=
goto Fini

:Next
if exist %P_TOOLROOT%\gnu\4.3.3-vxworks-6.9\x86-win32\bin\*.exe goto Fini
set P_TOOLROOT=

:Fini

