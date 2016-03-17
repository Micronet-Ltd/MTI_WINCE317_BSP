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
REM Revision 1.2  2007/04/29 20:08:20Z  Garyp
REM Documentation updated.
REM Revision 1.1  2006/11/02 19:47:08Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM There are no convenient ways to automatically detect the Green Hills
REM tools installation location.

set P_TOOLROOT=

