#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLUTIL library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: util.mak $
#	Revision 1.38  2011/03/23 23:20:54Z  garyp
#	Added dlsortinsert32.c.
#	Revision 1.37  2011/02/06 02:05:14Z  garyp
#	Added dlmbr.c.
#	Revision 1.36  2010/09/18 02:14:33Z  garyp
#	Added dlsleeplock.c.
#	Revision 1.35  2010/04/12 03:29:50Z  garyp
#	Added dlfsdirtree.c.
#	Revision 1.34  2010/02/23 20:30:40Z  garyp
#	Added a missing dependency.
#	Revision 1.33  2009/12/02 21:52:30Z  garyp
#	Added dlfmtbitflags.c and dlcritsec.c.
#	Revision 1.32  2009/10/08 22:11:27Z  garyp
#	Fixed the PB build to use a properly abstracted symbol.
#	Revision 1.31  2009/10/05 19:09:35Z  garyp
#	Added dleccomap35x.c.
#	Revision 1.30  2009/09/24 17:30:15Z  garyp
#	Added dlmuldiv64.c.
#	Revision 1.29  2009/09/14 22:26:36Z  garyp
#	Added dlmuldiv.c.
#	Revision 1.28  2009/09/10 17:31:30Z  garyp
#	Added dlsizetoulkb.c.
#	Revision 1.27  2009/06/27 20:16:07Z  garyp
#	Removed dlassert.c and dllog.c.  Added dlassertservice/requestor.c,
#	dlmemvalservice/requestor.c, dllogservice/requestor.c, dlatomic.c, and
#	dlsizetoul.c.
#	Revision 1.26  2009/02/08 04:48:28Z  garyp
#	Added dldisp.c and dlscale.c.
#	Revision 1.25  2009/02/08 00:54:40Z  garyp
#	Merged from the v4.0 branch.  Added dlinput.c and the missing dlcrc16.c.
#	Revision 1.24  2008/11/10 21:43:47Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.23  2008/11/07 05:04:38Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.22  2008/11/06 21:22:24Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.21  2008/11/06 17:19:37Z  johnb
#	Added RELEASETYPE=PLATFORM
#	Revision 1.20  2008/11/05 14:57:09Z  jimmb
#	Added dltudivdi3.c for unsigned 64 bit divide to the build system
#	Revision 1.19  2008/04/19 01:35:15Z  brandont
#	Added dependencies for dlfsstat.c, dlfsfile.c, dlfsdir.c, and
#	dlfsredir.c.
#	Revision 1.18  2007/10/30 21:08:16Z  Garyp
#	Added dlbit.c.
#	Revision 1.17  2007/10/16 20:16:50Z  Garyp
#	Added dldelay.c.
#	Revision 1.16  2007/08/19 17:38:51Z  garyp
#	Added dlver.c.
#	Revision 1.15  2007/08/15 23:07:01Z  garyp
#	Added dlerrlev.c.
#	Revision 1.14  2007/06/26 00:02:05Z  brandont
#	Added dlcmdln.c.
#	Revision 1.13  2007/05/16 22:16:56Z  garyp
#	Added dlenviron.c.
#	Revision 1.12  2007/04/11 02:17:49Z  Garyp
#	Added dlratio.c.
#	Revision 1.11  2007/04/02 15:44:45Z  thomd
#	Correct dependencies
#	Revision 1.10  2007/02/16 20:45:25Z  billr
#	Add new file.
#	Revision 1.9  2006/09/16 19:49:56Z  Garyp
#	Removed dlver.c.
#	Revision 1.8  2006/08/15 22:41:30Z  Garyp
#	Dependencies updated.
#	Revision 1.7  2006/07/14 01:46:57Z  brandont
#	Added dlrdwrsem.c.
#	Revision 1.6  2006/07/06 01:32:41Z  Garyp
#	Added dltimehr.c.
#	Revision 1.5  2006/03/14 23:54:37Z  Pauli
#	Added path utility module.
#	Revision 1.4  2006/02/25 04:13:00Z  Garyp
#	Added dlhamm.c.
#	Revision 1.3  2006/02/03 00:30:29Z  Pauli
#	Added dldate.c for common date/time functions.
#	Revision 1.2  2006/01/01 16:18:35Z  Garyp
#	Added dlsys,c.
#	Revision 1.1  2005/12/01 01:43:48Z  Pauli
#	Initial revision
#	Revision 1.3  2005/12/01 01:43:47Z  Pauli
#	Merge with 2.0 product line, build 173.
#	Revision 1.2  2005/11/06 03:44:12Z  Garyp
#	Added dlstats.c.
#	Revision 1.1  2005/09/28 19:41:24Z  Garyp
#	Initial revision
#	Revision 1.4  2005/09/28 19:41:23Z  Garyp
#	Added dllog.c.
#	Revision 1.3  2005/09/21 07:39:54Z  garyp
#	Added dlarg.c.
#	Revision 1.2  2005/08/15 06:40:24Z  garyp
#	Added dlver.c.
#	Revision 1.1  2005/07/06 03:54:06Z  pauli
#	Initial revision
#	Revision 1.5  2005/06/20 18:59:52Z  Pauli
#	Added 64-bit match module.
#	Revision 1.4  2005/06/13 02:22:50Z  PaulI
#	Added support for DCL lib prefix.
#	Revision 1.3  2005/04/10 19:20:26Z  PaulI
#	XP Merge
#	Revision 1.2.1.2  2005/04/10 19:20:26Z  garyp
#	Added dlassert.c, dloutput.c, and dlultoa.c.
#	Revision 1.2  2005/02/22 18:31:28Z  GaryP
#	Minor syntax cleanup.
#	Revision 1.1  2005/01/11 19:40:00Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dlutil
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

