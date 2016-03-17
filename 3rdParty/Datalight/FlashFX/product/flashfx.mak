#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	all the	environment defines, options, and
#	rules for Win32 hosted build environments that use MAKE.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: flashfx.mak $
#	Revision 1.13  2009/07/31 19:41:46Z  garyp
#	Merged from the v4.0 branch.  Removed fxoeapi.lib from the "APP" library
#	list which does not exist for all ports.
#	Revision 1.12  2009/01/20 01:33:23Z  keithg
#	Removed obsolete bbm library.
#	Revision 1.11  2008/06/03 00:26:17Z  keithg
#	Readded the current BBM which was inadvertantly removed.
#	Revision 1.10  2008/05/27 20:27:33Z  keithg
#	Added BBM5 library.
#	Revision 1.9  2007/09/27 22:21:44Z  jeremys
#	Removed fxrelfs.lib from the library lists, as the library no longer
#	exists.
#	Revision 1.8  2007/09/25 22:55:37Z  jeremys
#	Removed FXLOADER libraries from the FXDRVLIBS and FXAPPLIBS lists.
#	Revision 1.7  2006/08/25 22:04:41Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.6  2006/07/20 21:40:38Z  Pauli
#	Added the NAND Simulator library.
#	Revision 1.5  2006/05/07 23:42:06Z  Garyp
#	Added fxvbfutl.lib.
#	Revision 1.4  2006/03/16 19:50:07Z  timothyj
#	Added fxloader.lib to FFXDRVLIBS to resolve several functions for
#	WINCEBL when linking to FXBOOT.lib.
#	Revision 1.3  2006/02/08 02:08:37Z  Garyp
#	New libraries and headers.
#	Revision 1.2  2005/12/08 02:40:54Z  Garyp
#	Added the fxmmgr library.
#	Revision 1.1  2005/10/14 02:08:32Z  Pauli
#	Initial revision
#	Revision 1.2  2005/10/14 03:08:31Z  Garyp
#	Updated to use some renamed libraries.
#	Revision 1.1  2005/10/06 02:44:06Z  Garyp
#	Initial revision
#	Revision 1.3  2005/08/30 07:34:45Z  Garyp
#	Modified so that DCLLIBS is not included in FXDRV/APPLIBS.
#	Revision 1.2  2005/08/21 10:42:00Z  garyp
#	Updated to use renamed headers.
#	Revision 1.1  2005/08/03 19:38:10Z  pauli
#	Initial revision
#	Revision 1.3  2005/01/17 01:59:01Z  GaryP
#	Updated to use the new DCL tree.
#	Revision 1.2  2004/08/06 16:48:56Z  GaryP
#	Added fxrelfs.lib.
#	Revision 1.1  2004/07/07 03:15:06Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

ifndef FLASHFX_MAK_INCLUDED
FLASHFX_MAK_INCLUDED = 1


#--------------------------------------------------------------------
#	Include	the Datalight Common Libraries (which in turn
#	includes the ToolSet information).
#--------------------------------------------------------------------

include	$(P_DCLROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Common headers
#
#	Note that the headers in this list should match those found
#	in flashfx.h.
#--------------------------------------------------------------------

FXHEADERS =	$(DCLHEADERS)				\
		$(P_ROOT)\include\flashfx.h		\
		$(P_ROOT)\include\fxstatus.h		\
		$(P_ROOT)\include\fxtypes.h		\
		$(P_ROOT)\include\fxmacros.h		\
		$(P_ROOT)\include\fxoption.h		\
		$(P_ROOT)\include\fxparam.h		\
		$(P_ROOT)\include\oeslapi.h		\
		$(P_ROOT)\include\fxutil.h		\
		$(P_ROOT)\include\fxtrace.h		\
		$(P_ROOT)\os\$(P_OS)\include\oesl.h	\
		$(P_PROJDIR)\ffxconf.h


#--------------------------------------------------------------------
#	Define the library set to use
#--------------------------------------------------------------------

FXDRVLIBS    = $(B_RELDIR)\lib\fxproj.$(B_LIBEXT)   \
	       $(B_RELDIR)\lib\fxdrvrfw.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxmmgr.$(B_LIBEXT)   \
	       $(B_RELDIR)\lib\fxvbf.$(B_LIBEXT)    \
	       $(B_RELDIR)\lib\fxvbfutl.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxbbm5.$(B_LIBEXT)   \
	       $(B_RELDIR)\lib\fxdevmgr.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxfml.$(B_LIBEXT)    \
	       $(B_RELDIR)\lib\fxnand.$(B_LIBEXT)   \
	       $(B_RELDIR)\lib\fxnndsim.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxnor.$(B_LIBEXT)    \
	       $(B_RELDIR)\lib\fxosserv.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxhooks.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxutil.$(B_LIBEXT)   \
	       $(B_RELDIR)\lib\fxdebug.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxfatfs.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxtools.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxtests.$(B_LIBEXT)

FXAPPLIBS    = $(B_RELDIR)\lib\fxproj.$(B_LIBEXT)   \
	       $(B_RELDIR)\lib\fxextapi.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxosserv.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxhooks.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxutil.$(B_LIBEXT)   \
	       $(B_RELDIR)\lib\fxdebug.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxfatfs.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxvbfutl.$(B_LIBEXT) \
	       $(B_RELDIR)\lib\fxtools.$(B_LIBEXT)  \
	       $(B_RELDIR)\lib\fxtests.$(B_LIBEXT)


#--------------------------------------------------------------------
# NOTE:	The order in which the FlashFX libraries are specified is
#	critical.
#
# APPLIBS link with FXEXTAPI and do not link with the FXVBF, FXMMGR,
# FXBBM5, FXDEVMGR, FXFML, FXNAND, or FXNOR libraries.
#
# For APPLIBs, FXEXTAPI is always listed ahead of FXOSSERV so that
# different output routines can be used with those apps than are
# used in the device driver.
#--------------------------------------------------------------------

FXDRVLIBLST :  DCLLIBLST $(FXDRVLIBS) $(DCLLIBS)
	@echo $(B_RELDIR)\lib\fxproj.$(B_LIBEXT)    $(B_LIBADDCMD)  >$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxdrvrfw.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxmmgr.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxvbf.$(B_LIBEXT)	    $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxvbfutl.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxbbm5.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxdevmgr.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxfml.$(B_LIBEXT)     $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxnand.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxnndsim.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxnor.$(B_LIBEXT)      $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	$(B_COPY) $(P_PROJDIR)\fxdrvlib.lnk + $(P_PROJDIR)\dcllib.lnk $(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxosserv.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxhooks.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxutil.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxdebug.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxfatfs.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxtools.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk
	@echo $(B_RELDIR)\lib\fxtests.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxdrvlib.lnk

FXAPPLIBLST :  DCLLIBLST $(FXAPPLIBS) $(DCLLIBS)
	@echo $(B_RELDIR)\lib\fxproj.$(B_LIBEXT)    $(B_LIBADDCMD)  >$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxextapi.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	$(B_COPY) $(P_PROJDIR)\fxapplib.lnk + $(P_PROJDIR)\dcllib.lnk $(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxosserv.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxhooks.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxutil.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxdebug.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxfatfs.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxvbfutl.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxtools.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk
	@echo $(B_RELDIR)\lib\fxtests.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\fxapplib.lnk


endif	# FLASHFX_MAK_INCLUDED

