#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXFML library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fml.mak $
#	Revision 1.10  2009/07/22 17:17:15Z  garyp
#	Merged from the v4.0 branch.  Added fmlotp.c, fmlrange.c, fmllock.c, and
#	fmlpower.c.
#	Revision 1.9  2008/11/07 15:07:36Z  johnb
#	Cleaned up whitespace alignment.
#	Revision 1.8  2007/12/26 01:46:51Z  Garyp
#	Added fmlperflog.c.
#	Revision 1.7  2006/10/10 01:48:49Z  Garyp
#	Added fmlfmt.c.
#	Revision 1.6  2006/03/15 22:24:44Z  Garyp
#	Removed the All-In-One-Framework module -- now located in the Driver
#	Framework.
#	Revision 1.5  2006/03/15 01:42:30Z  rickc
#	Added include path for building with Windows CE Platform Builder
#	Revision 1.4  2006/02/10 09:47:37Z  Garyp
#	Refactored such that the FML is literally just the flash mapping layer.
#	Other functionality is moved into the Device Manager layer.
#	Revision 1.2  2006/01/13 22:27:33Z  Garyp
#	Updated to build oemutil.c.
#	Revision 1.1  2005/11/26 17:14:30Z  Pauli
#	Initial revision
#	Revision 1.4  2005/11/26 17:14:29Z  Garyp
#	Added a number of new modules and headers.
#	Revision 1.3  2005/10/27 03:49:15Z  Garyp
#	Updated header dependencies.
#	Revision 1.2  2005/10/09 22:54:49Z  Garyp
#	Added fml.c.  Removed modules related to singlethreaded FIMs.
#	Revision 1.1  2005/10/05 22:26:34Z  Garyp
#	Initial revision
#	Revision 1.17  2005/04/25 02:54:23Z  garyp
#	Updated dependencies.
#	Revision 1.16  2005/01/17 00:59:19Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.15  2004/07/07 02:58:38Z  GaryP
#	Eliminated the module header.
#	Revision 1.14  2004/06/30 03:19:30Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.13  2004/02/22 21:30:24Z  garys
#	Merge from FlashFXMT
#	Revision 1.10.1.4  2004/02/22 21:30:24	garyp
#	Added fmlthunk.c.
#	Revision 1.10.1.3  2003/11/22 04:02:36Z	 garyp
#	Renamed	FMSLAIOF.C to FMLAIOF.C.  Added	FMLDISP.C.
#	Revision 1.10.1.2  2003/11/04 00:36:58Z	 garyp
#	Re-checked into	variant	sandbox.
#	Revision 1.11  2003/11/04 00:36:58Z  garyp
#	Added FMLBOX.C.
#	Revision 1.10  2003/03/21 17:18:28Z  garyp
#	Added FMSLAIOF.C.  Deleted the tests stuff.
#	Revision 1.9  2002/12/05 11:28:26Z  garyp
#	Eliminated the use of CD commands so we	work with GMAKEW32.
#	Revision 1.8  2002/11/23 00:24:26Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.7  2002/11/15 21:22:48Z  garyp
#	Corrected the previous rev.
#	Revision 1.6  2002/11/15 21:12:12Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.5  2002/11/09 18:15:16Z  garyp
#	Moved VBFDATA.C	into VBF.
#	Revision 1.4  2002/11/07 10:31:34Z  garyp
#	Updated	to build CEPB SOURCES files.
#	Revision 1.3  2002/11/05 21:57:20Z  garys
#	enabled	testoem	build and linked into fxtests.$(B_LIBEXT)
#	Revision 1.2  2002/10/28 23:10:50  garyp
#	Modified the dependency	calculation process.  Added the	module header.
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\fmsl\include

TARGNAME = $(B_RELDIR)\lib\fxfml.$(B_LIBEXT)

FMLHEADERS = $(FXHEADERS) fml.h 		\
		..\..\include\fxiosys.h		\
		..\..\include\fxfmlapi.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
FMLSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
FMLOBJ		:= $(FMLSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

FMLTARGETS : FMLDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

FMLDIR	:
	@echo Processing FMSL\FML...

$(TARGNAME) : fml.mak $(FMLOBJ)
	if exist fxfml.$(B_LIBEXT) del fxfml.$(B_LIBEXT)
	$(B_BUILDLIB) fxfml.$(B_LIBEXT) *.$(B_OBJEXT)
	$(B_COPY) fxfml.$(B_LIBEXT) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fml.$(B_OBJEXT)        : $(FMLHEADERS)	fml.c
fmldisp.$(B_OBJEXT)    : $(FMLHEADERS)	fmldisp.c
fmlfmt.$(B_OBJEXT)     : $(FMLHEADERS)	fmlfmt.c
fmllock.$(B_OBJEXT)    : $(FMLHEADERS)	fmllock.c
fmlnand.$(B_OBJEXT)    : $(FMLHEADERS)	fmlnand.c
fmlnor.$(B_OBJEXT)     : $(FMLHEADERS)	fmlnor.c
fmlotp.$(B_OBJEXT)     : $(FMLHEADERS)	fmlotp.c
fmlperflog.$(B_OBJEXT) : $(FMLHEADERS)	fmlperflog.c
fmlpower.$(B_OBJEXT)   : $(FMLHEADERS)	fmlpower.c
fmlrange.$(B_OBJEXT)   : $(FMLHEADERS)	fmlrange.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: FMLDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXFML					>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(FMLSRC)					>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources


