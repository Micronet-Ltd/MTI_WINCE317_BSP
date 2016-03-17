REM If the assembler name is not defined, skip the CPU tree
if "%B_ASM%" == "" goto Fini

REM If there is no tree for the CPU family, skip
if not exist %P_CPUFAMILY%\*.* goto Fini

call %B_TRAVERSE% cpu %P_CPUFAMILY%

:Fini
