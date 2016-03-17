echo on
pushd beagleboard
call prjbuild.bat /nopause %1
popd
rem pushd micbl
rem call buildbl.bat %1
rem popd
