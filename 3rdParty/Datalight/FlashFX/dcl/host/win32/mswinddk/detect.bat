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
REM Revision 1.2  2007/04/29 20:08:17Z  Garyp
REM Documentation updated.
REM Revision 1.1  2005/12/01 02:18:54Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 02:18:54Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.2  2005/11/09 17:04:10Z  Pauli
REM Updated to use B_PAUSE so as to work with the "nopause" option.
REM Revision 1.1  2005/03/31 22:05:16Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

echo.
echo The ToolRoot cannot be automatically determined.
echo.
echo If you are using the NTDDK in combination with the MS Visual C
echo tools, use the MSVCDIR environment variable as the ToolRoot.
echo (Run VCVARS32.BAT to get this set up.)
echo.
echo If you are using the Windows IFS Kit, use root of the IFS kit
echo installation, such as "c:\winddk\3790" as the ToolRoot.
echo.
%B_PAUSE%

set P_TOOLROOT=

:Fini

