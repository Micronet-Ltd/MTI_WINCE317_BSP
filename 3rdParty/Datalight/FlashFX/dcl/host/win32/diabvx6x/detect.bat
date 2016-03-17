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
REM Revision 1.11  2012/04/04 19:37:25Z  jimmb
REM Update the diab compiler version number to support VxWorks 6.9.2
REM Revision 1.10  2010/06/09 16:44:28Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.9
REM Revision 1.9  2009/08/20 21:38:15Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.8
REM Revision 1.8  2008/08/25 19:41:12Z  jimmb
REM Updated for VxWorks 6.7
REM Revision 1.7  2007/09/05 20:44:13Z  jimmb
REM UPdated the files to support the Windriver compiler using
REM version 6.6 
REM Revision 1.6  2007/05/15 02:57:18Z  garyp
REM Fixed to work properly with VxWorks 6.x, which unlike v5.x, needs to use
REM WIND_HOME rather than WIND_BASE.
REM Revision 1.5  2007/04/29 20:08:21Z  Garyp
REM Documentation updated.
REM Revision 1.4  2007/03/11 21:04:37Z  Garyp
REM Updated to handle VxWorks 6.5.
REM Revision 1.3  2006/08/15 17:32:09Z  Garyp
REM Added VxWorks 6.4 support.
REM Revision 1.2  2006/08/01 19:39:19Z  Garyp
REM Updated to recognize VxWorks 6.2 and 6.3 installations.
REM Revision 1.1  2005/04/24 16:19:52Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=

REM If the DIAB tools are being used with something other than
REM VxWorks, we don't know how to find the Tool Root.
if not "%1" == "vxworks" goto Fini

REM Fail if WIND_HOME is not initialized
if "%WIND_HOME%" == "" goto Fini

set P_TOOLROOT=%WIND_HOME%

REM See if the DIAB tools are where we think they should be
if exist %P_TOOLROOT%\diab\5.2.2.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.2.3.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.3.1.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.4.0.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.5.0.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.5.1.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.6.0.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.7.0.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.8.0.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.9.0.0\win32\bin\dcc.exe goto Fini
if exist %P_TOOLROOT%\diab\5.9.1.0\win32\bin\dcc.exe goto Fini
set P_TOOLROOT=

:Fini

