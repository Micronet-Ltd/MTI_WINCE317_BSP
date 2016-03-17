@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This helper batch file is used to delete directory trees.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: treedel.bat $
REM Revision 1.2  2006/09/22 21:58:05Z  Garyp
REM Made the comments less intrusive.
REM Revision 1.1  2005/01/09 23:35:48Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "" goto syntax
if "%1" == "?" goto syntax
if "%1" == "/?" goto syntax

if "%OS%" == "Windows_NT" goto NTHACK

if exist %1\nul deltree /Y %1
goto Fini


:NTHACK

@REM Since we may be inside a hokey NT emulation of DOS (we might have
@REM been spawned by RECURSE), we can't even do a simple "if exist" on a
@REM directory like that done above.  (Nor will the /Q and /S parameters
@REM to RD be recognized.)  Instead we need to start another command
@REM processor (CMD, not COMMAND) to get into NT mode, and then issue
@REM the command.

%systemroot%\system32\cmd /c if exist %1\nul rd /q /s %1
goto Fini

:syntax
echo TREEDEL.BAT is intended as a helper batch file for use primarily
echo by CLEAN.BAT.  It tests for the existence of the specified
echo directory, and if found, removes it.

:Fini
