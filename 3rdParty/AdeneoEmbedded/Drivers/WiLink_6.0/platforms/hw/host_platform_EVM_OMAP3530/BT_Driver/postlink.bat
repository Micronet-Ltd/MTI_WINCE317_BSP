@echo off

echo Executing BTHCI Postlink.bat

echo copying brf6300.dll
copy "%_WINCEROOT%\%_TARGETPLATROOT%\%_CPUINDPATH%\brf6300.*" "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"

echo copying brf6300.dll
REM copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\brf6300.*" "%_FLATRELEASEDIR%"




