setlocal
pushd
cd /d C:\WINCE600\3rdparty\datalight\flashfx\projects
build
if exist build.log type build.log >> %_WINCEROOT%\build.log
if exist build.wrn type build.wrn >> %_WINCEROOT%\build.wrn
if exist build.err type build.err >> %_WINCEROOT%\build.err
cd /d C:\WINCE600\3rdparty\adeneoembedded\drivers\wilink_6.0
build
if exist build.log type build.log >> %_WINCEROOT%\build.log
if exist build.wrn type build.wrn >> %_WINCEROOT%\build.wrn
if exist build.err type build.err >> %_WINCEROOT%\build.err
popd
endlocal
