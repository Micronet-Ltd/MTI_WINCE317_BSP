REM Only build the CPUTYPE tree if it exists and if it is different
REM than CPUFAMILY.

REM We must build the CPUTYPE code prior to the CPUFAMILY code so that
REM the CPUTYPE make file can set up any code that must be excluded 
REM from the CPUFAMILY library.

set CPUT=

if not "%P_CPUTYPE%" == "%P_CPUFAMILY%" set CPUT=%P_CPUTYPE%

REM If a type is specified that we don't support, skip it
if not exist %CPUT%\*.* set CPUT=

call %B_TRAVERSE% cpu\%P_CPUFAMILY% %CPUT% cpubase

set CPUT=

