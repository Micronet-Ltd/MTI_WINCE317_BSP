@echo off

echo Executing SerialTest_App Postlink.bat

echo copying SerialTest_App.exe
copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\SerialTest_App.*" "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"

echo copying SerialTest_App.exe
REM copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\WL1271_Manager_App.*" "%_FLATRELEASEDIR%"




