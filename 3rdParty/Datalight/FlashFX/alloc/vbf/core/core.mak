#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXVBF library.  It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: core.mak $
#	Revision 1.9  2010/11/12 22:39:51Z  glenns
#	Added rule for vbfquickmount.c.
#	Revision 1.8  2009/07/21 20:23:52Z  garyp
#	Merged from the v4.0 branch.  Removed vbfshare.c and added vbfpower.c, 
#	vbfcompidle.c, vbfnand.c and vbfnor.c.  Updated to current makefile 
#	standards.
#	Revision 1.7  2008/11/07 15:03:07Z  johnb
#	Cleaned up whitespace alignment
#	Revision 1.6  2007/06/12 23:48:09Z  rickc
#	Removed vbf.c
#	Revision 1.5  2006/05/04 19:33:58Z  Garyp
#	Added vbfrefmt.c.
#	Revision 1.4  2006/03/15 21:54:59Z  Garyp
#	Removed the All-In-One-Framework module -- now located in the Driver
#	Framework.
#	Revision 1.3  2006/02/21 02:02:39Z  Garyp
#	Removed vbfuinfo.c and vbfpinfo.c.  Added vbfapi.c and vbfmets.c.
#	Revision 1.2  2005/12/15 06:37:43Z  Garyp
#	Updated include path for CEPB.
#	Revision 1.1  2005/12/08 03:10:24Z  Garyp
#	Initial revision
#	Revision 1.32  2005/03/04 20:57:39Z  GaryP
#	Minor formatting changes.
#	Revision 1.31  2005/01/17 00:59:07Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.30  2004/11/19 02:13:01Z  GaryP
#	Added vbfcomp.c/h.
#	Revision 1.29  2004/09/25 01:30:20Z  GaryP
#	Eliminated vbfprog.c.
#	Revision 1.28  2004/07/07 02:57:28Z  GaryP
#	Eliminated the module header.
#	Revision 1.27  2004/07/03 00:04:33Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.26  2004/01/20 22:02:54Z  garys
#	Merge from FlashFXMT
#	Revision 1.23.1.5  2004/01/20 22:02:54	garyp
#	Fixed the header dependencies.
#	Revision 1.23.1.4  2003/12/16 22:02:02Z	 garyp
#	Eliminated vbfdata.c.
#	Revision 1.23.1.3  2003/11/17 22:15:54Z	 garyp
#	Added VBFDISP.C.
#	Revision 1.23.1.2  2003/11/03 06:11:22Z	 garyp
#	Re-checked into	variant	sandbox.
#	Revision 1.24  2003/11/03 06:11:22Z  garyp
#	Eliminated VBFMEM.C.  Added VBFINST.C.
#	Revision 1.23  2003/06/13 06:37:12Z  garyp
#	Eliminated VBFLDATA.C
#	Revision 1.22  2003/06/12 19:42:40Z  dennis
#	removed	amdbtx8	from fim.mak (it was previously moved to
#	extras).  Removed .h dependencies from bbm.mak and vbf.mak to
#	placate	obfuscator.  Changed __LINE__ to a literal number in
#	tstwint.c to avoid obfuspew.  Various other changes to
#	obfusprocess to	work with included headers.
#	Revision 1.21  2003/05/21 02:27:44  garyp
#	Added TSTWINT.C	and TSTRPERF.C.	 Updated header	dependencies.
#	Revision 1.20  2003/05/02 20:35:00Z  garyp
#	Fixed the HAMMING.OBJ dependency.
#	Revision 1.19  2003/05/02 17:12:40Z  garyp
#	Removed	an obsolete header.
#	Revision 1.18  2003/04/29 23:46:38Z  garyp
#	Removed	FXVBFLOW.LIB.
#	Revision 1.17  2003/04/23 21:11:26Z  billr
#	Fix FXINFO for VBF4. Change the	UnitInformation	structure and add
#	vbfgetpartitioninfo()	to support this.
#	Revision 1.16  2003/04/22 01:06:02Z  dennis
#	Added more write interruption tests and	code to	deal with them.
#	GCs will now fully clean a region if called by a background process
#	without	disturbing temporal order.  GC "granularity" is	decreased.
#	Revision 1.15  2003/04/16 17:39:38  billr
#	Merge from VBF4	branch.
#	Revision 1.14  2003/04/15 17:49:12Z  garyp
#	Removed	VBFWINT.C.  Moved to the COMMON/TESTS directory.
#	Revision 1.11.1.7  2003/04/10 01:38:35Z	 dennis
#	Removed	most of	the conditional	code left over from the	MM merge.
#	Revision 1.11.1.6  2003/04/09 20:01:52	dennis
#	Stricter alignment in buffer used to read allocations off of units
#	during region mount.  Better checking of valid metadata.  Handle bad
#	allocs during region mount.
#	Revision 1.11.1.5  2003/04/04 23:41:10	billr
#	Add the	new Media Manager code.
#	Revision 1.11.1.4  2003/03/25 22:08:18Z	 dennis
#	All unit default unit tests pass with metadata working on NXMS,
#	comments, functions and	data cleaned up.
#	Revision 1.13  2003/03/21 18:08:04Z  garyp
#	Added VBFAIOF.C.  Eliminated the test stuff.
#	Revision 1.11.1.3  2003/02/12 00:16:13	dennis
#	Changed	the in-memory Allocation struct	definition and changes to the
#	MM interface.
#	Revision 1.11.1.2  2002/12/16 03:33:12	dennis
#	Established temporal order to units within a region.  Expanded the
#	representation of unit counts to 32-bits.
#	Revision 1.12  2002/12/16 03:33:12Z  dennis
#	No changes.
#	Revision 1.11  2002/12/16 03:33:12  GaryP
#	Fixed to work with the new GMAKEW32.EXE
#	Revision 1.10  2002/12/05 11:28:10Z  garyp
#	Eliminated the use of CD commands so we	work with GMAKEW32.
#	Revision 1.9  2002/11/23 00:23:58Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.8  2002/11/15 21:22:50Z  garyp
#	Corrected the previous rev.
#	Revision 1.7  2002/11/15 21:12:18Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.6  2002/11/12 22:11:10Z  garyp
#	Added VBFDATA.C
#	Revision 1.5  2002/11/08 22:59:18Z  garys
#	build testvbf into fxtests.$(B_LIBEXT)
#	Revision 1.4  2002/11/07 10:05:58  garyp
#	Updated	to build CEPB SOURCES files.
#	Revision 1.3  2002/10/28 23:07:20Z  garyp
#	Modified the dependency	calculation process.  Added the	module header.
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxvbf.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\alloc\vbf\include

