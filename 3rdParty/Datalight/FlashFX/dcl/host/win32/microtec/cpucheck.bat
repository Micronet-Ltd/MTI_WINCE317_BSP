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
REM Revision 1.2  2007/05/02 21:10:39Z  Garyp
REM Minor documentation and message cleanup.
REM Revision 1.1  2006/12/07 17:48:02Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM This implementation of the Microtec ToolSet only support PowerPC targets
REM at this time, and therefore does not need any special knowledge to conclude
REM that the P_CPUFAMILY value should be "ppc".
REM
REM Furthermore, since this ToolSet uses the B_CPUFLAGS to specify which PPC
REM variant CPU flags are required, we really don't need to worry (at this
REM level) what P_CPUTYPE value is specified.
REM
REM Note that thee P_CPUFAMILY value must be lower case.

set P_CPUFAMILY=ppc

