@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to validate the CPU type and set the P_CPUFAMILY
REM value for a ToolSet.  The first and only parameter is the CPU type.  On
REM exit, the P_CPUFAMILY value will be set to the CPU family name, or NULL
REM if the CPU type is not valid for the ToolSet.
REM
REM If the CPU type is not valid, the batch file should display a brief list
REM of supported CPU types.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cpucheck.bat $
REM Revision 1.2  2007/05/02 21:10:42Z  Garyp
REM Minor documentation and message cleanup.
REM Revision 1.1  2006/11/02 19:47:08Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM NOTE: These CPU types were derived from v4.2.3 of the Green Hills
REM 	  ARM and MIPS compilers.

set P_CPUFAMILY=

if "%1" == "arm6" 		set P_CPUFAMILY=arm
if "%1" == "arm7" 		set P_CPUFAMILY=arm
if "%1" == "arm7m" 		set P_CPUFAMILY=arm
if "%1" == "arm7tm" 		set P_CPUFAMILY=arm
if "%1" == "arm8" 		set P_CPUFAMILY=arm
if "%1" == "arm9" 		set P_CPUFAMILY=arm
if "%1" == "arm9e" 		set P_CPUFAMILY=arm
if "%1" == "arm10" 		set P_CPUFAMILY=arm
if "%1" == "strongarm" 		set P_CPUFAMILY=arm
if "%1" == "xscale" 		set P_CPUFAMILY=arm
if "%1" == "arm11" 		set P_CPUFAMILY=arm

if "%1" == "mips32" 		set P_CPUFAMILY=mips
if "%1" == "mips32_24kc" 	set P_CPUFAMILY=mips
if "%1" == "mips32_24kf" 	set P_CPUFAMILY=mips
if "%1" == "mips32_24kec" 	set P_CPUFAMILY=mips
if "%1" == "mips32_24kef" 	set P_CPUFAMILY=mips
if "%1" == "mips32_34kc" 	set P_CPUFAMILY=mips
if "%1" == "mips32_34kf" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4kec" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4kem" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4kep" 	set P_CPUFAMILY=mips
if "%1" == "mips32_m4k" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4ksc" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4ksd" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4kc" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4km" 	set P_CPUFAMILY=mips
if "%1" == "mips32_4kp" 	set P_CPUFAMILY=mips
if "%1" == "mips64" 		set P_CPUFAMILY=mips
if "%1" == "mips64_20kc" 	set P_CPUFAMILY=mips
if "%1" == "mips64_25kf" 	set P_CPUFAMILY=mips
if "%1" == "mips64_5kc" 	set P_CPUFAMILY=mips
if "%1" == "mips64_5kf" 	set P_CPUFAMILY=mips
if "%1" == "mipsii" 		set P_CPUFAMILY=mips
if "%1" == "r1900" 		set P_CPUFAMILY=mips
if "%1" == "r1900a" 		set P_CPUFAMILY=mips
if "%1" == "r3000" 		set P_CPUFAMILY=mips
if "%1" == "r3640" 		set P_CPUFAMILY=mips
if "%1" == "r3700" 		set P_CPUFAMILY=mips
if "%1" == "r3750" 		set P_CPUFAMILY=mips
if "%1" == "r3900" 		set P_CPUFAMILY=mips
if "%1" == "r4000" 		set P_CPUFAMILY=mips
if "%1" == "r4100" 		set P_CPUFAMILY=mips
if "%1" == "r4121" 		set P_CPUFAMILY=mips
if "%1" == "r4131" 		set P_CPUFAMILY=mips
if "%1" == "r4200" 		set P_CPUFAMILY=mips
if "%1" == "r4300" 		set P_CPUFAMILY=mips
if "%1" == "r4320" 		set P_CPUFAMILY=mips
if "%1" == "r4400" 		set P_CPUFAMILY=mips
if "%1" == "r4500" 		set P_CPUFAMILY=mips
if "%1" == "r4600" 		set P_CPUFAMILY=mips
if "%1" == "r4700" 		set P_CPUFAMILY=mips
if "%1" == "r4900" 		set P_CPUFAMILY=mips
if "%1" == "r5000" 		set P_CPUFAMILY=mips
if "%1" == "r5400" 		set P_CPUFAMILY=mips
if "%1" == "r5500" 		set P_CPUFAMILY=mips
if "%1" == "r7500" 		set P_CPUFAMILY=mips

if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo cpucheck.bat:  The "ghs" ToolSet does not recognize the CPU type "%1"
echo.
echo The following CPU types are supported:
echo.
echo ARM Family:  	MIPS Family:
echo	arm6 			mips32 		r1900
echo	arm7			mips32_24kc	r1900a
echo	arm7m			mips32_24kf	r3000
echo	arm7tm			mips32_24kec	r3640
echo	arm8			mips32_24kef	r3700
echo	arm9			mips32_34kc	r3750
echo	arm9e			mips32_34kf	r3900
echo	arm10			mips32_4kec	r4000
echo	strongarm		mips32_4kem	r4100
echo	xscale			mips32_4kep	r4121
echo	arm11			mips32_m4k 	r4131
echo				mips32_4ksc	r4200
echo				mips32_4ksd	r4300
echo				mips32_4kc 	r4320
echo				mips32_4km 	r4400
echo				mips32_4kp 	r4500
echo				mips64 		r4600
echo				mips64_20kc	r4700
echo				mips64_25kf	r4900
echo				mips64_5kc 	r5000
echo				mips64_5kf 	r5400
echo				mipsii 		r5500
echo						r7500

:Fini
