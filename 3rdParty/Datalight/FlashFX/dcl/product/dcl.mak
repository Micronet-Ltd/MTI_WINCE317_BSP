#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	all the	environment defines, options, and
#	rules for Win32 hosted build environments that use MAKE.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dcl.mak $
#	Revision 1.24  2009/07/15 07:04:41Z  keithg
#	Added FTPFS library to the DCL list.
#	Revision 1.23  2009/07/02 23:24:20Z  garyp
#	Corrected the GENERIC_INCLUDES logic to work properly with ToolSets
#	which use response files.
#	Revision 1.22  2009/06/25 02:27:00Z  garyp
#	Eliminated the B_PROJLIB workaround.  No longer use shredir.exe.
#	Revision 1.21  2009/02/09 08:04:53Z  garyp
#	Added the B_PROJLIB workaround which is necessary until such time as
#	the full WM branch merge is done.
#	Revision 1.20  2009/02/08 03:24:58Z  garyp
#	Merged from the v4.0 branch.  Removed dlfat.h and added dliosys.h to
#	DCLHEADERS.  Updated to use B_ASMDEFS and B_ASMINCS.
#	Revision 1.19  2007/10/17 01:29:53Z  brandont
#	Added the testfw components.
#	Revision 1.18  2007/10/05 23:27:06Z  brandont
#	Added the loader test library.
#	Revision 1.17  2007/10/05 19:40:19Z  brandont
#	Removed dlreaderio.h and dlloader.h from the global include list.
#	Revision 1.16  2007/10/03 00:55:44Z  brandont
#	Removed dlfatread.h and dlrelread.h from the global include list.
#	Revision 1.15  2007/09/28 00:05:40Z  jeremys
#	Added new headers and libraries to the DCLHEADERS and DCLLIBLST lists.
#	Revision 1.14  2007/09/26 22:07:42Z  jeremys
#	Added the Datalight Loader libraries to the DCLLIBS list.
#	Revision 1.13  2007/04/25 22:23:03Z  Garyp
#	Updated to accommodate ToolSets where quoted include paths cannot
#	be used.
#	Revision 1.12  2007/04/05 19:57:56Z  Garyp
#	Enhanced the previous rev to work properly when using a ToolSet which
#	used response files.
#	Revision 1.11  2007/04/05 02:35:27Z  Garyp
#	Per Keith -- updated to work with P_TOOLROOT paths which include spaces.
#	Revision 1.10  2007/01/01 22:35:01Z  Garyp
#	Added dlperflog.lib to the standard library list.
#	Revision 1.9  2006/12/13 04:26:46Z  Garyp
#	Updated to delete any existing libraries hanging around when cleaning
#	up "excluded" files.
#	Revision 1.8  2006/12/08 03:09:21Z  Garyp
#	Updated to define D_CPUFAMILY and D_CPUTYPE on the command-line as
#	standard symbols for all builds.
#	Revision 1.7  2006/08/28 02:15:19Z  Garyp
#	Updated to include shredir.tmp if it exists.  Modified the B_COPY
#	command to use a CMD shell so it will work right if GNU MAKE thinks it
#	is SH to which it is sending commands.
#	Revision 1.6  2006/08/15 19:52:37Z  Garyp
#	Added dlcpunum.h and dlenv.h to the list of standard headers.
#	Revision 1.5  2006/04/13 17:35:00Z  Pauli
#	Added tools library.
#	Revision 1.4  2006/01/10 21:38:05Z  Garyp
#	Removed dlostest from the standard list of libraries.
#	Revision 1.3  2006/01/04 02:08:53Z  Garyp
#	Refactored dlapi.h and moved internal DCL functions (those called only
#	by other Datalight code) into dlapiint.h.
#	Revision 1.2  2005/12/28 00:24:40Z  Pauli
#	Added dlend.h to dependency list.
#	Revision 1.1  2005/12/04 00:17:46Z  Pauli
#	Initial revision
#	Revision 1.4  2005/12/04 00:17:45Z  Garyp
#	Un-did the previous change regarding P_OSVER since it seems not to
#	be relevant anymore.
#	Revision 1.3  2005/11/07 14:46:42Z  Garyp
#	Modified so that D_OSVER get changed from "n/a" to zero (as needed).
#	Revision 1.2  2005/10/21 01:31:05Z  garyp
#	Documentation fixes.
#	Revision 1.1  2005/10/03 16:54:18Z  Garyp
#	Initial revision
#	Revision 1.3  2005/09/02 19:24:14Z  Garyp
#	Added some missing header dependencies.  Added common logic to support
#	exclusion handling.
#	Revision 1.2  2005/08/21 09:38:54Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/07/20 01:36:26Z  pauli
#	Initial revision
#	Revision 1.5  2005/06/14 20:27:50Z  PaulI
#	Removed dependency on dclconf.h, which is not yet defined.
#	Revision 1.4  2005/06/13 02:22:54Z  PaulI
#	Added support for dcl lib prefix.
#	Revision 1.3  2005/04/10 22:06:46Z  PaulI
#	XP Merge
#	Revision 1.2.1.2  2005/04/10 22:06:46Z  garyp
#	Added new headers as well as dlclib.lib and dlosserv.lib.
#	Revision 1.2  2005/01/26 01:56:40Z  GaryP
#	Fixed to use a renamed header.
#	Revision 1.1  2005/01/17 01:59:00Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

