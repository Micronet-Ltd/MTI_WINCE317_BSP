@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to validate the CPU type and set the P_CPUFAMILY
REM value for a ToolSet.  The first and only parameter is the CPU type.  On
REM exit, the P_CPUFAMILY value will be set to the CPU family name, or NULL
REM if the CPU type is not valid for the ToolSet.
REM
REM If the CPU type is not valid, the batch file should display a brief list
REM of supported CPU types.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cpucheck.bat $
REM Revision 1.4  2007/05/02 21:10:34Z  Garyp
REM Minor documentation and message cleanup.
REM Revision 1.3  2006/11/02 20:33:08Z  Garyp
REM Documentation updated.
REM Revision 1.2  2006/10/08 23:53:32Z  Garyp
REM First working version of the RVDS30 ToolSet.
REM Revision 1.1  2006/10/07 00:05:30Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM Since the ARM tools by definition only support ARM processors, this batch
REM file does not need any special knowledge to conclude that the P_CPUFAMILY
REM value should be "arm".
REM
REM Furthermore, since the "rvds30" ToolSet uses the B_CPUFLAGS to specify
REM which ARM variant CPU flags are required, we really don't need to worry
REM (at this level) what P_CPUTYPE value is specified.

set P_CPUFAMILY=arm

