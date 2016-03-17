#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXTESTS library.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tests.mak $
#	Revision 1.12  2010/07/07 19:49:41Z  garyp
#	Added fmslbbm.c.
#	Revision 1.11  2009/11/17 02:30:36Z  garyp
#	Added fmsllock.c.
#	Revision 1.10  2008/11/10 21:45:46Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.9  2008/11/07 15:06:34Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.8  2008/05/03 22:34:49Z  garyp
#	Fixed a dependency to work when building as a sub-product.
#	Revision 1.7  2008/04/14 23:17:37Z  garyp
#	Updated a dependency.
#	Revision 1.6  2007/10/27 02:41:09Z  Garyp
#	Added fmslstress.c.  Eliminated the obsolete unittest.h.
#	Revision 1.5  2007/10/22 21:33:05Z  pauli
#	Added fmslecc.c and vbfqa.c.
#	Revision 1.4  2006/08/25 01:51:45Z  Garyp
#	Updated to use a properly abstracted copy command.
#	Revision 1.3  2006/05/18 23:21:04Z  Garyp
#	Added fmslnor.c, fmsltst.h, and renamed fmslutil_iswf.c to fmsliswf.c.
#	Revision 1.2  2006/01/18 22:57:31Z  Rickc
#	Added Intel Sibley support files
#	Revision 1.1  2005/11/02 12:08:06Z  Pauli
#	Initial revision
#	Revision 1.2  2005/11/02 12:08:05Z  Garyp
#	Added fmslnand.c.
#	Revision 1.1  2005/09/30 03:15:42Z  Garyp
#	Initial revision
#	Revision 1.2  2005/09/30 03:15:42Z  Garyp
#	Movied FSIOTEST from FlashFX into DCL.
#	Revision 1.1  2005/07/16 16:15:46Z  pauli
#	Initial revision
#	Revision 1.10  2005/01/17 00:59:09Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.9  2004/07/07 02:56:45Z  GaryP
#	Eliminated the module header.
#	Revision 1.8  2004/07/01 18:44:51Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.7  2004/02/26 22:40:00Z  garys
#	Merge from FlashFXMT
#	Revision 1.6.1.5  2004/02/26 22:40:00  garyp
#	Minor cleanup, no functional changes.
#	Revision 1.6.1.4  2004/02/04 03:15:48Z	garyp
#	Renamed	mttst.c	to mtstrss.c.
#	Revision 1.6.1.3  2004/01/03 04:35:20Z	garyp
#	Renamed	mttest to mttst.
#	Revision 1.6.1.2  2003/12/11 19:44:34Z	billr
#	Simple multithreaded exerciser test.
#	Revision 1.6  2003/05/21 02:35:08Z  garyp
#	Updated	header dependencies.
#	Revision 1.5  2003/05/05 20:40:04Z  garyp
#	Added UNITTEST.H to the	headers	list.
#	Revision 1.4  2003/05/02 17:45:16Z  garyp
#	Eliminated the reference to VBFTST.H.
#	Revision 1.3  2003/04/23 00:12:43Z  dennis
#	Added pseudo-random interruption order in the write interruption tests
#	w/in VBFTST.C.  Removed VBFWINT.C from project and moved guts of that
#	module into VBFTST.C; updated \common\tests\tests.mak.  Added stub funcs
#	to $(P_OSDIR)dos\driver\memclien.c and to \common\tools\wrimage.c so
#	that these functions did not link in VBFTST.C during a debug build.
#	Added a new link line to \$(P_OSDIR)dos\tests\tests.mak so that the VBF
#	unit test does not link in wrimage.c to	satisfy	the vbfwriteint function
#	dependency instead of using the REAL vbfwriteint in VBFTST.C as it very
#	very clearly should be doing.
#	Revision 1.2  2003/04/15 17:49:12  garyp
#	Added VBFWINT.C
#	Revision 1.1  2003/03/26 04:00:24Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxtests.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

TESTSHEADERS = $(FXHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
TESTSSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
TESTSOBJ	:= $(TESTSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

TESTSTARGETS : TESTSDIR	TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

TESTSDIR :
	@echo Processing Common\Tests...

$(TARGNAME) : tests.mak	$(TESTSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fmslbbm.$(B_OBJEXT)    : $(TESTSHEADERS) fmslbbm.c    fmsltst.h fmslnand.h
fmslecc.$(B_OBJEXT)    : $(TESTSHEADERS) fmslecc.c    fmsltst.h fmslnand.h
fmsliswf.$(B_OBJEXT)   : $(TESTSHEADERS) fmsliswf.c   fmsltst.h
fmsllock.$(B_OBJEXT)   : $(TESTSHEADERS) fmsllock.c   fmsltst.h
fmslnand.$(B_OBJEXT)   : $(TESTSHEADERS) fmslnand.c   fmsltst.h fmslnand.h
fmslnor.$(B_OBJEXT)    : $(TESTSHEADERS) fmslnor.c    fmsltst.h
fmslstress.$(B_OBJEXT) : $(TESTSHEADERS) fmslstress.c fmsltst.h
fmsltst.$(B_OBJEXT)    : $(TESTSHEADERS) fmsltst.c    fmsltst.h
mtstrss.$(B_OBJEXT)    : $(TESTSHEADERS) mtstrss.c    $(P_DCLROOT)\include\dlprintf.h
vbftst.$(B_OBJEXT)     : $(TESTSHEADERS) vbftst.c     vbftst.h
vbfqa.$(B_OBJEXT)      : $(TESTSHEADERS) vbfqa.c      vbftst.h


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: TESTSDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXTESTS				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(TESTSSRC)				>>sources

















