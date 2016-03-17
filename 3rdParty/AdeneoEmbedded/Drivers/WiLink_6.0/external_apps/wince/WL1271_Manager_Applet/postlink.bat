@echo on

echo Executing WL1271_Manager_Applet Postlink.bat

echo copying WL1271Manager.cpl
copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\WL1271Manager.*" "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"

echo copying WL1271Manager.cpl
REM copy "%_WINCEROOT%\3rdParty\AdeneoEmbedded\Drivers\WiLink_6.0\Target\%WINCEDEBUG%\WL1271Manager.*" "%_FLATRELEASEDIR%"




