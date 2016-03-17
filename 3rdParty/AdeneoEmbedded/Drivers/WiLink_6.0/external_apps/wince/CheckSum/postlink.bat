@echo off

echo Executing WL1271_Manager_App Postlink.bat

echo copying Checksum.exe
copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\Checksum.*" "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"

echo copying Checksum.exe
REM copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\Checksum.*" "%_FLATRELEASEDIR%"




