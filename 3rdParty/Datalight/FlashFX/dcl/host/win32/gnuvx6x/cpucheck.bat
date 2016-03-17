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
REM of valid CPU types.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cpucheck.bat $
REM Revision 1.7  2010/06/14 17:54:35Z  jimmb
REM Additional changes to support VxWorks 6.9
REM Revision 1.6  2010/06/11 15:33:45Z  jimmb
REM Syncronized diab and gnu 
REM Support for VxWorks 6.9
REM Revision 1.5  2009/06/13 18:13:18Z  jimmb
REM Added support for the Omap 3530
REM Revision 1.4  2008/05/01 17:35:49Z  jimmb
REM Added support for the ARM v6 processor
REM Revision 1.3  2007/10/06 01:53:16Z  Garyp
REM Updated for VxWorks 6.6.
REM Revision 1.2  2007/05/02 21:10:40Z  Garyp
REM Minor documentation and message cleanup.
REM Revision 1.1  2005/04/24 16:25:14Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set P_CPUFAMILY=
if "%1" == "armarch4"        set P_CPUFAMILY=arm
if "%1" == "armarch4_t"      set P_CPUFAMILY=arm
if "%1" == "armarch5"        set P_CPUFAMILY=arm
if "%1" == "armarch5_t"      set P_CPUFAMILY=arm
if "%1" == "armarch6"        set P_CPUFAMILY=arm
if "%1" == "strongarm"  set P_CPUFAMILY=arm
if "%1" == "arm7tdmi"        set P_CPUFAMILY=arm
if "%1" == "arm7tdmi_t"      set P_CPUFAMILY=arm
if "%1" == "arm710a"         set P_CPUFAMILY=arm
if "%1" == "arm810"          set P_CPUFAMILY=arm
if "%1" == "armsa110"        set P_CPUFAMILY=arm
if "%1" == "cortexa8"        set P_CPUFAMILY=arm
if "%1" == "xscale"          set P_CPUFAMILY=arm
if "%1" == "mcf5200"    	 set P_CPUFAMILY=coldfire
if "%1" == "mcf5400"    	 set P_CPUFAMILY=coldfire
if "%1" == "mc68000"         set P_CPUFAMILY=mc68k
if "%1" == "mc68010"         set P_CPUFAMILY=mc68k
if "%1" == "mc68020"         set P_CPUFAMILY=mc68k
if "%1" == "mc68030"         set P_CPUFAMILY=mc68k
if "%1" == "mc68040"         set P_CPUFAMILY=mc68k
if "%1" == "mc68060"         set P_CPUFAMILY=mc68k
if "%1" == "mc68lc040"       set P_CPUFAMILY=mc68k
if "%1" == "mc68ec040"       set P_CPUFAMILY=mc68k
if "%1" == "cpu32"           set P_CPUFAMILY=mc68k
if "%1" == "pentium"         set P_CPUFAMILY=x86
if "%1" == "pentium2"        set P_CPUFAMILY=x86
if "%1" == "pentium3"        set P_CPUFAMILY=x86
if "%1" == "pentium4"        set P_CPUFAMILY=x86
if "%1" == "ppc403"          set P_CPUFAMILY=ppc
if "%1" == "ppc405"          set P_CPUFAMILY=ppc
if "%1" == "ppc440"          set P_CPUFAMILY=ppc
if "%1" == "ppc603"          set P_CPUFAMILY=ppc
if "%1" == "ppc604"          set P_CPUFAMILY=ppc
if "%1" == "ppc85xx"         set P_CPUFAMILY=ppc
if "%1" == "ppc32"           set P_CPUFAMILY=ppc
if "%1" == "ppc860"          set P_CPUFAMILY=ppc
if "%1" == "simsparcsolaris" set P_CPUFAMILY=simulator
if "%1" == "simnt"           set P_CPUFAMILY=simulator
if "%1" == "mips32"          set P_CPUFAMILY=mips
if "%1" == "mipsi32"         set P_CPUFAMILY=mips
if "%1" == "mips64"          set P_CPUFAMILY=mips
if "%1" == "mipsi64"         set P_CPUFAMILY=mips
if "%1" == "mipsi2"          set P_CPUFAMILY=mips
if "%1" == "mipsi3"          set P_CPUFAMILY=mips
if "%1" == "sh7600"          set P_CPUFAMILY=sh
if "%1" == "sh7700"          set P_CPUFAMILY=sh
if "%1" == "sh7750"          set P_CPUFAMILY=sh
if "%1" == "sh7751"          set P_CPUFAMILY=sh


if not "%P_CPUFAMILY%" == "" goto Fini

echo.
echo CPUCHECK.BAT:  This ToolSet does not recognize the CPU type "%1"
echo.
echo The following CPU types are supported:
echo.
echo ARM, StrongARM, XScale:  armarch4 armarch4_t armarch5 armarch5_t
echo                          armarch6 arm7tdmi arm7tdmi_t arm710a arm810
echo                          armsa110 cortexa8 strongarm xscale
echo.
echo MC68K:                   mc68000 mc68010 mc68020 mc68030 mc68040
echo                          mc68060 mc68lc040 mc68ec040 cpu32
echo.
echo Coldfire:                mcf5200 mcf5400
echo.
echo SuperH:                  sh7600 sh7700 sh7750 sh7751
echo.
echo MIPS:                    mips32 mipsi32 mips64 mipsi64 mipsi2 mipsi3
echo.
echo Pentium:                 pentium pentium2 pentium3 pentium4
echo.
echo PowerPC:                 ppc403 ppc405 ppc440 ppc603 ppc604
echo                          ppc85xx ppc860 ppc32
echo.
echo Simulator:               simsparcsolaris simnt
echo.
echo Note that the CPU type must be specified in all lower case.
echo.



:Fini
