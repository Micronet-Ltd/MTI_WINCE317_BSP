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
	Granite.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Mesh_float.cpp \
	$(CONTENTDIR)/Mesh_fixed.cpp \
	$(CONTENTDIR)/Granite.cpp

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

Granite.pvr: $(MEDIAPATH)/Granite.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Granite.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Mesh_float.cpp: Mesh_float.pod
	$(FILEWRAP)  -o $@ Mesh_float.pod

$(CONTENTDIR)/Mesh_fixed.cpp: Mesh_fixed.pod
	$(FILEWRAP)  -o $@ Mesh_fixed.pod

$(CONTENTDIR)/Granite.cpp: Granite.pvr
	$(FILEWRAP)  -o $@ Granite.pvr


############################################################################
# End of file (content.mak)
############################################################################
