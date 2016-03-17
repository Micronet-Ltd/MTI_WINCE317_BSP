#
# Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
# ALL RIGHTS RESERVED
#
################################################################################
# Name         : makefile
# Title        : CEBuild makefile for powervr
# Author       : Imagination Technologies
# Created      : 11th May 2004
#
#  Copyright   : 2003-2004 by Imagination Technologies. All rights reserved.
#              : No part of this software, either material or conceptual 
#              : may be copied or distributed, transmitted, transcribed,
#              : stored in a retrieval system or translated into any 
#              : human or computer language in any form by any means,
#              : electronic, mechanical, manual or other-wise, or 
#              : disclosed to third parties without the express written
#              : permission of Imagination Technologies Limited, Unit 8,
#              : HomePark Industrial Estate, King's Langley, Hertfordshire,
#              : WD4 8LZ, U.K.
#
# Description  : Makefile
#
# Platform     : Windows XP
#
# Modifications:-
# $Log: makefile-bin $
# 
#  --- Revision Logs Removed --- 
#
################################################################################

WINCEPROJ=powervr
#
# Include platform shared SYSGEN makefile
#
!INCLUDE $(_PUBLICROOT)\common\cesysgen\cesysgen.mak

!IF "$(__CLEANMODE)"=="1"
MAKE=$(MAKE) /a
!ENDIF


################################################################################
# Detect the appropriate SGX Default selection to minimize Boot-up time.

!IF "$(DEFAULT_SGX_REV)" == "" 
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"==""
DEFAULT_SGX_REV=121
!ELSE
DEFAULT_SGX_REV=125
!ENDIF
!ELSE
DEFAULT_SGX_REV=125
!ENDIF
!ENDIF

!IF "$(DEFAULT_SGX_REV)"=="121"
SGX_DIR=Rev121
REV121_SUFFIX=
REV125_SUFFIX=_Rev125
!ENDIF
!IF "$(DEFAULT_SGX_REV)"=="125"
SGX_DIR=Rev125
REV121_SUFFIX=_Rev121
REV125_SUFFIX=
!ENDIF

!IF "$(SGX_DIR)"==""
!ERROR Invalid DEFAULT_SGX_REV defined "$(DEFAULT_SGX_REV)"
!ENDIF

!MESSAGE Selected SGX Revision $(DEFAULT_SGX_REV)

_PUBLICROOT_OAK=$(_PUBLICROOT)\powervr\oak

################################################################################

OPENGLROOT=$(EURASIAROOT)\icdkit\OPENGL

!IF "$(SYSGEN_POWERVR)"=="1"
preproc: files $(POWERVR_MODULES) 

postproc: 

!ELSE
preproc:
	@echo PowerVr not built (preproc)
	
postproc:
	@echo PowerVr not built (postproc)

!ENDIF

files:
	-@COPY $(SG_INPUT_ROOT)\oak\files\$(WINCEPROJ).reg $(SG_OUTPUT_ROOT)\oak\files
	-@COPY $(SG_INPUT_ROOT)\oak\files\$(WINCEPROJ).bib $(SG_OUTPUT_ROOT)\oak\files

services: display ws
# Copy the associated files from the Default SGX Directory
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\srvclum.*		$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\srvkm.*			$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\services_test.*	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\sgx_blit_test.*	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\sgx_flip_test.*	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\sgx_init_test.*	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\sgx_render_flip_test.*	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\sgx_clipblit_test.*	$(SG_OUTPUT_OAKTGT)	
# @CESYSGEN IF POWERVR_MODULES_PDUMP
# @CESYSGEN ENDIF
# @CESYSGEN IF POWERVR_MODULES_GPE || POWERVR_MODULES_OMAPSGXGPE
	-@COPY $(_PUBLICROOT_OAK)\lib\$(SGX_DIR)\$(_CPUINDPATH)\srvclkm.*			$(SG_OUTPUT_OAKTGT)
