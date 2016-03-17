@if "%ECHO%" == "" set ECHO=off
@echo %ECHO%

REM ----------------------------------------------------------------------
REM NOTE: This batch file should contain the bare minimum level of
REM       functionality necessary to transfer control to clean.bat.
REM ----------------------------------------------------------------------

if not exist prjinfo.bat goto NoProj

call prjinfo.bat

REM Make sure P_ROOT at least appears to be valid...
if not exist %P_ROOT%\product\*.* goto NoRoot

REM Use the DCL that is found in the product tree, if it exists
if exist %P_ROOT%\dcl\product\*.* set P_DCLROOT=%P_ROOT%\dcl

REM If it doesn't exist in the product, it must be specified elsewhere
if "%P_DCLROOT%" == "" goto NoDCL

if not exist %P_DCLROOT%\host\win32\bin\clean.bat goto NoProd

shift
call %P_DCLROOT%\host\win32\bin\clean.bat %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto Fini

:NoRoot
echo PRJCLEAN: Bad P_ROOT value
set B_ERROR=prjclean_BadRoot
goto Fini

:NoProj
echo PRJCLEAN: prjinfo.bat not found
set B_ERROR=prjclean_BadProject
goto Fini

:NoDCL
echo PRJCLEAN: P_DCLROOT is not set (P_ROOT is invalid or DCL can't be found)
set B_ERROR=prjclean_BadDclRoot
goto Fini

:NoProd
echo PRJCLEAN: "%P_DCLROOT%\host\win32\bin\clean.bat" not found
set B_ERROR=prjclean_BadProductTree
goto Fini


:Fini
REM Ensure that ERRORLEVEL=0 if we were successful, and non-zero otherwise
if "%B_ERROR%" == "" (exit /b 0) else (exit /b 1)


