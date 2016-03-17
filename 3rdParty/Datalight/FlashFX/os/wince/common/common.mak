#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXOECMN library.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: common.mak $
#	Revision 1.6  2009/07/18 01:55:21Z  garyp
#	Merged from the v4.0 branch.  Added cefls.c.
#	Revision 1.5  2008/11/10 21:46:13Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 15:08:44Z  johnb
#	Cleaned up whitespace alignment
#	Cleaned up use of B_INCLUDES
#	Revision 1.3  2007/12/26 01:46:52Z  Garyp
#	Added ce5fmlioctl.c.
#	Revision 1.2  2007/12/01 18:02:41Z  Garyp
#	Added ceshlcmd.c.
#	Revision 1.1  2005/10/01 09:35:10Z  Pauli
#	Initial revision
#	Revision 1.9  2005/01/17 00:59:21Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.8  2004/09/03 23:45:16Z  jaredw
#	Fixed typo where option.ctr should have be option.c
#	Revision 1.7  2004/08/24 05:50:15Z  GaryP
#	Added cereg.c and cemapmem.c.
#	Revision 1.6  2004/08/19 00:21:51Z  garyp
#	Removed regread.c and regwrite.c.
#	Revision 1.5  2004/07/07 03:04:07Z  GaryP
#	Eliminated the module header.
#	Revision 1.4  2004/07/03 00:02:02Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.3  2004/06/24 21:37:24Z  BillR
#	New interface to multi-threaded	FIMs.  New project hooks for
#	initialization and configuration.
#	Revision 1.2  2004/01/03 02:25:32Z  garys
#	Merge from FlashFXMT
#	Revision 1.1.1.4  2004/01/03 02:25:32  garyp
#	Added cereg.c.
#	Revision 1.1.1.3  2003/12/04 22:00:36Z	garyp
#	Added cemount.c.
#	Revision 1.1.1.2  2003/11/22 03:18:36Z	garyp
#	Added REGREAD.C	and REGWRITE.C.
#	Revision 1.1  2003/02/17 01:46:52Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxoecmn.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSCOMMONHEADERS	= $(FXHEADERS)
OSCOMMONINCLUDES =


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
OSCOMMONSRC    	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OSCOMMONOBJ	:= $(OSCOMMONSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSCOMMONTARGETS	: OSCOMMONDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSCOMMONDIR :
	@echo Processing os\$(P_OS)\common...

$(TARGNAME) : common.mak $(OSCOMMONOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

ce5fmlioctl.$(B_OBJEXT) : $(OSCOMMONHEADERS) ce5fmlioctl.c
cefls.$(B_OBJEXT)       : $(OSCOMMONHEADERS) cefls.c
cemapmem.$(B_OBJEXT)    : $(OSCOMMONHEADERS) cemapmem.c
cereg.$(B_OBJEXT)       : $(OSCOMMONHEADERS) cereg.c
ceshlcmd.$(B_OBJEXT)    : $(OSCOMMONHEADERS) ceshlcmd.c
option.$(B_OBJEXT)      : $(OSCOMMONHEADERS) option.c
parsedrv.$(B_OBJEXT)    : $(OSCOMMONHEADERS) parsedrv.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: OSCOMMONDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXOECMN				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(OSCOMMONSRC)				>>sources


