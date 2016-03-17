REM ---------- Product/Project Settings ----------

set P_ROOT=%_WINCEROOT%\3rdParty\Datalight\flashfx
set P_PROJDIR=%P_ROOT%\projects\micbl
set P_OS=wincebl
set P_OSROOT=
set P_OSVER=
set P_CPUFAMILY=arm
set P_CPUTYPE=armv4i
set P_TOOLSET=mswce
set P_TOOLROOT=%_WINCEROOT%
 set P_PRIVATE=1

REM --------------- Build Settings ---------------

REM Define any product-wide C compiler flags to use
REM
set B_CFLAGS=/DTHUMBSUPPORT /QRthumb /QRinterwork-return /O1

REM Define any extra libraries required for linking
REM
set B_DRIVERLIBS=

