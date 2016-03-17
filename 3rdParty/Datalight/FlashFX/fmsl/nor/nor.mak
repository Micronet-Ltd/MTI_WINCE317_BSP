#----------------------------------------------------------------------------
#                       Description
#
#   This make file controls the generation of the FXNOR library.  It is
#   designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#               Revision History
#   $Log: nor.mak $
#   Revision 1.20  2008/11/07 15:08:31Z  johnb
#   Cleaned up whitespace alignment
#   Revision 1.19  2008/02/28 21:32:00Z  Glenns
#   Add support for Eclipse FIM
#   Revision 1.18  2007/01/02 22:27:35Z  rickc
#   Replaced cfix16 and cfi2x16 with norfim
#   Revision 1.17  2006/12/16 03:41:14Z  Garyp
#   Added norram.h.
#   Revision 1.16  2006/12/07 23:08:25Z  rickc
#   Re-enabled CFI FIMs
#   Revision 1.15  2006/11/29 22:05:21Z  rickc
#   Disabled CFI FIMs
#   Revision 1.14  2006/11/27 17:05:24Z  rickc
#   Re-enabled CFI FIMs
#   Revision 1.13  2006/11/22 00:40:07Z  rickc
#   Disabled CFI FIMs
#   Revision 1.12  2006/11/21 22:46:08Z  rickc
#   Re-enabled CFI FIMs
#   Revision 1.11  2006/11/18 03:01:37Z  Garyp
#   Disabled building the CFI stuff until it builds cleanly.
#   Revision 1.10  2006/11/01 22:41:19Z  rickc
#   Added cfix16 and cfi2x16 FIMs
#   Revision 1.9  2006/08/20 02:37:28Z  Garyp
#   Added normap.c.
#   Revision 1.8  2006/08/19 01:16:47Z  Garyp
#   Updated dependencies.
#   Revision 1.7  2006/05/01 23:44:17Z  DeanG
#   Revise to new makefile policy:  build all .C files in directory by default.
#   Revision 1.6  2006/03/15 00:59:36Z  rickc
#   Added include path for building with Windows CE Platform Builder
#   Revision 1.5  2006/02/09 22:41:55Z  Garyp
#   Added norwrap.c.  Eliminated fimhelp.c.
#   Revision 1.4  2006/01/18 21:52:06Z  Rickc
#   Added Intel Sibley support files
#   Revision 1.3  2006/01/12 20:08:03Z  Rickc
#   added asbx16.c
#   Revision 1.2  2005/12/15 06:48:52Z  garyp
#   Fixed to would properly with Windows CE Platform Builder.
#   Revision 1.1  2005/10/27 02:49:16Z  Pauli
#   Initial revision
#   Revision 1.2  2005/10/27 03:49:15Z  Garyp
#   Updated header dependencies.
#   Revision 1.1  2005/10/14 03:12:32Z  Garyp
#   Initial revision
#   Revision 1.24  2005/05/10 18:57:58Z  garyp
#   Added asux8.
#   Revision 1.23  2005/05/08 20:30:03Z  garyp
#   Added iff2x16.c.  Updated dependencies.
#   Revision 1.22  2005/04/07 00:14:19Z  tonyq
#   Added new 4x8 FIM for WindRiver's PowerQuicII board
#   Revision 1.21  2005/03/27 19:45:28Z  GaryP
#   Eliminated bogus cobf stuff.
#   Revision 1.20  2005/01/17 00:59:18Z  GaryP
#   Modified to no longer change into the target directory since that is
#   already handled in the traverse process.
#   Revision 1.19  2004/10/25 04:19:55Z  GaryP
#   Added norfile.c.
#   Revision 1.18  2004/08/10 21:39:24Z  jaredw
#   Added isfx16 MT fim.
#   Revision 1.17  2004/08/10 06:17:21Z  garyp
#   Fixed a typo.
#   Revision 1.16  2004/08/06 19:58:29Z  GaryP
#   Replace the ramdisk FIM with the norram FIM.
#   Revision 1.15  2004/07/26 18:44:06Z  jaredw
#   Added iffx8, iffx16, asb2x16, amb2x16 MT fims.
#   Revision 1.14  2004/07/08 00:04:43Z  GaryP
#   Re-added asu4x8.c
#   Revision 1.13  2004/07/07 02:58:50Z  GaryP
#   Eliminated the module header.
#   Revision 1.12  2004/06/30 03:19:31Z  GaryP
#   Updated to use new P_ and B_ build symbols.
#   Revision 1.1  2003/12/13 21:56:12Z  garyp
#   Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#   Settings
#-----------------------------------------------------------

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\fmsl\include

