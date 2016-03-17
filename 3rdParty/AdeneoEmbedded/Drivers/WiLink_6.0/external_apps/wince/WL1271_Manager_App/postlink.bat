@echo on

echo Executing WL1271_Manager_App Postlink.bat

echo copying WL1271_Manager_App.exe
copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\WL1271_Manager_App.*" "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"

echo copying WL1271_Manager_App.exe
REM copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\WL1271_Manager_App.*" "%_FLATRELEASEDIR%"




