@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM DETECT.BAT is used to try to automatically determine the ToolRoot
REM value if possible.  It is called by NEWPROJ.BAT.
REM
REM One parameter is supplied, which is the OS name.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: detect.bat $
REM Revision 1.1  2007/06/15 00:21:40Z  pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_TOOLROOT=

if exist C:\mgc\embedded\tools\gnu\arm\CSL-ARM-Toolchain\bin\arm-none-eabi-gcc.exe set P_TOOLROOT=C:\mgc\embedded\tools\gnu\arm\CSL-ARM-Toolchain
if exist C:\CSL-ARM-Toolchain\bin\arm-none-eabi-gcc.exe set P_TOOLROOT=C:\CSL-ARM-Toolchain
