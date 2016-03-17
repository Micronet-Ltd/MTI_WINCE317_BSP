REM Do "loader" last because it has subdirectories...

call %B_TRAVERSE% common clib debug fatfs perflog relfs servicefw testfw tests tools util ftpfs loader