ifndef DCL_MAK_INCLUDED
DCL_MAK_INCLUDED = 1


#--------------------------------------------------------------------
# 	Ensure that GNU Make will use cmd.exe rather than some sh
#	it might find in the path.
#--------------------------------------------------------------------
SHELL=cmd.exe


#--------------------------------------------------------------------
#	Include	the proper ToolSet
#--------------------------------------------------------------------
include	$(P_DCLROOT)\host\win32\$(P_TOOLSET)\toolset.mak


#--------------------------------------------------------------------
#	Miscellaneous generic settings
#--------------------------------------------------------------------
ifndef $(B_COPY)
B_COPY=cmd /c copy
COPYCMD=/y
endif

B_DEL =	del


#--------------------------------------------------------------------
#	Common Headers
#
#	This list is a base set of headers which are automatically
#	included by dcl.h.  This list is used to allow a base set
#	of dependencies to be specified in the make files.
#
#	ToDo: Headers which are only optionally included by the
#	      target source code should not be in this list.
#--------------------------------------------------------------------
DCLHEADERS =	$(P_DCLROOT)\include\dcl.h			\
		$(P_DCLROOT)\include\dlapi.h			\
		$(P_DCLROOT)\include\dlapiint.h			\
		$(P_DCLROOT)\include\dlapimap.h			\
		$(P_DCLROOT)\include\dlassert.h			\
		$(P_DCLROOT)\include\dlenv.h			\
		$(P_DCLROOT)\include\dlmacros.h			\
		$(P_DCLROOT)\include\dltlset.h			\
		$(P_DCLROOT)\include\dltlinit.h			\
		$(P_DCLROOT)\include\dltrace.h			\
		$(P_DCLROOT)\include\dlprod.h			\
		$(P_DCLROOT)\include\dlprof.h			\
		$(P_DCLROOT)\include\dlstatus.h			\
		$(P_DCLROOT)\include\dlend.h			\
		$(P_DCLROOT)\include\dl64bit.h			\
		$(P_DCLROOT)\include\dliosys.h			\
		$(P_DCLROOT)\include\dlfatapi.h			\
		$(P_DCLROOT)\include\dlpartid.h			\
		$(P_DCLROOT)\include\dlrelapi.h			\
		$(P_DCLROOT)\include\dlrelfs.h			\
		$(P_DCLROOT)\include\dltestfw.h			\
		$(P_DCLROOT)\include\dlmemval.h			\
		$(P_DCLROOT)\os\$(P_OS)\include\dlosconf.h	\
		$(P_PROJDIR)\dclconf.h


#--------------------------------------------------------------------
#	Determine the CPU libraries to link
#--------------------------------------------------------------------

# B_CPUCODE must be initialized (typically this is done in
# makeall.bat).  The following values are used:
#
# B_CPUCODE = 0	No CPU libraries are built or linked
# B_CPUCODE = 1	Only the CPU family library is used
# B_CPUCODE = 2	Both the CPU family and	type libraries are used

ifeq ($(B_CPUCODE),0)
DLCPULIBS =
endif

ifeq ($(B_CPUCODE),1)
DLCPULIBS = $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlcpufam.$(B_LIBEXT)
endif

