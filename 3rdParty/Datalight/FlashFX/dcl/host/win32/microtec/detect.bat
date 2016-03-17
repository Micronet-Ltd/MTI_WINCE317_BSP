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
REM Revision 1.2  2007/04/29 20:08:19Z  Garyp
REM Documentation updated.
REM Revision 1.1  2006/12/06 23:55:42Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM No convenient way to accomplish this...

set P_TOOLROOT=


