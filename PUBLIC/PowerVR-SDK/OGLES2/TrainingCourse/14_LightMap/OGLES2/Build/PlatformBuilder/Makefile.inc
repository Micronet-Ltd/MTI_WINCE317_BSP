
$(OBJDIR)\$(TARGETNAME).obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\$(TARGETNAME).cpp

$(OBJDIR)\Basetex.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\Basetex.cpp

$(OBJDIR)\Reflection.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\Reflection.cpp

$(OBJDIR)\Shadow.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\Shadow.cpp

$(OBJDIR)\FragShader.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\FragShader.cpp

$(OBJDIR)\VertShader.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\VertShader.cpp

$(OBJDIR)\Mask.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\Mask.cpp

$(OBJDIR)\Plane.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\Plane.cpp


!INCLUDE $(SDKROOT)\builds\PlatformBuilder\demo.inc

