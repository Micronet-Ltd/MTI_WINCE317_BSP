REM If the target is WINCE or WIN32, traverse into the "win" directory to
REM to build user mode Windows stuff which can be shared between CE and
REM regular Windows.

set TMPWINUSER=

REM SKIP THIS FOR NOW...
if "%P_OS%" == "wince" set TMPWINUSER=win

if "%P_OS%" == "win32" set TMPWINUSER=win

call %B_TRAVERSE% os %TMPWINUSER% %P_OS%

set TMPWINUSER=

