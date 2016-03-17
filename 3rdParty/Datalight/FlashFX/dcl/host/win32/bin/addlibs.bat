@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to add the contents of one library to another.
REM It is used internally by the product build process and is not run
REM directly.
REM
REM On entry, %1 will be the output library name, and params %2+ will
REM be the names of the other libraries whose contents must be added to
REM the output library.  Both the output and source libraries MUST be in
REM the current directory -- the specifications may not include a path.
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: addlibs.bat $
REM Revision 1.2  2009/02/08 00:14:57Z  garyp
REM Merged from the v4.0 branch.  Updated to allow more than 8 source libraries
REM to be specified.
REM Revision 1.1  2005/12/01 01:54:12Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 01:54:12Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.2  2005/11/09 17:04:10Z  Pauli
REM Updated to use B_PAUSE so as to work with the "nopause" option.
REM Revision 1.1  2005/01/09 20:58:25Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM The default behavior is simply to assume that the tools in question
REM have the ability to add the contents of one library to another by
REM using the same command used to add OBJ modules to a library.
REM
REM However, not all tools support this, which is why a given ToolSet may
REM have its own version of ADDLIBS.BAT.

set ADDLIBSNAME=%1
shift

:NextBatch
call %B_BUILDLIB% %ADDLIBSNAME% %1 %2 %3 %4 %5 %6 %7 %8 %9
if not "%B_ERROR%" == "" goto Fini

shift && shift && shift && shift && shift && shift && shift && shift && shift
if not "%1" == "" goto NextBatch

:Fini
set ADDLIBSNAME=


