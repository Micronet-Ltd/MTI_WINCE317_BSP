set WINTESTS=

if exist "%B_CESDKDIR%\commdlg.lib" set WINTESTS=winfsiotest

call %B_TRAVERSE% os\%P_OS%\tests dcltest deviotest fsiotest fsstresstest %WINTESTS%

set WINTESTS=

