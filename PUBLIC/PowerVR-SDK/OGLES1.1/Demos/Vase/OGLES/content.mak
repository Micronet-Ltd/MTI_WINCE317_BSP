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
	Flora.pvr \
	Backgrnd.pvr \
	Reflection.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Flora.cpp \
	$(CONTENTDIR)/Backgrnd.cpp \
	$(CONTENTDIR)/Reflection.cpp \
	$(CONTENTDIR)/Vase_float.cpp \
	$(CONTENTDIR)/Vase_fixed.cpp

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

Flora.pvr: $(MEDIAPATH)/Flora.bmp $(MEDIAPATH)/TFlora.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Flora.bmp -a$(MEDIAPATH)/TFlora.bmp -o$@

Backgrnd.pvr: $(MEDIAPATH)/Backgrnd.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Backgrnd.bmp -o$@

Reflection.pvr: $(MEDIAPATH)/Reflection.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Reflection.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Flora.cpp: Flora.pvr
	$(FILEWRAP)  -o $@ Flora.pvr

$(CONTENTDIR)/Backgrnd.cpp: Backgrnd.pvr
	$(FILEWRAP)  -o $@ Backgrnd.pvr

$(CONTENTDIR)/Reflection.cpp: Reflection.pvr
	$(FILEWRAP)  -o $@ Reflection.pvr

$(CONTENTDIR)/Vase_float.cpp: Vase_float.pod
	$(FILEWRAP)  -o $@ Vase_float.pod

$(CONTENTDIR)/Vase_fixed.cpp: Vase_fixed.pod
	$(FILEWRAP)  -o $@ Vase_fixed.pod


############################################################################
# End of file (content.mak)
############################################################################
