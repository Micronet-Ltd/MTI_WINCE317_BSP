@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to define the build numbers that correspond to
REM various product release levels.  A '.' indicates that the latest build
REM should be used, otherwise a build number must be specified.
REM
REM This file is typically updated only by the product/engineering manager.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: prodrevs.bat $
REM Revision 1.1  2005/01/26 22:18:50Z  Pauli
REM Initial revision
REM Revision 1.1  2005/01/26 22:18:50Z  Garyp
REM Initial revision
REM Revision 1.1  2005/01/26 21:18:50Z  pauli
REM Initial revision
REM Revision 1.1  2005/01/26 20:18:50Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set DLALPHA=.
set DLBETA=.
set DLRELEASE=.

