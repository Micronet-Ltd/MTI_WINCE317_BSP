
$(OBJDIR)\OVGHelloTriangle_Windows.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\OVGHelloTriangle_Windows.cpp


