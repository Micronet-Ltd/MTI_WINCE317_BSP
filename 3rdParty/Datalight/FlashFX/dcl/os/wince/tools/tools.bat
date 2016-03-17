set WINTOOLS=

if exist "%B_CESDKDIR%\commdlg.lib" set WINTOOLS=windlshell

call %B_TRAVERSE% os\%P_OS%\tools dclshell dlfilespy dlpart dlshellext loaddrv %WINTOOLS%

set WINTOOLS=

