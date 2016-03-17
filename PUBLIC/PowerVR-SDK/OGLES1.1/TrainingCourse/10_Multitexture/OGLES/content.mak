#--------------------------------------------------------------------------
# Name         : content.mak
# Title        : Makefile to build content files
# Author       : Auto-generated
#
# Copyright    : 2007 by Imagination Technologies.  All rights reserved.
#              : No part of this software, either material or conceptual 
#              : may be copied or distributed, transmitted, transcribed,
#              : stored in a retrieval system or translated into any 
#              : human or computer language in any form by any means,
#              : electronic, mechanical, manual or other-wise, or 
#              : disclosed to third parties without the express written
#              : permission of VideoLogic Limited, Unit 8, HomePark
#              : Industrial Estate, King's Langley, Hertfordshire,
#              : WD4 8LZ, U.K.
#
# Description  : Makefile to build content files for demos in the PowerVR SDK
#
# Platform     :
#
# $Revision: 1.2 $
#--------------------------------------------------------------------------

#############################################################################
## Variables
#############################################################################

PVRTEXTOOL 	= ..\..\..\Utilities\PVRTexTool\PVRTexToolCL\Win32\PVRTexTool.exe
FILEWRAP 	= ..\..\..\Utilities\Filewrap\Win32\Filewrap.exe


MEDIAPATH = ../Media
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

TEXTURES = \
	Crate.pvr \
	Stamp.pvr \
	Stampnm.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Crate.cpp \
	$(CONTENTDIR)/Stamp.cpp \
	$(CONTENTDIR)/Stampnm.cpp

all: resources 
	
help:
	@echo Valid targets are:
	@echo resources, textures, binary_shaders, clean
	@echo PVRTEXTOOL, FILEWRAP and VGPCOMPILER can be used to override the 
	@echo default paths to these utilities.

clean:
	-rm $(TEXTURES)
	-rm $(RESOURCES)
	
	
resources: 		$(CONTENTDIR) $(RESOURCES)
textures: 		$(TEXTURES)


$(CONTENTDIR):
	-mkdir $@

############################################################################
# Create textures
############################################################################

Crate.pvr: $(MEDIAPATH)/crate.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/crate.bmp -o$@

Stamp.pvr: $(MEDIAPATH)/stamp.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/stamp.bmp -o$@

Stampnm.pvr: $(MEDIAPATH)/stamp-nm.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/stamp-nm.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Crate.cpp: Crate.pvr
	$(FILEWRAP)  -o $@ Crate.pvr

$(CONTENTDIR)/Stamp.cpp: Stamp.pvr
	$(FILEWRAP)  -o $@ Stamp.pvr

$(CONTENTDIR)/Stampnm.cpp: Stampnm.pvr
	$(FILEWRAP)  -o $@ Stampnm.pvr


############################################################################
# End of file (content.mak)
############################################################################
