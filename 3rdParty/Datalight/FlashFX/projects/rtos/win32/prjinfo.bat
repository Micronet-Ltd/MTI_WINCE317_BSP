REM ---------- Product/Project Settings ----------

pushd ..\..\..
set P_ROOT=%CD%
popd

set P_PROJDIR=%P_ROOT%\projects\rtos\win32
set P_OS=rtos
set P_OSROOT=N/A
set P_OSVER=N/A
set P_CPUFAMILY=x86
set P_CPUTYPE=386
set P_TOOLSET=msvs
set P_TOOLROOT=%VCINSTALLDIR%
if "%P_TOOLROOT%" == "" set P_TOOLROOT=%MSVCDir%
set P_PRIVATE=

REM --------------- Build Settings ---------------

REM Define any product-wide C compiler flags to use
set B_CFLAGS=


