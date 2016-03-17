#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE device
#	driver.	 It is designed	for use	with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: sxipdll.mak $
#	Revision 1.5  2012/02/17 16:09:14Z  johnb
#	Changed _PROJECTOAKROOT to _TARGETPLATROOT
#	Revision 1.4  2008/11/10 21:46:36Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.3  2008/11/07 15:12:14Z  johnb
#	Cleaned up whitespace alignment
#	Cleaned up use of B_INCLUDES
#	Revision 1.2  2006/08/28 20:39:30Z  Garyp
#	Enclosed a copy command in quotes so the silly GNU MAKE does not
#	massage the string improperly when it things there is an SH in the system.
#	Revision 1.1  2005/10/01 09:36:30Z  Pauli
#	Initial revision
#	Revision 1.10  2005/01/28 21:20:47Z  GaryP
#	Modified to no longer change into the target directory since that is already
#	handled by the traverse process.  Modified to use the new DCL shared CPU
#	libraries.
#	Revision 1.9  2004/07/07 03:04:49Z  GaryP
#	Eliminated the module header.
#	Revision 1.8  2004/07/01 18:44:50Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.7  2003/05/07 00:04:56Z  garyp
#	Fixed several problems in the SOURCES file generation.
#	Revision 1.6  2003/04/15 17:49:14Z  garyp
#	Minor cleanup.
#	Revision 1.5  2003/03/08 20:16:34Z  garyp
#	Updated	to use the B_LIBEXT setting.
#	Revision 1.4  2002/11/23 00:31:14Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.3  2002/11/15 21:22:50Z  garyp
#	Corrected the previous rev.
#	Revision 1.2  2002/11/15 21:12:14Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.1  2002/11/13 19:13:46Z  qa
#	Initial	revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

TARGNAME = $(B_RELDIR)\fxsxip.dll

FXSXIPHEADERS =	$(FXHEADERS)
FXSXIPINCLUDES =

FXSXIPSRC = intl2x16.c uSA1110.c ioctl.s
FXSXIPOBJ = intl2x16.$(B_OBJEXT) uSA1110.$(B_OBJEXT) ioctl.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

FXSXIPTARGETS :	FXSXIPDIR TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

FXSXIPDIR :
	@echo Processing os\$(P_OS)\SXIPDLL...


$(TARGNAME) : fxsxip.mak fxsxip.dll
	$(B_COPY) fxsxip.dll $(TARGNAME)


fxsxip.dll : $(FXSXIPOBJ)
	@echo /out:fxsxip.dll /dll				 >fxsxip.lnk
	@echo /def:fxsxip.def $(FXSXIPOBJ)			>>fxsxip.lnk
	@echo $(B_RELDIR)\lib\DLCPUFAM.$(B_LIBEXT)		>>fxsxip.lnk
ifneq ($(P_CPUFAMILY), $(P_CPUTYPE))
	@echo $(B_RELDIR)\lib\DLCPUTYP.$(B_LIBEXT)		>>fxsxip.lnk
endif
	$(B_LINK) $(B_LFLAGS) @fxsxip.lnk


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

intl2x16.$(B_OBJEXT)  :	$(FXSXIPHEADERS)  intl2x16.c iJ3A.h sxipplat.h
uSA1110.$(B_OBJEXT)   :	$(FXSXIPHEADERS)  uSA1110.c sxipplat.h
ioctl.$(B_OBJEXT)     :	$(FXSXIPHEADERS)  ioctl.s
sa1110.$(B_OBJEXT)    :	$(FXSXIPHEADERS)  sa1110.s


#-----------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#-----------------------------------------------------------

mswcepb	: FXSXIPDIR TOOLSETINIT
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #									  >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT			 >>sources
	@echo #									 >>sources
	@echo RELEASETYPE=PLATFORM						 >>sources
	@echo TARGETNAME=FXSXIP							 >>sources
	@echo TARGETTYPE=DYNLINK						 >>sources
	@echo TARGETLIBS=							\>>sources
ifneq ($(P_CPUFAMILY), $(P_CPUTYPE))
	@if exist "$(P_ROOT)\dcl\cpu\$(P_CPUFAMILY)\$(P_CPUTYPE)\*.*" echo $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\DLCPUTYP.lib \>>sources
	@if exist "$(P_ROOT)\dcl\cpu\$(P_CPUFAMILY)\*.*"              echo $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\DLCPUFAM.lib  >>sources
else
	@if exist "$(P_ROOT)\dcl\cpu\$(P_CPUFAMILY)\*.*"              echo $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\DLCPUFAM.lib  >>sources
endif
	@echo DLLENTRY=DllMain							 >>sources
	@echo SOURCES=$(FXSXIPSRC)						 >>sources


