@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This is a helper batch file used when cleaning the project.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cleanhlp.bat $
REM Revision 1.9  2011/03/06 23:25:54Z  garyp
REM Updated to invoke the PROJCLEAN target within any project specific
REM make files.
REM Revision 1.8  2011/01/13 18:33:51Z  garyp
REM Reverted part of the previous revision and added better documentation.
REM Revision 1.7  2011/01/13 02:27:32Z  garyp
REM Removed legacy DOS box support.
REM Revision 1.6  2009/02/08 00:16:47Z  garyp
REM Merged from the v4.0 branch.  Updated to skip cleaning directories which
REM have a .MAK file which contains the keyword "SKIPCLEAN".
REM Revision 1.5  2008/01/31 22:50:29Z  Garyp
REM Comments updated.
REM Revision 1.4  2006/09/21 21:53:21Z  Garyp
REM Hid some comments to make debug output less verbose.
REM Revision 1.3  2006/08/23 00:00:07Z  Garyp
REM Updated to delete the CSK tree, and to no longer delete the obfus tree.
REM Revision 1.2  2006/07/31 16:32:13Z  Garyp
REM Minor cleanup -- no functional changes.
REM Revision 1.1  2005/01/11 21:20:18Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM If there is any makefile in the directory which contains the
REM word "SKIPCLEAN", then skip this directory.
REM
REM Note that we go out of our way to use the Windows version of
REM "find" because some environments, which shall not be named,
REM have their own version of "find" but with different command
REM syntax.  Not cool!
REM
for %%a in (*.mak) do (
  %windir%\system32\find /I "SKIPCLEAN" %%a >nul
  if not errorlevel 1 goto Fini
)

if not "%1" == "/PROJECT" goto NormalClean

@REM When this batch file is called with the /PROJECT option it means
@REM that we are recursing through the entire Projects tree and that
@REM the project information from PRJINFO.BAT has not yet been set.
@REM In this case, call PRJINFO so that we can determine the name
@REM of the CPU tree that we must delete.

if not exist prjinfo.bat goto Fini

set OLD_DCLROOT=%P_DCLROOT%
set OLD_ROOT=%P_ROOT%

call prjinfo.bat

set P_DCLROOT=%OLD_DCLROOT%
set P_ROOT=%OLD_ROOT%

set OLD_DCLROOT=
set OLD_ROOT=

if not "%P_CPUTYPE%" == "" if exist %P_CPUTYPE%\. rd /s/q %P_CPUTYPE%

@REM Delete any CSK and RELEASE trees created by the obfuscation process
@REM (FYI, this seems like it will fail since clean.bat specifically
@REM  excludes the projects\sdk directory...)
@REM
if exist csk\. rd /s/q csk
if exist release\. rd /s/q release

if not "%CLEANALL%" == "0" (
  @REM Invoke MAKE with the PROJCLEAN target.
  @REM
  for %%A in (???proj.mak) do call %B_MAKE% -f%%A PROJCLEAN
)

REM Now that we've gotten rid of that, just fall through the
REM remainder of the clean process.


:NormalClean
call %P_ROOT%\product\cleanext.bat

if exist obj\. rd /s/q obj


:Fini

