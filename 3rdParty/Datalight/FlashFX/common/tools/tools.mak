#----------------------------------------------------------------------------
#               Description
#
#   This make file controls the generation of the FXTOOLS library.  It is
#   designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#               Revision History
#   $Log: tools.mak $
#   Revision 1.14  2009/10/06 19:00:07Z  garyp
#   Fixed a tabbing problem with the previous checkin.
#   Revision 1.13  2009/10/06 18:23:30Z  garyp
#   Added fmsl\include to the include path.
#   Revision 1.12  2009/02/09 08:42:10Z  garyp
#   Merged from the v4.0 branch.  Added fxshlshare.c and fxshl.h.
#   Revision 1.11  2008/11/10 21:45:50Z  johnb
#   Remove B_INCLUDES default value.
#   Revision 1.10  2008/11/07 15:06:44Z  johnb
#   Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#   Revision 1.9  2007/09/12 22:08:07Z  Garyp
#   Added fxshldbg.c.  Removed a bogus header dependency.
#   Revision 1.8  2007/02/04 00:21:26Z  Garyp
#   Added fxshlcmd.c.
#   Revision 1.7  2006/08/25 01:52:18Z  Garyp
#   Updated to use a properly abstracted copy command.
#   Revision 1.6  2006/05/28 17:26:41Z  Garyp
#   Added fxshl.c.
#   Revision 1.5  2006/05/08 09:12:34Z  Garyp
#   Added fxdmpbbm.c.
#   Revision 1.4  2006/03/06 01:19:00Z  Garyp
#   Eliminated FXRDIMG/FXWRIMG and replaced with FXIMAGE.
#   Revision 1.3  2006/01/31 07:26:03Z  Garyp
#   Eliminated trace.c.
#   Revision 1.2  2005/12/08 03:02:06Z  Garyp
#   Updated for new VBF directory structure.
#   Revision 1.1  2005/10/14 02:15:50Z  Pauli
#   Initial revision
#   Revision 1.18  2005/01/28 21:04:46Z  GaryP
#   Modified to no longer change into the target directory since that is
#   already handled in the traverse process.
#   Revision 1.17  2004/10/27 23:30:22Z  jaredw
#   Changed fxdump to fxdmp.
#   Revision 1.16  2004/10/21 18:53:16Z  jaredw
#   Added fxdump to make.
#   Revision 1.15  2004/07/07 02:56:51Z  GaryP
#   Eliminated the module header.
#   Revision 1.14  2004/06/30 03:19:45Z  GaryP
#   Updated to use new P_ and B_ build symbols.
#   Revision 1.13  2004/01/19 00:20:42Z  garys
#   Merge from FlashFXMT
#   Revision 1.12.1.2  2004/01/19 00:20:42  garyp
#   Added format.c and trace.c.
#   Revision 1.12  2003/05/22 21:30:40Z  brandont
#   FXCHK will once again be built for DOS, VxWorks, and WinCE
#   Revision 1.11  2003/04/22 18:03:59  billr
#   Fix FXINFO for VBF4. Change the UnitInformation structure and add
#   vbfgetpartitioninfo() to support this.
#   Revision 1.10  2003/04/21 20:22:43Z  garys
#   Enable building reclaim.c and remount.c
#   Revision 1.9  2003/04/16 16:18:44  billr
#   Merge from VBF4 branch.
#   Revision 1.8  2003/03/24 05:38:44Z  garyp
#   Updated to use the B_LIBEXT setting.
#   Revision 1.7.1.2  2003/02/13 23:26:17Z  dennis
#   Changed the in-memory Allocation struct definition and changes to the
#   MM interface.
#   Revision 1.7  2002/11/23 00:21:54Z  garyp
#   Updated to use $(FXHEADERS).
#   Revision 1.6  2002/11/15 21:22:48Z  garyp
#   Corrected the previous rev.
#   Revision 1.5  2002/11/15 21:12:12Z  garyp
#   Changed the CE PB ToolSet name to "mswcepb".
#   Revision 1.4  2002/11/07 10:05:06Z  garyp
#   Updated to build CEPB SOURCES files.
#   Revision 1.3  2002/10/28 22:59:38Z  garyp
#   Fixed a TAB problem.
#   Revision 1.2  2002/10/28 22:34:38Z  garyp
#   Updated dependency calculation process.
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#   Settings
#--------------------------------------------------------------------

TARGFILE = fxtools.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

TOOLSHEADERS = $(FXHEADERS) $(P_ROOT)\include\fxtools.h

B_LOCALINC = $(P_ROOT)\fmsl\include


#--------------------------------------------------------------------
#   Build a list of all the C source files, except those in the
#   EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC          := $(wildcard *.c)
EXCLUDE         :=
TOOLSSRC        := $(filter-out $(EXCLUDE), $(ALLSRC))
TOOLSOBJ	:= $(TOOLSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#   Targets
#--------------------------------------------------------------------

TOOLSTARGETS : TOOLSDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#   Default Project Rules
#--------------------------------------------------------------------

include $(P_ROOT)\product\flashfx.mak

#--------------------------------------------------------------------
#   Build Commands
#--------------------------------------------------------------------

TOOLSDIR :
	@echo Processing Common\Tools...

$(TARGNAME) : tools.mak $(TOOLSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#   Dependencies
#--------------------------------------------------------------------

chkunits.$(B_OBJEXT)    : $(TOOLSHEADERS) chkunits.c
fdinfo.$(B_OBJEXT)  	: $(TOOLSHEADERS) fdinfo.c
format.$(B_OBJEXT)  	: $(TOOLSHEADERS) format.c
fxdmp.$(B_OBJEXT)       : $(TOOLSHEADERS) fxdmp.c fxdmpbbm.h
fxdmpbbm.$(B_OBJEXT)    : $(TOOLSHEADERS) fxdmpbbm.c fxdmpbbm.h
fximg.$(B_OBJEXT)       : $(TOOLSHEADERS) fximg.c
fxshl.$(B_OBJEXT)   	: $(TOOLSHEADERS) fxshl.c
fxshlcmd.$(B_OBJEXT)    : $(TOOLSHEADERS) fxshlcmd.c fxshl.h
fxshldbg.$(B_OBJEXT)    : $(TOOLSHEADERS) fxshldbg.c fxshl.h
fxshlshare.$(B_OBJEXT)  : $(TOOLSHEADERS) fxshlshare.c fxshl.h
reclaim.$(B_OBJEXT) 	: $(TOOLSHEADERS) reclaim.c
remount.$(B_OBJEXT) 	: $(TOOLSHEADERS) remount.c


#--------------------------------------------------------------------
#   Special Targets for Windows CE Platform Builder only
#--------------------------------------------------------------------

mswcepb : TOOLSDIR TOOLSETINIT
	@echo #                         			 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT   >>sources
	@echo #                         			>>sources
	@echo RELEASETYPE=PLATFORM              		>>sources
	@echo TARGETNAME=FXTOOLS                		>>sources
	@echo TARGETTYPE=LIBRARY                		>>sources
	@echo TARGETLIBS=                   			>>sources
	@echo EXEENTRY=                     			>>sources
	@echo SOURCES=$(TOOLSSRC)               		>>sources


