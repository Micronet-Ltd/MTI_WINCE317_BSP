#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE device
#	driver.	 It is designed	for use	with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: driver.mak $
#	Revision 1.13.1.2  2012/04/12 18:18:50Z  johnb
#	Updated MSWCEPB libraries.
#	Revision 1.13.1.1  2009/02/09 07:37:06Z  johnb
#	Duplicate revision
#	Revision 1.13  2009/02/09 07:37:06Z  garyp
#	Merged from the v4.0 branch.  Modified the linker flags to more closely
#	mirror what WinMobile uses.  Major simplification to link the driver using
#	fxdrvlib.lnk.  Flashfx.lib is still built, but it only contains libraries which are
#	used to create external FlashFX aware applications.  Updated to link 
#	with B_DRIVERLIBS to allow the project to specify additional link libraries.
#	Added some logic so we operate better when building for the "mswcepb"
#	target (may still need some work).
#	Revision 1.12  2008/11/11 04:28:05Z  johnb
#	Remove B_INCLUDES default setting
#	Revision 1.10  2008/11/07 15:10:19Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.  Added new
#   target to DriverTarget to correct platform builder build issue.
#	Revision 1.9  2008/02/08 03:58:25Z  Garyp
#	Added ffxfmd.c.  Modified to build a library as well as the DLL.
#	Revision 1.8  2007/11/25 03:53:43Z  Garyp
#	Properly corrected the "problem" from the previous revision.
#	Revision 1.7  2007/06/05 19:08:14Z  rickc
#	Removed undeclared DRIVERDIR from PB section.
#	Revision 1.6  2007/04/02 16:18:21Z  keithg
#	FSINTTEST has been removed from FFX, it is now part of DCL.
#	Revision 1.5  2007/03/02 00:26:31Z  Garyp
#	Removed CE 3 support.
#	Revision 1.4  2007/01/24 00:22:35Z  Garyp
#	Fixed the source files list to ensure there are no duplicated entries.
#	Revision 1.3  2007/01/23 23:35:34Z  Garyp
#	Updated to copy fsitdev.c and .h from the projects tree into the current
#	directory to be built.
#	Revision 1.2  2006/08/25 22:37:31Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.1  2005/10/01 09:35:16Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/30 07:34:44Z  Garyp
#	Modified so that DCLLIBS is specified separately from the other product
#	libraries.
#	Revision 1.1  2005/03/18 05:18:40Z  pauli
#	Initial revision
#	Revision 1.27  2005/03/18 03:18:39Z  GaryP
#	Updated to use new mechanisms for generating the MSWCEPB link libraries.
#	Revision 1.26  2005/02/27 04:08:26Z  GaryP
#	Updated to use new CE settings that work better with CE 5.
#	Revision 1.25  2005/02/25 02:38:33Z  GaryP
#	Cosmetic cleanup.
#	Revision 1.24  2005/01/17 00:59:23Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled by the traverse process.  Modified to use the new DCL
#	shared CPU libraries.
#	Revision 1.23  2004/09/27 19:12:52Z  tonyq
#	Updated for Windows CE 5.0 Platform builder (command line build)
#	Revision 1.22  2004/09/23 21:06:08Z  tonyq
#	Updated to support building from the command line under Windows CE 5.0
#	(with the MSWCE4 toolset)
#	Revision 1.21  2004/09/10 20:41:17Z  jaredw
#	Added oemcommon library to linklist.
#	Revision 1.20  2004/08/06 16:48:56Z  GaryP
#	Added fxrelfs.lib.
#	Revision 1.19  2004/07/07 03:04:12Z  GaryP
#	Eliminated the module header.
#	Revision 1.18  2004/07/01 18:45:19Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.17  2004/02/27 23:47:24Z  garys
#	Merge from FlashFXMT
#	Revision 1.16.1.6  2004/02/27 23:47:24	garyp
#	Added FXDEBUG.LIB.
#	Revision 1.16.1.5  2004/01/03 02:23:10Z	 garyp
#	Renamed	DSKDiskEntry() to FlashFXDriverEntry().
#	Revision 1.16.1.4  2003/12/13 21:52:22Z	 garyp
#	Renamed	FIM2 to	FIMMT.
#	Revision 1.16.1.3  2003/12/04 22:16:24Z	 garyp
#	Split ffxdrv.c into a number of	modules.
#	Revision 1.16.1.2  2003/11/23 22:36:08Z	 garyp
#	Added the FXFIM2 and FXPRJHKS libraries.
#	Revision 1.16  2003/05/29 02:01:38Z  garyp
#	Fixed to build right under CE3
#	Revision 1.15  2003/04/15 17:49:12Z  garyp
#	Updated	to link	with the new FXDRVRFW and FXFATFS libraries.
#	Revision 1.14  2003/03/26 03:08:22Z  garyp
#	Cleaned	up now that we don't the the ugly tests	hack.  Modified	to
#	place the MAP files in the release directory.
#	Revision 1.13  2003/02/26 18:25:02Z  garys
#	Release	flashfx.rel along with flashfx.dll so we can load into slot1
#	instead	of using process virtual memory	space.
#	Revision 1.12  2003/02/25 20:27:02  garyp
#	Fixed the linker flags to match	those used in PB.
#	Revision 1.11  2002/12/16 18:40:59Z  GaryP
#	Fixed to work with the new GMAKEW32.EXE
#	Revision 1.10  2002/11/23 00:29:20Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.9  2002/11/15 21:22:48Z  garyp
#	Corrected the previous rev.
#	Revision 1.8  2002/11/15 21:12:12Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.7  2002/11/14 21:29:08Z  garyp
#	Modified to avoid linking with the CPU libraries if they do not	exist.
#	Revision 1.6  2002/11/14 10:23:14Z  garyp
#	Hacked the SOURCES generation process to copy the necessary
#	files to compile in the	unit tests.
#	Revision 1.5  2002/11/13 21:24:39Z  qa
#	Add support for	SXIP.
#	Revision 1.4  2002/11/07 19:44:06Z  garyp
#	Fixed syntax errors in the SOURCES generation.
#	Revision 1.3  2002/11/07 10:14:10Z  garyp
#	Updated	to build CEPB SOURCES files.
#	Revision 1.2  2002/10/28 23:14:44Z  garyp
#	Modified the dependency	calculation process.  Added the	module header.
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = ..\..\..\dcl\os\win\include

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

