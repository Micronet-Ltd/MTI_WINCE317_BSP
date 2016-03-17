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
REM Revision 1.2  2011/03/01 21:33:52Z  garyp
REM Fixed to not be C6000 specific.
REM Revision 1.1  2010/04/27 15:07:44Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%
  
set P_TOOLROOT=

if "%TI_DIR%" == "" goto Fini

REM See if the tools are where we think they should be
REM
if not exist %TI_DIR%\tools\compiler\. goto Fini

set P_TOOLROOT=%TI_DIR%\tools\compiler

:Fini

