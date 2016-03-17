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
	Head_clonespacePVRTC.pvr \
	Head_clonespaceBGRA.pvr \
	Head_diffuse.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Head_clonespacePVRTC.cpp \
	$(CONTENTDIR)/Head_clonespaceBGRA.cpp \
	$(CONTENTDIR)/Head_diffuse.cpp \
	$(CONTENTDIR)/Head_float.cpp \
	$(CONTENTDIR)/Head_fixed.cpp

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

Head_clonespacePVRTC.pvr: $(MEDIAPATH)/Head_clonespace.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Head_clonespace.bmp -o$@

Head_clonespaceBGRA.pvr: $(MEDIAPATH)/Head_clonespace.bmp
	$(PVRTEXTOOL) -nt -m -fOGLBGRA8888 -i$(MEDIAPATH)/Head_clonespace.bmp -o$@

Head_diffuse.pvr: $(MEDIAPATH)/Head_diffuse.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Head_diffuse.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Head_clonespacePVRTC.cpp: Head_clonespacePVRTC.pvr
	$(FILEWRAP)  -o $@ Head_clonespacePVRTC.pvr

$(CONTENTDIR)/Head_clonespaceBGRA.cpp: Head_clonespaceBGRA.pvr
	$(FILEWRAP)  -o $@ Head_clonespaceBGRA.pvr

$(CONTENTDIR)/Head_diffuse.cpp: Head_diffuse.pvr
	$(FILEWRAP)  -o $@ Head_diffuse.pvr

$(CONTENTDIR)/Head_float.cpp: Head_float.pod
	$(FILEWRAP)  -o $@ Head_float.pod

$(CONTENTDIR)/Head_fixed.cpp: Head_fixed.pod
	$(FILEWRAP)  -o $@ Head_fixed.pod


############################################################################
# End of file (content.mak)
############################################################################
