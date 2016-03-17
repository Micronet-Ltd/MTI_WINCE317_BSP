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
REM Revision 1.3  2007/04/29 20:08:18Z  Garyp
REM Documentation updated.
REM Revision 1.2  2006/09/21 21:35:49Z  Garyp
REM Updated to preserve the echo state.
REM Revision 1.1  2005/01/09 22:58:18Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=%_WINCEROOT%

if "%P_TOOLROOT%" == "" goto Fini

if not exist %P_TOOLROOT%\sdk\bin\i386\*.exe set P_TOOLROOT=

:Fini