UTILHEADERS = $(DCLHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:= dltls.c dltlstemp.c
UTILSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
UTILOBJ		:= $(UTILSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

UTILTARGETS : UTILDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

# Always rebuild dlsignon.c because that is where the compile-date comes from.
#
UTILDIR	:
	@echo Processing Common\Util...
	if exist dlsignon.$(B_OBJEXT) del dlsignon.$(B_OBJEXT)

$(TARGNAME) : util.mak $(UTILOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies, in ALPHABETICAL order
#--------------------------------------------------------------------

dl64bit.$(B_OBJEXT)  		: $(UTILHEADERS) dl64bit.c
dlarg.$(B_OBJEXT)    		: $(UTILHEADERS) dlarg.c
dlassertrequestor.$(B_OBJEXT) 	: $(UTILHEADERS) dlassertrequestor.c
dlassertservice.$(B_OBJEXT) 	: $(UTILHEADERS) dlassertservice.c
dlatomic.$(B_OBJEXT) 	 	: $(UTILHEADERS) dlatomic.c
dlbit.$(B_OBJEXT)    		: $(UTILHEADERS) dlbit.c
dlcmdln.$(B_OBJEXT)  		: $(UTILHEADERS) dlcmdln.c $(P_ROOT)/include/dlcmdln.h
dlcrc16.$(B_OBJEXT)   		: $(UTILHEADERS) dlcrc16.c
dlcrc32.$(B_OBJEXT)   		: $(UTILHEADERS) dlcrc32.c
dlcritsec.$(B_OBJEXT)   	: $(UTILHEADERS) dlcritsec.c
dldate.$(B_OBJEXT)   		: $(UTILHEADERS) dldate.c
dldelay.$(B_OBJEXT)     	: $(UTILHEADERS) dldelay.c
dldisp.$(B_OBJEXT)   		: $(UTILHEADERS) dldisp.c
dlecc.$(B_OBJEXT)    		: $(UTILHEADERS) dlecc.c
dleccomap35x.$(B_OBJEXT)    	: $(UTILHEADERS) dleccomap35x.c
dlend.$(B_OBJEXT)    		: $(UTILHEADERS) dlend.c
dlenviron.$(B_OBJEXT)  		: $(UTILHEADERS) dlenviron.c
dlerrlev.$(B_OBJEXT)  		: $(UTILHEADERS) dlerrlev.c $(P_ROOT)/include/dlerrlev.h
dlfmtbitflags.$(B_OBJEXT)  	: $(UTILHEADERS) dlfmtbitflags.c
dlfsdir.$(B_OBJEXT)  		: $(UTILHEADERS) dlfsdir.c
dlfsdirtree.$(B_OBJEXT)  	: $(UTILHEADERS) dlfsdirtree.c
dlfsfile.$(B_OBJEXT)  		: $(UTILHEADERS) dlfsfile.c
dlfsredir.$(B_OBJEXT)  		: $(UTILHEADERS) dlfsredir.c
dlfsstat.$(B_OBJEXT)  		: $(UTILHEADERS) dlfsstat.c
dlhamm.$(B_OBJEXT)   		: $(UTILHEADERS) dlhamm.c
dlheap.$(B_OBJEXT)   		: $(UTILHEADERS) dlheap.c
dlinput.$(B_OBJEXT)   		: $(UTILHEADERS) dlinput.c
dllogrequestor.$(B_OBJEXT)    	: $(UTILHEADERS) dllogrequestor.c $(P_ROOT)/include/dllog.h
dllogservice.$(B_OBJEXT)    	: $(UTILHEADERS) dllogservice.c $(P_ROOT)/include/dllog.h
dlmbr.$(B_OBJEXT)    	  	: $(UTILHEADERS) dlmbr.c $(P_ROOT)/include/dlmbr.h
dlmem.$(B_OBJEXT)    	  	: $(UTILHEADERS) dlmem.c dlmemtracking.h
dlmemtracking.$(B_OBJEXT) 	: $(UTILHEADERS) dlmemtracking.c dlmemtracking.h
dlmemvalrequestor.$(B_OBJEXT) 	: $(UTILHEADERS) dlmemvalrequestor.c
dlmemvalservice.$(B_OBJEXT)  	: $(UTILHEADERS) dlmemvalservice.c
dlmuldiv.$(B_OBJEXT)  		: $(UTILHEADERS) dlmuldiv.c
dlmuldiv64.$(B_OBJEXT)  	: $(UTILHEADERS) dlmuldiv64.c
dlmutex.$(B_OBJEXT)  		: $(UTILHEADERS) dlmutex.c
dloutput.$(B_OBJEXT) 		: $(UTILHEADERS) dloutput.c
dlpath.$(B_OBJEXT)   		: $(UTILHEADERS) dlpath.c
dlratio.$(B_OBJEXT)   		: $(UTILHEADERS) dlratio.c
dlrdwrsem.$(B_OBJEXT)		: $(UTILHEADERS) dlrdwrsem.c
dlscale.$(B_OBJEXT)    		: $(UTILHEADERS) dlscale.c
dlsem.$(B_OBJEXT)    		: $(UTILHEADERS) dlsem.c
dlsignon.$(B_OBJEXT) 		: $(UTILHEADERS) dlsignon.c $(P_ROOT)/include/dlver.h
dlsizetoul.$(B_OBJEXT) 	  	: $(UTILHEADERS) dlsizetoul.c
dlsizetoulkb.$(B_OBJEXT) 	: $(UTILHEADERS) dlsizetoulkb.c
dlsleeplock.$(B_OBJEXT) 	: $(UTILHEADERS) dlsleeplock.c
dlsortinsert32.$(B_OBJEXT) 	: $(UTILHEADERS) dlsortinsert32.c
dlstats.$(B_OBJEXT)  		: $(UTILHEADERS) dlstats.c $(P_ROOT)/include/dlstats.h
dlsys.$(B_OBJEXT)    		: $(UTILHEADERS) dlsys.c $(P_ROOT)/include/dlstats.h
dltimehr.$(B_OBJEXT) 		: $(UTILHEADERS) dltimehr.c
dltimer.$(B_OBJEXT)  		: $(UTILHEADERS) dltimer.c
dludivdi3.$(B_OBJEXT)  		: $(UTILHEADERS) dludivdi3.c
dlver.$(B_OBJEXT)  		: $(UTILHEADERS) dlver.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: UTILDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)$(TARGBASE)		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(UTILSRC)				>>sources

