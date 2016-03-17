#----------------------------------------------------------------------------
#                       Description
#
#   This make file controls the generation of the FXNAND library.  It is
#   designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#               Revision History
#   $Log: nand.mak $
#   Revision 1.52  2011/11/28 22:22:22Z  jimmb
#   enable the MX35 and MX51 NTMs
#   Revision 1.51  2011/11/22 17:35:12Z  glenns
#   Add support for standard and enhanced ClearNAND.
#   Revision 1.50  2011/04/06 20:07:29Z  jimmb
#   Updated to exclude non-verified NTMs
#   Revision 1.49  2011/02/09 02:52:21Z  garyp
#   Don't try building the nttegra2 or ntmx51 NTMs for the time being.
#   Revision 1.48  2010/09/28 21:25:20Z  glenns
#   Back out previous change pending product release.
#   Revision 1.47  2010/08/05 21:24:38Z  glenns
#   Add dependency for [TRADE NAME TBA] NTM.
#   Revision 1.46  2010/06/19 17:01:02Z  garyp
#   Updated for a relocated ntmicron.h.
#   Revision 1.45  2009/12/11 20:39:33Z  garyp
#   Eliminated nthelp.h.
#   Revision 1.44  2009/12/03 21:52:35Z  garyp
#   Updated dependencies.
#   Revision 1.43  2009/11/11 00:45:09Z  garyp
#   Removed the invalid dependency for ntads5121e.c.  Added a missing
#   dependency for ntpecchelp.c.
#   Revision 1.42  2009/10/07 17:43:39Z  garyp
#   Eliminated the use of ntspare.h.  Replaced with fxnandapi.h.
#   Revision 1.41  2009/08/31 17:06:27Z  jimmb
#   Removed the WindRiver specific NTM from the build.
#   Revision 1.40  2009/08/28 13:16:20Z  jimmb
#   Removed the VxWork specific NTM from the build process
#   Revision 1.39  2009/08/27 17:45:43Z  jimmb
#   Added support for the ads5121 (supplied by WindRiver)
#   Revision 1.38  2009/08/07 19:39:04Z  garyp
#   Fixed an include path to work properly in a 4GR build environment.
#   Revision 1.37  2009/08/04 18:45:47Z  garyp
#   Merged from the v4.0 branch.  Added several modules.
#   Revision 1.36  2009/07/24 18:50:24Z  garyp
#   Merged from the v4.0 branch.  Include the "hardware" directory in the
#   include path.
#   Revision 1.35  2009/04/09 21:26:35Z  garyp
#   Removed support for the Atlas NTM.
#   Revision 1.34  2009/03/23 19:43:26Z  keithg
#   Fixed bug 2509, added module nthelptags.c
#   Revision 1.33  2009/02/06 08:31:40Z  keithg
#   Updated to reflect new location of NAND header files and macros,.
#   Revision 1.32  2008/11/07 07:08:03  johnb
#   Cleaned up whitespace alignment
#   Revision 1.31  2008/07/23 18:24:42Z  keithg
#   Clarified comments with professional customer facing terms.
#   Revision 1.30  2008/05/05 20:17:58Z  garyp
#   Updated a dependency.
#   Revision 1.29  2008/03/23 18:11:45Z  Garyp
#   Enabled the LSI and Atlas NTMs.
#   Revision 1.28  2008/01/30 23:13:51Z  Garyp
#   Removed ntomap.c.
#   Revision 1.27  2008/01/30 02:26:14Z  Garyp
#   Removed ntbyteio.c and ntmx21.c.  Temporarily prevent ntlsi.c, ntomap.c,
#   and ntatlas.c from compiling.
#   Revision 1.26  2007/09/12 19:56:30Z  Garyp
#   Added ntmicron.c.
#   Revision 1.25  2007/02/21 23:50:44Z  rickc
#   Renamed pxa290 to pxa320
#   Revision 1.24  2006/11/03 01:02:53Z  billr
#   Build ntpxa290.c.
#   Revision 1.23  2006/08/30 21:20:46Z  Garyp
#   Added dependencies for the MX21 NTM.
#   Revision 1.22  2006/07/26 20:36:56Z  Pauli
#   Added NAND Simulator NTM.
#   Revision 1.21  2006/07/10 22:38:43Z  Garyp
#   Renamed fhcadio to fhcad.
#   Revision 1.20  2006/06/16 14:30:18Z  johnb
#   Added LSI NTM to list of NTMs.
#   Revision 1.19  2006/05/08 22:10:55  timothyj
#   Added dependancies for ntcadio.c
#   Revision 1.18  2006/05/01 23:44:16Z  DeanG
#   Revise to new makefile policy:  build all .C files in directory by default.
#   Revision 1.17  2006/04/12 16:36:40Z  billr
#   The MX21 NTM has been removed, The MX31 NTM is intended to be a permanent
#   replacement for it.
#   Revision 1.16  2006/03/15 00:59:18Z  rickc
#   Added include path for building with Windows CE Platform Builder
#   Revision 1.15  2006/03/12 22:16:40Z  Garyp
#   Added ntatlas.c and fhpageio_file.c.
#   Revision 1.14  2006/03/07 20:44:10Z  Garyp
#   Re-enabled the omap and pageio NTMs.
#   Revision 1.13  2006/03/07 01:03:51Z  billr
#   Add MX21 NTM.
#   Revision 1.12  2006/02/28 23:51:01Z  Garyp
#   Commented out the omap and pageio NTMs temporarily.
#   Revision 1.11  2006/02/24 04:25:07Z  Garyp
#   Updated to use refactored headers.
#   Revision 1.10  2006/02/21 18:08:28Z  Garyp
#   Added ntomap.c.
#   Revision 1.9  2006/02/18 05:25:37Z  Garyp
#   Enabled ntpageio.c.
#   Revision 1.8  2006/02/17 21:26:54Z  timothyj
#   Re-added OneNAND NTM (was removed for Alpha shipment).
#   Revision 1.7  2006/02/14 22:03:40Z  Garyp
#   Enabled ntbyteio.c.
#   Revision 1.6  2006/02/14 03:31:51Z  Pauli
#   Removed reference to oem.h.
#   Revision 1.5  2006/02/10 23:35:41Z  Garyp
#   Commented out nt1nand.c.
#   Revision 1.4  2006/02/09 22:27:40Z  Garyp
#   Temporarility disabled building ntbyteio and ntpageio.
#   Revision 1.3  2006/01/24 23:59:43Z  timothyj
#   Added reference to OneNAND NTM
#   Revision 1.2  2005/12/15 06:48:51Z  garyp
#   Fixed to would properly with Windows CE Platform Builder.
#   Revision 1.1  2005/12/02 01:16:28Z  Pauli
#   Initial revision
#   Revision 1.3  2005/12/02 01:16:28Z  Garyp
#   Added a number of new modules and headers.
#   Revision 1.2  2005/11/01 23:33:14Z  Garyp
#   Added nthelp.c.
#   Revision 1.1  2005/10/14 03:08:32Z  Garyp
#   Initial revision
#   Revision 1.3  2005/07/29 20:57:18Z  Garyp
#   Renamed the "pinio" NTD to "byteio".
#   Revision 1.2  2005/07/29 16:40:42Z  Garyp
#   Added ntpageio.c.
#   Revision 1.1  2005/07/28 03:43:10Z  pauli
#   Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#   Settings
#--------------------------------------------------------------------