VBFHEADERS = 	$(FXHEADERS)			\
		..\include\vbfint.h		\
		..\include\vbfunit.h		\
		..\include\mediamgr.h 		\
		$(P_ROOT)\include\vbf.h 	\
		$(P_ROOT)\include\vbfconf.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
VBFSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
VBFOBJ		:= $(VBFSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

VBFTARGETS : VBFDIR TOOLSETINIT	$(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

VBFDIR :
	@echo Processing Alloc\VBF\Core...

$(TARGNAME) : core.mak $(VBFOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fxoption.$(B_OBJEXT)      : $(VBFHEADERS) fxoption.c
vbfapi.$(B_OBJEXT)        : $(VBFHEADERS) vbfapi.c
vbfcomp.$(B_OBJEXT)  	  : $(VBFHEADERS) vbfcomp.c
vbfcompidle.$(B_OBJEXT)	  : $(VBFHEADERS) vbfcompidle.c
vbfdeb.$(B_OBJEXT)        : $(VBFHEADERS) vbfdeb.c
vbfdisp.$(B_OBJEXT)       : $(VBFHEADERS) vbfdisp.c
vbffmt.$(B_OBJEXT)        : $(VBFHEADERS) vbffmt.c
vbfinst.$(B_OBJEXT)       : $(VBFHEADERS) vbfinst.c
vbfnand.$(B_OBJEXT)  	  : $(VBFHEADERS) vbfnand.c
vbfnor.$(B_OBJEXT)   	  : $(VBFHEADERS) vbfnor.c
vbfmets.$(B_OBJEXT)       : $(VBFHEADERS) vbfmets.c
vbfpower.$(B_OBJEXT) 	  : $(VBFHEADERS) vbfpower.c
vbfrefmt.$(B_OBJEXT)      : $(VBFHEADERS) vbfrefmt.c
vbfreg.$(B_OBJEXT)        : $(VBFHEADERS) vbfreg.c
vbfquickmount.$(B_OBJEXT) : $(VBFHEADERS) vbfquickmount.c
vbfsup.$(B_OBJEXT)        : $(VBFHEADERS) vbfsup.c ..\include\tstwint.h
tstwint.$(B_OBJEXT)       : $(VBFHEADERS) tstwint.c ..\include\tstwint.h
tstrperf.$(B_OBJEXT)      : $(VBFHEADERS) tstrperf.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: VBFDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXVBF					>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(VBFSRC)					>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources


