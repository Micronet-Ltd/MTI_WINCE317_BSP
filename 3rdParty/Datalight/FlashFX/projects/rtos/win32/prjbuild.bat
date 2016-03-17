@if "%ECHO%" == "" set ECHO=off
@echo %ECHO%

REM ----------------------------------------------------------------------
REM NOTE: This batch file should contain the bare minimum level of
REM       functionality necessary to call to makeall.bat.
REM ----------------------------------------------------------------------

if not exist prjinfo.bat goto NoProj

call prjinfo.bat

REM Make sure P_ROOT at least appears to be valid...
if not exist %P_ROOT%\product\*.* goto NoRoot

REM Use the DCL that is found in the product tree, if it exists
if exist %P_ROOT%\include\dlver.h ((set P_DCLROOT=%P_ROOT%) && (goto DoBuild))
if exist %P_ROOT%\dcl\product\*.* ((set P_DCLROOT=%P_ROOT%\dcl) && (goto DoBuild))

REM If it doesn't exist in the product, it must be specified elsewhere
if "%P_DCLROOT%" == "" goto NoDCL

:DoBuild
if not exist %P_DCLROOT%\host\win32\bin\makeall.bat goto NoProd

shift
call %P_DCLROOT%\host\win32\bin\makeall.bat %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto Fini

:NoRoot
echo PRJBUILD: Bad P_ROOT value
set B_ERROR=prjbuild_BadRoot
goto Fini

:NoProj
echo PRJBUILD: prjinfo.bat not found
set B_ERROR=prjbuild_BadProject
goto Fini

:NoDCL
echo PRJBUILD: P_DCLROOT is not set (P_ROOT is invalid or DCL can't be found)
set B_ERROR=prjbuild_BadDclRoot
goto Fini

:NoProd
echo PRJBUILD: "%P_DCLROOT%\host\win32\bin\makeall.bat" not found
set B_ERROR=prjbuild_BadProductTree
goto Fini

:Fini
REM Ensure that ERRORLEVEL=0 if we were successful, and non-zero otherwise
if "%B_ERROR%" == "" (exit /b 0) else (exit /b 1)