ifeq ($(B_CPUCODE),2)
DLCPULIBS = $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlcputyp.$(B_LIBEXT) \
	    $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlcpufam.$(B_LIBEXT)
endif


#--------------------------------------------------------------------
#	Define the library set to use, and the link order
#--------------------------------------------------------------------
DCLLIBS =	$(DLCPULIBS)						\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlproj.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlclib.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dldebug.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlserv.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlosserv.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlperflog.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltests.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlfatcmn.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlrelcmn.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlloader.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltloader.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlfatrdr.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlrelrdr.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltestfw.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlftpfs.$(B_LIBEXT)


# NOTE:	The order in which the CPU libraries are specified is
#	important.  If used, DLCPUTYP must be specified before
#	DLCPUFAM.

DCLLIBLST : $(DCLLIBS)
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlproj.$(B_LIBEXT)    $(B_LIBADDCMD)  >$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlclib.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dldebug.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlserv.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlosserv.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlperflog.$(B_LIBEXT) $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltests.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltools.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlutil.$(B_LIBEXT)    $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlfatcmn.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlrelcmn.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlloader.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltloader.$(B_LIBEXT) $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlfatrdr.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlrelrdr.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dltestfw.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlftpfs.$(B_LIBEXT)   $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
ifeq ($(B_CPUCODE),1)
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlcpufam.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
endif
ifeq ($(B_CPUCODE),2)
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlcputyp.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
	@echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlcpufam.$(B_LIBEXT)  $(B_LIBADDCMD) >>$(P_PROJDIR)\dcllib.lnk
endif


#--------------------------------------------------------------------
#	This section implements the generalized defined symbols that
#	are set for all modules compiled.
#--------------------------------------------------------------------
B_SYMBOLS    = D_PRODUCTNUM=$(P_PRODUCTNUM)
B_SYMBOLS   += D_PRODUCTVER=$(P_VERSIONVAL)
B_SYMBOLS   += D_PRODUCTBUILD=$(P_BUILDNUM)
ifdef P_SUBPRODUCTNUM
  B_SYMBOLS += D_SUBPRODUCTNUM=$(P_SUBPRODUCTNUM)
endif
B_SYMBOLS   += D_TOOLSETNUM=$(B_TOOLSETNUM)
B_SYMBOLS   += D_CPUFAMILY=D_CPUFAMILY_$(P_CPUFAMILY)
B_SYMBOLS   += D_CPUTYPE=$(P_CPUTYPE)
B_SYMBOLS   += D_DEBUG=$(B_DEBUG)
B_SYMBOLS   += D_OSVER=$(P_OSVER)
ifdef B_LOCALDEFINES
  B_SYMBOLS += $(B_LOCALDEFINES)
endif
ifdef B_OSDEFINES
ifdef B_NEEDSOSDEFINES
  B_SYMBOLS += $(B_OSDEFINES)
endif
endif
ifdef B_TOOLSETDEFINES
  B_SYMBOLS += $(B_TOOLSETDEFINES)
endif

#----------------------------------------------------------
# If B_CRESNAME is defined, then we are using response
# files to hold the various compiler flags.
#
# If B_CRESNAME is NOT defined, then we use the B_CDEFS
# variable to store the various defines.
#----------------------------------------------------------
ifdef B_CRESNAME

.PHONY: $(B_SYMBOLS)

$(B_SYMBOLS):
ifdef B_ASMDEFCMD
	@echo $(B_ASMDEFCMD)$@				>>$(B_ARESNAME)
endif
ifdef B_CDEFCMD
	@echo $(B_CDEFCMD)$@				>>$(B_CRESNAME)
endif

GENERIC_DEFINES: $(B_SYMBOLS)

else	# B_CRESNAME is NOT defined

GENERIC_DEFINES:

ifdef B_ASMDEFCMD
  B_ASMDEFS =
  B_ASMDEFS := $(foreach path,$(B_SYMBOLS),$(B_ASMDEFCMD)$(path))
endif

ifdef B_CDEFCMD
  B_CDEFS =
  B_CDEFS := $(foreach path,$(B_SYMBOLS),$(B_CDEFCMD)$(path))
endif

endif


