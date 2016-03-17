@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file iterates through a list of directories (typically the
REM P_TREES directory list), and calls cleanhlp.bat for each tree.
REM
REM The main purpose for this batch file is to specifically handle cleaning
REM sub-product directories properly, as well as avoiding cleaning the PROJECTS
REM directory tree.
REM
REM This batch file is always called with a separate command shell, and
REM therefore does not need to clean up any environment variables.  There
REM is no mechanism for indicating an error.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cleandir.bat $
REM Revision 1.4  2011/01/13 18:35:02Z  garyp
REM Reverted part of the previous revision and added better documentation.
REM Revision 1.3  2011/01/12 19:57:33Z  garyp
REM Removed legacy DOS box support.
REM Revision 1.2  2006/07/31 16:32:14Z  Garyp
REM Updated to recognize when we are processing sub-product directories and
REM properly handle them according to  the clean rules for the sub-product.
REM Revision 1.1  2005/12/07 04:54:04Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/07 04:54:04Z  Garyp
REM Modified to support per-product exclude directories when cleaning.
REM Revision 1.1  2005/01/17 23:27:14Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

:NextDir
shift
if "%0" == "" goto Fini
if "%0" == "projects" goto NextDir

if not exist %0\product\prodinfo.bat goto NotASubProduct

REM We have found a sub-product.  We must query the prodinfo.bat to get the
REM list of directory trees in that product to clean.  We must ensure that
REM P_ROOT is set properly for the sub-product.
REM
REM Note also that we must spawn another command processor because this 
REM batch file is recursively calling itself, and we must preserve some
REM environment variables.

set OLDROOT=%P_ROOT%
set P_ROOT=%P_ROOT%\%0
cd %0
call product\prodinfo.bat /SUBPRODUCT
%COMSPEC% /c %P_DCLROOT%\host\win32\bin\cleandir.bat %P_SUBTREES%
cd ..
set P_ROOT=%OLDROOT%
goto NextDir

:NotASubProduct

cd %0
%P_DCLROOT%\host\win32\bin\recurse /n /r %COMSPEC% /c %P_DCLROOT%\host\win32\bin\cleanhlp.bat
cd ..
goto NextDir

:Fini
set OLDROOT=