TARGLIB = $(B_RELDIR)\flashfx.lib
TARGNAME = $(B_RELDIR)\flashfx.dll

DRIVERHEADERS =	$(FXHEADERS) ffxdrv.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

EXTRASRC	:=
ALLSRC      	:= $(wildcard ffx*.c) $(EXTRASRC)
EXCLUDE     	:=
DRIVERSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
DRIVEROBJ	:= $(DRIVERSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DRIVERTARGETS :	DRIVERDIR TOOLSETINIT $(TARGLIB) $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

# Create an APPLIBS symbol which is the complete set of libraries to
# include in flashfx.lib -- for building FlashFX aware applications
# (not device drivers).  This essentially must consist of everything
# in the B_RELDIR\lib directory minus any libraries which appear in
# the FXDRVLIBS list, but not in the FXAPPLIBS list.  Note that the
# "wildcard" and "filter-out" commands do not seem to work when the
# symbols include backslashes.
#
PTH := $(subst \,/,$(B_RELDIR))
APP := $(subst \,/,$(FXAPPLIBS))
DRV := $(subst \,/,$(FXDRVLIBS))
ALLLIBS		:= $(wildcard $(PTH)/lib/*.$(B_LIBEXT))
DRVLIBEXCLUDE   := $(filter-out $(APP), $(DRV))
APPLIBS         := $(filter-out $(DRVLIBEXCLUDE), $(ALLLIBS))
EXTRALIBEXCLUDE := $(PTH)/lib/dlwinutilgui.$(B_LIBEXT)
APPLIBS         := $(filter-out $(EXTRALIBEXCLUDE), $(APPLIBS))

DRIVERDIR :
	@echo Processing os\$(P_OS)\Driver...
	if exist flashfx.lib del flashfx.lib

$(TARGLIB) : driver.mak $(APPLIBS)
	$(B_ADDLIBS) flashfx.lib $(APPLIBS)
	$(B_COPY) flashfx.lib $(TARGLIB)

$(TARGNAME) : flashfx.dll
	if exist flashfx.rel $(B_COPY) flashfx.rel $(B_RELDIR)\.
	if exist flashfx.map $(B_COPY) flashfx.map $(B_RELDIR)\.
	if exist flashfx.pdb $(B_COPY) flashfx.pdb $(B_RELDIR)\.
	$(B_COPY) flashfx.dll $(TARGNAME)

flashfx.dll : driver.mak FXDRVLIBLST $(DRIVEROBJ) $(B_RELDIR)/lib/fxoecmn.lib
	echo /out:flashfx.dll /dll					 >flashfx.lnk
	echo /def:flashfx.def $(DRIVEROBJ)				>>flashfx.lnk
	echo /map:flashfx.map						>>flashfx.lnk
	echo /incremental:no						>>flashfx.lnk
	echo /merge:.rdata=.text					>>flashfx.lnk
	echo /stack:65536,4096 /base:0x10000000				>>flashfx.lnk
	echo /savebaserelocations:flashfx.rel				>>flashfx.lnk
	echo /largeaddressaware						>>flashfx.lnk
	echo $(B_LFLAGS)						>>flashfx.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>flashfx.lnk
	echo $(B_CESDKDIR)\corelibc.lib		  	 		>>flashfx.lnk
ifdef B_DRIVERLIBS
	echo $(B_DRIVERLIBS)					 	>>flashfx.lnk
endif
	echo $(B_RELDIR)\lib\dlosutil.lib            			>>flashfx.lnk
	echo $(B_RELDIR)\lib\dlwindebug.lib          			>>flashfx.lnk
	echo $(B_RELDIR)\lib\fxoecmn.lib			 	>>flashfx.lnk
	$(B_COPY) /a flashfx.lnk + $(P_PROJDIR)\fxdrvlib.lnk	  	  flashfx.lnk
	$(B_LINK) @flashfx.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

ffxdrv.$(B_OBJEXT)   : $(DRIVERHEADERS)	ffxdrv.c
ffxfmd.$(B_OBJEXT)   : $(DRIVERHEADERS)	ffxfmd.c
ffxinit.$(B_OBJEXT)  : $(DRIVERHEADERS)	ffxinit.c
ffxioctl.$(B_OBJEXT) : $(DRIVERHEADERS)	ffxioctl.c
ffxsgio.$(B_OBJEXT)  : $(DRIVERHEADERS)	ffxsgio.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PBPRODLIBS =	$(FXDRVLIBS)				\
		$(DCLLIBS)				\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwindebug.$(B_LIBEXT)	\
		fxnand.$(B_LIBEXT)			\
		fxoecmn.$(B_LIBEXT)

.PHONY: $(PBPRODLIBS)

$(PBPRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>pbprodlibs.lst

mswcepb	: DRIVERDIR TOOLSETINIT $(PBPRODLIBS)
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=FlashFX					 >>sources
	@echo TARGETTYPE=DYNLINK					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\ceddk.lib				\>>sources
	$(B_COPY) /a sources + pbprodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo DLLENTRY=DllMain						 >>sources
	@echo SOURCES=$(DRIVERSRC)					 >>sources

