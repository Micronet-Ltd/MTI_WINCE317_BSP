#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXUTIL library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: util.mak $
#	Revision 1.14  2011/02/09 00:58:33Z  garyp
#	Eliminated dskdeb.c.
#	Revision 1.13  2009/12/04 19:39:50Z  garyp
#	Added fxinfohelp.c.
#	Revision 1.12  2009/02/09 00:43:13Z  garyp
#	Modified to always rebuild signon.c because that is where the compile-
#	date comes from.
#	Revision 1.11  2008/11/10 21:45:54Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.10  2008/11/07 15:07:04Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.9  2008/03/09 17:19:36Z  Garyp
#	Fixed a header dependency.
#	Revision 1.8  2007/10/27 16:43:51Z  Garyp
#	Eliminated highbit.c.
#	Revision 1.7  2007/10/14 21:12:34Z  Garyp
#	Added fxecc.c.
#	Revision 1.6  2007/10/05 20:22:12Z  Garyp
#	Removed fmlold.c and record.c.
#	Revision 1.5  2007/07/31 01:53:32Z  Garyp
#	Added fxperflog.c.  Minor logic cleanup.
#	Revision 1.4  2006/10/06 00:45:40Z  Garyp
#	Eliminated printf.c, string.c, atoi.c, and reverse.c.
#	Revision 1.3  2006/05/08 08:05:47Z  Garyp
#	Added fxstats.c.
#	Revision 1.2  2006/02/01 23:42:54Z  Garyp
#	Added fmlold.c.
#	Revision 1.1  2005/11/25 23:02:10Z  Pauli
#	Initial revision
#	Revision 1.2  2005/11/25 23:02:10Z  Garyp
#	Removed rand.c.
#	Revision 1.1  2005/10/02 03:28:34Z  Garyp
#	Initial revision
#	Revision 1.2  2005/08/15 06:54:01Z  garyp
#	Added fxver.c.
#	Revision 1.1  2005/07/06 03:52:58Z  pauli
#	Initial revision
#	Revision 1.23  2005/01/17 00:59:07Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.22  2004/08/30 20:22:33Z  GaryP
#	Added support for stbmutex.c and stbsem.c.
#	Revision 1.21  2004/08/29 18:29:16Z  GaryP
#	Added fxheap.c and fxmemcpy.c.
#	Revision 1.20  2004/07/26 22:45:05Z  BillR
#	Add an implementation of _sysdelay() that uses a calibrated loop for
#	use on platforms that lack a microsecond-resolution time reference,
#	Revision 1.19  2004/07/19 20:44:20Z  BillR
#	Change FindHighestBit() to FfxFindHighestBit() and move it out of BBM.
#	Revision 1.18  2004/07/07 02:57:08Z  GaryP
#	Eliminated the module header.
#	Revision 1.17  2004/06/30 03:19:46Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.16  2004/01/25 06:14:34Z  garys
#	Merge from FlashFXMT
#	Revision 1.15.1.4  2004/01/25 06:14:34	garyp
#	Added dskpart.c	and dskdeb.c.  Eliminated dskutil.c and	trace.c.
#	Revision 1.15.1.3  2004/01/03 06:37:08Z	 garyp
#	Added rand.c.
#	Revision 1.15.1.2  2003/11/29 21:33:58Z	 garyp
#	Added strcmp.c.
#	Revision 1.15  2003/06/11 00:26:40Z  dennis
#	Revision 1.14  2003/05/09 00:39:42  billr
#	Add debug trace	facility and fxtrace tool (DOS only at present).
#	Revision 1.13  2003/04/13 02:41:22Z  garyp
#	Removed	the FAT	specific modules (now in FATFS directory).
#	Revision 1.12  2003/03/26 09:16:12Z  garyp
#	Added a	number of new modules.
#	Revision 1.11  2002/12/16 03:34:48Z  GaryP
#	Fixed to work with the new GMAKEW32.EXE
#	Revision 1.10  2002/12/02 22:04:42Z  garyp
#	Eliminated GETS.C and PUTS.C
#	Revision 1.9  2002/11/23 00:22:08Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.8  2002/11/15 21:22:48Z  garyp
#	Corrected the previous rev.
#	Revision 1.7  2002/11/15 21:12:12Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.6  2002/11/14 19:49:30Z  garyp
#	Fixed the library dependency tree.
#	Revision 1.5  2002/11/09 22:49:09Z  garys
#	Added timer.c
#	Revision 1.4  2002/11/07 10:05:24  garyp
#	Updated	to build CEPB SOURCES files.
#	Revision 1.3  2002/10/28 22:52:16Z  garyp
#	Fixed a	TAB problem.
#	Revision 1.2  2002/10/28 22:36:00Z  garyp
#	Updated	the dependency calculation process.
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxutil.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

UTILHEADERS = $(FXHEADERS)

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
UTILSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
UTILOBJ		:= $(UTILSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

UTILTARGETS : UTILDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

# Always rebuild signon.c because that is where the compile-date comes from.
#
UTILDIR	:
	@echo Processing Common\Util...
	if exist signon.$(B_OBJEXT) del signon.$(B_OBJEXT)

$(TARGNAME) : util.mak $(UTILOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

delay.$(B_OBJEXT)      : $(UTILHEADERS) delay.c
dskchs.$(B_OBJEXT)     : $(UTILHEADERS) dskchs.c
dskinfo.$(B_OBJEXT)    : $(UTILHEADERS) dskinfo.c
dskmbr.$(B_OBJEXT)     : $(UTILHEADERS) dskmbr.c
dskpart.$(B_OBJEXT)    : $(UTILHEADERS) dskpart.c
fxecc.$(B_OBJEXT)      : $(UTILHEADERS) fxecc.c $(P_ROOT)\include\ecc.h
fxinfohelp.$(B_OBJEXT) : $(UTILHEADERS) fxinfohelp.c
fxperflog.$(B_OBJEXT)  : $(UTILHEADERS) fxperflog.c $(P_ROOT)\include\fxperflog.h
fxstats.$(B_OBJEXT)    : $(UTILHEADERS) fxstats.c
fxver.$(B_OBJEXT)      : $(UTILHEADERS) fxver.c
signon.$(B_OBJEXT)     : $(UTILHEADERS) signon.c $(P_ROOT)\include\fxver.h


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: UTILDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXUTIL					>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(UTILSRC)				>>sources

