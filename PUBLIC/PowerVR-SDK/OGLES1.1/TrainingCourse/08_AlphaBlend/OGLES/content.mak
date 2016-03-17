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
	Background.pvr \
	Foreground.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Background.cpp \
	$(CONTENTDIR)/Foreground.cpp

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

Background.pvr: $(MEDIAPATH)/backgrnd.bmp
	$(PVRTEXTOOL) -nt -m -fOGL888 -i$(MEDIAPATH)/backgrnd.bmp -o$@

Foreground.pvr: $(MEDIAPATH)/forgrnd.bmp $(MEDIAPATH)/tforgrnd.bmp
	$(PVRTEXTOOL) -nt -m -fOGL8888 -i$(MEDIAPATH)/forgrnd.bmp -a$(MEDIAPATH)/tforgrnd.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Background.cpp: Background.pvr
	$(FILEWRAP)  -o $@ Background.pvr

$(CONTENTDIR)/Foreground.cpp: Foreground.pvr
	$(FILEWRAP)  -o $@ Foreground.pvr


############################################################################
# End of file (content.mak)
############################################################################
