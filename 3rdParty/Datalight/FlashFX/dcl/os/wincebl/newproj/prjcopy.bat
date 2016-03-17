@if "%ECHO%" == "" set ECHO=off
@echo %ECHO%

REM ----------------------------------------------------------------------
REM NOTE: This batch file should contain the bare minimum level
REM       of functionality necessary to transfer control to the
REM	  copybuild.bat located in the MSWCE ToolSet directory.
REM ----------------------------------------------------------------------

if "%P_ROOT%" == "" goto BadRoot

shift

REM Note that we are transferring control, not "calling"...
REM
if exist %P_ROOT%\dcl\host\win32\mswce\copybuild.bat %P_ROOT%\dcl\host\win32\mswce\copybuild.bat %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
if exist %P_ROOT%\host\win32\mswce\copybuild.bat     %P_ROOT%\host\win32\mswce\copybuild.bat     %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

:BadTree
echo PRJCOPY: Can't find the master copybuild.bat under "%P_ROOT%"
set B_ERROR=prjcopy_BadProduct
exit /b 1

:BadRoot
echo PRJCOPY: P_ROOT is not set
set B_ERROR=prjcopy_BadRoot
exit /b 1



