REM ---------- Product/Project Settings ----------

pushd ..\..
set P_ROOT=%CD%
popd

set P_PROJDIR=%P_ROOT%\projects\Mistral_OMAP35x
set P_OS=wince
set P_OSROOT=
set P_OSVER=
set P_CPUFAMILY=arm
set P_CPUTYPE=armv4i
set P_TOOLSET=mswce
set P_TOOLROOT=%_WINCEROOT%

REM --------------- Build Settings ---------------

REM Define any product-wide C compiler flags to use
set B_CFLAGS=

REM Define any extra libraries required for linking
set B_DRIVERLIBS=

