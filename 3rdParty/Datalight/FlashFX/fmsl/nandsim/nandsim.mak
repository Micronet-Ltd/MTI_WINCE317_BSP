#----------------------------------------------------------------------------
#                 		Description
#
#	This make file controls	the generation of the FXNNDSIM library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: nandsim.mak $
#	Revision 1.12  2009/12/11 22:17:03Z  garyp
#	Eliminated nthelp.h.
#	Revision 1.11  2009/10/07 17:34:23Z  garyp
#	Eliminated the use of ntspare.h.  Replaced with fxnandapi.h.
#	Revision 1.10  2009/02/09 02:36:08Z  garyp
#	Added a dependency for the missing nsprealmodel.c.
#	Revision 1.9  2009/02/06 02:19:16Z  keithg
#	Updated to reflect new location of NAND header files and macros,.
#	Revision 1.8  2008/11/07 15:08:21Z  johnb
#	Cleaned up whitespace alignment
#	Revision 1.7  2007/09/13 22:06:34Z  Garyp
#	Formatting cleanup -- no functional changes.
#	Revision 1.6  2007/07/24 19:11:41Z  pauli
#	Removed dependency on ntm.h.  Only the Simulator NTM needs it.
#	Revision 1.5  2007/06/05 19:48:16Z  rickc
#	Hardcode B_INCLUDES instead of using B_LOCALINC to deal Platform
#	Builder's need for semi-colon spacing between paths.
#	Revision 1.4  2007/05/10 00:58:31Z  rickc
#	Fixed build issue in Platform Builder
#	Revision 1.3  2006/12/05 22:44:56Z  DeanG
#	Add table-driven error policy module
#	Revision 1.2  2006/07/27 22:43:14Z  DeanG
#	Changes to add "random" policy module
#	Revision 1.1  2006/07/26 20:43:56Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
# 	NOTE: B_LOCALINC is not used below in the CE Platform Builder
#	section below as it is in most of the .mak files due to the
#	need in PB for semi-colon spacers between paths.  Therefore,
#	changes to B_LOCALINC will also need to be made below in
#	B_INCLUDES.
B_LOCALINC = $(P_ROOT)\fmsl\include $(P_ROOT)\fmsl\nand

TARGFILE = fxnndsim.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

NANDSIMHEADERS =	$(FXHEADERS)			\
			..\nand\nand.h			\
			..\include\fxnandapi.h		\
			..\..\include\fxiosys.h		\
			..\..\include\nandid.h 		\
			..\..\include\nandsim.h

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

EXCLUDE		:=
ALLSRC		:= $(wildcard *.c)
NANDSIMSRC	:= $(filter-out $(EXCLUDE), $(ALLSRC))
NANDSIMOBJ	:= $(NANDSIMSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

NANDSIMTARGETS : NANDSIMDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

NANDSIMDIR :
	@echo Processing FMSL\NANDSIM...

$(TARGNAME) : nandsim.mak $(NANDSIMOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

nsmech.$(B_OBJEXT)	 : $(NANDSIMHEADERS) nsmech.c
nspnull.$(B_OBJEXT)	 : $(NANDSIMHEADERS) nspnull.c
nsprand.$(B_OBJEXT)	 : $(NANDSIMHEADERS) nsprand.c
nsprealmodel.$(B_OBJEXT) : $(NANDSIMHEADERS) nsprealmodel.c
nsptable.$(B_OBJEXT)	 : $(NANDSIMHEADERS) nsptable.c
nssfile.$(B_OBJEXT)	 : $(NANDSIMHEADERS) nssfile.c
nssram.$(B_OBJEXT)	 : $(NANDSIMHEADERS) nssram.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: NANDSIMDIR TOOLSETINIT
	@echo #								 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		>>sources
	@echo #								>>sources
	@echo RELEASETYPE=PLATFORM					>>sources
	@echo TARGETNAME=fxnndsim					>>sources
	@echo TARGETTYPE=LIBRARY					>>sources
	@echo TARGETLIBS=						>>sources
	@echo EXEENTRY=							>>sources
	@echo SOURCES=$(NANDSIMSRC)					>>sources
	@echo B_INCLUDES=$(P_ROOT)\fmsl\include;$(P_ROOT)\fmsl\nand;	>>sources