#--------------------------------------------------------------------
#	This section implements the generalized include directory
#	hierarchy for the build system.
#
#	The general hierarchy is designed to move from more specific
# 	to the more generalized.
#
#	The following is a general description:
#
#	1) The current directory, which is handled implicitly
#	2) The project directory
#	3) The OS include directory for the Datalight product
#	4) The general include directory for the Datalight product
#	5) The DCL OS include directory (if different)
#	6) The DCL general include directory (if different)
#	7) The B_LOCALINC directory as defined in the local make file
#	8) Any C-Library include paths -- typically defined by the
#	   OS, if any
#	9) Any ToolSet specific include path (if used, B_TOOLINC may
#	   only be a single directory, and it must be relative to the
#	   B_TOOLROOT value
#      10) Any OS specific directories, if the B_NEEDSOSHEADERS
#	   variable is defined
#
# NOTE: The B_CLIBINC and P_TOOLROOT\B_TOOLINC paths below are the
#       only include paths where long directory names which contain
#       spaces are allowed, and therefore they are double quoted.
#	B_OSINC may contain multiple paths, separated by spaces,
#       and therefore any of those paths may not included embedded
#	spaces.
#--------------------------------------------------------------------

B_INCPATHS   =  $(P_PROJDIR)
B_INCPATHS   += $(P_ROOT)\os\$(P_OS)\include
B_INCPATHS   += $(P_ROOT)\include
ifneq ($(P_ROOT), $(P_DCLROOT))
  B_INCPATHS += $(P_DCLROOT)\os\$(P_OS)\include
  B_INCPATHS += $(P_DCLROOT)\include
endif
ifdef B_LOCALINC
  B_INCPATHS += $(B_LOCALINC)
endif
ifdef B_CLIBINC
ifdef B_NOQUOTEDINCPATHS
  B_INCPATHS += $(B_CLIBINC)
else
  B_INCPATHS += "$(B_CLIBINC)"
endif
endif
ifdef B_TOOLINC
ifdef B_NOQUOTEDINCPATHS
  B_INCPATHS += $(P_TOOLROOT)\$(B_TOOLINC)
else
  B_INCPATHS += "$(P_TOOLROOT)\$(B_TOOLINC)"
endif
endif
ifdef B_OSINC
ifdef B_NEEDSOSHEADERS
  B_INCPATHS += $(B_OSINC)
endif
endif

#----------------------------------------------------------
# If B_CRESNAME is defined, then we are using response
# files to hold the various compiler flags.
#
# If B_CRESNAME is NOT defined, then we use the B_ASMINCS
# and B_CINCS variables to store the various include paths.
#----------------------------------------------------------

ifdef B_ASMINCCMD
  B_ASMINCS =
  B_ASMINCS := $(foreach inc, $(B_INCPATHS), $(addprefix $(B_ASMINCCMD),$(inc)))
endif

ifdef B_CINCCMD
  B_CINCS =
  B_CINCS := $(foreach inc, $(B_INCPATHS), $(addprefix $(B_CINCCMD),$(inc)))
endif

GENERIC_INCLUDES:
ifdef B_CRESNAME
	@echo $(B_ASMINCS) >>$(B_ARESNAME)
	@echo $(B_CINCS)   >>$(B_CRESNAME)
endif


#--------------------------------------------------------------------
#	This section contains helper logic for the exclude process.
#
#	The DCLCLEANEXCLUDE target is used to delete any old object
#	code for modules that may now be overridden by code in the
# 	Project Directory, in the event that a new module was added
#	and a generic clean was not performed.
#
#	Not only must this delete any object code hanging around, but
#	it must delete any existing libraries as well, including not
#	only those in the current build directory, but those in the
#	target directory, referenced by TARGNAME.
#
#	Make files that use this logic must define TARGNAME to be the
#	target library name, and EXCLUSIONDEL to the list of modules
#	to be excluded, sans the file extension.
#--------------------------------------------------------------------
DCLCLEANEXCLUDE : $(EXCLUSIONDEL)

$(EXCLUSIONDEL):
	if exist $@.$(B_OBJEXT) if exist *.$(B_LIBEXT) del *.$(B_LIBEXT)
	if exist $@.$(B_OBJEXT) if exist $(TARGNAME) del $(TARGNAME)
	if exist $@.$(B_OBJEXT) del $@.$(B_OBJEXT)
	if exist $@.err del $@.err




endif	# DCL_MAK_INCLUDED