# Define extra general include paths
B_LOCALINC = 	$(P_ROOT)\fmsl\include			\
		$(P_DCLROOT)\include\hardware

TARGFILE = fxnand.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

NANDHEADERS =	$(FXHEADERS) 			\
		nand.h 				\
		..\include\fxnandapi.h		\
		..\..\include\nandid.h 		\
		..\..\include\nandsim.h		\
		..\..\include\fxiosys.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, excluding those in
#	the EXCLUDE list, and then generate the object file list.
#--------------------------------------------------------------------

EXCLUDE := nttegra2.c
ALLSRC  := $(wildcard *.c)
NANDSRC := $(filter-out $(EXCLUDE), $(ALLSRC))
NANDOBJ := $(NANDSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#   Targets
#--------------------------------------------------------------------

NANDTARGETS : NANDDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#   Default Project Rules
#--------------------------------------------------------------------

include $(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#   Build Commands
#--------------------------------------------------------------------

NANDDIR :
	@echo Processing FMSL\NAND...

$(TARGNAME) : nand.mak $(NANDOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#   Dependencies
#--------------------------------------------------------------------

# Miscellaneous stuff
#
nand.$(B_OBJEXT)             : $(NANDHEADERS) nand.c
nandid.$(B_OBJEXT)           : $(NANDHEADERS) nandid.c
nandparampage.$(B_OBJEXT)    : $(NANDHEADERS) nandparampage.c
nthelp.$(B_OBJEXT)           : $(NANDHEADERS) nthelp.c
nthelpoffsetzero.$(B_OBJEXT) : $(NANDHEADERS) nthelpoffsetzero.c
nthelpssfdc.$(B_OBJEXT)      : $(NANDHEADERS) nthelpssfdc.c
nthelptags.$(B_OBJEXT)       : $(NANDHEADERS) nthelptags.c
ntlegacy.$(B_OBJEXT)         : $(NANDHEADERS) ntlegacy.c
ntpecchelp.$(B_OBJEXT)       : $(NANDHEADERS) ntpecchelp.c
fhpageio_file.$(B_OBJEXT)    : $(NANDHEADERS) fhpageio_file.c fhpageio_file.h

# NTMs in alphabetical order
#
nt1nand.$(B_OBJEXT)          : $(NANDHEADERS) nt1nand.c       ntm.h ..\include\deverr.h
ntcad.$(B_OBJEXT)            : $(NANDHEADERS) ntcad.c         ntm.h ..\include\deverr.h
nteclrnand.$(B_OBJEXT)       : $(NANDHEADERS) nteclrnand.c    ntm.h ..\include\deverr.h
ntflex1nand.$(B_OBJEXT)      : $(NANDHEADERS) ntflex1nand.c   ntm.h ..\include\deverr.h
ntlsi.$(B_OBJEXT)            : $(NANDHEADERS) ntlsi.c         ntm.h ..\include\deverr.h
ntmicron.$(B_OBJEXT)         : $(NANDHEADERS) ntmicron.c      ntm.h ..\include\deverr.h ..\include\ntmicron.h
ntmx31.$(B_OBJEXT)           : $(NANDHEADERS) ntmx31.c        ntm.h ..\include\deverr.h
ntpageio.$(B_OBJEXT)         : $(NANDHEADERS) ntpageio.c      ntm.h ..\include\deverr.h fhpageio_file.h
ntpxa320.$(B_OBJEXT)         : $(NANDHEADERS) ntpxa320.c      ntm.h ..\include\deverr.h $(P_ROOT)/fmsl/include/fhpxa320.h
ntram.$(B_OBJEXT)            : $(NANDHEADERS) ntram.c         ntm.h ..\include\deverr.h
ntsim.$(B_OBJEXT)            : $(NANDHEADERS) ntsim.c         ntm.h ..\include\deverr.h
ntstdclrnand.$(B_OBJEXT)     : $(NANDHEADERS) ntstdclrnand.c  ntm.h ..\include\deverr.h


#--------------------------------------------------------------------
#   Special Targets for Windows CE Platform Builder only
#--------------------------------------------------------------------

mswcepb : NANDDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=fxnand					>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(NANDSRC)				>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources

