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
REM Revision 1.10  2010/06/09 15:10:45Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.9
REM Revision 1.9  2009/08/20 21:43:37Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.8
REM Revision 1.8  2008/08/25 19:42:14Z  jimmb
REM Updated for VxWorks 6.7
REM Revision 1.7  2007/08/29 22:29:52Z  jimmb
REM Udated the bat files to support the new VxWorks 6.6 and 
REM the GNU compiler 4.1.2
REM Revision 1.6  2007/05/15 02:57:19Z  garyp
REM Fixed to work properly with VxWorks 6.x, which unlike v5.x, needs to use
REM WIND_HOME rather than WIND_BASE.
REM Revision 1.5  2007/04/29 20:08:19Z  Garyp
REM Documentation updated.
REM Revision 1.4  2007/03/11 21:04:38Z  Garyp
REM Updated to handle VxWorks 6.5.
REM Revision 1.3  2006/08/28 01:55:27Z  Garyp
REM Added VxWorks 6.4 support.
REM Revision 1.2  2006/08/01 19:39:18Z  Garyp
REM Updated to recognize VxWorks 6.2 and 6.3 installations.
REM Revision 1.1  2005/04/24 16:19:52Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=

REM If the GNU tools are being used with something other than
REM VxWorks, we don't know how to find the Tool Root.
if not "%1" == "vxworks" goto Fini

REM Fail if WIND_HOME is not initialized
if "%WIND_HOME%" == "" goto Fini

set P_TOOLROOT=%WIND_HOME%

REM See if the GNU tools are where we think they should be
if exist %P_TOOLROOT%\gnu\3.3.2-vxworks60\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\3.3.2-vxworks61\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\3.3.2-vxworks-6.2\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\3.4.4-vxworks-6.3\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\3.4.4-vxworks-6.4\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\3.4.4-vxworks-6.5\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\4.1.2-vxworks-6.6\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\4.1.2-vxworks-6.7\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\4.1.2-vxworks-6.8\x86-win32\bin\*.exe goto Fini
if exist %P_TOOLROOT%\gnu\4.3.3-vxworks-6.9\x86-win32\bin\*.exe goto Fini
set P_TOOLROOT=

:Fini

