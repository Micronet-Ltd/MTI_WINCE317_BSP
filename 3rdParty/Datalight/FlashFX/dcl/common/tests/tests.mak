#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLTESTS library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tests.mak $
#	Revision 1.23  2010/12/18 03:55:14Z  jeremys
#	Updated dependencies.
#	Revision 1.22  2010/09/06 17:38:19Z  garyp
#	Added a mutex test.
#	Revision 1.21  2010/04/11 19:19:08Z  garyp
#	Added dltfsshared.c/h.
#	Revision 1.20  2009/11/14 02:09:51Z  garyp
#	Added dltprofiler.c.
#	Revision 1.19  2009/09/08 19:55:42Z  garyp
#	Added dltdevio.c and dltshared.c.
#	Revision 1.18  2009/06/28 00:35:36Z  garyp
#	Added dltassert.c, dltatomic.c, dltthread.c, dltmemmgr.c, and
#	dltmemval.c.
#	Revision 1.17  2009/06/12 02:11:02Z  garyp
#	Added dltfsstress.c.
#	Revision 1.16  2009/05/27 16:41:44Z  garyp
#	Added some missing dependencies.
#	Revision 1.15  2009/05/08 02:13:05Z  garyp
#	Added UTF-8 tests.
#	Revision 1.14  2009/01/19 20:55:19Z  johnb
#	Added multibyte/wide char test function to list of test modules.
#	Revision 1.13  2008/11/10 21:42:35Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.12  2008/11/07 05:04:21Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.11  2008/11/06 21:22:32Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.10  2008/11/06 17:19:11Z  johnb
#	Added RELEASETYPE=PLATFORM.
#	Revision 1.9  2008/11/05 14:56:31Z  jimmb
#	Added dltudivdi3.c for unsigned 64 bit divide to the build system
#	Revision 1.8  2007/05/16 22:18:35Z  garyp
#	Added dltcompiler.c.
#	Revision 1.7  2006/08/25 01:50:15Z  Garyp
#	Updated to use the properly abstracted copy command.
#	Revision 1.6  2006/05/06 22:03:40Z  Garyp
#	Added dltmem.c.
#	Revision 1.5  2006/03/15 00:00:39Z  Pauli
#	Add path test module.
#	Revision 1.4  2006/02/03 00:32:26Z  Pauli
#	Added tests for Date/Time functions.
#	Revision 1.3  2005/12/27 23:03:37Z  Pauli
#	Added 64-bit math and byte order tests.
#	Revision 1.2  2005/12/15 06:53:55Z  garyp
#	Fixed to build properly with Platform Builder.
#	Revision 1.1  2005/09/26 11:59:48Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dltests.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

TESTSHEADERS = $(DCLHEADERS) dltests.h


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

include	$(P_ROOT)\product\dcl.mak


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

dlt64bit.$(B_OBJEXT)		: $(TESTSHEADERS) dlt64bit.c
dltassert.$(B_OBJEXT)		: $(TESTSHEADERS) dltassert.c
dltatomic.$(B_OBJEXT)		: $(TESTSHEADERS) dltatomic.c
dltcompiler.$(B_OBJEXT)		: $(TESTSHEADERS) dltcompiler.c
dltdate.$(B_OBJEXT)		: $(TESTSHEADERS) dltdate.c
dltdevio.$(B_OBJEXT)		: $(TESTSHEADERS) dltdevio.c dltshared.h
dltend.$(B_OBJEXT)		: $(TESTSHEADERS) dltend.c
dltfsio.$(B_OBJEXT)		: $(TESTSHEADERS) dltfsio.c dltfsshared.h dltshared.h
dltfsstress.$(B_OBJEXT)		: $(TESTSHEADERS) dltfsstress.c dltfsshared.h dltshared.h
dltfsshared.$(B_OBJEXT)		: $(TESTSHEADERS) dltfsshared.c dltfsshared.h
dltmain.$(B_OBJEXT)		: $(TESTSHEADERS) dltmain.c
dltmbwc.$(B_OBJEXT)		: $(TESTSHEADERS) dltmbwc.c
dltmem.$(B_OBJEXT)		: $(TESTSHEADERS) dltmem.c
dltmemmgr.$(B_OBJEXT)		: $(TESTSHEADERS) dltmemmgr.c
dltmemval.$(B_OBJEXT)		: $(TESTSHEADERS) dltmemval.c
dltmutex.$(B_OBJEXT)		: $(TESTSHEADERS) dltmutex.c
dltpath.$(B_OBJEXT)		: $(TESTSHEADERS) dltpath.c
dltprofiler.$(B_OBJEXT)		: $(TESTSHEADERS) dltprofiler.c
dltshared.$(B_OBJEXT)		: $(TESTSHEADERS) dltshared.c dltshared.h
dltthread.$(B_OBJEXT)		: $(TESTSHEADERS) dltthread.c
dltudivdi3.$(B_OBJEXT)		: $(TESTSHEADERS) dltudivdi3.c
dltutf8.$(B_OBJEXT)		: $(TESTSHEADERS) dltutf8.c
dltutf8_strings.$(B_OBJEXT)	: $(TESTSHEADERS) dltutf8_strings.c
dltutf_casedata.$(B_OBJEXT)	: $(TESTSHEADERS) dltutf_casedata.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: TESTSDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLTESTS		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(TESTSSRC)				>>sources

