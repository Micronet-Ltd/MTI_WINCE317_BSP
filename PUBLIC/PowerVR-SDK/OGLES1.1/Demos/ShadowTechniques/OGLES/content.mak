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
# $Revision: 1.1 $
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
	Blob.pvr \
	TableCover.pvr \
	Kettle.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Scene_fixed.cpp \
	$(CONTENTDIR)/Scene_float.cpp \
	$(CONTENTDIR)/Blob.cpp \
	$(CONTENTDIR)/TableCover.cpp \
	$(CONTENTDIR)/Kettle.cpp

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

Blob.pvr: $(MEDIAPATH)/Blob.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Blob.bmp -o$@

TableCover.pvr: $(MEDIAPATH)/TableCover.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/TableCover.bmp -o$@

Kettle.pvr: $(MEDIAPATH)/kettle.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/kettle.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Scene_fixed.cpp: Scene_fixed.pod
	$(FILEWRAP)  -o $@ Scene_fixed.pod

$(CONTENTDIR)/Scene_float.cpp: Scene_float.pod
	$(FILEWRAP)  -o $@ Scene_float.pod

$(CONTENTDIR)/Blob.cpp: Blob.pvr
	$(FILEWRAP)  -o $@ Blob.pvr

$(CONTENTDIR)/TableCover.cpp: TableCover.pvr
	$(FILEWRAP)  -o $@ TableCover.pvr

$(CONTENTDIR)/Kettle.cpp: Kettle.pvr
	$(FILEWRAP)  -o $@ Kettle.pvr


############################################################################
# End of file (content.mak)
############################################################################
