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
REM Revision 1.5  2011/03/12 22:38:39Z  garyp
REM Fixed to make a short name before trying to use the variable.
REM Revision 1.4  2010/09/16 13:30:44Z  jimmb
REM Added extended command extention call to make sure the toolroot is using
REM short file names.
REM Revision 1.3  2010/09/13 15:00:30Z  jimmb
REM Updated the ToolSet to support RVDS versions 4.0 and 4.1.
REM Revision 1.2  2007/04/29 20:08:17Z  Garyp
REM Documentation updated.
REM Revision 1.1  2006/10/07 00:04:58Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set ARMVER= 
set ARMBUILD= 

call :MakeShortName P_TOOLROOT "%ARMROOT%"

if not exist %P_TOOLROOT%\rvct\programs\3.0\586\win_32-pentium\armcc.exe goto arm40
set ARMVER=3.0
set ARMBUILD=586
goto Fini

:arm40
if not exist %P_TOOLROOT%\rvct\programs\4.0\771\win_32-pentium\armcc.exe goto arm41
set ARMVER=4.0
set ARMBUILD=771
goto Fini

:arm41
if not exist %P_TOOLROOT%\rvct\programs\4.1\462\win_32-pentium\armcc.exe goto NotFound
set ARMVER=4.1
set ARMBUILD=462
goto Fini

:NotFound
set P_ROOTROOT=
goto Fini

:MakeShortName
set %1=%~s2
goto :EOF

:Fini

