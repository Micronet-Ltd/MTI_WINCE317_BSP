#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLCLIB library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: clib.mak $
#	Revision 1.20  2010/12/18 03:57:22Z  jeremys
#	Updated dependencies.
#	Revision 1.19  2009/05/27 16:41:44Z  garyp
#	Corrected for a file which was renamed.
#	Revision 1.18  2009/05/20 16:58:37Z  keithg
#	Added dclutf8stricmp.c dependancy.
#	Revision 1.17  2009/05/08 18:10:07Z  garyp
#	Added a number of modules related to UCS-4 and UTF-8 functionality.
#	Revision 1.16  2009/05/07 17:27:14Z  garyp
#	Added dlwctolower.c.
#	Revision 1.15  2009/01/19 20:57:48Z  johnb
#	Added dlmbtowc.c and dlwctomb.c to list of modules.
#	Revision 1.14  2008/11/10 21:41:38Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.13  2008/11/07 05:02:34Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.12  2008/11/06 21:23:03Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.11  2008/11/03 20:20:54Z  johnb
#	Added RELEASETYPE=PLATFORM
#	Revision 1.10  2008/07/23 04:52:28Z  keithg
#	Added dlmemmove.c module dependancies.
#	Revision 1.9  2008/06/05 01:38:41Z  brandont
#	Added dlstrnchr.
#	Revision 1.8  2008/05/19 22:12:44Z  brandont
#	Added dependencies for dlstrrchr.c.
#	Revision 1.7  2007/04/01 17:40:15Z  Garyp
#	Added dlstrncat.c.
#	Revision 1.6  2006/11/29 23:18:02Z  Garyp
#	Added dlntoul.c.
#	Revision 1.5  2006/10/02 17:20:15Z  Garyp
#	Added dlhtoul.c.
#	Revision 1.4  2006/07/06 01:32:42Z  Garyp
#	Cleaned up the library generation logic.
#	Revision 1.3  2006/05/06 22:03:40Z  Garyp
#	Added dlltoa.c.
#	Revision 1.2  2006/01/10 08:21:56Z  Garyp
#	Added dlstrcat.c.
#	Revision 1.1  2005/12/05 17:26:14Z  Pauli
#	Initial revision
#	Revision 1.3  2005/12/05 17:26:13Z  Pauli
#	Merge with 2.0 product line, build 173.
#	Revision 1.2  2005/11/25 23:02:10Z  Garyp
#	Added dlrand.c.
#	Revision 1.1  2005/07/31 00:35:12Z  Garyp
#	Initial revision
#	Revision 1.2  2005/07/31 00:35:12Z  Garyp
#	Added dlulltoa.c.
#	Revision 1.1  2005/07/18 08:54:44Z  pauli
#	Initial revision
#	Revision 1.2  2005/06/13 02:22:52Z  PaulI
#	Added support for DCL lib prefix.
#	Revision 1.1  2005/04/10 19:20:28Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlclib.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

CLIBHEADERS = $(DCLHEADERS)

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
CLIBSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
CLIBOBJ		:= $(CLIBSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

CLIBTARGETS : CLIBDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

CLIBDIR	:
	@echo Processing Common\Clib...

$(TARGNAME) : clib.mak $(CLIBOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlatoi.$(B_OBJEXT)		: $(CLIBHEADERS) dlatoi.c
dlatol.$(B_OBJEXT)		: $(CLIBHEADERS) dlatol.c
dlhtoul.$(B_OBJEXT)		: $(CLIBHEADERS) dlhtoul.c
dlltoa.$(B_OBJEXT)		: $(CLIBHEADERS) dlltoa.c
dlntoul.$(B_OBJEXT)		: $(CLIBHEADERS) dlntoul.c
dlmbtowc.$(B_OBJEXT)		: $(CLIBHEADERS) dlmbtowc.c
dlmemaln.$(B_OBJEXT)		: $(CLIBHEADERS) dlmemaln.c
dlmemcmp.$(B_OBJEXT)		: $(CLIBHEADERS) dlmemcmp.c
dlmemcpy.$(B_OBJEXT)		: $(CLIBHEADERS) dlmemcpy.c
dlmemmove.$(B_OBJEXT)		: $(CLIBHEADERS) dlmemmove.c
dlmemset.$(B_OBJEXT)		: $(CLIBHEADERS) dlmemset.c
dlprintf.$(B_OBJEXT)		: $(CLIBHEADERS) dlprintf.c
dlrand.$(B_OBJEXT)		: $(CLIBHEADERS) dlrand.c
dlsprint.$(B_OBJEXT)		: $(CLIBHEADERS) dlsprint.c
dlstrcat.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrcat.c
dlstrchr.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrchr.c
dlstrnchr.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrnchr.c
dlstrrchr.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrrchr.c
dlstrcmp.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrcmp.c
dlstrcpn.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrcpn.c
dlstrcpy.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrcpy.c
dlstrlen.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrlen.c
dlstrncat.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrncat.c
dlstrncm.$(B_OBJEXT)		: $(CLIBHEADERS) dlstrncm.c
dlultoa.$(B_OBJEXT)		: $(CLIBHEADERS) dlultoa.c
dlulltoa.$(B_OBJEXT)		: $(CLIBHEADERS) dlulltoa.c
dlutftolower.$(B_OBJEXT)	: $(CLIBHEADERS) dlutftolower.c
dlutf8.$(B_OBJEXT)		: $(CLIBHEADERS) dlutf8.c
dlutf8strlower.$(B_OBJEXT)	: $(CLIBHEADERS) dlutf8strlower.c
dlutf8stricmp.$(B_OBJEXT)	: $(CLIBHEADERS) dlutf8stricmp.c
dlutf8strnicompare.$(B_OBJEXT)	: $(CLIBHEADERS) dlutf8strnicompare.c
dlwctomb.$(B_OBJEXT)		: $(CLIBHEADERS) dlwctomb.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: CLIBDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLCLIB		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(CLIBSRC)				>>sources