TARGFILE = fxnor.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

NORHEADERS =    $(FXHEADERS) nor.h norread.h flashcmd.h \
		..\..\include\fxfmlapi.h ..\include\fimdev.h


#----------------------------------------------------------------------------
#   Datalight makefiles compile all source files in the current directory
#   by default.  Any source files in the current directory that should
#   NOT be compiled, must be listed in the EXCLUDE variable below.
#   Note the use of ":=";  this is required.
#----------------------------------------------------------------------------

EXCLUDE := 
ALLSRC  := $(wildcard *.c)
NORSRC  := $(filter-out $(EXCLUDE), $(ALLSRC))
NOROBJ  := $(NORSRC:.c=.$(B_OBJEXT))


#-----------------------------------------------------------
#   Targets
#-----------------------------------------------------------

NORTARGETS : NORDIR TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#   Default Project Rules
#-----------------------------------------------------------

include $(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#   Build Commands
#-----------------------------------------------------------

NORDIR :
	@echo Processing FMSL\NOR...

$(TARGNAME) : nor.mak $(NOROBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#-----------------------------------------------------------
#   Dependencies
#-----------------------------------------------------------

asux8.$(B_OBJEXT)       : $(NORHEADERS) asux8.c     amdcmd.h
asu4x8.$(B_OBJEXT)      : $(NORHEADERS) asu4x8.c    amdcmd.h
ambx16.$(B_OBJEXT)      : $(NORHEADERS) ambx16.c    amdcmd.h
amb2x16.$(B_OBJEXT)     : $(NORHEADERS) amb2x16.c   amdcmd.h
asb2x16.$(B_OBJEXT)     : $(NORHEADERS) asb2x16.c   amdcmd.h
asbx16.$(B_OBJEXT)      : $(NORHEADERS) asbx16.c    amdcmd.h
norfim.$(B_OBJEXT)      : $(NORHEADERS) norfim.c    norfim.h
iffx8.$(B_OBJEXT)       : $(NORHEADERS) iffx8.c     intelcmd.h
iffx16.$(B_OBJEXT)      : $(NORHEADERS) iffx16.c    intelcmd.h
iff2x16.$(B_OBJEXT)     : $(NORHEADERS) iff2x16.c   intelcmd.h
isfx16.$(B_OBJEXT)      : $(NORHEADERS) isfx16.c    intelcmd.h
isf2x16.$(B_OBJEXT)     : $(NORHEADERS) isf2x16.c   intelcmd.h
isf4x8.$(B_OBJEXT)      : $(NORHEADERS) isf4x8.c    intelcmd.h
iswfx16.$(B_OBJEXT)     : $(NORHEADERS) iswfx16.c   intelcmd.h
norfile.$(B_OBJEXT)     : $(NORHEADERS) norfile.c
normap.$(B_OBJEXT)      : $(NORHEADERS) normap.c    ..\devmgr\devmgr.h
norram.$(B_OBJEXT)      : $(NORHEADERS) norram.c    norram.h
norread.$(B_OBJEXT)     : $(NORHEADERS) norread.c
norwrap.$(B_OBJEXT)     : $(NORHEADERS) norwrap.c
eclipsex16.$(B_OBJEXT)  : $(NORHEADERS) eclipsex16.c amdcmd_eclipse.h

#-----------------------------------------------------------
#   Special Targets for Windows CE Platform Builder only
#-----------------------------------------------------------

mswcepb : NORDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=fxnor					>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(NORSRC)					>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources

