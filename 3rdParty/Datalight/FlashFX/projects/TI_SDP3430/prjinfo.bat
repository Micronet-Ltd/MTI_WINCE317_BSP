REM ---------- Product/Project Settings ----------

REM DLFFX_ROOT must already be set prior to calling this batch file.
REM
set P_ROOT=%DLFFX_ROOT%

set P_PROJDIR=%P_ROOT%\projects\%DLFFX_PROJECT%
set P_OS=wince
set P_OSROOT=
set P_OSVER=
set P_CPUFAMILY=arm
set P_CPUTYPE=armv4i
set P_TOOLSET=mswce
set P_TOOLROOT=%_WINCEROOT%

REM --------------- Build Settings ---------------

REM Define any product-wide C compiler flags to use
REM
set B_CFLAGS=

REM Define any extra libraries required for linking
REM
set B_DRIVERLIBS=