# @CESYSGEN ENDIF
# @CESYSGEN IF POWERVR_MODULES_PVRSCOPESERVICES
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\pvrscopeservices.*	$(SG_OUTPUT_OAKTGT)
# @CESYSGEN ENDIF
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
!MESSAGE Makefile:  Copying files for SGX Core Revision 121= "$(SYSGEN_PVR_SGXCOREREV_121)" with Suffix "$(REV121_SUFFIX)"
# Suffix Exception
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\srvkm.dll 			$(SG_OUTPUT_OAKTGT)\srvkm_Rev121.dll
#
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\srvclum.dll 			$(SG_OUTPUT_OAKTGT)\srvclum$(REV121_SUFFIX).dll 	 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\services_test.exe 	$(SG_OUTPUT_OAKTGT)\services_test$(REV121_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\sgx_blit_test.exe 	$(SG_OUTPUT_OAKTGT)\sgx_blit_test$(REV121_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\sgx_flip_test.exe 	$(SG_OUTPUT_OAKTGT)\sgx_flip_test$(REV121_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\sgx_init_test.exe 	$(SG_OUTPUT_OAKTGT)\sgx_init_test$(REV121_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\sgx_render_flip_test.exe $(SG_OUTPUT_OAKTGT)\sgx_render_flip_test$(REV121_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\sgx_clipblit_test.exe $(SG_OUTPUT_OAKTGT)\sgx_clipblit_test$(REV121_SUFFIX).exe 
# @CESYSGEN IF POWERVR_MODULES_PDUMP
# @CESYSGEN ENDIF
# @CESYSGEN IF POWERVR_MODULES_GPE || POWERVR_MODULES_OMAPSGXGPE
	-@COPY $(_PUBLICROOT_OAK)\lib\Rev121\$(_CPUINDPATH)\srvclkm.*				$(SG_OUTPUT_OAKTGT)\srvclkm$(REV121_SUFFIX).dll
# @CESYSGEN ENDIF
# @CESYSGEN IF POWERVR_MODULES_PVRSCOPESERVICES
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\PVRscopeservices.dll $(SG_OUTPUT_OAKTGT)\PVRscopeservices$(REV121_SUFFIX).dll 	
# @CESYSGEN ENDIF
!ENDIF
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"=="1"
!MESSAGE Makefile:  Copying files for SGX Core Revision 125= "$(SYSGEN_PVR_SGXCOREREV_125)" with Suffix "$(REV125_SUFFIX)"
# Suffix Exception
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\srvkm.dll 			$(SG_OUTPUT_OAKTGT)\srvkm_Rev125.dll
#
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\srvclum.dll 			$(SG_OUTPUT_OAKTGT)\srvclum$(REV125_SUFFIX).dll 	 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\services_test.exe 	$(SG_OUTPUT_OAKTGT)\services_test$(REV125_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\sgx_blit_test.exe 	$(SG_OUTPUT_OAKTGT)\sgx_blit_test$(REV125_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\sgx_flip_test.exe 	$(SG_OUTPUT_OAKTGT)\sgx_flip_test$(REV125_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\sgx_init_test.exe 	$(SG_OUTPUT_OAKTGT)\sgx_init_test$(REV125_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\sgx_render_flip_test.exe $(SG_OUTPUT_OAKTGT)\sgx_render_flip_test$(REV125_SUFFIX).exe 
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\sgx_clipblit_test.exe $(SG_OUTPUT_OAKTGT)\sgx_clipblit_test$(REV125_SUFFIX).exe 
# @CESYSGEN IF POWERVR_MODULES_PDUMP
# @CESYSGEN ENDIF
# @CESYSGEN IF POWERVR_MODULES_GPE || POWERVR_MODULES_OMAPSGXGPE
	-@COPY $(_PUBLICROOT_OAK)\lib\Rev125\$(_CPUINDPATH)\srvclkm.*				$(SG_OUTPUT_OAKTGT)\srvclkm$(REV125_SUFFIX).dll
# @CESYSGEN ENDIF
# @CESYSGEN IF POWERVR_MODULES_PVRSCOPESERVICES
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\PVRscopeservices.dll $(SG_OUTPUT_OAKTGT)\PVRscopeservices$(REV125_SUFFIX).dll 	
# @CESYSGEN ENDIF
!ENDIF


# @CESYSGEN IF POWERVR_MODULES_GPE || POWERVR_MODULES_OMAPSGXGPE
ws:
	-@COPY $(_PUBLICROOT_OAK)\lib\$(_CPUINDPATH)\ddi_powervr.*					$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(_CPUINDPATH)\wince6_proxy.*				$(SG_OUTPUT_OAKTGT)
# @CESYSGEN ELSE
# @CESYSGEN ENDIF


# @CESYSGEN IF POWERVR_MODULES_PVRPDP
# @CESYSGEN ELSE
# @CESYSGEN IF POWERVR_MODULES_DC_NOHW
# @CESYSGEN ELSE
display:
# @CESYSGEN ENDIF
# @CESYSGEN ENDIF

pvr2dtests:
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\pvr2dtest.* 		$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\pvr2d_unittest.* $(SG_OUTPUT_OAKTGT)
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\pvr2dtest.exe 		$(SG_OUTPUT_OAKTGT)\pvr2dtest$(REV121_SUFFIX).exe
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\pvr2d_unittest.exe 	$(SG_OUTPUT_OAKTGT)\pvr2d_unittest$(REV121_SUFFIX).exe	
!ENDIF	
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\pvr2dtest.exe 		$(SG_OUTPUT_OAKTGT)\pvr2dtest$(REV125_SUFFIX).exe
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\pvr2d_unittest.exe 	$(SG_OUTPUT_OAKTGT)\pvr2d_unittest$(REV125_SUFFIX).exe	
!ENDIF	

egl: pvr2dtests
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\libimgegl*.* 	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\libegl.* 		$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\nullws*.* 		$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\eglinfo.*		$(SG_OUTPUT_OAKTGT)	
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\libimgegl.dll 		$(SG_OUTPUT_OAKTGT)\libimgegl$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\libegl.dll 			$(SG_OUTPUT_OAKTGT)\libEGL$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\nullws.dll 			$(SG_OUTPUT_OAKTGT)\nullws$(REV121_SUFFIX).dll
!ENDIF	
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\libimgegl.dll 		$(SG_OUTPUT_OAKTGT)\libimgegl$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\libegl.dll 			$(SG_OUTPUT_OAKTGT)\libEGL$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\nullws.dll 			$(SG_OUTPUT_OAKTGT)\nullws$(REV125_SUFFIX).dll
!ENDIF	

# @CESYSGEN IF POWERVR_MODULES_OPENGL2
# @CESYSGEN ELSE
opengl2:
# @CESYSGEN ENDIF

# @CESYSGEN IF POWERVR_MODULES_OPENGLES2
opengles2: egl
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\libglesv2.*		$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\glslcompiler.*	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\gles2test1.*		$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\gles2_texture_stream.*	$(SG_OUTPUT_OAKTGT)	
	-@COPY $(_PUBLICROOT_OAK)\data\glsltest1_*.txt 								$(_FLATRELEASEDIR)
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\libglesv2.dll 		$(SG_OUTPUT_OAKTGT)\libglesv2$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\glslcompiler.dll 	$(SG_OUTPUT_OAKTGT)\glslcompiler$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\gles2test1.exe 		$(SG_OUTPUT_OAKTGT)\gles2test1$(REV121_SUFFIX).exe
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\gles2_texture_stream.exe $(SG_OUTPUT_OAKTGT)\gles2_texture_stream$(REV121_SUFFIX).exe
!ENDIF	
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\libglesv2.dll 		$(SG_OUTPUT_OAKTGT)\libglesv2$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\glslcompiler.dll 	$(SG_OUTPUT_OAKTGT)\glslcompiler$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\gles2test1.exe 		$(SG_OUTPUT_OAKTGT)\gles2test1$(REV125_SUFFIX).exe
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\gles2_texture_stream.exe $(SG_OUTPUT_OAKTGT)\gles2_texture_stream$(REV125_SUFFIX).exe
!ENDIF	
	
# @CESYSGEN ELSE
# @CESYSGEN ENDIF

# @CESYSGEN IF POWERVR_MODULES_OPENGLES1
opengles1: egl bc_example
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\libgles_CM.* 	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\gles1test1.* 	$(SG_OUTPUT_OAKTGT)
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\libgles_CM.dll 		$(SG_OUTPUT_OAKTGT)\libgles_CM$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\gles1test1.exe 		$(SG_OUTPUT_OAKTGT)\gles1test1$(REV121_SUFFIX).exe
!ENDIF	
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\libgles_CM.dll 		$(SG_OUTPUT_OAKTGT)\libgles_CM$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\gles1test1.exe 		$(SG_OUTPUT_OAKTGT)\gles1test1$(REV125_SUFFIX).exe
!ENDIF	
# @CESYSGEN ELSE
# @CESYSGEN ENDIF

# @CESYSGEN IF POWERVR_MODULES_OPENVG
openvg: egl
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\libopenvg.* 		$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\libopenvgu.* 	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\ovg_unit_test.* 	$(SG_OUTPUT_OAKTGT)
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\libopenvg.dll 		$(SG_OUTPUT_OAKTGT)\libopenvg$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\libopenVGU.dll 		$(SG_OUTPUT_OAKTGT)\libopenVGU$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\ovg_unit_test.exe 	$(SG_OUTPUT_OAKTGT)\ovg_unit_test$(REV121_SUFFIX).exe
!ENDIF	
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\libopenvg.dll 		$(SG_OUTPUT_OAKTGT)\libopenvg$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\libopenVGU.dll 		$(SG_OUTPUT_OAKTGT)\libopenVGU$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\ovg_unit_test.exe 	$(SG_OUTPUT_OAKTGT)\ovg_unit_test$(REV125_SUFFIX).exe
!ENDIF	
# @CESYSGEN ELSE
# @CESYSGEN ENDIF

# @CESYSGEN IF POWERVR_MODULES_OPENGLES1
bc_example:
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\bc_example.* 	$(SG_OUTPUT_OAKTGT)
	-@COPY $(_PUBLICROOT_OAK)\target\$(SGX_DIR)\$(_CPUINDPATH)\gles1_texture_stream.* 	$(SG_OUTPUT_OAKTGT)
!IF  "$(SYSGEN_PVR_SGXCOREREV_121)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\bc_example.dll 		$(SG_OUTPUT_OAKTGT)\bc_example$(REV121_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev121\$(_CPUINDPATH)\gles1_texture_stream.exe $(SG_OUTPUT_OAKTGT)\gles1_texture_stream$(REV121_SUFFIX).exe
!ENDIF	
!IF  "$(SYSGEN_PVR_SGXCOREREV_125)"=="1"
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\bc_example.dll 		$(SG_OUTPUT_OAKTGT)\bc_example$(REV125_SUFFIX).dll
	-@COPY $(_PUBLICROOT_OAK)\target\Rev125\$(_CPUINDPATH)\gles1_texture_stream.exe $(SG_OUTPUT_OAKTGT)\gles1_texture_stream$(REV125_SUFFIX).exe
!ENDIF	
# @CESYSGEN ELSE
# @CESYSGEN ENDIF

3dapi: opengl2 opengles2 opengles1 openvg bc_example


