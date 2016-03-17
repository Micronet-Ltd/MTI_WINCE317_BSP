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
	tex_base.pvr \
	tex_arm.pvr

	

RESOURCES = \
	$(CONTENTDIR)/tex_base.cpp \
	$(CONTENTDIR)/tex_arm.cpp \
	$(CONTENTDIR)/IntroducingPOD_float.cpp \
	$(CONTENTDIR)/IntroducingPOD_fixed.cpp

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

tex_base.pvr: $(MEDIAPATH)/tex_base.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/tex_base.bmp -o$@

tex_arm.pvr: $(MEDIAPATH)/tex_arm.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/tex_arm.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/tex_base.cpp: tex_base.pvr
	$(FILEWRAP)  -o $@ tex_base.pvr

$(CONTENTDIR)/tex_arm.cpp: tex_arm.pvr
	$(FILEWRAP)  -o $@ tex_arm.pvr

$(CONTENTDIR)/IntroducingPOD_float.cpp: IntroducingPOD_float.pod
	$(FILEWRAP)  -o $@ IntroducingPOD_float.pod

$(CONTENTDIR)/IntroducingPOD_fixed.cpp: IntroducingPOD_fixed.pod
	$(FILEWRAP)  -o $@ IntroducingPOD_fixed.pod


############################################################################
# End of file (content.mak)
############################################################################
