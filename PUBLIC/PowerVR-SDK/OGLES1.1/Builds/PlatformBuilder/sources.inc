!INCLUDE $(SDKROOT)\Builds\PlatformBuilder\sources.cmn

PLATFORM = WindowsCE6

CDEFINES = $(CDEFINES) -DBUILD_$(API)=1

SHELLROOT    = $(SDKROOT)\Shell
SHELLAPIROOT = $(SDKROOT)\Shell\API\KEGL
SHELLOSROOT  = $(SDKROOT)\Shell\OS\Windows
TOOLSAPI	 = $(SDKROOT)\Tools\$(API)

TOOLSLIB	 = $(TOOLSAPI)\Build\PlatformBuilder\oak\lib\$(_TGTCPU)\$(WINCEDEBUG)\

WINCETARGETFILE0=\
	$(WINCETARGETFILE0)			\
	$(OBJDIR)\PVRShell.obj		\
	$(OBJDIR)\PVRShellAPI.obj	\
	$(OBJDIR)\PVRShellOS.obj				
	
INCLUDES=\
	$(INCLUDES)									\
	$(APPROOT);										\
	$(SDKROOT)\Tools;							\
	$(TOOLSAPI);					\
	$(SHELLROOT);								\
	$(SHELLAPIROOT);							\
	$(SHELLOSROOT);

SOURCES= $(SOURCES)		    				\
	     $(SHELLROOT)\PVRShell.cpp			\
		 $(SHELLOSROOT)\PVRShellOS.cpp  	\
		 $(SHELLAPIROOT)\PVRShellAPI.cpp

TARGETLIBS = \
	$(BUILDSDKROOT)\lib\$(_CPUINDPATH)\coredll.lib
	

TARGETLIBS= \
	$(TARGETLIBS)						\
	$(SDKROOT)\Builds\$(API)\$(PLATFORM)$(_TGTCPU)\Lib\libgles_cm.lib	\
	$(TOOLSLIB)\OGLESTools.lib
	
INCLUDES=\
	$(INCLUDES)									\
	$(SDKROOT)\Builds\$(API)\Include;			\
	$(SDKROOT)\Builds\$(API)\Windows\Include;
	


	

