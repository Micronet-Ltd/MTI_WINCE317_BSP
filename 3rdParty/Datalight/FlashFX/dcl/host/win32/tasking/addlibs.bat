@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM  !! This overrides the the default batch file located in ..\bin. !!
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
REM Up to 8 source libraries may be specified in addition to the output
REM library name.
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------
REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: addlibs.bat $
REM Revision 1.1  2005/01/09 22:58:24Z  Pauli
REM Initial revision
REM Revision 1.1  2005/01/09 22:58:24Z  Garyp
REM Initial revision
REM Revision 1.1  2005/01/09 21:58:24Z  pauli
REM Initial revision
REM Revision 1.1  2005/01/09 20:58:24Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %echo%

REM The default behavior is simply to assume that the tools in question
REM have the ability to add the contents of one library to another by
REM using the same command used to add OBJ modules to a library.
REM
REM However, not all tools support this, which is why a given ToolSet may
REM have its own version of ADDLIBS.BAT.
REM call %B_BUILDLIB% %1 %2 %3 %4 %5 %6 %7 %8 %9

if exist %B_RELDIR%\%P_PRODUCT%.lib goto success
goto fini

:success
REM Indicate success
set B_ERROR=
@echo %P_PRODUCT%.LIB Created Sucessfully
goto fini

:fini
