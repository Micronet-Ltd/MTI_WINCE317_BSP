@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This is a helper batch file used when cleaning the project.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cleanext.bat $
REM Revision 1.2  2007/05/13 17:48:47Z  garyp
REM Modified to use the default implementation of this functionality in DCL.
REM Revision 1.1  2004/07/07 02:13:32Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM Just delete the default set of stuff that is handled by
REM the cleanext.bat in DCL.

call %P_DCLROOT%\product\cleanext.bat

REM Delete any extra stuff which is product specific here...


