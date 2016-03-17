@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This is a helper batch file used when cleaning the project.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cleanext.bat $
REM Revision 1.8  2011/10/10 00:50:00Z  garyp
REM Updated to clean .d and .o files.
REM Revision 1.7  2011/03/06 22:43:40Z  garyp
REM Updated to delete SLN and SUO files.
REM Revision 1.6  2009/02/08 01:59:52Z  garyp
REM Merged from the v4.0 branch.  Fixed to work properly in development 
REM environments which have their own private (and incompatible) version
REM of FIND.EXE.  Updated to clean files named SOURCES, DIRS, and MAKEFILE,
REM only if they contain the text "auto-generated".
REM Revision 1.5  2007/10/06 00:19:59Z  Garyp
REM Reversed the polarity on the "echo is always off" message.
REM Revision 1.4  2007/09/28 17:29:51Z  Garyp
REM Minor logic fix to on display the warning message if echo is ON.
REM Revision 1.3  2007/09/28 17:22:01Z  Garyp
REM Modified so echo is always OFF for this batch file.
REM Revision 1.2  2007/05/13 17:48:45Z  garyp
REM Updated to clean a few additional file types.
REM Revision 1.1  2005/01/07 04:22:58Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

if "%ECHO%" == "on" @echo (ECHO is always OFF in cleanext.bat)

if exist       *.a   del       *.a
if exist       *.abm del       *.abm
if exist       *.bak del       *.bak
if exist       *.cod del       *.cod
if exist       *.cpl del       *.cpl
if exist       *.d   del       *.d  
if exist       *.dbo del       *.dbo
if exist       *.dbg del       *.dbg
if exist       *.dll del       *.dll
if exist       *.err del       *.err
if exist       *.exe del       *.exe
if exist       *.exp del       *.exp
if exist       *.idb del       *.idb
if exist       *.ilk del       *.ilk
if exist       *.img del       *.img
if exist       *.lib del       *.lib
if exist       *.lnk del       *.lnk
if exist       *.log del       *.log
if exist       *.lst del       *.lst
if exist       *.map del       *.map
if exist       *.o   del       *.o  
if exist       *.obd del       *.obd
if exist       *.obj del       *.obj
if exist       *.obr del       *.obr
if exist       *.od  del       *.od
if exist       *.oj  del       *.oj
if exist       *.or  del       *.or
if exist       *.pdb del       *.pdb
if exist       *.rel del       *.rel
if exist       *.res del       *.res
if exist       *.rsp del       *.rsp
if exist       *.sln del       *.sln
if exist       *.src del       *.src
if exist       *.suo del       *.suo
if exist       *.sys del       *.sys
if exist       *.tmp del       *.tmp
if exist sources.gen del sources.gen

REM Delete SOURCES, DIRS, and MAKEFILE only if they contain the
REM text "auto-generated".
REM
REM Note that we go out of our way to use the Windows version of
REM "find" because some environments, which shall not be named,
REM have their own version of "find" but with different command
REM syntax.  Not cool!

if exist sources (
  %windir%\system32\find /I "auto-generated" sources >nul
  if not errorlevel 1 del sources
)

if exist dirs (
  %windir%\system32\find /I "auto-generated" dirs >nul
  if not errorlevel 1 del dirs
)

if exist makefile (
  %windir%\system32\find /I "auto-generated" makefile >nul
  if not errorlevel 1 del makefile
)

@echo %ECHO%


